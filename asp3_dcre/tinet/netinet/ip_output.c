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
 *	@(#)output.c	8.3 (Berkeley) 1/21/94
 * $FreeBSD: src/sys/netinet/output.c,v 1.85.2.4 1999/08/29 16:29:49 peter Exp $
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
#include <net/ppp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#if defined(SUPPORT_IPSEC)
#include <netinet6/ipsec.h>
#endif

#if defined(_IP4_CFG)

static uint16_t frag_id = 0;

/*
 *  IP output -- IP の出力関数
 *
 *    注意: バージョンとヘッダ長は上位層で設定する。
 */

ER
ip_output (T_NET_BUF *output, TMO tmout)
{
	T_IP4_HDR	*ip4h;
	ER		error = E_OK;
	T_IN4_ADDR	gw;

#ifdef SUPPORT_IPSEC
	T_SECPOLICY* sp;
	int ipsec_error;
        int policy;
#endif /* SUPPORT_IPSEC */

	NET_COUNT_MIB(ip_stats.ipOutRequests, 1);

#ifdef SUPPORT_IPSEC
        /* XXX: IPsecの処理はパケット断片化の前に行われる */

	/* SPDを取得する */
	sp = ipsec4_getpolicybyaddr(output, IPSEC_DIR_OUTBOUND, &ipsec_error);
	if (sp==NULL) {
                error = ipsec_error;
                return error;
        }

        /* ポリシをチェックする*/
#if 0
        switch (sp->policy) {

        case IPSEC_POLICY_BYPASS:
        case IPSEC_POLICY_NONE:
        case IPSEC_POLICY_TCP:
                goto skip_ipsec;

                break; /* NOTREACHED */

        case  IPSEC_POLICY_IPSEC:
                /* XXX: SH2ではここでillegal Instructionが発生する。コンパイラのバグ? */
                if (sp->req == NULL) {
                        /* 鍵交換デーモンに対してSAの取得を要求する */
                        goto bad;
                }
                break;

        default:
                /* 不正なポリシ */
                goto bad;
                break;
        }
#else /* 0 */
        /* XXX: コンパイラの不具合回避のため switchをやめif文で書き直した */
        policy = sp->spinitb->policy;

        if (policy == IPSEC_POLICY_BYPASS ||
            policy == IPSEC_POLICY_NONE ||
            policy == IPSEC_POLICY_TCP)
                goto skip_ipsec;
        else if (policy == IPSEC_POLICY_IPSEC) {
                if (sp->req == NULL)
                        /* 鍵交換デーモンに対してSAの取得を要求する */
                        goto bad;
        } else
                goto bad;

#endif /* 0 */

		{
			T_IPSEC_OUTPUT_STATE state;

			state.m = output;
			/* XXX: ipsec4_output()内では、ip->{len,off}をネットワークバイトオーダとする */
			error = ipsec4_output (&state, sp, 0 /* flags */ );
			output = state.m;

			if (error) {
				/* net_buf はipsec4_outputによって既に解放されている */
				switch (error) {
				default:
					syslog (LOG_NOTICE, "ip_output:%d error", __LINE__);
				}
			}
		}

        /* ipsec4_outputに影響を受けた値を更新する */
        /* XXX: 必要か? */

skip_ipsec:
#endif /* SUPPORT_IPSEC */

#ifdef IP4_CFG_FRAGMENT

	ip4h = GET_IP4_HDR(output);

	gw = in4_rtalloc(ntohl(ip4h->dst));

	/*
	 *  データグラムサイズがネットワークの MTU を超えていれば、
	 *  分割して送信する。
	 */
	if (ntohs(ip4h->len) > IF_MTU) {
		T_NET_BUF	*frag;
		T_IP4_HDR	*fip4h;
		uint_t		off, hlen, len, flen, align;

		NET_COUNT_IP4(net_count_ip4[NC_IP4_FRAG_OUT], 1);
		NET_COUNT_MIB(ip_stats.ipFragCreates, 1);

		len  = ntohs(ip4h->len);
		off  = hlen = GET_IP4_HDR_SIZE(output);
		while (off < len) {
			if (off + (IF_MTU - IP4_HDR_SIZE) < len)
				flen = IF_MTU - IP4_HDR_SIZE;
			else
				flen = len - off;

			/* データ長は 4 オクテット境界に調整する。	*/
			align = (flen + 3) >> 2 << 2;

			/* ネットワークバッファを獲得する。*/
			if (tget_net_buf(&frag, align + IF_IP4_HDR_SIZE, TMO_IP4_FRAG_GET_NET_BUF) == E_OK) {

				/* フラグメントをコピーする。*/
				memcpy(GET_IP4_SDU(frag), (uint8_t *)ip4h + off, flen);

				/* ヘッダを埋める。*/
				fip4h		= GET_IP4_HDR(frag);
				*fip4h		= *ip4h;
				fip4h->flg_off	= htons(IP4_MAKE_FLGOFF(off + flen == len ? 0 : IP4_MF,
						                       (off - hlen) >> 3));
				fip4h->len	= htons(flen + IP4_HDR_SIZE);
				fip4h->id	= htons(frag_id);
				fip4h->sum	= 0;
				fip4h->sum	= in_cksum(fip4h, GET_IP4_HDR_SIZE(frag));

				NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_OCTETS], ntohs(fip4h->len));
				NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_PACKETS], 1);
				NET_COUNT_IP4(net_count_ip4[NC_IP4_FRAG_OUT_FRAGS], 1);

				IF_SET_PROTO(frag, IF_PROTO_IP);
				if ((error = IF_OUTPUT(frag, &gw, NULL, tmout)) != E_OK) {
					syscall(rel_net_buf(output));
					NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_ERR_PACKETS], 1);
					NET_COUNT_MIB(ip_stats.ipFragFails, 1);
					return error;
					}
				}
			else {
				/* 獲得できなければ、送信をあきらめる。*/
				syscall(rel_net_buf(output));
				NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_ERR_PACKETS], 1);
				NET_COUNT_MIB(ip_stats.ipFragFails, 1);
				return E_NOMEM;
				}

			off += IF_MTU - IP4_HDR_SIZE;
			}
		syscall(rel_net_buf(output));
		frag_id ++;
		NET_COUNT_MIB(ip_stats.ipFragOKs, 1);
		}
	else {

		/* ヘッダを埋める。*/
		ip4h->id  = htons(frag_id);
		frag_id ++;
		ip4h->sum = 0;
		ip4h->sum = in_cksum(ip4h, GET_IP4_HDR_SIZE(output));

		NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_OCTETS], ntohs(ip4h->len));
		NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_PACKETS], 1);

		IF_SET_PROTO(output, IF_PROTO_IP);
		if ((error = IF_OUTPUT(output, &gw, NULL, tmout)) != E_OK)
			NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_ERR_PACKETS], 1);
		}

#else	/* of #ifdef IP4_CFG_FRAGMENT */

	ip4h = GET_IP4_HDR(output);

	/* データグラムサイズがネットワークの MTU を超えていればエラー */
	if (ntohs(ip4h->len) > IF_MTU)
		return E_PAR;

	/* ヘッダを埋める。*/
	ip4h->id  = htons(frag_id);
	frag_id ++;
	ip4h->sum = 0;
	ip4h->sum = in_cksum(ip4h, (uint_t)GET_IP4_HDR_SIZE(output));

	NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_OCTETS], ntohs(ip4h->len));
	NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_PACKETS], 1);

#ifdef ETHER_CFG_MULTICAST
	/* マルチキャストアドレスの場合はIPアドレスはそのまま */
	gw = ntohl(ip4h->dst);
	if(!IN4_IS_ADDR_MULTICAST(gw))
		gw = in4_rtalloc(gw);
#else
	gw = in4_rtalloc(ntohl(ip4h->dst));
#endif
	IF_SET_PROTO(output, IF_PROTO_IP);
	if ((error = IF_OUTPUT(output, &gw, NULL, tmout)) != E_OK)
		NET_COUNT_IP4(net_count_ip4[NC_IP4_OUT_ERR_PACKETS], 1);
		NET_COUNT_MIB(ip_stats.ipOutDiscards, 1);

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

#ifdef SUPPORT_IPSEC
	bad:
#endif /* SUPPORT_IPSEC */

	return error;
	}

#endif	/* of #if defined(_IP4_CFG) */
