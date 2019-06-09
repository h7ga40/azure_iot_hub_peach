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
 * $FreeBSD: src/sys/netinet/in.h,v 1.38.2.3 1999/08/29 16:29:34 peter Exp $
 */
 
#ifndef _IN4_H_
#define _IN4_H_

/*
 *  IPv4 アドレス
 */

/* 前方参照 */

#ifndef T_IN4_ADDR_DEFINED

typedef uint32_t T_IN4_ADDR;

#define T_IN4_ADDR_DEFINED

#endif	/* of #ifndef T_IN4_ADDR_DEFINED */

/*
 *  ITRON TCP/IPv4 アドレス/ポート番号の定義
 */

typedef struct t_ipv4ep {
	uint32_t	ipaddr;		/* IPv4 アドレス	*/
	uint16_t	portno;		/* ポート番号		*/
	} T_IPV4EP;

#define T_IPV4EP_DEFINED

/*
 *  アドレスの長さ
 */

#define IPV4_ADDR_LEN		4

/*
 *  IPv4 アドレスの定義
 */

#define IPV4_ADDRANY		ULONG_C(0x00000000)	/* ITRON TCP/IP 標準	*/
#define IPV4_ADDRBROADCAST	ULONG_C(0xffffffff)

#define IPV4_ADDR_LOOPBACK	ULONG_C(0x7f000001)
#define IPV4_ADDR_LOOPBACK_MASK	IPV4_CLASS_A_MASK
#define IPV4_ADDR_BROADCAST	ULONG_C(0xffffffff)

#define IPV4_CLASS_A(i)		(((i) & ULONG_C(0x80000000)) == 0)
#define IPV4_CLASS_A_NET	ULONG_C(0xff000000)
#define IPV4_CLASS_A_MASK	ULONG_C(0xff000000)
#define IPV4_CLASS_A_HOST	ULONG_C(0x00ffffff)

#define IPV4_CLASS_B(i)		(((i) & ULONG_C(0xc0000000)) == ULONG_C(0x80000000))
#define IPV4_CLASS_B_NET	ULONG_C(0xffff0000)
#define IPV4_CLASS_B_MASK	ULONG_C(0xffff0000)
#define IPV4_CLASS_B_HOST	ULONG_C(0x0000ffff)

#define IPV4_CLASS_C(i)		(((i) & ULONG_C(0xe0000000)) == ULONG_C(0xc0000000))
#define IPV4_CLASS_C_NET	ULONG_C(0xffffff00)
#define IPV4_CLASS_C_MASK	ULONG_C(0xffffff00)
#define IPV4_CLASS_C_HOST	ULONG_C(0x000000ff)

#define IPV4_CLASS_D(i)		(((i) & ULONG_C(0xf0000000)) == ULONG_C(0xe0000000))

#define IN4_IS_ADDR_MULTICAST(i)	IPV4_CLASS_D(i)

#define MAKE_IPV4_LOCAL_BROADCAST(i)	(IPV4_CLASS_A(i)?((i)|IPV4_CLASS_A_HOST):\
					 IPV4_CLASS_B(i)?((i)|IPV4_CLASS_B_HOST):\
					 IPV4_CLASS_C(i)?((i)|IPV4_CLASS_C_HOST):\
					 IPV4_ADDR_BROADCAST)

#define MAKE_IPV4_LOCAL_MASK(i)		(IPV4_CLASS_A(i)?IPV4_CLASS_A_MASK:\
					 IPV4_CLASS_B(i)?IPV4_CLASS_B_MASK:\
					 IPV4_CLASS_C(i)?IPV4_CLASS_C_MASK:\
					 IPV4_ADDRANY)

#define MAKE_IPV4_ADDR(a,b,c,d)		((T_IN4_ADDR)(((uint32_t)(a)<<24)|((uint32_t)(b)<<16)|((uint32_t)(c)<<8)|(d)))

/*
 *  関数シミュレーションマクロ
 */

#define IN4_ARE_ADDR_EQUAL(dst,src)	(*(dst)==*(src))
#define IN4_IS_ADDR_ANY(addr)		(*(addr)==IPV4_ADDRANY)
#define IN4_IS_ADDR_BROADCAST(addr)	(*(addr)==IPV4_ADDRBROADCAST)
#define ip2str(buf,p_ipaddr)		(ipv42str(buf,p_ipaddr))

/*
 *  動的生成用 TCP 通信端点
 */

typedef struct t_tcp_ccep {
	/* 標準 */
	ATR		cepatr;		/* 通信端点属性			*/
	void		*sbuf;		/* 送信用ウィンドバッファ	*/
	int_t		sbufsz;		/* 送信用ウィンドバッファサイズ	*/
	void		*rbuf;		/* 受信用ウィンドバッファ	*/
	int_t		rbufsz;		/* 受信用ウィンドバッファサイズ	*/
	FP		callback;	/* コールバック			*/
	/* 実装依存 */
	} T_TCP_CCEP;

/*
 *  動的生成用 TCP 受付口
 */

typedef struct t_tcp_crep {
	/* 標準 */
	ATR		repatr;		/* 受付口属性		*/
	T_IPV4EP	myaddr;		/* 自分のアドレス	*/
	/* 実装依存 */
	} T_TCP_CREP;

/*
 *  動的生成用 UDP 通信端点
 */

typedef struct t_udp_ccep {
	/* 標準 */
	ATR		cepatr;		/* UDP 通信端点属性		*/
	T_IPV4EP	myaddr;		/* 自分のアドレス		*/
	FP		callback;	/* コールバック関数		*/
	/* 実装依存 */
	} T_UDP_CCEP;

/*
 *  TINET 独自 API
 */

extern char *ipv42str (char *buf, const T_IN4_ADDR *p_ipaddr);
extern uint_t in4_get_maxnum_ifaddr (void);
extern const T_IN4_ADDR *in4_get_ifaddr (int_t index);
extern ER in4_add_ifaddr (T_IN4_ADDR addr, T_IN4_ADDR mask);
extern ER in4_add_route (int_t index, T_IN4_ADDR target,
                                      T_IN4_ADDR mask, T_IN4_ADDR gateway);

#endif	/* of #ifndef _IN4_H_ */
