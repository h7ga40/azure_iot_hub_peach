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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
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
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
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

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#ifdef SUPPORT_UDP

#ifdef UDP_CFG_NON_BLOCKING

/*
 *  IPv6 と IPv4 で引数が異なる関数のコンパイル
 */

#undef	IN_GET_DATAGRAM

#if defined(_IP6_CFG) 

#define UDP_OUTPUT		udp6_output
#define GET_UDP_CEPID		GET_UDP6_CEPID
#define T_UDP_CEP		T_UDP6_CEP
#define API_PROTO		API_PROTO_IPV6

#if defined(_IP4_CFG)
#define	IN_GET_DATAGRAM		inn_get_datagram
#else
#define	IN_GET_DATAGRAM		in6_get_datagram
#endif

#include <netinet/udpn_output.c>

#undef	UDP_OUTPUT
#undef	GET_UDP_CEPID
#undef	T_UDP_CEP
#undef  API_PROTO
#undef	IN_GET_DATAGRAM

#endif	/* of #if defined(_IP6_CFG)  */

#if defined(_IP4_CFG) && TNUM_UDP4_CEPID > 0

#define UDP_OUTPUT		udp4_output
#define GET_UDP_CEPID		GET_UDP4_CEPID
#define T_UDP_CEP		T_UDP4_CEP
#define API_PROTO		API_PROTO_IPV4

#define	IN_GET_DATAGRAM		in4_get_datagram

#include <netinet/udpn_output.c>

#endif	/* of #if defined(_IP4_CFG) && TNUM_UDP4_CEPID > 0 */

/*
 *  UDP 出力タスク
 *  ノンブロッキングコールを組み込んだとき使用する。
 */

void
udp_output_task (intptr_t exinf)
{
	ID	tskid;
	int_t	ix;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[UDP OUTPUT:%d] started.", tskid);


#if defined(_IP6_CFG) && !defined(SUPPORT_TCP)

	/* IPv6 のステートレス・アドレス自動設定を実行する。*/
	in6_if_up(IF_GET_IFNET());

#endif	/* of #if defined(_IP6_CFG) && !defined(SUPPORT_TCP) */

	while (true) {

		/* 出力がポストされるまで待つ。*/
		syscall(wai_sem(SEM_UDP_POST_OUTPUT));

#if defined(_IP6_CFG) && TNUM_UDP6_CEPID > 0

		for (ix = TNUM_UDP6_CEPID - 1; ix -- > 0; ) {

			if (udp6_cep[ix].flags & UDP_CEP_FLG_POST_OUTPUT) {
				udp6_cep[ix].flags &= ~UDP_CEP_FLG_POST_OUTPUT;
				udp6_output(&udp6_cep[ix]);
				}
			}

#endif	/* of #if defined(_IP6_CFG) && TNUM_UDP6_CEPID > 0 */

#if defined(_IP4_CFG) && TNUM_UDP4_CEPID > 0

		for (ix = TNUM_UDP4_CEPID - 1; ix -- > 0; ) {

			if (udp4_cep[ix].flags & UDP_CEP_FLG_POST_OUTPUT) {
				udp4_cep[ix].flags &= ~UDP_CEP_FLG_POST_OUTPUT;
				udp4_output(&udp4_cep[ix]);
				}
			}

#endif	/* of #if defined(_IP4_CFG) && TNUM_UDP4_CEPID > 0 */

		}
	}

#endif	/* of #ifdef UDP_CFG_NON_BLOCKING */

#endif	/* of #ifdef SUPPORT_UDP */
