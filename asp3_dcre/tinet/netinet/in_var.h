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
 
#ifndef _IN_VAR_H_
#define _IN_VAR_H_

#include <netinet/in4_var.h>
#include <netinet6/in6_var.h>

/*
 *  独自のエラーコード
 */

#define EV_NOPND		(-97)		/* ペンディングしていない */

/*
 *  ネットワークエラーコード
 */

#define EV_MSIZE		(-98)		/* データグラム長超過	*/
#define EV_NURCH		(-99)		/* ネットワーク到達不能	*/
#define EV_HDOWN		(-100)		/* ホストダウン		*/
#define EV_HURCH		(-101)		/* ホスト到達不能	*/
#define EV_CNNRF		(-102)		/* 接続不能		*/
#define EV_CNRST		(-103)		/* 接続リセット		*/
#define EV_REXMTMO		(-104)		/* 再送タイムアウト	*/

/*
 *  共通タイマの起動周期の定義
 */

#define IN_TIMER_TMO		(SYSTIM_HZ/NET_TIMER_CYCLE)

/*
 *  ルーティングエントリ状態フラグ
 */

#define IN_RTF_DEFINED		UINT_C(0x01)	/* 定義済み			*/
#define IN_RTF_REDIRECT		UINT_C(0x10)	/* 向け直しにより自動設定された	*/

/*
 *  IPv4 と IPv6 をコンパイル時に選択するためのマクロ
 */

#if defined(_IP6_CFG)

#define T_IN_ADDR			T_IN6_ADDR
#define T_IPEP				T_IPV6EP
#define IP_ADDRANY			IPV6_ADDRANY

#elif defined(_IP4_CFG)	/* of #if defined(_IP6_CFG) */

#define T_IN_ADDR			T_IN4_ADDR
#define T_IPEP				T_IPV4EP
#define IP_ADDRANY			IPV4_ADDRANY

#endif	/* of #if defined(_IP6_CFG) */

#if defined(_IP6_CFG)

#if defined(_IP4_CFG)

#define IN_CKSUM(nbuf,proto,off,len)	(GET_IP_VER(nbuf)==IPV6_VERSION	\
					?in6_cksum(nbuf,proto,off,len)	\
					:in4_cksum(nbuf,proto,off,len))
#define IN_IS_DSTADDR_ACCEPT(myaddr,nbuf)	\
					inn_is_dstaddr_accept(myaddr,nbuf)
#define IN_ARE_HDR_ADDR_EQUAL(nbuf)	(GET_IP_VER(nbuf)==IPV6_VERSION	\
					?IN6_ARE_ADDR_EQUAL(&GET_IP6_HDR(nbuf)->dst,&GET_IP6_HDR(nbuf)->src)	\
					:IN4_ARE_ADDR_EQUAL(&GET_IP4_HDR(nbuf)->dst,&GET_IP4_HDR(nbuf)->src))
#define IN_ARE_NET_SRCADDR_EQUAL(ipaddr,nbuf)	\
		    			inn_are_net_srcaddr_equal(ipaddr,nbuf)
#define IN_COPY_TO_HOST(dst,nbuf)	inn_copy_to_host(dst,nbuf)
#define IN_GET_DATAGRAM(nbuf,len,maxlen,dst,src,next,hlim,nbatr,tmout)	\
					inn_get_datagram(nbuf,len,maxlen,dst,src,next,hlim,nbatr,tmout)
#define IN_IS_ADDR_MULTICAST(addr)	inn_is_addr_multicast(addr)
#define IN_IS_NET_ADDR_MULTICAST(nbuf)	(GET_IP_VER(input)==IPV6_VERSION	\
					?IN_IS_ADDR_MULTICAST(&GET_IP6_HDR(input)->dst)	\
					:IN4_IS_ADDR_MULTICAST(ntohl(GET_IP4_HDR(input)->dst)))
#define IN_ADDRWITHIFP(ifp,src,dst)	inn_addrwithifp(ifp,src,dst)
#define IN_IS_ADDR_ANY(addr)		IN6_IS_ADDR_UNSPECIFIED(addr)

#else	/* of #if defined(_IP4_CFG) */

#define IN_CKSUM(nbuf,proto,off,len)	in6_cksum(nbuf,proto,off,len)
#define IN_IS_DSTADDR_ACCEPT(myaddr,nbuf)	\
					INN6_IS_DSTADDR_ACCEPT(myaddr,nbuf)
#define IN_ARE_HDR_ADDR_EQUAL(nbuf)	IN6_ARE_ADDR_EQUAL(&GET_IP6_HDR(nbuf)->dst,&GET_IP6_HDR(nbuf)->src)
#define IN_ARE_NET_SRCADDR_EQUAL(ipaddr,nbuf)	\
		    			IN6_ARE_ADDR_EQUAL(ipaddr,&GET_IP6_HDR(nbuf)->src)
#define IN_COPY_TO_HOST(dst,nbuf)	IN6_COPY_TO_HOST(dst, nbuf)
#define IN_GET_DATAGRAM(nbuf,len,maxlen,dst,src,next,hlim,nbatr,tmout)	\
					in6_get_datagram(nbuf,len,maxlen,dst,src,next,hlim,nbatr,tmout)
#define IN_IS_ADDR_MULTICAST(addr)	IN6_IS_ADDR_MULTICAST(addr)
#define IN_IS_NET_ADDR_MULTICAST(nbuf)	IN6_IS_ADDR_MULTICAST(&GET_IP6_HDR(nbuf)->dst)
#define IN_ADDRWITHIFP(ifp,src,dst)	in6_addrwithifp(ifp,src,dst)
#define IN_IS_ADDR_ANY(addr)		IN6_IS_ADDR_UNSPECIFIED(addr)

#endif	/* of #if defined(_IP4_CFG) */

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

#define IN_CKSUM(nbuf,proto,off,len)	in4_cksum(nbuf,proto,off,len)
#define IN_IS_DSTADDR_ACCEPT(myaddr,nbuf)	\
					inn4_is_dstaddr_accept(myaddr,nbuf)
#define IN_ARE_HDR_ADDR_EQUAL(nbuf)	IN4_ARE_ADDR_EQUAL(&GET_IP4_HDR(nbuf)->dst,&GET_IP4_HDR(nbuf)->src)
#define IN_ARE_NET_SRCADDR_EQUAL(ipaddr,nbuf)	\
					(*(ipaddr)==ntohl(GET_IP4_HDR(nbuf)->src))
#define IN_COPY_TO_HOST(dst,nbuf)	IN4_COPY_TO_HOST(dst, nbuf)
#define IN_GET_DATAGRAM(nbuf,len,maxlen, dst,src,proto,ttl,nbatr,tmout)	\
					in4_get_datagram(nbuf,len,maxlen, dst,src,proto,ttl,nbatr,tmout)
#define IN_IS_ADDR_MULTICAST(addr)	IN4_IS_ADDR_MULTICAST(*(addr))
#define IN_IS_NET_ADDR_MULTICAST(nbuf)	IN4_IS_ADDR_MULTICAST(ntohl(GET_IP4_HDR(nbuf)->dst))
#define IN_ADDRWITHIFP(ifp,src,dst)	in4_addrwithifp(ifp,src,dst)
#define IN_IS_ADDR_ANY(addr)		IN4_IS_ADDR_ANY(addr)

#else	/* of #if defined(_IP4_CFG) */

#error "not implemented."

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */

#ifndef _MACRO_ONLY

/*
 *  関数
 */
extern bool_t	inn_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_NET_BUF *nbuf);
extern bool_t	inn_is_addr_multicast (T_IN6_ADDR *addr);
extern bool_t	inn_are_net_srcaddr_equal (T_IN6_ADDR *ipaddr, T_NET_BUF *nbuf);
extern void	inn_copy_to_host (T_IN6_ADDR *dst, T_NET_BUF *nbuf);
extern ER	inn_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
		                  T_IN6_ADDR *dstaddr, T_IN6_ADDR *srcaddr,
		                  uint8_t next, uint8_t hlim, ATR nbatr, TMO tmout);
extern T_IN6_ADDR *inn_addrwithifp (T_IFNET *ifp, T_IN6_ADDR *src, T_IN6_ADDR *dst);

#endif	/* of #ifndef _MACRO_ONLY */

#endif	/* of #ifndef _IN_VAR_H_ */
