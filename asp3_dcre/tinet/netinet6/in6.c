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

/*	$FreeBSD: src/sys/netinet6/in6.c,v 1.21 2002/04/19 04:46:22 suz Exp $	*/
/*	$KAME: in6.c,v 1.259 2002/01/21 11:37:50 keiichi Exp $	*/

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
 * Copyright (c) 1982, 1986, 1991, 1993
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
 *	@(#)in.c	8.2 (Berkeley) 11/15/93
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_types.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <netinet6/in6_ifattach.h>

#include <net/if_var.h>

#ifdef _IP6_CFG

/*
 *  全域変数
 */

const T_IN6_ADDR in6_addr_unspecified =
	IPV6_ADDR_UNSPECIFIED_INIT;

const T_IN6_ADDR in6_addr_linklocal_allnodes =
	IPV6_ADDR_LINKLOCAL_ALLNODES_INIT;

const T_IN6_ADDR in6_addr_linklocal_allrouters =
	IPV6_ADDR_LINKLOCAL_ALLROUTERS_INIT;

/*
 *  in6_addmulti -- マルチキャストアドレスを登録する。
 */

static ER
in6_addmulti (T_IFNET *ifp, T_IN6_ADDR *maddr)
{
	ER	error = E_OK;

	if ((error = if_addmulti(ifp, maddr, AT_INET6)) != E_OK)
		return error;

	return error;
	}

/*
 *  in6_ifinit -- インタフェースにアドレス情報を設定し、初期化する。
 */

static ER
in6_ifinit (T_IFNET *ifp, T_IN6_IFADDR *ia, const T_IN6_ADDR *addr, uint_t prefix_len)
{
	/* アドレスとプレフィックス長を設定する。*/
	ia->addr       = *addr;
	ia->prefix_len = prefix_len;

	/* フラグを初期化する。*/
	ia->flags = 0;

	/*
	 *  初めてアドレスが設定されたとき、インタフェース独自の初期化を行う。
	 */

	return E_OK;
	}

/*
 *  in6_ifainit -- インタフェースアドレス構造体の初期化
 */

void
in6_ifainit (void)
{
	T_IN6_IFADDR	*ia = NULL;
	T_IFNET		*ifp = IF_GET_IFNET();
	int_t 		ix;

	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
		ia = &ifp->in6_ifaddrs[ix];
		memset(ia, sizeof(T_IN6_IFADDR), 0);
		ia->router_index = IN6_RTR_IX_UNREACH;
		ia->prefix_index = ND6_PREFIX_IX_INVALID;
		}
	}

/*
 *  in6_if2idlen -- インタフェース ID 長を返す。
 */

int_t
in6_if2idlen (T_IFNET *ifp)
{
#if IF_TYPE == IFT_ETHER
	return 64;
#else
	return 64;
#endif
	}

/*
 *  in6_addr2ifaix -- アドレスからインタフェースアドレス情報のインデックスに変換する。
 */

int_t
in6_addr2ifaix (const T_IN6_ADDR *addr)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN6_IFADDR	*ia;
	int_t 		ix;

	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
		ia = &ifp->in6_ifaddrs[ix];
		if ((ia->flags & IN6_IFF_DEFINED) != 0 &&
		    in6_are_prefix_equal(addr, &ia->addr, ia->prefix_len))
			return ix;
		}

	return IPV6_IFADDR_IX_INVALID;
	}

/*
 *  in6_addr2maix -- アドレスからマルチキャストアドレスのインデックスに変換する。
 */

int_t
in6_addr2maix (const T_IN6_ADDR *addr)
{
	if (addr->s6_addr8[0] == 0xff) {
		if (addr->s6_addr8[1] == 0x02) {
			if (addr->s6_addr8[11] == 0x01 && addr->s6_addr8[12] == 0xff)
				return IPV6_MADDR_IX_SOL_NODE;
			else
				return IPV6_MADDR_IX_LL_ALL_NODE;
			}
		else
			return IPV6_MADDR_IX_EXTRA;
		}
	else
		return IPV6_MADDR_IX_EXTRA;
	}

/*
 *  in6_update_ifa -- インタフェースのアドレス情報を更新する。
 */

ER
in6_update_ifa (T_IFNET *ifp, T_IN6_IFADDR *ia, const T_IN6_ADDR *addr,
                uint_t prefix_len, uint32_t vltime, uint32_t pltime,
                int_t router_index, int_t prefix_index, uint_t flags)
{
	T_IN6_ADDR	llsol;
	ER		error = E_OK;
	SYSTIM		now;

	/* 引数をチェックする。*/
	if (addr == NULL)
		return E_PAR;

	/*
	 *  有効時間をチェックする。
	 *  vltime (有効時間) と pltime (推奨有効時間) の単位は [s]。
	 */
	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

	if (((vltime != ND6_INFINITE_LIFETIME) && (vltime + now < now)) || vltime == 0)
		return E_PAR;

	if ((pltime != ND6_INFINITE_LIFETIME) && (pltime + now < now))
		return E_PAR;

	/* インタフェースを初期化する。*/
	if ((error = in6_ifinit(ifp, ia, addr, prefix_len)) != E_OK)
		return error;

	if (IN6_IS_ADDR_LINKLOCAL(addr)) {

		/* 要請マルチキャストアドレスを登録する。*/
		llsol.s6_addr32[0] = IPV6_ADDR_INT32_MLL;
		llsol.s6_addr32[1] = 0x00000000;
		llsol.s6_addr32[2] = IPV6_ADDR_INT32_ONE;
		llsol.s6_addr32[3] = addr->s6_addr32[3];
		llsol.s6_addr8[12] = 0xff;
		if ((error = in6_addmulti(ifp, &llsol)) != E_OK)
			return error;

		/* 全ノード・リンクローカル・マルチキャストアドレスを登録する。*/
		llsol.s6_addr32[2] = 0x00000000;
		llsol.s6_addr32[3] = IPV6_ADDR_INT32_ONE;
		if ((error = in6_addmulti(ifp, &llsol)) != E_OK)
			return error;

		}

	/* ルータインデックスを設定する。*/
	ia->router_index = router_index;

	/* プレフィックスインデックスを設定する。*/
	ia->prefix_index = prefix_index;

	/* フラグを設定する。*/
	ia->flags  = flags & ~(IN6_IFF_DUPLICATED | IN6_IFF_NODAD);
	ia->flags |= IN6_IFF_DEFINED;

 	/*
 	 *  アドレス有効時間を設定する。
	 *  vltime (有効時間) と pltime (推奨有効時間) の単位は [s]。
 	 */
	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

 	ia->lifetime.vltime = vltime;
 	if (vltime != ND6_INFINITE_LIFETIME)
 		ia->lifetime.expire = now + vltime;
 	else
 		ia->lifetime.expire = 0;

 	ia->lifetime.pltime = pltime;
 	if (pltime != ND6_INFINITE_LIFETIME)
 		ia->lifetime.preferred = now + pltime;
 	else
 		ia->lifetime.preferred = 0;

	/* 近隣探索の初期設定を行う。*/
	nd6_ifattach(ifp);

	/* 重複アドレス検出を行う。*/
	if ((flags & IN6_IFF_NODAD) == 0) {
		ia->flags |= IN6_IFF_TENTATIVE;
		nd6_dad_start(ifp, ia, NULL);
		}

	return error;
	}

/*
 * in6ifa_ifpwithix -- インデックス番号から、ネットワークインタフェースに
 *                     割り当てられているアドレスを探索する。
 */

T_IN6_IFADDR *
in6ifa_ifpwithix (T_IFNET *ifp, int_t ix)
{
	return ix < NUM_IN6_IFADDR_ENTRY? &ifp->in6_ifaddrs[ix] : NULL;
	}

/*
 * in6ifa_ifpwithrtrix -- ルータのインデックス番号から、ネットワークインタフェースに
 *                        割り当てられているアドレスを探索する。
 */

T_IN6_IFADDR *
in6ifa_ifpwithrtrix (T_IFNET *ifp, int_t router_index)
{
	int_t ix;

	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; )
		if (ifp->in6_ifaddrs[ix].router_index == router_index)
			return &ifp->in6_ifaddrs[ix];
	return NULL;
	}

/*
 * in6_ifawithifp -- 宛先アドレスにふさわしい送信元アドレス情報を、
 *                   ネットワークインタフェースから探索する。
 */

T_IN6_IFADDR *
in6_ifawithifp (T_IFNET *ifp, const T_IN6_ADDR *dst)
{
	uint_t	scope;
	int_t	ix;

	/* 宛先アドレスと同じスコープのアドレスを返す。*/
	if ((ix = in6_addr2ifaix(dst)) != IPV6_IFADDR_IX_INVALID)
		return &ifp->in6_ifaddrs[ix];
	else if ((ix = in6_addr2maix(dst)) == IPV6_MADDR_IX_SOL_NODE ||
	          ix                       == IPV6_MADDR_IX_LL_ALL_NODE)
		return &ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL];
	else {
		scope = in6_addrscope(dst);
		for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
			if ((ifp->in6_ifaddrs[ix].flags & IN6_IFF_DEFINED) &&
			    (ifp->in6_ifaddrs[ix].flags & IN6_IFF_DETACHED) == 0 &&
			    in6_addrscope(&ifp->in6_ifaddrs[ix].addr) == scope) {
				return &ifp->in6_ifaddrs[ix];
				}
			}
		return NULL;
		}
	}

/*
 * in6_addrwithifp -- 宛先アドレスにふさわしい送信元アドレスを、
 *                    ネットワークインタフェースから探索する。
 */

const T_IN6_ADDR *
in6_addrwithifp (T_IFNET *ifp, T_IN6_ADDR *src, const T_IN6_ADDR *dst)
{
	T_IN6_IFADDR *ifaddr;

	if ((ifaddr = in6_ifawithifp(ifp, dst)) == NULL)
		return NULL;
	else {
		*src = ifaddr->addr;
		return src;
		}
	}

/*
 * in6_if_up -- インタフェース起動後の重複アドレス検出を行う。
 */

void
in6_if_up (T_IFNET *ifp)
{
#if 0						/* 保留 */
	int_t dad_delay, ix;
#endif	/* of #if 0 */

	in6_ifattach(ifp);

#if 0						/* 保留 */
	/* 重複アドレス検出を行う。*/
	dad_delay = 0;
	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; )
		if ((ifp->in6_ifaddrs[ix].flags & IN6_IFF_DEFINED) &&
		    (ifp->in6_ifaddrs[ix].flags & IN6_IFF_TENTATIVE))
			nd6_dad_start(ifp, &ifp->in6_ifaddrs[ix], &dad_delay);
#endif	/* of #if 0 */

#if NUM_ND6_RTR_SOL_RETRY > 0

	/* ルータ要請を出力する。*/
	nd6_rtrsol_ctl();

#endif	/* of #if NUM_ND6_RTR_SOL_RETRY > 0 */
	}

/*
 *  in6_are_prefix_equal -- プレフィックスが同一か調べる。
 */

bool_t
in6_are_prefix_equal (const T_IN6_ADDR *addr, const T_IN6_ADDR *prefix, uint_t prefix_len)
{
	uint_t bitlen, bytelen;

	bytelen = prefix_len / 8;
	if (memcmp(&addr->s6_addr, &prefix->s6_addr, bytelen))
		return false;

	bitlen = prefix_len % 8;
	if (  addr->s6_addr[bytelen] >> (8 - bitlen) !=
	    prefix->s6_addr[bytelen] >> (8 - bitlen))
		return false;

	return true;
	}

/*
 *  in6_ifaddr_timer -- ネットワークインタフェースのアドレスの管理タイマー
 */

void
in6_ifaddr_timer (T_IFNET *ifp)
{
	T_IN6_IFADDR	*ia;
	SYSTIM		now;
	int_t		ix;

	syscall(get_tim(&now));
	for (ix = NUM_IN6_IFADDR_ENTRY; ix -- > 0; ) {
		ia = &ifp->in6_ifaddrs[ix];

		if ((ia->flags & IN6_IFF_DEFINED) == 0)
			;
		else if (IFA6_IS_INVALID(ia, now)) {

			/*
			 *  有効時間を過ぎたので削除する。
			 */
			ia->flags &= ~IN6_IFF_DEFINED;
			}

		else if (IFA6_IS_DEPRECATED(ia, now)) {

			/*
			 *  推奨有効時間を過ぎたので非推奨にする。
			 */
			ia->flags |= IN6_IFF_DEPRECATED;
			}
		}
	}

#endif /* of #ifdef _IP6_CFG */
