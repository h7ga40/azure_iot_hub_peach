/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2017 by TOPPERS PROJECT Educational Working Group.
 *
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 *
 *  $Id$
 */

/*
 *  Written by Limor Fried/Ladyada for Adafruit Industries.
 *  MIT license, all text above must be included in any redistribution
 *　http://opensource.org/licenses/mit-license.php 
 */

/*
 *  ADAFRUIT ST7735 1.8"LCD制御プログラムのヘッダファイル
 */

#ifndef _ADAFRUIT_ST7735_H_
#define _ADAFRUIT_ST7735_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include "device.h"
#include "pinmap.h"
#include "spi_api.h"

/*
 *  LCDカラーモード定義
 */
#define CM_ARGB8888         0x00000000			/* ARGB8888 color mode */
#define CM_RGB888           0x00000001			/* RGB888 color mode */
#define CM_RGB565           0x00000002			/* RGB565 color mode */

// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define INITR_18GREENTAB    INITR_GREENTAB
#define INITR_18REDTAB      INITR_REDTAB
#define INITR_18BLACKTAB    INITR_BLACKTAB
#define INITR_144GREENTAB   0x1

#define ST7735_TFTWIDTH  128
// for 1.44" display
#define ST7735_TFTHEIGHT_144 128
// for 1.8" display
#define ST7735_TFTHEIGHT_18  160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF


#ifndef TOPPERS_MACRO_ONLY

typedef struct GFXfont GFXfont;

/*
 *  LCDハンドラ構造体定義
 */
typedef struct
{
	spi_t					hspi;		/* spiハンドラ */
	uint16_t                _width;		/* 幅ピクセル数 */
	uint16_t				_height;	/* 高さピクセル数 */
	int16_t cursor_x, cursor_y;
	uint16_t textcolor, textbgcolor;
	uint8_t textsize, rotation;
	bool wrap;
	GFXfont *gfxFont;
	uint8_t                 colstart;
	uint8_t                 rowstart;
	uint8_t                 tabcolor;	/* some displays need this changed */
	uint8_t                 dummy;
	ID                      spi_lock;
	gpio_t                  cs_pin;
	gpio_t                  rs_pin;
	gpio_t                  rst_pin;
	gpio_t                  cs2_pin;
}LCD_Handler_t;

/*
 *  描画構造体
 */
typedef struct
{
	LCD_Handler_t           *hlcd;
	uint16_t                TextColor;
	uint16_t                BackColor;
	void                    *pFont;
}LCD_DrawProp_t;

typedef struct
{
	int16_t X;
	int16_t Y;
}Point, * pPoint;

/*
 *  関数のプロトタイプ宣言
 */
extern ER lcd_writecommand(LCD_Handler_t *hlcd, uint8_t c);
extern ER lcd_writedata(LCD_Handler_t *hlcd, uint8_t c);
extern ER lcd_writedata2(LCD_Handler_t *hlcd, uint16_t c, int cnt);
extern ER lcd_writedata3(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height);
extern ER lcd_writedata4(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height);

extern void lcd_initB(LCD_Handler_t *hlcd);
extern void lcd_initR(LCD_Handler_t *hlcd, uint8_t options);
extern void lcd_setAddrWindow(LCD_Handler_t *hlcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
extern void lcd_fillRect(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
extern void lcd_pushColor(LCD_Handler_t *hlcd, uint16_t color);
extern void lcd_drawPixel(LCD_Handler_t *hlcd, int16_t x, int16_t y, uint16_t color);
extern void lcd_drawFastVLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t h, uint16_t color);
extern void lcd_drawFastHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, uint16_t color);
extern void lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint8_t *pbmp);
extern void lcd_drawImage(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, int16_t w, int16_t h, uint8_t *pbmp);
extern void lcd_invertDisplay(LCD_Handler_t *hlcd, bool_t i);

extern void lcd_fillScreen(LCD_DrawProp_t *pDrawProp);
extern void lcd_drawRect(LCD_DrawProp_t *pDrawProp, int16_t x, int16_t y, int16_t w, int16_t h);
extern void lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint16_t Radius);
extern void lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
extern void lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount);

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif

#endif	/* _ADAFRUIT_ST7735_H_ */

