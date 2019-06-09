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
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/in_itron.h>

#include <net/if_var.h>

/*
 *  in_cksum_sum -- チェックサムの合計計算関数
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 *
 *        戻り値はホストバイトオーダ
 */

uint32_t
in_cksum_sum (void *data, uint_t len /*オクテット単位*/)
{
	uint32_t sum = 0;

	for ( ; len > 0; len -= 2) {
		sum += *((uint16_t*)data);
		data = (uint8_t*)data + 2;
		}

#if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN
	return sum;
#elif _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN
	return ((sum >> 8) & 0xffff) + ((sum & 0xff) << 8) + ((sum >> 24) & 0xff);
#endif	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

	}

/*
 *  in_cksum_carry -- チェックサムの桁上げ計算関数
 *
 *  注意: data は 4 オクテット単位でパディングすること。
 *        data が 2 オクテット単位にアラインされていないと
 *        例外が発生する可能性がある。
 *        len  は 4 オクテット単位にアラインされていること。
 */

uint32_t
in_cksum_carry (uint32_t sum)
{
	uint32_t	carry;

	while (sum >= 0x00010000) {
		carry = sum >> 16;
		sum   = (sum & 0xffff) + carry;
		}

	return sum;
	}

/*
 *  in_strtfn -- API 機能コードの文字表現を返す。
 */

typedef struct t_strtfn {
	FN		fncd;	/* API 機能コード	*/
	const char	*str;	/* 文字表現		*/
	} T_STRTFN;

const T_STRTFN strtfn[] = {
	{ TEV_TCP_RCV_OOB,	"TEV_TCP_RCV_OOB" },
	{ TFN_TCP_DEL_REP,	"TFN_TCP_DEL_REP" },
	{ TFN_TCP_CRE_CEP,	"TFN_TCP_CRE_CEP" },
	{ TFN_TCP_DEL_CEP,	"TFN_TCP_DEL_CEP" },
	{ TFN_TCP_ACP_CEP,	"TFN_TCP_ACP_CEP" },
	{ TFN_TCP_CON_CEP,	"TFN_TCP_CON_CEP" },
	{ TFN_TCP_SHT_CEP,	"TFN_TCP_SHT_CEP" },
	{ TFN_TCP_CLS_CEP,	"TFN_TCP_CLS_CEP" },
	{ TFN_TCP_SND_DAT,	"TFN_TCP_SND_DAT" },
	{ TFN_TCP_RCV_DAT,	"TFN_TCP_RCV_DAT" },
	{ TFN_TCP_GET_BUF,	"TFN_TCP_GET_BUF" },
	{ TFN_TCP_SND_BUF,	"TFN_TCP_SND_BUF" },
	{ TFN_TCP_RCV_BUF,	"TFN_TCP_RCV_BUF" },
	{ TFN_TCP_REL_BUF,	"TFN_TCP_REL_BUF" },
	{ TFN_TCP_SND_OOB,	"TFN_TCP_SND_OOB" },
	{ TFN_TCP_RCV_OOB,	"TFN_TCP_RCV_OOB" },
	{ TFN_TCP_CAN_CEP,	"TFN_TCP_CAN_CEP" },
	{ TFN_TCP_SET_OPT,	"TFN_TCP_SET_OPT" },
	{ TFN_TCP_GET_OPT,	"TFN_TCP_GET_OPT" },
	{ TFN_TCP_ALL,		"TFN_TCP_ALL"     },

	{ TEV_UDP_RCV_DAT,	"TEV_UDP_RCV_DAT" },
	{ TFN_UDP_DEL_CEP,	"TFN_UDP_DEL_CEP" },
	{ TFN_UDP_SND_DAT,	"TFN_UDP_SND_DAT" },
	{ TFN_UDP_RCV_DAT,	"TFN_UDP_RCV_DAT" },
	{ TFN_UDP_CAN_CEP,	"TFN_UDP_CAN_CEP" },
	{ TFN_UDP_SET_OPT,	"TFN_UDP_SET_OPT" },
	{ TFN_UDP_GET_OPT,	"TFN_UDP_GET_OPT" },
	};

const char *
in_strtfn (FN fncd)
{
	int_t	ix;

	for (ix = sizeof(strtfn) / sizeof(T_STRTFN); ix -- > 0; )
		if (strtfn[ix].fncd == fncd)
			return strtfn[ix].str;

	return "unknown TFN";
	}

#if defined(_IP4_CFG)

/*
 *  ipv42str -- IPv4 アドレスを文字列に変換する。
 */

char *
ipv42str (char *buf, const T_IN4_ADDR *ipaddr)
{
	static char	addr_sbuf[NUM_IPV4ADDR_STR_BUFF][sizeof("123.123.123.123")];
	static int_t	bix = NUM_IPV4ADDR_STR_BUFF;

	char *start;

	if (buf == NULL) {
		syscall(wai_sem(SEM_IP2STR_BUFF_LOCK));
		buf = addr_sbuf[-- bix];
		if (bix <= 0)
			bix = NUM_IPV4ADDR_STR_BUFF;
		syscall(sig_sem(SEM_IP2STR_BUFF_LOCK));
		}

	start = buf;
	buf += convert_hexdigit(buf, (uint_t)((*ipaddr >> 24) & 0xff), 10, 0, ' ');
	*(buf ++) = '.';
	buf += convert_hexdigit(buf, (uint_t)((*ipaddr >> 16) & 0xff), 10, 0, ' ');
	*(buf ++) = '.';
	buf += convert_hexdigit(buf, (uint_t)((*ipaddr >>  8) & 0xff), 10, 0, ' ');
	*(buf ++) = '.';
	buf += convert_hexdigit(buf, (uint_t)((*ipaddr      ) & 0xff), 10, 0, ' ');
	*buf = '\0';
	return start;
	}

#endif	/* of #if defined(_IP4_CFG) */

/*
 *  in6_is_addr_ipv4mapped -- IPv4 射影アドレスである事を検査する。
 */

bool_t
in6_is_addr_ipv4mapped (const T_IN6_ADDR *addr)
{
	return IN6_IS_ADDR_V4MAPPED(addr);
	}

#if defined(_IP6_CFG) && defined(_IP4_CFG)

/*
 *  ip_exchg_addr -- IP アドレスを交換する。
 */

static void
ip6_exchg_addr (T_NET_BUF *nbuf)
{
	T_IP6_HDR	*iph;
	T_IN6_ADDR	ipaddr;

	iph  = GET_IP6_HDR(nbuf);

	/* IPv6 アドレスを交換する。*/
	ipaddr = iph->src;
	iph->src = iph->dst;
	iph->dst = ipaddr;
	}

static void
ip4_exchg_addr (T_NET_BUF *nbuf)
{
	T_IP4_HDR	*iph;
	T_IN4_ADDR	ipaddr;

	iph  = GET_IP4_HDR(nbuf);

	/* IPv4 アドレスを交換する。*/
	ipaddr = iph->src;
	iph->src = iph->dst;
	iph->dst = ipaddr;
	}

void
ip_exchg_addr (T_NET_BUF *nbuf)
{
	if (GET_IP_VER(nbuf) == 6)
		ip6_exchg_addr(nbuf);
	else
		ip4_exchg_addr(nbuf);
	}

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

/*
 *  ip_exchg_addr -- IP アドレスを交換する。
 */

void
ip_exchg_addr (T_NET_BUF *nbuf)
{
	T_IP_HDR	*iph;
	T_IN_ADDR	ipaddr;

	iph  = GET_IP_HDR(nbuf);

	/* IP アドレスを交換する。*/
	ipaddr = iph->src;
	iph->src = iph->dst;
	iph->dst = ipaddr;
	}

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

#if defined(_IP6_CFG) && defined(_IP4_CFG)

/*
 *  inn_is_dstaddr_accept -- 宛先アドレスとして正しいかチェックする。
 */

bool_t
inn_is_dstaddr_accept (T_IN6_ADDR *myaddr, T_NET_BUF *nbuf)
{
	if        (GET_IP_VER(nbuf)==IPV6_VERSION)
		return INN6_IS_DSTADDR_ACCEPT(myaddr, nbuf);
	else /*if (GET_IP_VER(nbuf)==IPV4_VERSION)*/ {
		T_IP4_HDR	*ip4h;

		ip4h  = GET_IP4_HDR(nbuf);
		if (IN6_IS_ADDR_UNSPECIFIED(myaddr))
			return ntohl(ip4h->dst) == IF_GET_IFNET()->in4_ifaddr.addr;
		else {
			T_IN6_ADDR	dstaddr;

			return IN6_ARE_ADDR_EQUAL(myaddr, in6_make_ipv4mapped(&dstaddr, ntohl(ip4h->dst)));
			}
		}
	}

/*
 *  inn_are_net_srcaddr_equal -- アドレスが同一かチェックする。
 */

bool_t
inn_are_net_srcaddr_equal (T_IN6_ADDR *ipaddr, T_NET_BUF *nbuf)
{
	if        (GET_IP_VER(nbuf)==IPV6_VERSION)
		return IN6_ARE_ADDR_EQUAL(ipaddr, &GET_IP6_HDR(nbuf)->src);
	else /*if (GET_IP_VER(nbuf)==IPV4_VERSION)*/ {


		T_IN6_ADDR	srcaddr;

		return IN6_ARE_ADDR_EQUAL(ipaddr, in6_make_ipv4mapped(&srcaddr, ntohl(GET_IP4_HDR(nbuf)->src)));
		}
	}

/*
 *  inn_copy_to_host -- IP ヘッダからホスト表現変換して、IP アドレスをコピーする。
 */

void
inn_copy_to_host (T_IN6_ADDR *dst, T_NET_BUF *nbuf)
{
	if        (GET_IP_VER(nbuf)==IPV6_VERSION)
		memcpy(dst, &GET_IP6_HDR(nbuf)->src, sizeof(T_IN6_ADDR));
	else /*if (GET_IP_VER(nbuf)==IPV4_VERSION)*/
		in6_make_ipv4mapped(dst, ntohl(GET_IP4_HDR(nbuf)->src));

	}

/*
 *  inn_get_datagram -- IPv6/IPv4 データグラムを獲得し、ヘッダを設定する。
 */

ER
inn_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
                  T_IN6_ADDR *dstaddr, T_IN6_ADDR *srcaddr,
                  uint8_t next, uint8_t hlim, ATR nbatr, TMO tmout)
{
	if (IN6_IS_ADDR_V4MAPPED(dstaddr)) {
		T_IN4_ADDR ip4dstaddr, ip4srcaddr;

		ip4dstaddr = ntohl(dstaddr->s6_addr32[3]);
		if (IN6_IS_ADDR_UNSPECIFIED(srcaddr))
			ip4srcaddr = IF_GET_IFNET()->in4_ifaddr.addr;
		else
			ip4srcaddr = ntohl(srcaddr->s6_addr32[3]);

		return in4_get_datagram(nbuf, len, maxlen, &ip4dstaddr, &ip4srcaddr, next, hlim, nbatr, tmout);
		}
	else
		return in6_get_datagram(nbuf, len, maxlen, dstaddr, srcaddr, next, hlim, nbatr, tmout);
	}

/*
 * inn_addrwithifp -- 宛先アドレスにふさわしい送信元アドレスを、
 *                   ネットワークインタフェースから探索する。
 */

T_IN6_ADDR *
inn_addrwithifp (T_IFNET *ifp, T_IN6_ADDR *src, T_IN6_ADDR *dst)
{
	T_IN6_IFADDR *ifaddr;

	if (IN6_IS_ADDR_V4MAPPED(dst))
		return in6_make_ipv4mapped (src, ifp->in4_ifaddr.addr);
	else if ((ifaddr = in6_ifawithifp(ifp, dst)) == NULL)
		return NULL;
	else {
		*src = ifaddr->addr;
		return src;
		}
	}

/*
 *  inn_is_addr_multicast -- アドレスがマルチキャストアドレスかチェックする。
 */

bool_t
inn_is_addr_multicast (T_IN6_ADDR *addr)
{

	if (IN6_IS_ADDR_V4MAPPED(addr))
		return IN4_IS_ADDR_MULTICAST(ntohl(addr->s6_addr32[3]));
	else
		return IN6_IS_ADDR_MULTICAST(addr);
	}

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

/*
 *  バイトオーダ関数の定義
 *
 *    tinet/net/net.h でもバイトオーダの定義を行っているが、
 *    tinet/net/net.h をインクルードしない
 *    アプリケーションプログラム用に
 *    ターゲット依存しないバイトオーダ関数を定義する。
 */

#if defined(_NET_CFG_BYTE_ORDER)

#undef ntohs
#undef htons
#undef ntohl
#undef htonl

#if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN

uint16_t
ntohs (uint16_t net)
{
	return net;
	}

uint16_t
htons (uint16_t host)
{
	return host;
	}

uint32_t
ntohl (uint32_t net)
{
	return net;
	}

uint32_t
htonl (uint32_t host)
{
	return host;
	}

#elif _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN	/* of #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

uint16_t
ntohs (uint16_t net)
{
	return NET_REV_ENDIAN_HWORD(net);
	}

uint16_t
htons (uint16_t host)
{
	return NET_REV_ENDIAN_HWORD(host);
	}

uint32_t
ntohl (uint32_t net)
{
	return NET_REV_ENDIAN_WORD(net);
	}

uint32_t
htonl (uint32_t host)
{
	return NET_REV_ENDIAN_WORD(host);
	}

#else	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

#error "_NET_CFG_BYTE_ORDER expected."

#endif	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

#endif	/* of #if defined(_NET_CFG_BYTE_ORDER) */
