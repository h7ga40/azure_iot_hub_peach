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

/*	$FreeBSD: src/sys/netinet6/in6_ifattach.c,v 1.2.2.5 2001/08/13 16:26:17 ume Exp $	*/
/*	$KAME: in6_ifattach.c,v 1.118 2001/05/24 07:44:00 itojun Exp $	*/

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

#include <net/if.h>
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_endian.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

#include <netinet6/in6.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <netinet6/in6_ifattach.h>

#include <net/if_var.h>

#ifdef _IP6_CFG

/*
 * EUI64 の定義
 */

#define EUI64_UBIT		0x02

#ifdef SUPPORT_ETHER

/*
 * get_mac6_ifid -- MAC (6 オクテット) アドレスで、インタフェース識別子を設定する。
 */

static ER
get_mac6_ifid (T_IFNET *ifp, T_IN6_ADDR *ifra_addr)
{
	ifra_addr->s6_addr[ 8] = ifp->ic->ifaddr.lladdr[0] ^ EUI64_UBIT;
	ifra_addr->s6_addr[ 9] = ifp->ic->ifaddr.lladdr[1];
	ifra_addr->s6_addr[10] = ifp->ic->ifaddr.lladdr[2];
	ifra_addr->s6_addr[11] = UINT_C(0xff);
	ifra_addr->s6_addr[12] = UINT_C(0xfe);
	ifra_addr->s6_addr[13] = ifp->ic->ifaddr.lladdr[3];
	ifra_addr->s6_addr[14] = ifp->ic->ifaddr.lladdr[4];
	ifra_addr->s6_addr[15] = ifp->ic->ifaddr.lladdr[5];

	return E_OK;
	}

#endif	/* of #ifdef SUPPORT_ETHER */

#ifdef SUPPORT_PPP

/*
 * get_rand_ifid -- 乱数により、インタフェース識別子を設定する。
 */

static ER
get_rand_ifid (T_IFNET *ifp, T_IN6_ADDR *ifra_addr)
{
	return E_OK;
	}

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef IP6_CFG_AUTO_LINKLOCAL

/*
 *  in6_ifattach_linklocal -- インタフェースにリンクローカル・アドレスを割り当てる。
 */

static ER
in6_ifattach_linklocal (T_IFNET *ifp)
{
	T_IN6_ADDR	addr;
	ER		error = E_OK;

	/*
	 *  リンクローカル・プレフィックス fe80::/64 を設定する。
	 */
	addr.s6_addr32[0] = IPV6_ADDR_INT32_ULL;
	addr.s6_addr32[1] = ULONG_C(0);

	/* インタフェース識別子を設定する。*/
	if ((error = IF_IN6_IFID(ifp, &addr)) != E_OK)
		return error;

	/* インタフェースのアドレス情報を更新する。*/
	if ((error = in6_update_ifa(ifp, &ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL],
	                            &addr, 64, ND6_INFINITE_LIFETIME,
	                                       ND6_INFINITE_LIFETIME, 
	                                       IN6_RTR_IX_UNREACH, ND6_PREFIX_IX_INVALID, 0)) != E_OK)
		return error;

	return error;
	}

#endif	/* of #ifdef IP6_CFG_AUTO_LINKLOCAL */

/*
 *  in6_ifattach -- インタフェースにアドレスを割り当てる。
 */

ER
in6_ifattach (T_IFNET *ifp)
{
	ER error = E_OK;

#ifdef IP6_CFG_AUTO_LINKLOCAL

	if ((ifp->in6_ifaddrs[IPV6_IFADDR_IX_LINKLOCAL].flags & IN6_IFF_DEFINED) == 0)
		if ((error = in6_ifattach_linklocal(ifp)) != E_OK)
			return error;

#endif	/* of #ifdef IP6_CFG_AUTO_LINKLOCAL */

	return error;
	}

#endif /* of #ifdef _IP6_CFG */
