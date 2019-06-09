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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
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
 *  udp_reply_no_port -- 送信元にポートが定義されていないことを通知する。
 */

void
UDP_REPLY_NO_PORT (T_NET_BUF *input, uint_t off)
{
	T_UDP_HDR	*udph;

	udph = (T_UDP_HDR *)(input->buf + off);

#if defined(_IP4_CFG)

	if (GET_IP_VER(input)==IPV6_VERSION) {

		/* マルチキャストアドレスに届いたデータグラムは ICMP エラーを通知しない。*/
		if (!IN6_IS_ADDR_MULTICAST(&GET_IP6_HDR(input)->dst)) {
			syslog(LOG_INFO, "[UDP] unexp port: %d.", ntohs(udph->dport));

			/* icmp6_error で、ネットワークバッファ input を返却する。*/
			icmp6_error(input, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOPORT, 0);
			}
		else
			syscall(rel_net_buf(input));
		}
	else /*if (GET_IP_VER(input)==IPV4_VERSION)*/ {

		/* ローカル IP アドレスに届いたデータグラムのみ ICMP エラーを通知する。*/
		if (ntohl(GET_IP4_HDR(input)->dst) == IF_GET_IFNET()->in4_ifaddr.addr) {
			syslog(LOG_INFO, "[UDP] unexp port: %d.", ntohs(udph->dport));

			icmp_error(ICMP4_UNREACH_PORT, input);
			/* icmp_error では、ネットワークバッファ input を返却しないので下へ抜ける。*/
			}
		syscall(rel_net_buf(input));
		}

#else	/* of #if defined(_IP4_CFG) */

	NET_COUNT_MIB(udp_stats.udpNoPorts, 1);

	/* マルチキャストアドレスに届いたデータグラムは ICMP エラーを通知しない。*/
	if (!IN6_IS_ADDR_MULTICAST(&GET_IP6_HDR(input)->dst)) {
		syslog(LOG_INFO, "[UDP] unexp port: %d.", ntohs(udph->dport));

		/* icmp6_error で、ネットワークバッファ input を返却する。*/
		icmp6_error(input, ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_NOPORT, 0);
		}
	else
		syscall(rel_net_buf(input));

#endif	/* of #if defined(_IP4_CFG) */

	NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
	NET_COUNT_MIB(udp_stats.udpInErrors, 1);
	NET_COUNT_MIB(udp_stats.udpNoPorts, 1);
	}
