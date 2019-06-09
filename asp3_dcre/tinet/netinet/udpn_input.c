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

#if (API_PROTO == API_PROTO_IPV6) || (TNUM_UDP4_CEPID > 0)

/*
 *  udp_input_select -- UDP の入力処理の選択
 */

void
UDP_INPUT_SELECT (T_UDP_CEP *cep, T_NET_BUF *input, uint_t off)
{
	T_UDP_HDR	*udph;
	T_UDP_RCV_DAT_PARA para;

	udph = (T_UDP_HDR *)(input->buf + off);
	para.len  = (uint_t)(ntohs(udph->ulen) - UDP_HDR_SIZE);

	if (cep->rcv_tskid != TA_NULL) {	/* 非ノンブロッキングコールでペンディング中 */
		if (psnd_dtq(cep->rcvqid, (intptr_t)input) != E_OK) {
			NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
			NET_COUNT_MIB(udp_stats.udpInErrors, 1);
			syscall(rel_net_buf(input));
			}
		}

#ifdef UDP_CFG_NON_BLOCKING

	else if (cep->rcv_p_dstaddr != NULL) {	/* ノンブロッキングコールでペンディング中 */

		/* p_dstaddr を設定する。*/
		cep->rcv_p_dstaddr->portno = ntohs(udph->sport);
		IN_COPY_TO_HOST(&cep->rcv_p_dstaddr->ipaddr, input);

		/* データをバッファに移す。*/
		memcpy(cep->rcv_data, GET_UDP_SDU(input, off),
		       (size_t)(para.len < cep->rcv_len ? para.len : cep->rcv_len));
		syscall(rel_net_buf(input));

		if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
			(*cep->callback)(GET_UDP_CEPID(cep), TFN_UDP_RCV_DAT, (void*)(uint32_t)para.len);
#else
			(*cep->callback)(GET_UDP_CEPID(cep), TFN_UDP_RCV_DAT, (void*)&para.len);
#endif
		else
			syslog(LOG_WARNING, "[UDP] no call back, CEP: %d.", GET_UDP_CEPID(cep));
		cep->rcv_p_dstaddr = NULL;
		}

#endif/* of #ifdef UDP_CFG_NON_BLOCKING */

	else if (IS_PTR_DEFINED(cep->callback)) {

		/* コールバック関数を呼び出す。*/
		cep->cb_netbuf = input;

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
		(*cep->callback)(GET_UDP_CEPID(cep), TEV_UDP_RCV_DAT, (void*)(uint32_t)para.len);
#else
		para.input = input;
		para.off = off;
#if API_PROTO == API_PROTO_IPV4
		para.rep4.portno = ntohs(udph->sport);
		IN_COPY_TO_HOST(&para.rep4.ipaddr, input);
#else
		para.rep6.portno = ntohs(udph->sport);
		IN_COPY_TO_HOST(&para.rep6.ipaddr, input);
#endif
		(*cep->callback)(GET_UDP_CEPID(cep), TEV_UDP_RCV_DAT, (void*)&para.len);
#endif
		/*
		 *  ネットワークバッファがそのままであれば、コールバック関数内で
		 *  データを読み出さなかったことになるので、捨てる。
		 */
		if (cep->cb_netbuf != NULL) {
			if ((input->flags & NB_FLG_NOREL_IFOUT) == 0)
				syscall(rel_net_buf(cep->cb_netbuf));
			cep->cb_netbuf = NULL;
			}
		}
	else {
		NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
		NET_COUNT_MIB(udp_stats.udpInErrors, 1);
		syscall(rel_net_buf(input));
		}

	}

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) || (TNUM_UDP4_CEPID > 0) */

/*
 *  udp_input -- UDP の入力関数
 */

uint_t
UDP_INPUT (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_UDP_CEP	*cep;
	T_UDP_HDR	*udph;
	uint_t		hlen, ulen;

#if defined(_IP6_CFG) && defined(_IP4_CFG) && API_PROTO == API_PROTO_IPV4
	T_UDP6_CEP	*cep6;
#endif

	hlen = (uint_t)GET_IF_IP_HDR_SIZE(input);

	NET_COUNT_UDP(net_count_udp.in_octets,  input->len - hlen);
	NET_COUNT_UDP(net_count_udp.in_packets, 1);
	NET_COUNT_MIB(udp_stats.udpInDatagrams, 1);

	/* ヘッダ長をチェックする。*/
	if (input->len < hlen + UDP_HDR_SIZE)
		goto buf_rel;

	udph = (T_UDP_HDR *)(input->buf + *offp);

	/* データグラム長をチェックする */
	ulen  = ntohs(udph->ulen);
	if (ulen != input->len - hlen)
		goto buf_rel;

	/* 宛先ポートが 0 のデータグラムは破棄する。RFC768 */
	if (udph->dport == 0)
		goto buf_rel;

#ifdef UDP_CFG_IN_CHECKSUM

	/* チェックサムをチェックする */
	if (udph->sum && IN_CKSUM(input, IPPROTO_UDP, *offp, ulen) != 0)
		goto buf_rel;

#endif	/* of #ifdef UDP_CFG_IN_CHECKSUM */

#if defined(_IP6_CFG) && defined(_IP4_CFG) && API_PROTO == API_PROTO_IPV4

#if TNUM_UDP4_CEPID > 0

	/* UDP 通信端点を得る。*/
	cep = udp4_find_cep(input, *offp);

	if (cep != NULL)
		udp4_input_select(cep, input, *offp);
	else if ((cep6 = udp6_find_cep(input, *offp)) != NULL)
		udp6_input_select(cep6, input, *offp);
	else
		udp4_reply_no_port(input, *offp);

	return IPPROTO_DONE;

#else	/* of #if TNUM_UDP4_CEPID > 0 */

	cep = 0;	/* warning 回避用のダミー */
	if ((cep6 = udp6_find_cep(input, *offp)) != NULL)
		udp6_input_select(cep6, input, *offp);
	else
		udp4_reply_no_port(input, *offp);

	return IPPROTO_DONE;

#endif	/* of #if TNUM_UDP4_CEPID > 0 */

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) && API_PROTO == API_PROTO_IPV4 */

	/* UDP 通信端点を得る。*/
	cep = UDP_FIND_CEP(input, *offp);

	if (cep != NULL)
		UDP_INPUT_SELECT(cep, input, *offp);
	else
		UDP_REPLY_NO_PORT(input, *offp);

	return IPPROTO_DONE;

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) && API_PROTO == API_PROTO_IPV4 */

buf_rel:
	NET_COUNT_UDP(net_count_udp.in_err_packets, 1);
	NET_COUNT_MIB(udp_stats.udpInErrors, 1);
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}
