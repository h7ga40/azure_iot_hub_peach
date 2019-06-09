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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1994, 1995
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

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1994, 1995
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

/* 
 *  ネットワーク統計情報の計測
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

#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_count.h>

/*
 *  計測用変数
 */

#ifdef SUPPORT_PPP

#if NET_COUNT_ENABLE & PROTO_FLG_PPP

T_NET_COUNT	net_count_ppp;
T_NET_COUNT_VAL	net_count_ppp_no_bufs;		/* net_buf 割り当て失敗数	*/

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP */

#if NET_COUNT_ENABLE & PROTO_FLG_PPP_LCP

T_NET_COUNT_VAL	net_count_ppp_lcp_in_octets;
T_NET_COUNT_VAL	net_count_ppp_lcp_in_packets;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP_LCP */

#if NET_COUNT_ENABLE & PROTO_FLG_PPP_IPV6CP

T_NET_COUNT_VAL	net_count_ppp_ipv6cp_in_octets;
T_NET_COUNT_VAL	net_count_ppp_ipv6cp_in_packets;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP_IPV6CP */

#if NET_COUNT_ENABLE & PROTO_FLG_PPP_IPCP

T_NET_COUNT_VAL	net_count_ppp_ipcp_in_octets;
T_NET_COUNT_VAL	net_count_ppp_ipcp_in_packets;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP_IPCP */

#if NET_COUNT_ENABLE & PROTO_FLG_PPP_PAP

T_NET_COUNT_VAL	net_count_ppp_upap_in_octets;
T_NET_COUNT_VAL	net_count_ppp_upap_in_packets;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP_PAP */

#if NET_COUNT_ENABLE & PROTO_FLG_PPP_HDLC

T_NET_COUNT	net_count_hdlc;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_PPP_HDLC */

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_LOOP

#if NET_COUNT_ENABLE & PROTO_FLG_LOOP

T_NET_COUNT	net_count_loop;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_LOOP */

#endif	/* of #ifdef SUPPORT_LOOP */

#ifdef SUPPORT_ETHER

#if NET_COUNT_ENABLE & PROTO_FLG_ETHER

T_NET_COUNT	net_count_ether;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ETHER */

#if NET_COUNT_ENABLE & PROTO_FLG_ETHER_NIC

T_NET_COUNT_VAL net_count_ether_nic[NC_ETHER_NIC_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ETHER_NIC */

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(_IP4_CFG)

#if NET_COUNT_ENABLE & PROTO_FLG_ARP

T_NET_COUNT	net_count_arp;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ARP */

#if NET_COUNT_ENABLE & PROTO_FLG_IP4

T_NET_COUNT_VAL net_count_ip4[NC_IP4_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_IP4 */

#if NET_COUNT_ENABLE & PROTO_FLG_ICMP4

T_NET_COUNT	net_count_icmp4;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ICMP4 */

#if NET_COUNT_ENABLE & PROTO_FLG_IGMP

T_NET_COUNT	net_count_igmp;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_IGMP */

#endif	/* of #if defined(_IP4_CFG) */

#if defined(_IP6_CFG)

#if NET_COUNT_ENABLE & PROTO_FLG_IP6

T_NET_COUNT_VAL net_count_ip6[NC_IP6_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_IP6 */

#if NET_COUNT_ENABLE & PROTO_FLG_ICMP6

T_NET_COUNT_VAL	net_count_icmp6[NC_ICMP6_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ICMP6 */

#if NET_COUNT_ENABLE & PROTO_FLG_ND6

T_NET_COUNT_VAL	net_count_nd6[NC_ND6_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_ND6 */

#endif	/* of #if defined(_IP6_CFG) */

#ifdef SUPPORT_TCP

#if NET_COUNT_ENABLE & PROTO_FLG_TCP

T_NET_COUNT_VAL net_count_tcp[NC_TCP_SIZE];

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_TCP */

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

#if NET_COUNT_ENABLE & PROTO_FLG_UDP

T_NET_COUNT	net_count_udp;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_UDP */

#endif	/* of #ifdef SUPPORT_PPP */
