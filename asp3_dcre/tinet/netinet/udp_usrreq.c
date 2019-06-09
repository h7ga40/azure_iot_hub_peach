/*
 *  TINET (UDP/IP Protocol Stack)
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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must ceproduce the above copyright
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
 *	@(#)udp_usrreq.c	8.6 (Berkeley) 5/23/95
 * $FreeBSD: src/sys/netinet/udp_usrreq.c,v 1.49.2.2 1999/08/29 16:29:58 peter Exp $
 */

/*
 *  ノンブロッキングコールを組込まない場合にリンクされる関数の定義
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>
#include <net/ppp_ipcp.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/ip_igmp.h>

#ifdef SUPPORT_UDP

/*
 *  TINET をライブラリ化しない場合は、全ての機能を
 *  オブジェクトファイルに出力するため、マクロを有効にする。
 */

#ifndef UDP_CFG_LIBRARY

#define __udp_send_data
#define __udp_cre_cep
#define __udp_del_cep
#define __udp_can_cep
#define __udp_set_opt
#define __udp_get_opt
#define __udp_can_snd
#define __udp_can_rcv
#define __udp_snd_dat
#define __udp_rcv_dat

#endif	/* of #ifndef UDP_CFG_LIBRARY */

/*
 *  IPv6 と IPv4 で引数が異なる関数のコンパイル
 */

#undef	IN_GET_DATAGRAM
#undef	IN_COPY_TO_HOST

#if defined(SUPPORT_INET6) && TNUM_UDP6_CEPID > 0

#define UDP_CANCEL_CEP		udp6_cancel_cep
#define UDP_DELETE_CEP		udp6_delete_cep
#define UDP_SET_OPTION		udp6_set_option
#define UDP_GET_OPTION		udp6_get_option
#define UDP_CRE_CEP		udp6_cre_cep
#define UDP_DEL_CEP		udp6_del_cep
#define UDP_SET_OPT		udp6_set_opt
#define UDP_GET_OPT		udp6_get_opt
#define UDP_SND_DAT		udp6_snd_dat
#define UDP_RCV_DAT		udp6_rcv_dat
#define UDP_CAN_SND		udp6_can_snd
#define UDP_CAN_RCV		udp6_can_rcv
#define UDP_ALLOC_AUTO_PORT	udp6_alloc_auto_port
#define UDP_ALLOC_PORT		udp6_alloc_port
#define UDP_SEND_DATA		udp6_send_data
#define VALID_UDP_CEPID		VALID_UDP6_CEPID
#define GET_UDP_CEP		GET_UDP6_CEP
#define GET_UDP_CEPID		GET_UDP6_CEPID
#define T_UDP_CEP		T_UDP6_CEP
#define T_UDPN_CCEP		T_UDP6_CCEP
#define T_IPEP			T_IPV6EP
#define API_PROTO		API_PROTO_IPV6

#if defined(_IP4_CFG)
#define	IN_GET_DATAGRAM		inn_get_datagram
#define IN_COPY_TO_HOST		inn_copy_to_host
#else
#define	IN_GET_DATAGRAM		in6_get_datagram
#define IN_COPY_TO_HOST		IN6_COPY_TO_HOST
#endif

#include <netinet/udpn_usrreq.c>

#undef	UDP_CANCEL_CEP
#undef	UDP_DELETE_CEP
#undef	UDP_SET_OPTION
#undef	UDP_GET_OPTION
#undef	UDP_CRE_CEP
#undef	UDP_DEL_CEP
#undef	UDP_SET_OPT
#undef	UDP_GET_OPT
#undef	UDP_SND_DAT
#undef	UDP_RCV_DAT
#undef	UDP_CAN_SND
#undef	UDP_CAN_RCV
#undef	UDP_ALLOC_AUTO_PORT
#undef	UDP_ALLOC_PORT
#undef	UDP_SEND_DATA
#undef	VALID_UDP_CEPID
#undef	GET_UDP_CEP
#undef	GET_UDP_CEPID
#undef	T_UDP_CEP
#undef	T_UDPN_CCEP
#undef  T_IPEP
#undef  API_PROTO
#undef	IN_GET_DATAGRAM
#undef	IN_COPY_TO_HOST

#endif	/* of #if defined(SUPPORT_INET6) && TNUM_UDP6_CEPID > 0 */

#if defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0

#define UDP_CANCEL_CEP		udp4_cancel_cep
#define UDP_DELETE_CEP		udp4_delete_cep
#define UDP_SET_OPTION		udp4_set_option
#define UDP_GET_OPTION		udp4_get_option
#define UDP_CRE_CEP		udp_cre_cep
#define UDP_DEL_CEP		udp_del_cep
#define UDP_SET_OPT		udp_set_opt
#define UDP_GET_OPT		udp_get_opt
#define UDP_SND_DAT		udp_snd_dat
#define UDP_RCV_DAT		udp_rcv_dat
#define UDP_CAN_SND		udp4_can_snd
#define UDP_CAN_RCV		udp4_can_rcv
#define UDP_ALLOC_AUTO_PORT	udp4_alloc_auto_port
#define UDP_ALLOC_PORT		udp4_alloc_port
#define UDP_SEND_DATA		udp4_send_data
#define VALID_UDP_CEPID		VALID_UDP4_CEPID
#define GET_UDP_CEP		GET_UDP4_CEP
#define GET_UDP_CEPID		GET_UDP4_CEPID
#define T_UDP_CEP		T_UDP4_CEP
#define T_UDPN_CCEP		T_UDP_CCEP
#define T_IPEP			T_IPV4EP
#define API_PROTO		API_PROTO_IPV4

#define IN_COPY_TO_HOST		IN4_COPY_TO_HOST
#define	IN_GET_DATAGRAM		in4_get_datagram

#include <netinet/udpn_usrreq.c>

#endif	/* of #if defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0 */

#if defined(SUPPORT_INET6) && (defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0)

/*
 *  udp_can_cep -- ペンディングしている処理のキャンセル【標準機能】
 */

#ifdef __udp_can_cep

ER
udp_can_cep (ID cepid, FN fncd)
{
	/* IPv6 用の UDP 通信端点 ID をチェックする。*/
	if (VALID_UDP6_CEPID(cepid)) {

		/* UDP 通信端点を得てメイン関数を呼び出す。*/
		return udp6_cancel_cep(GET_UDP6_CEP(cepid), fncd);
		}

	/* IPv4 用の UDP 通信端点 ID をチェックする。*/
	else if (VALID_UDP4_CEPID(cepid)) {

		/* UDP 通信端点を得てメイン関数を呼び出す。*/
		return udp4_cancel_cep(GET_UDP4_CEP(cepid), fncd);
		}

	else
		return E_ID;
	}

#endif	/* of #ifdef __udp_can_cep */

#endif	/* of #if defined(SUPPORT_INET6) && (defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0) */

#endif	/* of #ifdef SUPPORT_UDP */
