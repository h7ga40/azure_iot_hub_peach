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
 
#ifndef _IN_H_
#define _IN_H_

#include <tinet_kernel_defs.h>

#include <netinet/in4.h>
#include <netinet6/in6.h>

/*
 *  TINET のバージョン情報
 */

#define TINET_PRVER		UINT_C(0x1070)	/* TINET のバージョン番号 */

/*
 *  インターネットシステムの定義、RFC790 参照
 */

/*
 *  プロトコル (RFC1700)
 */

#define IPPROTO_IP		UINT_C(0)	/* IP のダミー				*/
#define IPPROTO_HOPOPTS		UINT_C(0)	/* IPv6 中継点 (Hop-by-Hop) オプション	*/
#define IPPROTO_ICMP		UINT_C(1)	/* ICMP					*/
#define IPPROTO_IGMP		UINT_C(2)	/* IGMP					*/
#define IPPROTO_TCP		UINT_C(6)	/* TCP					*/
#define IPPROTO_UDP		UINT_C(17)	/* UDP					*/
#define IPPROTO_IPV6		UINT_C(41)	/* IPv6					*/
#define IPPROTO_ROUTING		UINT_C(43)	/* 経路制御オプション			*/
#define IPPROTO_FRAGMENT	UINT_C(44)	/* 断片化オプション			*/
#define IPPROTO_RSVP		UINT_C(46)	/* RSVP					*/
#define IPPROTO_ESP		UINT_C(50)	/* IPv6 暗号化オプション		*/
#define IPPROTO_AH		UINT_C(51)	/* IPv6 認証オプション			*/
#define IPPROTO_ICMPV6		UINT_C(58)	/* ICMPv6				*/
#define IPPROTO_NONE		UINT_C(59)	/* IPv6 次ヘッダ無し			*/
#define IPPROTO_DSTOPTS		UINT_C(60)	/* IPv6 終点オプション			*/
#define IPPROTO_IPCOMP		UINT_C(108)	/* ペイロード圧縮			*/
#define IPPROTO_DONE		UINT_C(257)	/* IPv6 で上位プロトコル入力終了	*/

/*
 *  ポートの長さ
 */

#define PORT_NUM_LEN		2

/*
 *  ポート番号の定義
 */

#define TCP_PORTANY			UINT_C(0x0000)	/* ITRON TCP/IP 標準	*/
#define UDP_PORTANY			UINT_C(0x0000)	/* ITRON TCP/IP 標準	*/

#define TCP_PORT_LAST_WELL_KNOWN	UINT_C(1023)	/* Well Known 終了番号	*/
#define TCP_PORT_FIRST_AUTO		UINT_C(1024)	/* 自動割り当て開始番号	*/
#define TCP_PORT_LAST_AUTO		UINT_C(4999)	/* 自動割り当て終了番号	*/

#define UDP_PORT_LAST_WELL_KNOWN	UINT_C(1023)	/* Well Known 終了番号	*/
#define UDP_PORT_FIRST_AUTO		UINT_C(1024)	/* 自動割り当て開始番号	*/
#define UDP_PORT_LAST_AUTO		UINT_C(4999)	/* 自動割り当て終了番号	*/

/*
 *  API IPプロトコルの指定
 */

#define API_PROTO_IPVn			'n'
#define API_PROTO_IPV6			'6'
#define API_PROTO_IPV4			'4'

/*
 *  動的生成用 TCP/IPv6 通信端点
 */

#define T_TCP6_CCEP			T_TCP_CCEP

/*
 *  バイトオーダ関数の定義
 *
 *    tinet/net/net.h でもバイトオーダの定義を行っているが、
 *    tinet/net/net.h をインクルードしない
 *    アプリケーションプログラム用に
 *    ターゲット依存しないバイトオーダ関数を定義する。
 */

#if defined(_NET_CFG_BYTE_ORDER)

/*  tinet/net/net.h をインクルードしている。*/

#else	/* of #if defined(_NET_CFG_BYTE_ORDER) */

extern uint16_t ntohs (uint16_t net);
extern uint16_t htons (uint16_t host);
extern uint32_t ntohl (uint32_t net);
extern uint32_t htonl (uint32_t host);

#define NTOHS(n)	(n=ntohs(n))
#define HTONS(h)	(h=htons(h))
#define NTOHL(n)	(n=ntohl(n))
#define HTONL(h)	(h=htonl(h))

#endif	/* of #if defined(_NET_CFG_BYTE_ORDER) */

/*
 *  TINET 独自 API
 */

extern const char *in_strtfn (FN fncd);

#endif	/* of #ifndef _IN_H_ */
