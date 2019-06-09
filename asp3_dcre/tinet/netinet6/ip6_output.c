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

/*	$FreeBSD: src/sys/netinet6/ip6_output.c,v 1.43 2002/10/31 19:45:48 ume Exp $	*/
/*	$KAME: ip6_output.c,v 1.279 2002/01/26 06:12:30 jinmei Exp $	*/

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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993
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
 *	@(#)ip_output.c	8.3 (Berkeley) 1/21/94
 */

#include <string.h>

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
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
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
 *  全域変数
 */

/* リンク MTU */

uint32_t	linkmtu = IF_MTU;

/*
 *  局所変数
 */

#ifdef IP6_CFG_FRAGMENT

static uint32_t	ip6_id;

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

/*
 *  ip6_output -- IPv6 の出力関数
 */

ER
ip6_output (T_NET_BUF *output, uint16_t flags, TMO tmout)
{
	T_IP6_HDR		*ip6h;
	const T_IN6_ADDR	*gw;
	T_IFNET			*ifp = IF_GET_IFNET();
	ER			error = E_OK;

#ifdef IP6_CFG_FRAGMENT

	T_IP6_HDR	*fip6h;
	T_IP6_FRAG_HDR	*fip6f;
	T_NET_BUF	*frag;
	uint32_t	mtu, id;
	uint16_t	plen, foff, flen;
	uint8_t		nextproto;
	bool_t		alwaysfrag = false;

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

	ip6h = GET_IP6_HDR(output);

	NET_COUNT_IP6(net_count_ip6[NC_IP6_OUT_OCTETS],
	              GET_IP6_HDR_SIZE(output) + GET_IP6_SDU_SIZE(output));
	NET_COUNT_IP6(net_count_ip6[NC_IP6_OUT_PACKETS], 1);
	NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutRequests, 1);

	/*
	 *  重複アドレス検出要請以外には、送信元アドレスに
	 *  無指定を指定できない。
	 */
	if (IN6_IS_ADDR_UNSPECIFIED(&ip6h->src) && (flags & IPV6_OUT_FLG_DAD) == 0) {
		error = E_PAR;
		goto drop;
		}

	/* 送信元アドレスにはマルチキャストアドレスを指定できない。*/
	if (IN6_IS_ADDR_MULTICAST(&ip6h->src)) {
		error = E_PAR;
		goto drop;
		}

	/*
	 *  経路選択を行う。
	 */
	if ((gw = in6_rtalloc(ifp, &ip6h->dst)) == NULL) {
		error = E_PAR;
		goto drop;
		}

	if (!IN6_IS_ADDR_MULTICAST(&ip6h->dst)) {
		/* ユニキャストアドレスの処理 */
		}
	else {
		/* マルチキャストアドレスの処理 */
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutMcastPkts, 1);
		if (flags & IPV6_OUT_FLG_HOP_LIMIT)
			ip6h->hlim = IPV6_OUT_GET_HOP_LIMIT(flags);
		else
			ip6h->hlim = IPV6_DEFAULT_MULTICAST_HOPS;
		}

	/* Traffic Class と Flow Label をサポートしないので 0 に設定する。*/
	ip6h->vcf = htonl(IP6_MAKE_VCF(IPV6_VERSION, 0));

#ifdef IP6_CFG_FRAGMENT


#if NUM_IN6_HOSTCACHE_ENTRY > 0

	mtu = in6_hostcache_getmtu(&ip6h->dst);

	if (0 < mtu && mtu <= IPV6_MMTU) {

		/*
		 *  Path MTU が登録されている場合、
		 *  MTU が IPv6 MMTU（1280 オクテット）以下の場合は、
		 *  MTU を IPv6 MMTU に設定し分割して送信する。
		 */
		mtu = IPV6_MMTU;
		alwaysfrag = true;
		}
	else if (mtu == 0)
		mtu  = linkmtu;

#else	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

	mtu  = linkmtu;

#endif	/* of #if NUM_IN6_HOSTCACHE_ENTRY > 0 */

	plen = ntohs(ip6h->plen);
	if (plen + IP6_HDR_SIZE <= mtu && !alwaysfrag) {

		/* 分割する必要がないときの処理 */
		if ((error = nd6_output(ifp, output, gw, NULL, tmout)) != E_OK) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_OUT_ERR_PACKETS], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutDiscards, 1);
			}
		}
	else {

		/* 断片 ID を設定する。*/
		id = ip6_id ++;

		/* MTU が IPv6 の最大パケットサイズを超えないようにする。*/
		if (mtu > IPV6_MAXPACKET)
			mtu = IPV6_MAXPACKET;

		/*
		 *  MTU から、分割不能部分（TINET-1.4 では、分割ヘッダ以外の
		 *  拡張ヘッダの出力は未実装のため 0 オクテット）
		 *  と断片ヘッダのサイズを引いたとき、8 オクテット以上なければ
		 *  分割して送信できないためエラーにする。
		 */
		if (((mtu - sizeof(T_IP6_FRAG_HDR)) & ~7) < 8) {
			error = E_PAR;
			goto drop;
			}

		/*
		 *  IPv6 ヘッダの next フィールドに設定する値は、断片ヘッダ。
		 *  断片ヘッダの  next フィールドに設定する値は、
		 *  元のデータグラムの next フィールドの値。
		 */
		nextproto  = ip6h->next;
		ip6h->next = IPPROTO_FRAGMENT;

		/* 分割して送信する。*/
		NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_OUT], 1);
		for (foff = 0; foff < plen; foff += flen) {

			/* 断片の長さを計算し、8 オクテット境界に調整する。*/
			if (foff + (mtu - (IP6_HDR_SIZE + sizeof(T_IP6_FRAG_HDR))) < plen)
				flen = (mtu - (IP6_HDR_SIZE + sizeof(T_IP6_FRAG_HDR))) >> 3 << 3;
			else
				flen = plen - foff;

			/* ネットワークバッファを獲得する。*/
			if (tget_net_buf(&frag, flen + IF_IP6_HDR_SIZE + sizeof(T_IP6_FRAG_HDR), TMO_IP6_FRAG_GET_NET_BUF) == E_OK) {

				/* IPv6 ヘッダをコピーする。*/
				fip6h = GET_IP6_HDR(frag);
				*fip6h = *ip6h;

				/* IPv6 ヘッダを埋める。*/
				fip6h->plen	= htons(flen + sizeof(T_IP6_FRAG_HDR));

				/* 断片ヘッダを埋める。*/
				fip6f = (T_IP6_FRAG_HDR *)GET_IP6_NEXT_HDR(frag);
				fip6f->off_flag	= htons((uint16_t)(foff & ~7));
				if (foff + (mtu - (IP6_HDR_SIZE + sizeof(T_IP6_FRAG_HDR))) < plen)
					fip6f->off_flag	|= IP6F_MORE_FRAG;
				fip6f->reserved	= 0;
				fip6f->ident	= htonl(id);
				fip6f->next	= nextproto;

				/* SDU をコピーする。*/
				memcpy((uint8_t*)(fip6f + 1), GET_IP6_NEXT_HDR(output) + foff, flen);
				NET_COUNT_IP6(net_count_ip6[NC_IP6_FRAG_OUT_FRAGS], 1);
				NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutFragCreates, 1);

				/* 送信する。*/
				if ((error = nd6_output(ifp, frag, gw, NULL, tmout)) != E_OK) {
					NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutFragFails, 1);
					goto drop;
					}
				}
			else {
				/* ネットワークバッファが獲得できなければ、送信をあきらめる。*/
				error = E_NOMEM;
				goto drop;
				}
			}

		/* IF でネットワークバッファを開放しないフラグをチェックする。*/
		if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
			syscall(rel_net_buf(output));
			}
		else
			output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;

		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutFragOKs, 1);
		}

#else	/* #ifdef IP6_CFG_FRAGMENT */

	if ((error = nd6_output(ifp, output, gw, NULL, tmout)) != E_OK) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_OUT_ERR_PACKETS], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutDiscards, 1);
		}

#endif	/* #ifdef IP6_CFG_FRAGMENT */

	return error;

drop:
	/* IF でネットワークバッファを開放しないフラグをチェックする。*/
	if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
		syscall(rel_net_buf(output));
		}
	else
		output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;

	NET_COUNT_IP6(net_count_ip6[NC_IP6_OUT_ERR_PACKETS], 1);
	NET_COUNT_MIB(in6_ifstat.ipv6IfStatsOutDiscards, 1);
	return error;
	}

#endif /* of #ifdef _IP6_CFG */
