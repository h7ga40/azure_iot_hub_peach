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

#ifndef _NETAPP_H_
#define _NETAPP_H_

/*
 *  IPv4 と IPv6 の切換えマクロ
 */

#if defined(SUPPORT_INET6)

#define PUT_IPADDR(p,a,w)	put_ipv6addr(p,a,w)

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#define PUT_IPADDR(p,a,w)	put_ipv4addr(p,a,w)

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  コンソールに用いるシリアルポート番号
 */

#ifndef CONSOLE_PORTID
#define	CONSOLE_PORTID		LOGTASK_PORTID
#endif

/*
 *  コンソールに用いる LCD ポート番号
 */

#define	LCD_PORTID		0

/*
 *  数値変換のための変換表
 */

extern const char radhex[];
extern const char radHEX[];

/*
 *  関数
 */

extern char *skip_blanks (char *line);
extern int_t cons_getline (ID portid, char *line, int_t size);
extern char *get_int (int_t *val, char *line);
extern char *get_xuint (uint_t *val, char *line);
extern void cons_putchar (ID portid, char ch);
extern int_t cons_getchar (ID portid);
extern int_t cons_putnumber (ID portid, ulong_t val, int_t radix,
                           const char *radchar, int_t width,
                           bool_t minus, char padchar);
extern void cons_printf (ID portid, const char *fmt, ...);
extern int_t put_macaddr (ID portid, uint8_t *mac, int_t width);

#ifdef T_IN4_ADDR_DEFINED
extern int_t put_ipv4addr (ID portid, T_IN4_ADDR *addr, int_t width);
#endif
#ifdef T_IN6_ADDR_DEFINED
extern int_t put_ipv6addr (ID portid, const T_IN6_ADDR *addr, int_t width);
#endif

#endif	/* of #ifndef _NETAPP_H_ */
