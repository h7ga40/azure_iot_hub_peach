/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2018-2019 Cores Co., Ltd. Japan
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
 *  @(#) $Id$
 */

#ifndef DRAW_FONT_H
#define DRAW_FONT_H

#if defined(SHNM16_FONT)
#include "shnm16_font.h"
#elif defined(SHNM12_FONT)
#include "shnm12_font.h"
#else
#include "misaki_font.h"
#endif

#define DISP_X_ADJ	0
#define DISP_Y_ADJ	0
#define X_ZENKAKU_CHARACTERS(hlcd)	((hlcd)->_width / FONT_WIDTH)
#define X_HANKAKU_CHARACTERS(hlcd)	((hlcd)->_width / FONT_HALF_WIDTH)
#define Y_CHARACTERS(hlcd)			((hlcd)->_height / FONT_HEIGHT)
#define X_LINE_TO_PIX(hlcd, x)		((FONT_WIDTH*(x))-DISP_X_ADJ)
#define X_LINE_HALF_TO_PIX(hlcd, x)	((FONT_HALF_WIDTH*(x))-DISP_X_ADJ)
#define Y_ROW_TO_PIX(hlcd, y)		((FONT_HEIGHT*(y))-DISP_Y_ADJ)

#ifdef __cplusplus
extern "C" {
#endif

void get_bitmap_font(const uint8_t *string, uint8_t *bitmap_data, uint32_t *use_chars);
void lcd_drawFont(LCD_Handler_t *hlcd, uint8_t *bitmap_data, int x, int y, uint16_t color, uint16_t back_color);
void lcd_drawFontHalf(LCD_Handler_t *hlcd, uint8_t *bitmap_data, int x, int y, uint16_t color, uint16_t back_color);
void lcd_drawString(LCD_Handler_t *hlcd, const char *string, int x, int y, uint16_t color, uint16_t back_color );

#ifdef __cplusplus
}
#endif

#endif /* DRAW_FONT_H */
