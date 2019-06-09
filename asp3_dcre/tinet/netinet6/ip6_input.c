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

/*	$FreeBSD: src/sys/netinet6/ip6_input.c,v 1.11.2.10 2001/07/24 19:10:18 brooks Exp $	*/
/*	$KAME: ip6_input.c,v 1.194 2001/05/27 13:28:35 itojun Exp $	*/

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
 *	@(#)ip_input.c	8.2 (Berkeley) 1/4/94
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
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp_var.h>

#include <netinet6/nd6.h>
#include <netinet6/ah6.h>
#include <netinet6/esp6.h>

#include <net/if_var.h>

#ifdef _IP6_CFG

/*
 *  関数
 */

static uint_t ip6_no_header_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);

/*
 *  上位プロトコル選択構造体
 */

typedef struct t_proto_switch {
	uint_t (*func)(T_NET_BUF **, uint_t *, uint_t *);	/* 入力関数		*/
	uint_t proto;						/* プロトコル番号	*/
	} T_PROTO_SWITCH;

static const T_PROTO_SWITCH proto_switch[] = {
	{ esp6_input,	IPPROTO_ESP		},	/* IPv6 暗号化ヘッダ	*/
	{ ah6_input,	IPPROTO_AH		},	/* IPv6 認証ヘッダ	*/
	{ frag6_input,	IPPROTO_FRAGMENT	},	/* 断片ヘッダ		*/
	{ route6_input,	IPPROTO_ROUTING		},	/* 経路制御ヘッダ	*/
	{ dest6_input,	IPPROTO_DSTOPTS		},	/* IPv6 終点オプション	*/
	{ ip6_no_header_input,
			IPPROTO_NONE		},	/* 次ヘッダ無し		*/
	{ icmp6_input,	IPPROTO_ICMPV6		},

#if defined(SUPPORT_TCP)

	{ tcp_input,	IPPROTO_TCP		},

#endif	/* of #if defined(SUPPORT_TCP) */

#if defined(SUPPORT_UDP) && TNUM_UDP6_CEPID > 0

	{ udp6_input,	IPPROTO_UDP		},

#endif	/* of #if defined(SUPPORT_UDP) && TNUM_UDP6_CEPID > 0 */

	};

#ifdef SUPPORT_MIB

T_IN6_IFSTAT in6_ifstat;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  get_upper_proto -- 上位プロトコル処理関数を返す。
 */

static uint_t (*
get_upper_proto(uint_t proto))(T_NET_BUF **, uint_t *, uint_t *)
{
	int_t ix;

	for (ix = sizeof(proto_switch) / sizeof(T_PROTO_SWITCH); ix -- > 0; )
		if (proto_switch[ix].proto == proto)
			return proto_switch[ix].func;
	return NULL;
	}

/*
 *  ip6_no_header_input -- 次ヘッダが無しの場合の入力処理
 */

static uint_t
ip6_no_header_input (T_NET_BUF **input, uint_t *offp, uint_t *nextp)
{
	syscall(rel_net_buf(*input));
	return IPPROTO_DONE;
	}

/*
 *  ip6_hopopts_input -- 中継点オプション入力処理
 *
 *    本実装では、パディング以外のオプションを全て未知オプションとして処理する。
 *
 *    戻り値:
 *
 *      0 以上 		正常
 *      IP6_OPT_RET_ERR (-1)	エラー、呼出側でネットワークバッファを開放する。
 *      IP6_OPT_RET_REL (-2)	エラー、ネットワークバッファは開放されている。
 */

static ER_UINT
ip6_hopopts_input (T_NET_BUF *input, uint_t *offp, uint_t *nextp)
{
	T_IP6_HBH_HDR	*hbh;
	ER_UINT		optlen;
	uint_t		len;
	uint8_t		*opt;

	hbh    = (T_IP6_HBH_HDR *)(input->buf + *offp);
	len    = (hbh->len + 1) << 3;
	*nextp = *offp - IF_HDR_SIZE + offsetof(T_IP6_HBH_HDR, next);
	*offp += len;
	opt    = (uint8_t *)hbh + sizeof(T_IP6_HBH_HDR);

	for (len -= sizeof(T_IP6_HBH_HDR); len > 0; len -= optlen) {
		switch (*opt) {
		case IP6OPT_PAD1:
			optlen = 1;
			break;
		case IP6OPT_PADN:
			if (len < IP6OPT_MINLEN)
				return IP6_OPT_RET_ERR;
			optlen = *(opt + 1) + 2;
			break;
		case IP6OPT_JUMBO:
		case IP6OPT_NSAP_ADDR:
		case IP6OPT_TUNNEL_LIMIT:
		case IP6OPT_RTR_ALERT:
		default:
			if (len < IP6OPT_MINLEN)
				return IP6_OPT_RET_ERR;
			optlen = ip6_unknown_opt(input, opt);
			if (optlen < 0)
				return optlen;
			optlen += 2;
			break;
			}

		opt += optlen;
		}

	return *offp;
	}

/*
 *  ip6_get_prev_hdr -- 直前の拡張ヘッダの next フィールドのアドレスを返す。
 *
 *    注意: off には IF_HDR_SIZE も含まれている。
 */

uint8_t *
ip6_get_prev_hdr (T_NET_BUF *nbuf, uint_t off)
{
	T_IP6_HDR	*ip6h;
	T_IP6_EXT_HDR	*ip6eh;
	uint_t		len;
	uint8_t		next;

	ip6h = GET_IP6_HDR(nbuf);
	if (off == IF_HDR_SIZE + sizeof(T_IP6_HDR))
		return &ip6h->next;
	else {
		next  = ip6h->next;
		len   = IF_HDR_SIZE + sizeof(T_IP6_HDR);
		ip6eh = NULL;

		while (len < off) {
			ip6eh = (T_IP6_EXT_HDR *)(nbuf->buf + len);

			switch (next) {
			case IPPROTO_FRAGMENT:
				len += sizeof(T_IP6_FRAG_HDR);
				break;
			case IPPROTO_AH:
				len += (ip6eh->len + 2) << 2;
				break;
			default:
				len += (ip6eh->len + 2) << 3;
				break;
				}
			next  = ip6eh->next;
			}

		if (ip6eh == NULL)
			return NULL;
		else
			return &ip6eh->next;
		}
	}

/*
 *  ip6_init -- IPv6 の初期化関数
 */

void
ip6_init (void)
{
	in6_init();
	in6_ifainit();
	}

/*
 *  ip6_input -- IPv6 の入力関数
 */

void
ip6_input (T_NET_BUF *input)
{
	T_IP6_HDR	*ip6h;
	T_IN6_IFADDR	*ia6;
	T_IFNET		*ifp;
	ER_UINT		noff;
	uint_t		next, nest, plen, offp, nextp;
	uint_t		(*func)(T_NET_BUF **, uint_t *, uint_t *);

	NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_OCTETS], input->len - IF_HDR_SIZE);
	NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_PACKETS], 1);
	NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInReceives, 1);

	/* IP ヘッダの長さをチェックする。*/
	if (input->len < IF_IP6_HDR_SIZE) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_SHORT], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInHdrErrors, 1);
		goto buf_rel;
		}

	ip6h = GET_IP6_HDR(input);

	/* バージョンをチェックする。*/
	if (IP6_VCF_V(ntohl(ip6h->vcf)) != IPV6_VERSION) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_VER], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInHdrErrors, 1);
		goto buf_rel;
		}

	/*
	 *  次のデータグラムは破棄する。
	 *
	 *    ・始点アドレスがマルチキャスト
	 *    ・終点アドレスが未定義
	 *
	 *  ネットワークインタフェースがループバックでないとき、
	 *  次のデータグラムも破棄する。
	 *
	 *    ・始点アドレスがループバック
	 *    ・終点アドレスがループバック
	 *
	 *  悪意のあるユーザが TCP/UDP スタックの混乱や
	 *  セキュリティチェックをバイパスするため、IPv4
	 *  マップアドレスを悪用することを防ぐため
	 *  以下のデータグラムは破棄する。  
	 *
	 *    ・始点アドレスが ::ffff:127.0.0.1
	 *    ・終点アドレスが ::ffff:127.0.0.1
	 */

	if (IN6_IS_ADDR_MULTICAST(&ip6h->src) ||
	    IN6_IS_ADDR_UNSPECIFIED(&ip6h->dst)) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
		goto buf_rel;
		}

	if (IN6_IS_ADDR_V4MAPPED(&ip6h->src) ||
	    IN6_IS_ADDR_V4MAPPED(&ip6h->dst)) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
		goto buf_rel;
		}

#ifdef SUPPORT_LOOP

#else	/* of #ifdef SUPPORT_LOOP */

	if (IN6_IS_ADDR_LOOPBACK(&ip6h->src) ||
	    IN6_IS_ADDR_LOOPBACK(&ip6h->dst)) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
		goto buf_rel;
		}

#endif	/* of #ifdef SUPPORT_LOOP */

	ifp = IF_GET_IFNET();

	if (IN6_IS_ADDR_MULTICAST(&ip6h->dst)) {
		/* 宛先がマルチキャストの場合の処理 */
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInMcastPkts, 1);
		if (!in6_lookup_multi(ifp, &ip6h->dst)) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
			goto buf_rel;
			}
		}
	else {
		if ((ia6 = in6_lookup_ifaddr(ifp, &ip6h->dst)) == NULL) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
			goto buf_rel;
			}

		/*  アドレスが未解決の場合はデータグラムを破棄する。*/
		if (IFA6_IS_NOTREADY(ia6)) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_ADDR], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInAddrErrors, 1);
			goto buf_rel;
			}
		}

	/* オフセットを設定する。*/
	offp  = GET_IP6_NEXT_HDR(input) - input->buf;

	/* ペイロード長を取り出す。*/
	plen = ntohs(ip6h->plen);


	/* 次ヘッダの位置を初期化する。*/
	nextp = offsetof(T_IP6_HDR, next);

	/*
	 *  中継点 (Hop-by-Hop) オプションヘッダのチェック
	 */
	if (ip6h->next == IPPROTO_HOPOPTS) {

		/* 中継点ヘッダの長さをチェックする。*/
		if (plen < sizeof(T_IP6_HBH_HDR)) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_SHORT], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInHdrErrors, 1);
			goto buf_rel;
			}

		if ((noff = ip6_hopopts_input(input, &offp, &nextp)) < 0) {
			if (noff == IP6_OPT_RET_ERR) {
				NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PACKETS], 1);
				NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInHdrErrors, 1);
				goto buf_rel;
				}
			else if (noff == IP6_OPT_RET_REL) {
				NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PACKETS], 1);
				NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInHdrErrors, 1);
				return;
				}
			}

		/*
		 *  ペイロード長が 0 で、中継点オプションがあるときは、
		 *  巨大ペイロード・オプションが含まれなければならない。
		 *  このとき、巨大ペイロード・オプションにある
		 *  ペイロード長をチェックする必要があるが、本実装では、
		 *  巨大ペイロード・オプションを未知のオプションとして
		 *  扱うので、何もしない。
		 */
		next = ((T_IP6_HBH_HDR *)(ip6h + 1))->next;
		}
	else
		next = ip6h->next;

	/* IPv6 データグラム長をチェックする。*/
	if (input->len - IF_IP6_HDR_SIZE < plen) {
		NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_SHORT], 1);
		NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInTruncatedPkts, 1);
		goto buf_rel;
		}
	else if (input->len > IF_IP6_HDR_SIZE + plen)
		input->len = IF_IP6_HDR_SIZE + plen;

	/* 上位プロトコル処理関数を呼び出す。*/
	nest = 0;
	while (next != IPPROTO_DONE) {
		if (++ nest > IP6_HDR_NEST_LIMIT) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PROTO], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInUnknownProtos, 1);
			goto buf_rel;
			}
		else if ((func = get_upper_proto(next)) == NULL) {
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PROTO], 1);
			NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PACKETS], 1);
			NET_COUNT_MIB(in6_ifstat.ipv6IfStatsInUnknownProtos, 1);

			/* icmp6_error でネットワークバッファを開放する。*/
			icmp6_error(input, ICMP6_PARAM_PROB,
			                   ICMP6_PARAMPROB_NEXT_HEADER, nextp);
			return;
			}
		else {
			next = (*func)(&input, &offp, &nextp);
			}
		}
	return;

buf_rel:
	NET_COUNT_IP6(net_count_ip6[NC_IP6_IN_ERR_PACKETS], 1);
	syscall(rel_net_buf(input));
	}

/*
 *  ip6_nexthdr -- 次のヘッダのオフセットを返す。
 *
 *    戻り値 == -1 : 最後のヘッダ、またはエラー。
 *    戻り値 >   0 : さらにヘッダが続く。
 */

ER_UINT
ip6_nexthdr (T_NET_BUF *nbuf, uint_t off, uint_t proto, uint_t *nextp)
{
	T_IP6_FRAG_HDR	*ip6fh;
	T_IP6_EXT_HDR	*ip6eh;

	switch (proto) {

	case IPPROTO_IPV6:				/* IPv6 */
		if (nbuf->len < off + IP6_HDR_SIZE)
			return (ER_UINT)-1;
		*nextp = ((T_IP6_HDR *)(nbuf->buf + off))->next;
		return off + IP6_HDR_SIZE;
		break;

	case IPPROTO_FRAGMENT:			/* 断片ヘッダ */
		if (nbuf->len < off + sizeof(T_IP6_FRAG_HDR))
			return (ER_UINT)-1;

		/* 中間のフラグメントの場合は処理をやめる。*/
		ip6fh = (T_IP6_FRAG_HDR *)(nbuf->buf + off);
		if (ntohs(ip6fh->off_flag) & IP6F_MORE_FRAG)
			return (ER_UINT)-1;

		*nextp = ip6fh->next;
		return off + sizeof(T_IP6_FRAG_HDR);
		break;

	case IPPROTO_AH:		/* IPv6 認証ヘッダ			*/
	case IPPROTO_HOPOPTS:		/* IPv6 中継点 (Hop-by-Hop) オプション	*/
	case IPPROTO_ROUTING:		/* 経路制御ヘッダ			*/
	case IPPROTO_DSTOPTS:		/* IPv6 終点オプション			*/
		if (nbuf->len < off + sizeof(T_IP6_EXT_HDR))
			return (ER_UINT)-1;

		ip6eh = (T_IP6_EXT_HDR *)(nbuf->buf + off);
		*nextp =  ip6eh->next;
		if (proto == IPPROTO_AH)
			return off + ((ip6eh->len + 2) << 2);
		else
			return off + ((ip6eh->len + 1) << 3);
		break;
		}

	return (ER_UINT)-1;
	}

/*
 *  ip6_lasthdr -- 最後のヘッダのオフセットを返す。
 */

ER_UINT
ip6_lasthdr (T_NET_BUF *nbuf, uint_t off, uint_t proto, uint_t *nextp)
{
	ER_UINT	newoff;

	while (1) {
		newoff = ip6_nexthdr(nbuf, off, proto, nextp);
		if (newoff < 0)
			return (ER_UINT)off;
		off   = newoff;
		proto = *nextp;
		}
	return (ER_UINT)0u;
	}

/*
 *  ip6_unknown_opt -- 未知のオプションの処理
 *
 *    戻り値:
 *
 *      0 以上 			正常、オプション長。
 *      IP6_OPT_RET_ERR (-1)	エラー、呼出側でネットワークバッファを開放する。
 *      IP6_OPT_RET_REL (-2)	エラー、ネットワークバッファは開放されている。
 */

ER_UINT
ip6_unknown_opt (T_NET_BUF *input, uint8_t *opt)
{
	switch (IP6OPT_TYPE(*opt)) {
	case IP6OPT_TYPE_SKIP:		/* 無視する。			*/
		return *(opt + 1);
		break;
	case IP6OPT_TYPE_ICMP:		/* 破棄して、マルチキャストでなければ ICMP を返送する。	*/
		if (IN6_IS_ADDR_MULTICAST(&GET_IP6_HDR(input)->dst))
			return IP6_OPT_RET_ERR;
		/* no break; */
	case IP6OPT_TYPE_FORCEICMP:	/* 破棄して、ICMP を返送する。	*/
		icmp6_error(input, ICMP6_PARAM_PROB,
		                   ICMP6_PARAMPROB_OPTION,
		                   (opt - input->buf) - IF_HDR_SIZE);
		return IP6_OPT_RET_REL;
		break;
	/*case IP6OPT_TYPE_DISCARD:	データグラムを破棄する。	*/
		}

	return IP6_OPT_RET_ERR;
	}

/*
 *  ip6_remove_exthdrs -- IPv6 拡張ヘッダを削除し、以降を前に詰める。
 */

ER
ip6_remove_exthdrs (T_NET_BUF *nbuf)
{
	ER_UINT	off;
	uint_t	next;

	/* 開始オフセットを IP ヘッダに設定する。*/
	off = ((uint8_t *)GET_IP6_HDR(nbuf)) - nbuf->buf;

	/* 最終ヘッダを探索する。*/
	off = ip6_lasthdr(nbuf, off, IPPROTO_IPV6, &next);

	if (off <= 0)
		return E_OBJ;
	else {
		uint_t sdu;

		sdu = GET_IP6_SDU(nbuf) - nbuf->buf;
		if (off > sdu) {
			memmove(nbuf->buf + sdu, nbuf->buf + off, off - sdu);
			GET_IP6_HDR(nbuf)->plen -= off - sdu;
			nbuf->len              -= off - sdu;
			}
		}
	return E_OK;
	}

#endif /* of #ifdef _IP6_CFG */
