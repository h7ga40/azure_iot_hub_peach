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

/*	$FreeBSD: src/sys/netinet6/frag6.c,v 1.9 2002/04/19 04:46:22 suz Exp $	*/
/*	$KAME: frag6.c,v 1.33 2002/01/07 11:34:48 kjc Exp $	*/

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

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
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
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_var.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet6/nd6.h>

#ifdef _IP6_CFG

#ifdef IP6_CFG_FRAGMENT

/*
 *  データグラム再構成キュー配列
 */

static T_NET_BUF	*ip6_frag_queue[NUM_IP6_FRAG_QUEUE];
static T_IN6_ADDR	 ip6_frag_dest [NUM_IP6_FRAG_QUEUE];

/*
 *  ip6_get_frag_queue -- データグラム再構成キューを獲得する。
 */

const T_NET_BUF **
ip6_get_frag_queue (void)
{
	return (const T_NET_BUF **)ip6_frag_queue;
	}

/*
 *  frag6_free_queue -- データグラム再構成キューを解放する。
 *
 *    注意: データグラム再構成キュー配列をロックしてから呼び出すこと。
 */

static void
frag6_free_queue (T_NET_BUF **queue)
{
	T_NET_BUF 	*frag, *next;
	T_QIP6_HDR	*qip6h;

	frag = *queue;
	while (frag != NULL) {
		qip6h = GET_QIP6_HDR(frag);
		next  = qip6h->next_frag;
		syscall(rel_net_buf(frag));
		frag = next;
		}
	*queue = NULL;
	}

/*
 *  frag6_timer -- データグラム再構成管理タイマ
 */

void
frag6_timer (void)
{
	T_NET_BUF	*frag;
	T_QIP6_HDR	*qip6h;
	T_IP6_FRAG_HDR	*qip6fh;
	int_t		ix;

	syscall(wai_sem(SEM_IP6_FRAG_QUEUE));
	for (ix = NUM_IP6_FRAG_QUEUE; ix -- > 0; ) {
		frag = ip6_frag_queue[ix];
		if (frag != NULL) {
			qip6h = GET_QIP6_HDR(frag);
			if (qip6h->ftim > 0 && -- qip6h->ftim == 0) {

				qip6fh = (T_IP6_FRAG_HDR *)(frag->buf + qip6h->foff);
				if (ntohs(qip6fh->off_flag & IP6F_OFF_MASK) == 0) {

					/*
					 *  先頭の断片のオフセットが 0 の時のみ
					 *  ICMPv6 エラーメッセージを送信する。
					 */

					/* 先頭の断片をデータグラム再構成キューから外す。*/
					ip6_frag_queue[ix] = qip6h->next_frag;

					/* あて先アドレスを元に戻す。*/
					GET_IP6_HDR(frag)->dst = ip6_frag_dest[ix];

					/* ICMPv6 エラーメッセージを送信する。*/
					icmp6_error(frag, ICMP6_TIME_EXCEEDED,
					                  ICMP6_TIME_EXCEED_REASSEMBLY, 0);
					}

				/* データグラム再構成キューを解放する。*/
				NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_IN_TMOUT], 1);
				NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_IN_DROP], 1);
				frag6_free_queue(&ip6_frag_queue[ix]);
				}
			}
		}
	syscall(sig_sem(SEM_IP6_FRAG_QUEUE));
	}

#endif	/* #ifdef IP6_CFG_FRAGMENT */

/*
 *  frag6_input -- 断片ヘッダの入力関数
 */

uint_t
frag6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_IP6_HDR	*ip6h;
	T_IP6_FRAG_HDR	*ip6fh;
	T_NET_BUF	*input = *inputp;
	uint_t		off = *offp;

#ifdef IP6_CFG_FRAGMENT

	T_QIP6_HDR	*qip6h, *rip6h;
	T_IP6_FRAG_HDR	*qip6fh = NULL;
	T_NET_BUF	**ip6fq = NULL, *prev, *next, *frag;
	int_t		ix;
	int32_t		unfraglen, diff;
	uint16_t	fragpartlen, qfragpartlen, fragoff, qfragoff, plen;
	uint8_t		ftim, *prev_next;
	uint_t		nextproto = IPPROTO_DONE;

#endif	/* #ifdef IP6_CFG_FRAGMENT */

	/* ネットワークバッファの残りの長さをチェックする。*/
	if (input->len - off < sizeof(T_IP6_FRAG_HDR))
		goto buf_rel;

	/* 
	 *  巨大ペイロードオプションと断片ヘッダを同時に使用することは
	 *  できない。ペイロード長がが 0 なら、巨大ペイロードオプションが
	 *  指定されていることになるので ICMP でエラーを送信者に
	 *  通知する。
	 */
	ip6h = GET_IP6_HDR(input);
	if (ip6h->plen == 0) {
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   off - IF_HDR_SIZE);
		NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmFails, 1);
		return IPPROTO_DONE;
		}

	/*
	 *  オフセットが 0 で、最終フラグメントであれば、
	 *  分割不要のデータグラムなので、
	 *  次ヘッダを指す変数を調整して終了する。
	 */
	ip6fh = (T_IP6_FRAG_HDR *)(input->buf + off);
	if (ip6fh->off_flag == 0) {
		*nextp = *offp - IF_HDR_SIZE + offsetof(T_IP6_FRAG_HDR, next);
		*offp = off + sizeof(T_IP6_FRAG_HDR);
		return ip6fh->next;
		}

#ifdef IP6_CFG_FRAGMENT

	/*
	 *  ペイロード長をチェックする。
	 */
	if ((ip6fh->off_flag & IP6F_MORE_FRAG) && 
	    ((ntohs(ip6h->plen) - (off - IF_HDR_SIZE)) & 0x7) != 0) {
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_HEADER,
		                   offsetof(T_IP6_HDR, plen));
		NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmFails, 1);
		return nextproto;
		}
	NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmReqds, 1);

	syscall(wai_sem(SEM_IP6_FRAG_QUEUE));

	/*
 	 *  データグラム再構成キュー配列から、対応するエントリを探す。
 	 */
	for (ix = NUM_IP6_FRAG_QUEUE; ix -- > 0; ) {
		if (ip6_frag_queue[ix] != NULL) {
			qip6h  = GET_QIP6_HDR(ip6_frag_queue[ix]);
			qip6fh = (T_IP6_FRAG_HDR *)(ip6_frag_queue[ix]->buf + qip6h->foff);
			if (ip6fh->ident == qip6fh->ident               &&
			    IN6_ARE_ADDR_EQUAL(&ip6h->src, &qip6h->src) &&
			    IN6_ARE_ADDR_EQUAL(&ip6h->dst, &ip6_frag_dest[ix])) {
				ip6fq = &ip6_frag_queue[ix];
			    	break;
			    	}
			}
		}

 	if (ip6fq == NULL) {

		/*
	 	 *  断片の正当性をチェックする。
		 */
		unfraglen = 0;
		fragoff = ntohs(ip6fh->off_flag & IP6F_OFF_MASK);
		if (fragoff == 0) {

			/*
		 	 *  入力した断片が先頭の断片
			 *  分割できない部分の長さ unfraglen を計算する。
			 */
			unfraglen = (off - IF_HDR_SIZE) - sizeof(T_IP6_HDR);
			}

		/*
		 *             off
		 *  |<--------------------------->|
		 *  |          off - IF_HDR_SIZE  |
		 *  |        |<------------------>|
		 *  |        | off - IF_HDR_SIZE + IP6_FRAG_HDR_SIZE |
		 *  |        |<------------------------------------->|
		 *  +--------+----------+---------+------------------+-------------+
		 *  | IF HDR | IPv6 HDR | EXT HDR |     FRAG HDR     |   Payload   |
		 *  +--------+----------+---------+------------------+-------------+
		 *                      |<---------------------------------------->|
		 *                      |             Payload Size                 |
		 *           |<--------------------------------------------------->|
		 *           |         IP6_HDR_SIZE + Payload Size                 |
		 *                                                   |<----------->|
		 *                                                   | fragpartlen |
		 */
		fragpartlen = sizeof(T_IP6_HDR) + ntohs(ip6h->plen) - (off - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));
		if (unfraglen + fragoff + fragpartlen > IPV6_MAXPACKET) {
			icmp6_error(input, ICMP6_PARAM_PROB,
			                   ICMP6_PARAMPROB_HEADER,
			                   (off - IF_HDR_SIZE) +
			                   offsetof(T_IP6_FRAG_HDR, off_flag));
			goto sig_ret;
			}

		/*
	 	 *  ip6fq == NULL であれば、対応するエントリは無いので、
	 	 *  データグラム再構成キュー配列に、エントリを追加する。
	 	 */
		for (ix = NUM_IP6_FRAG_QUEUE; ix -- > 0; ) {

			/* 空きエントリを探す。*/
			if (ip6_frag_queue[ix] == NULL) {
				ip6fq = &ip6_frag_queue[ix];
				break;
				}
			}

		/*
		 *  ip6fq == NULL であれば、空きエントリが無いので、
		 *  タイムアウト（hlim）が短いエントリを開放する。
		 */
 		if (ip6fq == NULL) {

		 	ftim = IPV6_MAXHLIM;
			for (ix = NUM_IP6_FRAG_QUEUE; ix -- > 0; ) {
				if (ip6_frag_queue[ix] != NULL) {
					rip6h  = GET_QIP6_HDR(ip6_frag_queue[ix]);
					if (rip6h->ftim < ftim) {
						ftim  = rip6h->ftim;
						ip6fq = &ip6_frag_queue[ix];
						}
					}
				}
			frag6_free_queue(ip6fq);
			}

		/*
		 *  エントリを設定する。
		 */
		*ip6fq = input; 
		ip6_frag_dest[ip6fq - ip6_frag_queue] = ip6h->dst;
		qip6h  = GET_QIP6_HDR(*ip6fq);
		qip6h->ftim = IPV6_FRAGTTL;
		qip6h->foff = off;
		qip6h->flen = fragpartlen;
		qip6h->next_frag = NULL;
 		}
 	else {

		/*
	 	 *  断片の正当性をチェックする。
		 */
		unfraglen = 0;
		fragoff = ntohs(((T_IP6_FRAG_HDR *)((*ip6fq)->buf + GET_QIP6_HDR(*ip6fq)->foff))->off_flag & IP6F_OFF_MASK);
		if (fragoff == 0) {

			/*
		 	 *  データグラム再構成キューの最初の断片が先頭の断片
			 *  分割できない部分の長さ unfraglen を計算する。
			 */
			unfraglen = (GET_QIP6_HDR(*ip6fq)->foff - IF_HDR_SIZE) - sizeof(T_IP6_HDR);
			}

		fragoff = ntohs(ip6fh->off_flag & IP6F_OFF_MASK);
		if (fragoff == 0) {

			/*
		 	 *  入力した断片が先頭の断片
			 *  分割できない部分の長さ unfraglen を計算する。
			 */
			unfraglen = (off - IF_HDR_SIZE) - sizeof(T_IP6_HDR);
			}

		/*
		 *             off
		 *  |<--------------------------->|
		 *  |          off - IF_HDR_SIZE  |
		 *  |        |<------------------>|
		 *  |        | off - IF_HDR_SIZE + IP6_FRAG_HDR_SIZE |
		 *  |        |<------------------------------------->|
		 *  +--------+----------+---------+------------------+-------------+
		 *  | IF HDR | IPv6 HDR | EXT HDR |     FRAG HDR     |   Payload   |
		 *  +--------+----------+---------+------------------+-------------+
		 *                      |<---------------------------------------->|
		 *                      |             Payload Size                 |
		 *           |<--------------------------------------------------->|
		 *           |         IP6_HDR_SIZE + Payload Size                 |
		 *                                                   |<----------->|
		 *                                                   | fragpartlen |
		 */
		fragpartlen = sizeof(T_IP6_HDR) + ntohs(ip6h->plen) - (off - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));
		if (unfraglen + fragoff + fragpartlen > IPV6_MAXPACKET) {
			icmp6_error(input, ICMP6_PARAM_PROB,
			                   ICMP6_PARAMPROB_HEADER,
			                   (off - IF_HDR_SIZE) +
			                   offsetof(T_IP6_FRAG_HDR, off_flag));
			goto sig_ret;
			}
		
		if (ntohs(ip6fh->off_flag & IP6F_OFF_MASK) == 0) {

			/*
		 	 *  入力した断片が先頭の断片
		 	 *  データグラム再構成キューの最後の断片が、
		 	 *  データグラムの最大長を超えていないかチェックする。
			 */

			/* 最後の断片を探索する。*/
			next = *ip6fq;
			while ((qip6h = GET_QIP6_HDR(next))->next_frag != NULL)
				next = qip6h->next_frag;

			/* 最後の断片のオフセットと断片サイズを計算する。*/
			qip6fh = (T_IP6_FRAG_HDR *)(next->buf + qip6h->foff);
			fragoff = ntohs(qip6fh->off_flag & IP6F_OFF_MASK);
			fragpartlen = sizeof(T_IP6_HDR) + ntohs(qip6h->plen) - (qip6h->foff - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));
			if (unfraglen + fragoff + fragpartlen > IPV6_MAXPACKET) {

				/*
				 *  データグラムの最大長を超えている場合は、
			 	 *  データグラム再構成キュー全体を破棄する。
			 	 */
				frag6_free_queue(ip6fq);
				syscall(rel_net_buf(input));
				goto sig_ret;
				}
			}

		/* ECN の正当性チェックは未実装 */

		/*
		 *  断片をエントリに挿入する。
	 	 *  データグラム再構成キューから、入力した断片より前の断片を探索する。
		 */
		prev = NULL;
		next = *ip6fq;
		while (next != NULL) {
			qip6h  = GET_QIP6_HDR(next);
			qip6fh = (T_IP6_FRAG_HDR *)(next->buf + qip6h->foff);
			if (ntohs(ip6fh->off_flag & IP6F_OFF_MASK) <= ntohs(qip6fh->off_flag & IP6F_OFF_MASK))
				break;
			prev = next;
			next = qip6h->next_frag;
			}

		/*
		 *  prev は、入力した断片より前の断片。ただし NULL の場合は、
		 *  入力した断片がデータグラム再構成キューの中で最も前の断片。
		 *  next は、入力した断片より後の断片。ただし NULL の場合は、
		 *  入力した断片がデータグラム再構成キューの中で最も後の断片。
		 */
		fragoff = ntohs(ip6fh->off_flag & IP6F_OFF_MASK);
		if (prev != NULL) {

			/*
			 *  前の断片と、入力した断片が重なっていないかチェックする。
			 *
			 *    qfragoff
			 *    |   qfragparglen
			 *    |<------------------>|
			 *    +--------------------+
			 *    |        prev        |
			 *    +--------------------+
			 *
			 *                         +--------------------+
			 *                         |        input       |
			 *                         +--------------------+
			 *                         |
			 *                         fragoff
			 */
			qip6h  = GET_QIP6_HDR(prev);
			qip6fh = (T_IP6_FRAG_HDR *)(prev->buf + qip6h->foff);
			qfragoff = ntohs(qip6fh->off_flag & IP6F_OFF_MASK);
			qfragpartlen = sizeof(T_IP6_HDR) + ntohs(qip6h->plen) - (off - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));

			diff = ((int32_t)qfragoff + qfragpartlen) - fragoff;
			if (diff > 0) {

				/* 重なっていれば、入力した断片を破棄して終了する。*/
				syscall(rel_net_buf(input));
				goto sig_ret;
				}
			}

		if (next != NULL) {

			/*
			 *  後の断片と、入力した断片が重なっていないかチェックする。
			 *
			 *    fragoff
			 *    |    fragparglen
			 *    |<------------------>|
			 *    +--------------------+
			 *    |       input        |
			 *    +--------------------+
			 *
			 *                         +--------------------+
			 *                         |         next       |
			 *                         +--------------------+
			 *                         |
			 *                         qfragoff
			 */
			qip6h  = GET_QIP6_HDR(next);
			qip6fh = (T_IP6_FRAG_HDR *)(next->buf + qip6h->foff);
			qfragoff = ntohs(qip6fh->off_flag & IP6F_OFF_MASK);
			fragpartlen = sizeof(T_IP6_HDR) + ntohs(ip6h->plen) - (off - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));

			diff = ((int32_t)fragoff + fragpartlen) - qfragoff;
			if (diff > 0) {

				/* 重なっていれば、入力した断片を破棄して終了する。*/
				syscall(rel_net_buf(input));
				goto sig_ret;
				}
			}

		/* 入力した断片を設定する。*/
		qip6h  = GET_QIP6_HDR(input);
		qip6h->foff = off;
		qip6h->flen = sizeof(T_IP6_HDR) + ntohs(qip6h->plen) - (off - IF_HDR_SIZE + sizeof(T_IP6_FRAG_HDR));

		if (prev == NULL) {
			*ip6fq = input;
			qip6h->ftim = IPV6_FRAGTTL;
			}
		else
			GET_QIP6_HDR(prev)->next_frag = input;
		qip6h->next_frag = next;

		/*
		 *  全ての断片を受信したかチェックする。
		 */
		plen = 0;
		frag = *ip6fq;
		while (frag != NULL) {
			qip6h  = GET_QIP6_HDR(frag);
			qip6fh = (T_IP6_FRAG_HDR *)(frag->buf + qip6h->foff);
			if (ntohs(qip6fh->off_flag & IP6F_OFF_MASK) != plen) {

				/* 断片が連続していない。*/
				goto sig_ret;
				}
			plen += qip6h->flen;
			frag = qip6h->next_frag;
			}
		if (ntohs(qip6fh->off_flag & IP6F_MORE_FRAG) != 0) {

			/* 最後の断片を受信していない。*/
			goto sig_ret;
			}

		/*
		 *  全ての断片を受信したので、再構成する。
		 */

		/* ネットワークバッファを獲得する。*/
		qip6h  = GET_QIP6_HDR(*ip6fq);
		qip6fh = (T_IP6_FRAG_HDR *)((*ip6fq)->buf + qip6h->foff);
		if (tget_net_buf(inputp, qip6h->foff + plen, TMO_IP6_FRAG_GET_NET_BUF) == E_OK) {

			/*
			 *  断片ヘッダの直前のヘッダの NEXT フィールドのアドレスを獲得し、
			 *  元に戻す。
			 */
			if ((prev_next = ip6_get_prev_hdr(*ip6fq, qip6h->foff)) == NULL) {
				syscall(rel_net_buf(input));
				goto sig_ret;
				}
			*prev_next = qip6fh->next;
			input = *inputp;

			NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_IN_OK], 1);
			NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmOKs, 1);

			/*
			 *  IPv6 ヘッダと分割できない部分を
			 *  データグラム再構成キューの中の先頭の断片からコピーする。
			 */
			memcpy(input->buf, (*ip6fq)->buf, qip6h->foff);

			/* 送信先アドレスを元に戻す。*/
			ip6h = GET_IP6_HDR(input);
			memcpy(&ip6h->dst, &ip6_frag_dest[ip6fq - ip6_frag_queue], sizeof(T_IN6_ADDR));

			/* offp を、再構成した断片の先頭に設定する。*/
			*offp = qip6h->foff;

			/* ネットワークバッファ長を設定する。*/
			input->len = plen + qip6h->foff;

			/*
			 *  全ての断片をコピーする。
			 */
			frag = *ip6fq;
			off  = qip6h->foff;
			while (frag != NULL) {
				qip6h  = GET_QIP6_HDR(frag);
				memcpy(input->buf + off,
				       frag->buf + qip6h->foff + sizeof(T_IP6_FRAG_HDR),
				       qip6h->flen);
				off += qip6h->flen;
				frag = qip6h->next_frag;
				}

			/* ペイロード長を設定する。*/
			ip6h->plen = htons(plen);

			*nextp = offsetof(T_IP6_HDR, next);
			nextproto = ip6h->next;
			}
		else {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_IN_NO_BUF], 1);
			NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_IN_DROP], 1);
			NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmFails, 1);
			}

		/* データグラム再構成キューを解放する。*/
		frag6_free_queue(ip6fq);
 		}

sig_ret:
	syscall(sig_sem(SEM_IP6_FRAG_QUEUE));
	return nextproto;

#else	/* #ifdef IP6_CFG_FRAGMENT */

	/*
	 *  データグラムの分割・再構成行わない場合は、全断片を破棄し、
	 *  最後の断片を受信したとき、
	 *  ICMP エラーを送信者に通知する。
	 */
	if ((ip6fh->off_flag & IP6F_MORE_FRAG) == 0) {
		icmp6_error(input, ICMP6_TIME_EXCEEDED,
		                   ICMP6_TIME_EXCEED_REASSEMBLY, 0);
		NET_COUNT_MIB(in6_stats.ipv6IfStatsReasmFails, 1);
		return IPPROTO_DONE;
		}

#endif	/* #ifdef IP6_CFG_FRAGMENT */

buf_rel:
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}

#endif /* of #ifdef _IP6_CFG */
