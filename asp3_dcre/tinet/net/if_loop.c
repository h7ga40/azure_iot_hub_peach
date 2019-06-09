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
 *	@(#)if_loop.c	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/net/if_loop.c,v 1.37.2.1 1999/08/29 16:28:19 peter Exp $
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_loop.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <net/if_var.h>

#ifdef SUPPORT_LOOP

/*
 *  ローカルループバックインタフェース
 */

/*
 *  外部関数の定義
 */

extern const char *itron_strerror (ER ercd);

/*
 *  変数
 */

/* ネットワークインタフェース構造体 */

#if defined(_IP6_CFG)

static T_IFNET loop_ifnet = {
	NULL,			/* ネットワークインタフェースのソフトウェア情報	*/
	{},			/* IPv6 アドレス情報				*/
	{},			/* マルチキャスト IPv6 アドレス			*/
	};

#elif defined(_IP4_CFG)	/* of #if defined(_IP6_CFG) */

static T_IFNET loop_ifnet = {
	NULL,			/* ネットワークインタフェースのソフトウェア情報	*/
	{
		IPV4_ADDR_LOOPBACK,		/* IPv4 アドレス			*/
		IPV4_ADDR_LOOPBACK_MASK,	/* サブネットマスク			*/
		},
	};

#endif	/* of #if defined(_IP6_CFG) */

/*
 *  loop_get_ifnet -- ネットワークインタフェース構造体を返す。
 */

T_IFNET *
loop_get_ifnet (void)
{
	return &loop_ifnet;
	}

/*
 *  looutput -- ローカルループバックインタフェースの出力関数
 */

ER
looutput (T_NET_BUF *output, TMO tmout)
{
	ER error;

	NET_COUNT_LOOP(net_count_loop.out_octets,  output->len);
	NET_COUNT_LOOP(net_count_loop.out_packets, 1);

	if ((error = tsnd_dtq(DTQ_LOOP_OUTPUT, output, tmout)) != E_OK)
		syslog(LOG_NOTICE, "[LOOP] drop error: %s", itron_strerror(error));
	return error;
	}

/*
 *  ローカルループバックインタフェース・出力タスク
 */

void
if_loop_output_task (intptr_t exinf)
{
	T_NET_BUF	*output;
	ER		error;
	ID		tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[LOOP OUTPUT:%d] started.", tskid);
	while (true) {
		if (rcv_dtq(DTQ_LOOP_OUTPUT, (intptr_t*)&output) == E_OK) {
			NET_COUNT_LOOP(net_count_loop.in_octets,  output->len);
			NET_COUNT_LOOP(net_count_loop.in_packets, 1);
			if ((error = snd_dtq(DTQ_LOOP_INPUT, output)) != E_OK)
				syslog(LOG_NOTICE, "[LOOP OUTPUT] drop error: %s", itron_strerror(error));
			}
		}
	}

/*
 *  ローカルループバックインタフェース・入力タスク
 */

void
if_loop_input_task (intptr_t exinf)
{
	T_NET_BUF	*input;
	ID		tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[LOOP INPUT:%d] started.", tskid);
	while (true) {
		if (rcv_dtq(DTQ_LOOP_INPUT, (intptr_t)&input) == E_OK) {
			NET_COUNT_LOOP(net_count_loop.in_octets,  input->len);
			NET_COUNT_LOOP(net_count_loop.in_packets, 1);

#if defined(_IP4_CFG)

			/* IPv4 入力関数を呼び出す */
			if (IP4_VHL_V(GET_IP4_HDR(input)->vhl) == IPV4_VERSION)
				ip_input(input);

#endif	/* of #if defined(_IP4_CFG) */

#if defined(_IP6_CFG)

			/* IPv6 入力関数を呼び出す */
			if (IP6_VCF_V(ntohl(GET_IP6_HDR(input)->vcf)) == IPV6_VERSION)
				ip6_input(input);

#endif	/* of #if defined(_IP6_CFG) */

			}
		}
	}

#endif	/* of #ifdef SUPPORT_LOOP */
