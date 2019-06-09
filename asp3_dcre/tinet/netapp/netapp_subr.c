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

/* 
 *  ネットワーク応用プログラムサポートルーチン
 */

#include <stdarg.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
#include <target_syssvc.h>
#define LOGTASK_PORTID SIO_PORTID
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/net.h>
#include <net/net_endian.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/resolver.h>

#ifdef USE_NETAPP_SUBR

/*
 *  IPv6 と IPv4 で引数が異なる関数のコンパイル
 */

#if defined(SUPPORT_INET6)

#ifdef USE_TCP_EXTENTIONS

const ID vrid_tcp6_cep[] =
{
#if NUM_VRID_TCP6_CEPS >= 1
	TCP6_RSV_CEPID1,
#endif
#if NUM_VRID_TCP6_CEPS >= 2
	TCP6_RSV_CEPID2,
#endif
#if NUM_VRID_TCP6_CEPS >= 3
	TCP6_RSV_CEPID3,
#endif
#if NUM_VRID_TCP6_CEPS >= 4
	TCP6_RSV_CEPID4,
#endif
	};

const ID vrid_tcp6_rep[] =
{
#if NUM_VRID_TCP6_REPS >= 1
	TCP6_RSV_REPID1,
#endif
#if NUM_VRID_TCP6_REPS >= 2
	TCP6_RSV_REPID2,
#endif
	};

ID tskid_tcp6_cep[NUM_VRID_TCP6_CEPS];
ID tskid_tcp6_rep[NUM_VRID_TCP6_REPS];

#endif	/* #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

const ID vrid_udp6_cep[] =
{
#if NUM_VRID_UDP6_CEPS >= 1
	UDP6_RSV_CEPID1,
#endif
#if NUM_VRID_UDP6_CEPS >= 2
	UDP6_RSV_CEPID2,
#endif
	};

ID tskid_udp6_cep[NUM_VRID_UDP6_CEPS];

#endif	/* #ifdef USE_UDP_EXTENTIONS */

#undef	TCP_CRE_REP
#undef	UDP_CRE_CEP
#undef	UDP_DEL_CEP
#undef	TCP_IS_CEPID
#undef  ALLOC_TCP_REP
#undef  ALLOC_TCP_CEP
#undef  ALLOC_UDP_CEP
#undef  FREE_TCP_REP
#undef  FREE_TCP_CEP
#undef  FREE_UDP_CEP
#undef	T_TCPN_CREP
#undef	T_UDPN_CCEP
#undef	NUM_VRID_TCP_CEPS
#undef	NUM_VRID_TCP_REPS
#undef	NUM_VRID_UDP_CEPS
#undef	VRID_TCP_CEP
#undef	VRID_UDP_CEP
#undef	VRID_TCP_REP
#undef	TSKID_TCP_CEP
#undef	TSKID_TCP_REP
#undef	TSKID_UDP_CEP
#undef	API_IPPROTO

#define TCP_CRE_REP		tcp6_cre_rep
#define UDP_CRE_CEP		udp6_cre_cep
#define UDP_DEL_CEP		udp6_del_cep

#define TCP_IS_CEPID		tcp6_is_cepid
#define ALLOC_TCP_REP		alloc_tcp6_rep
#define ALLOC_TCP_CEP		alloc_tcp6_cep
#define ALLOC_UDP_CEP		alloc_udp6_cep
#define FREE_TCP_REP		free_tcp6_rep
#define FREE_TCP_CEP		free_tcp6_cep
#define FREE_UDP_CEP		free_udp6_cep
#define T_TCPN_CREP		T_TCP6_CREP
#define T_UDPN_CCEP		T_UDP6_CCEP
#define NUM_VRID_TCP_CEPS	NUM_VRID_TCP6_CEPS
#define NUM_VRID_TCP_REPS	NUM_VRID_TCP6_REPS
#define NUM_VRID_UDP_CEPS	NUM_VRID_UDP6_CEPS
#define VRID_TCP_CEP		vrid_tcp6_cep
#define VRID_TCP_REP		vrid_tcp6_rep
#define VRID_UDP_CEP		vrid_udp6_cep
#define TSKID_TCP_CEP		tskid_tcp6_cep
#define TSKID_TCP_REP		tskid_tcp6_rep
#define TSKID_UDP_CEP		tskid_udp6_cep
#define API_IPPROTO		API_PROTO_IPV6

#include <netapp/netappn_subr.c>

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#ifdef USE_TCP_EXTENTIONS

const ID vrid_tcp4_cep[] =
{
#if NUM_VRID_TCP4_CEPS >= 1
	TCP4_RSV_CEPID1,
#endif
#if NUM_VRID_TCP4_CEPS >= 2
	TCP4_RSV_CEPID2,
#endif
#if NUM_VRID_TCP4_CEPS >= 3
	TCP4_RSV_CEPID3,
#endif
#if NUM_VRID_TCP4_CEPS >= 4
	TCP4_RSV_CEPID4,
#endif
	};

const ID vrid_tcp4_rep[] =
{
#if NUM_VRID_TCP4_REPS >= 1
	TCP4_RSV_REPID1,
#endif
#if NUM_VRID_TCP4_REPS >= 2
	TCP4_RSV_REPID2,
#endif
	};

ID tskid_tcp4_cep[NUM_VRID_TCP4_CEPS];
ID tskid_tcp4_rep[NUM_VRID_TCP4_REPS];

#endif	/* #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

const ID vrid_udp4_cep[] =
{
#if NUM_VRID_UDP4_CEPS >= 1
	UDP4_RSV_CEPID1,
#endif
#if NUM_VRID_UDP4_CEPS >= 2
	UDP4_RSV_CEPID2,
#endif
	};

ID tskid_udp4_cep[NUM_VRID_UDP4_CEPS];

#endif	/* #ifdef USE_UDP_EXTENTIONS */

#undef	TCP_CRE_REP
#undef	UDP_CRE_CEP
#undef	UDP_DEL_CEP
#undef	TCP_IS_CEPID
#undef  ALLOC_TCP_REP
#undef  ALLOC_TCP_CEP
#undef  ALLOC_UDP_CEP
#undef  FREE_TCP_REP
#undef  FREE_TCP_CEP
#undef  FREE_UDP_CEP
#undef	T_TCPN_CREP
#undef	T_UDPN_CCEP
#undef	NUM_VRID_TCP_CEPS
#undef	NUM_VRID_TCP_REPS
#undef	NUM_VRID_UDP_CEPS
#undef	VRID_TCP_CEP
#undef	VRID_TCP_REP
#undef	VRID_UDP_CEP
#undef	TSKID_TCP_CEP
#undef	TSKID_TCP_REP
#undef	TSKID_UDP_CEP
#undef	API_IPPROTO

#define TCP_CRE_REP		tcp_cre_rep
#define UDP_CRE_CEP		udp_cre_cep
#define UDP_DEL_CEP		udp_del_cep
#define TCP_IS_CEPID		tcp4_is_cepid
#define ALLOC_TCP_REP		alloc_tcp4_rep
#define ALLOC_TCP_CEP		alloc_tcp4_cep
#define ALLOC_UDP_CEP		alloc_udp4_cep
#define FREE_TCP_REP		free_tcp4_rep
#define FREE_TCP_CEP		free_tcp4_cep
#define FREE_UDP_CEP		free_udp4_cep
#define T_TCPN_CREP		T_TCP_CREP
#define T_UDPN_CCEP		T_UDP_CCEP
#define NUM_VRID_TCP_CEPS	NUM_VRID_TCP4_CEPS
#define NUM_VRID_TCP_REPS	NUM_VRID_TCP4_REPS
#define NUM_VRID_UDP_CEPS	NUM_VRID_UDP4_CEPS
#define VRID_TCP_CEP		vrid_tcp4_cep
#define VRID_TCP_REP		vrid_tcp4_rep
#define VRID_UDP_CEP		vrid_udp4_cep
#define TSKID_TCP_CEP		tskid_tcp4_cep
#define TSKID_TCP_REP		tskid_tcp4_rep
#define TSKID_UDP_CEP		tskid_udp4_cep
#define API_IPPROTO		API_PROTO_IPV4

#include <netapp/netappn_subr.c>

#endif	/* of #if defined(SUPPORT_INET4) */

#undef	TCP_CRE_REP
#undef  ALLOC_TCPN_REP
#undef  ALLOC_TCPN_CEP
#undef  ALLOC_UDPN_CEP
#undef  FREE_TCPN_REP
#undef  FREE_TCPN_CEP
#undef  FREE_UDPN_CEP
#undef	T_TCPN_CREP
#undef	NUM_VRID_TCP_REPS
#undef	NUM_VRID_UDP_CEPS
#undef	VRID_TCP_CEP
#undef	VRID_TCP_REP
#undef	VRID_UDP_CEP
#undef	TSKID_TCP_CEP
#undef	TSKID_TCP_REP
#undef	TSKID_UDP_CEP

/*
 *  変数
 */

static uint32_t rand_next = ULONG_C(1);

#if !defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

const T_IN6_ADDR in6_addr_unspecified =
	IPV6_ADDR_UNSPECIFIED_INIT;

#endif	/* of #if !defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  シリアルポートへの書式付文字列出力ライブラリ
 */

#define EOF		(-1)

/*
 *  数値変換のための変換表
 */

const char radhex[] = "0123456789abcdef";
const char radHEX[] = "0123456789ABCDEF";

/*
 *  netapp_rand -- 乱数を返す。
 */

uint32_t
netapp_rand (void)
{
	rand_next = (rand_next * 99991 + 12345) & ULONG_C(0x7fffffff);
	return rand_next;
	}

/*
 *  srand -- 乱数を初期化する。
 */

void
netapp_srand (uint32_t seed)
{
	SYSTIM now;

	syscall(get_tim(&now));
	rand_next += now + seed;
	}

#ifndef USE_NET_CONS

/*
 *  cons_putchar -- シリアルポートへの文字出力
 */

void
cons_putchar (ID portid, char ch)
{
	serial_wri_dat(portid, &ch, 1);
	}

/*
 *  cons_getchar -- シリアルポートからの文字入力
 */

int_t
cons_getchar (ID portid)
{
	char ch;

	if (serial_rea_dat(portid, &ch, sizeof(ch)) > 0)
		return ch;
	else
		return EOF;
	}

#endif	/* of #ifndef USE_NET_CONS */

/*
 *  cons_putnumber -- cons_printf の数値変換
 */

int_t
cons_putnumber(ID portid, ulong_t 	val, int_t radix,
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
			cons_putchar(portid, padchar);

	while (ix -- > 0)
		cons_putchar(portid, digits[ix]);

	if (left)	/* 左詰め */
		for ( ; pad < width; pad ++)
			cons_putchar(portid, padchar);

	return pchars;
	}

/*
 *  put_ipv4addr -- IPv4 アドレス出力
 */

int_t
put_ipv4addr (ID portid, T_IN4_ADDR *addr, int_t width)
{
	int_t len = 3;	/* 3 は '.' の文字数 */

	len += cons_putnumber(portid, (*addr >> 24) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid, (*addr >> 16) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid, (*addr >>  8) & 0xff, 10, radhex, 0, false, ' '); 
	cons_putchar(portid, '.');
	len += cons_putnumber(portid,  *addr        & 0xff, 10, radhex, 0, false, ' '); 

	for ( ; len < width; len ++)
		cons_putchar(portid, ' ');

	return len;
	}

/*
 *  ipv6addr -- IPv6 アドレス出力
 */

int_t
put_ipv6addr (ID portid, const T_IN6_ADDR *addr, int_t width)
{
	int_t	len = 0, ix, len6;
	bool_t	omit = false, zero = false;

	if (addr == NULL || IN6_IS_ADDR_UNSPECIFIED(addr)) {
		cons_putchar(portid, '0');
		cons_putchar(portid, ':');
		cons_putchar(portid, ':');
		cons_putchar(portid, '0');
		len = 4;
		}
	else {
		if (in6_is_addr_ipv4mapped(addr))
			len6 = sizeof(T_IN6_ADDR) / 2 - 2;
		else
			len6 = sizeof(T_IN6_ADDR) / 2;
		for (ix = 0; ix < len6; ix ++) {
			if (omit) {
				len += cons_putnumber(portid, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					cons_putchar(portid, ':');
					len ++;
					}
				}
			else if (ix > 0 && ix < 7 && addr->s6_addr16[ix] == 0)
				zero = true;
			else {
				if (zero) {
					omit = true;
					cons_putchar(portid, ':');
					len ++;
					}
				len += cons_putnumber(portid, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					cons_putchar(portid, ':');
					len ++;
					}
				}
			}

		if (len6 == sizeof(T_IN6_ADDR) / 2 - 2) {
			T_IN4_ADDR ip4addr;

			ip4addr = ntohl(addr->s6_addr32[3]);
			len += put_ipv4addr(portid, &ip4addr, 0);
			}

		for ( ; len < width; len ++)
			cons_putchar(portid, ' ');
		}
	return len;
	}

/*
 *  put_macaddr -- MAC アドレス出力
 */

int_t
put_macaddr (ID portid, uint8_t *mac, int_t width)
{
	int_t oct, len;

	for (oct = 5; oct -- > 0; ) {
		cons_putnumber(portid, *mac ++, 16, radhex, 2, false, '0'); 
		cons_putchar(portid, ':');
		}
	cons_putnumber(portid, *mac, 16, radhex, 2, false, '0'); 

	for (len = 17; len < width; len ++)
		cons_putchar(portid, ' ');

	return len;
	}

/*
 *  cons_printf -- シリアルポートへの書式付文字列出力
 */

void
cons_printf (ID portid, const char *fmt, ...)
{
	va_list	ap;
	long_t	val;
	char	padchar, *str;
	int_t	ch, width, longflag, shortflag, left;
	T_IN4_ADDR	*addr;

	va_start(ap, fmt);
	while ((ch = *fmt ++) != '\0') {
		if (ch != '%') {		/* 書式指定以外 */
			cons_putchar(portid, (char)ch);
			continue;
			}

		width = longflag = shortflag = 0;
		padchar = ' ';

		if (*fmt == '-') {		/* 左詰め */
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
			val = longflag ? (ulong_t)va_arg(ap, long_t)
			               : (ulong_t)va_arg(ap, int_t);
			if (val >= 0)
				cons_putnumber(portid,  val, 10, radhex, width * left, false, padchar);
			else
				cons_putnumber(portid, -val, 10, radhex, width * left, true, padchar);
			break;

		case 'u':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 10, radhex, width * left, false, padchar);
			break;

		case 'x':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 16, radhex, width * left, false, padchar);
			break;

		case 'X':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			cons_putnumber(portid, val, 16, radHEX, width * left, false, padchar);
			break;

		case 'c':
			ch = va_arg(ap, int_t);
			cons_putchar(portid, (char)ch);
			break;

		case 's':
			str = va_arg(ap, char*);
			while ((ch = *str ++) != '\0') {
				cons_putchar(portid, (char)ch);
				width --;
				}
			while (width -- > 0)
				cons_putchar(portid, ' ');
			break;

		case 'I':

			if (longflag) {
				str = va_arg(ap, char*);
				put_ipv6addr(portid, (T_IN6_ADDR *)str, width);
				}
			else if (shortflag) {
				addr = va_arg(ap, T_IN4_ADDR *);
				put_ipv4addr(portid, addr, width);
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
			put_macaddr(portid, (uint8_t *)str, width);
			break;

		case '%':
			cons_putchar(portid, '%');
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

/*
 *  host2msg16 -- 16ビットの値を、バイトオーダーを調整してメッセージに書き込む。
 */

void
host2msg16 (void *msg, uint16_t host_data)
{
	uint16_t	msg_data;

	msg_data = htons(host_data);
	memcpy(msg, &msg_data, sizeof(msg_data));
	}

/*
 *  host2msg32 -- 32ビットの値を、バイトオーダーを調整してメッセージに書き込む。
 */

void
host2msg32 (void *msg, uint32_t host_data)
{
	uint32_t	msg_data;

	msg_data = htonl(host_data);
	memcpy(msg, &msg_data, sizeof(msg_data));
	}

/*
 *  msg2host16 -- 16ビットの値を、バイトオーダーを調整してメッセージから読み出す。
 */

uint16_t
msg2host16 (void *msg)
{
	uint16_t	host_data;

	memcpy(&host_data, msg, sizeof(host_data));
	return htons(host_data);
	}

/*
 *  msg2host32 -- 32ビットの値を、バイトオーダーを調整してメッセージから読み出す。
 */

uint32_t
msg2host32 (void *msg)
{
	uint32_t	host_data;

	memcpy(&host_data, msg, sizeof(host_data));
	return htonl(host_data);
	}

/*
 *  skip_blanks -- 空白と TAB をスキップする。
 */

char *
skip_blanks (char *line)
{
	while (*line == ' ' || *line == '\t')
		line ++;
	return line;
	}

/*
 *  get_ipv4addr -- 文字列の IPv4 アドレスを T_IN4_ADDR 値に変換する。
 */

char *
get_ipv4addr (T_IN4_ADDR *addr, char *line)
{
	int_t oct;

	*addr = IPV4_ADDRANY;
	while ('0' <= *line && *line <= '9') {
		oct = 0;
		while ('0' <= *line && *line <= '9')
			oct = oct * 10 + (*line ++) - '0';
		*addr = (*addr << 8) | (oct & 0xff);
		if (*line == '.')
			line ++;
		}
#if 1
	return *addr == IPV4_ADDRANY ? NULL : line;
#else
	return line;
#endif
	}

#if defined(SUPPORT_INET6) || defined(USE_RESOLVER)

/*
 *  get_ipv6addr -- 文字列の IPv6 アドレスを T_IN6_ADDR 値に変換する。
 */

char *
get_ipv6addr (T_IN6_ADDR *addr, char *line)
{
	int_t word, ix = 0, omit = 0, six;
	char *ipv4line;

	memset(addr, 0, sizeof(T_IN6_ADDR));

	/*
	 * IPv4 アドレス（数字.）を検出したら IPv4 アドレスとして入力し、
	 * IPv4 射影アドレスに変換する。
	 */
	for (ipv4line = line; '0' <= *ipv4line && *ipv4line <= '9'; ipv4line ++)
		;
	if (*ipv4line == '.') {
		T_IN4_ADDR ipv4addr;

		line = get_ipv4addr(&ipv4addr, line);
		in6_make_ipv4mapped (addr, ipv4addr);
		}
	else {
		while (ix < 8 &&
		       (('0' <= *line && *line <= '9') ||
		        ('a' <= *line && *line <= 'f') ||
		        ('A' <= *line && *line <= 'F'))) {
			word = 0;
			while (('0' <= *line && *line <= '9') ||
 		               ('a' <= *line && *line <= 'f') ||
		               ('A' <= *line && *line <= 'F')) {
				if      ('0' <= *line && *line <= '9')
					word = (word << 4) + (*line ++) - '0';
				else if ('a' <= *line && *line <= 'f')
					word = (word << 4) + (*line ++) - 'a' + 10;
				else if ('A' <= *line && *line <= 'F')
					word = (word << 4) + (*line ++) - 'A' + 10;
				}
			addr->s6_addr16[ix ++] = htons(word);
			if (*line == ':') {
				line ++;
				if (*line == ':') {
					omit = ix;
					line ++;
					}
				}
			}
		if (omit > 0) {
			six = 7;
			while (ix > omit) {
				addr->s6_addr16[six --] = addr->s6_addr16[-- ix];
				addr->s6_addr16[ ix   ] = 0;
				}
			}
		}
	return line;
	}

#endif	/* of #if defined(SUPPORT_INET6) || defined(USE_RESOLVER) */

/*
 *  デバッグ行入力
 */

int_t
cons_getline (ID portid, char *line, int_t size)
{
	int_t	ch, len;

	len = size;
	while (size > 0) {
		if ((ch = cons_getchar(portid)) != EOF) {
			if (ch == '\r') {
				cons_putchar(portid, '\n');
				break;
				}
			else if (ch == '\b') {
				if (size < len) {
					line --;
					size ++;
					}
				}
			else {
				*line ++ = ch;
				size --;
				}
			}
		}
	*line = '\0';
	return len - size;
	}

/*
 *  get_int -- 整数変換
 */

char *
get_int (int_t *val, char *line)
{
	int_t sign = 1;

	line = skip_blanks(line);
	if (*line == '-') {
		sign = -1;
		line ++;
		}
	else if (*line == '+')
		line ++;

	line = skip_blanks(line);
	*val = 0;
	while ('0' <= *line && *line <= '9') {
		*val = *val * 10 + *line - '0';
		line ++;
		}

	*val = *val * sign;
	return line;
	}

/*
 *  get_xuint -- 符号なし整数 (16進) 変換
 */

char *
get_xuint (uint_t *val, char *line)
{
	line = skip_blanks(line);
	*val = 0;
	while (1) {
		if      ('0' <= *line && *line <= '9')
			*val = (*val << 4) + *line - '0';
		else if ('a' <= *line && *line <= 'f')
			*val = (*val << 4) + *line - 'a' + 10;
		else if ('A' <= *line && *line <= 'F')
			*val = (*val << 4) + *line - 'A' + 10;
		else
			break;
		line ++;
		}
	return line;
	}

#ifdef USE_RESOLVER

/*
 *  dns_strtype -- DNS の type の文字列を返す。
 */

const char *
dns_strtype (uint_t type)
{
	switch (type) {
	case DNS_TYPE_A:
		return "A";
		break;
	case DNS_TYPE_NS:
		return "NS";
		break;
	case DNS_TYPE_CNAME:
		return "CNAME";
		break;
	case DNS_TYPE_SOA:
		return "SOA";
		break;
	case DNS_TYPE_PTR:
		return "PTR";
		break;
	case DNS_TYPE_AAAA:
		return "AAAA";
		break;
	default:
		return "unknown type";
		break;
		}
	}

/*
 *  dns_strclass -- DNS の class を表示する。
 */

const char *
dns_strclass (uint_t class)
{
	switch (class) {
	case DNS_CLASS_IN:
		return "IN";
		break;
	default:
		return "unknown class";
		break;
		}
	}

/*
 *  resolv_hoststr -- 文字列のFQDN・ホスト名・IPv6/IPv4 アドレスを解析する。
 */

#define IS_HOSTCH_DIGIT(c)	(('0'<=(c)&&(c)<='9'))
#define IS_HOSTCH_XDIGIT(c)	(IS_HOSTCH_DIGIT(c)||('a'<=(c)&&(c)<='f')||('A'<=(c)&&(c)<='F'))
#define IS_HOSTCH_ALPHA(c)	(('a'<=(c)&&(c)<='z')||('A'<=(c)&&(c)<='z'))
#define IS_HOSTCH_ALNUM(c)	(IS_HOSTCH_ALPHA(c)||IS_HOSTCH_DIGIT(c))
#define IS_HOSTCH_IPV6ADDR(c)	(IS_HOSTCH_XDIGIT(c)||(c)==':')
#define IS_HOSTCH_IPV4ADDR(c)	(IS_HOSTCH_DIGIT(c)||(c)=='.')
#define IS_HOSTCH_IPADDR(c)	(IS_HOSTCH_XDIGIT(c)||(c)==':'||(c)=='.')
#define IS_HOSTCH_NAME_FIRST(c)	(IS_HOSTCH_ALPHA(c))
#define IS_HOSTCH_NAME(c)	(IS_HOSTCH_ALNUM(c)||(c)=='-'||(c)=='.')
#define IS_HOSTCH_BLANK(c)	((c)=='\0'||(c)==' '||(c)=='\t')

char *
resolv_hoststr (uint_t *flags, char *hostname, uint_t name_size, char *line)
{
	char *h, *p;

	h = hostname;
	p = line = skip_blanks(line);

	if (IS_HOSTCH_IPADDR(*p)) {
		while (IS_HOSTCH_DIGIT(*p)) {
			if (h - hostname > name_size)
				return NULL;
			*h ++ = *p ++;
			}
		if (p > line && *p == '.') {
			while (IS_HOSTCH_IPV4ADDR(*p)) {
				if (h - hostname > name_size)
					return NULL;
				*h ++ = *p ++;
				}
			if (IS_HOSTCH_BLANK(*p)) {
				*flags |= HOSTSTR_IPV4;
				*h = '\0';
				return p;
				}
			}
		else if (IS_HOSTCH_IPADDR(*p)) {
			while (IS_HOSTCH_IPADDR(*p)) {
				if (h - hostname > name_size)
					return NULL;
				*h ++ = *p ++;
				}
			if (IS_HOSTCH_BLANK(*p)) {
				*flags |= HOSTSTR_IPV6;
				*h = '\0';
				return p;
				}
			}
		h = hostname;
		p = line;
		}

	if (IS_HOSTCH_NAME_FIRST(*p)) {
		*flags |= HOSTSTR_HOSTNAME;
		while (IS_HOSTCH_NAME(*p)) {
			if (h - hostname > name_size)
				return NULL;
			if (*p == '.')
				*flags |= HOSTSTR_FQDN;
			*h ++ = *p ++;
			}
		}
	else
		*flags |= HOSTSTR_OTHER;

	*h = '\0';
	return p;
	}

/*
 *  show_dns_domain_name -- DNS のドメイン名を表示する。
 */

uint_t
show_dns_domain_name (ID portid, uint8_t *hdr, uint_t offset)
{
	uint8_t	*ptr;
	uint_t	c;

	ptr = hdr + offset;
	while (*ptr) {
		if ((*ptr & DNS_MSG_COMP_MARK) == DNS_MSG_COMP_MARK) {
			show_dns_domain_name(portid, hdr, (*ptr & ~DNS_MSG_COMP_MARK) << 8 | *(ptr + 1));
			ptr += 2;
			break;
			}
		else {
			for (c = 1; c <= *ptr; c++)
				cons_printf(portid, "%c", *(ptr + c));
			ptr += *ptr + 1;
			if (*ptr)
				cons_printf(portid, ".");
			}
		}
	return ptr - hdr;
	}

/*
 *  resolv_options -- アドレス解決のオプション解析
 */

char *
resolv_options (uint_t *flags, char *line, char apip)
{
#if defined(SUPPORT_INET6)
	T_IN6_ADDR	in6_addr;
#endif

#if defined(SUPPORT_INET4)
	T_IN4_ADDR	in4_addr;
#endif

	line = skip_blanks(line);

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

	*flags |= DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_PROTO_IPV4;
	if (apip == API_PROTO_IPV4)
		*flags |= DNS_LUP_FLAGS_QTYPE_A;
	else
		*flags |= DNS_LUP_FLAGS_QTYPE_AAAA;

	if (*line == '-') {
		line ++;
		if (*line == '6') {
			line ++;
			*flags &= ~DNS_LUP_FLAGS_PROTO_IPV4;
			}
		else if (*line == '4') {
			line ++;
			*flags &= ~DNS_LUP_FLAGS_PROTO_IPV6;
			}
		if (*line == 'Q' || *line == 'q') {
			*flags &= ~DNS_LUP_FLAGS_QTYPE_A;
			line ++;                   
			}
		else if (*line == 'A' || *line == 'a') {
			*flags &= ~DNS_LUP_FLAGS_QTYPE_AAAA;
			line ++;                   
			}
		while (*line && !(*line == ' ' || *line == '\t'))
			line ++;
		}

#else	/* of #if defined(SUPPORT_INET4) */

	*flags = DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_QTYPE_AAAA;
	if (*line == '-') {
		line ++;
		while (*line && !(*line == ' ' || *line == '\t'))
			line ++;
		}

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

	*flags = DNS_LUP_FLAGS_PROTO_IPV4 | DNS_LUP_FLAGS_QTYPE_A;
	if (*line == '-') {
		line ++;
		while (*line && !(*line == ' ' || *line == '\t'))
			line ++;
		}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET6)

	if (*flags | DNS_LUP_FLAGS_PROTO_IPV6) {
		/*
		 *  IPv6 で DNS サーバに照会することが指定されても、
		 *  サーバの IPv6 アドレスが未定義の時は、指定を外す。
		 */
		dns_in6_get_addr(&in6_addr);
		if (IN6_IS_ADDR_UNSPECIFIED(&in6_addr))
			*flags &= ~DNS_LUP_FLAGS_PROTO_IPV6;
		}

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

	if (*flags | DNS_LUP_FLAGS_PROTO_IPV4) {
		/*
		 *  IPv4 で DNS サーバに照会することが指定されても、
		 *  サーバの IPv4 アドレスが未定義の時は、指定を外す。
		 */
		dns_in4_get_addr(&in4_addr);
		if (in4_addr == IPV4_ADDRANY)
			*flags &= ~DNS_LUP_FLAGS_PROTO_IPV4;
		}

#endif	/* of #if defined(SUPPORT_INET4) */

	return line;
	}

#endif	/* of #ifdef USE_RESOLVER */

/*
 *  lookup_ipaddr -- ホスト名・リテラルのIPアドレスをIP アドレスに変換する。
 */

char *
lookup_ipaddr (T_IN_ADDR *addr, char *line, char apip)
{

#ifdef USE_RESOLVER

	static char hostname[DBG_LINE_SIZE + 1];

	ER_UINT	error;
	uint_t	flags = 0;
	char	*last;

	line = skip_blanks(resolv_options(&flags, line, apip));
	if ((last = resolv_hoststr(&flags, hostname, sizeof(hostname), line)) == NULL) {
		cons_printf(CONSOLE_PORTID, "unknown host: %s.\n", hostname);
		return NULL;
		}

	switch (flags & DNS_LUP_FLAGS_NAME_MASK) {
	case DNS_LUP_FLAGS_NAME_IPV4:
	case DNS_LUP_FLAGS_NAME_IPV6:
		line = GET_IPADDR(addr, line);		/* IP Address by Literal */
		break;

	case DNS_LUP_FLAGS_NAME_HOST:
	case DNS_LUP_FLAGS_NAME_FQDN:
		line = last;
		if ((flags & (DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_PROTO_IPV4)) == 0) {
			cons_printf(CONSOLE_PORTID, "DNS server not available.\n");
			return NULL;
			}

		flags |= DNS_LUP_OPCODE_FORWARD;
		if ((error = dns_host_addr(flags, hostname, addr)) != E_OK) {
			cons_printf(CONSOLE_PORTID, "error: %s.\n", itron_strerror(error));
			return NULL;
			}
		break;

		}

	return line;

#else	/* of #ifdef USE_RESOLVER */

	return GET_IPADDR(addr, skip_blanks(line));		/* IP Address by Literal */

#endif	 /* of #ifdef USE_RESOLVER */

	}

#endif	/* of #ifdef USE_NETAPP_SUBR */
