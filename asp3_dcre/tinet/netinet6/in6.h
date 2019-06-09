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

/*	$FreeBSD: src/sys/netinet6/in6.h,v 1.7.2.4 2001/07/04 09:45:23 ume Exp $	*/
/*	$KAME: in6.h,v 1.89 2001/05/27 13:28:35 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)in.h	8.3 (Berkeley) 1/3/94
 */
 
#ifndef _IN6_H_
#define _IN6_H_

/*
 *  IPv6 アドレス構造
 *
 *    RFC4291（RFC1884、RFC2373、RFC3513 のアップデート）
 */

/*
 *  IPv6 アドレス
 */

struct t_in6_addr {
	union {
		uint8_t		__u6_addr8[16];
		uint16_t	__u6_addr16[8];
		uint32_t	__u6_addr32[4];
		} __u6_addr;
	};

#define s6_addr		__u6_addr.__u6_addr8
#define s6_addr8	__u6_addr.__u6_addr8
#define s6_addr16	__u6_addr.__u6_addr16
#define s6_addr32	__u6_addr.__u6_addr32

#ifndef T_IN6_ADDR_DEFINED

typedef struct t_in6_addr T_IN6_ADDR;

#define T_IN6_ADDR_DEFINED

#endif	/* of #ifndef T_IN6_ADDR_DEFINED */

/*
 *  IPv6 アドレスの初期化データ
 */

#define IPV6_ADDR_UNSPECIFIED_INIT	\
	{{{ UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00) }}}

#define IPV6_ADDR_LINKLOCAL_ALLNODES_INIT	\
	{{{ UINT_C(0xff), UINT_C(0x02), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x01) }}}

#define IPV6_ADDR_LINKLOCAL_ALLROUTERS_INIT	\
	{{{ UINT_C(0xff), UINT_C(0x02), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x02) }}}

/*
 *  特別なアドレスのチェック
 */

/* 無指定 */

#define IN6_IS_ADDR_UNSPECIFIED(a)	\
	(memcmp((void *)(a), (void *)&in6_addr_unspecified, sizeof(T_IN6_ADDR)) == 0)

/* ループバック */

#define IN6_IS_ADDR_LOOPBACK(a)		\
	(memcmp((void *)(a), (void *)&in6_addr_unspecified, sizeof(T_IN6_ADDR) - 1) == 0 && \
	 (a)->s6_addr8[15] == 0x01)

/*
 *  スコープ ID
 */

#define IPV6_ADDR_SCOPE_NODELOCAL	UINT_C(0x01)
#define IPV6_ADDR_SCOPE_LINKLOCAL	UINT_C(0x02)
#define IPV6_ADDR_SCOPE_SITELOCAL	UINT_C(0x05)
#define IPV6_ADDR_SCOPE_ORGLOCAL	UINT_C(0x08)
#define IPV6_ADDR_SCOPE_GLOBAL		UINT_C(0x0e)

/*
 *  ユニキャスト・スコープ
 *
 *  注意: 上位 10 ビットのみチェックしなければならない。RFC2373 参照
 */

/* リンクローカル */

#define IN6_IS_ADDR_LINKLOCAL(a)	\
	(((a)->s6_addr8[0] == UINT_C(0xfe)) && (((a)->s6_addr8[1] & UINT_C(0xc0)) == UINT_C(0x80)))

/* サイトローカル */

#define IN6_IS_ADDR_SITELOCAL(a)	\
	(((a)->s6_addr8[0] == UINT_C(0xfe)) && (((a)->s6_addr8[1] & UINT_C(0xc0)) == UINT_C(0xc0)))

/* 集約可能ユニキャスト (グローバル) */

#define IN6_IS_ADDR_AGGREGATE_UNICAST(a)	\
	(((a)->s6_addr8[0] & UINT_C(0xe0)) == UINT_C(0x20))

/*
 *  マルチキャスト
 */

#define IN6_IS_ADDR_MULTICAST(a)	((a)->s6_addr8[0] == UINT_C(0xff))

/*
 *  同一のチェック
 */

#define IN6_ARE_ADDR_EQUAL(a,b)		(memcmp((a), (b), sizeof(T_IN6_ADDR)) == 0)

/*
 *  経路制御用定義
 */

#define IPV6_RTHDR_TYPE_0		UINT_C(0)	/* IPv6 経路制御ヘッダタイプ 0	*/

/*
 *  規定値と制限値
 */

#define IPV6_DEFAULT_MULTICAST_HOPS	1	/* マルチキャスト時のホップリミットの規定値	*/
#define ND6_INFINITE_LIFETIME		0xffffffff	/* アドレスの無制限有効時間		*/

/*
 *  ITRON TCP/IPv6 アドレス/ポート番号の定義
 */

typedef struct t_ipv6ep {
	T_IN6_ADDR	ipaddr;		/* IPv6 アドレス	*/
	uint16_t	portno;		/* ポート番号	*/
	} T_IPV6EP;

#define T_IPV6EP_DEFINED

/*
 *  ITRON/TCP/IP 用 IP アドレスの定義
 */

#define IPV6_ADDRANY		IPV6_ADDR_UNSPECIFIED_INIT

/*
 *  動的生成用 IPv6 TCP 受付口
 */

typedef struct t_tcp6_crep {
	/* 標準 */
	ATR		repatr;		/* 受付口属性		*/
	T_IPV6EP	myaddr;		/* 自分のアドレス	*/
	/* 実装依存 */
	} T_TCP6_CREP;

/*
 *  動的生成用 IPv6 UDP 通信端点
 */

typedef struct t_udp6_ccep {
	/* 標準 */
	ATR		cepatr;		/* UDP 通信端点属性		*/
	T_IPV6EP	myaddr;		/* 自分のアドレス		*/
	FP		callback;	/* コールバック関数		*/
	/* 実装依存 */
	} T_UDP6_CCEP;

/*
 *  前方参照
 */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

/*
 *  全域変数
 */

extern const T_IN6_ADDR in6_addr_unspecified;
extern const T_IN6_ADDR in6_addr_linklocal_allnodes;
extern const T_IN6_ADDR in6_addr_linklocal_allrouters;

#define ipv6_addrany	in6_addr_unspecified

/*
 *  TINET 独自 API
 */

extern char *ipv62str (char *buf, const T_IN6_ADDR *p_addr);
extern uint_t in6_get_maxnum_ifaddr (void);
extern const T_IN6_ADDR *in6_get_ifaddr (int_t index);
extern ER in6_upd_ifaddr (T_IN6_ADDR *addr, uint_t prefixlen,
                          uint32_t vltime, uint32_t pltime);
extern ER in6_del_ifaddr (T_IN6_ADDR *addr);
extern T_IN6_ADDR *in6_make_ipv4mapped (T_IN6_ADDR *dst, T_IN4_ADDR src);
extern bool_t in6_is_addr_ipv4mapped (const T_IN6_ADDR *addr);

#endif	/* of #ifndef _IN6_H_ */
