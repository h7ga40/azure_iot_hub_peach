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

/*	$FreeBSD: src/sys/netinet6/nd6.c,v 1.20 2002/08/02 20:49:14 rwatson Exp $	*/
/*	$KAME: nd6.c,v 1.144 2001/05/24 07:44:00 itojun Exp $	*/

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
 * XXX
 * KAME 970409 note:
 * BSD/OS version heavily modifies this code, related to llinfo.
 * Since we don't have BSD/OS version of net/route.c in our hand,
 * I left the code mostly as it was in 970310.  -- itojun
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
#include <net/ppp.h>
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

#include <net/if_var.h>

#ifdef _IP6_CFG

/*
 *  近隣キャッシュ
 */

static T_LLINFO_ND6 nd6_cache[NUM_ND6_CACHE_ENTRY];

/*
 *  nd6_free -- ルータを削除する。
 */

static void
nd6_free (T_LLINFO_ND6	*ln)
{
	T_DEF_ROUTER	*dr;

	dr = nd6_defrtrlist_lookup(&ln->addr);
	if (dr != NULL) {
		ln->state = ND6_LLINFO_INCOMPLETE;

		/* プレフィックス・リストからルータを削除する。*/
		nd6_defrtrlist_del(dr);
		}
	}

/*
 *  nd6_timer -- 近隣探索管理タイマー
 */

void
nd6_timer (void)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_LLINFO_ND6	*ln;
	SYSTIM		now;
	int_t		ix;

	/* 近隣キャッシュの処理 */
	syscall(wai_sem(SEM_ND6_CACHE));
	syscall(get_tim(&now));
	for (ix = NUM_ND6_CACHE_ENTRY; ix -- > 0; ) {
		ln = &nd6_cache[ix];
		if (ln->state >= ND6_LLINFO_NO_STATE && (int32_t)(ln->expire - now) < 0) {
		                                               /* ln->expire < now */

			switch (ln->state) {

			case ND6_LLINFO_INCOMPLETE:	/* 未解決		*/
				if (ln->asked < ND6_MAX_MCAST_QUERY) {
					/*
					 *  マルチキャストアドレス要請の再送回数以下の場合の処理
					 */
					ln->asked ++;
					ln->expire = now + ND6_RETRANS_TIME;	/* 近隣探索送信間隔 */
					nd6_ns_output(ifp, &in6_addr_unspecified, &ln->addr, ln, false);
					}
				else  {
					if (ln->hold != NULL) {
						/*
						 *  最終的には、保留されているネットワークバッファが
						 *  あれば、ICMP6 ERROR を自分自身に返したあと、
						 *  ネットワークバッファを開放する。
						 *  しかし、現在、icmp6_error の実装が不完全のため、
						 *  ネットワークバッファの開放のみ行う。
						 */
#if 0	/* 保留 */
						icmp6_error(ln->hold, ICMP6_DST_UNREACH,
						                      ICMP6_DST_UNREACH_ADDR, 0);
#endif	/* #if 0 */
						syscall(rel_net_buf(ln->hold));
						ln->hold = NULL;
						}
					nd6_free(ln);
					memset((void*)ln, 0, sizeof(*ln));
					}
				break;

			case ND6_LLINFO_REACHABLE:	/* 到達可能		*/
				if (ln->expire) {
					ln->expire = now + ND6_GCOLLECTION_TIME;
					ln->state = ND6_LLINFO_STALE;
					}
				break;

			case ND6_LLINFO_STALE:		/* 到達可能性は未確認	*/
				if (ln->expire) {
					if (ln->hold != NULL) {
						syscall(rel_net_buf(ln->hold));
						ln->hold = NULL;
						}
					nd6_free(ln);
					memset((void*)ln, 0, sizeof(*ln));
					}
				break;

			case ND6_LLINFO_DELAY:		/* 到達可能性の確認待機	*/
				ln->asked = 1;
				ln->expire = now + ND6_RETRANS_TIME;		/* 近隣探索送信間隔 */
				ln->state = ND6_LLINFO_PROBE;
				nd6_ns_output(ifp, &ln->addr, &ln->addr, ln, false);
				break;

			case ND6_LLINFO_PROBE:		/* 到達可能性を確認中	*/
				if (ln->asked < ND6_MAX_UCAST_QUERY) {
					/*
					 *  ユニキャストアドレス要請の再送回数以下の場合の処理
					 */
					ln->asked ++;
					ln->expire += ND6_RETRANS_TIME;	/* 近隣探索送信間隔 */
					nd6_ns_output(ifp, &ln->addr, &ln->addr, ln, false);
					}
				else {
					if (ln->hold != NULL) {
						syscall(rel_net_buf(ln->hold));
						ln->hold = NULL;
						}
					nd6_free(ln);
					memset((void*)ln, 0, sizeof(*ln));
					}
				break;

				}
			}
		}
	syscall(sig_sem(SEM_ND6_CACHE));

	/* ディフォルトルータ・リストの処理 */
	nd6_defrtrlist_timer();

	/* プレフィックス・リストの処理 */
	nd6_prelist_timer();

	/* ネットワークインタフェースのアドレスの処理 */
	in6_ifaddr_timer(ifp);

	/*
	 *  到達可能状態の有効時間の更新
	 */
	if ((int32_t)(nd6_recalc_reachtm_interval - now) < 0) {
	           /* nd6_recalc_reachtm_interval < now */
		nd6_recalc_reachtm_interval	= now + ND6_RECALC_REACHTM_INTERVAL;
		nd6_reachable_time		= ND6_CALC_REACH_TIME(nd6_base_reachable_time);
		}
	timeout((callout_func)nd6_timer, NULL, ND6_TIMER_TMO);
	}

/*
 *  nd6_get_cache -- 近隣キャッシュを獲得する。
 */

const T_LLINFO_ND6 *
nd6_get_cache (void)
{
	return nd6_cache;
	}

/*
 *  nd6_ifattach -- 近隣探索の初期設定を行う。
 */

void
nd6_ifattach (T_IFNET *ifp)
{
	}

/*
 *  nd6_output_hold -- 近隣探索キャッシュに保留されている
 *                     アドレス解決待ちのデータグラムがあれば送信する。
 *
 *    注意: セマフォ SEM_ND6_CACHE によりロック状態で呼出すこと。
 */

ER
nd6_output_hold (T_IFNET *ifp, T_LLINFO_ND6 *ln)
{
	SYSTIM now;

	/* アドレス解決待ちのデータグラムがあれば送信する。*/
	if (ln->hold) {

		/* タイムアウトの処理 */
		if (ln->tmout != TMO_FEVR) {

			/*
			 *  タイムアウトの指定が永久でなく、時間を過ぎていれば、
			 *  出力しないで終了する。
			 */
			syscall(get_tim(&now));
			if ((int32_t)(ln->tmout - now) < 0) {
			           /* ln->tmout < now */
				syscall(rel_net_buf(ln->hold));
				ln->hold = NULL;
				return E_TMOUT;
				}
			}
				
		nd6_output(ifp, ln->hold, &ln->addr, ln, TMO_FEVR);
		ln->hold = NULL;
		}
	return E_OK;
	}

/*
 *  nd6_lookup -- 近隣探索キャッシュを探索する。
 *
 *    注意: セマフォ SEM_ND6_CACHE によりロック状態で呼出すこと。
 *
 */

T_LLINFO_ND6 *
nd6_lookup (const T_IN6_ADDR *addr, bool_t create)
{
	SYSTIM	min = 0xffffffff;
	int_t	ix, fix = -1, mix = -1;

	for (ix = NUM_ND6_CACHE_ENTRY; ix -- > 0; ) {
		if (nd6_cache[ix].state == 0) {
			/* 未使用エントリのインデックスを記録する。*/
			if (fix == -1)
				fix = ix;
			}
		else if (IN6_ARE_ADDR_EQUAL(addr, &nd6_cache[ix].addr)) {
			return &nd6_cache[ix];
			}
		else if ((int32_t)(nd6_cache[ix].expire - min) < 0) {	/* nd6_cache[ix].expire < min */
			/* 有効期限が最短エントリのインデックスを記録する。*/
			min = nd6_cache[ix].expire;
			mix = ix;
			}
		}

	if (create) {
		if (fix == -1)  {
			if (nd6_cache[mix].hold != NULL) {
				syscall(rel_net_buf(nd6_cache[mix].hold));
				nd6_cache[mix].hold = NULL;
				}
			fix = mix;
			}
		syscall(get_tim(&nd6_cache[fix].expire));
		nd6_cache[fix].addr   = *addr;
		nd6_cache[fix].state  = ND6_LLINFO_NO_STATE;
		return &nd6_cache[fix];
		}
	else
		return NULL;
	}

/*
 *  nd6_cache_lladdr -- 近隣探索キャッシュに登録する。
 */

T_LLINFO_ND6 *
nd6_cache_lladdr (T_IFNET *ifp, const T_IN6_ADDR *from,
                  T_IF_ADDR *lladdr, uint8_t type, uint8_t code)
{
	T_LLINFO_ND6	*ln;
	SYSTIM		now;
	bool_t		newentry, olladdr, llchange, doupdate;
	int_t		newstate = ND6_LLINFO_INCOMPLETE;

	/* アドレスが無指定なら何もしない。*/
	if (IN6_IS_ADDR_UNSPECIFIED(from))
		return NULL;

	/* 近隣キャッシュに登録する。*/
	syscall(wai_sem(SEM_ND6_CACHE));
	if ((ln = nd6_lookup(from, false)) == NULL) {
		ln = nd6_lookup(from, true);
		newentry = true;
		}
	else
		newentry = false;

	olladdr = ln->state > ND6_LLINFO_INCOMPLETE;
	if (!olladdr || lladdr == NULL)
		llchange = false;
	else if (memcmp(lladdr, &ln->ifaddr, sizeof(T_IF_ADDR)))
		llchange = true;
	else
		llchange = false;

	/*
	 *  状態遷移表
	 *
	 *  nd_na_flags
	 *  newnetry olladdr lladdr llchange 処理 (L: lladdr を登録する)
	 *
	 *      F       F       N       -    (1)
	 *      F       T       N       -    (2)
	 *      F       F       Y       -    (3) L * -> STALE
	 *      F       T       Y       F    (4) L
	 *      F       T       Y       T    (5) L * -> STALE
	 *      T       -       N       -    (6)   * -> NO_STATE
	 *      T       -       Y       -    (7) L * -> STALE
	 */

	/* データリンク層のアドレスが通知されていれば更新する。*/
	if (lladdr != NULL) {
		ln->ifaddr = *(T_IF_ADDR *)lladdr;
		}

	if (newentry) {
		if (lladdr != NULL)					/* (7) */
			newstate = ND6_LLINFO_STALE;
		else							/* (6) */
			newstate = ND6_LLINFO_NO_STATE;
		doupdate = true;
		}
	else {
		if ((!olladdr && lladdr != NULL) ||			/* (3) */
		    ( olladdr && lladdr != NULL && llchange)) {		/* (5) */
			newstate = ND6_LLINFO_STALE;
			doupdate = true;
			}
		else							/* (1),(2),(4) */
			doupdate = false;
		}

	/* 近隣キャッシュエントリを更新する。*/
	if (doupdate) {
		ln->state = newstate;

		syscall(get_tim(&now));
		if (ln->state == ND6_LLINFO_STALE) {
			ln->expire = now + ND6_GCOLLECTION_TIME;

			/* アドレス解決待ちのデータグラムがあれば送信する。*/
			nd6_output_hold(ifp, ln);
			}
		else if (ln->state == ND6_LLINFO_INCOMPLETE)
			ln->expire = now;
		}
	syscall(sig_sem(SEM_ND6_CACHE));


	/* ルータ通知の処理 */
	switch (type) {

	case ND_NEIGHBOR_SOLICIT:
		if (newentry)
			ln->flags &= ~ND6_LLIF_ROUTER;
		break;

	case ND_REDIRECT:
		if (code == ND_REDIRECT_ROUTER)
			ln->flags |=  ND6_LLIF_ROUTER;
		else if (newentry)
			ln->flags &= ~ND6_LLIF_ROUTER;
		break;

	case ND_ROUTER_SOLICIT:
		ln->flags &= ~ND6_LLIF_ROUTER;
		break;

	case ND_ROUTER_ADVERT:
		if ((!newentry && (olladdr || lladdr != NULL)) ||
		    ( newentry && lladdr))
			ln->flags |=  ND6_LLIF_ROUTER;
		break;
		}

	return ln;
	}

/*
 *  nd6_is_addr_neighbor -- 指定されたアドレスのノードが同一リンクにあるかチェックする。
 */

bool_t
nd6_is_addr_neighbor (T_IFNET *ifp, const T_IN6_ADDR *addr)
{
	if (IN6_IS_ADDR_LINKLOCAL(addr))
		return true;

	if (nd6_onlink_prefix_lookup(addr) != NULL)
		return true;

	if (nd6_lookup(addr, false) != NULL)
		return true;

	return false;
	}

/*
 *  nd6_output -- 近隣探索の出力関数
 *
 *    注意: セマフォ SEM_ND6_CACHE によりロック状態で呼出すこと。
 */

ER
nd6_output (T_IFNET *ifp, T_NET_BUF *output, const T_IN6_ADDR *dst, T_LLINFO_ND6 *ln, TMO tmout)
{
	ER	error = E_OK;
	SYSTIM	now;

	/*
	 *  あて先アドレスがマルチキャストか、近隣キャッシュが
	 *  不要の場合は直ちに送信する。
	 */
	if (!(IN6_IS_ADDR_MULTICAST(dst) || !IF_IN6_NEED_CACHE(ifp))) {
		if (ln == NULL) {
			/* 近隣キャッシュが未定義の場合 */
			if (nd6_is_addr_neighbor(ifp, dst)) {
				/*
				 *  指定されたアドレスのノードが同一リンクにある時は、
				 *  近隣キャッシュを探索し、未定義の場合は登録する。
				 */
				ln = nd6_lookup(dst, true);
				}
			else {
				error = E_CLS;
				goto err_ret;
				}
			}

		/*
		 *  近隣キャッシュの状態が、データリンク層のアドレスが分かっていても、
		 *  到達可能性が確認されていないときは、到達不能通知を開始する。
		 */
		if (ln->state == ND6_LLINFO_STALE) {
			syscall(get_tim(&now));
			ln->expire = now + ND6_DELAY_TIME;
			ln->asked = 0;
			ln->state = ND6_LLINFO_DELAY;
			}

		/*
		 *  近隣キャッシュの状態が、データリンク層のアドレス未解決の場合は
		 *  近隣探索を行う。
		 */
		if (ln->state <= ND6_LLINFO_INCOMPLETE) {

			/* タイムアウトの指定がポーリングならタイムアウトで終了する。*/
			if (tmout == TMO_POL) {
				error = E_TMOUT;
				goto err_ret;
				}

			if (ln->state == ND6_LLINFO_NO_STATE)
				ln->state = ND6_LLINFO_INCOMPLETE;

			/* すでに、未解決のネットワークバッファがあれば、開放する。*/
			if (ln->hold != NULL) {
				syscall(rel_net_buf(ln->hold));
				ln->hold = NULL;
				}

			/*
			 *  送信をペンディングする。
			 *  IF でネットワークバッファを開放しないフラグが設定されているときは、
			 *  送信をペンディングしない。
			 */
			if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
				ln->hold = output;

				/* タイムアウトの設定 */
				if (tmout == TMO_FEVR)
					ln->tmout = tmout;
				else {
					syscall(get_tim(&now));
					ln->tmout = now + tmout;
					}
				}
			else {
				output->flags &= ~NB_FLG_NOREL_IFOUT;
				syscall(get_tim(&now));
				ln->tmout = now + tmout;
				}

			if (ln->expire) {
				if (ln->asked < ND6_MAX_MCAST_QUERY) {
					syscall(get_tim(&now));
					if ((int32_t)(ln->expire - now) < 0) {
					           /* ln->expire < now */
						ln->expire += ND6_RETRANS_TIME;	/* 近隣探索送信間隔 */
						ln->asked ++;
						nd6_ns_output(ifp, &in6_addr_unspecified, dst, ln, false);
						}
					}
				}
			return error;
			}
		}

	/*
	 *  重複が検出されたインタフェースには出力しない。
	 */
	if ((ifp->flags & ND6_IFF_IFDISABLED) != 0) {
		return E_OK;
		}

	IF_SET_PROTO(output, IF_PROTO_IPV6);
	error = IF_OUTPUT(output, dst, ln ? & ln->ifaddr : NULL, tmout);
	return error;

err_ret:
	syscall(rel_net_buf(output));
	return error;
	}

/*
 *  nd6_storelladdr -- 出力するネットワークインタフェースのアドレスを返す。
 */

ER
nd6_storelladdr (T_IF_ADDR *out, const T_IN6_ADDR *dst, T_IF_ADDR *ifa)
{
	if (IN6_IS_ADDR_MULTICAST(dst)) {
		IF_IN6_RESOLVE_MULTICAST(out, dst);
		}
	else {
		*out = *ifa;
		}

	return E_OK;
	}

/*
 *  nd6_option -- 近隣探索オプションの処理
 *
 *    注意: オプションオフセット配列には、オフセット + 8 が設定されている。
 */

ER
nd6_options (uint8_t *opt, void *nh, uint_t len)
{
	T_ND_OPT_HDR	*opth;
	ER		error = E_OK;
	int_t		pi_count = 0;	/* プレフィックス情報の個数 */

	opth = (T_ND_OPT_HDR *)nh;
	memset(opt, 0, ND_OPT_OFF_ARRAY_SIZE);

	while (error == E_OK && ((uint8_t *)opth - (uint8_t *)nh) < (256 - 8) && len > 0) {

		/* オプション長が 0 の時は、不正なオプションとして処理する。*/
		if (opth->len == 0) {
			error = E_PAR;
			break;
			}

		/* オプションのタイプにより分岐する。*/
		switch (opth->type) {
		case ND_OPT_SOURCE_LINKADDR:		/* 送信先ネットワークインタフェースのアドレス	*/
		case ND_OPT_TARGET_LINKADDR:		/* 対象ネットワークインタフェースのアドレス	*/
		case ND_OPT_REDIRECTED_HEADER:		/* リダイレクト・ヘッダ				*/
		case ND_OPT_MTU:			/* MTU						*/
			if (opt[ND_OPT_OFF_ARRAY_IX(opth->type)]) {
				/* プレフィックス情報以外の同じオプションが複数指定されている。*/
				error = E_PAR;
				}
			else {
				opt[ND_OPT_OFF_ARRAY_IX(opth->type)] = (uint8_t *)opth - (uint8_t *)nh + 8;
				}
			break;
		case ND_OPT_PREFIX_INFORMATION:		/* プレフィックス情報				*/
			if (!opt[ND_OPT_OFF_ARRAY_IX(ND_OPT_PREFIX_INFORMATION)]) {
				/*
				 *  プレフィックス情報は最初に現れたオフセットのみ記録する。
				 */
				opt[ND_OPT_OFF_ARRAY_IX(opth->type)] = (uint8_t *)opth - (uint8_t *)nh + 8;
				}
			pi_count ++;
			break;
		default:
			/*
			 *  将来定義されるプロトコルへの拡張に対応するため、
			 *  認識できないオプションは単に無視し、処理を続行する。
			 */
			break;
			}
		len -= opth->len << 3;		/* オプション長の単位は 8 バイト */
		opth = (T_ND_OPT_HDR *)((uint8_t *)opth + (opth->len << 3));
		}

	opt[ND_OPT_OFF_ARRAY_IX(ND_OPT_PREFIX_INFO_COUNT)] = pi_count;
	return error;
	}

#endif /* of #ifdef _IP6_CFG */
