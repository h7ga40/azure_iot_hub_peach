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

/*	$FreeBSD: src/sys/netinet6/route6.c,v 1.6 2002/10/16 01:54:45 sam Exp $	*/
/*	$KAME: route6.c,v 1.24 2001/03/14 03:07:05 itojun Exp $	*/

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

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/net.h>
#include <net/net_endian.h>
#include <net/if.h>
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/ppp_ipcp.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet6/nd6.h>

#ifdef _IP6_CFG

#if 0
/*
 *  通信帯域を食いつぶす攻撃への対応のため、
 *  タイプ 0 経路制御ヘッダの処理を行わない。
 */

/*
 *  ip6_rthdr0 -- タイプ 0 経路制御ヘッダの入力関数
 */

static ER
ip6_rthdr0 (T_NET_BUF *input, T_IP6_RT0_HDR *rt0h)
{
	if (rt0h->segleft == 0) {
		/* 中継ノード数は 0 なので、終点ノード */
		return E_OK;
		}

	/* len と segleft の正当性チェック */
#ifdef IP6_CFG_COMPAT_RFC1883
	if ((rt0h->len % 2) || (rt0h->len > 46)) {
#else
	if (rt0h->len % 2) {
#endif
		/*
		 *  len は偶数で、(23 * 2) まで。
		 *  ただし、(23 * 2) の制限は RFC 2462 で削除されている。
		 */
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   ((uint8_t*)&rt0h->len - input->buf) - IF_HDR_SIZE);
		return E_PAR;
		}

	/* len が中継ノード数と矛盾していないかチェックする。*/
	if ((rt0h->len / 2) < rt0h->segleft) {
		/*
		 *  len は 8オクテット単位、segleft は 16オクテット単位なので、
		 *  len / 2 == segleft となるはず。
		 *  len / 2 <  segleft はエラー。
		 */
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   ((uint8_t*)&rt0h->segleft - input->buf) - IF_HDR_SIZE);
		return E_PAR;
		}

	/*
	 *  残りの中継ノード数が 1 以上であれば、このノードは最終的な
	 *  終点ノードではない。本実装では、中継をしないので、
	 *  入力データグラムの処理を終了する。
	 */
	syscall(rel_net_buf(input));
	return E_PAR;
	}

#endif

/*
 *  route6_input -- 経路制御ヘッダの入力関数
 */

uint_t
route6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_IP6_RT_HDR	*rth;
	uint_t		len, off = *offp;

	/* ネットワークバッファの残りの長さをチェックする。*/
	if (input->len - off < sizeof(T_IP6_RT_HDR))
		goto buf_rel;

	rth = (T_IP6_RT_HDR *)(input->buf + off);
	len = (rth->len + 1) << 3;

	/* 経路制御ヘッダの長さをチェックする。*/
	if (input->len < off + len)
		goto buf_rel;

#if 0
	/*
	 *  通信帯域を食いつぶす攻撃への対応のため、
	 *  タイプ 0 経路制御ヘッダの処理を行わない。
	 */
	if (rth->type == IPV6_RTHDR_TYPE_0) {
		if (ip6_rthdr0(input, (T_IP6_RT0_HDR*)rth) != E_OK) {

			/* エラーの場合は、ip6_rthdr0 でバッファを開放している。*/
			return IPPROTO_DONE;
			}
		}
	else if (rth->segleft != 0) {

		/*
		 *  残りの中継ノード数が 1 以上であれば、このノードは最終的な
		 *  終点ノードではない。本実装では、中継をしないので、
		 *  ICMP6 エラーを返送して処理を終了する。
		 */
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   ((uint8_t*)&rth->type - input->buf) - IF_HDR_SIZE);
		return IPPROTO_DONE;
		}
#else
	if (rth->segleft != 0) {

		/*
		 *  残りの中継ノード数が 1 以上であれば、このノードは最終的な
		 *  終点ノードではない。本実装では、中継をしないので、
		 *  ICMP6 エラーを返送して処理を終了する。
		 */
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   ((uint8_t*)&rth->type - input->buf) - IF_HDR_SIZE);
		return IPPROTO_DONE;
		}
#endif

	*nextp = *offp - IF_HDR_SIZE + offsetof(T_IP6_RT_HDR, next);
	*offp += len;
	return rth->next;

buf_rel:
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}

#endif /* of #ifdef _IP6_CFG */
