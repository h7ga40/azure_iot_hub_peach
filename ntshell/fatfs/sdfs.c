/* mbed Microcontroller Library
	* Copyright (c) 2006-2012 ARM Limited
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	* SOFTWARE.
	*/

/* Introduction
	* ------------
	* SD and MMC cards support a number of interfaces, but common to them all
	* is one based on SPI. This is the one I'm implmenting because it means
	* it is much more portable even though not so performant, and we already
	* have the mbed SPI Interface!
	*
	* The main reference I'm using is Chapter 7, "SPI Mode" of:
	*  http://www.sdcard.org/developers/tech/sdcard/pls/Simplified_Physical_Layer_Spec.pdf
	*
	* SPI Startup
	* -----------
	* The SD card powers up in SD mode. The SPI interface mode is selected by
	* asserting CS low and sending the reset command (CMD0). The card will
	* respond with a (R1) response.
	*
	* CMD8 is optionally sent to determine the voltage range supported, and
	* indirectly determine whether it is a version 1.x SD/non-SD card or
	* version 2.x. I'll just ignore this for now.
	*
	* ACMD41 is repeatedly issued to initialise the card, until "in idle"
	* (bit 0) of the R1 response goes to '0', indicating it is initialised.
	*
	* You should also indicate whether the host supports High Capicity cards,
	* and check whether the card is high capacity - i'll also ignore this
	*
	* SPI Protocol
	* ------------
	* The SD SPI protocol is based on transactions made up of 8-bit words, with
	* the host starting every bus transaction by asserting the CS signal low. The
	* card always responds to commands, data blocks and errors.
	*
	* The protocol supports a CRC, but by default it is off (except for the
	* first reset CMD0, where the CRC can just be pre-calculated, and CMD8)
	* I'll leave the CRC off I think!
	*
	* Standard capacity cards have variable data block sizes, whereas High
	* Capacity cards fix the size of data block to 512 bytes. I'll therefore
	* just always use the Standard Capacity cards with a block size of 512 bytes.
	* This is set with CMD16.
	*
	* You can read and write single blocks (CMD17, CMD25) or multiple blocks
	* (CMD18, CMD25). For simplicity, I'll just use single block accesses. When
	* the card gets a read command, it responds with a response token, and then
	* a data token or an error.
	*
	* SPI Command Format
	* ------------------
	* Commands are 6-bytes long, containing the command, 32-bit argument, and CRC.
	*
	* +---------------+------------+------------+-----------+----------+--------------+
	* | 01 | cmd[5:0] | arg[31:24] | arg[23:16] | arg[15:8] | arg[7:0] | crc[6:0] | 1 |
	* +---------------+------------+------------+-----------+----------+--------------+
	*
	* As I'm not using CRC, I can fix that byte to what is needed for CMD0 (0x95)
	*
	* All Application Specific commands shall be preceded with APP_CMD (CMD55).
	*
	* SPI Response Format
	* -------------------
	* The main response format (R1) is a status byte (normally zero). Key flags:
	*  idle - 1 if the card is in an idle state/initialising
	*  cmd  - 1 if an illegal command code was detected
	*
	*    +-------------------------------------------------+
	* R1 | 0 | arg | addr | seq | crc | cmd | erase | idle |
	*    +-------------------------------------------------+
	*
	* R1b is the same, except it is followed by a busy signal (zeros) until
	* the first non-zero byte when it is ready again.
	*
	* Data Response Token
	* -------------------
	* Every data block written to the card is acknowledged by a byte
	* response token
	*
	* +----------------------+
	* | xxx | 0 | status | 1 |
	* +----------------------+
	*              010 - OK!
	*              101 - CRC Error
	*              110 - Write Error
	*
	* Single Block Read and Write
	* ---------------------------
	*
	* Block transfers have a byte header, followed by the data, followed
	* by a 16-bit CRC. In our case, the data will always be 512 bytes.
	*
	* +------+---------+---------+- -  - -+---------+-----------+----------+
	* | 0xFE | data[0] | data[1] |        | data[n] | crc[15:8] | crc[7:0] |
	* +------+---------+---------+- -  - -+---------+-----------+----------+
	*/
#include "sdfs.h"
#include "diskio.h"
//#include "mbed_debug.h"
#include "mbed_wait_api.h"
#include <stdio.h>

#define debug(...) printf(__VA_ARGS__)
#define debug_if(cond, ...) if(cond){ printf(__VA_ARGS__); }

static int sdfs_initialise_card_v1(sdfs_t *obj);
static int sdfs_initialise_card_v2(sdfs_t *obj);
static int sdfs__cmd(sdfs_t *obj, int cmd, int arg);
static int sdfs__cmd8(sdfs_t *obj);
static int sdfs__cmd58(sdfs_t *obj);
static uint64_t sdfs__sd_sectors(sdfs_t *obj);
static int sdfs__write(sdfs_t *obj, const uint8_t*buffer, uint32_t length);
static int sdfs__read(sdfs_t *obj, uint8_t *buffer, uint32_t length);

#define SD_COMMAND_TIMEOUT 5000

#define SD_DBG             0

void sdfs_init(sdfs_t *obj, PinName mosi, PinName miso, PinName sclk, PinName cs, const char* name)
{
	obj->name = name;
	spi_init(&obj->_spi, mosi, miso, sclk, NC);
	gpio_init_out(&obj->_cs, cs);
	gpio_write(&obj->_cs, 1);

    // Set default to 100kHz for initialisation and 1MHz for data transfer
    obj->_init_sck = 100000;
    obj->_transfer_sck = 1000000;
	obj->_is_initialized = STA_NOINIT;
}

#define R1_IDLE_STATE           (1 << 0)
#define R1_ERASE_RESET          (1 << 1)
#define R1_ILLEGAL_COMMAND      (1 << 2)
#define R1_COM_CRC_ERROR        (1 << 3)
#define R1_ERASE_SEQUENCE_ERROR (1 << 4)
#define R1_ADDRESS_ERROR        (1 << 5)
#define R1_PARAMETER_ERROR      (1 << 6)

int sdfs_initialise_card(sdfs_t *obj)
{
	spi_format(&obj->_spi, 8, 0, 0);
	// Set to 100kHz for initialisation, and clock card with cs = 1
	spi_frequency(&obj->_spi, obj->_init_sck);
	gpio_write(&obj->_cs, 1);
	for (int i = 0; i < 16; i++) {
		spi_master_write(&obj->_spi, 0xFF);
	}

	// send CMD0, should return with all zeros except IDLE STATE set (bit 0)
	if (sdfs__cmd(obj, 0, 0) != R1_IDLE_STATE) {
		debug("No disk, or could not put SD card in to SPI idle state\n");
		return CT_FAIL;
	}

	// send CMD8 to determine whther it is ver 2.x
	int r = sdfs__cmd8(obj);
	if (r == R1_IDLE_STATE) {
		return sdfs_initialise_card_v2(obj);
	}
	else if (r == (R1_IDLE_STATE | R1_ILLEGAL_COMMAND)) {
		return sdfs_initialise_card_v1(obj);
	}
	else {
		debug("Not in idle state after sending CMD8 (not an SD card?)\n");
		return CT_FAIL;
	}
}

static int sdfs_initialise_card_v1(sdfs_t *obj)
{
	sdfs__cmd(obj, 55, 0);
	if (sdfs__cmd(obj, 41, 0) <= 1) {
		obj->cdv = 512;
		debug_if(SD_DBG, "\n\rInit: SEDCARD_V1\n\r");
		for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
			sdfs__cmd(obj, 55, 0);
			if (sdfs__cmd(obj, 41, 0) == 0) {
				break;
			}
		}
		return CT_SD1;
	}
	else {
		for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
			if (sdfs__cmd(obj, 1, 0) == 0)
				break;
		}
		return CT_MMC;
	}
	//if (!Timer1 || sdfs__cmd(obj, 16, 512) != 0)	/* Set block length: 512 */
	//	ty = 0;

	debug("Timeout waiting for v1.x card\n");
	return CT_FAIL;
}

static int sdfs_initialise_card_v2(sdfs_t *obj)
{
	int n;
	uint8_t ocr[4];

	/* Get 32 bit return value of R7 resp */
	for (n = 0; n < 4; n++) ocr[n] = spi_master_write(&obj->_spi, 0xFF);
	for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
		sdfs__cmd(obj, 55, 0);
		if (sdfs__cmd(obj, 41, 0x40000000) == 0) {
			if (sdfs__cmd58(obj) == 0) {
				for (n = 0; n < 4; n++) ocr[n] = spi_master_write(&obj->_spi, 0xFF);
				debug_if(SD_DBG, "\n\rInit: SDCARD_V2\n\r");
				obj->cdv = 1;
				return (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
			}
		}
		wait_ms(50);
	}

	debug("Timeout waiting for v2.x card\n");
	return CT_FAIL;
}

int sdfs_initialize(sdfs_t *obj)
{
	obj->_is_initialized |= STA_NOINIT;

	obj->_card_type = sdfs_initialise_card(obj);
	if (obj->_card_type == 0) {
		debug("Fail to initialize card\n");
		return 1;
	}
	debug_if(SD_DBG, "init card = %x\n", obj->_is_initialized);

	obj->_sectors = sdfs__sd_sectors(obj);

	if (sdfs__cmdx(obj, 10, 0) != 0		/* READ_CID */
		|| sdfs__read(obj, obj->_cid, 16) != 0) {
		obj->_card_type = CT_FAIL;
		debug("Fail to read cid\n");
		return 1;
	}

	// Set block length to 512 (CMD16)
	if (sdfs__cmd(obj, 16, 512) != 0) {
		obj->_card_type = CT_FAIL;
		debug("Set 512-byte block timed out\n");
		return 1;
	}

	obj->_is_initialized &= ~STA_NOINIT;

	// Set SCK for data transfer
	spi_frequency(&obj->_spi, obj->_transfer_sck);
	return 0;
}

int sdfs_write(sdfs_t *obj, const uint8_t *buffer, uint32_t block_number, uint32_t count)
{
	if (obj->_is_initialized & (STA_NOINIT | STA_PROTECT)) {
		return -1;
	}

	for (uint32_t b = block_number; b < block_number + count; b++) {
		// set write address for single block (CMD24)
		if (sdfs__cmd(obj, 24, b * obj->cdv) != 0) {
			return 1;
		}

		// send the data block
		sdfs__write(obj, buffer, 512);
		buffer += 512;
	}

	return 0;
}

int sdfs_read(sdfs_t *obj, uint8_t *buffer, uint32_t block_number, uint32_t count)
{
	if (obj->_is_initialized & STA_NOINIT) {
		return -1;
	}

	for (uint32_t b = block_number; b < block_number + count; b++) {
		// set read address for single block (CMD17)
		if (sdfs__cmd(obj, 17, b * obj->cdv) != 0) {
			return 1;
		}

		// receive the data
		sdfs__read(obj, buffer, 512);
		buffer += 512;
	}

	return 0;
}

int sdfs_status(sdfs_t *obj)
{
	// FATFileSystem::disk_status() returns 0 when initialized
	if ((obj->_is_initialized & STA_NOINIT) == 0) {
		return 0;
	}
	else {
		return 1;
	}
}

uint64_t sdfs_sectors(sdfs_t *obj) { return obj->_sectors; }


// PRIVATE FUNCTIONS
static int sdfs__cmd(sdfs_t *obj, int cmd, int arg)
{
	gpio_write(&obj->_cs, 0);

	// send a command
	spi_master_write(&obj->_spi, 0x40 | cmd);
	spi_master_write(&obj->_spi, arg >> 24);
	spi_master_write(&obj->_spi, arg >> 16);
	spi_master_write(&obj->_spi, arg >> 8);
	spi_master_write(&obj->_spi, arg >> 0);
	spi_master_write(&obj->_spi, 0x95);

	// wait for the repsonse (response[7] == 0)
	for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
		int response = spi_master_write(&obj->_spi, 0xFF);
		if (!(response & 0x80)) {
			gpio_write(&obj->_cs, 1);
			spi_master_write(&obj->_spi, 0xFF);
			return response;
		}
	}
	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return -1; // timeout
}

int sdfs__cmdx(sdfs_t *obj, int cmd, int arg)
{
	gpio_write(&obj->_cs, 0);

	// send a command
	spi_master_write(&obj->_spi, 0x40 | cmd);
	spi_master_write(&obj->_spi, arg >> 24);
	spi_master_write(&obj->_spi, arg >> 16);
	spi_master_write(&obj->_spi, arg >> 8);
	spi_master_write(&obj->_spi, arg >> 0);
	spi_master_write(&obj->_spi, 0x95);

	// wait for the repsonse (response[7] == 0)
	for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
		int response = spi_master_write(&obj->_spi, 0xFF);
		if (!(response & 0x80)) {
			return response;
		}
	}
	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return -1; // timeout
}

static int sdfs__cmd58(sdfs_t *obj)
{
	gpio_write(&obj->_cs, 0);
	int arg = 0;

	// send a command
	spi_master_write(&obj->_spi, 0x40 | 58);
	spi_master_write(&obj->_spi, arg >> 24);
	spi_master_write(&obj->_spi, arg >> 16);
	spi_master_write(&obj->_spi, arg >> 8);
	spi_master_write(&obj->_spi, arg >> 0);
	spi_master_write(&obj->_spi, 0x95);

	// wait for the repsonse (response[7] == 0)
	for (int i = 0; i < SD_COMMAND_TIMEOUT; i++) {
		int response = spi_master_write(&obj->_spi, 0xFF);
		if (!(response & 0x80)) {
			int ocr = spi_master_write(&obj->_spi, 0xFF) << 24;
			ocr |= spi_master_write(&obj->_spi, 0xFF) << 16;
			ocr |= spi_master_write(&obj->_spi, 0xFF) << 8;
			ocr |= spi_master_write(&obj->_spi, 0xFF) << 0;
			gpio_write(&obj->_cs, 1);
			spi_master_write(&obj->_spi, 0xFF);
			return response;
		}
	}
	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return -1; // timeout
}

static int sdfs__cmd8(sdfs_t *obj)
{
	gpio_write(&obj->_cs, 0);

	// send a command
	spi_master_write(&obj->_spi, 0x40 | 8); // CMD8
	spi_master_write(&obj->_spi, 0x00);     // reserved
	spi_master_write(&obj->_spi, 0x00);     // reserved
	spi_master_write(&obj->_spi, 0x01);     // 3.3v
	spi_master_write(&obj->_spi, 0xAA);     // check pattern
	spi_master_write(&obj->_spi, 0x87);     // crc

	// wait for the repsonse (response[7] == 0)
	for (int i = 0; i < SD_COMMAND_TIMEOUT * 1000; i++) {
		char response[5];
		response[0] = spi_master_write(&obj->_spi, 0xFF);
		if (!(response[0] & 0x80)) {
			for (int j = 1; j < 5; j++) {
				response[i] = spi_master_write(&obj->_spi, 0xFF);
			}
			gpio_write(&obj->_cs, 1);
			spi_master_write(&obj->_spi, 0xFF);
			return response[0];
		}
	}
	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return -1; // timeout
}

static int sdfs__read(sdfs_t *obj, uint8_t *buffer, uint32_t length)
{
	gpio_write(&obj->_cs, 0);

	// read until start byte (0xFF)
	while (spi_master_write(&obj->_spi, 0xFF) != 0xFE);

	// read data
	for (int i = 0; i < length; i++) {
		buffer[i] = spi_master_write(&obj->_spi, 0xFF);
	}
	spi_master_write(&obj->_spi, 0xFF); // checksum
	spi_master_write(&obj->_spi, 0xFF);

	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return 0;
}

static int sdfs__write(sdfs_t *obj, const uint8_t*buffer, uint32_t length)
{
	gpio_write(&obj->_cs, 0);

	// indicate start of block
	spi_master_write(&obj->_spi, 0xFE);

	// write the data
	for (int i = 0; i < length; i++) {
		spi_master_write(&obj->_spi, buffer[i]);
	}

	// write the checksum
	spi_master_write(&obj->_spi, 0xFF);
	spi_master_write(&obj->_spi, 0xFF);

	// check the response token
	if ((spi_master_write(&obj->_spi, 0xFF) & 0x1F) != 0x05) {
		gpio_write(&obj->_cs, 1);
		spi_master_write(&obj->_spi, 0xFF);
		return 1;
	}

	// wait for write to finish
	while (spi_master_write(&obj->_spi, 0xFF) == 0);

	gpio_write(&obj->_cs, 1);
	spi_master_write(&obj->_spi, 0xFF);
	return 0;
}

static uint32_t sdfs_ext_bits(sdfs_t *obj, unsigned char *data, int msb, int lsb)
{
	uint32_t bits = 0;
	uint32_t size = 1 + msb - lsb;
	for (uint32_t i = 0; i < size; i++) {
		uint32_t position = lsb + i;
		uint32_t byte = 15 - (position >> 3);
		uint32_t bit = position & 0x7;
		uint32_t value = (data[byte] >> bit) & 1;
		bits |= value << i;
	}
	return bits;
}

static uint64_t sdfs__sd_sectors(sdfs_t *obj)
{
	uint32_t c_size, c_size_mult, read_bl_len;
	uint32_t mult, blocknr;
	uint32_t hc_c_size;
	uint64_t blocks;

	// CMD9, Response R2 (R1 byte + 16-byte block read)
	if (sdfs__cmdx(obj, 9, 0) != 0) {
		debug("Didn't get a response from the disk\n");
		return 0;
	}

	if (sdfs__read(obj, obj->_csd, 16) != 0) {
		debug("Couldn't read csd response from disk\n");
		return 0;
	}

	// csd_structure : csd[127:126]
	// c_size        : csd[73:62]
	// c_size_mult   : csd[49:47]
	// read_bl_len   : csd[83:80] - the *maximum* read block length

	int csd_structure = sdfs_ext_bits(obj, obj->_csd, 127, 126);

	switch (csd_structure) {
	case 0:
		obj->cdv = 512;
		c_size = sdfs_ext_bits(obj, obj->_csd, 73, 62);
		c_size_mult = sdfs_ext_bits(obj, obj->_csd, 49, 47);
		read_bl_len = sdfs_ext_bits(obj, obj->_csd, 83, 80);

		obj->_block_len = 1 << read_bl_len;
		mult = 1 << (c_size_mult + 2);
		blocknr = (c_size + 1) * mult;
		obj->_capacity = blocknr * obj->_block_len;
		blocks = obj->_capacity / 512;
		debug_if(SD_DBG, "\n\rSDCard\n\rc_size: %d \n\rcapacity: %ld \n\rsectors: %lld\n\r", c_size, obj->_capacity, blocks);
		break;

	case 1:
		obj->cdv = 1;
		hc_c_size = sdfs_ext_bits(obj, obj->_csd, 63, 48);
		blocks = (hc_c_size + 1) * 1024;
		debug_if(SD_DBG, "\n\rSDHC Card \n\rhc_c_size: %d\n\rcapacity: %lld \n\rsectors: %lld\n\r", hc_c_size, blocks * 512, blocks);
		break;

	default:
		debug("CSD struct unsupported\r\n");
		return 0;
	};
	return blocks;
}

static
int wait_ready(sdfs_t *obj, uint32_t wt)
{
	/* Wait until card goes ready or timeout */
	for (int i = 0; i < wt; i++) {
		if (spi_master_write(&obj->_spi, 0xFF) == 0xFF)
			/* Card goes ready */
			return 1;
	}

	/* Timeout occured */
	return 0;
}

static
void deselect(sdfs_t *obj)
{
	/* Set CS# high */
	gpio_write(&obj->_cs, 0);

	/* Dummy clock (force DO hi-z for multiple slave SPI) */
	spi_master_write(&obj->_spi, 0xFF);
}

static
int select(sdfs_t *obj)
{
	/* Set CS# low */
	gpio_write(&obj->_cs, 1);

	/* Dummy clock (force DO enabled) */
	spi_master_write(&obj->_spi, 0xFF);

	/* Wait for card ready */
	if (wait_ready(obj, 500))
		return 0;

	deselect(obj);

	/* Failed to select the card due to timeout */
	return -1;
}

int sdfs_sync(sdfs_t *obj)
{
	int ret = select(obj);
	deselect(obj);
	return ret;
}

int sdfs_trim(sdfs_t *obj, uint32_t st, uint32_t ed)
{
	int res = -1;

	for(;;) {
		/* Check if the card is SDC */
		if (!(obj->_card_type & CT_SDC))
			break;
		/* Check if sector erase can be applied to the card */
		if (!(obj->_csd[0] >> 6) && !(obj->_csd[10] & 0x40))
			break;
		/* Load sector block */
		if (!(obj->_card_type & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		/* Erase sector block */
		if ((sdfs__cmd(obj, 32, st) == 0)
			&& (sdfs__cmd(obj, 33, ed) == 0)
			&& (sdfs__cmd(obj, 38, 0) == 0)
			&& wait_ready(obj, 30000))
			res = 0;
	}

	return res;
}

int sdfs_get_ocr(sdfs_t *obj, uint8_t buff[4])
{
	int res = -1, n;

	/* READ_OCR */
	if (sdfs__cmd58(obj) == 0) {
		for (n = 4; n; n--) {
			*buff++ = spi_master_write(&obj->_spi, 0xFF);
		}
		res = 0;
	}

	deselect(obj);

	return res;
}

int sdfs_get_sdstat(sdfs_t *obj, uint8_t buff[64])
{
	int res = -1;

	sdfs__cmd(obj, 55, 0);

	/* SD_STATUS */
	if (sdfs__cmd(obj, 13, 0) == 0) {
		spi_master_write(&obj->_spi, 0xFF);
		if (sdfs__read(obj, buff, 64) == 0)
			return 0;
	}

	deselect(obj);

	return res;
}
