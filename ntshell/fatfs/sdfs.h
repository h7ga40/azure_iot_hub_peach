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
#ifndef MBED_SDFILESYSTEM_H
#define MBED_SDFILESYSTEM_H

#include <stdint.h>
//#include <time.h>
#define __NEED_time_t
#include "../musl-1.1.18/include/bits/alltypes.h"
#include "gpio_api.h"
#include "spi_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_FAIL		0x00
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK	0x08		/* Block addressing */

/** Access the filesystem on an SD Card using SPI
	*
	* @code
	* #include "mbed.h"
	* #include "SDFileSystem.h"
	*
	* SDFileSystem sd(p5, p6, p7, p12, "sd"); // MOSI, MISO, SCLK, SSEL
	*  
	* int main() {
	*     FILE *fp = fopen("/sd/mbed.txt", "w");
	*     fprintf(fp, "Hello World!\n");
	*     fclose(fp);
	* }
	* @endcode
	*/
typedef struct sdfs_s{
	const char* name;
	volatile uint8_t _is_initialized;
	uint8_t _card_type;
	uint8_t _csd[16], _cid[16];
	uint64_t _capacity;
	uint64_t _block_len;
	uint64_t _sectors;
	spi_t _spi;
	gpio_t _cs;
	int cdv;
    uint32_t _init_sck;
    uint32_t _transfer_sck;
} sdfs_t;

/** Create the File System for accessing an SD Card using SPI
	*
	* @param mosi SPI mosi pin connected to SD Card
	* @param miso SPI miso pin conencted to SD Card
	* @param sclk SPI sclk pin connected to SD Card
	* @param cs   DigitalOut pin used as SD Card chip select
	* @param name The name used to access the virtual filesystem
	*/
void sdfs_init(sdfs_t *obj, PinName mosi, PinName miso, PinName sclk, PinName cs, const char* name);

int sdfs_initialize(sdfs_t *obj);
int sdfs_status(sdfs_t *obj);
int sdfs_read(sdfs_t *obj, uint8_t * buffer, uint32_t sector, uint32_t count);
int sdfs_write(sdfs_t *obj, const uint8_t * buffer, uint32_t sector, uint32_t count);
int sdfs_sync(sdfs_t *obj);
uint64_t sdfs_sectors(sdfs_t *obj);
int sdfs_trim(sdfs_t *obj, uint32_t st, uint32_t ed);
int sdfs_get_ocr(sdfs_t *obj, uint8_t buff[4]);
int sdfs_get_sdstat(sdfs_t *obj, uint8_t buff[64]);

#ifdef __cplusplus
}
#endif

#endif
