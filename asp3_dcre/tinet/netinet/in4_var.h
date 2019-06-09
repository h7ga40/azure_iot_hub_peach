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
 * Copyright (c) 1985, 1986, 1993
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
 *	@(#)in_var.h	8.2 (Berkeley) 1/9/95
 * $FreeBSD: src/sys/netinet/in_var.h,v 1.29.2.1 1999/08/29 16:29:38 peter Exp $
 */
 
#ifndef _IN4_VAR_H_
#define _IN4_VAR_H_

#ifndef _MACRO_ONLY

/*
 *  インタフェースアドレス構造体
 */

typedef struct t_in4_ifaddr {
	T_IN4_ADDR		addr;		/* IPv4 アドレス	*/
	T_IN4_ADDR		mask;		/* サブネットマスク	*/
	} T_IN4_IFADDR;

/*
 *  ルーティング表エントリ構造体
 */

typedef struct t_in4_rtentry {
	T_IN4_ADDR	target;		/* 目標ネットワークアドレス			*/
	T_IN4_ADDR	mask;		/* 目標ネットワークアドレスのサブネットマスク	*/
	T_IN4_ADDR	gateway;	/* ゲートウェイの IP アドレス			*/
	uint32_t	expire;		/* 有効時間が切れる時刻 [s]			*/
	uint8_t		flags;		/* フラグ					*/
	} T_IN4_RTENTRY;
/*
 *  TINET 1.1 との互換性のための定義
 *
 *  TINET 1.5 との互換性のために定義を無効にした。
 */

#if 0
#ifdef NUM_ROUTE_ENTRY

#define NUM_IN4_ROUTE_ENTRY		NUM_ROUTE_ENTRY
#define NUM_IN4_STATIC_ROUTE_ENTRY	NUM_ROUTE_ENTRY
#define NUM_IN4_REDIRECT_ROUTE_ENTRY	0
#define T_RT_ENTRY			T_IN4_RTENTRY

#endif	/* of #ifdef NUM_ROUTE_ENTRY */
#endif

/*
 *  ルーティング表
 */

#ifdef SUPPORT_INET4

extern T_IN4_RTENTRY routing4_tbl[];

#endif	/* of #ifdef SUPPORT_INET4 */

/*
 *  前方参照
 */

#ifndef T_NET_BUF_DEFINED
typedef struct t_net_buf T_NET_BUF;
#define T_NET_BUF_DEFINED
#endif

#ifndef T_IFNET_DEFINED
typedef struct t_ifnet T_IFNET;
#define T_IFNET_DEFINED
#endif

/*
 *  関数シミュレーションマクロ
 */

#define IN4_COPY_TO_HOST(dst,nbuf)	(*(dst)=ntohl(GET_IP4_HDR(nbuf)->src))

/*
 *  関数
 */

extern void ip_input (T_NET_BUF *data);
extern ER in4_set_header (T_NET_BUF *nbuf, uint_t len,
                          T_IN4_ADDR *dstaddr, T_IN4_ADDR *srcaddr, uint8_t proto, uint8_t ttl);
extern ER in4_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
                            T_IN4_ADDR *dstaddr, T_IN4_ADDR *srcaddr,
                            uint8_t proto, uint8_t ttl, ATR nbatr, TMO tmout);
extern uint16_t in4_cksum (T_NET_BUF *nbuf, uint8_t proto, uint_t off, uint_t len);
extern T_IN4_ADDR in4_rtalloc (T_IN4_ADDR dst);
extern void in4_rtredirect (T_IN4_ADDR gateway, T_IN4_ADDR target, uint8_t flags, uint32_t tmo);
extern void in4_init (void);
extern T_IN4_ADDR *in4_addrwithifp (T_IFNET *ifp, T_IN4_ADDR *src, T_IN4_ADDR *dst);
extern const T_NET_BUF**ip_get_frag_queue (void);

/*
 *  in4_is_dstaddr_accept -- 宛先アドレスとして正しいかチェックする。
 *
 *    注意: dstaddr は、
 *          TINET-1.2 からネットワークバイトオーダ、
 *          TINET-1.1 までは、ホストバイトオーダ
 */
extern bool_t in4_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_IN4_ADDR *dstaddr);
extern bool_t inn4_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_NET_BUF *input);

/*
 *  in_cksum -- チェックサム計算関数、IPv4、ICMPv4 用
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 */
extern uint16_t in_cksum(void *data, uint_t len /*オクテット単位*/);

/*
 *  in_cksum_sum -- チェックサムの合計計算関数
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 */
extern uint32_t in_cksum_sum (void *data, uint_t len /*オクテット単位*/);

/*
 *  in_cksum_carry -- チェックサムの桁上げ計算関数
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 */
extern uint32_t in_cksum_carry (uint32_t sum);

#endif	/* of #ifndef _MACRO_ONLY */

#endif	/* of #ifndef _IN4_VAR_H_ */
