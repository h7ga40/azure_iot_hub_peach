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

#ifndef MISAKI_FONT_H
#define MISAKI_FONT_H

#include <stdint.h>

#define FONT_HALF_WIDTH			4
#define FONT_WIDTH				8
#define FONT_HEIGHT				8

struct utf8_code_bitmap {
	const uint8_t code;
	const uint8_t bitmap[FONT_WIDTH * FONT_HEIGHT / 8];
};

#define UTF8_CODE_2B_1_NUM		6
#define UTF8_CODE_3B_1_NUM		9
#define UTF8_CODE_3B_2_NUM		(343+1)		/*半角追加2/2*/

#define UTF8_1B_CODE_BITMAP_NUM	128
#define UTF8_2B_CODE_BITMAP_NUM	122													/*"￢","＼","￠""￡"移動)*/
#define UTF8_3B_CODE_BITMAP_NUM	(6757+31+32)	/*半角追加1/2*/	/*半角追加2/2*/		/*"￢","＼","￠""￡"追加)*/

// 1バイトコード
extern const uint8_t UTF8_1B_CODE_BITMAP[UTF8_1B_CODE_BITMAP_NUM][FONT_HALF_WIDTH * FONT_HEIGHT / 8];

// 2バイトコード　1バイト目
extern const uint8_t Utf8CodeTable_2B_1st[UTF8_CODE_2B_1_NUM][2];
// 2バイトコード 2バイト目 bitmapデータ
extern const struct utf8_code_bitmap UTF8_2B_CODE_BITMAP[UTF8_2B_CODE_BITMAP_NUM];

// 3バイトコード 1バイト目
extern const uint8_t Utf8CodeTable_3B_1st[UTF8_CODE_3B_1_NUM][2];
// 3バイトコード 2バイト目
extern const uint8_t Utf8CodeTable_3B_2nd[UTF8_CODE_3B_2_NUM][2];

// 3バイトコード 3バイト目 bitmapデータ
extern const struct utf8_code_bitmap UTF8_3B_CODE_BITMAP[UTF8_3B_CODE_BITMAP_NUM];

#endif /* MISAKI_FONT_H */
