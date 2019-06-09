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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 * $FreeBSD: src/sys/netinet/ip_var.h,v 1.45.2.1 1999/08/29 16:29:51 peter Exp $
 */
 
#ifndef _IP_VAR_H_
#define _IP_VAR_H_

#include <netinet/ip4_var.h>
#include <netinet6/ip6_var.h>

/*
 *  IPv4 と IPv6 をコンパイル時に選択するためのマクロ
 */

#define GET_IP_VER(nbuf)		(*(uint8_t*)((nbuf)->buf+IF_HDR_SIZE)>>4)

#if defined(_IP6_CFG)

#if defined(_IP4_CFG)

#define IF_IP_HDR_SIZE(nbuf)		(GET_IP_VER(nbuf)==IPV6_VERSION?IF_IP6_HDR_SIZE:IF_IP4_HDR_SIZE)
#define IF_IP_NET_HDR_SIZE(addr)	(IN6_IS_ADDR_V4MAPPED(addr)?IF_IP4_HDR_SIZE:IF_IP6_HDR_SIZE)

#define IP_OUTPUT(nbuf,tmout)		(GET_IP_VER(nbuf)==IPV6_VERSION?ip6_output(nbuf,0,tmout):ip_output(nbuf,tmout))
#define IP_REMOVE_OPTIONS(nbuf)		(GET_IP_VER(nbuf)==IPV6_VERSION?ip6_remove_exthdrs(nbuf):ip_remove_options(nbuf))

#define GET_IP_HDR(nbuf)		(GET_IP_VER(nbuf)==IPV6_VERSION?GET_IP6_HDR(nbuf):GET_IP4_HDR(nbuf))
#define GET_IP_HDR_SIZE(nbuf)		(GET_IP_VER(nbuf)==IPV6_VERSION?GET_IP6_HDR_SIZE(nbuf):GET_IP4_HDR_SIZE(nbuf))
#define GET_IP_SDU_SIZE(nbuf)		(GET_IP_VER(nbuf)==IPV6_VERSION?GET_IP6_SDU_SIZE(nbuf):GET_IP4_SDU_SIZE(nbuf))
#define GET_IF_IP_HDR_SIZE(nbuf)	(GET_IP_VER(nbuf)==IPV6_VERSION?GET_IF_IP6_HDR_SIZE(nbuf):GET_IF_IP4_HDR_SIZE(nbuf))
#define GET_IP_TCP_Q_HDR_OFFSET(nbuf)	(GET_IP_VER(nbuf)==IPV6_VERSION?\
					((T_IP6_TCP_Q_HDR*)GET_IP6_HDR(nbuf))->thoff:((T_IP4_TCP_Q_HDR*)GET_IP4_HDR(nbuf))->thoff)

#define SET_IP_CF(nbuf,cf)		(GET_IP_VER(nbuf)==IPV6_VERSION?SET_IP6_CF(nbuf,cf):(0))
#define SET_IP_SDU_SIZE(nbuf,len)	(GET_IP_VER(nbuf)==IPV6_VERSION?SET_IP6_SDU_SIZE(nbuf,len):SET_IP4_SDU_SIZE(nbuf,len))
#define SET_IP_TCP_Q_HDR_OFFSET(nbuf,thoff)	\
					(GET_IP_VER(nbuf)==IPV6_VERSION\
						?(((T_IP6_TCP_Q_HDR*)GET_IP6_HDR(nbuf))->thoff=thoff)\
						:(((T_IP4_TCP_Q_HDR*)GET_IP4_HDR(nbuf))->thoff=thoff))

#else	/* of #if defined(_IP4_CFG) */

#define IF_IP_HDR_SIZE(nbuf)		IF_IP6_HDR_SIZE
#define IF_IP_NET_HDR_SIZE(addr)	IF_IP6_HDR_SIZE

#define IP_OUTPUT(nbuf,tmout)		ip6_output(nbuf,0,tmout)
#define IP_REMOVE_OPTIONS(nbuf)		ip6_remove_exthdrs(nbuf)

#define GET_IP_HDR(nbuf)		GET_IP6_HDR(nbuf)
#define GET_IP_HDR_SIZE(nbuf)		GET_IP6_HDR_SIZE(nbuf)
#define GET_IP_SDU_SIZE(nbuf)		GET_IP6_SDU_SIZE(nbuf)
#define GET_IF_IP_HDR_SIZE(nbuf)	GET_IF_IP6_HDR_SIZE(nbuf)
#define GET_IP_TCP_Q_HDR_OFFSET(nbuf)	(((T_IP6_TCP_Q_HDR*)GET_IP6_HDR(nbuf))->thoff)

#define SET_IP_CF(nbuf,cf)		SET_IP6_CF(nbuf,cf)
#define SET_IP_SDU_SIZE(nbuf,len)	SET_IP6_SDU_SIZE(nbuf,len)
#define SET_IP_TCP_Q_HDR_OFFSET(nbuf,thoff)	\
					(((T_IP6_TCP_Q_HDR*)GET_IP6_HDR(nbuf))->thoff=thoff)

#endif	/* of #if defined(_IP4_CFG) */

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

#define IF_IP_HDR_SIZE(nbuf)		IF_IP4_HDR_SIZE
#define IF_IP_NET_HDR_SIZE(addr)	IF_IP4_HDR_SIZE

#define IP_OUTPUT(nbuf,tmout)		ip_output(nbuf,tmout)
#define IP_REMOVE_OPTIONS(nbuf)		ip_remove_options(nbuf)

#define GET_IP_HDR(nbuf)		GET_IP4_HDR(nbuf)
#define GET_IP_HDR_SIZE(nbuf)		GET_IP4_HDR_SIZE(nbuf)
#define GET_IP_SDU_SIZE(nbuf)		GET_IP4_SDU_SIZE(nbuf)
#define GET_IF_IP_HDR_SIZE(nbuf)	GET_IF_IP4_HDR_SIZE(nbuf)
#define GET_IP_TCP_Q_HDR_OFFSET(nbuf)	(((T_IP4_TCP_Q_HDR*)GET_IP4_HDR(nbuf))->thoff)

#define SET_IP_SDU_SIZE(nbuf,len)	SET_IP4_SDU_SIZE(nbuf,len)
#define SET_IP_TCP_Q_HDR_OFFSET(nbuf,thoff)	\
					(((T_IP4_TCP_Q_HDR*)GET_IP4_HDR(nbuf))->thoff=thoff)

#else	/* of #if defined(_IP4_CFG) */

#error "not implemented."

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */

#endif	/* of #ifndef _IP_VAR_H_ */
