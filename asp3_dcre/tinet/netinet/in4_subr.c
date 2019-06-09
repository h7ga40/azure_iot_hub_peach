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
//#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include <net/if_var.h>

#ifdef SUPPORT_IGMP
#include <netinet/udp_var.h>
#include <netinet/ip_igmp.h>
#endif /* of #ifdef SUPPORT_IGMP */

#if defined(_IP4_CFG)

#if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0

/*
 *  in_rtinit -- ルーティング表を初期化する。
 */

void
in_rtinit (void)
{
	int_t ix;

	for (ix = 0; ix < NUM_IN4_STATIC_ROUTE_ENTRY; ix ++)
		routing4_tbl[ix].flags = IN_RTF_DEFINED;

	for ( ; ix < NUM_IN4_ROUTE_ENTRY; ix ++)
		routing4_tbl[ix].flags = 0;
	}

/*
 *  in_rtnewentry -- 新しいエントリを獲得する。
 */

T_IN4_RTENTRY *
in_rtnewentry (uint8_t flags, uint32_t tmo)
{
	SYSTIM		now;
	T_IN4_RTENTRY	*rt, *frt = NULL;
	int_t		ix;

	/* 空きエントリを探す。*/
	for (ix = NUM_IN4_STATIC_ROUTE_ENTRY; ix < NUM_IN4_ROUTE_ENTRY; ix ++) {
		rt = &routing4_tbl[ix];
		if ((routing4_tbl[ix].flags & IN_RTF_DEFINED) == 0) {
			frt = rt;
			break;
			}
		}

	/* expire の単位は [s]。*/
	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

	if (frt == NULL) {
		/* 空きがなければ、有効時間がもっとも短いエントリを空きにする。*/
		T_IN4_RTENTRY	*srt = NULL;
		int_t		diff, sdiff = INT_MAX;

		syscall(wai_sem(SEM_IN4_ROUTING_TBL));
		for (ix = NUM_IN4_STATIC_ROUTE_ENTRY; ix < NUM_IN4_ROUTE_ENTRY; ix ++) {
			rt = &routing4_tbl[ix];
			diff = (int_t)(rt->expire - now);
			if (diff <= 0) {	/* rt->expire <= now */
				/* 既に、有効時間が過ぎている。*/
				frt = rt;
				break;
				}
			else if (diff < sdiff) {
				srt = rt;
				sdiff = diff;
				}
			}
		if (frt == NULL)
			frt = srt;
		frt->flags = 0;
		syscall(sig_sem(SEM_IN4_ROUTING_TBL));
		}

	frt->flags  = (uint8_t)(flags | IN_RTF_DEFINED);
	frt->expire = now + tmo / SYSTIM_HZ;
	return frt;
	}

/*
 *  in_rttimer -- ルーティング表の管理タイマー
 */

void
in_rttimer (void)
{
	SYSTIM	now;
	int_t	ix;

	/* expire の単位は [s]。*/
	syscall(get_tim(&now));
	now /= SYSTIM_HZ;

	syscall(wai_sem(SEM_IN4_ROUTING_TBL));
	for (ix = NUM_IN4_STATIC_ROUTE_ENTRY; ix < NUM_IN4_ROUTE_ENTRY; ix ++)
		if ((routing4_tbl[ix].flags & IN_RTF_DEFINED) &&
		    (int_t)(routing4_tbl[ix].expire - now) <= 0)
			routing4_tbl[ix].flags = 0;
	syscall(sig_sem(SEM_IN4_ROUTING_TBL));
	}

#endif	/* of #if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0 */

/*
 *  in4_get_maxnum_ifaddr -- インタフェースに設定可能な最大アドレス数を返す。
 */

uint_t
in4_get_maxnum_ifaddr (void)
{
	return 1;
	}

/*
 *  in4_get_ifaddr -- インタフェースに設定されているアドレスを返す。
 */

const T_IN4_ADDR *
in4_get_ifaddr (int_t index)
{
	T_IFNET		*ifp = IF_GET_IFNET();

	return &ifp->in4_ifaddr.addr;
	}

/*
 *  in4_set_header -- IPv4 ヘッダを設定する。
 */

ER
in4_set_header (T_NET_BUF *nbuf, uint_t len,
                T_IN4_ADDR *dstaddr, T_IN4_ADDR *srcaddr, uint8_t proto, uint8_t ttl)
{
	T_IP4_HDR	*ip4h = GET_IP4_HDR(nbuf);
	T_IFNET		*ifp = IF_GET_IFNET();

	/* IP ヘッダを設定する。*/
	ip4h->vhl	= IP4_MAKE_VHL(IPV4_VERSION, IP4_HDR_SIZE >> 2);
	ip4h->len	= htons(IP4_HDR_SIZE + len);
	ip4h->proto	= proto;
	ip4h->ttl	= ttl;
	ip4h->type	= 0;
	ip4h->id	= ip4h->flg_off = ip4h->sum = 0;

	/* IP アドレスを設定する。*/
	ip4h->dst	= htonl(*dstaddr);

	if (srcaddr == NULL || *srcaddr == IPV4_ADDRANY)
		ip4h->src = htonl(ifp->in4_ifaddr.addr);
	else
		ip4h->src = htonl(*srcaddr);

	return E_OK;
	}

/*
 *  in4_get_datagram -- IPv4 データグラムを獲得し、ヘッダを設定する。
 */

ER
in4_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
                  T_IN4_ADDR *dstaddr, T_IN4_ADDR *srcaddr,
                  uint8_t proto, uint8_t ttl, ATR nbatr, TMO tmout)
{
	ER	error;
	uint_t	align;

	/* データ長を 4 オクテット境界に調整する。*/
	align = (len + 3) >> 2 << 2;

	/* ネットワークバッファを獲得する。*/
	if ((error = tget_net_buf_ex(nbuf, (uint_t)(IF_IP4_HDR_SIZE + align),
	                                   (uint_t)(IF_IP4_HDR_SIZE + maxlen), nbatr, tmout)) != E_OK)
		return error;

	/*
	 *  より大きなサイズのネットワークバッファを獲得する場合のみ長さを調整する。
	 *  より小さなサイズのネットワークバッファの獲得は、送信ウィンドバッファの
	 *  省コピー機能で使用され、実際に送信するまで、データサイズは決定できない。
	 */
	if ((nbatr & NBA_SEARCH_ASCENT) != 0)
		(*nbuf)->len = (uint16_t)(IF_IP4_HDR_SIZE + len);

	/* IP ヘッダを設定する。*/
	if ((error = in4_set_header(*nbuf, len, dstaddr, srcaddr, proto, ttl)) != E_OK) {
		syscall(rel_net_buf(*nbuf));
		*nbuf = NULL;
		return error;
		}

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset((GET_IP4_SDU(*nbuf) + len), 0, (size_t)(align - len));

	return E_OK;
	}

/*
 *  in4_cksum -- IPv4 のトランスポート層ヘッダのチェックサムを計算する。
 *
 *  注意: 戻り値はネットワークバイトオーダ
 */

uint16_t
in4_cksum (T_NET_BUF *nbuf, uint8_t proto, uint_t off, uint_t len)
{
	uint32_t	sum;
	uint_t		align;

	/* 4 オクテット境界のデータ長 */
	align = (len + 3) >> 2 << 2;

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset((uint8_t*)nbuf->buf + off + len, 0, (size_t)(align - len));

	sum = in_cksum_sum(nbuf->buf + off, align)
	    + in_cksum_sum(&GET_IP4_HDR(nbuf)->src, sizeof(T_IN4_ADDR) * 2)
	    + len + proto;
	sum = in_cksum_carry(sum);

	return (uint16_t)(~htons((uint16_t)sum));
	}

/*
 *  in_cksum -- チェックサム計算関数、IPv4、ICMPv4 用
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 *
 *        戻り値はネットワークバイトオーダ
 */

uint16_t
in_cksum (void *data, uint_t len /*オクテット単位*/)
{
	uint16_t	sum;

	sum = (uint16_t)in_cksum_carry(in_cksum_sum(data, len));
	return (uint16_t)(~htons(sum));
	}

/*
 *  in4_is_dstaddr_accept -- 宛先アドレスとして正しいかチェックする。
 *
 *    注意: dstaddr は、
 *          TINET-1.2 からネットワークバイトオーダ、
 *          TINET-1.1 までは、ホストバイトオーダ
 */

bool_t
in4_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_IN4_ADDR *dstaddr)
{
	if (*myaddr == IPV4_ADDRANY)
		return ntohl(*dstaddr) == IF_GET_IFNET()->in4_ifaddr.addr;
	else
		return ntohl(*dstaddr) == *myaddr;
	}

/*
 *  inn4_is_dstaddr_accept -- 宛先アドレスとして正しいかチェックする。
 *
 *    注意: dstaddr は、
 *          TINET-1.2 からネットワークバイトオーダ、
 *          TINET-1.1 までは、ホストバイトオーダ
 */

bool_t
inn4_is_dstaddr_accept (T_IN4_ADDR *myaddr, T_NET_BUF *nbuf)
{
	T_IP4_HDR	*iph;

	iph  = GET_IP4_HDR(nbuf);
	if (*myaddr == IPV4_ADDRANY)
		return ntohl(iph->dst) == IF_GET_IFNET()->in4_ifaddr.addr;
	else
		return ntohl(iph->dst) == *myaddr;
	}

/*
 * in4_addrwithifp -- 宛先アドレスにふさわしい送信元アドレスを、
 *                   ネットワークインタフェースから探索する。
 *                   in6_addrwithifp をシミュレートするだけで、
 *                   エラーを返すことはない。
 */

T_IN4_ADDR *
in4_addrwithifp (T_IFNET *ifp, T_IN4_ADDR *src, T_IN4_ADDR *dst)
{
	*src = ifp->in4_ifaddr.addr;
	return src;
	}

/*
 *  in4_add_ifaddr -- インタフェースに IPv4 アドレスを設定する。
 */

ER
in4_add_ifaddr (T_IN4_ADDR addr, T_IN4_ADDR mask)
{
	T_IFNET	*ifp = IF_GET_IFNET();

	ifp->in4_ifaddr.addr = addr;
	ifp->in4_ifaddr.mask = mask;
	return E_OK;
	}

#if NUM_IN4_ROUTE_ENTRY > 0

/*
 *  in4_add_route -- 経路表にエントリを設定する。
 */

ER
in4_add_route (int_t index, T_IN4_ADDR target, T_IN4_ADDR mask, T_IN4_ADDR gateway)
{

	if (0 <= index && index < NUM_IN4_STATIC_ROUTE_ENTRY) {
		routing4_tbl[index].target  = target;
		routing4_tbl[index].mask    = mask;
		routing4_tbl[index].gateway = gateway;
		return E_OK;
		}
	else
		return E_PAR;
	}

#endif	/* of #if NUM_IN4_ROUTE_ENTRY > 0 */

/*
 *  in4_rtalloc -- ルーティング表を探索する。
 */

T_IN4_ADDR
in4_rtalloc (T_IN4_ADDR dst)
{
	int_t ix;
#ifdef ETHER_CFG_MULTICAST
	if(IN4_IS_ADDR_MULTICAST(dst))
		return dst;
#endif /* of #ifdef ETHER_CFG_MULTICAST */
	for (ix = NUM_IN4_ROUTE_ENTRY; ix --; )
		if ((routing4_tbl[ix].flags & IN_RTF_DEFINED) &&
		    (dst & routing4_tbl[ix].mask) == routing4_tbl[ix].target) {
			if (routing4_tbl[ix].gateway == 0)
				return dst;
			else {
				return routing4_tbl[ix].gateway;
				}
			}
	return dst;
	}

#if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0

/*
 *  in4_rtredirect -- ルーティング表にエントリを登録する。
 *
 *    注意: 引数 tmo の単位は [ms]。
 */

void
in4_rtredirect (T_IN4_ADDR gateway, T_IN4_ADDR target, uint8_t flags, uint32_t tmo)
{
	T_IN4_RTENTRY	*frt;

	frt = in_rtnewentry(flags, tmo);
	frt->gateway    = gateway;
	frt->target     = target;
	frt->mask       = 0xffffffff;
	}

#endif	/* of #if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0 */

/*
 *  in4_timer -- IPv4 共通タイマー
 *
 *    1秒周期で起動される。
 */

static void
in4_timer (void *ignore)
{
#if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0

	in_rttimer();

#endif	/* of #if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0 */

#ifdef IP4_CFG_FRAGMENT

	ip_frag_timer();

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

	timeout(in4_timer, NULL, IN_TIMER_TMO);
	}

/*
 *  in4_init -- IPv4 共通機能を初期化する。
 */

void
in4_init (void)
{
#if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0

	in_rtinit();

#endif	/* of #if NUM_IN4_REDIRECT_ROUTE_ENTRY > 0 */

#ifdef SUPPORT_IGMP
	igmp_init();
#endif /* of #ifdef SUPPORT_IGMP */

	timeout(in4_timer, NULL, IN_TIMER_TMO);
	}

#endif	/* of #if defined(_IP4_CFG) */
