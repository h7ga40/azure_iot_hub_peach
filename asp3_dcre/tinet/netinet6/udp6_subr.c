/*
 *  TINET (UDP/IP Protocol Stack)
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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must ceproduce the above copyright
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

/*
 *  udp6_is_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    以下の場合もデータグラムを受信する。
 *
 *      ・あて先アドレスがマルチキャスト
 */

static bool_t
udp6_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_NET_BUF *input)
{
	T_IP6_HDR	*ip6h;

	ip6h = GET_IP6_HDR(input);

	if (IN6_IS_ADDR_MULTICAST(&ip6h->dst))
		return true;
	else
		return INN6_IS_DSTADDR_ACCEPT(myaddr, input);
	}

#if defined(_IP4_CFG) && defined(API_CFG_IP4MAPPED_ADDR)

#ifdef DHCP_CFG

/*
 *  udpn_is_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    DHCP_CFG が定義されているときは、以下のデータグラムを受信する。
 *
 *      ・あて先アドレスがブロードキャスト
 *      ・ローカルアドレスが未定義
 */

static bool_t
udpn_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_NET_BUF *input)
{
	T_IFNET *ifp = IF_GET_IFNET();

	if        (GET_IP_VER(input)==IPV6_VERSION)
		return udp6_is_dstaddr_accept(myaddr, input);
	else /*if (GET_IP_VER(input)==IPV4_VERSION)*/ {
		T_IP4_HDR	*ip4h;

		ip4h = GET_IP4_HDR(input);

		if (ifp->in4_ifaddr.addr == IPV4_ADDRANY    ||
		    ntohl(ip4h->dst) == IPV4_ADDR_BROADCAST ||
		    ntohl(ip4h->dst) == ((ifp->in4_ifaddr.addr & ifp->in4_ifaddr.mask) | ~ifp->in4_ifaddr.mask))
			return true;
		else
			return inn_is_dstaddr_accept(myaddr, input);
		}
	}

#else	/* of #ifdef DHCP_CFG */

/*
 *  udpn_is_dstaddr_accept -- UDP のあて先アドレスが正しいかチェックする。
 *
 *    以下の場合もデータグラムを受信する。
 *
 *      ・あて先アドレスがブロードキャスト
 */

static bool_t
udpn_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_NET_BUF *input)
{
	T_IFNET *ifp = IF_GET_IFNET();

	if        (GET_IP_VER(input) == IPV6_VERSION)
		return udp6_is_dstaddr_accept(myaddr, input);
	else /*if (GET_IP_VER(input) == IPV4_VERSION)*/ {
		T_IP4_HDR	*ip4h;

		ip4h = GET_IP4_HDR(input);

		if (ntohl(ip4h->dst) == IPV4_ADDR_BROADCAST ||
		    ntohl(ip4h->dst) == ((ifp->in4_ifaddr.addr & ifp->in4_ifaddr.mask) | ~ifp->in4_ifaddr.mask))
			return true;
		else
			return inn_is_dstaddr_accept(myaddr, input);
		}
	}

#endif	/* of #ifdef DHCP_CFG */

#endif	/* of #if defined(_IP4_CFG) && defined(API_CFG_IP4MAPPED_ADDR) */
