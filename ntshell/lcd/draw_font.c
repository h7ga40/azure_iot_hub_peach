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

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <string.h>
#include <target_syssvc.h>
#ifndef ADAFRUIT_SSD1306
#include "adafruit_st7735.h"
#else
#include "adafruit_ssd1306.h"
#endif
#include "draw_font.h"

void get_bitmap_font(const uint8_t *string, uint8_t *bitmap_data, uint32_t *use_chars)
{
	uint32_t len, code;
	uint8_t i, j, k;
	uint32_t totalj, totalk;

	*use_chars = 0;
	len = 0;
	if ((string[0] & 0x80) == 0) { len = 1; }
	else if ((string[0] & 0xE0) == 0xC0) { len = 2; }
	else if ((string[0] & 0xF0) == 0xE0) { len = 3; }
	else if ((string[0] & 0xF8) == 0xF0) { len = 4; }
	else { return; }

	j = k = totalj = totalk = 0;

	if (len == 1) {
		code = string[0];
		memcpy(bitmap_data, &UTF8_1B_CODE_BITMAP[code][0], FONT_WIDTH * FONT_HEIGHT / 8);
		*use_chars = 1;
		return;
	}

	if (len == 2) {
		code = string[0];
		// 1バイト目サーチ
		for (i = 0; i < UTF8_CODE_2B_1_NUM; i++) {
			if (Utf8CodeTable_2B_1st[i][0] == code) {
				code = string[1];
				for (j = 0; j < Utf8CodeTable_2B_1st[i][1]; j++) {
					if (UTF8_2B_CODE_BITMAP[totalk].code == code) {
						memcpy(bitmap_data, UTF8_2B_CODE_BITMAP[totalk].bitmap, FONT_WIDTH * FONT_HEIGHT / 8);
						*use_chars = 2;
						return;
					}
					totalk++;
				}
			}
			else {
				totalk += Utf8CodeTable_2B_1st[i][1];
			}
		}
		return;
	}

	if (len == 3) {
		code = string[0];
		// 1バイト目サーチ
		for (i = 0; i < UTF8_CODE_3B_1_NUM; i++) {
			if (Utf8CodeTable_3B_1st[i][0] == code) {
				code = string[1];
				// 2バイト目サーチ
				for (j = 0; j < Utf8CodeTable_3B_1st[i][1]; j++) {
					if (Utf8CodeTable_3B_2nd[totalj][0] == code) {
						code = string[2];
						// 3バイト目サーチ
						for (k = 0; k < Utf8CodeTable_3B_2nd[totalj][1]; k++) {
							if (UTF8_3B_CODE_BITMAP[totalk].code == code) {
								memcpy(bitmap_data, UTF8_3B_CODE_BITMAP[totalk].bitmap, FONT_WIDTH * FONT_HEIGHT / 8);
								*use_chars = 3;
								return;
							}
							totalk++;
						}
						return;
					}
					else {/*読み飛ばすbitmap個数を蓄積*/
						totalk += Utf8CodeTable_3B_2nd[totalj][1];
					}
					totalj++;
				}
			}
			else {/*読み飛ばすbitmap個数を蓄積*/
				for (j = 0; j < Utf8CodeTable_3B_1st[i][1]; j++) {
					totalk += Utf8CodeTable_3B_2nd[totalj][1];
					totalj++;
				}
			}
		}
		return;
	}
}

void lcd_drawFont(LCD_Handler_t *hlcd, uint8_t *bitmap_data, int x, int y, uint16_t color, uint16_t back_color)
{
	int i, j, b;
	uint8_t *bitmap = bitmap_data;

	b = 0x80;
	for (i = 0; i < FONT_HEIGHT; i++) {
		for (j = 0; j < FONT_WIDTH; j++) {
			if ((*bitmap & b) != 0) {
				lcd_drawPixel(hlcd, x + j, y + i, color);
			}
			else {
				lcd_drawPixel(hlcd, x + j, y + i, back_color);
			}
			b >>= 1;
			if (b == 0) {
				b = 0x80;
				bitmap++;
			}
		}
	}
}

void lcd_drawFontHalf(LCD_Handler_t *hlcd, uint8_t *bitmap_data, int x, int y, uint16_t color, uint16_t back_color)
{
	int i, j, b;
	uint8_t *bitmap = bitmap_data;

	b = 0x80;
	for (i = 0; i < FONT_HEIGHT; i++) {
		for (j = 0; j < FONT_HALF_WIDTH; j++) {
			if ((*bitmap & b) != 0) {
				lcd_drawPixel(hlcd, x + j, y + i, color);
			}
			else {
				lcd_drawPixel(hlcd, x + j, y + i, back_color);
			}
			b >>= 1;
			if (b == 0) {
				b = 0x80;
				bitmap++;
			}
		}
	}
}

void lcd_drawString(LCD_Handler_t *hlcd, const char *string, int x, int y, uint16_t color, uint16_t back_color)
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
				local_x = X_LINE_TO_PIX(hlcd, 0);
				continue;
			}
			if (ctrl_code == 0x0A) { // LF
				local_y = local_y + FONT_HEIGHT;
				continue;
			}

			if (local_x + FONT_HALF_WIDTH > hlcd->_width) {
				local_x = X_LINE_HALF_TO_PIX(hlcd, 0);
				local_y = local_y + FONT_HEIGHT;
			}
			lcd_drawFontHalf(hlcd, bitmap_data, local_x, local_y, color, back_color);
			local_x += FONT_HALF_WIDTH;
			continue;
		}

		//3バイトコード半角文字
		if (use_chars == 3) {
			if (((0xEFBDA1 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBDBF)) ||
				((0xEFBE80 <= for_3B_hankaku_code) && (for_3B_hankaku_code <= 0xEFBE9F))) {
				//3バイトコード半角文字
				if (local_x + FONT_HALF_WIDTH > hlcd->_width) {
					local_x = X_LINE_HALF_TO_PIX(hlcd, 0);
					local_y = local_y + FONT_HEIGHT;
				}
				lcd_drawFontHalf(hlcd, bitmap_data, local_x, local_y, color, back_color);
				local_x += FONT_HALF_WIDTH;
				continue;
			}
		}

		//全角文字
		if (local_x + FONT_WIDTH > hlcd->_width) {
			local_x = X_LINE_TO_PIX(hlcd, 0);
			local_y = local_y + FONT_HEIGHT;
		}
		lcd_drawFont(hlcd, bitmap_data, local_x, local_y, color, back_color);
		local_x += FONT_WIDTH;
	}
}

