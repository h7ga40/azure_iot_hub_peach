/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
	*/

#include <string.h>
#include <math.h>
#include <kernel.h>
#include "adafruit_gfx.h"
#include "gfxfont.h"
#include "draw_font.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#endif

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

/**************************************************************************/
/*!
	@brief    Instatiate a GFX context for graphics! Can only be done by a superclass
	@param    w   Display width, in pixels
	@param    h   Display height, in pixels
*/
/**************************************************************************/
void lcd_adafruit_gfx(LCD_Handler_t *lcd, int16_t w, int16_t h)
{
	lcd->_width = w;
	lcd->_height = h;
	lcd->rotation = 0;
	lcd->cursor_y = lcd->cursor_x = 0;
	lcd->textsize = 1;
	lcd->textcolor = lcd->textbgcolor = 0xFFFF;
	lcd->wrap = true;
	lcd->gfxFont = NULL;
}

/**************************************************************************/
/*!
	@brief    Write a line.  Bresenham's algorithm - thx wikpedia
	@param    x0  Start point x coordinate
	@param    y0  Start point y coordinate
	@param    x1  End point x coordinate
	@param    y1  End point y coordinate
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_writeLine(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
	uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			lcd_writePixel(lcd, y0, x0, color);
		}
		else {
			lcd_writePixel(lcd, x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

/**************************************************************************/
/*!
	@brief    Start a display-writing routine, overwrite in subclasses.
*/
/**************************************************************************/
void lcd_startWrite(LCD_Handler_t *lcd)
{
}

/**************************************************************************/
/*!
	@brief    Write a pixel, overwrite in subclasses if startWrite is defined!
	@param   x   x coordinate
	@param   y   y coordinate
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_writePixel(LCD_Handler_t *lcd, int16_t x, int16_t y, uint16_t color)
{
	lcd_drawPixel(lcd, x, y, color);
}

/**************************************************************************/
/*!
	@brief    Write a perfectly vertical line, overwrite in subclasses if startWrite is defined!
	@param    x   Top-most x coordinate
	@param    y   Top-most y coordinate
	@param    h   Height in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_writeFastVLine(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t h, uint16_t color)
{
// Overwrite in subclasses if startWrite is defined!
// Can be just lcd_writeLine(lcd, x, y, x, y+h-1, color);
// or lcd_writeFillRect(lcd, x, y, 1, h, color);
	lcd_drawFastVLine(lcd, x, y, h, color);
}

/**************************************************************************/
/*!
	@brief    Write a perfectly horizontal line, overwrite in subclasses if startWrite is defined!
	@param    x   Left-most x coordinate
	@param    y   Left-most y coordinate
	@param    w   Width in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_writeFastHLine(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t w, uint16_t color)
{
// Overwrite in subclasses if startWrite is defined!
// Example: lcd_writeLine(lcd, x, y, x+w-1, y, color);
// or lcd_writeFillRect(lcd, x, y, w, 1, color);
	lcd_drawFastHLine(lcd, x, y, w, color);
}

/**************************************************************************/
/*!
	@brief    Write a rectangle completely with one color, overwrite in subclasses if startWrite is defined!
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    w   Width in pixels
	@param    h   Height in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_writeFillRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color)
{
// Overwrite in subclasses if desired!
	lcd_fillRect_(lcd, x, y, w, h, color);
}

/**************************************************************************/
/*!
	@brief    End a display-writing routine, overwrite in subclasses if startWrite is defined!
*/
/**************************************************************************/
void lcd_endWrite(LCD_Handler_t *lcd)
{
}

/**************************************************************************/
/*!
	@brief    Draw a perfectly vertical line (this is often optimized in a subclass!)
	@param    x   Top-most x coordinate
	@param    y   Top-most y coordinate
	@param    h   Height in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_drawFastVLine_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t h, uint16_t color)
{
	lcd_startWrite(lcd);
	lcd_writeLine(lcd, x, y, x, y + h - 1, color);
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief    Draw a perfectly horizontal line (this is often optimized in a subclass!)
	@param    x   Left-most x coordinate
	@param    y   Left-most y coordinate
	@param    w   Width in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_drawFastHLine_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	int16_t w, uint16_t color)
{
	lcd_startWrite(lcd);
	lcd_writeLine(lcd, x, y, x + w - 1, y, color);
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief    Fill a rectangle completely with one color. Update in subclasses if desired!
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    w   Width in pixels
	@param    h   Height in pixels
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_fillRect_(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color)
{
	lcd_startWrite(lcd);
	for (int16_t i = x; i < x + w; i++) {
		lcd_writeFastVLine(lcd, i, y, h, color);
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief    Fill the screen completely with one color. Update in subclasses if desired!
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_fillScreen_(LCD_Handler_t *lcd, uint16_t color)
{
	lcd_fillRect_(lcd, 0, 0, lcd->_width, lcd->_height, color);
}

/**************************************************************************/
/*!
	@brief    Draw a line
	@param    x0  Start point x coordinate
	@param    y0  Start point y coordinate
	@param    x1  End point x coordinate
	@param    y1  End point y coordinate
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawLine_(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
	uint16_t color)
{
// Update in subclasses if desired!
	if (x0 == x1) {
		if (y0 > y1) _swap_int16_t(y0, y1);
		lcd_drawFastVLine(lcd, x0, y0, y1 - y0 + 1, color);
	}
	else if (y0 == y1) {
		if (x0 > x1) _swap_int16_t(x0, x1);
		lcd_drawFastHLine(lcd, x0, y0, x1 - x0 + 1, color);
	}
	else {
		lcd_startWrite(lcd);
		lcd_writeLine(lcd, x0, y0, x1, y1, color);
		lcd_endWrite(lcd);
	}
}

/**************************************************************************/
/*!
	@brief    Draw a circle outline
	@param    x0   Center-point x coordinate
	@param    y0   Center-point y coordinate
	@param    r   Radius of circle
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawCircle(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	lcd_startWrite(lcd);
	lcd_writePixel(lcd, x0, y0 + r, color);
	lcd_writePixel(lcd, x0, y0 - r, color);
	lcd_writePixel(lcd, x0 + r, y0, color);
	lcd_writePixel(lcd, x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		lcd_writePixel(lcd, x0 + x, y0 + y, color);
		lcd_writePixel(lcd, x0 - x, y0 + y, color);
		lcd_writePixel(lcd, x0 + x, y0 - y, color);
		lcd_writePixel(lcd, x0 - x, y0 - y, color);
		lcd_writePixel(lcd, x0 + y, y0 + x, color);
		lcd_writePixel(lcd, x0 - y, y0 + x, color);
		lcd_writePixel(lcd, x0 + y, y0 - x, color);
		lcd_writePixel(lcd, x0 - y, y0 - x, color);
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief    Quarter-circle drawer, used to do circles and roundrects
	@param    x0   Center-point x coordinate
	@param    y0   Center-point y coordinate
	@param    r   Radius of circle
	@param    cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawCircleHelper(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t r, uint8_t cornername, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			lcd_writePixel(lcd, x0 + x, y0 + y, color);
			lcd_writePixel(lcd, x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			lcd_writePixel(lcd, x0 + x, y0 - y, color);
			lcd_writePixel(lcd, x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			lcd_writePixel(lcd, x0 - y, y0 + x, color);
			lcd_writePixel(lcd, x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			lcd_writePixel(lcd, x0 - y, y0 - x, color);
			lcd_writePixel(lcd, x0 - x, y0 - y, color);
		}
	}
}

/**************************************************************************/
/*!
	@brief    Draw a circle with filled color
	@param    x0   Center-point x coordinate
	@param    y0   Center-point y coordinate
	@param    r   Radius of circle
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_fillCircle(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint16_t color)
{
	lcd_startWrite(lcd);
	lcd_writeFastVLine(lcd, x0, y0 - r, 2 * r + 1, color);
	lcd_fillCircleHelper(lcd, x0, y0, r, 3, 0, color);
	lcd_endWrite(lcd);
}


/**************************************************************************/
/*!
	@brief    Quarter-circle drawer with fill, used to do circles and roundrects
	@param    x0   Center-point x coordinate
	@param    y0   Center-point y coordinate
	@param    r   Radius of circle
	@param    cornername  Mask bit #1 or bit #2 to indicate which quarters of the circle we're doing
	@param    delta  Offset from center-point, used for round-rects
	@param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void lcd_fillCircleHelper(LCD_Handler_t *lcd, int16_t x0, int16_t y0, int16_t r,
	uint8_t cornername, int16_t delta, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			lcd_writeFastVLine(lcd, x0 + x, y0 - y, 2 * y + 1 + delta, color);
			lcd_writeFastVLine(lcd, x0 + y, y0 - x, 2 * x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			lcd_writeFastVLine(lcd, x0 - x, y0 - y, 2 * y + 1 + delta, color);
			lcd_writeFastVLine(lcd, x0 - y, y0 - x, 2 * x + 1 + delta, color);
		}
	}
}

/**************************************************************************/
/*!
	@brief   Draw a rectangle with no fill color
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    w   Width in pixels
	@param    h   Height in pixels
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawRect_(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color)
{
	lcd_startWrite(lcd);
	lcd_writeFastHLine(lcd, x, y, w, color);
	lcd_writeFastHLine(lcd, x, y + h - 1, w, color);
	lcd_writeFastVLine(lcd, x, y, h, color);
	lcd_writeFastVLine(lcd, x + w - 1, y, h, color);
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a rounded rectangle with no fill color
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    w   Width in pixels
	@param    h   Height in pixels
	@param    r   Radius of corner rounding
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawRoundRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w,
	int16_t h, int16_t r, uint16_t color)
{
// smarter version
	lcd_startWrite(lcd);
	lcd_writeFastHLine(lcd, x + r, y, w - 2 * r, color); // Top
	lcd_writeFastHLine(lcd, x + r, y + h - 1, w - 2 * r, color); // Bottom
	lcd_writeFastVLine(lcd, x, y + r, h - 2 * r, color); // Left
	lcd_writeFastVLine(lcd, x + w - 1, y + r, h - 2 * r, color); // Right
	// draw four corners
	lcd_drawCircleHelper(lcd, x + r, y + r, r, 1, color);
	lcd_drawCircleHelper(lcd, x + w - r - 1, y + r, r, 2, color);
	lcd_drawCircleHelper(lcd, x + w - r - 1, y + h - r - 1, r, 4, color);
	lcd_drawCircleHelper(lcd, x + r, y + h - r - 1, r, 8, color);
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a rounded rectangle with fill color
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    w   Width in pixels
	@param    h   Height in pixels
	@param    r   Radius of corner rounding
	@param    color 16-bit 5-6-5 Color to draw/fill with
*/
/**************************************************************************/
void lcd_fillRoundRect(LCD_Handler_t *lcd, int16_t x, int16_t y, int16_t w,
	int16_t h, int16_t r, uint16_t color)
{
// smarter version
	lcd_startWrite(lcd);
	lcd_writeFillRect(lcd, x + r, y, w - 2 * r, h, color);

	// draw four corners
	lcd_fillCircleHelper(lcd, x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	lcd_fillCircleHelper(lcd, x + r, y + r, r, 2, h - 2 * r - 1, color);
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a triangle with no fill color
	@param    x0  Vertex #0 x coordinate
	@param    y0  Vertex #0 y coordinate
	@param    x1  Vertex #1 x coordinate
	@param    y1  Vertex #1 y coordinate
	@param    x2  Vertex #2 x coordinate
	@param    y2  Vertex #2 y coordinate
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawTriangle(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	lcd_drawLine_(lcd, x0, y0, x1, y1, color);
	lcd_drawLine_(lcd, x1, y1, x2, y2, color);
	lcd_drawLine_(lcd, x2, y2, x0, y0, color);
}

/**************************************************************************/
/*!
	@brief     Draw a triangle with color-fill
	@param    x0  Vertex #0 x coordinate
	@param    y0  Vertex #0 y coordinate
	@param    x1  Vertex #1 x coordinate
	@param    y1  Vertex #1 y coordinate
	@param    x2  Vertex #2 x coordinate
	@param    y2  Vertex #2 y coordinate
	@param    color 16-bit 5-6-5 Color to fill/draw with
*/
/**************************************************************************/
void lcd_fillTriangle(LCD_Handler_t *lcd, int16_t x0, int16_t y0,
	int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}
	if (y1 > y2) {
		_swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
	}
	if (y0 > y1) {
		_swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
	}

	lcd_startWrite(lcd);
	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a)      a = x1;
		else if (x1 > b) b = x1;
		if (x2 < a)      a = x2;
		else if (x2 > b) b = x2;
		lcd_writeFastHLine(lcd, a, y0, b - a + 1, color);
		lcd_endWrite(lcd);
		return;
	}

	int16_t
		dx01 = x1 - x0,
		dy01 = y1 - y0,
		dx02 = x2 - x0,
		dy02 = y2 - y0,
		dx12 = x2 - x1,
		dy12 = y2 - y1;
	int32_t
		sa = 0,
		sb = 0;

		// For upper part of triangle, find scanline crossings for segments
		// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
		// is included here (and second loop will be skipped, avoiding a /0
		// error there), otherwise scanline y1 is skipped here and handled
		// in the second loop...which also avoids a /0 error here if y0=y1
		// (flat-topped triangle).
	if (y1 == y2) last = y1;   // Include y1 scanline
	else         last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) _swap_int16_t(a, b);
		lcd_writeFastHLine(lcd, a, y, b - a + 1, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		*/
		if (a > b) _swap_int16_t(a, b);
		lcd_writeFastHLine(lcd, a, y, b - a + 1, color);
	}
	lcd_endWrite(lcd);
}

// BITMAP / XBITMAP / GRAYSCALE / RGB BITMAP FUNCTIONS ---------------------

/**************************************************************************/
/*!
	@brief      Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
			if (byte & 0x80) lcd_writePixel(lcd, x + i, y, color);
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief      Draw a PROGMEM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw pixels with
	@param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void lcd_drawBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h,
	uint16_t color, uint16_t bg)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
			lcd_writePixel(lcd, x + i, y, (byte & 0x80) ? color : bg);
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief      Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground color (unset bits are transparent).
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw with
*/
/**************************************************************************/
void lcd_drawBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = bitmap[j * byteWidth + i / 8];
			if (byte & 0x80) lcd_writePixel(lcd, x + i, y, color);
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief      Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw pixels with
	@param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void lcd_drawBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = bitmap[j * byteWidth + i / 8];
			lcd_writePixel(lcd, x + i, y, (byte & 0x80) ? color : bg);
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief      Draw PROGMEM-resident XBitMap Files (*.xbm), exported from GIMP.
	Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
	C Array can be directly used with this function.
	There is no RAM-resident version of this function; if generating bitmaps
	in RAM, use the format defined by drawBitmap() and call that instead.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw pixels with
*/
/**************************************************************************/
void lcd_drawXBitmap(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
	int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte >>= 1;
			else      byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
			// Nearly identical to drawBitmap(), only the bit order
			// is reversed here (left-to-right = LSB to MSB):
			if (byte & 0x01) lcd_writePixel(lcd, x + i, y, color);
		}
	}
	lcd_endWrite(lcd);
}


/**************************************************************************/
/*!
	@brief   Draw a PROGMEM-resident 8-bit image (grayscale) at the specified (x,y) pos.
	Specifically for 8-bit display devices such as IS31FL3731; no color reduction/expansion is performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with grayscale bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawGrayscaleBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], int16_t w, int16_t h)
{
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			lcd_writePixel(lcd, x + i, y, (uint8_t)pgm_read_byte(&bitmap[j * w + i]));
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a RAM-resident 8-bit image (grayscale) at the specified (x,y) pos.
	Specifically for 8-bit display devices such as IS31FL3731; no color reduction/expansion is performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with grayscale bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawGrayscaleBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h)
{
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			lcd_writePixel(lcd, x + i, y, bitmap[j * w + i]);
		}
	}
	lcd_endWrite(lcd);
}


/**************************************************************************/
/*!
	@brief   Draw a PROGMEM-resident 8-bit image (grayscale) with a 1-bit mask
	(set bits = opaque, unset bits = clear) at the specified (x,y) position.
	BOTH buffers (grayscale and mask) must be PROGMEM-resident.
	Specifically for 8-bit display devices such as IS31FL3731; no color reduction/expansion is performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with grayscale bitmap
	@param    mask  byte array with mask bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawGrayscaleBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint8_t bitmap[], const uint8_t mask[],
	int16_t w, int16_t h)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = pgm_read_byte(&mask[j * bw + i / 8]);
			if (byte & 0x80) {
				lcd_writePixel(lcd, x + i, y, (uint8_t)pgm_read_byte(&bitmap[j * w + i]));
			}
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a RAM-resident 8-bit image (grayscale) with a 1-bit mask
	(set bits = opaque, unset bits = clear) at the specified (x,y) position.
	BOTH buffers (grayscale and mask) must be RAM-residentt, no mix-and-match
	Specifically for 8-bit display devices such as IS31FL3731; no color reduction/expansion is performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with grayscale bitmap
	@param    mask  byte array with mask bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawGrayscaleBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = mask[j * bw + i / 8];
			if (byte & 0x80) {
				lcd_writePixel(lcd, x + i, y, bitmap[j * w + i]);
			}
		}
	}
	lcd_endWrite(lcd);
}


/**************************************************************************/
/*!
	@brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
	For 16-bit display devices; no color reduction performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with 16-bit color bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawRGBBitmap1(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint16_t bitmap[], int16_t w, int16_t h)
{
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			lcd_writePixel(lcd, x + i, y, pgm_read_word(&bitmap[j * w + i]));
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a RAM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
	For 16-bit display devices; no color reduction performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with 16-bit color bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawRGBBitmap2(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint16_t *bitmap, int16_t w, int16_t h)
{
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			lcd_writePixel(lcd, x + i, y, bitmap[j * w + i]);
		}
	}
	lcd_endWrite(lcd);
}


/**************************************************************************/
/*!
	@brief   Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask (set bits = opaque, unset bits = clear) at the specified (x,y) position. BOTH buffers (color and mask) must be PROGMEM-resident. For 16-bit display devices; no color reduction performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with 16-bit color bitmap
	@param    mask  byte array with monochrome mask bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawRGBBitmap3(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const uint16_t bitmap[], const uint8_t mask[],
	int16_t w, int16_t h)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = pgm_read_byte(&mask[j * bw + i / 8]);
			if (byte & 0x80) {
				lcd_writePixel(lcd, x + i, y, pgm_read_word(&bitmap[j * w + i]));
			}
		}
	}
	lcd_endWrite(lcd);
}

/**************************************************************************/
/*!
	@brief   Draw a RAM-resident 16-bit image (RGB 5/6/5) with a 1-bit mask (set bits = opaque, unset bits = clear) at the specified (x,y) position. BOTH buffers (color and mask) must be RAM-resident. For 16-bit display devices; no color reduction performed.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with 16-bit color bitmap
	@param    mask  byte array with monochrome mask bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Height of bitmap in pixels
*/
/**************************************************************************/
void lcd_drawRGBBitmap4(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h)
{
	int16_t bw = (w + 7) / 8; // Bitmask scanline pad = whole byte
	uint8_t byte = 0;
	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++) {
			if (i & 7) byte <<= 1;
			else      byte = mask[j * bw + i / 8];
			if (byte & 0x80) {
				lcd_writePixel(lcd, x + i, y, bitmap[j * w + i]);
			}
		}
	}
	lcd_endWrite(lcd);
}

// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

/**************************************************************************/
/*!
	@brief      Draw a RAM-resident 1-bit image at the specified (x,y) position, using the specified foreground (for set bits) and background (unset bits) colors.
	@param    x   Top left corner x coordinate
	@param    y   Top left corner y coordinate
	@param    bitmap  byte array with monochrome bitmap
	@param    w   Width of bitmap in pixels
	@param    h   Hieght of bitmap in pixels
	@param    color 16-bit 5-6-5 Color to draw pixels with
	@param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void lcd_drawFont_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg)
{
	int16_t b = 0;
	uint8_t byte = 0;

	lcd_startWrite(lcd);
	for (int16_t j = 0; j < h; j++, y++) {
		for (int16_t i = 0; i < w; i++, b++) {
			if (b & 7) byte <<= 1;
			else      byte = *bitmap++;
			lcd_writePixel(lcd, x + i, y, (byte & 0x80) ? color : bg);
		}
	}
	lcd_endWrite(lcd);
}

// Draw a character
/**************************************************************************/
/*!
	@brief   Draw a single character
	@param    x   Bottom left corner x coordinate
	@param    y   Bottom left corner y coordinate
	@param    c   The 8-bit font-indexed character (likely ascii)
	@param    color 16-bit 5-6-5 Color to draw chraracter with
	@param    bg 16-bit 5-6-5 Color to fill background with (if same as color, no background)
	@param    size  Font magnification level, 1 is 'original' size
*/
/**************************************************************************/
void lcd_drawChar(LCD_Handler_t *lcd, int16_t x, int16_t y, const char *utf8,
	uint16_t color, uint16_t bg, uint8_t size)
{
	if (!lcd->gfxFont) { // 'Classic' built-in font
		uint8_t bitmap_data[FONT_WIDTH * FONT_HEIGHT / 8];
		uint32_t use_chars, for_3B_hankaku_code;
		const uint8_t *code = (const uint8_t *)utf8;

		if ((x >= lcd->_width) || // Clip right
			(y >= lcd->_height) || // Clip bottom
			((x + FONT_WIDTH * size - 1) < 0) || // Clip left
			((y + FONT_HEIGHT * size - 1) < 0))   // Clip top
			return;

		memset(bitmap_data, 0x0, FONT_WIDTH * FONT_HEIGHT / 8);

		get_bitmap_font(code, bitmap_data, &use_chars);
		if (use_chars == 0)
			return;

		//3バイトコード半角文字用
		if (use_chars == 3) {
			for_3B_hankaku_code = 0;
			for_3B_hankaku_code = ((code[0] << 16) |
				(code[1] << 8) |
				(code[2]));
		}

		lcd_startWrite(lcd);
		//1バイトコード半角文字
		if (use_chars == 1) {
			lcd_drawFont_(lcd, x, y, bitmap_data, FONT_HALF_WIDTH, FONT_HEIGHT, color, bg);
		}
		//3バイトコード半角文字
		else if ((use_chars == 3) &&
			(((0xEFBDA1 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBDBF)) ||
			((0xEFBE80 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBE9F)))) {
			//3バイトコード半角文字
			lcd_drawFont_(lcd, x, y, bitmap_data, FONT_HALF_WIDTH, FONT_HEIGHT, color, bg);
		}
		//全角文字
		else {
			lcd_drawFont_(lcd, x, y, bitmap_data, FONT_WIDTH, FONT_HEIGHT, color, bg);
		}
		if (bg != color) { // If opaque, draw vertical line for last column
			if (size == 1) lcd_writeFastVLine(lcd, x + FONT_WIDTH, y, FONT_HEIGHT, bg);
			else          lcd_writeFillRect(lcd, x + FONT_WIDTH * size, y, size, FONT_HEIGHT*size, bg);
		}
		lcd_endWrite(lcd);

	}
	else { // Custom font
		unsigned char c = *utf8;

		// Character is assumed previously filtered by write() to eliminate
		// newlines, returns, non-printable characters, etc.  Calling
		// drawChar() directly with 'bad' characters of font may cause mayhem!

		c -= (uint8_t)pgm_read_byte(&lcd->gfxFont->first);
		GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&lcd->gfxFont->glyph))[c]);
		uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&lcd->gfxFont->bitmap);

		uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
		uint8_t  w = pgm_read_byte(&glyph->width),
			h = pgm_read_byte(&glyph->height);
		int8_t   xo = pgm_read_byte(&glyph->xOffset),
			yo = pgm_read_byte(&glyph->yOffset);
		uint8_t  xx, yy, bits = 0, bit = 0;
		int16_t  xo16 = 0, yo16 = 0;

		if (size > 1) {
			xo16 = xo;
			yo16 = yo;
		}

		// Todo: Add character clipping here

		// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
		// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
		// has typically been used with the 'classic' font to overwrite old
		// screen contents with new data.  This ONLY works because the
		// characters are a uniform size; it's not a sensible thing to do with
		// proportionally-spaced fonts with glyphs of varying sizes (and that
		// may overlap).  To replace previously-drawn text when using a custom
		// font, use the getTextBounds() function to determine the smallest
		// rectangle encompassing a string, erase the area with fillRect(),
		// then draw new text.  This WILL infortunately 'blink' the text, but
		// is unavoidable.  Drawing 'background' pixels will NOT fix this,
		// only creates a new set of problems.  Have an idea to work around
		// this (a canvas object type for MCUs that can afford the RAM and
		// displays supporting setAddrWindow() and pushColors()), but haven't
		// implemented this yet.

		lcd_startWrite(lcd);
		for (yy = 0; yy < h; yy++) {
			for (xx = 0; xx < w; xx++) {
				if (!(bit++ & 7)) {
					bits = pgm_read_byte(&bitmap[bo++]);
				}
				if (bits & 0x80) {
					if (size == 1) {
						lcd_writePixel(lcd, x + xo + xx, y + yo + yy, color);
					}
					else {
						lcd_writeFillRect(lcd, x + (xo16 + xx)*size, y + (yo16 + yy)*size,
							size, size, color);
					}
				}
				bits <<= 1;
			}
		}
		lcd_endWrite(lcd);

	} // End classic vs custom font
}
/**************************************************************************/
/*!
	@brief  Print one byte/character of data, used to support print()
	@param  c  The 8-bit ascii character to write
*/
/**************************************************************************/
size_t lcd_write(LCD_Handler_t *lcd, uint8_t c)
{
	if (!lcd->gfxFont) { // 'Classic' built-in font

		if (c == '\n') {                    // Newline?
			lcd->cursor_x = 0;                     // Reset x to zero,
			lcd->cursor_y += lcd->textsize * 8;          // advance y one line
		}
		else if (c != '\r') {             // Ignore carriage returns
			if (lcd->wrap && ((lcd->cursor_x + lcd->textsize * 6) > lcd->_width)) { // Off right?
				lcd->cursor_x = 0;                 // Reset x to zero,
				lcd->cursor_y += lcd->textsize * 8;      // advance y one line
			}
			lcd_drawChar(lcd, lcd->cursor_x, lcd->cursor_y, (char *)&c, lcd->textcolor, lcd->textbgcolor, lcd->textsize);
			lcd->cursor_x += lcd->textsize * 6;          // Advance x one char
		}

	}
	else { // Custom font

		if (c == '\n') {
			lcd->cursor_x = 0;
			lcd->cursor_y += (int16_t)lcd->textsize *
				(uint8_t)pgm_read_byte(&lcd->gfxFont->yAdvance);
		}
		else if (c != '\r') {
			uint8_t first = pgm_read_byte(&lcd->gfxFont->first);
			if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&lcd->gfxFont->last))) {
				GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
					&lcd->gfxFont->glyph))[c - first]);
				uint8_t   w = pgm_read_byte(&glyph->width),
					h = pgm_read_byte(&glyph->height);
				if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
					int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
					if (lcd->wrap && ((lcd->cursor_x + lcd->textsize * (xo + w)) > lcd->_width)) {
						lcd->cursor_x = 0;
						lcd->cursor_y += (int16_t)lcd->textsize *
							(uint8_t)pgm_read_byte(&lcd->gfxFont->yAdvance);
					}
					lcd_drawChar(lcd, lcd->cursor_x, lcd->cursor_y, (char *)&c, lcd->textcolor, lcd->textbgcolor, lcd->textsize);
				}
				lcd->cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)lcd->textsize;
			}
		}

	}
	return 1;
}

void lcd_drawString_(LCD_Handler_t *lcd, int16_t x, int16_t y,
	const char *string, uint16_t color, uint16_t bg)
{
	uint32_t current_top, use_chars, for_3B_hankaku_code;
	uint8_t bitmap_data[FONT_WIDTH * FONT_HEIGHT / 8], ctrl_code;
	int local_x, local_y, len = strlen(string);
	const uint8_t *code = (const uint8_t *)string;

	local_x = x;
	local_y = y;

	current_top = 0;
	while (current_top < len) {
		memset(bitmap_data, 0x0, FONT_WIDTH * FONT_HEIGHT / 8);
		ctrl_code = code[current_top];
		get_bitmap_font(&code[current_top], bitmap_data, &use_chars);
		if (use_chars == 0)
			return;

		//3バイトコード半角文字用
		if (use_chars == 3) {
			for_3B_hankaku_code = 0;
			for_3B_hankaku_code = ((code[current_top] << 16) |
				(code[current_top + 1] << 8) |
				(code[current_top + 2]));
		}

		current_top += use_chars;

		//1バイトコード半角文字
		if (use_chars == 1) {
			if (ctrl_code == 0x0D) { // CR
				local_x = X_LINE_TO_PIX(this, 0);
				continue;
			}
			if (ctrl_code == 0x0A) { // LF
				local_y = local_y + FONT_HEIGHT;
				continue;
			}

			if (local_x + FONT_HALF_WIDTH > lcd->_width) {
				local_x = X_LINE_HALF_TO_PIX(this, 0);
				local_y = local_y + FONT_HEIGHT;
			}
			lcd_drawFont_(lcd, local_x, local_y, bitmap_data, FONT_HALF_WIDTH, FONT_HEIGHT, color, bg);
			local_x += FONT_HALF_WIDTH;
			continue;
		}

		//3バイトコード半角文字
		if (use_chars == 3) {
			if (((0xEFBDA1 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBDBF)) ||
				((0xEFBE80 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBE9F))) {
				//3バイトコード半角文字
				if (local_x + FONT_HALF_WIDTH > lcd->_width) {
					local_x = X_LINE_HALF_TO_PIX(this, 0);
					local_y = local_y + FONT_HEIGHT;
				}
				lcd_drawFont_(lcd, local_x, local_y, bitmap_data, FONT_HALF_WIDTH, FONT_HEIGHT, color, bg);
				local_x += FONT_HALF_WIDTH;
				continue;
			}
		}

		//全角文字
		if (local_x + FONT_WIDTH > lcd->_width) {
			local_x = X_LINE_TO_PIX(this, 0);
			local_y = local_y + FONT_HEIGHT;
		}
		lcd_drawFont_(lcd, local_x, local_y, bitmap_data, FONT_WIDTH, FONT_HEIGHT, color, bg);
		local_x += FONT_WIDTH;
	}
}

/**************************************************************************/
/*!
	@brief  Set text cursor location
	@param  x    X coordinate in pixels
	@param  y    Y coordinate in pixels
*/
/**************************************************************************/
void lcd_setCursor(LCD_Handler_t *lcd, int16_t x, int16_t y)
{
	lcd->cursor_x = x;
	lcd->cursor_y = y;
}

/**************************************************************************/
/*!
	@brief  Get text cursor X location
	@returns    X coordinate in pixels
*/
/**************************************************************************/
int16_t lcd_getCursorX(LCD_Handler_t *lcd)
{
	return lcd->cursor_x;
}

/**************************************************************************/
/*!
	@brief      Get text cursor Y location
	@returns    Y coordinate in pixels
*/
/**************************************************************************/
int16_t lcd_getCursorY(LCD_Handler_t *lcd)
{
	return lcd->cursor_y;
}

/**************************************************************************/
/*!
	@brief   Set text 'magnification' size. Each increase in s makes 1 pixel that much bigger.
	@param  s  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
*/
/**************************************************************************/
void lcd_setTextSize(LCD_Handler_t *lcd, uint8_t s)
{
	lcd->textsize = (s > 0) ? s : 1;
}

/**************************************************************************/
/*!
	@brief   Set text font color with transparant background
	@param   c   16-bit 5-6-5 Color to draw text with
*/
/**************************************************************************/
void lcd_setTextColor(LCD_Handler_t *lcd, uint16_t c)
{
// For 'transparent' background, we'll set the bg
// to the same as fg instead of using a flag
	lcd->textcolor = lcd->textbgcolor = c;
}

/**************************************************************************/
/*!
	@brief   Set text font color with custom background color
	@param   c   16-bit 5-6-5 Color to draw text with
	@param   b   16-bit 5-6-5 Color to draw background/fill with
*/
/**************************************************************************/
void lcd_setTextColor2(LCD_Handler_t *lcd, uint16_t c, uint16_t b)
{
	lcd->textcolor = c;
	lcd->textbgcolor = b;
}

/**************************************************************************/
/*!
	@brief      Whether text that is too long should 'wrap' around to the next line.
	@param  w Set true for wrapping, false for clipping
*/
/**************************************************************************/
void lcd_setTextWrap(LCD_Handler_t *lcd, bool w)
{
	lcd->wrap = w;
}

/**************************************************************************/
/*!
	@brief      Get rotation setting for display
	@returns    0 thru 3 corresponding to 4 cardinal rotations
*/
/**************************************************************************/
uint8_t lcd_getRotation(LCD_Handler_t *lcd)
{
	return lcd->rotation;
}

/**************************************************************************/
/*!
	@brief      Set rotation setting for display
	@param  x   0 thru 3 corresponding to 4 cardinal rotations
*/
/**************************************************************************/
void lcd_setRotation(LCD_Handler_t *lcd, uint8_t x)
{
	lcd->rotation = (x & 3);
	switch (lcd->rotation) {
	case 0:
	case 2:
		lcd->_width = lcd_init_width;
		lcd->_height = lcd_init_height;
		break;
	case 1:
	case 3:
		lcd->_width = lcd_init_height;
		lcd->_height = lcd_init_width;
		break;
	}
}

/**************************************************************************/
/*!
	@brief Set the font to display when print()ing, either custom or default
	@param  f  The GFXfont object, if NULL use built in 6x8 font
*/
/**************************************************************************/
void lcd_setFont(LCD_Handler_t *lcd, const GFXfont *f)
{
	if (f) {            // Font struct pointer passed in?
		if (!lcd->gfxFont) { // And no current font struct?
			// Switching from classic to new font behavior.
			// Move cursor pos down 6 pixels so it's on baseline.
			lcd->cursor_y += 6;
		}
	}
	else if (lcd->gfxFont) { // NULL passed.  Current font struct defined?
	 // Switching from new to classic font behavior.
	 // Move cursor pos up 6 pixels so it's at top-left of char.
		lcd->cursor_y -= 6;
	}
	lcd->gfxFont = (GFXfont *)f;
}


/**************************************************************************/
/*!
	@brief    Helper to determine size of a character with current font/size.
		Broke this out as it's used by both the PROGMEM- and RAM-resident getTextBounds() functions.
	@param    c     The ascii character in question
	@param    x     Pointer to x location of character
	@param    y     Pointer to y location of character
	@param    minx  Minimum clipping value for X
	@param    miny  Minimum clipping value for Y
	@param    maxx  Maximum clipping value for X
	@param    maxy  Maximum clipping value for Y
*/
/**************************************************************************/
void lcd_charBounds(LCD_Handler_t *lcd, char c, int16_t *x, int16_t *y,
	int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy)
{

	if (lcd->gfxFont) {

		if (c == '\n') { // Newline?
			*x = 0;    // Reset x to zero, advance y by one line
			*y += lcd->textsize * (uint8_t)pgm_read_byte(&lcd->gfxFont->yAdvance);
		}
		else if (c != '\r') { // Not a carriage return; is normal char
			uint8_t first = pgm_read_byte(&lcd->gfxFont->first),
				last = pgm_read_byte(&lcd->gfxFont->last);
			if ((c >= first) && (c <= last)) { // Char present in this font?
				GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
					&lcd->gfxFont->glyph))[c - first]);
				uint8_t gw = pgm_read_byte(&glyph->width),
					gh = pgm_read_byte(&glyph->height),
					xa = pgm_read_byte(&glyph->xAdvance);
				int8_t  xo = pgm_read_byte(&glyph->xOffset),
					yo = pgm_read_byte(&glyph->yOffset);
				if (lcd->wrap && ((*x + (((int16_t)xo + gw)*lcd->textsize)) > lcd->_width)) {
					*x = 0; // Reset x to zero, advance y by one line
					*y += lcd->textsize * (uint8_t)pgm_read_byte(&lcd->gfxFont->yAdvance);
				}
				int16_t ts = (int16_t)lcd->textsize,
					x1 = *x + xo * ts,
					y1 = *y + yo * ts,
					x2 = x1 + gw * ts - 1,
					y2 = y1 + gh * ts - 1;
				if (x1 < *minx) *minx = x1;
				if (y1 < *miny) *miny = y1;
				if (x2 > *maxx) *maxx = x2;
				if (y2 > *maxy) *maxy = y2;
				*x += xa * ts;
			}
		}

	}
	else { // Default font

		if (c == '\n') {                     // Newline?
			*x = 0;                        // Reset x to zero,
			*y += lcd->textsize * 8;             // advance y one line
			// min/max x/y unchaged -- that waits for next 'normal' character
		}
		else if (c != '\r') {  // Normal char; ignore carriage returns
			if (lcd->wrap && ((*x + lcd->textsize * 6) > lcd->_width)) { // Off right?
				*x = 0;                    // Reset x to zero,
				*y += lcd->textsize * 8;         // advance y one line
			}
			int x2 = *x + lcd->textsize * 6 - 1, // Lower-right pixel of char
				y2 = *y + lcd->textsize * 8 - 1;
			if (x2 > *maxx) *maxx = x2;      // Track max x, y
			if (y2 > *maxy) *maxy = y2;
			if (*x < *minx) *minx = *x;      // Track min x, y
			if (*y < *miny) *miny = *y;
			*x += lcd->textsize * 6;             // Advance x one char
		}
	}
}

/**************************************************************************/
/*!
	@brief    Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
	@param    str     The ascii string to measure
	@param    x       The current cursor X
	@param    y       The current cursor Y
	@param    x1      The boundary X coordinate, set by function
	@param    y1      The boundary Y coordinate, set by function
	@param    w      The boundary width, set by function
	@param    h      The boundary height, set by function
*/
/**************************************************************************/
void lcd_getTextBounds(LCD_Handler_t *lcd, const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w = *h = 0;

	int16_t minx = lcd->_width, miny = lcd->_height, maxx = -1, maxy = -1;

	while ((c = *str++))
		lcd_charBounds(lcd, c, &x, &y, &minx, &miny, &maxx, &maxy);

	if (maxx >= minx) {
		*x1 = minx;
		*w = maxx - minx + 1;
	}
	if (maxy >= miny) {
		*y1 = miny;
		*h = maxy - miny + 1;
	}
}

/**************************************************************************/
/*!
	@brief    Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
	@param    str    The ascii string to measure (as an arduino String() class)
	@param    x      The current cursor X
	@param    y      The current cursor Y
	@param    x1     The boundary X coordinate, set by function
	@param    y1     The boundary Y coordinate, set by function
	@param    w      The boundary width, set by function
	@param    h      The boundary height, set by function
*/
/**************************************************************************/
void lcd_getTextBounds2(LCD_Handler_t *lcd, const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	if (strlen(str) != 0) {
		lcd_getTextBounds(lcd, str, x, y, x1, y1, w, h);
	}
}


/**************************************************************************/
/*!
	@brief    Helper to determine size of a PROGMEM string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
	@param    str     The flash-memory ascii string to measure
	@param    x       The current cursor X
	@param    y       The current cursor Y
	@param    x1      The boundary X coordinate, set by function
	@param    y1      The boundary Y coordinate, set by function
	@param    w      The boundary width, set by function
	@param    h      The boundary height, set by function
*/
/**************************************************************************/
void lcd_getTextBounds3(LCD_Handler_t *lcd, const char *str,
	int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	uint8_t *s = (uint8_t *)str, c;

	*x1 = x;
	*y1 = y;
	*w = *h = 0;

	int16_t minx = lcd->_width, miny = lcd->_height, maxx = -1, maxy = -1;

	while ((c = pgm_read_byte(s++)))
		lcd_charBounds(lcd, c, &x, &y, &minx, &miny, &maxx, &maxy);

	if (maxx >= minx) {
		*x1 = minx;
		*w = maxx - minx + 1;
	}
	if (maxy >= miny) {
		*y1 = miny;
		*h = maxy - miny + 1;
	}
}

/**************************************************************************/
/*!
	@brief      Get width of the display, accounting for the current rotation
	@returns    Width in pixels
*/
/**************************************************************************/
int16_t lcd_width(LCD_Handler_t *lcd)
{
	return lcd->_width;
}

/**************************************************************************/
/*!
	@brief      Get height of the display, accounting for the current rotation
	@returns    Height in pixels
*/
/**************************************************************************/
int16_t lcd_height(LCD_Handler_t *lcd)
{
	return lcd->_height;
}

/***************************************************************************/
