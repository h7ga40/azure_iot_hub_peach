/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/

#include <string.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include <time.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "adafruit_ssd1306.h"

#define HIGH 1
#define LOW 0

uint16_t lcd_init_height = SSD1306_LCDHEIGHT;
uint16_t lcd_init_width = SSD1306_LCDWIDTH;

// the memory buffer for the LCD

static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x80, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0xFF,
#if (SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH > 96*16)
0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
0x80, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x8C, 0x8E, 0x84, 0x00, 0x00, 0x80, 0xF8,
0xF8, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80,
0x00, 0xE0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xC7, 0x01, 0x01,
0x01, 0x01, 0x83, 0xFF, 0xFF, 0x00, 0x00, 0x7C, 0xFE, 0xC7, 0x01, 0x01, 0x01, 0x01, 0x83, 0xFF,
0xFF, 0xFF, 0x00, 0x38, 0xFE, 0xC7, 0x83, 0x01, 0x01, 0x01, 0x83, 0xC7, 0xFF, 0xFF, 0x00, 0x00,
0x01, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x07, 0x01, 0x01, 0x01, 0x00, 0x00, 0x7F, 0xFF,
0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0xFF,
0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x0F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x8F,
0x8F, 0x9F, 0xBF, 0xFF, 0xFF, 0xC3, 0xC0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC,
0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x01, 0x03, 0x03, 0x03,
0x03, 0x03, 0x01, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01,
0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x03,
0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#if (SSD1306_LCDHEIGHT == 64)
0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x0F,
0x87, 0xC7, 0xF7, 0xFF, 0xFF, 0x1F, 0x1F, 0x3D, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0x7C, 0x7D, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x00, 0x30, 0x30, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F,
0x0F, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xE0,
0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00,
0x00, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xF0, 0xF8, 0x1C, 0x0E,
0x06, 0x06, 0x06, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xFC,
0xFE, 0xFC, 0x00, 0x18, 0x3C, 0x7E, 0x66, 0xE6, 0xCE, 0x84, 0x00, 0x00, 0x06, 0xFF, 0xFF, 0x06,
0x06, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x06, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0xC0, 0xF8,
0xFC, 0x4E, 0x46, 0x46, 0x46, 0x4E, 0x7C, 0x78, 0x40, 0x18, 0x3C, 0x76, 0xE6, 0xCE, 0xCC, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x0F, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00,
0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0C,
0x18, 0x18, 0x0C, 0x06, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x0F, 0x0E, 0x0C, 0x18, 0x0C, 0x0F,
0x07, 0x01, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00,
0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x07,
0x07, 0x0C, 0x0C, 0x18, 0x1C, 0x0C, 0x06, 0x06, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif
#endif
};

void lcd_fastSPIwrite(LCD_Handler_t *lcd, uint8_t d);
#define lcd_swap(a, b) { int16_t t = a; a = b; b = t; }

// the most basic function, set a single pixel
void lcd_drawPixel(LCD_Handler_t *lcd, int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= lcd->_width) || (y < 0) || (y >= lcd->_height))
		return;

	// check rotation, move pixel around if necessary
	switch (lcd->rotation) {
	case 1:
		lcd_swap(x, y);
		x = lcd->_width - x - 1;
		break;
	case 2:
		x = lcd->_width - x - 1;
		y = lcd->_height - y - 1;
		break;
	case 3:
		lcd_swap(x, y);
		y = lcd->_height - y - 1;
		break;
	}

	// x is which column
	switch (color)
	{
	case WHITE:   buffer[x + (y / 8)*SSD1306_LCDWIDTH] |= (1 << (y & 7)); break;
	case BLACK:   buffer[x + (y / 8)*SSD1306_LCDWIDTH] &= ~(1 << (y & 7)); break;
	case INVERSE: buffer[x + (y / 8)*SSD1306_LCDWIDTH] ^= (1 << (y & 7)); break;
	}
}

void lcd_init1(LCD_Handler_t *lcd, int8_t SID, int8_t SCLK, int8_t DC, int8_t RST, int8_t CS)
{
	lcd->_width = SSD1306_LCDWIDTH;
	lcd->_height = SSD1306_LCDHEIGHT;
	gpio_init(&lcd->cs, CS);
	gpio_init(&lcd->rst, RST);
	gpio_init(&lcd->dc, DC);
	gpio_init(&lcd->sclk, SCLK);
	gpio_init(&lcd->sid, SID);
	lcd->hwSPI = false;
}

// constructor for hardware SPI - we indicate DataCommand, ChipSelect, Reset
void lcd_init2(LCD_Handler_t *lcd, int8_t DC, int8_t RST, int8_t CS)
{
	lcd->_width = SSD1306_LCDWIDTH;
	lcd->_height = SSD1306_LCDHEIGHT;
	gpio_init(&lcd->dc, DC);
	gpio_init(&lcd->rst, RST);
	gpio_init(&lcd->cs, CS);
	lcd->hwSPI = true;
}

// initializer for I2C - we only indicate the reset pin!
void lcd_init3(LCD_Handler_t *lcd, int8_t reset)
{
	lcd->_width = SSD1306_LCDWIDTH;
	lcd->_height = SSD1306_LCDHEIGHT;
	gpio_init(&lcd->sclk, NC);
	gpio_init(&lcd->dc, NC);
	gpio_init(&lcd->cs, NC);
	gpio_init(&lcd->sid, NC);
	gpio_init(&lcd->rst, reset);
}

void lcd_begin(LCD_Handler_t *lcd, uint8_t vccstate, uint8_t i2caddr, bool_t reset) {
	lcd->_vccstate = vccstate;
	lcd->_i2caddr = i2caddr;

	// set pin directions
	if (lcd->sid.pin != NC) {
		gpio_dir(&lcd->dc, PIN_OUTPUT);
		gpio_dir(&lcd->cs, PIN_OUTPUT);
		if (!lcd->hwSPI) {
			// set pins for software-SPI
			gpio_dir(&lcd->sid, PIN_OUTPUT);
			gpio_dir(&lcd->sclk, PIN_OUTPUT);
		}
		if (lcd->hwSPI) {
			spi_format(&lcd->spi, 8, 0, 0);
			spi_master_write(&lcd->spi, 8000000);
		}
	}
	else
	{
		// I2C Init
		//i2c_begin(&lcd->i2c);
#ifdef __SAM3X8E__
		// Force 400 KHz I2C, rawr! (Uses pins 20, 21 for SDA, SCL)
		TWI1->TWI_CWGR = 0;
		TWI1->TWI_CWGR = ((VARIANT_MCK / (2 * 400000)) - 4) * 0x101;
#endif
	}
	if ((reset) && (lcd->rst.pin != NC)) {
		// Setup reset pin direction (used by both SPI and I2C)
		gpio_dir(&lcd->rst, PIN_OUTPUT);
		gpio_write(&lcd->rst, HIGH);
		// VDD (3.3V) goes high at start, lets just chill for a ms
		dly_tsk(1 * 1000);
		// bring reset low
		gpio_write(&lcd->rst, LOW);
		// wait 10ms
		dly_tsk(10 * 1000);
		// bring out of reset
		gpio_write(&lcd->rst, HIGH);
		// turn on VCC (9V?)
	}

	// Init sequence
	lcd_command(lcd, SSD1306_DISPLAYOFF);                    // 0xAE
	lcd_command(lcd, SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
	lcd_command(lcd, 0x80);                                  // the suggested ratio 0x80

	lcd_command(lcd, SSD1306_SETMULTIPLEX);                  // 0xA8
	lcd_command(lcd, SSD1306_LCDHEIGHT - 1);

	lcd_command(lcd, SSD1306_SETDISPLAYOFFSET);              // 0xD3
	lcd_command(lcd, 0x0);                                   // no offset
	lcd_command(lcd, SSD1306_SETSTARTLINE | 0x0);            // line #0
	lcd_command(lcd, SSD1306_CHARGEPUMP);                    // 0x8D
	if (vccstate == SSD1306_EXTERNALVCC)
	{
		lcd_command(lcd, 0x10);
	}
	else
	{
		lcd_command(lcd, 0x14);
	}
	lcd_command(lcd, SSD1306_MEMORYMODE);                    // 0x20
	lcd_command(lcd, 0x00);                                  // 0x0 act like ks0108
	lcd_command(lcd, SSD1306_SEGREMAP | 0x1);
	lcd_command(lcd, SSD1306_COMSCANDEC);

#if defined SSD1306_128_32
	lcd_command(lcd, SSD1306_SETCOMPINS);                    // 0xDA
	lcd_command(lcd, 0x02);
	lcd_command(lcd, SSD1306_SETCONTRAST);                   // 0x81
	lcd_command(lcd, 0x8F);

#elif defined SSD1306_128_64
	lcd_command(lcd, SSD1306_SETCOMPINS);                    // 0xDA
	lcd_command(lcd, 0x12);
	lcd_command(lcd, SSD1306_SETCONTRAST);                   // 0x81
	if (vccstate == SSD1306_EXTERNALVCC)
	{
		lcd_command(lcd, 0x9F);
	}
	else
	{
		lcd_command(lcd, 0xCF);
	}

#elif defined SSD1306_96_16
	lcd_command(lcd, SSD1306_SETCOMPINS);                    // 0xDA
	lcd_command(lcd, 0x2);   //ada x12
	lcd_command(lcd, SSD1306_SETCONTRAST);                   // 0x81
	if (vccstate == SSD1306_EXTERNALVCC)
	{
		lcd_command(lcd, 0x10);
	}
	else
	{
		lcd_command(lcd, 0xAF);
	}

#endif

	lcd_command(lcd, SSD1306_SETPRECHARGE);                  // 0xd9
	if (vccstate == SSD1306_EXTERNALVCC)
	{
		lcd_command(lcd, 0x22);
	}
	else
	{
		lcd_command(lcd, 0xF1);
	}
	lcd_command(lcd, SSD1306_SETVCOMDETECT);                 // 0xDB
	lcd_command(lcd, 0x40);
	lcd_command(lcd, SSD1306_DISPLAYALLON_RESUME);           // 0xA4
	lcd_command(lcd, SSD1306_NORMALDISPLAY);                 // 0xA6

	lcd_command(lcd, SSD1306_DEACTIVATE_SCROLL);

	lcd_command(lcd, SSD1306_DISPLAYON);//--turn on oled panel
}


void lcd_invertDisplay(LCD_Handler_t *lcd, uint8_t i) {
	if (i) {
		lcd_command(lcd, SSD1306_INVERTDISPLAY);
	}
	else {
		lcd_command(lcd, SSD1306_NORMALDISPLAY);
	}
}

void lcd_command(LCD_Handler_t *lcd, uint8_t c) {
	if (lcd->sid.pin != NC)
	{
		// SPI
		gpio_write(&lcd->cs, HIGH);
		gpio_write(&lcd->dc, LOW);
		gpio_write(&lcd->cs, LOW);
		lcd_fastSPIwrite(lcd, c);
		gpio_write(&lcd->cs, HIGH);
	}
	else
	{
		// I2C
		uint8_t data[2] = { 0x00/* Co = 0, D/C = 0*/, c };
		i2c_write(&lcd->i2c, lcd->_i2caddr, data, sizeof(data), 1);
	}
}

// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void lcd_startscrollright(LCD_Handler_t *lcd, uint8_t start, uint8_t stop) {
	lcd_command(lcd, SSD1306_RIGHT_HORIZONTAL_SCROLL);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, start);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, stop);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, 0XFF);
	lcd_command(lcd, SSD1306_ACTIVATE_SCROLL);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void lcd_startscrollleft(LCD_Handler_t *lcd, uint8_t start, uint8_t stop) {
	lcd_command(lcd, SSD1306_LEFT_HORIZONTAL_SCROLL);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, start);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, stop);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, 0XFF);
	lcd_command(lcd, SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void lcd_startscrolldiagright(LCD_Handler_t *lcd, uint8_t start, uint8_t stop) {
	lcd_command(lcd, SSD1306_SET_VERTICAL_SCROLL_AREA);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, SSD1306_LCDHEIGHT);
	lcd_command(lcd, SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, start);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, stop);
	lcd_command(lcd, 0X01);
	lcd_command(lcd, SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void lcd_startscrolldiagleft(LCD_Handler_t *lcd, uint8_t start, uint8_t stop) {
	lcd_command(lcd, SSD1306_SET_VERTICAL_SCROLL_AREA);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, SSD1306_LCDHEIGHT);
	lcd_command(lcd, SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, start);
	lcd_command(lcd, 0X00);
	lcd_command(lcd, stop);
	lcd_command(lcd, 0X01);
	lcd_command(lcd, SSD1306_ACTIVATE_SCROLL);
}

void lcd_stopscroll(LCD_Handler_t *lcd) {
	lcd_command(lcd, SSD1306_DEACTIVATE_SCROLL);
}

// Dim the display
// dim = true: display is dimmed
// dim = false: display is normal
void lcd_dim(LCD_Handler_t *lcd, bool_t dim) {
	uint8_t contrast;

	if (dim) {
		contrast = 0; // Dimmed display
	}
	else {
		if (lcd->_vccstate == SSD1306_EXTERNALVCC) {
			contrast = 0x9F;
		}
		else {
			contrast = 0xCF;
		}
	}
	// the range of contrast to too small to be really useful
	// it is useful to dim the display
	lcd_command(lcd, SSD1306_SETCONTRAST);
	lcd_command(lcd, contrast);
}

void lcd_display(LCD_Handler_t *lcd) {
	lcd_command(lcd, SSD1306_COLUMNADDR);
	lcd_command(lcd, 0);   // Column start address (0 = reset)
	lcd_command(lcd, SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	lcd_command(lcd, SSD1306_PAGEADDR);
	lcd_command(lcd, 0); // Page start address (0 = reset)
#if SSD1306_LCDHEIGHT == 64
	lcd_command(lcd, 7); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 32
	lcd_command(lcd, 3); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 16
	lcd_command(lcd, 1); // Page end address
#endif

	if (lcd->sid.pin != NC)
	{
		// SPI
		gpio_write(&lcd->cs, HIGH);
		gpio_write(&lcd->dc, HIGH);
		gpio_write(&lcd->cs, LOW);

		for (uint16_t i = 0; i < (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT / 8); i++) {
			lcd_fastSPIwrite(lcd, buffer[i]);
		}
		gpio_write(&lcd->cs, HIGH);
	}
	else
	{
		// save I2C bitrate
#ifdef TWBR
		uint8_t twbrbackup = TWBR;
		TWBR = 12; // upgrade to 400KHz!
#endif
		//Serial.println(TWBR, DEC);
		//Serial.println(TWSR & 0x3, DEC);

		// I2C
		uint8_t data[17] = { 0x40, };
		for (uint16_t i = 0; i < (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT / 8); i+=16) {
			// send a bunch of data in one xmission
			memcpy(&data[1], &buffer[i], 16);
			i2c_write(&lcd->i2c, lcd->_i2caddr, data, sizeof(data), 1);
		}
#ifdef TWBR
		TWBR = twbrbackup;
#endif
	}
}

// clear everything
void lcd_clearDisplay(LCD_Handler_t *lcd) {
	memset(buffer, 0, (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT / 8));
}

void lcd_fastSPIwrite(LCD_Handler_t *lcd, uint8_t d)
{
	if (lcd->hwSPI) {
		spi_master_write(&lcd->spi, d);
	}
	else {
		for (uint8_t bit = 0x80; bit; bit >>= 1) {
			gpio_write(&lcd->sclk, LOW);
			if (d & bit) gpio_write(&lcd->sid, HIGH);
			else        gpio_write(&lcd->sid, LOW);
			gpio_write(&lcd->sclk, HIGH);
		}
	}
}

void lcd_drawFastHLine(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, uint16_t color) {
	bool_t bSwap = false;
	switch (lcd->rotation) {
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x
		bSwap = true;
		lcd_swap(x, y);
		x = lcd->_width - x - 1;
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = lcd->_width - x - 1;
		y = lcd->_height - y - 1;
		x -= (w - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
		bSwap = true;
		lcd_swap(x, y);
		y = lcd->_height - y - 1;
		y -= (w - 1);
		break;
	}

	if (bSwap) {
		lcd_drawFastVLineInternal(lcd, x, y, w, color);
	}
	else {
		lcd_drawFastHLineInternal(lcd, x, y, w, color);
	}
}

void lcd_drawFastHLineInternal(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, uint16_t color) {
	// Do bounds/limit checks
	if (y < 0 || y >= lcd->_height) { return; }

	// make sure we don't try to draw below 0
	if (x < 0) {
		w += x;
		x = 0;
	}

	// make sure we don't go off the edge of the display
	if ((x + w) > lcd->_width) {
		w = (lcd->_width - x);
	}

	// if our width is now negative, punt
	if (w <= 0) { return; }

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = 1 << (y & 7);

	switch (color)
	{
	case WHITE:         while (w--) { *pBuf++ |= mask; }; break;
	case BLACK: mask = ~mask;   while (w--) { *pBuf++ &= mask; }; break;
	case INVERSE:         while (w--) { *pBuf++ ^= mask; }; break;
	}
}

void lcd_drawFastVLine(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t h, uint16_t color) {
	bool_t bSwap = false;
	switch (lcd->rotation) {
	case 0:
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
		bSwap = true;
		lcd_swap(x, y);
		x = lcd->_width - x - 1;
		x -= (h - 1);
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = lcd->_width - x - 1;
		y = lcd->_height - y - 1;
		y -= (h - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y
		bSwap = true;
		lcd_swap(x, y);
		y = lcd->_height - y - 1;
		break;
	}

	if (bSwap) {
		lcd_drawFastHLineInternal(lcd, x, y, h, color);
	}
	else {
		lcd_drawFastVLineInternal(lcd, x, y, h, color);
	}
}


void lcd_drawFastVLineInternal(LCD_Handler_t *lcd, int16_t x, int16_t __y, int16_t __h, uint16_t color) {

	// do nothing if we're off the left or right side of the screen
	if (x < 0 || x >= lcd->_width) { return; }

	// make sure we don't try to draw below 0
	if (__y < 0) {
		// __y is negative, this will subtract enough from __h to account for __y being 0
		__h += __y;
		__y = 0;

	}

	// make sure we don't go past the height of the display
	if ((__y + __h) > lcd->_height) {
		__h = (lcd->_height - __y);
	}

	// if our height is now negative, punt
	if (__h <= 0) {
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t y = __y;
	register uint8_t h = __h;


	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (y & 7);
	if (mod) {
		// mask off the high n bits we want to set
		mod = 8 - mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if (h < mod) {
			mask &= (0XFF >> (mod - h));
		}

		switch (color)
		{
		case WHITE:   *pBuf |= mask;  break;
		case BLACK:   *pBuf &= ~mask;  break;
		case INVERSE: *pBuf ^= mask;  break;
		}

		// fast exit if we're done here!
		if (h < mod) { return; }

		h -= mod;

		pBuf += SSD1306_LCDWIDTH;
	}

	// write solid bytes while we can - effectively doing 8 rows at a time
	if (h >= 8) {
		if (color == INVERSE) {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
			do {
				*pBuf = ~(*pBuf);

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		}
		else {
			// store a local value to work with
			register uint8_t val = (color == WHITE) ? 255 : 0;

			do {
				// write our value in
				*pBuf = val;

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while (h >= 8);
		}
	}

	// now do the final partial byte, if necessary
	if (h) {
		mod = h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
		register uint8_t mask = postmask[mod];
		switch (color)
		{
		case WHITE:   *pBuf |= mask;  break;
		case BLACK:   *pBuf &= ~mask;  break;
		case INVERSE: *pBuf ^= mask;  break;
		}
	}
}
