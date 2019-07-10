#ifndef _ADAFRUIT_GFX_H
#define _ADAFRUIT_GFX_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "gpio_api.h"
#include "spi_api.h"
#include "i2c_api.h"
#ifndef ADAFRUIT_SSD1306
#include "adafruit_st7735.h"
#else
#include "adafruit_ssd1306.h"
#endif

extern uint16_t lcd_init_height;
extern uint16_t lcd_init_width;

void lcd_adafruit_gfx(LCD_Handler_t *lcd, int16_t w, int16_t h);
void lcd_writeLine(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
	uint16_t color);
void lcd_startWrite(LCD_Handler_t *lcd);
void lcd_writePixel(LCD_Handler_t *lcd, int16_t x, int16_t y, uint16_t color);
void lcd_writeFastVLine(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t h, uint16_t color);
void lcd_writeFastHLine(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t w, uint16_t color);
void lcd_writeFillRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color);
void lcd_endWrite(LCD_Handler_t *lcd);
void lcd_drawFastVLine_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t h, uint16_t color);
void lcd_drawFastHLine_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t w, uint16_t color);
void lcd_fillRect_(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color);
void lcd_fillScreen_(LCD_Handler_t *lcd, uint16_t color);
void lcd_drawLine_(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
	uint16_t color);
void lcd_drawCircle(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint16_t color);
void lcd_drawCircleHelper(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t r, uint8_t cornername, uint16_t color);
void lcd_fillCircle(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint16_t color);
void lcd_fillCircleHelper(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint8_t cornername, int16_t delta, uint16_t color);
void lcd_drawRect_(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color);
void lcd_drawRoundRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w,
	int16_t h, int16_t r, uint16_t color);
void lcd_fillRoundRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w,
	int16_t h, int16_t r, uint16_t color);
void lcd_drawTriangle(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void lcd_fillTriangle(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void lcd_drawBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
void lcd_drawBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h,
	uint16_t color, uint16_t bg);
void lcd_drawBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
void lcd_drawBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg);
void lcd_drawXBitmap(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
void lcd_drawGrayscaleBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h);
void lcd_drawGrayscaleBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h);
void lcd_drawGrayscaleBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], const uint8_t mask[],
	int16_t w, int16_t h);
void lcd_drawGrayscaleBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
void lcd_drawRGBBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint16_t bitmap[], int16_t w, int16_t h);
void lcd_drawRGBBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint16_t *bitmap, int16_t w, int16_t h);
void lcd_drawRGBBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint16_t bitmap[], const uint8_t mask[],
	int16_t w, int16_t h);
void lcd_drawRGBBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
void lcd_drawFont_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg);
void lcd_drawChar(LCD_Handler_t *lcd, int16_t x, int16_t y, const char *utf8,
	uint16_t color, uint16_t bg, uint8_t size);
size_t lcd_write(LCD_Handler_t *lcd, uint8_t c);
void lcd_drawString_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const char *string, uint16_t color, uint16_t bg);
void lcd_setCursor(LCD_Handler_t *lcd, int16_t x, int16_t y);
int16_t lcd_getCursorX(LCD_Handler_t *lcd);
int16_t lcd_getCursorY(LCD_Handler_t *lcd);
void lcd_setTextSize(LCD_Handler_t *lcd, uint8_t s);
void lcd_setTextColor(LCD_Handler_t *lcd, uint16_t c);
void lcd_setTextColor2(LCD_Handler_t *lcd, uint16_t c, uint16_t b);
void lcd_setTextWrap(LCD_Handler_t *lcd, bool w);
uint8_t lcd_getRotation(LCD_Handler_t *lcd);
void lcd_setRotation(LCD_Handler_t *lcd, uint8_t x);
void lcd_setFont(LCD_Handler_t *lcd, const GFXfont *f);
void lcd_charBounds(LCD_Handler_t *lcd, char c, int16_t *x, int16_t *y,
	int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
void lcd_getTextBounds(LCD_Handler_t *lcd, const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
void lcd_getTextBounds2(LCD_Handler_t *lcd, const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
void lcd_getTextBounds3(LCD_Handler_t *lcd, const char *str,
	int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
int16_t lcd_width(LCD_Handler_t *lcd);
int16_t lcd_height(LCD_Handler_t *lcd);

#endif // _ADAFRUIT_GFX_H
