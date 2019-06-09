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

/*	$FreeBSD: src/sys/netinet6/nd6_nbr.c,v 1.13 2002/10/16 01:54:45 sam Exp $	*/
/*	$KAME: nd6_nbr.c,v 1.86 2002/01/21 02:33:04 jinmei Exp $	*/

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
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>

#include <netinet6/nd6.h>

#include <net/if6_var.h>

#ifdef _IP6_CFG

/*
 *  局所関数
 */

static void nd6_dad_timer (T_IN6_IFADDR *ifa);
static void nd6_dad_duplicated (T_IFNET *ifp, T_IN6_IFADDR *ifa);
static void nd6_dad_ns_output (T_IFNET *ifp, T_IN6_IFADDR *ifa);
static void nd6_dad_ns_input (T_IFNET *ifp, T_IN6_IFADDR *ifa);
static void nd6_dad_na_input (T_IFNET *ifp, T_IN6_IFADDR *ifa);

/*
 *  nd6_dad_duplicated -- 重複アドレスを検出した時の処理
 */

static void
nd6_dad_duplicated (T_IFNET *ifp, T_IN6_IFADDR *ifa)
{
	if ((ifa->flags & IN6_IFF_TENTATIVE) == 0)
		syslog(LOG_ERROR, "[ND6 DAD DUP] no pending.");
	else {
		syslog(LOG_ERROR, "[ND6 DAD DUP] dup IPv6 addr: %s.", ipv62str(NULL, &ifa->addr));
		ifa->flags &= ~IN6_IFF_TENTATIVE;
		ifa->flags |=  IN6_IFF_DUPLICATED;

		/* タイマーを停止する。*/
		untimeout((callout_func)nd6_dad_timer, ifa);

		/*
		 *  もし IPv6 アドレスが、ユニークに割り当てらた
		 * （イーサネット用の EUI-64）ハードウェアアドレスをベースとした
		 *  インタフェース ID で生成されたのであれば、
		 *  このインタフェースでの IPv6 の動作は無効にすべきである
		 * （RFC2462bis-03 セクション 5.4.5）。
		 */
		if (IN6_IS_ADDR_LINKLOCAL(&ifa->addr))
			ifp->flags |= ND6_IFF_IFDISABLED;
		}
	}

/*
 *  nd6_dad_ns_output -- 重複アドレス検出 (DAD) 要請を出力する。
 */

static void
nd6_dad_ns_output (T_IFNET *ifp, T_IN6_IFADDR *ifa)
{
	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_DAD_OUT_PACKETS], 1);

	ifa->ns_ocount ++;
	nd6_ns_output(ifp, &in6_addr_unspecified, &ifa->addr, NULL, true);
	}

/*
 *  nd6_dad_ns_input -- 重複アドレス検出 (DAD) 要請入力
 */

static void
nd6_dad_ns_input (T_IFNET *ifp, T_IN6_IFADDR *ifa)
{
	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NS_IN_PACKETS], 1);

	/*
	 *  まだ、重複アドレス検出要請を出力していない場合
	 */
	if (ifa->ns_ocount == 0)
		nd6_dad_duplicated(ifp, ifa);
	else
		ifa->ns_icount ++;
	}

/*
 *  nd6_dad_na_input -- 重複アドレス検出 (DAD) 通知入力
 */

static void
nd6_dad_na_input (T_IFNET *ifp, T_IN6_IFADDR *ifa)
{
	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NA_IN_PACKETS], 1);
	ifa->na_icount ++;
	nd6_dad_duplicated(ifp, ifa);
	}

/*
 *  nd6_dad_timer -- 重複アドレス検出 (DAD) タイマー
 */

static void
nd6_dad_timer (T_IN6_IFADDR *ifa)
{
	T_IFNET *ifp = IF_GET_IFNET();

	if (ifa->flags & IN6_IFF_DUPLICATED) {
		syslog(LOG_ERROR, "[ND6 DAD] dup IPv6 addr: %s.", ipv62str(NULL, &ifa->addr));
		return;
		}

	if ((ifa->flags & IN6_IFF_TENTATIVE) == 0) {
		syslog(LOG_ERROR, "[ND6 DAD] no tentative IPv6 addr: %s.", ipv62str(NULL, &ifa->addr));
		return;
		}

	if (ifa->ns_ocount < NUM_IP6_DAD_COUNT) {
		nd6_dad_ns_output(ifp, ifa);

		/* タイムアウトを近隣探索の送信間隔に設定する。*/
		timeout((callout_func)nd6_dad_timer, ifa, ND6_RETRANS_TIME * NET_TIMER_HZ / SYSTIM_HZ);
		}
	else if (ifa->na_icount || ifa->ns_icount)
		nd6_dad_duplicated(ifp, ifa);
	else {
		ifa->flags &= ~IN6_IFF_TENTATIVE;
		syslog(LOG_NOTICE, "[ND6 DAD] no dup IPv6 addr: %s.", ipv62str(NULL, &ifa->addr));
		}
	}

/*
 *  nd6_ns_input -- 近隣要請の入力処理。
 */

void
nd6_ns_input (T_NET_BUF *input, uint_t off)
{
	T_IFNET			*ifp = IF_GET_IFNET();
	T_IP6_HDR		*ip6h;
	T_NEIGHBOR_SOLICIT_HDR	*nsh;
	T_ND_OPT_HDR		*opth;
	T_IN6_IFADDR		*ifa;
	bool_t			tlladdr;
	uint_t			lladdr_len = 0;
	uint32_t		flags;
	uint8_t			*lladdr = NULL;
	uint8_t			nd_opt_off[ND_OPT_OFF_ARRAY_SIZE];

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NS_IN_PACKETS], 1);

	ip6h = GET_IP6_HDR(input);

	/*
	 *  ヘッダのチェック、以下の場合は破棄する。
	 *    ・ホップリミットが IPV6_MAXHLIM (255) 以外
	 *    ・ヘッダ長が短い
	 */
	if (ip6h->hlim != IPV6_MAXHLIM || input->len - off < NEIGHBOR_SOLICIT_HDR_SIZE)
		goto err_ret;

	nsh  = (T_NEIGHBOR_SOLICIT_HDR *)(input->buf + off);

	if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src)) {
		/*
		 *  始点アドレスが無指定なら、重複アドレス検出
		 *  あて先アドレスは、要請マルチキャストでなければならない。
		 */
		if (!IN6_IS_ADDR_NS_MULTICAST(&ip6h->dst))
			goto err_ret;
		}

	/* 目的アドレスがマルチキャストならエラー */
	if (IN6_IS_ADDR_MULTICAST(&nsh->target))
		goto err_ret;

	/* 近隣探索オプションのオフセットを記録する。*/
	if (nd6_options(nd_opt_off, input->buf + (off + NEIGHBOR_SOLICIT_HDR_SIZE),
	                            input->len - (off + NEIGHBOR_SOLICIT_HDR_SIZE)) != E_OK)
		goto err_ret;

	/* 近隣探索オプション (始点リンクアドレス) */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_SOURCE_LINKADDR)]) {
		opth = (T_ND_OPT_HDR *)((uint8_t *)(input->buf + off + NEIGHBOR_SOLICIT_HDR_SIZE) +
		                        nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_SOURCE_LINKADDR)] - 8);
	 	/* 注意: オプションオフセット配列には、オフセット + 8 が設定されている。*/
		lladdr     = (uint8_t *)(opth + 1);
		lladdr_len = (opth->len << 3);
		}

	if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src) && lladdr != NULL)
		goto err_ret;

	/* 宛先アドレスがマルチキャストなら近隣通知にデータリンク層のアドレスを付ける。*/
	if (IN6_IS_ADDR_MULTICAST(&ip6h->dst))
		tlladdr = true;
	else
		tlladdr = false;

	/*
	 *  目的アドレスが、自分のネットワークインタフェースに
	 *  割り当てられているアドレスか調べる。
	 *  なお、代理サービスは実装していない。
	 */
	ifa = in6_lookup_ifaddr(ifp, &nsh->target);

	if (ifa == NULL)
		goto free_ret;

	/* 探索結果アドレスが重複していれば応答しないで終了する。*/
	if (ifa->flags & IN6_IFF_DUPLICATED)
		goto err_ret;

	/*
	 *  ネットワークインタフェースのアドレス長が一致しなければエラー
	 */
	if (lladdr && lladdr_len != ((sizeof(T_IF_ADDR) + sizeof(T_ND_OPT_HDR) + 7) & ~7))
		goto err_ret;

	/*
	 *  始点アドレスが自分のアドレスと一致すれば重複している。
	 */
	if (IN6_ARE_ADDR_EQUAL(&ifa->addr, &ip6h->src))
		goto free_ret;

	/* 重複検出中に、近隣要請を受信したときの処理 */
	if (ifa->flags & IN6_IFF_TENTATIVE) {
		if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src))
			nd6_dad_ns_input(ifp, ifa);
		goto free_ret;
		}

	if ((ifa->flags & IN6_IFF_ANYCAST) || !tlladdr)
		flags = 0;
	else
		flags = ND_NA_FLG_OVERRIDE;

	/*
	 *  始点アドレスが無指定なら、送信相手は重複アドレス検出中で、
	 *  直接送信することはできないので、全ノードマルチキャスト
	 *  アドレスに送信する。
	 */
	if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src)) {
		nd6_na_output(ifp, &in6_addr_linklocal_allnodes, &nsh->target, flags, tlladdr);
		goto free_ret;
		}

	/* 近隣キャッシュに登録する。*/
	nd6_cache_lladdr(ifp, &ip6h->src, (T_IF_ADDR *)lladdr, ND_NEIGHBOR_SOLICIT, 0);

	nd6_na_output(ifp, &ip6h->src, &nsh->target, flags | ND_NA_FLG_SOLICITED, tlladdr);

free_ret:
	syscall(rel_net_buf(input));
	return;

err_ret:
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_IN_ERR_PACKETS], 1);
	syscall(rel_net_buf(input));
	}

/*
 *  nd6_ns_output -- 近隣要請を出力する。
 */

void
nd6_ns_output (T_IFNET *ifp, const T_IN6_ADDR *daddr,
               const T_IN6_ADDR *taddr, T_LLINFO_ND6 *ln, bool_t dad)
{
	T_NEIGHBOR_SOLICIT_HDR	*nsh;
	T_NET_BUF		*output;
	T_IP6_HDR		*ip6h;
	T_IF_ADDR		*mac = NULL;
	uint_t			len;
	uint16_t		ipflags = 0;

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NS_OUT_PACKETS], 1);

	if (IN6_IS_ADDR_MULTICAST(taddr))
		return;

	/* 近隣要請ペイロード長を計算する。*/
	if (!dad && IF_SOFTC_TO_IFADDR(ifp->ic))
		len = (NEIGHBOR_SOLICIT_HDR_SIZE + ND_OPT_HDR_SIZE + sizeof(T_IF_ADDR) + 7) >> 3 << 3;
	else
		len = (NEIGHBOR_SOLICIT_HDR_SIZE + 7) >> 3 << 3;

	/*
	 *  要請マルチキャスト・アドレス宛の近隣探索では、
	 *  他のリンクからの偽造されたデータグラムを
	 *  排除するため、ホップリミットに IPV6_MAXHLIM (255) を設定する。
	 */
	if (IN6_IS_ADDR_UNSPECIFIED(daddr) || IN6_IS_ADDR_MULTICAST(daddr))
		ipflags = IPV6_OUT_SET_HOP_LIMIT(IPV6_OUT_FLG_HOP_LIMIT, IPV6_MAXHLIM);

	/* ネットワークバッファを獲得し、IPv6 ヘッダを設定する。*/
	if (in6_get_datagram(&output, len, 0, daddr, NULL,
	                     IPPROTO_ICMPV6, IPV6_MAXHLIM,
	                     NBA_SEARCH_ASCENT, TMO_ND6_NS_OUTPUT) != E_OK)
		return;

	ip6h = GET_IP6_HDR(output);

	if (IN6_IS_ADDR_UNSPECIFIED(daddr)) {

		/* 宛先アドレスが無指定の場合は、要請マルチキャストアドレスを設定する。*/
		ip6h->dst.s6_addr32[0] = IPV6_ADDR_INT32_MLL;
		ip6h->dst.s6_addr32[2] = IPV6_ADDR_INT32_ONE;
		ip6h->dst.s6_addr32[3] = taddr->s6_addr32[3];
		ip6h->dst.s6_addr8[12] = 0xff;
		}

	/* 送信元アドレスの設定 */
	if (!dad) {

		/* 重複アドレス検出ではない時の送信元アドレス決定処理。*/
		T_IN6_ADDR *saddr;

		if (ln && ln->hold) {
			/*
			 * 送信がペンディングされているデータグラムの
			 * 送信元アドレスを利用する。
			 */
			if (ln->hold->len > IF_IP6_HDR_SIZE)
				saddr = &GET_IP6_HDR(ln->hold)->src;
			else
				saddr = NULL;
			}
		else
			saddr = NULL;

		if (saddr && in6_lookup_ifaddr(ifp, saddr))
			memcpy(&ip6h->src, saddr, sizeof(T_IN6_ADDR));
		else {
			/*
			 *  宛先アドレスにふさわしい送信元アドレスを、
			 *  ネットワークインタフェースから探索して利用する。
			 */
			T_IN6_IFADDR *ifa;

			if ((ifa = in6_ifawithifp(ifp, &ip6h->dst)) == NULL) {
				syscall(rel_net_buf(output));
				return;
				}
			memcpy(&ip6h->src, &ifa->addr, sizeof(T_IN6_ADDR));
			}
		}
	else {

		/* 重複アドレス検出時の送信元アドレスは無指定。*/
		memset(&ip6h->src, 0, sizeof(T_IN6_ADDR));
		}

 	/* 近隣要請ヘッダを設定する。*/
 	nsh = GET_NEIGHBOR_SOLICIT_HDR(output, IF_IP6_NEIGHBOR_SOLICIT_HDR_OFFSET);
 	nsh->hdr.type        = ND_NEIGHBOR_SOLICIT;
 	nsh->hdr.code        = 0;
 	nsh->hdr.data.data32 = 0;
	memcpy(&nsh->target, taddr, sizeof(T_IN6_ADDR));

	if (!dad && (mac = IF_SOFTC_TO_IFADDR(ifp->ic)) != NULL) {

		/* 近隣探索オプションとして、探索目標の MAC アドレスを設定する。*/
		T_ND_OPT_HDR	*opth;
		uint_t		optlen;

		opth   = (T_ND_OPT_HDR *)GET_NEIGHBOR_SOLICIT_SDU(output, IF_IP6_NEIGHBOR_SOLICIT_HDR_OFFSET);
		optlen = (ND_OPT_HDR_SIZE + sizeof(T_IF_ADDR) + 7) >> 3 << 3;
		memset(opth, 0, optlen);
		opth->type = ND_OPT_SOURCE_LINKADDR;
		opth->len  = optlen >> 3;
		memcpy((uint8_t *)opth + sizeof(T_ND_OPT_HDR), mac, sizeof(T_IF_ADDR));
		}

	/* チェックサムを計算する。*/
	nsh->hdr.sum = 0;
	nsh->hdr.sum = in6_cksum(output, IPPROTO_ICMPV6, (uint8_t*)nsh - output->buf, len);

	/* 送信する。*/
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_OCTETS],
	               output->len - GET_IF_IP6_HDR_SIZE(output));
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutNeighborSolicits, 1);
	ip6_output(output, ipflags | IPV6_OUT_FLG_DAD, TMO_ND6_NS_OUTPUT);
	}

/*
 *  nd6_na_input -- 近隣通知の入力処理。
 */

void
nd6_na_input (T_NET_BUF *input, uint_t off)
{
	T_IFNET			*ifp = IF_GET_IFNET();
	T_IP6_HDR		*ip6h;
	T_NEIGHBOR_ADVERT_HDR	*nah;
	T_ND_OPT_HDR		*opth;
	T_IN6_IFADDR		*ifa;
	T_LLINFO_ND6		*ln;
	SYSTIM			now;
	bool_t			llchange;
	uint_t			lladdr_len = 0;
	uint8_t			*lladdr = NULL;
	uint8_t			nd_opt_off[ND_OPT_OFF_ARRAY_SIZE];

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NS_IN_PACKETS], 1);

	ip6h = GET_IP6_HDR(input);

	/*
	 *  ヘッダのチェック、以下の場合は破棄する。
	 *    ・ホップリミットが IPV6_MAXHLIM (255) 以外
	 *    ・ヘッダ長が短い
	 */
	if (ip6h->hlim != IPV6_MAXHLIM || input->len - off < NEIGHBOR_ADVERT_HDR_SIZE)
		goto err_ret;

	nah  = (T_NEIGHBOR_ADVERT_HDR *)(input->buf + off);

	/* 目的アドレスがマルチキャストならエラー */
	if (IN6_IS_ADDR_MULTICAST(&nah->target))
		goto err_ret;

	/* 近隣要請への応答で、宛先アドレスがマルチキャストならエラー */
	if ((nah->nd_na_flags_reserved & ND_NA_FLG_SOLICITED) &&
	    IN6_IS_ADDR_MULTICAST(&ip6h->dst))
		goto err_ret;

	/* 近隣探索オプションのオフセットを記録する。*/
	if (nd6_options(nd_opt_off, input->buf + (off + NEIGHBOR_ADVERT_HDR_SIZE), 
	                            input->len - (off + NEIGHBOR_ADVERT_HDR_SIZE)) != E_OK)
		goto err_ret;

	/* 近隣探索オプション (目的リンクアドレス) */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_TARGET_LINKADDR)]) {
		opth = (T_ND_OPT_HDR *)((uint8_t *)(input->buf + off + NEIGHBOR_ADVERT_HDR_SIZE) +
		                        nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_TARGET_LINKADDR)] - 8);
	 	/* 注意: オプションオフセット配列には、オフセット + 8 が設定されている。*/
		lladdr     = (uint8_t *)(opth + 1);
		lladdr_len = (opth->len << 3);
		}

	ifa = in6_lookup_ifaddr(ifp, &nah->target);

	/*
	 *  目的アドレスが自分のネットワークインタフェースに割り当てられているアドレスの
	 *  いずれかに一致したときは、重複していることを意味している。
	 */
	if (ifa) {
		if (ifa->flags & IN6_IFF_TENTATIVE)
			nd6_dad_na_input(ifp, ifa);
		else
			syslog(LOG_ERROR, "[ND6 NA INPUT] dup IPv6 addr: %s.", ipv62str(NULL, &ifa->addr));
		goto err_ret;
		}

	/*
	 *  ネットワークインタフェースのアドレス長が一致しなければエラー
	 */
	if (lladdr && lladdr_len != ((sizeof(T_IF_ADDR) + sizeof(T_ND_OPT_HDR) + 7) & ~7))
		goto err_ret;

	/* 近隣キャッシュを探索する。*/
	syscall(wai_sem(SEM_ND6_CACHE));
	if ((ln = nd6_lookup(&nah->target, false)) == NULL)
		goto free_ret;

	/* 近隣キャッシュの状態が、データリンク層のアドレス未解決の場合 */
	if (ln->state == ND6_LLINFO_INCOMPLETE) {
		/* 通知されたデータリンク層のアドレスの長さが 0 の場合 */
		if (lladdr == NULL)
			goto free_ret;

		ln->ifaddr = *(T_IF_ADDR *)lladdr;
		if (nah->nd_na_flags_reserved & ND_NA_FLG_SOLICITED) {
			ln->state = ND6_LLINFO_REACHABLE;
			/*ln->byhint = 0*/;
			if (ln->expire) {
				syscall(get_tim(&now));
				ln->expire = now + ND6_REACHABLE_TIME;
				}
			}
		else {
			syscall(get_tim(&now));
			ln->expire = now + ND6_GCOLLECTION_TIME;
			ln->state  = ND6_LLINFO_STALE;
			}

		if ((nah->nd_na_flags_reserved & ND_NA_FLG_ROUTER) != 0)
			ln->flags |=  ND6_LLIF_ROUTER;
		else
			ln->flags &= ~ND6_LLIF_ROUTER;
		if ((ln->flags & ND6_LLIF_ROUTER) != 0)
			/*pfxlist_onlink_check()*/;
		}
	else {
		if (lladdr == NULL)
			llchange = false;
		else if (memcmp(lladdr, &ln->ifaddr, sizeof(T_IF_ADDR)))
			llchange = true;
		else
			llchange = false;

		/*
		 *  状態遷移表
		 *
		 *  nd_na_flags_reserved
		 *  OVERRIDE    SOLICTED lladdr llchange 処理 (L: lladdr を登録する)
		 *
		 *      F           F       N      -    (2c)
		 *      F           F       Y      F    (2b) L
		 *      F           F       Y      T    (1)    REACHABLE -> STALE
		 *      F           T       N      -    (2c)           * -> REACHABLE
		 *      F           T       Y      F    (2b) L         * -> REACHABLE
		 *      F           T       Y      T    (1)    REACHABLE -> STALE
		 *      T           F       N      -    (2a)
		 *      T           F       Y      F    (2a) L
		 *      T           F       Y      T    (2a) L         *-> STALE
		 *      T           T       N      -    (2a)           * -> REACHABLE
		 *      T           T       Y      F    (2a) L         * -> REACHABLE
		 *      T           T       Y      T    (2a) L         * -> REACHABLE
		 */
		if ((nah->nd_na_flags_reserved & ND_NA_FLG_OVERRIDE) == 0 && (lladdr != NULL && llchange)) {	/* (1) */
			/* 状態が REACHABLE なら STALE に遷移する。*/
			if (ln->state == ND6_LLINFO_REACHABLE) {
				syscall(get_tim(&now));
				ln->expire = now + ND6_GCOLLECTION_TIME;
				ln->state  = ND6_LLINFO_STALE;
				}
			goto free_ret;
			}
		else if ((nah->nd_na_flags_reserved & ND_NA_FLG_OVERRIDE) ||						/* (2a) */
		         ((nah->nd_na_flags_reserved & ND_NA_FLG_OVERRIDE) == 0 && (lladdr != NULL && !llchange)) ||	/* (2b) */
		         lladdr == NULL) {										/* (2c) */

			/* データリンク層のアドレスが通知されていれば更新する。*/
			if (lladdr != NULL)
				ln->ifaddr = *(T_IF_ADDR *)lladdr;

			/* 近隣要請への応答なら REACHABLE に遷移する。*/
			if (nah->nd_na_flags_reserved & ND_NA_FLG_SOLICITED) {
				ln->state = ND6_LLINFO_REACHABLE;
				/*ln->byhint = 0*/;
				if (ln->expire) {
					syscall(get_tim(&now));
					ln->expire = now + ND6_REACHABLE_TIME;
					}
				}
			/*
			 *  データリンク層のアドレスが通知され、
			 *  異なるアドレスなら更新する。
			 */
			else if (lladdr != NULL && llchange) {
				syscall(get_tim(&now));
				ln->expire = now + ND6_GCOLLECTION_TIME;
				ln->state  = ND6_LLINFO_STALE;
				}
			}

		/* ルータ通知フラグの処理 */
		if ((ln->flags        & ND6_LLIF_ROUTER ) != 0 &&
		    (nah->nd_na_flags_reserved & ND_NA_FLG_ROUTER) == 0) {
			/*
			 *  送信相手がルータ通知フラグを無効にした場合。
			 *  ディフォルト・ルータリストから対象のルータを削除し、
			 *  近隣探索キャッシュを更新する。
			 */
			T_DEF_ROUTER	*dr = NULL;

			syscall(wai_sem(SEM_ND6_DEFRTRLIST));
			if ((dr = nd6_defrtrlist_lookup(&ln->addr)) != NULL) {
				nd6_defrtrlist_del(dr);
				}
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			}
		if ((nah->nd_na_flags_reserved & ND_NA_FLG_ROUTER) != 0)
			ln->flags |=  ND6_LLIF_ROUTER;
		else
			ln->flags &= ~ND6_LLIF_ROUTER;
		}

	ln->asked = 0;

	/* アドレス解決待ちのデータグラムがあれば送信する。*/
	nd6_output_hold(ifp, ln);

free_ret:
	syscall(sig_sem(SEM_ND6_CACHE));
	syscall(rel_net_buf(input));
	return;

err_ret:
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_IN_ERR_PACKETS], 1);
	syscall(rel_net_buf(input));
	}

/*
 *  nd6_na_output -- 近隣通知を出力する。
 */

void
nd6_na_output (T_IFNET *ifp, const T_IN6_ADDR *daddr,
               const T_IN6_ADDR *taddr, uint32_t flags, bool_t tlladdr)
{
	T_NEIGHBOR_ADVERT_HDR	*nah;
	T_NET_BUF		*output;
	T_IP6_HDR		*ip6h;
	T_IN6_IFADDR		*ifa;
	T_IF_ADDR		*mac = NULL;
	uint_t			len;
	uint16_t		ipflags = 0;

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_NA_OUT_PACKETS], 1);

	/*
	 *  近隣通知ペイロード長を計算する。
	 *  tlladdr が真ならネットワークインタフェースのアドレスを追加する。
	 */
	if (tlladdr && IF_SOFTC_TO_IFADDR(ifp->ic))
		len = (NEIGHBOR_ADVERT_HDR_SIZE + ND_OPT_HDR_SIZE + sizeof(T_IF_ADDR) + 7) >> 3 << 3;
	else
		len = (NEIGHBOR_ADVERT_HDR_SIZE + 7) >> 3 << 3;

	/*
	 *  他のリンクからの偽造されたデータグラムを
	 *  排除するため、ホップリミットに IPV6_MAXHLIM (255) を設定する。
	 */
	ipflags = IPV6_OUT_SET_HOP_LIMIT(IPV6_OUT_FLG_HOP_LIMIT, IPV6_MAXHLIM);

	/* ネットワークバッファを獲得し、IPv6 ヘッダを設定する。*/
	if (in6_get_datagram(&output, len, 0, daddr, NULL,
	                     IPPROTO_ICMPV6, IPV6_MAXHLIM,
	                     NBA_SEARCH_ASCENT, TMO_ND6_NA_OUTPUT) != E_OK)
		return;

	ip6h = GET_IP6_HDR(output);

	if (IN6_IS_ADDR_UNSPECIFIED(daddr)) {
		/*
		 *  宛先アドレスが無指定の場合は、
		 *  リンクローカル全ノード・マルチキャストアドレスを設定する。
		 */
		ip6h->dst.s6_addr32[0] = IPV6_ADDR_INT32_MLL;
		ip6h->dst.s6_addr32[1] = 0;
		ip6h->dst.s6_addr32[2] = 0;
		ip6h->dst.s6_addr32[3] = IPV6_ADDR_INT32_ONE;
		flags &= ~ND_NA_FLG_SOLICITED;
		}
	else
		memcpy(&ip6h->dst, daddr, sizeof(T_IN6_ADDR));

	/*
	 *  宛先アドレスにふさわしい送信元アドレスを、
	 *  ネットワークインタフェースから探索して利用する。
	 */
	if ((ifa = in6_ifawithifp(ifp, &ip6h->dst)) == NULL) {
		syscall(rel_net_buf(output));
		return;
		}
	memcpy(&ip6h->src, &ifa->addr, sizeof(T_IN6_ADDR));

	/* 近隣通知ヘッダを設定する。*/
	nah = GET_NEIGHBOR_ADVERT_HDR(output, IF_IP6_NEIGHBOR_ADVERT_HDR_OFFSET);
	nah->hdr.type        = ND_NEIGHBOR_ADVERT;
	nah->hdr.code        = 0;
	memcpy(&nah->target, taddr, sizeof(T_IN6_ADDR));

	/* tlladdr が真ならネットワークインタフェースのアドレスを追加する。*/
	if (tlladdr && (mac = IF_SOFTC_TO_IFADDR(ifp->ic)) != NULL) {

		T_ND_OPT_HDR	*opth;
		uint_t		optlen;

		opth   = (T_ND_OPT_HDR *)GET_NEIGHBOR_ADVERT_SDU(output, IF_IP6_NEIGHBOR_ADVERT_HDR_OFFSET);
		optlen = (ND_OPT_HDR_SIZE + sizeof(T_IF_ADDR) + 7) >> 3 << 3;
		memset(opth, 0, optlen);
		opth->type = ND_OPT_TARGET_LINKADDR;
		opth->len  = optlen >> 3;
		memcpy((uint8_t *)opth + sizeof(T_ND_OPT_HDR), mac, sizeof(T_IF_ADDR));
		}
	else
		flags &= ~ND_NA_FLG_OVERRIDE;

	nah->nd_na_flags_reserved = flags;

	/* チェックサムを計算する。*/
	nah->hdr.sum = 0;
	nah->hdr.sum = in6_cksum(output, IPPROTO_ICMPV6, (uint8_t*)nah - output->buf, len);

	/* 送信する。*/
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_OCTETS],
	               output->len - GET_IF_IP6_HDR_SIZE(output));
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutNeighborAdvertisements, 1);
	ip6_output(output, ipflags, TMO_ND6_NS_OUTPUT);
	}

/*
 *  nd6_dad_start -- 重複アドレス検出 (DAD) を開始する。
 *
 *    第3引数のtick は、インタフェースが起動されてから、重複検出を
 *    送信するまでの最小遅延値である。
 */

void
nd6_dad_start (T_IFNET *ifp, T_IN6_IFADDR *ifa, int_t *tick)
{
	/* フラグをチェックする。*/
	if (!(ifa->flags & IN6_IFF_TENTATIVE))
		return;

#if NUM_IP6_DAD_COUNT == 0

	ifa->flags &= ~IN6_IFF_TENTATIVE;

#else	/* of #if NUM_IP6_DAD_COUNT == 0 */

	if (ifa->flags & IN6_IFF_ANYCAST) {
		ifa->flags &= ~IN6_IFF_TENTATIVE;
		return;
		}
	else if ((ifa->flags & IN6_IFF_TENTATIVE) == 0)
		return;

	/* 重複アドレス検出カウンタをリセットする。*/
	ifa->ns_icount = ifa->na_icount = ifa->ns_ocount = 0;

	/*
	 *  重複アドレス検出要請を送信する。ただし、インタフェースを
	 *  初期化した直後に、最初に送信する場合は、ランダムな遅延を行う。
	 */
	if (tick == NULL) {
		nd6_dad_ns_output(ifp, ifa);

		/* タイムアウトを近隣探索の送信間隔に設定する。*/
		timeout((callout_func)nd6_dad_timer, ifa, ND6_RETRANS_TIME * NET_TIMER_HZ / SYSTIM_HZ);
		}
	else {
		int_t ntick;

		if (*tick == 0)	/* 最初の送信 */
			ntick = net_rand() % (ND6_FIRST_DAD_DELAY_TIME * NET_TIMER_HZ / SYSTIM_HZ);
		else
			ntick = *tick + net_rand() % (ND6_DAD_DELAY_TIME * NET_TIMER_HZ / SYSTIM_HZ);
		*tick = ntick;
		timeout((callout_func)nd6_dad_timer, ifa, ntick);
		}

#endif	/* of #if NUM_IP6_DAD_COUNT == 0 */

	}

#endif /* of #ifdef _IP6_CFG */
