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

/*	$FreeBSD: src/sys/netinet6/nd6_rtr.c,v 1.11 2002/04/19 04:46:23 suz Exp $	*/
/*	$KAME: nd6_rtr.c,v 1.111 2001/04/27 01:37:15 jinmei Exp $	*/

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
#include <netinet6/in6_ifattach.h>

#include <net/if_var.h>

/*
 *  外部関数の定義
 */

extern const char *itron_strerror (ER ercd);

#ifdef _IP6_CFG

#if NUM_ND6_DEF_RTR_ENTRY > 0

/*
 *  全域変数
 */

SYSTIM	nd6_base_reachable_time		= ND6_DEF_REACHABLE_TIME;	/* ルータから通知された値		*/
SYSTIM	nd6_reachable_time		= ND6_DEF_REACHABLE_TIME;	/* ルータから通知された値の補正値	*/
SYSTIM	nd6_recalc_reachtm_interval	= ND6_RECALC_REACHTM_INTERVAL;	/* 到達可能状態の有効時間再計算間隔	*/
SYSTIM	nd6_retrans_time		= ND6_DEF_RETRANS_TIME;		/* 近隣探索の送信間隔			*/
uint8_t	ip6_defhlim			= IPV6_DEFHLIM;			/* ホップリミットの規定値		*/

/*
 *  局所変数
 */

/*
 *  プレフィックス・リスト
 */

static T_ND6_PREFIX nd6_prefix[NUM_ND6_PREFIX_ENTRY];

/*
 *  ディフォルトルータ・リスト
 */

static T_DEF_ROUTER	nd6_def_router[NUM_ND6_DEF_RTR_ENTRY];
static uint_t		def_router_count = 0;				/* リストにあるルータ数 */

/*
 *  make_mask -- マップ用のマスクを生成する。
 */

static uint_t
make_mask (int_t index)
{
	uint_t	mask;
	int_t	ix;

	/* マスクを生成する。*/
	mask = 1;
	for (ix = index; ix -- > 0; )
		mask <<= 1;

	return mask;
	}

/*
 *  find_pfxlist_reachable_router --
 *      指定したプレフィックスを通知したルータがリンク上に存在するかチェックする。
 */

static T_DEF_ROUTER *
find_pfxlist_reachable_router (T_ND6_PREFIX *pr)
{
	T_DEF_ROUTER	*dr = NULL;
	T_LLINFO_ND6	*ln;
	uint_t		mask;
	int_t		ix;

	mask = 1;
	for (ix = 0; ix < def_router_count; ix ++) {
		if ((pr->routermap & mask) != 0) {
			if ((ln = nd6_lookup(&nd6_def_router[ix].addr, false)) != NULL &&
			    ND6_IS_LLINFO_PROBREACH(ln))
				return dr;
			}
		mask <<= 1;
		}
	return NULL;
	}

/*
 *  pfxlist_onlink_check -- プレフィックス・リストのオンリンク状態をチェックする。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

static void
pfxlist_onlink_check (void)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_ND6_PREFIX	*pr = NULL;
	T_IN6_IFADDR	*ia;
	ER		error;
	int_t		ix;

	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) != 0 &&
		    find_pfxlist_reachable_router(pr))
			break;
		}

	if (ix >= 0) {
		/*
		 *  オンリンク状態のプレフィックスが最低一つ存在するときの処理。
		 *  各プレフィックスのオンリンク状態を、それらを通知してきたルータの
		 *  状態に従って設定する。
		 */

		
		for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
			pr = &nd6_prefix[ix];

			if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) == 0)
				continue;

			if ((pr->flags & ND6_PREFIX_STATE_DETACHED) == 0 &&
			    find_pfxlist_reachable_router(pr) == NULL)
				pr->flags |= ND6_PREFIX_STATE_DETACHED;

			if ((pr->flags & ND6_PREFIX_STATE_DETACHED) != 0 &&
			    find_pfxlist_reachable_router(pr) != NULL)
				pr->flags &= ~ND6_PREFIX_STATE_DETACHED;
			}
		}
	else {
		/*
		 *  どのプレフィックスもオンリンクではなく、
		 *  ディフォルトルータも存在しないときの処理。
		 *  すべてのプレフィックスを使用可能状態に設定する。
		 */

		
		for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
			pr = &nd6_prefix[ix];

			if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) == 0)
				continue;

			if ((pr->flags & ND6_PREFIX_STATE_DETACHED) != 0)
				pr->flags &= ~ND6_PREFIX_STATE_DETACHED;
			}
		}

	/*
	 *  各プレフィックス毎に、通知してきたルータの状態と
	 *  オンリンクの状態が一致しなければ、オンリンク状態を更新する。
	 */
	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];

		if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) == 0)
			continue;

		if ((pr->flags & ND6_PREFIX_STATE_DETACHED) != 0 &&
		    (pr->flags & ND6_PREFIX_STATE_ONLINK  ) != 0) {
			if ((error = nd6_prefix_offlink(pr)) != E_OK)
				syslog(LOG_ERROR, "[ND6 RTR] prefix offlink failed: %s", itron_strerror(error));
			}

		if ((pr->flags & ND6_PREFIX_STATE_DETACHED) == 0 &&
		    (pr->flags & ND6_PREFIX_STATE_ONLINK  ) == 0 &&
		    (pr->flags & ND6_PREFIX_FLAG_ONLINK   ) == 0) {
			if ((error = nd6_prefix_onlink(pr)) != E_OK)
				syslog(LOG_ERROR, "[ND6 RTR] prefix onlink failed: %s", itron_strerror(error));
			}
		}

	/*
	 *  リンク上にプレフィックスを通知しているルータが
	 *  少なくとも 1 台存在するか確認する。
	 */
	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
		ia = &ifp->in6_ifaddrs[ix];
		if ((ia->flags & (IN6_IFF_DEFINED | IN6_IFF_AUTOCONF)) != 0 &&
		     ia->prefix_index != ND6_PREFIX_IX_INVALID) {
			pr = &nd6_prefix[ia->prefix_index];
			if (find_pfxlist_reachable_router(pr) != NULL)
				break;
			}
		}

	if (ix >= 0) {
		/*
		 *  リンク上にプレフィックスを通知しているルータが
		 *  少なくとも 1 台存在する時の処理。
		 */
		for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
			ia = &ifp->in6_ifaddrs[ix];
			if ((ia->flags & (IN6_IFF_DEFINED | IN6_IFF_AUTOCONF)) != 0 &&
			     ia->prefix_index != ND6_PREFIX_IX_INVALID) {
				pr = &nd6_prefix[ia->prefix_index];
				if (find_pfxlist_reachable_router(pr) != NULL)
					ia->flags &= ~IN6_IFF_DETACHED;
				else
					ia->flags |= IN6_IFF_DETACHED;
				}
			}
		}
	else {
		/*
		 *  リンク上にプレフィックスを通知しているルータが
		 *  1 台存在しない時は全てのアドレスを利用できるようにする。
		 */
		for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
			ia = &ifp->in6_ifaddrs[ix];
			if ((ia->flags & (IN6_IFF_DEFINED | IN6_IFF_AUTOCONF)) != 0)
				ia->flags &= ~IN6_IFF_DETACHED;
			}
		}
	}

/*
 *  pfxrtr_add -- プレフィックスエントリとディフォルトルータ・エントリをリンクする。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

static void
pfxrtr_add (int_t prefix_index, int_t rtr_index)
{
	/*  プレフィックスエントリとディフォルトルータ・エントリをリンクする。*/
	nd6_prefix[prefix_index].routermap |= make_mask(rtr_index);
	nd6_def_router[rtr_index].plistmap |= make_mask(prefix_index);

	pfxlist_onlink_check();
	}

/*
 *  nd6_pfxrtr_lookup -- プレフィックスエントリからルータを探索する。
 */

static uint_t
pfxrtr_lookup (const T_ND6_PREFIX *pr, int_t rtr_index)
{
	return (pr->routermap & make_mask(rtr_index)) == 0 ? IN6_RTR_IX_UNREACH : rtr_index;
	}

/*
 *  defrouter_select -- 優先ディフォルト・ルータを選択しなおす。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

static void
defrouter_select (void)
{
	T_DEF_ROUTER	*dr = NULL, router;
	T_LLINFO_ND6	*ln;
	int_t		ix, tix;

	for (ix = 0; ix < def_router_count; ix ++) {
		dr = &nd6_def_router[ix];
		if ((ln = nd6_lookup(&dr->addr, false)) != NULL && ND6_IS_LLINFO_PROBREACH(ln)) {
		
			/* 選択したルータを先頭に移す。*/
			router = *dr;
			for (tix = ix; tix > 0; tix --)
				nd6_def_router[tix] = nd6_def_router[tix - 1];
			nd6_def_router[0] = router;
			break;
			}
		}
	}

/*
 *  defrtrlist_update -- ディフォルトルータ・リストを更新する。
 *
 *  	戻り値は、ルータのインデックス。
 *      ただし、値が -1 なら、プレフィックス・リストは更新しない。
 */

static int_t
defrtrlist_update (T_IN6_ADDR *src, uint32_t lifetime, uint8_t flags)
{
	T_DEF_ROUTER	*dr, *fdr = NULL;
	SYSTIM		now;
	int_t		ix;

	syscall(get_tim(&now));
	syscall(wai_sem(SEM_ND6_DEFRTRLIST));

	/* 登録済か調べる。*/
	if ((fdr = nd6_defrtrlist_lookup(src)) != NULL) {
		/* 登録済みの場合の処理 */
		if (lifetime == 0) {
			nd6_defrtrlist_del(fdr);
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			return -1;
			}
		else {
			fdr->lifetime = lifetime * SYSTIM_HZ;		/* lifetime の単位は秒 */
			fdr->flags    = flags;
			fdr->expire   = now + fdr->lifetime;
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			return fdr - nd6_def_router;
			}
		}
	else if (lifetime == 0) {
		syscall(sig_sem(SEM_ND6_DEFRTRLIST));
		return -1;
		}

	if (fdr == NULL && def_router_count < NUM_ND6_DEF_RTR_ENTRY) {
		/*
		 *  def_router_count < NUM_ND6_DEF_RTR_ENTRY であれば、
		 *  nd6_def_router[def_router_count]; が空きエントリ
		 */
		fdr = &nd6_def_router[def_router_count];
		}

	/* 空きエントリがなければ、有効時間が短いエントリを削除する。*/
	if (fdr == NULL) {
		T_DEF_ROUTER	*sdr = NULL;
		int32_t		diff, sdiff = 0xffffffff;

		for (ix = def_router_count; ix -- > 0; ) {
			dr = &nd6_def_router[ix];
			diff = (int32_t)(dr->expire - now);
			if (diff <= 0) {	/* dr->expire <= now */
				/* 既に、有効時間が過ぎている。*/
				sdr = dr;
				break;
				}
			else if (diff < sdiff)
				sdr = dr;
			}
		if (sdr != NULL) {
			fdr = sdr;
			nd6_defrtrlist_del(fdr);
			}
		}

	/* 各メンバに値を設定する。*/
	fdr->plistmap = 0;
	memcpy(&fdr->addr, src, sizeof(T_IN6_ADDR));
	fdr->lifetime = lifetime * SYSTIM_HZ;		/* lifetime の単位は秒 */
	fdr->flags    = flags;
	fdr->expire   = now + fdr->lifetime;
	fdr->state    = ND6_DEF_RTR_BUSY;
	def_router_count ++;

	/*
	 *  ディフォルトルータとして登録するが、
	 *  既にディフォルトルータがあれば選択しなおさない。
	 */
	if (fdr - nd6_def_router != 0)
		defrouter_select();

	syscall(sig_sem(SEM_ND6_DEFRTRLIST));
	return fdr - nd6_def_router;
	}

/*
 *  in6_ifadd -- インタフェースにアドレスを追加する。
 */

static ER
in6_ifadd (T_ND6_PREFIX *pr, T_IN6_IFADDR *ia, int_t router_index)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_ADDR	mask, addr;
	T_IN6_IFADDR	*lla;
	int_t		ix;

	lla = &ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL];

	/*
	 *  リンクローカルアドレスが未登録であれば何もしない。
	 */
	if ((lla->flags & IN6_IFF_DEFINED) == 0)
		return E_OBJ;

	/*
	 *  ルータから通知されたプレフィックス長が、
	 *  リンクローカルアドレスのプレフィックス長と
	 *  異なっていればエラー
	 */
	if (lla->prefix_len != pr->prefix_len)
		return E_PAR;

	/* プレフィックスマスクを生成する。*/
	in6_plen2pmask(&mask, pr->prefix_len);

	/* アドレスを生成する。*/
	for (ix = 0; ix < 4; ix ++)
		addr.s6_addr32[ix] = (pr->prefix.s6_addr32[ix] &  mask.s6_addr32[ix])
		                   | ( lla->addr.s6_addr32[ix] & ~mask.s6_addr32[ix]);

	/* フラグを初期化する。*/
	ia->flags = 0;

	return in6_update_ifa(ifp, ia, &addr,
	                      pr->prefix_len, pr->vltime, pr->pltime,
	                      router_index, pr - nd6_prefix, IN6_IFF_AUTOCONF);
	}

/*
 *  nd6_prefix_lookup -- プレフィックスを探索する。
 */

static T_ND6_PREFIX *
nd6_prefix_lookup (const T_IN6_ADDR *prefix, uint_t plen)
{
	T_ND6_PREFIX	*pr;
	int_t		ix;

	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_STATE_BUSY) &&
		     pr->prefix_len == plen && IN6_ARE_ADDR_EQUAL(&pr->prefix, prefix))
			return pr;
		}

	return NULL;
	}

/*
 *  prelist_remove -- プレフィックスリストからプレフィックスを削除する。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

static void
prelist_remove (T_ND6_PREFIX *pr)
{
	ER	error = E_OK;
	uint_t	mask;
	int_t	rix;

	/* 有効時間切れにする。*/
	pr->vltime = 0;
	pr->pltime = 0;

	if ((pr->flags & ND6_PREFIX_STATE_ONLINK) != 0 &&
	    (error = nd6_prefix_offlink(pr)) != E_OK) {
		syslog(LOG_ERROR, "[ND6 RTR] prefix remove failed: %s", itron_strerror(error));
		return;
		}

	/*  プレフィックスエントリとディフォルトルータ・エントリのリンクを解除する。*/
	pr->routermap = 0;
	mask = make_mask(pr - nd6_prefix);
	for (rix = def_router_count; rix --; )
		nd6_def_router[rix].plistmap &= ~mask;

	/* プレフィックスを無効にする。*/
	pr->flags &= ~ND6_PREFIX_STATE_BUSY;

	pfxlist_onlink_check();
	}

/*
 *  nd6_prefix_onlink -- プレフィックスをオンリンクに設定し、アドレスを使用可能にする。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

ER
nd6_prefix_onlink (T_ND6_PREFIX *pr)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_IFADDR	*lla, *ia = NULL;
	int_t		ix;

	/* 既にオンリンク状態になっている場合はエラー。*/
	if ((pr->flags & ND6_PREFIX_STATE_ONLINK) != 0)
		return E_OBJ;

	/* リンクローカルアドレスが未登録であれば何もしない。*/
	lla = &ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL];
	if ((lla->flags & IN6_IFF_DEFINED) == 0)
		return E_OK;

	/*
	 *  指定されたプレフィックスが割り当てられているインタフェースの
	 *  アドレス情報を探す。
	 */
	for (ix = NUM_IN6_IFADDR_ENTRY; ix --; ) {
		ia = &ifp->in6_ifaddrs[ix];
		if (ia->prefix_index == (pr - nd6_prefix))
			break;
		}

	if (ix >= 0) {

		/* アドレス情報を使用可能にする。*/
		ia->flags &= ~IN6_IFF_DETACHED;

		/* プレフィックスをオンリンクに設定する。*/
		pr->flags |= ND6_PREFIX_STATE_ONLINK;
		}

	return E_OK;
	}

/*
 *  nd6_prefix_offlink -- プレフィックスをオフリンクに設定し、アドレスを使用不可にする。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

ER
nd6_prefix_offlink (T_ND6_PREFIX *pr)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_IFADDR	*ia = NULL;
	int_t		ix;

	/* 既にオフリンク状態になっている場合はエラー。*/
	if ((pr->flags & ND6_PREFIX_STATE_ONLINK) == 0)
		return E_OBJ;

	/*
	 *  指定されたプレフィックスが割り当てられているインタフェースの
	 *  アドレス情報を探す。
	 */
	for (ix = NUM_IN6_IFADDR_ENTRY; ix --; ) {
		ia = &ifp->in6_ifaddrs[ix];
		if (ia->prefix_index == (pr - nd6_prefix))
			break;
		}

	if (ix >= 0) {

		/* アドレス情報を使用不可にする。*/
		ia->flags |= IN6_IFF_DETACHED;

		/* プレフィックスをオフリンクに設定する。*/
		pr->flags &= ~ND6_PREFIX_STATE_ONLINK;
		}

	return E_OK;
	}

/*
 *  in6_init_prefix_ltimes -- プレフィックスの有効時間を設定する。
 */

static ER
in6_init_prefix_ltimes (T_ND6_PREFIX *pr)
{
	SYSTIM		now;

	/* 推奨有効時間が有効時間より長いときはエラー */
	if (pr->pltime > pr->vltime)
		return E_PAR;

	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

	if (pr->pltime == ND6_INFINITE_LIFETIME)
		pr->preferred = 0;
	else
		pr->preferred = pr->pltime + now;

	if (pr->vltime == ND6_INFINITE_LIFETIME)
		pr->expire = 0;
	else
		pr->expire = pr->vltime + now;

	return E_OK;
	}

/*
 *  nd6_prelist_add -- プレフィックスリストにプレフィックスを追加する。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

static T_ND6_PREFIX *
nd6_prelist_add (T_ND_OPT_PREFIX_INFO *pi, int_t rtr_index)
{
	T_ND6_PREFIX	*pr = NULL;
	int_t		ix;

	/* プレフィックスリストから空きのプレフィックスエントリを探索する。*/
	for (ix = NUM_ND6_PREFIX_ENTRY; ix --; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_STATE_BUSY) == 0)
			break;
		}

	/* プレフィックスリストに空きがなければエラー */
	if (ix < 0)
		return NULL;

	pr->flags |= ND6_PREFIX_STATE_BUSY;

	/* プレフィックスを追加する。*/
	pr->prefix     = pi->prefix;
	pr->prefix_len = pi->prefix_len;

	/* フラグを設定する。*/
	if ((pi->flags & ND_OPT_PI_FLAG_ONLINK) != 0)
		pr->flags |= ND6_PREFIX_FLAG_ONLINK;
	if ((pi->flags & ND_OPT_PI_FLAG_AUTO) != 0)
		pr->flags |= ND6_PREFIX_FLAG_AUTO;

	/* プレフィックスの有効時間を設定する。*/
	pr->vltime = ntohl(pi->valid);
	pr->pltime = ntohl(pi->preferred);
	in6_init_prefix_ltimes(pr);

	if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) != 0) {
		if (nd6_prefix_onlink(pr) != E_OK)
			return NULL;
		}

	/* ルータマップを初期化する。*/
	pr->routermap = 0;

	/* ルータを追加する。*/
	pfxrtr_add(pr - nd6_prefix, rtr_index);

	return pr;
	}

/*
 *  prelist_update -- プレフィックス・リストを更新する。
 */

static void
prelist_update (T_ND_OPT_PREFIX_INFO *pi, uint_t router_index)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_ND6_PREFIX	*pr;
	T_IN6_IFADDR	*ia = NULL;
	int_t		ix, ifidlen;

	syscall(wai_sem(SEM_ND6_DEFRTRLIST));
	if ((pr = nd6_prefix_lookup(&pi->prefix, pi->prefix_len)) != NULL) {
		/*
		 *  登録済みの場合は、プレフィックスリストを更新する。
		 */

		/*
		 *  フラグの更新の注意:
		 *    on-link ビットと autonomous ビットは
		 *    1 から 0 に変更してはならない。
		 */

		if ((pi->flags & ND_OPT_PI_FLAG_ONLINK) != 0)
			pr->flags |= ND6_PREFIX_FLAG_ONLINK;
		if ((pi->flags & ND_OPT_PI_FLAG_AUTO) != 0)
			pr->flags |= ND6_PREFIX_FLAG_AUTO;

		if ((pi->flags & ND_OPT_PI_FLAG_ONLINK) != 0) {
			/*
			 *  通知されたプレフィックスがオンリンクの時は、
			 *  有効時間を更新する。
			 */
			pr->vltime = ntohl(pi->valid);
			pr->pltime = ntohl(pi->preferred);
			in6_init_prefix_ltimes(pr);
			}

		if ((pi->flags & ND_OPT_PI_FLAG_ONLINK  ) != 0 &&
		    (pr->flags & ND6_PREFIX_STATE_ONLINK) == 0) {
			/*
			 *  オフリンクのプレフィックスがオンリンクになった時の処理を実行する。
			 */
			if (nd6_prefix_onlink(pr) != E_OK) {
				syscall(sig_sem(SEM_ND6_DEFRTRLIST));
				syslog(LOG_ERROR, "[ND6 RTR] prefix update failed.");
				return;
				}
			}

		if (router_index != IN6_RTR_IX_UNREACH && pfxrtr_lookup(pr, router_index) == IN6_RTR_IX_UNREACH)
			pfxrtr_add(pr - nd6_prefix, router_index);
		}
	else {	/* 未登録の場合 */

		/* 有効時間 0 のプレフィックスは追加しない。*/
		if (pi->valid == 0) {
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			return;
			}

		/* オフリンクでアドレス自動設定に使用しなければ追加しない。*/
		if ((pi->flags & (ND_OPT_PI_FLAG_ONLINK | ND_OPT_PI_FLAG_AUTO)) !=
		                 (ND_OPT_PI_FLAG_ONLINK | ND_OPT_PI_FLAG_AUTO)) {
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			return;
			}

		/* プレフィックスを追加する。*/
		if ((pr = nd6_prelist_add(pi, router_index)) == NULL) {
			syslog(LOG_ERROR, "[ND6 RTR] prefix add failed.");
			syscall(sig_sem(SEM_ND6_DEFRTRLIST));
			return;
			}

#if 1		/* プレフィックスが未登録の場合だけ表示する。*/
		syslog(LOG_NOTICE, "[ND6 RTR] add prefix: flags[MOH]: %02x, addr: %s\n"
		                   "                                      from: %s.",
		                   nd6_def_router[router_index].flags,
		                   ipv62str(NULL, &pi->prefix),
		                   ipv62str(NULL, &nd6_def_router[router_index].addr));
#endif
		if ((pr->flags & ND6_PREFIX_FLAG_ONLINK) == 0) {
			/*
			 *  オフリンク・プレフィックスの場合は、有効時間を 0 に設定する。
			 *  あとで有効期限切れとなり、リストから削除される。
			 */
			pr->vltime = 0;
			pr->pltime = 0;
			in6_init_prefix_ltimes(pr);
			}
		}

	/*
	 *  ステートレス・アドレス自動設定を行う。
	 */
	if ((pr->flags & ND6_PREFIX_FLAG_AUTO) == 0) {
		syscall(sig_sem(SEM_ND6_DEFRTRLIST));
		return;
		}

	/*
	 *  通知されたプレフィックスが割り当てられているインタフェースの
	 *  アドレス情報を探す。
	 */
	for (ix = NUM_IN6_IFADDR_ENTRY; ix --; ) {
		ia = &ifp->in6_ifaddrs[ix];
		if ((ia->flags & IN6_IFF_DEFINED) != 0 &&
		    ia->prefix_len == pr->prefix_len &&
		    in6_are_prefix_equal(&ia->addr, &pr->prefix, pr->prefix_len))
			break;
		}

	if (ix >= 0) {	/* 登録済みのとき */
		if ((ia->flags & IN6_IFF_AUTOCONF) == 0) {
			/*
			 *  ステートレス・アドレス自動設定により
			 *  設定されていなければ更新しない。
			 */
			;
			}
		else if (ia->flags & IN6_IFF_TENTATIVE) {
			/*
			 *  重複検出中なら更新しない。
			 */
			;
			}
		else {
			T_IN6_ADDR	mask, addr;
			T_IN6_IFADDR	*lla;
			uint32_t		vltime;
			SYSTIM		now;
			int_t		ix;

			/* 有効時間を設定する。*/
			syscall(get_tim(&now));

			if (ia->lifetime.vltime == ND6_INFINITE_LIFETIME)
				vltime = ND6_INFINITE_LIFETIME;
			else if (IFA6_IS_INVALID(ia, now))
				vltime = 0;
			else
				vltime = ia->lifetime.expire - now / SYSTIM_HZ;

			if (pr->vltime > (2 * 60 * 60) || pr->vltime > vltime) {
				/*
				 *  lifetime.expire、pi->valid (有効時間) と
				 *  pi->preferred (推奨有効時間) の単位は [s]。
				 *  通知された有効時間が 2 時間以上か、現在の残り時間より
				 *  長ければ、新しい有効時間には、通知された有効時間を設定する。
				 */
				vltime = pr->vltime;
				}
			else if (vltime <= (2 * 60 * 60))
				;
			else {
				/*
				 *  IPsec は未実装のため、2 時間を設定する。
				 */
				vltime = 2 * 60 * 60;
				}

			lla = &ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL];

			/* プレフィックスマスクを生成する。*/
			in6_plen2pmask(&mask, pr->prefix_len);

			/* アドレスを生成する。*/
			for (ix = 0; ix < 4; ix ++)
				addr.s6_addr32[ix] = (pr->prefix.s6_addr32[ix] &  mask.s6_addr32[ix])
				                   | ( lla->addr.s6_addr32[ix] & ~mask.s6_addr32[ix]);

			in6_update_ifa(ifp, ia, &addr, pr->prefix_len, vltime,
			               pr->pltime, router_index, pr - nd6_prefix,
			               IN6_IFF_AUTOCONF | IN6_IFF_NODAD);
			}
		}

	else { /* 未登録のとき */

		/*
		 *  プレフィックス長をチェックする。
		 *  もし、プレフィックス長とインタフェース ID の合計が
		 *  128 ビットに一致しない時は、プレフィックス情報を無視する。
		 */
		if ((ifidlen = in6_if2idlen(ifp)) < 0)
			syslog(LOG_ERROR, "[ND6 RTR] IFID undefined.");
		else if (ifidlen + pr->prefix_len != 128)
			syslog(LOG_NOTICE, "[ND6 RTR] invalid prefix len: %d.", pr->prefix_len);
		else {

			for (ix = NUM_IN6_IFADDR_ENTRY; ix --; ) {
				ia = &ifp->in6_ifaddrs[ix];
				if ((ia->flags & IN6_IFF_DEFINED) == 0) {
					in6_ifadd(pr, ia, router_index);
					pfxlist_onlink_check();
					break;
					}
				}

			if (ix < 0)
				syslog(LOG_ERROR, "[ND6 RTR] prefix update failed.");
			}
		}
	syscall(sig_sem(SEM_ND6_DEFRTRLIST));
	}

/*
 *  nd6_prelist_timer -- プレフィックス・リストの管理タイマー
 */

void
nd6_prelist_timer (void)
{
	T_ND6_PREFIX	*pr;
	SYSTIM		now;
	int_t		ix;

	syscall(get_tim(&now));
	now /= SYSTIM_HZ;		/* 有効時間の単位は [s] */

	syscall(wai_sem(SEM_ND6_DEFRTRLIST));
	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_STATE_BUSY) != 0 && (int32_t)(pr->expire - now) <= 0)
		                                                       /* pr->expire <= now */
			prelist_remove(pr);
		}
	syscall(sig_sem(SEM_ND6_DEFRTRLIST));
	}

/*
 *  nd6_rs_output -- ルータ要請を出力する。
 */

static void
nd6_rs_output (void)
{
	T_ROUTER_SOLICIT_HDR	*rsh;
	T_NET_BUF		*output;
	T_IN6_IFADDR		*ifa;
	const T_IN6_ADDR	*saddr;
	uint16_t		len;

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_RS_OUT_PACKETS], 1);

	/*
	 *  送信元アドレスには、リンクローカルアドレスを設定するが、
	 *  まだ設定されいなければ、無指定を設定する。
	 */
	if ((ifa = in6ifa_ifpwithix(IF_GET_IFNET(), IPV6_IFADDR_IX_LINKLOCAL)) != NULL && !IFA6_IS_NOTREADY(ifa))
		saddr = &ifa->addr;
	else
		saddr = &in6_addr_unspecified;

	len = (ROUTER_SOLICIT_HDR_SIZE + 7) >> 3 << 3;

	/* ネットワークバッファを獲得し、IPv6 ヘッダを設定する。*/
	if (in6_get_datagram(&output, len, 0, &in6_addr_linklocal_allrouters,
	                     saddr, IPPROTO_ICMPV6,
	                     IPV6_MAXHLIM, NBA_SEARCH_ASCENT, TMO_ND6_RS_OUTPUT) != E_OK)
		return;

	/* ルータ要請ヘッダを設定する。*/
	rsh = GET_ROUTER_SOLICIT_HDR(output, IF_IP6_ROUTER_SOLICIT_HDR_OFFSET);
	rsh->hdr.type       = ND_ROUTER_SOLICIT;
	rsh->hdr.code       = 0;
	rsh->nd_rs_reserved = 0;

	/* チェックサムを計算する。*/
	rsh->hdr.sum = 0;
	rsh->hdr.sum = in6_cksum(output, IPPROTO_ICMPV6, (uint8_t*)rsh - output->buf, len);

	/* 送信する。*/
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_OCTETS],
	               output->len - GET_IF_IP6_HDR_SIZE(output));
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutRouterSolicits, 1);
	ip6_output(output, IPV6_OUT_SET_HOP_LIMIT(IPV6_OUT_FLG_HOP_LIMIT, IPV6_MAXHLIM), TMO_ND6_RS_OUTPUT);
	}

/*
 *  nd6_defrtrlist_lookup -- ディフォルトルータ・リストを探索する。
 */

T_DEF_ROUTER *
nd6_defrtrlist_lookup (const T_IN6_ADDR *src)
{
	T_DEF_ROUTER	*dr = NULL;
	int_t		ix;

	for (ix = def_router_count; ix -- > 0; ) {
		dr = &nd6_def_router[ix];
		if (IN6_ARE_ADDR_EQUAL(&dr->addr, src)) {
			return dr;
			}
		}
	return NULL;
	}

/*
 *  nd6_router_lookup -- ルータのアドレスを返す。
 *
 */

const T_IN6_ADDR *
nd6_router_lookup (void)
{
	if (def_router_count > 0)
		return &nd6_def_router[0].addr;
	else
		return NULL;
	}

/*
 *  nd6_defrtrlist_del -- ディフォルトルータ・リストからルータを削除する。
 *
 *    注意: この関数を呼び出すときは、SEM_ND6_DEFRTRLIST で、
 *          ロックしてから呼び出すこと。
 */

void
nd6_defrtrlist_del (T_DEF_ROUTER *dr)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_IFADDR	*ia;
	T_ND6_PREFIX	*pr;
	uint_t		mask;
	int_t		ix, rix, pix, rtrix = dr - nd6_def_router;

	if ((ia = in6ifa_ifpwithrtrix(ifp, rtrix)) != NULL) {
		/*
		 *  ネットワークインタフェースのプレフィックスを通知した
		 *  ルータを到達不能にする。
		 */
		ia->router_index = IN6_RTR_IX_UNREACH;
		}

	/* プレフィックスリストからルータを削除する。*/
	mask = make_mask(rtrix);
	for (pix = NUM_ND6_PREFIX_ENTRY; pix --; )
		nd6_prefix[pix].routermap &= ~mask;

	/* 削除したルータ以降を前に詰める。*/
	for (rix = rtrix + 1; rix < def_router_count; rix ++)
		nd6_def_router[rix - 1] = nd6_def_router[rix];

	/* プレフィックスリストのルータも更新する。*/
	if (dr != nd6_def_router) {

		/* マスクを生成する。*/
		mask = 1;
		for (ix = rtrix - 1; ix -- > 0; )
			mask = (mask << 1) | 1;

		for (pix = NUM_ND6_PREFIX_ENTRY; pix --; ) {
			pr = &nd6_prefix[pix];
			pr->routermap = (pr->routermap & mask) | (((pr->routermap & ~mask) >> 1) & ~mask);
			}
		}
	def_router_count --;

	/* プレフィックスのオンリンク状態を確認する。*/
	pfxlist_onlink_check();

	/*
	 *  削除するルータが優先ディフォルトルータの場合は、
	 *  優先ディフォルトルータを選択しなおす。
	 */
	if (rtrix == 0)
		defrouter_select();
	}

/*
 *  nd6_defrtrlist_timer -- ディフォルトルータ・リストの管理タイマー
 */

void
nd6_defrtrlist_timer (void)
{
	T_DEF_ROUTER	*dr;
	SYSTIM		now;
	int_t		ix;

	syscall(get_tim(&now));

	syscall(wai_sem(SEM_ND6_DEFRTRLIST));
	for (ix = def_router_count; ix -- > 0; ) {
		dr = &nd6_def_router[ix];
		if ((int32_t)(dr->expire - now) <= 0) {
		           /* dr->expire <= now */
			nd6_defrtrlist_del(dr);
			}
		}
	syscall(sig_sem(SEM_ND6_DEFRTRLIST));
	}

/*
 *  nd6_get_drl -- ディフォルトルータ・リストを獲得する。
 */

const T_DEF_ROUTER *
nd6_get_drl (uint_t *count)
{
	*count = def_router_count;
	return nd6_def_router;
	}

/*
 *  nd6_get_prl -- プレフィックス・リストを獲得する。
 */

const T_ND6_PREFIX *
nd6_get_prl (void)
{
	return nd6_prefix;
	}

#endif	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */

/*
 *  nd6_ra_input -- ルータ通知の入力処理。
 */

void
nd6_ra_input (T_NET_BUF *input, uint_t off)
{
#if NUM_ND6_DEF_RTR_ENTRY > 0

	T_IFNET			*ifp = IF_GET_IFNET();
	T_IP6_HDR		*ip6h;
	T_ROUTER_ADVERT_HDR	*rah;
	T_ND_OPT_HDR		*opth;
	SYSTIM			now;
	int_t			rix;
	uint_t			lladdr_len = 0;
	uint32_t		advrea;
	uint8_t			*lladdr = NULL;
	uint8_t			nd_opt_off[ND_OPT_OFF_ARRAY_SIZE];

	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_RA_IN_PACKETS], 1);

	ip6h = GET_IP6_HDR(input);

	/*
	 *  ヘッダのチェック、以下の場合は破棄する。
	 *    ・ホップリミットが IPV6_MAXHLIM (255) 以外
	 *    ・ヘッダ長が短い
	 *    ・始点アドレスが、リンクローカルではない (RFC2461)
	 */
	if (ip6h->hlim != IPV6_MAXHLIM                ||
	    input->len - off < ROUTER_ADVERT_HDR_SIZE ||
	    !IN6_IS_ADDR_LINKLOCAL(&ip6h->src))
		goto err_ret;

	rah  = GET_ROUTER_ADVERT_HDR(input, off);

	/* 近隣探索オプションのオフセットを記録する。*/
	if (nd6_options(nd_opt_off, input->buf + (off + ROUTER_ADVERT_HDR_SIZE),
	                            input->len - (off + ROUTER_ADVERT_HDR_SIZE)) != E_OK)
		goto err_ret;
	/*
	 *  到達可能状態の有効時間の更新

	 */
	if (rah->reachable) {
		advrea = ntohl(rah->reachable) * SYSTIM_HZ / 1000;	/* rah->reachable の単位は ms */
		if (advrea <= ND6_MAX_REACHABLE_TIME &&
		    advrea != nd6_base_reachable_time) {
			syscall(get_tim(&now));
			nd6_recalc_reachtm_interval
				= now + ND6_RECALC_REACHTM_INTERVAL;
			nd6_base_reachable_time	= advrea;
			nd6_reachable_time
				= ND6_CALC_REACH_TIME(nd6_base_reachable_time);
			}
		}

	/* 近隣探索の送信間隔の更新 */
	if (rah->retransmit) {
		nd6_retrans_time = ntohl(rah->retransmit) * SYSTIM_HZ / 1000;
									/* rah->retransmit の単位は ms */
		}

	/* ホップリミットの規定値の更新 */
	if (rah->nd_ra_current_hoplimit) {
		ip6_defhlim = rah->nd_ra_current_hoplimit;
		}

	/* ディフォルトルータリストの更新 */
	rix = defrtrlist_update(&ip6h->src,
	                       ntohs(rah->nd_ra_router_lifetime),
	                       rah->nd_ra_flags);		/* rah->nd_ra_router_lifetime の単位は秒 */

	/* プレフィックスリストの更新 */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_PREFIX_INFO_COUNT)]) {
		T_ND_OPT_PREFIX_INFO	*pi;
		uint_t			pi_off, pi_count;

		pi_count = nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_PREFIX_INFO_COUNT)];
		pi_off   = off + ROUTER_ADVERT_HDR_SIZE
		               + nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_PREFIX_INFORMATION)] - 8;
	 	/* 注意: オプションオフセット配列には、オフセット + 8 が設定されている。*/
		while (pi_count -- > 0) {
			pi = (T_ND_OPT_PREFIX_INFO*)(input->buf + pi_off);
			if (pi->type != ND_OPT_PREFIX_INFORMATION)
				/* オプションタイプがプレフィックス情報でなければ何もしない。*/
				;
			else if (pi->len != 4)
			    	/* プレフィックス情報の長さが 32 (8×4) でなければ不正 */
				syslog(LOG_NOTICE, "[ND6 RTR] prefix opt len: %d.", pi->len);
			else if (pi->prefix_len > 128)
				/* プレフィックス長は 128 ビット以下 */
				syslog(LOG_NOTICE, "[ND6 RTR] prefix len: %d.", pi->prefix_len);
			else if (IN6_IS_ADDR_MULTICAST(&pi->prefix) ||
			         IN6_IS_ADDR_LINKLOCAL(&pi->prefix))
				/* マルチキャストとリンクローカルは不正 */
				syslog(LOG_NOTICE, "[ND6 RTR] invalid addr: %s from %s.",
				                   ipv62str(NULL, &pi->prefix), ipv62str(NULL, &ip6h->src));
#if 0	/* FreeBSD 6.3 で削除されている。*/
			else if (IN6_IS_ADDR_AGGREGATE_UNICAST(&pi->prefix) &&		
			         (pi->prefix_len != 64)                       )
				/* 集約可能ユニキャストで、プレフィックス長が 64 ビット以外は不正 */
				syslog(LOG_NOTICE, "[ND6 RTR] invalid prefix len for global: %d.", pi->prefix_len);
#endif
			else if (ntohl(pi->preferred) > ntohl(pi->valid))
				/* 推奨有効時間が有効時間より長ければ不正 */
				syslog(LOG_NOTICE, "[ND6 RTR] preferred(%d) > valid time(%d).", ntohl(pi->preferred), ntohl(pi->valid));
			else {
#if 0	/* プレフィックスが未登録の場合だけ表示する。*/
				syslog(LOG_NOTICE, "[ND6 RTR] update prefix: flags[MOH]: %02x, addr: %s\n"
				                   "                                         from: %s.",
				                   rah->nd_ra_flags, ipv62str(NULL, &pi->prefix), ipv62str(NULL, &ip6h->src));
#endif
				if (rix >= 0) {
					/*
					 *  defrtrlist_update の戻り値は、ルータのインデックス。
					 *  ただし、値が -1 なら、プレフィックス・リストは更新しない。
					 */
					prelist_update(pi, rix);	/* プレフィックス・リストを更新する。*/
					}
				}
			pi_off += pi->len << 3;		/* オプション長の単位は 8 バイト */
			}
		}

	/* MTU の更新 */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_MTU)]) {
		T_ND_OPT_MTU_HDR	*optmtuh;
		uint32_t			mtu;

		optmtuh = (T_ND_OPT_MTU_HDR *)((uint8_t *)(input->buf + off + ROUTER_ADVERT_HDR_SIZE) +
		                               nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_MTU)] - 8);

		/*
		 *  値のチェック
		 *
		 *    オプション長: 1
		 *    予約:         0
		 */
		if (optmtuh->len != 1 || optmtuh->reserved != 0)
			goto err_ret;

		/* MTU が IPv6 MMTU（1280 オクテット）以下なら無視する。*/
		mtu = ntohl(optmtuh->mtu);
		if (mtu >= IPV6_MMTU) {
			if (mtu <= (linkmtu > IF_MTU ? linkmtu : IF_MTU))
				linkmtu = mtu;
			}
		}

	/*
	 *  ルータの始点リンクアドレスを近隣キャッシュに登録する。
	 */
	if (nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_SOURCE_LINKADDR)]) {
		opth = (T_ND_OPT_HDR *)((uint8_t *)(input->buf + off + ROUTER_ADVERT_HDR_SIZE) +
		                        nd_opt_off[ND_OPT_OFF_ARRAY_IX(ND_OPT_SOURCE_LINKADDR)] - 8);
	 	/* 注意: オプションオフセット配列には、オフセット + 8 が設定されている。*/
		lladdr     = (uint8_t *)(opth + 1);
		lladdr_len = (opth->len << 3);
		}

	/*
	 *  ネットワークインタフェースのアドレス長が一致しなければエラー
	 */
	if (lladdr && lladdr_len != ((sizeof(T_IF_ADDR) + sizeof(T_ND_OPT_HDR) + 7) & ~7))
		goto err_ret;

	/* 近隣キャッシュに登録する。*/
	nd6_cache_lladdr(ifp, &ip6h->src, (T_IF_ADDR *)lladdr, ND_ROUTER_ADVERT, 0);

	/*
	 *  ルータの始点リンクアドレスを近隣キャッシュに登録したので、
	 *  オンリンク状態が変化する可能性があるため、それをチェックする。
	 */
	pfxlist_onlink_check();

	syscall(rel_net_buf(input));
	return;

err_ret:
	NET_COUNT_ICMP6(net_count_nd6[NC_ICMP6_IN_ERR_PACKETS], 1);
	syscall(rel_net_buf(input));

#else	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */

	/*
	 *  ルータ通知を受信しない場合は、破棄して終了する。
	 */
	NET_COUNT_ICMP6(net_count_nd6[NC_ND6_RA_IN_PACKETS], 1);
	syscall(rel_net_buf(input));

#endif	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */
	}

#if NUM_ND6_RTR_SOL_RETRY > 0

/*
 *  nd6_rtrsol_ctl -- ルータ要請出力の制御
 */

void
nd6_rtrsol_ctl (void)
{
	T_IN6_IFADDR	*ifa;
	int_t		try;

	/* リンクローカルアドレスのアドレス情報を得る。*/
	if ((ifa = in6ifa_ifpwithix(IF_GET_IFNET(), IPV6_IFADDR_IX_LINKLOCAL)) == NULL)
		return;

	/* リンクローカルアドレスの重複検出終了を待つ。*/
	do {
		/* リンクローカルアドレスの重複検出終了待ち時間を設定する。*/
		syscall(dly_tsk(net_rand() % TMO_ND6_RTR_SOL_DELAY));

		/* 待っている間に重複が検出されたら終了する。*/
		if (ifa->flags & IN6_IFF_DUPLICATED)
			return;

		} while (!((ifa->flags & IN6_IFF_DEFINED) && (ifa->flags & IN6_IFF_TENTATIVE) == 0));

	for (try = 0; try < NUM_ND6_RTR_SOL_RETRY; try ++) {

		/* ディフォルトルータ・リストにエントリがあれば終了 */
		if (def_router_count > 0)
			return;

		/* ルータ要請を出力する。*/
		nd6_rs_output();

		/* 出力の 2 回目以降の遅延時間を設定する。*/
		syscall(dly_tsk(TMO_ND6_RTR_SOL_INTERVAL));
		}
	}

#endif	/* of #if NUM_ND6_RTR_SOL_RETRY > 0 */

/*
 *  nd6_onlink_prefix_lookup -- プレフィックスを探索する。
 */

T_ND6_PREFIX *
nd6_onlink_prefix_lookup (const T_IN6_ADDR *dst)
{
	SYSTIM		now;
	T_ND6_PREFIX	*pr;
	int_t		ix;

	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

	/* 推奨有効時間内のプレフィックスを探索する。*/
	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_STATE_BUSY)  &&
		    (pr->flags & ND6_PREFIX_FLAG_ONLINK) &&
		    in6_are_prefix_equal(dst, &pr->prefix, pr->prefix_len) &&
		    (int32_t)(pr->preferred - now) > 0) {
		           /* pr->preferred > now */
			return pr;
			}
		}

	/* 有効時間内のプレフィックスを探索する。*/
	for (ix = NUM_ND6_PREFIX_ENTRY; ix -- > 0; ) {
		pr = &nd6_prefix[ix];
		if ((pr->flags & ND6_PREFIX_STATE_BUSY)  &&
		    (pr->flags & ND6_PREFIX_FLAG_ONLINK) &&
		    in6_are_prefix_equal(dst, &pr->prefix, pr->prefix_len) &&
		    (int32_t)(pr->expire - now) > 0) {
		           /* pr->expire > now */
			return pr;
			}
		}

	return NULL;
	}

#endif /* of #ifdef _IP6_CFG */
