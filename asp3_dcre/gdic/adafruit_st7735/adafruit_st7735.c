/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2012 by Embedded and Real-Time Systems Laboratory
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
 *  ADAFRUIT ST7735 1.8"LCD制御プログラムの本体
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "gpio_api.h"
#include "spi_api.h"
#include "adafruit_st7735.h"

/*
 *  サービスコールのエラーのログ出力
 */
Inline void
svc_perror(const char *file, int_t line, const char *expr, ER ercd)
{
	if (ercd < 0) {
		t_perror(LOG_ERROR, file, line, expr, ercd);
	}
}

#define	SVC_PERROR(expr)	svc_perror(__FILE__, __LINE__, #expr, (expr))
#define ABS(X)  ((X) > 0 ? (X) : -(X))


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t
  Bcmd[] = {                  // Initialization commands for 7735B screens
    18,                       // 18 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
      50,                     //     50 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
      255,                    //     255 = 500 ms delay
    ST7735_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
      0x05,                   //     16-bit color
      10,                     //     10 ms delay
    ST7735_FRMCTR1, 3+DELAY,  //  4: Frame rate control, 3 args + delay:
      0x00,                   //     fastest refresh
      0x06,                   //     6 lines front porch
      0x03,                   //     3 lines back porch
      10,                     //     10 ms delay
    ST7735_MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
      0x08,                   //     Row addr/col addr, bottom to top refresh
    ST7735_DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
      0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                              //     rise, 3 cycle osc equalize
      0x02,                   //     Fix on VTL
    ST7735_INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
      0x0,                    //     Line inversion
    ST7735_PWCTR1 , 2+DELAY,  //  8: Power control, 2 args + delay:
      0x02,                   //     GVDD = 4.7V
      0x70,                   //     1.0uA
      10,                     //     10 ms delay
    ST7735_PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
      0x05,                   //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
      0x01,                   //     Opamp current small
      0x02,                   //     Boost frequency
    ST7735_VMCTR1 , 2+DELAY,  // 11: Power control, 2 args + delay:
      0x3C,                   //     VCOMH = 4V
      0x38,                   //     VCOML = -1.1V
      10,                     //     10 ms delay
    ST7735_PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST7735_GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
      0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
      0x21, 0x1B, 0x13, 0x19, //      these config values represent)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E, //     (ditto)
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                     //     10 ms delay
    ST7735_CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 2
      0x00, 0x81,             //     XEND = 129
    ST7735_RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 1
      0x00, 0x81,             //     XEND = 160
    ST7735_NORON  ,   DELAY,  // 17: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,   DELAY,  // 18: Main screen turn on, no args, w/delay
      255};                   //     255 = 500 ms delay

static const uint8_t
  Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 };                 //     16-bit color

static const uint8_t
  Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01 };      //     XEND = 159

static const uint8_t
  Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F };           //     XEND = 159

static const uint8_t
  Rcmd2green144[] = {              // Init for 7735R, part 2 (green 1.44 tab)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F };           //     XEND = 127

static const uint8_t
  Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay




/* Buffer used for transmission */
static uint8_t aTxBuffer[4];

#define PORT_HIGH           1
#define PORT_LOW            0

#define cs_set(sw)  gpio_write(&hlcd->cs_pin, sw)
#define rs_set(sw)  gpio_write(&hlcd->rs_pin, sw)
#define rst_set(sw) gpio_write(&hlcd->rst_pin, sw)
#define cs2_set(sw) gpio_write(&hlcd->cs2_pin, sw)

/*
 *  LCDへのコマンド送信関数
 */
ER
lcd_writecommand(LCD_Handler_t *hlcd, uint8_t c)
{
	ER ercd = E_OK;

	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	rs_set(PORT_LOW);
	cs_set(PORT_LOW);
	spi_master_write(&hlcd->hspi, c);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへのデータ送信関数
 */
ER
lcd_writedata(LCD_Handler_t *hlcd, uint8_t c)
{
	ER ercd = E_OK;

	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	rs_set(PORT_HIGH);
	cs_set(PORT_LOW);
	//dly_tsk(100/*us*/);
	spi_master_write(&hlcd->hspi, c);
	//dly_tsk(100/*us*/);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへの２バイトデータ送信関数
 */
ER
lcd_writedata2(LCD_Handler_t *hlcd, uint16_t c, int cnt)
{
	ER  ercd = E_OK;
	int i;

	if(cnt == 0)
		return ercd;
	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	rs_set(PORT_HIGH);
	cs_set(PORT_LOW);
	//dly_tsk(100/*us*/);
	for(i = 0 ; i < cnt ; i++){
		spi_master_write(&hlcd->hspi, c >> 8);
		spi_master_write(&hlcd->hspi, c);
	}
	//dly_tsk(100/*us*/);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへのRGBデータ送信関数
 */
ER
lcd_writedata3(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height)
{
	ER  ercd = E_OK;
	int index, i;

	if(width == 0 || height == 0)
		return ercd;
	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	rs_set(PORT_HIGH);
	cs_set(PORT_LOW);
	//dly_tsk(100/*us*/);
	for(index=0; index < height; index++){
		uint8_t *p = pbmp;
		for (i = 0; i < width; i++){
			aTxBuffer[0]  = (*p++) & 0xf8;
			aTxBuffer[0] |= *p >> 5;
			aTxBuffer[1]  = (*p++ << 3) & 0xE0;
			aTxBuffer[1] |= (*p++ >> 3) & 0x1F;
			spi_master_write(&hlcd->hspi, aTxBuffer[0]);
			spi_master_write(&hlcd->hspi, aTxBuffer[1]);
		}
		pbmp -= width*3;
	}
	//dly_tsk(100/*us*/);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  LCDへのRGBデータ送信関数
 */
ER
lcd_writedata4(LCD_Handler_t *hlcd, uint8_t *pbmp, uint16_t width, uint16_t height)
{
	ER  ercd = E_OK;
	int index, i;

	if(width == 0 || height == 0)
		return ercd;
	if(hlcd->spi_lock != 0){
		if((ercd = wai_sem(hlcd->spi_lock)) != E_OK)
			return ercd;
	}
	rs_set(PORT_HIGH);
	cs_set(PORT_LOW);
	//dly_tsk(100/*us*/);
	for(index=0; index < height; index++){
		uint8_t *p = pbmp;
		for (i = 0; i < width; i++){
			spi_master_write(&hlcd->hspi, *p++);
			spi_master_write(&hlcd->hspi, *p++);
		}
		pbmp -= width*2;
	}
	//dly_tsk(100/*us*/);
	cs_set(PORT_HIGH);
	if(hlcd->spi_lock != 0)
		sig_sem(hlcd->spi_lock);
	return ercd;
}

/*
 *  ST7735へのコマンドリスト送信
 */
static void
Adafruit_ST7735_commandList(LCD_Handler_t *hlcd, const uint8_t *addr)
{
	uint8_t  numCommands, numArgs;
	uint16_t ms;

	numCommands = *addr++;   // Number of commands to follow
	while(numCommands--) {                 // For each command...
		SVC_PERROR(lcd_writecommand(hlcd, *addr++)); //   Read, issue command
		numArgs  = *addr++;    //   Number of args to follow
		ms       = numArgs & DELAY;          //   If hibit set, delay follows args
		numArgs &= ~DELAY;                   //   Mask out delay bit
		while(numArgs--) {                   //   For each argument...
			SVC_PERROR(lcd_writedata(hlcd, *addr++));  //     Read, issue argument
		}

		if(ms) {
			ms = *addr++; // Read post-command delay time (ms)
			if(ms == 255) ms = 500;     // If 255, delay for 500 ms
			dly_tsk(ms * 1000);
		}
	}
}


/*
 *  ST7735B または ST7735RタイプのLEDの共通初期化関数
 */
static void
Adafruit_ST7735_commonInit(LCD_Handler_t *hlcd, const uint8_t *cmdList)
{
	hlcd->colstart  = hlcd->rowstart = 0; // May be overridden in init func
	hlcd->_width = ST7735_TFTWIDTH;
	hlcd->_height = ST7735_TFTHEIGHT_18;

	// toggle RST low to reset; CS low so it'll listen to us
	cs_set(PORT_LOW);
	rst_set(PORT_HIGH);
	dly_tsk(500 * 1000);
	rst_set(PORT_LOW);
	dly_tsk(500 * 1000);
	rst_set(PORT_HIGH);

	if(cmdList) Adafruit_ST7735_commandList(hlcd, cmdList);
}


/*
 *  表示ウィンドウ設定
 */
void
lcd_setAddrWindow(LCD_Handler_t *hlcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	SVC_PERROR(lcd_writecommand(hlcd, ST7735_CASET)); // Column addr set
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, x0+hlcd->colstart));     // XSTART 
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, x1+hlcd->colstart));     // XEND

	SVC_PERROR(lcd_writecommand(hlcd, ST7735_RASET)); // Row addr set
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, y0+hlcd->rowstart));     // YSTART
	SVC_PERROR(lcd_writedata(hlcd, 0x00));
	SVC_PERROR(lcd_writedata(hlcd, y1+hlcd->rowstart));     // YEND

	SVC_PERROR(lcd_writecommand(hlcd, ST7735_RAMWR)); // write to RAM
}

/*
 *  ST7735B screensの初期化関数
 */
void
lcd_initB(LCD_Handler_t *hlcd)
{
	Adafruit_ST7735_commonInit(hlcd, Bcmd);
}

/*
 *  ST7735R screens (green or red tabs)の初期化関数
 */
void
lcd_initR(LCD_Handler_t *hlcd, uint8_t options)
{
	Adafruit_ST7735_commonInit(hlcd, Rcmd1);

	if(options == INITR_GREENTAB){
		Adafruit_ST7735_commandList(hlcd, Rcmd2green);
		hlcd->colstart = 2;
		hlcd->rowstart = 1;
	}
	else if(options == INITR_144GREENTAB){
		hlcd->_height = ST7735_TFTHEIGHT_144;
		Adafruit_ST7735_commandList(hlcd, Rcmd2green144);
 		hlcd->colstart = 2;
		hlcd->rowstart = 3;
	}
	else{
		// colstart, rowstart left at default '0' values
		Adafruit_ST7735_commandList(hlcd, Rcmd2red);
	}
	Adafruit_ST7735_commandList(hlcd, Rcmd3);

	// if black, change MADCTL color filter
	if(options == INITR_BLACKTAB){
		SVC_PERROR(lcd_writecommand(hlcd, ST7735_MADCTL));
		SVC_PERROR(lcd_writedata(hlcd, 0xC0));
	}
	syslog_2(LOG_NOTICE, "## width(%d) height(%d) ##", hlcd->_width, hlcd->_height);
	hlcd->tabcolor = options;
}

/*
 *  RECTANGLEのフィル描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  w: width
 *  param5  h: height
 *  param6  color: color value
 */
void
lcd_fillRect(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x + w - 1) >= hlcd->_width)  w = hlcd->_width  - x;
	if((y + h - 1) >= hlcd->_height) h = hlcd->_height - y;

	lcd_setAddrWindow(hlcd, x, y, x+w-1, y+h-1);
	SVC_PERROR(lcd_writedata2(hlcd, color, h*w));
}

void
lcd_pushColor(LCD_Handler_t *hlcd, uint16_t color)
{
  SVC_PERROR(lcd_writedata2(hlcd, color, 1));
}

/*
 *  PIXEL描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: X position
 *  param3  y: Y position
 *  param4  color: color value
 */
void
lcd_drawPixel(LCD_Handler_t *hlcd, int16_t x, int16_t y, uint16_t color)
{
	if((x < 0) ||(x >= hlcd->_width) || (y < 0) || (y >= hlcd->_height)) return;

	lcd_setAddrWindow(hlcd, x, y, x+1, y+1);
	SVC_PERROR(lcd_writedata2(hlcd, color, 1));
}

/*
 *  垂直方向LINEの高速描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  h: height
 *  param5  color: color value
 */
void
lcd_drawFastVLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((y+h-1) >= hlcd->_height) h = hlcd->_height-y;
	lcd_setAddrWindow(hlcd, x, y, x, y+h-1);
	SVC_PERROR(lcd_writedata2(hlcd, color, h));
}

/*
 *  水平方向LINEの高速描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x: start X position
 *  param3  y: start Y position
 *  param4  w: width
 *  param5  color: color value
 */
void lcd_drawFastHLine(LCD_Handler_t *hlcd, int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x+w-1) >= hlcd->_width)  w = hlcd->_width-x;
	lcd_setAddrWindow(hlcd, x, y, x+w-1, y);
	SVC_PERROR(lcd_writedata2(hlcd, color, w));
}

/*
 *  BITMAP描画
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  x0: Bmp X position in the LCD
 *  param3  y0: Bmp Y position in the LCD
 *  param4  pbmp: Pointer to Bmp picture address in the internal Flash
 */
void
lcd_drawBitmap(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, uint8_t *pbmp)
{
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
	uint32_t input_color_mode = 0;

	/* Get bitmap data address offset */
	index = *(uint16_t *) (pbmp + 10);
	index |= (*(uint16_t *) (pbmp + 12)) << 16;

	/* Read bitmap width */
	width = *(uint16_t *) (pbmp + 18);
	width |= (*(uint16_t *) (pbmp + 20)) << 16;

	/* Read bitmap height */
	height = *(uint16_t *) (pbmp + 22);
	height |= (*(uint16_t *) (pbmp + 24)) << 16;

	/* Read bit/pixel */
	bit_pixel = *(uint16_t *) (pbmp + 28);

	/* Get the layer pixel format */
	if ((bit_pixel/8) == 4){
		input_color_mode = CM_ARGB8888;
	}
	else if ((bit_pixel/8) == 2){
		input_color_mode = CM_RGB565;
	}
	else{
		input_color_mode = CM_RGB888;
	}

	/* Bypass the bitmap header */
	pbmp += (index + (width * (height - 1) * (bit_pixel/8)));  
	syslog_4(LOG_NOTICE, "## input_color_mode(%d) width(%d) height(%d) bit_pixel(%d) ##", input_color_mode, width, height, bit_pixel);

	lcd_setAddrWindow(hlcd, x0, y0, x0+width-1, y0+height-1);
	lcd_writedata3(hlcd, pbmp, width, height);
}

void
lcd_drawImage(LCD_Handler_t *hlcd, uint16_t x0, uint16_t y0, int16_t w, int16_t h, uint8_t *pbmp)
{
	lcd_setAddrWindow(hlcd, x0, y0, x0+w-1, y0+h-1);
	pbmp += w * (h - 1) * 3;  
	lcd_writedata3(hlcd, pbmp, w, h);
}

/*
 *  INVERT DISPLAY
 *  param1  hlcd: Pointer to LCD Handler
 *  param2  i: invert value
 */
void
lcd_invertDisplay(LCD_Handler_t *hlcd, bool_t i)
{
  SVC_PERROR(lcd_writecommand(hlcd, i ? ST7735_INVON : ST7735_INVOFF));
}


/*
 *  スクリーンフィル
 *  param1  pDrawProp: Pointer to Draw Prop
 */
void
lcd_fillScreen(LCD_DrawProp_t *pDrawProp)
{
	lcd_fillRect(pDrawProp->hlcd, 0, 0,  pDrawProp->hlcd->_width, pDrawProp->hlcd->_height, pDrawProp->BackColor);
}

/*
 *  RECTANGLE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x: left X position
 *  param3  y: top Y position
 *  param4  w: width
 *  param5  h: height
 */
void
lcd_drawRect(LCD_DrawProp_t *pDrawProp, int16_t x, int16_t y, int16_t w, int16_t h)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	uint16_t color;

	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= hlcd->_width) || (y >= hlcd->_height)) return;
	if((x + w - 1) >= hlcd->_width)  w = hlcd->_width  - x;
	if((y + h - 1) >= hlcd->_height) h = hlcd->_height - y;

	color = pDrawProp->TextColor;
	lcd_drawFastVLine(hlcd, x, y, h, color);
	lcd_drawFastHLine(hlcd, x, y+h-1, w, color);
	lcd_drawFastVLine(hlcd, x+w-1, y, h, color);
	lcd_drawFastHLine(hlcd, x, y, w, color);
}

/*
 *  円描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x0: X position
 *  param3  y0: Y position
 *  param4  Radius: Circle radius
 */
void
lcd_DrawCircle(LCD_DrawProp_t *pDrawProp, uint16_t x0, uint16_t y0, uint16_t Radius)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
 	int32_t   decision;    /* Decision Variable */ 
	uint32_t  current_x;   /* Current X Value */
	uint32_t  current_y;   /* Current Y Value */

	decision = 3 - (Radius << 1);
	current_x = 0;
	current_y = Radius;

	while(current_x <= current_y){
		lcd_drawPixel(hlcd, (x0 + current_x), (y0 - current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_x), (y0 - current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_y), (y0 - current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_y), (y0 - current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_x), (y0 + current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_x), (y0 + current_y), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 + current_y), (y0 + current_x), pDrawProp->TextColor);
		lcd_drawPixel(hlcd, (x0 - current_y), (y0 + current_x), pDrawProp->TextColor);

		if (decision < 0){
			decision += (current_x << 2) + 6;
		}
		else{
			decision += ((current_x - current_y) << 2) + 10;
			current_y--;
		}
		current_x++;
	}
}

/*
 *  線描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  x1: Point 1 X position
 *  param3  y1: Point 1 Y position
 *  param4  x2: Point 2 X position
 *  param5  y2: Point 2 Y position
 */
void
lcd_drawLine(LCD_DrawProp_t *pDrawProp, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_Handler_t *hlcd = pDrawProp->hlcd;
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0, 
	curpixel = 0;

	deltax = ABS(x2 - x1);		/* The difference between the x's */
	deltay = ABS(y2 - y1);		/* The difference between the y's */
	x = x1;						/* Start x off at the first pixel */
	y = y1;						/* Start y off at the first pixel */

	if(x2 >= x1){				/* The x-values are increasing */
		xinc1 = 1;
		xinc2 = 1;
	}
	else{						/* The x-values are decreasing */
		xinc1 = -1;
		xinc2 = -1;
	}

	if(y2 >= y1){				/* The y-values are increasing */
		yinc1 = 1;
		yinc2 = 1;
	}
	else{						/* The y-values are decreasing */
		yinc1 = -1;
		yinc2 = -1;
	}

	if(deltax >= deltay){		/* There is at least one x-value for every y-value */
		xinc1 = 0;				/* Don't change the x when numerator >= denominator */
		yinc2 = 0;				/* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		num_add = deltay;
		num_pixels = deltax;	/* There are more x-values than y-values */
	}
	else{						/* There is at least one y-value for every x-value */
		xinc2 = 0;				/* Don't change the x for every iteration */
		yinc1 = 0;				/* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		num_add = deltax;
		num_pixels = deltay;	/* There are more y-values than x-values */
	}

	for (curpixel = 0; curpixel <= num_pixels; curpixel++){
		lcd_drawPixel(hlcd, x, y, pDrawProp->TextColor);	/* Draw the current pixel */
		num += num_add;			/* Increase the numerator by the top of the fraction */
		if(num >= den){			/* Check if numerator >= denominator */
			num -= den;			/* Calculate the new numerator value */
			x += xinc1;			/* Change the x as appropriate */
			y += yinc1;			/* Change the y as appropriate */
		}
		x += xinc2;				/* Change the x as appropriate */
		y += yinc2;				/* Change the y as appropriate */
	}
}

/*
 *  PLOY-LINE描画
 *  param1  pDrawProp: Pointer to Draw Prop
 *  param2  Points: Pointer to the points array
 *  param3  PointCount: Number of points
 */
void
lcd_drawPolygon(LCD_DrawProp_t *pDrawProp, pPoint Points, uint16_t PointCount)
{
	int16_t x = 0, y = 0;

	if(PointCount < 2){
		return;
	}

	lcd_drawLine(pDrawProp, Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
	while(--PointCount){
		x = Points->X;
		y = Points->Y;
		Points++;
		lcd_drawLine(pDrawProp, x, y, Points->X, Points->Y);
	}
}

