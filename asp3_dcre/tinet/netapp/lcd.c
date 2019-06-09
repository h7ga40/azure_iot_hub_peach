/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2017 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *
 *  上記著作権者は，以下の (1)～(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次の条件を満たすこと．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id$
 */

#include <stdarg.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <sil.h>
#include "target_syssvc.h"
#include "target_lcd.h"
#include <kernel.h>
#include <t_syslog.h>

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "hw_lcd.h"
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/lcd.h>

/*
 *  局所変数の定義
 */

static bool_t newline = false;
static int_t col = 0;
static int8_t line[LCD_COLUMN_SIZE + 1] = { "                " };

/*
 *  lcd_cls -- LCD 表示クリア
 */

void
lcd_cls (void)
{
	target_lcd_cls();
	col = 0;
	newline = false;
	}

/*
 *  lcd_newline -- LCD の表示を改行する。
 */

static void
lcd_newline (void)
{
	target_lcd_newline(line, col);
	col = 0;
	newline = false;
	}

/*
 *  lcd_initialize -- LCD 初期化
 */

void
lcd_initialize (intptr_t exinf)
{
	target_lcd_initialize();
	lcd_cls();
	}

/*
 *  lcd_putc -- LCD 出力
 */

void
lcd_putc (ID portid, int8_t data)
{
	if (data == '\n')
		newline = true;
	else {
		if (newline)
			lcd_newline();
		target_lcd_data_putc(data);
		line[col ++] = data;
		if (col >= LCD_COLUMN_SIZE)
			newline = true;
		}
	}

/*
 *  lcd_puts -- LCD への文字列の出力
 */

void
lcd_puts (ID portid, const char *str)
{
	while (*str != '\0')
		lcd_putc(portid, *str ++);
	}

/*
 *  LCD への書式付文字列出力ライブラリ
 */

/*
 *  数値変換のための変換表
 */

static const char radhex[] = "0123456789abcdef";
static const char radHEX[] = "0123456789ABCDEF";

/*
 *  もっとも長い整数型 (LONGEST) と符号なし整数型 (ULONGEST)
 */

#ifdef _int64_

typedef _int64_			LONGEST;
typedef unsigned _int64_	ULONGEST;

#else	/* of #ifdef _int64_ */

#ifdef _int32_

typedef _int32_			LONGEST;
typedef unsigned _int32_	ULONGEST;

#else	/* of #ifdef _int32_ */

typedef int_t			LONGEST;
typedef uint_t			ULONGEST;

#endif	/* of #ifdef _int32_ */

#endif	/* of #ifdef _int64_ */

/*
 *  convert -- lcd_printf の数値変換
 */

static int_t
convert (ID portid, ULONGEST val, int_t radix,
         const char *radchar, int_t width, bool_t minus, char padchar)
{
	char	digits[24];
	int_t	ix, pad, pchars;
	bool_t	left;

	if (width < 0) {
		width = -width;
		left = true;
		}
	else
		left = false;

	ix = 0;
	do {
		digits[ix ++] = radchar[val % radix];
		val /= radix;
		} while (val != 0);

	if (minus)
		digits[ix ++] = '-';

	if (width > ix)
		pchars = width;
	else
		pchars = ix;

	pad = ix;
	if (!left)	/* 右詰め */
		for ( ; pad < width; pad ++)
			lcd_putc(portid, padchar);

	while (ix -- > 0)
		lcd_putc(portid, digits[ix]);

	if (left)	/* 左詰め */
		for ( ; pad < width; pad ++)
			lcd_putc(portid, padchar);

	return pchars;
	}

#if defined(SUPPORT_INET4)

/*
 *  ipv4addr -- IPv4 アドレス出力
 */

static void
put_ipv4addr (ID portid, T_IN4_ADDR *addr, int_t width)
{
	int_t len = 3;	/* 3 は '.' の文字数 */

	len += convert(portid, (*addr >> 24) & 0xff, 10, radhex, 0, false, ' '); 
	lcd_putc(portid, '.');
	len += convert(portid, (*addr >> 16) & 0xff, 10, radhex, 0, false, ' '); 
	lcd_putc(portid, '.');
	len += convert(portid, (*addr >>  8) & 0xff, 10, radhex, 0, false, ' '); 
	lcd_putc(portid, '.');
	len += convert(portid,  *addr        & 0xff, 10, radhex, 0, false, ' '); 

	for ( ; len < width; len ++)
		lcd_putc(portid, ' ');
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 *  ipv6addr -- IPv6 アドレス出力
 */

static void
put_ipv6addr (ID portid, const T_IN6_ADDR *addr, int_t width)
{
	int_t	len = 0, ix;
	bool_t	omit = false, zero = false;

	if (addr == NULL || IN6_IS_ADDR_UNSPECIFIED(addr)) {
		lcd_putc(portid, ':');
		lcd_putc(portid, ':');
		}
	else {
		for (ix = 0; ix < sizeof(T_IN6_ADDR) / 2; ix ++) {
			if (omit) {
				if (ix < 7) {
					lcd_putc(portid, ':');
					len ++;
					}
				}
			else if (ix > 0 && ix < 7 && addr->s6_addr16[ix] == 0)
				zero = true;
			else {
				if (zero) {
					omit = true;
					lcd_putc(portid, ':');
					len ++;
					}
				if (ix < 7) {
					lcd_putc(portid, ':');
					len ++;
					}
				}
			}

		for ( ; len < width; len ++)
			lcd_putc(portid, ' ');
		}
	}

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  macaddr -- MAC アドレス出力
 */

static void
macaddr (ID portid, uint8_t *mac, int_t width)
{
	int_t oct, len;

	for (oct = 5; oct -- > 0; ) {
		convert(portid, *mac ++, 16, radhex, 2, false, '0'); 
		lcd_putc(portid, ':');
		}
	convert(portid, *mac, 16, radhex, 2, false, '0'); 

	for (len = 17; len < width; len ++)
		lcd_putc(portid, ' ');
	}

/*
 *  引数を取り出すためのマクロ
 */

#ifdef _int32_

#define GET_ARG(ap,lf)	 (lf ? va_arg(ap, _int32_) : va_arg(ap, int_t))

#else	/* of #ifdef _int32_ */

#define GET_ARG(ap,lf)	(va_arg(ap, int_t))

#endif	/* of #ifdef _int32_ */

/*
 *  lcd_printf -- シリアルポートへの書式付文字列出力
 */

void
lcd_printf (ID portid, const char *fmt, ...)
{
	va_list	ap;
	LONGEST	val;
	char	padchar, *str;
	int_t	ch, width, longflag, shortflag, left;

#if defined(SUPPORT_INET4)
	T_IN4_ADDR	*addr;
#endif	/* of #if defined(SUPPORT_INET4) */

	va_start(ap, fmt);
	while ((ch = *fmt ++) != '\0') {
		if (ch != '%') {		/* 書式指定以外 */
			lcd_putc(portid, (char)ch);
			continue;
			}

		width = longflag = shortflag = 0;
		padchar = ' ';

		if (ch == '-') {		/* 左詰め */
			fmt ++;
			left = -1;
			}
		else
			left = 1;

		if ((ch = *fmt ++) == '0') {	/* 上位桁の 0 */
			padchar = '0';
			ch = *fmt ++;
			}

		while ('0' <= ch && ch <= '9') {	/* 出力幅 */
			width = width * 10 + ch - '0';
			ch = *fmt ++;
			}

		while (ch == 'l') {		/* long (long) の指定 */
			longflag ++;
			ch = *fmt ++;
			}

		while (ch == 'h') {		/* short の指定 */
			shortflag ++;
			ch = *fmt ++;
			}

		switch (ch) {
		case 'd':
			val = GET_ARG(ap, longflag);
			if (val >= 0)
				convert(portid, val, 10, radhex, width * left, false, padchar);
			else
				convert(portid, -val, 10, radhex, width * left, true, padchar);
			break;

		case 'u':
			val = GET_ARG(ap, longflag);
			convert(portid, val, 10, radhex, width * left, false, padchar);
			break;

		case 'x':
			val = GET_ARG(ap, longflag);
			convert(portid, val, 16, radhex, width * left, false, padchar);
			break;

		case 'X':
			val = GET_ARG(ap, longflag);
			convert(portid, val, 16, radHEX, width * left, false, padchar);
			break;

		case 'c':
			ch = va_arg(ap, int_t);
			lcd_putc(portid, (char)ch);
			break;

		case 's':
			str = va_arg(ap, char*);
			while ((ch = *str ++) != '\0')
				lcd_putc(portid, (char)ch);
			break;

		case 'I':

			if (longflag) {

#if defined(SUPPORT_INET6)

				str = va_arg(ap, char*);
				put_ipv6addr(portid, (T_IN6_ADDR *)str, width);

#else	/* of #if defined(SUPPORT_INET6) */

				addr = va_arg(ap, T_IN4_ADDR *);
				put_ipv4addr(portid, addr, width);

#endif	/* of #if defined(SUPPORT_INET6) */

				}
			else if (shortflag) {

#if defined(SUPPORT_INET4)

				addr = va_arg(ap, T_IN4_ADDR *);
				put_ipv4addr(portid, addr, width);

#else	/* of #if defined(SUPPORT_INET4) */

				str = va_arg(ap, char*);
				put_ipv6addr(portid, (T_IN6_ADDR *)str, width);

#endif	/* of #if defined(SUPPORT_INET4) */

				}
			else {

#if defined(SUPPORT_INET6)

				str = va_arg(ap, char*);
				put_ipv6addr(portid, (T_IN6_ADDR *)str, width);

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

				addr = va_arg(ap, T_IN4_ADDR *);
				put_ipv4addr(portid, addr, width);

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

				}
			break;

		case 'M':
			str = va_arg(ap, char*);
			macaddr(portid, str, width);
			break;

		case '%':
			lcd_putc(portid, '%');
			break;

		case '0':
			fmt --;
			break;

		default:
			break;
			}

		}
	va_end(ap);
	}
