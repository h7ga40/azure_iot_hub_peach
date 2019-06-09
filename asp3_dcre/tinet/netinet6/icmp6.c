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

/*	$FreeBSD: src/sys/netinet6/icmp66.c,v 1.6.2.6 2001/07/10 09:44:16 ume Exp $	*/
/*	$KAME: icmp66.c,v 1.211 2001/04/04 05:56:20 itojun Exp $	*/

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

/*
 * Copyright (c) 1982, 1986, 1988, 1993
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
 *	@(#)ip_icmp6.c	8.2 (Berkeley) 1/4/94
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
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
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp_var.h>

#include <netinet6/nd6.h>

#ifdef _IP6_CFG

/*
 *  変数
 */

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

T_ICMP6_IFSTAT icmp6_ifstat;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  icmp6_output -- ICMP6 パケットを送信する。
 */

static void
icmp6_output (T_NET_BUF *output, uint_t off)
{
	T_IN6_IFADDR	*ia;
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IP6_HDR	*ip6h;
	T_ICMP6_HDR	*icmp6h;

	/*
	 *  宛先アドレスにふさわしい送信元アドレスを、
	 *  ネットワークインタフェースから探索して利用する。
	 */
	ip6h = GET_IP6_HDR(output);
	if ((ia = in6_ifawithifp(ifp, &ip6h->dst)) == NULL) {
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutErrors, 1);
		syscall(rel_net_buf(output));
		return;
		}
	memcpy(&ip6h->src, &ia->addr, sizeof(T_IN6_ADDR));

	icmp6h = GET_ICMP6_HDR(output, off);

	/* チェックサムを計算する。*/
	icmp6h->sum = 0;
	icmp6h->sum = in6_cksum(output, IPPROTO_ICMPV6, (uint8_t*)icmp6h - output->buf, 
	                        ntohs(ip6h->plen) - (off - IF_IP6_HDR_SIZE));

	/* 送信する。*/
	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_OUT_OCTETS],
	               output->len - GET_IF_IP6_HDR_SIZE(output));
	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	ip6_output(output, 0, TMO_ICMP_OUTPUT);
	}

/*
 *  icmp6_echo_request -- エコー要求を受信したときの処理
 */

static void
icmp6_echo_request (T_NET_BUF *input, uint_t off)
{
	T_IP6_HDR	*ip6h;
	T_ICMP6_HDR	*icmp6h;
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_IFADDR	*ia;
	T_IN6_ADDR	dst;
	uint_t		diff;

	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInEchos, 1);

	ip6h       = GET_IP6_HDR(input);

	/*
	 *  IPv6 ヘッダと ICMPv6 ヘッダ間の拡張ヘッダを除去する。
	 */
	diff = off - IF_IP6_HDR_SIZE;
	if (diff > 0) {
		/* ICMPv6 ヘッダを前に詰める。*/
		memmove(input->buf + IF_IP6_HDR_SIZE, input->buf + off, input->len - off);
		off = IF_IP6_HDR_SIZE;

		/* ペイロード長を調整する。*/
		ip6h->plen  = htons(ntohs(ip6h->plen) - diff);
		input->len -= diff;
		}

	icmp6h = GET_ICMP6_HDR(input, off);
	if (icmp6h->code != 0) {
		NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS], 1);
		syscall(rel_net_buf(input));
		return;
		}

	/*
	 *  メッセージの型をエコー要求 (128) から
	 *  エコー応答 (129) に変更して送り返す。
	 */
	icmp6h->type = ICMP6_ECHO_REPLY;

	/* IPv6 ヘッダの発信元アドレスと宛先アドレスを入替える。*/
	memcpy(&dst, &ip6h->dst, sizeof(T_IN6_ADDR));
	memcpy(&ip6h->dst, &ip6h->src, sizeof(T_IN6_ADDR));
	memcpy(&ip6h->src, &dst, sizeof(T_IN6_ADDR));

	/*
	 *  元の宛先アドレスがマルチキャストの場合は、
	 *  宛先アドレスにふさわしい送信元アドレスを、
	 *  ネットワークインタフェースから探索して利用する。
	 */
	if (IN6_IS_ADDR_MULTICAST(&dst)) {
		if ((ia = in6_ifawithifp(ifp, &ip6h->dst)) == NULL) {
			syscall(rel_net_buf(input));
			return;
			}
		else
			memcpy(&ip6h->src, &ia->addr, sizeof(T_IN6_ADDR));
		}

	/* 次ヘッダとホップリミットを設定する。*/
	ip6h->next = IPPROTO_ICMPV6;
	ip6h->hlim = IP6_DEFTTL;

	/* チェックサムを計算する。*/
	icmp6h->sum = 0;
	icmp6h->sum = in6_cksum(input, IPPROTO_ICMPV6, (uint8_t*)icmp6h - input->buf, 
	                        ntohs(ip6h->plen) - (off - IF_IP6_HDR_SIZE));

	/* 送信する。*/
	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_OUT_OCTETS],
	               input->len - GET_IF_IP6_HDR_SIZE(input));
	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutEchoReplies, 1);
	ip6_output(input, 0, TMO_ICMP_OUTPUT);
	}

/*
 *  icmp6_notify_error -- エラーを通知する。
 */

static void
icmp6_notify_error(T_NET_BUF *input, uint_t poff)
{
	T_ICMP6_HDR	*icmp6h;
	ER_UINT		loff;
	uint_t		len, next;

	len  = input->len - poff;

	/* データグラム長が ICMP6 ヘッダ長 + IP6 ヘッダ長以下なら何もしない。*/
	if (len < ICMP6_HDR_SIZE + IP6_HDR_SIZE)
		return;

	icmp6h = GET_ICMP6_HDR(input, poff);

	/* 最終ヘッダを探索する。*/
	loff = ip6_lasthdr(input, poff + ICMP6_HDR_SIZE, IPPROTO_IPV6, &next);

	/* 最終ヘッダが TCP/UDP のみ対応する。*/
	if (loff >= 0 && (next == IPPROTO_TCP || next == IPPROTO_UDP)) {

		memcpy(GET_IP6_HDR(input), input->buf + (loff + ICMP6_HDR_SIZE),
		                           input->len - (loff + ICMP6_HDR_SIZE));
		input->len -= loff + ICMP6_HDR_SIZE;

#if defined(SUPPORT_TCP)

		if (next == IPPROTO_TCP)

			tcp_notify(input, icmp6h->code == ICMP6_DST_UNREACH_NOPORT ? EV_CNNRF : EV_HURCH);

#endif	/* of #if defined(SUPPORT_TCP) */

#if defined(SUPPORT_UDP) && TNUM_UDP6_CEPID > 0

		if (next == IPPROTO_UDP)

			udp6_notify(input, icmp6h->code == ICMP6_DST_UNREACH_NOPORT ? EV_CNNRF : EV_HURCH);

#endif	/* of #if defined(SUPPORT_UDP) && TNUM_UDP6_CEPID > 0 */

		}
	else
		syslog(LOG_NOTICE, "[ICMP6] error code: %d.", icmp6h->code);
	}

#if NUM_IN6_HOSTCACHE_ENTRY > 0

/*
 *  icmp6_mtudisc_update -- Path MTU を更新する。
 */

static void
icmp6_mtudisc_update(T_NET_BUF *input, uint_t off, uint32_t mtu)
{
	T_IP6_HDR	*pip6h;

	pip6h = (T_IP6_HDR*)GET_ICMP6_SDU(input, off);

	if (mtu < IP6_HDR_SIZE + sizeof(T_IP6_FRAG_HDR) + 8)
		return;

	if (mtu > IF_MTU)
		return;

	in6_hostcache_update(&pip6h->dst, mtu);
	}

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

#if defined(NUM_IN6_REDIRECT_ROUTE_ENTRY)
#if NUM_IN6_REDIRECT_ROUTE_ENTRY > 0

/*
 *  icmp6_redirect_input -- 向け直しメッセージの処理
 */

static void
icmp6_redirect_input (T_NET_BUF *input, uint_t off)
{
	T_IP6_HDR		*ip6h;
	const T_IN6_ADDR	*gw;
	T_ND_REDIRECT_HDR	*rdh;
	T_ND_OPT_HDR		*opth;
	T_IFNET			*ifp = IF_GET_IFNET();
	bool_t			router, onlink;
	uint_t			lladdr_len = 0;
	uint8_t			*lladdr = NULL;
	uint8_t			nd_opt_off[ND_OPT_OFF_ARRAY_SIZE];

	ip6h = GET_IP6_HDR(input);

	/*
	 *  IP ヘッダのチェック、以下の場合は破棄する。
	 *    ・リンクローカル以外から送られてきたメッセージ
	 *    ・ホップリミットが IPV6_MAXHLIM (255) 以外
	 */
	if (!IN6_IS_ADDR_LINKLOCAL(&ip6h->src) || ip6h->hlim != IPV6_MAXHLIM)
		goto err_ret;

	rdh  = (T_ND_REDIRECT_HDR *)(input->buf + off);

	/* 向け直しの終点アドレスがマルチキャストアドレスの場合は破棄する。*/
	if (IN6_IS_ADDR_MULTICAST(&rdh->dst))
		goto err_ret;

	/* ターゲットアドレスが経路表に登録されているかチェックする。*/
	if ((gw = in6_rtalloc(ifp, &rdh->dst)) != NULL) {
		/* GW アドレスと送信元アドレスが異なっていれば破棄する。*/
		if (!IN6_ARE_ADDR_EQUAL(gw, &ip6h->src))
			goto err_ret;
		}
	else	/* 経路表になければ破棄する。*/
		goto err_ret;

	router = onlink = false;

	/*
	 *  ターゲットアドレスがリンクローカルアドレスであれば、
	 *  ターゲットはルータ
	 */
	if (IN6_IS_ADDR_LINKLOCAL(&rdh->target))
		router = true;

	/*
	 *  ターゲットアドレスと終点アドレスが同一であれば、オンリンク
	 */
	if (IN6_ARE_ADDR_EQUAL(&rdh->target, &rdh->dst))
		onlink = true;
		
	/*
	 *  ルータでも、オンリンクでもなければエラー
	 */
	if (!router && !onlink)
		goto err_ret;

	/* 近隣探索オプションのオフセットを記録する。*/
	if (nd6_options(nd_opt_off, input->buf +  off + ND_REDIRECT_HDR_SIZE,
	                            input->len - (off + ND_REDIRECT_HDR_SIZE)) != E_OK)
		goto err_ret;

	/* 近隣探索オプション (向け直しのルータのリンクアドレス) */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_TARGET_LINKADDR)]) {
		opth = (T_ND_OPT_HDR *)((uint8_t *)(input->buf + off + ND_REDIRECT_HDR_SIZE) +
		                        nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_TARGET_LINKADDR)] - 8);
	 	/* 注意: オプションオフセット配列には、オフセット + 8 が設定されている。*/
		lladdr     = (uint8_t *)(opth + 1);
		lladdr_len = (opth->len << 3);
		}

	/*
	 *  ネットワークインタフェースのアドレス長が一致しなければエラー
	 */
	if (lladdr && lladdr_len != ((sizeof(T_IF_ADDR) + sizeof(T_ND_OPT_HDR) + 7) & ~7))
		goto err_ret;

	/* 向け直しのルータのリンクアドレスを近隣キャッシュに登録する。*/
	nd6_cache_lladdr(ifp, &rdh->target, (T_IF_ADDR *)lladdr, ND_REDIRECT, 
	                 onlink ? ND_REDIRECT_ONLINK : ND_REDIRECT_ROUTER);

	/* ルーティング表に登録する。*/
	in6_rtredirect(&rdh->target, &rdh->dst, 128, IN_RTF_REDIRECT, TMO_IN_REDIRECT);

	syscall(rel_net_buf(input));
	return;

err_ret:
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_IN_ERR_PACKETS], 1);
	syscall(rel_net_buf(input));
	}

#endif	/* of #if NUM_IN6_REDIRECT_ROUTE_ENTRY > 0 */
#endif	/* of #if defined(NUM_IN6_REDIRECT_ROUTE_ENTRY) */

/*
 *  icmp6_input -- ICMP6 の入力関数
 */

uint_t
icmp6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_IP6_HDR	*ip6h;
	T_ICMP6_HDR	*icmp6h;
	uint_t		len, off = *offp;
	uint8_t		code;

	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_OCTETS],
	               input->len - GET_IF_IP6_HDR_SIZE(input));
	NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInMsgs, 1);

	/* ICMP6 ヘッダの長さをチェックする。*/
	len  = input->len - off;
	if (len < ICMP6_HDR_SIZE) {
		NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS], 1);
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInErrors, 1);
		goto buf_rel;
		}

	ip6h   = GET_IP6_HDR(input);
	icmp6h = GET_ICMP6_HDR(input, off);

	/* チェックサムを計算する。*/
	if ((in6_cksum(input, IPPROTO_ICMPV6, off, ntohs(ip6h->plen) - (off - IF_IP6_HDR_SIZE)) & 0xffff) != 0) {
		NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS], 1);
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInErrors, 1);
		goto buf_rel;
		}

	code = icmp6h->code;

	/* メッセージの型により分岐する。*/
	switch (icmp6h->type) {

	case ICMP6_DST_UNREACH:			/* 宛先到達不可能		*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInDestUnreachs, 1);
		icmp6_notify_error(input, off);
		break;

	case ICMP6_ECHO_REQUEST:		/* エコー要求			*/
		icmp6_echo_request(input, off);
		return IPPROTO_DONE;
		break;

	case ICMP6_ECHO_REPLY:			/* エコー応答			*/

#ifdef ICMP_CFG_CALLBACK_ECHO_REPLY

		icmp6_echo_reply(input, off);

#endif	/* of #ifdef ICMP_CFG_CALLBACK_ECHO_REPLY */

		break;

	case ND_ROUTER_ADVERT:			/* ルータ通知			*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInRouterAdvertisements, 1);
		if (code != 0 || len < ROUTER_ADVERT_HDR_SIZE)
			goto buf_rel;
		nd6_ra_input(input, off);
		return IPPROTO_DONE;
		break;

	case ND_NEIGHBOR_SOLICIT:		/* 近隣要請			*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInNeighborSolicits, 1);
		if (code != 0 || len < NEIGHBOR_SOLICIT_HDR_SIZE)
			goto buf_rel;
		nd6_ns_input(input, off);
		return IPPROTO_DONE;
		break;

	case ND_NEIGHBOR_ADVERT:		/* 近隣通知			*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInNeighborAdvertisements, 1);
		if (code != 0 || len < NEIGHBOR_ADVERT_HDR_SIZE)
			goto buf_rel;
		nd6_na_input(input, off);
		return IPPROTO_DONE;
		break;

	case ND_REDIRECT:			/* 向け直し			*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInRedirects, 1);

#if defined(NUM_IN6_REDIRECT_ROUTE_ENTRY)
#if NUM_IN6_REDIRECT_ROUTE_ENTRY > 0

		if (code != 0 || len < ND_REDIRECT_HDR_SIZE)
			goto buf_rel;
		icmp6_redirect_input(input, off);
		return IPPROTO_DONE;

#else	/* of #if NUM_IN6_REDIRECT_ROUTE_ENTRY > 0 */

		syslog(LOG_WARNING, "[ICMP6] redirect ignored.");

#endif	/* of #if NUM_IN6_REDIRECT_ROUTE_ENTRY > 0 */
#else	/* of #if defined(NUM_IN6_REDIRECT_ROUTE_ENTRY) */

		syslog(LOG_WARNING, "[ICMP6] redirect ignored.");

#endif	/* of #if defined(NUM_IN6_REDIRECT_ROUTE_ENTRY) */

		break;

	case ICMP6_PACKET_TOO_BIG:		/* パケットが大きすぎる。	*/

#if NUM_IN6_HOSTCACHE_ENTRY > 0

		icmp6_mtudisc_update(input, off, ntohl(icmp6h->icmp6_mtu));

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInPktTooBigs, 1);
		break;

	case ICMP6_TIME_EXCEEDED:		/* TTL を超過した。		*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInTimeExcds, 1);
		break;

	case ICMP6_PARAM_PROB:			/* IPv6 ヘッダが不正。		*/
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInParmProblems, 1);
		break;

	case MLD_LISTENER_QUERY:		/* マルチキャスト受信者照会	*/
	case MLD_LISTENER_REPORT:		/* マルチキャスト受信者報告	*/
	case MLD_LISTENER_DONE:			/* マルチキャスト受信者終了	*/
	case ND_ROUTER_SOLICIT:			/* ルータ要請			*/
	case ICMP6_ROUTER_RENUMBERING:		/* ルータ番号再設定		*/
	case ICMP6_FQDN_QUERY:			/* FQDN 照会			*/
	case ICMP6_FQDN_REPLY:			/* FQDN 応答			*/
		syslog(LOG_INFO, "[ICMP6] unsupported type: %d.", icmp6h->type);
		break;

	default:
		syslog(LOG_WARNING, "[ICMP6] unknown type: %d.", icmp6h->type);
		NET_COUNT_ICMP6(net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS], 1);
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpInErrors, 1);
		break;
		}

buf_rel:
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}

/*
 *  icmp6_error -- ICMP6 エラー送信関数
 */

void
icmp6_error (T_NET_BUF *input, uint8_t type, uint8_t code, uint32_t param)
{
	T_NET_BUF	*output;
	T_IP6_HDR	*ip6h;
	T_ICMP6_HDR	*icmp6h;
	ER_UINT		off;
	uint_t		next, len;

	ip6h = GET_IP6_HDR(input);

	/*
	 *  宛先アドレスが、マルチキャストの場合は、
	 *  以下の条件以外のときは処理を行わない。
	 *
	 *    ・データグラムが大きすぎる。
	 *    ・IPv6 ヘッダの不正で、オプションが認識できない。
	 */
	if (IN6_IS_ADDR_MULTICAST(&ip6h->dst) &&
	    !((type == ICMP6_PACKET_TOO_BIG) ||
	      (type == ICMP6_PARAM_PROB && code == ICMP6_PARAMPROB_OPTION)))
		goto buf_rel;

	/*
	 *  始点アドレスが、以下の場合は処理を行わない。
	 *
	 *    ・無指定
	 *    ・マルチキャスト
	 */
	if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src) ||
	    IN6_IS_ADDR_MULTICAST  (&ip6h->src))
		goto buf_rel;

	/* 最後のヘッダを探索する。*/
	off = ip6_lasthdr(input, 0, IPPROTO_IPV6, &next);

	/*
	 *  タイプが、エラーか向け直しの ICMP6 パケットのエラーには返答しない。
	 */
	if (off >= 0 && next == IPPROTO_ICMPV6) {
		T_ICMP6_HDR	*icmp6h;

		if (input->len - off < ICMP6_HDR_SIZE)
			goto buf_rel;

		icmp6h = GET_ICMP6_HDR(input, off);
		if (icmp6h->type <  ICMP6_ECHO_REQUEST ||
		    icmp6h->type == ND_REDIRECT)
			goto buf_rel;
		}

	/* DoS 攻撃に対する防御は未実装 */

	/*
	 *  送り返すメッセージ長を、ICMPv6 の最大ペイロード長に調整する。
	 *  IPV6_MMTU - (IP6_HDR_SIZE + ICMP6_HDR_SIZE)
	 */
	if (input->len - IF_HDR_SIZE > IPV6_MMTU - (IP6_HDR_SIZE + ICMP6_HDR_SIZE))
		len = IPV6_MMTU - (IP6_HDR_SIZE + ICMP6_HDR_SIZE);
	else
		len = input->len - IF_HDR_SIZE;

	/* 送信用のネットワークバッファを獲得する。*/
	if (in6_get_datagram(&output, len + ICMP6_HDR_SIZE, 0,
	                     &ip6h->src, &in6_addr_unspecified,
	                     IPPROTO_ICMPV6, IPV6_MAXHLIM,
	                     NBA_SEARCH_ASCENT, TMO_ICMP_OUTPUT) != E_OK)
		goto buf_rel;

	/* ICMP6 ヘッダを設定する。*/
 	icmp6h = GET_ICMP6_HDR(output, IF_IP6_ICMP6_HDR_OFFSET);
 	icmp6h->type       = type;
 	icmp6h->code       = code;
 	icmp6h->icmp6_pptr = htonl(param);
 
 	/* エラーデータを設定する。*/
 	memcpy(GET_ICMP6_SDU(output, IF_IP6_ICMP6_HDR_OFFSET), (uint8_t *)ip6h, len);

#ifdef SUPPORT_MIB

	switch (type) {
	case ICMP6_DST_UNREACH:
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutDestUnreachs, 1);
		break;

	case ICMP6_PARAM_PROB:
		NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutParmProblems, 1);
		break;

	default:
		break;

		}

#endif	/* of #ifdef SUPPORT_MIB */

	/* 送信する。*/
	icmp6_output(output, IF_IP6_HDR_SIZE);

buf_rel:
	syscall(rel_net_buf(input));
	}

#endif /* of #ifdef _IP6_CFG */
