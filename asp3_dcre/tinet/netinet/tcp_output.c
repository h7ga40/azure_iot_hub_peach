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
 *
 *	@(#)tcp_output.c	8.4 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_output.c,v 1.32.2.2 1999/08/29 16:29:55 peter Exp $
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
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
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>

#ifdef SUPPORT_TCP

/*
 *  関数
 */

static ER send_segment (bool_t *sendalot, T_TCP_CEP *cep, uint_t doff, uint_t win, uint_t len, uint8_t flags);
static void tcp_output (T_TCP_CEP *cep);

/*
 *  変数
 */

/* 出力時のフラグを FSM 状態により選択するための表 */

const static uint8_t tcp_outflags[] = {
	TCP_FLG_RST | TCP_FLG_ACK,	/*  0, クローズ				*/
	0,				/*  1, 受動オープン			*/
	TCP_FLG_SYN,			/*  2, 能動オープン、SYN 送信済み	*/
	TCP_FLG_SYN | TCP_FLG_ACK,	/*  3, SYM を受信し、SYN 送信済み	*/
	TCP_FLG_ACK,			/*  4, コネクション開設完了		*/
	TCP_FLG_ACK,			/*  5, FIN 受信、クローズ待ち		*/
	TCP_FLG_FIN | TCP_FLG_ACK,	/*  6, 終了して、FIN 送信済み		*/
	TCP_FLG_FIN | TCP_FLG_ACK,	/*  7, 終了、FIN 交換済み、ACK 待ち	*/
	TCP_FLG_FIN | TCP_FLG_ACK,	/*  8, FIN 受信、終了、ACK 待ち		*/
	TCP_FLG_ACK,			/*  9, 終了、FIN 伝達確認受信、FIN待ち	*/
	TCP_FLG_ACK,			/* 10, 終了、時間待ち			*/
	};

/*
 *  send_segment -- TCP 出力処理
 */

static ER
send_segment (bool_t *sendalot, T_TCP_CEP *cep, uint_t doff, uint_t win, uint_t len, uint8_t flags)
{
	T_NET_BUF	*output;
	T_TCP_HDR	*tcph;
	uint_t		optlen, hdr_offset;
	ER		error;

#ifdef TCP_CFG_OPT_MSS

	uint8_t		*optp;

	if (flags & TCP_FLG_SYN)
		optlen = TCP_OPT_LEN_MAXSEG;
	else
		optlen = 0;

#else/* of #ifdef TCP_CFG_OPT_MSS */

	optlen = 0;

#endif/* of #ifdef TCP_CFG_OPT_MSS */

	NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_SEGS], 1);
	NET_COUNT_MIB(tcp_stats.tcpOutSegs, 1);

	/*
	 *  セグメント長を、相手の最大受信セグメント長に調整する。
	 *  もし、超えている場合は、超えた分を後で送信する。
	 *  このため、FIN ビットをクリアする。
	 *
	 *  オリジナルでは、t_maxopd を制限長にしているが、
	 *  本実装では、相手の最大受信セグメントにする。
	 */
	if (len + optlen > cep->maxseg) {
		flags &= ~TCP_FLG_FIN;
		len = cep->maxseg - optlen;
		*sendalot = true;
		}

	/*
	 *  送信バッファが空になるときは PUSH フラグを設定する。
	 */
	if (len && doff + len >= cep->swbuf_count)
		flags |= TCP_FLG_PUSH;

#if defined(TCP_CFG_SWBUF_CSAVE_ONLY)
		
	if (len > 0 && ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SEND_READY ||
	                (cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SENT)) {

		/*
		 *  送信ウインドバッファが開放されないようにして、
		 *  ネットワークバッファを出力に移す。
		 */
		cep->swbufq->flags |= NB_FLG_NOREL_IFOUT;
		output = cep->swbufq; 
		}
	else {

		/*
		 *  ACK 完了状態で、この関数が呼び出されることもある。
		 *  この時は、len を 0 にして、処理を継続する。
		 */
		len = 0;
		if ((error = tcpn_get_segment(&output, cep, optlen,
	                                    len, (uint_t)net_buf_max_siz(),
	                                    NBA_SEARCH_ASCENT, TMO_TCP_GET_NET_BUF)) != E_OK) {
			if (cep->timer[TCP_TIM_REXMT] == 0)
				cep->timer[TCP_TIM_REXMT] = cep->rxtcur;
			goto err_ret;
			}
		}

#elif defined(TCP_CFG_SWBUF_CSAVE)	/* of #if defined(TCP_CFG_SWBUF_CSAVE_ONLY) */

	if (IS_PTR_DEFINED(cep->sbuf)) {
		if ((error = tcpn_get_segment(&output, cep, optlen,
		                             len, (uint_t)net_buf_max_siz(),
		                             NBA_SEARCH_ASCENT, TMO_TCP_GET_NET_BUF)) != E_OK) {
			if (cep->timer[TCP_TIM_REXMT] == 0)
				cep->timer[TCP_TIM_REXMT] = cep->rxtcur;
			goto err_ret;
			}
		}
	else if (len > 0 && ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SEND_READY ||
	                     (cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SENT)) {

		/*
		 *  送信ウインドバッファが開放されないようにして、
		 *  ネットワークバッファを出力に移す。
		 */
		cep->swbufq->flags |= NB_FLG_NOREL_IFOUT;
		output = cep->swbufq; 
		}
	else {

		/*
		 *  ACK 完了状態で、この関数が呼び出されることもある。
		 *  この時は、len を 0 にして、処理を継続する。
		 */
		len = 0;
		if ((error = tcpn_get_segment(&output, cep, optlen,
	                                    len, (uint_t)net_buf_max_siz(),
	                                    NBA_SEARCH_ASCENT, TMO_TCP_GET_NET_BUF)) != E_OK) {
			if (cep->timer[TCP_TIM_REXMT] == 0)
				cep->timer[TCP_TIM_REXMT] = cep->rxtcur;
			goto err_ret;
			}
		}

#else	/* of #if defined(TCP_CFG_SWBUF_CSAVE_ONLY) */

	if ((error = tcpn_get_segment(&output, cep, optlen,
	                             len, (uint_t)net_buf_max_siz(),
	                             NBA_SEARCH_ASCENT, TMO_TCP_GET_NET_BUF)) != E_OK) {
		if (cep->timer[TCP_TIM_REXMT] == 0)
			cep->timer[TCP_TIM_REXMT] = cep->rxtcur;
		goto err_ret;
		}

#endif	/* of #if defined(TCP_CFG_SWBUF_CSAVE_ONLY) */

	/*
	 *  TCP オプションの設定を行う。
	 *  本実装では、最大セグメントサイズのみ設定する。
	 */
	hdr_offset = IF_IP_TCP_HDR_OFFSET(output);
	if (flags & TCP_FLG_SYN) {
		cep->snd_nxt = cep->iss;

#ifdef TCP_CFG_OPT_MSS

		optp = GET_TCP_OPT(output, hdr_offset);
		*optp ++ = TCP_OPT_MAXSEG;
		*optp ++ = TCP_OPT_LEN_MAXSEG;
		*(uint16_t*)optp = htons(DEF_TCP_RCV_SEG);

#endif/* of #ifdef TCP_CFG_OPT_MSS */

		}

	/* TCP SDU に送信データをコピーする。*/

	if (len > 0) {
		if (SEQ_LT(cep->snd_nxt, cep->snd_max)) {
			NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_REXMIT_SEGS], 1);
			NET_COUNT_MIB(tcp_stats.tcpRetransSegs, 1);
			}
		TCP_READ_SWBUF(cep, output, len, doff);
		}
	else {
		if (cep->flags & TCP_CEP_FLG_ACK_NOW)
			NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_ACKS], 1);
		if (flags & (TCP_FLG_FIN | TCP_FLG_SYN | TCP_FLG_RST))
			NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_CNTL_SEGS],  1);


#ifdef TCP_CFG_EXTENTIONS

		if (SEQ_LT(cep->snd_up, cep->snd_una))
			NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_URG_SEGS], 1);

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

		}

	/*
	 * snd_max: 送信した最大 SEQ
	 * snd_nxt: 次に送信する SEQ
	 *
	 *  相手から FIN を受信し、まだ FIN を送信していないか、
	 *  送るデータがないときは、FIN を相手に届けるため、
	 *  セグメントを送信するが、SEQ は進めない。
	 */
	if ((flags & TCP_FLG_FIN) && (cep->flags & TCP_CEP_FLG_SENT_FIN) &&
	    cep->snd_nxt == cep->snd_max) {
		cep->snd_nxt --;
		}

	tcph = GET_TCP_HDR(output, hdr_offset);

	/*
	 *  SEQ、ACK、フラグの設定。
	 */
	if (len > 0 || (flags & (TCP_FLG_SYN | TCP_FLG_FIN)) || cep->timer[TCP_TIM_PERSIST] != 0)
		tcph->seq = htonl(cep->snd_nxt);
	else
		tcph->seq = htonl(cep->snd_max);

	/*
	 *  rcv_nxt: 受信を期待している最小の SEQ
	 */
	tcph->ack   = htonl(cep->rcv_nxt);
	tcph->flags = flags;

	/*
	 *  受信ウィンドの計算
	 *
	 *  rbufsz: 受信用バッファサイズ
	 *  maxseg: 相手の最大受信セグメントサイズ	
	 */
	if (win < (cep->rbufsz / 4) && win < cep->maxseg)
		win = 0;

	/*
	 *  rcv_nxt: 受信を期待している最小の SEQ
	 *  rcv_adv: 受信を期待している最大の SEQ
	 */
	if ((int32_t)win < (int32_t)(cep->rcv_adv - cep->rcv_nxt))
		win = (uint_t)(cep->rcv_adv - cep->rcv_nxt);

	tcph->win = htons(win);

#ifdef TCP_CFG_EXTENTIONS

	/*
	 *  緊急ポインタの設定
	 */
	if (SEQ_GT(cep->snd_up, cep->snd_nxt)) {
		if (TCP_CFG_URG_OFFSET)
			tcph->urp    = htons((uint16_t)(cep->snd_up - cep->snd_nxt));
		else
			tcph->urp    = htons((uint16_t)(cep->snd_up - cep->snd_nxt - 1));
		tcph->flags |= TCP_FLG_URG;
		}
	else {
		tcph->urp = 0;
		cep->snd_up  = cep->snd_una;
		}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	/*
	 *  チェックサムを設定する。
	 */
	tcph->sum = 0;
	tcph->sum = IN_CKSUM(output, IPPROTO_TCP, hdr_offset, GET_TCP_HDR_SIZE(output, hdr_offset) + len);

	/* ネットワークバッファ長を調整する。*/
	output->len = (uint16_t)(GET_IF_IP_TCP_HDR_SIZE(output, hdr_offset) + len);

	/*
	 *  タイマの調整
	 */
	if ((cep->flags & TCP_CEP_FLG_FORCE) == 0 || cep->timer[TCP_TIM_PERSIST] == 0) {
		T_TCP_SEQ startseq = cep->snd_nxt;

		/*
		 *  次に送信する SEQ (snd_nxt) を今回送信するデータ数分進める。
		 */
		if (flags & TCP_FLG_SYN)
			cep->snd_nxt ++;
		if (flags & TCP_FLG_FIN) {
			cep->flags |= TCP_CEP_FLG_SENT_FIN;
			cep->snd_nxt ++;
			}

		cep->snd_nxt += len;

		/*
		 *  次に送信する SEQ (snd_nxt) が
		 *  送信した最大 SEQ (snd_max) より進んでいれば、
		 *  送信した最大 SEQ (snd_max) を更新する。
		 */
		if (SEQ_GT(cep->snd_nxt, cep->snd_max)) {
			cep->snd_max = cep->snd_nxt;
			/*
			 *  もし、往復時間計測を行っていなければ、
			 *  この送信に時間を合わせる。
			 */
			if (cep->rtt == 0) {
				cep->rtt   = 1;
				cep->rtseq = startseq;	/* 更新前の cep->snd_nxt */
				}
			}

		/*
		 *  もし設定されていないか、ACK または保留が発生していなければ、
		 *  再送タイマを設定する。設定する初期値は、
		 * 「滑らかな往復時間 + 2 × 往復時間変動」である。
		 *  再送時間のバックオフに使われるシフトカウントも初期化する。
		 */
		if (cep->timer[TCP_TIM_REXMT] == 0 && cep->snd_nxt != cep->snd_una) {
			cep->timer[TCP_TIM_REXMT] = cep->rxtcur;
			if (cep->timer[TCP_TIM_PERSIST] != 0) {
				cep->timer[TCP_TIM_PERSIST] = 0;
				cep->rxtshift = 0;
				}
			}
		}

	/*
	 *  次に送信する SEQ (snd_nxt) + 今回送信するデータ数 (len) が
	 *  送信した最大 SEQ (snd_max) より進んでいれば、
	 *  送信した最大 SEQ (snd_max) を更新する。
	 */
	else if (SEQ_GT(cep->snd_nxt + len, cep->snd_max))
		cep->snd_max = cep->snd_nxt + len;

#ifdef TCP_CFG_SWBUF_CSAVE

	if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SEND_READY)
		cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_SENT;

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

#ifdef TCP_CFG_TRACE

	tcp_output_trace(output, cep);

#endif	/* of #ifdef TCP_CFG_TRACE */

	/* ネットワーク層 (IP) の出力関数を呼び出す。*/
	if ((error = IP_OUTPUT(output, TMO_TCP_OUTPUT)) != E_OK)
		goto err_ret;

	/*
	 *  相手に伝えたウィンドウサイズ (win) が 0 以上で、
	 *  受信を期待している最小の SEQ (rcv_nxt) + win が
	 *  受信を期待している最大の SEQ (rcv_adv) より進んでいれば
	 *  受信を期待している最大の SEQ を更新する。
	 */
	if (win > 0 && SEQ_GT(cep->rcv_nxt + win, cep->rcv_adv)) {
		cep->rcv_adv = cep->rcv_nxt + win;
		}

	/*
	 *  最後に送信した ACK (last_ack_sent) を更新する。
	 */
	cep->last_ack_sent = cep->rcv_nxt;

	/*
	 *  フラグの設定を行う。
	 */
	cep->flags &= ~(TCP_CEP_FLG_ACK_NOW | TCP_CEP_FLG_DEL_ACK);
	if (cep->flags & TCP_CEP_FLG_FORCE_CLEAR)
		cep->flags &= ~(TCP_CEP_FLG_FORCE | TCP_CEP_FLG_FORCE_CLEAR);

	return E_OK;

err_ret:
	/*
	 * 以下に関係しないフラグをクリアーする。
	 * ・送受信ウィンドバッファの省コピー機能
	 * ・動的な通信端点の生成・削除機能
	 */
	cep->flags &= TCP_CEP_FLG_NOT_CLEAR;

	return error;
	}

/*
 *  tcp_output -- TCP 出力処理
 */

void
tcp_output (T_TCP_CEP *cep)
{
	bool_t	sendalot = true, idle;
	ER	error = E_OK;
	int32_t	len;
	uint_t	doff, win;
	uint8_t	flags;

	/*
	 *  snd_una: 未確認の最小送信 SEQ	 または、確認された最大送信 SEQ
	 *  snd_max: 送信した最大 SEQ
	 */
	idle = (cep->snd_max == cep->snd_una);

	/*
	 *  idle:   アイドル時間
	 *  rxtcur: 現在の再送タイムアウト
	 */
	if (idle && cep->idle >= cep->rxtcur)

		/*
		 *  snd_cwnd: 輻輳ウィンドサイズ
		 *  maxseg  : 相手の最大受信セグメントサイズ
		 *
		 *  長時間アイドルだったのでスロースタート制御に設定する。
		 */
		cep->snd_cwnd = cep->maxseg;

	while (error == E_OK && sendalot) {
		sendalot = false;

		/*
		 *  snd_nxt: 次に送信する SEQ、この時点では、前回送信した SEQ
		 *  snd_una: 未確認の最小送信 SEQ、または確認された最大送信 SEQ
		 *
		 *  doff: 送信を開始するオフセット。
		 *                                    swbuf_count (送信バッファにあるオクテット数)
		 *    0                               V
		 *    +-------------------------------------------+
		 *    |                    sbuf                   |
		 *    +-------------------------------------------+
		 *    ^               ^
		 *    |<------------->snd_nxt (前回送信した SEQ)
		 *    |       doff
		 *    snd_una (まだ確認されていない)
		 */
		doff = (uint_t)(cep->snd_nxt - cep->snd_una);

		/*
		 *  snd_wnd:  相手の受信可能ウィンドサイズ
		 *  snd_cwnd: 輻輳ウィンドサイズ
		 *
		 *  win: どちらか小さいウィンドサイズに設定する。
		 */
		win   = cep->snd_wnd < cep->snd_cwnd ? cep->snd_wnd : cep->snd_cwnd;
	
		/* 出力フラグの設定 */
		flags = tcp_outflags[cep->fsm_state];
		if (cep->flags & TCP_CEP_FLG_NEED_FIN)
			flags |= TCP_FLG_FIN;
		if (cep->flags & TCP_CEP_FLG_NEED_SYN)
			flags |= TCP_FLG_SYN;
		if (cep->flags & TCP_CEP_FLG_FORCE) {

			/*
			 *  もし、送信ウインドサイズ (win) が 0 なら 1 オクテット送信する。
			 *  そうでなければ、持続タイムアウトをキャンセルし、
			 *  再送信回数 (rxtshift) を 0 にする。
			 */
			if (win == 0) {

				/*
				 *  doff:        送信するオクテット数。
				 *  swbuf_count: 送信バッファの使用中サイズ
				 *
				 *  送信バッファに残っているオクテットが、これから
				 *  送信しようとしているオクテット数より多ければ
				 *  FIN フラグをクリアする。
				 */
				if (doff < cep->swbuf_count)
					flags &=~TCP_FLG_FIN;
				win = 1;
				}
			else {
				/*
				 *  TCP_TIM_PERSIST: 持続タイマ
				 *  rxtshift:        再送信回数の log(2)
				 */
				cep->timer[TCP_TIM_PERSIST] = 0;
				cep->rxtshift = 0;
				}
		 	}

		/*
		 *  len: 今回送信するオクテット数
		 *        swbuf_count (送信バッファにあるオクテット数)
		 *                                    |
		 *    0                               V
		 *    +-------------------------------------------+
		 *    |                    sbuf       |           |
		 *    +-------------------------------------------+
		 *    ^               ^<------------->
		 *    |               |      len
		 *    |<------------->snd_nxt (前回送信した SEQ)
		 *    |       doff
		 *    snd_una (まだ確認されていない)
		 */
		if (cep->swbuf_count < win)
			len = (int32_t)cep->swbuf_count - doff;
		else
			len = (int32_t)win - doff;

		/*
		 *  すでに送信されていれば、SYN ビットをオフする。
		 *  しかし、以下の条件では送信を控える。
		 *
		 *    ・状態が SYN 送信。
		 *    ・セグメントがデータを含んでいる。
		 */
		if ((flags & TCP_FLG_SYN) && SEQ_GT(cep->snd_nxt, cep->snd_una)) {
			flags &= ~TCP_FLG_SYN;
			doff --;		/* -1 は SYN フラグ分 */
			len ++;			/* +1 は SYN フラグ分 */
			if (len > 0 && cep->fsm_state == TCP_FSM_SYN_SENT)
				break;
			}

		if (flags & TCP_FLG_SYN) {
			len = 0;
			flags &= ~TCP_FLG_FIN;
			}

		if (len < 0) {

			/*
			 *  len が 0 以下なら、0 に設定する。
			 *  もし、送信ウィンドウサイズが 0 なら、
			 *  再送信タイマをキャンセルし、
			 *  前回送信した SEQ (snd_nxt) を
			 *  確認された最大送信 SEQ (snd_una) に戻す。
			 *  そして、持続タイマーが止まっていれば、再設定する。
			 */
			len = 0;
			if (win == 0) {
				cep->timer[TCP_TIM_REXMT] = 0;
				cep->rxtshift = 0;
				cep->snd_nxt  = cep->snd_una;
				if (cep->timer[TCP_TIM_PERSIST] == 0)
					tcp_set_persist_timer(cep);
				}
			}


		/*
		 *  今回送信するオクテット数 (len) は、
		 *  相手の最大受信セグメントサイズ (maxseg) を超えないようにする。
		 */
		if (len > cep->maxseg) {
			len = cep->maxseg;
			sendalot = true;
			}

		/*
                 *        swbuf_count (送信バッファにあるオクテット数)
                 *                                           |
		 *    0                                      V
		 *    +-------------------------------------------+
		 *    |                    sbuf       |           |
		 *    +-------------------------------------------+
		 *    ^               ^<------------->
		 *    |               |      len
		 *    |<------------->snd_nxt (前回送信した SEQ)
		 *    |       doff
		 *    snd_una (まだ確認されていない)
		 *
		 *  今回送信後も、送信バッファにデータが残っていれば
		 *  FIN フラグをクリアする。
		 */
		if (SEQ_LT(cep->snd_nxt + len, cep->snd_una + cep->swbuf_count))
			flags &= ~TCP_FLG_FIN;

		/*
		 *  ここから win は、受信ウィンドウサイズ。
		 *  受信バッファの空き容量
		 */
		win = cep->rbufsz - cep->rwbuf_count;

		/*
		 *  愚かなウィンドウ・シンドロームの回避処理 (送信側)
		 *
		 *  以下の条件で、送信を行う。
		 *
		 *    ・フルサイズ (maxseg) のセグメントを送ることができる。
		 *    ・相手の最大の受信ウィンドウサイズの 1/2 のデータを
		 *      送ることができる。
		 *    ・送信バッファを空にでき、アイドルか非遅延オプションが有効なとき。
		 */
		if (len) {

			/*
			 *  今回送信するオクテット数 (len) が
			 *  相手の最大受信セグメントサイズ (maxseg) に
			 *  一致するときは送信する。
			 */
			if (len == cep->maxseg) {
				error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
				continue;
				}

			/*
			 *  今回の送信で、送信バッファを空にでき、
			 *  アイドルか非 PUSH オプションが有効なとき。
			 */
			if ((idle || (cep->flags & TCP_CEP_FLG_NO_DELAY)) &&
			    (cep->flags & TCP_CEP_FLG_NO_PUSH) == 0 &&
			    len + doff >= cep->swbuf_count) {
				error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
				continue;
				}

			/*
			 *  max_sndwnd: 今までの最大送信ウィンドサイズ
			 *  snd_nxt:    次に送信する SEQ
			 *  snd_max:    送信した最大 SEQ
			 *
			 *  次の条件では送信を行う。
			 *
			 *    ・強制送信フラグがセットされている。
			 *    ・データ長が相手の最大の受信ウィンドウサイズの 1/2 以上で、
			 *      相手の最大の受信ウィンドウサイズが 0 より大きい。
			 *    ・次に送信する SEQ が送信した最大 SEQ より小さい、
			 *      つまり、再送するとき。
			 */
			if ((cep->flags & TCP_CEP_FLG_FORCE) ||
			    (len >= cep->max_sndwnd / 2 && cep->max_sndwnd > 0) ||
			    SEQ_LT(cep->snd_nxt, cep->snd_max)) {
				error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
				continue;
				}
			}


		/*
		 *  愚かなウィンドウ・シンドロームの回避処理 (受信側)
		 *
		 *  ウィンドウサイズがフルサイズの 2 倍のセグメント、あるいは
		 *  受信バッファ容量の 1/2 の、いずれか小さいほうの
		 *  サイズで増加される場合は、ウィンドウサイズの更新を行う。
		 */
		if (win > 0) {
			long adv;

			/*
			 *  win:              受信バッファの空き容量
			 *  MAX_TCP_WIN_SIZE: TCP ヘッダの win フィールドに設定できる最大値
			 *  rcv_adv:          受信を期待している最大の SEQ
			 *  rcv_nxt:          受信を期待している最小の SEQ
			 */
			if (win < MAX_TCP_WIN_SIZE)
				adv = win - (cep->rcv_adv - cep->rcv_nxt);
			else
				adv = MAX_TCP_WIN_SIZE - (cep->rcv_adv - cep->rcv_nxt);

			if (adv     >= (long)(cep->maxseg * 2) ||
			    adv * 2 >= (long) cep->rbufsz) {
				error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
				continue;
				}
			}
	
		/*
		 *  ACK を送信する。
		 */
		if (cep->flags & TCP_CEP_FLG_ACK_NOW) {
			error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
			continue;
			}

		if ( (flags & TCP_FLG_RST) ||
		    ((flags & TCP_FLG_SYN) && (cep->flags & TCP_CEP_FLG_NEED_SYN) == 0)) {
			error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
			continue;
			}

#ifdef TCP_CFG_EXTENTIONS

		if (SEQ_GT(cep->snd_up, cep->snd_una)) {
			error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
			continue;
			}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

		/*
		 *  snd_nxt: 次に送信する SEQ
		 *  snd_una: 未確認の最小送信 SEQ、または確認された最大送信 SEQ
		 *
		 *  相手から FIN を受信し、まだ FIN を送信していないか、
		 *  送るデータがないときは、FIN を相手に届けるため、
		 *  セグメントを送信する。
		 */
		if ((flags & TCP_FLG_FIN) &&
		    ((cep->flags & TCP_CEP_FLG_SENT_FIN) == 0 || cep->snd_nxt == cep->snd_una)) {
			error = send_segment(&sendalot, cep, doff, win, (uint_t)len, flags);
			continue;
			}

		/*
		 *  送信すべきデータがあり、再送タイマと持続タイマが切れているときは
		 *  持続タイマを設定する。
		 */
		if (cep->swbuf_count && cep->timer[TCP_TIM_REXMT  ] == 0 &&
		                        cep->timer[TCP_TIM_PERSIST] == 0) {
			cep->rxtshift = 0;
			tcp_set_persist_timer(cep);
			break;
			}
		
		}
	}

#ifdef TCP_CFG_SWBUF_CSAVE

/*
 *  tcptsk_alloc_swbufq -- 送信ウィンドバッファ割り当て
 */

static void
tcptsk_alloc_swbufq (T_TCP_CEP *cep)
{
 	ER	error;
	uint_t	win;

	/*
	 *  snd_wnd:  相手の受信可能ウィンドサイズ
	 *  snd_cwnd: 輻輳ウィンドサイズ
	 *
	 *  win: どちらか小さいウィンドサイズに設定する。
	 */
	win = cep->snd_wnd < cep->snd_cwnd ? cep->snd_wnd : cep->snd_cwnd;

	/*
	 *  相手の受信ウィンドが閉じている場合は、開くまで待機する。
	 */
	if (win == 0) {
		cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_WOPEN_PEND;
		}
	else {

#ifdef TCP_CFG_NON_BLOCKING

		/* ノンブロッキングコール */
		if (!IS_PTR_DEFINED(cep->callback)) {
			syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));

			/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
			cep->snd_tskid = TA_NULL;
			cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
			return;
			}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		if ((error = tcpn_get_segment(&cep->swbufq, cep, 0,
		                             (uint_t)TCP_CFG_SWBUF_CSAVE_MIN_SIZE,
		                             (uint_t)TCP_CFG_SWBUF_CSAVE_MAX_SIZE,
		                             (ATR)(NBA_SEARCH_DESCENT |
		                                   NBA_RESERVE_TCP    |
		                                   (GET_TCP_CEPID(cep) & NBA_ID_MASK)), TMO_POL)) != E_OK) {

			/* ネットワークバッファを予約する。*/
			cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_NBUF_PEND;
			}
		else {

			/* 送信ウィンドバッファを初期化する。*/
			tcp_init_swbuf(cep);

#ifdef TCP_CFG_NON_BLOCKING

			if (cep->snd_nblk_tfn == TFN_TCP_GET_BUF) {

				uint_t len;

				/* 送信ウィンドバッファの書き込みアドレスを設定する。*/
				len = TCP_GET_SWBUF_ADDR(cep, cep->snd_p_buf);

				/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)(uint32_t)len);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&len);
#endif


				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->snd_tskid = TA_NULL;
				cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
				}

			else {	/* cep->snd_nblk_tfn == TFN_TCP_SND_DAT || */
				/* cep->snd_nblk_tfn == TFN_TCP_SND_OOB    */

				uint_t len;

				/* 送信ウィンドバッファにデータを書き込む。*/
				len = TCP_WRITE_SWBUF(cep, cep->snd_data, (uint_t)cep->snd_len);

#ifdef TCP_CFG_EXTENTIONS

				/* 送信緊急ポインタを設定する。*/
			        if (cep->snd_nblk_tfn == TFN_TCP_SND_OOB)
					cep->snd_up = cep->snd_una + len;

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

				/* フラグを、送信可能に設定し、強制的に送信する。*/
				cep->flags |= TCP_CEP_FLG_FORCE | TCP_CEP_FLG_FORCE_CLEAR | TCP_CEP_FLG_POST_OUTPUT;

				/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)(uint32_t)len);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&len);
#endif


				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->snd_tskid = TA_NULL;
				cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
				}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			}
		}
	}

/*
 *  tcptsk_free_swbufq -- 送信ウィンドバッファ開放
 */

static void
tcptsk_free_swbufq (T_TCP_CEP *cep)
{
	/*
	 *  受信確認が完了し、ネットワークインタフェースからの
	 *  出力も完了したときは、送信ウィンドバッファキューを解放する。
	 */

	/* 送信ウィンドバッファの使用中サイズをリセットする。*/
	cep->swbuf_count = 0;

	/* 送信ウィンドバッファキューのネットワークバッファを解放する。*/
	syscall(rel_net_buf(cep->swbufq));
	cep->swbufq = NULL;

	/* フラグを空きに設定する。*/
	cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_FREE;

	/* 送信ウィンドバッファに空きができたことを知らせる。*/
	syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));

	/* 送信ウィンドバッファの空き待ちのときは、TCP 出力タスクを起動する。*/
	if ((cep->flags & TCP_CEP_FLG_WBCS_NBUF_REQ) != 0) {
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

/*
 *  TCP 出力タスク
 */

void
tcp_output_task (intptr_t exinf)
{
	static int_t last_ix = 0;

	T_TCP_CEP	*cep;
	ID		tskid;
 	int_t		ix, sel_ix;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP OUTPUT:%d] started.", tskid);

	tcp_init();

#ifdef _IP6_CFG

	/* IPv6 のステートレス・アドレス自動設定を実行する。*/
	in6_if_up(IF_GET_IFNET());

#endif	/* of #ifdef _IP6_CFG */

	while (true) {

		/* 出力がポストされるまで待つ。*/
		syscall(wai_sem(SEM_TCP_POST_OUTPUT));

		if (++ last_ix == tmax_tcp_cepid)
			last_ix = 0;
		sel_ix = ix = last_ix;
		do {
			cep = &tcp_cep[ix];

#ifdef TCP_CFG_SWBUF_CSAVE

			if ((cep->flags & TCP_CEP_FLG_WBCS_NBUF_REQ) != 0 &&
			    ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_FREE ||
			     (cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_NBUF_RSVD)) {
				tcptsk_alloc_swbufq(cep);
				sel_ix = ix;
				}

			if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_ACKED &&
			    (cep->swbufq->flags & NB_FLG_NOREL_IFOUT) == 0) {
				tcptsk_free_swbufq(cep);
				sel_ix = ix;
				}

			/*
			 *  ネットワークインタフェースから送信が終わっていないときは、
			 *  送信を予約する。
			 */
			if (cep->flags & TCP_CEP_FLG_POST_OUTPUT &&
			   (cep->flags & TCP_CEP_FLG_WBCS_MASK) >= TCP_CEP_FLG_WBCS_SENT) {
				syscall(wai_sem(cep->semid_lock));
				if (cep->swbufq == NULL)
					cep->flags &= ~TCP_CEP_FLG_POST_OUTPUT;
				else if (cep->swbufq->flags & NB_FLG_NOREL_IFOUT) {
					cep->flags &= ~TCP_CEP_FLG_POST_OUTPUT;
					cep->flags |=  TCP_CEP_FLG_RESERVE_OUTPUT;
					}
				syscall(sig_sem(cep->semid_lock));
				}

			/*
			 *  送信予約中に、ネットワークインタフェースから送信が終了したら、
			 *  送信を開始する。ただし、完全に送信が終了したときは何もしない。
			 */
			if (cep->flags & TCP_CEP_FLG_RESERVE_OUTPUT) {
				syscall(wai_sem(cep->semid_lock));
				if (cep->swbufq != NULL && (cep->swbufq->flags & NB_FLG_NOREL_IFOUT) == 0) {
					cep->flags |=  TCP_CEP_FLG_POST_OUTPUT;
					}
				syscall(sig_sem(cep->semid_lock));
				cep->flags &= ~TCP_CEP_FLG_RESERVE_OUTPUT;
				}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

			if (cep->flags & TCP_CEP_FLG_POST_OUTPUT) {

				cep->flags &= ~TCP_CEP_FLG_POST_OUTPUT;

#ifdef TCP_CFG_NON_BLOCKING

				if (cep->snd_nblk_tfn == TFN_TCP_CON_CEP && cep->myaddr.portno == TCP_PORTANY) {
				 	ER	error;

					/*
					 *  tcp_con_cep のノンブロッキングコールで、
					 *  未割当のの場合は、ポート番号を割り当てる。
					 *  p_myaddr が NADR (-1) か、
					 *  自ポート番号が TCP_PORTANY なら、自動で割り当てる。
					 */
					if (cep->p_myaddr == NADR || cep->p_myaddr->portno == TCP_PORTANY)
						tcp_alloc_auto_port(cep);
					else if ((error = tcp_alloc_port(cep, cep->p_myaddr->portno)) != E_OK) {

						if (IS_PTR_DEFINED(cep->callback))
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
							(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)error);
#else
							(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&error);
#endif
						else
							syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));

						/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
						cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
						cep->snd_tskid = TA_NULL;
						continue;
						}
					}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

				tcp_output(cep);

				if (cep->flags & TCP_CEP_FLG_CLOSE_AFTER_OUTPUT) {
					/* コネクションを閉じる。*/
					tcp_close(cep);
					cep->flags &= ~TCP_CEP_FLG_CLOSE_AFTER_OUTPUT;
					}

				if (cep->flags & TCP_CEP_FLG_RESTORE_NEXT_OUTPUT) {
					/* snd_nxt を元に戻す。*/
					if (SEQ_GT(cep->snd_old_nxt, cep->snd_nxt))
						cep->snd_nxt = cep->snd_old_nxt;
					cep->flags &= ~TCP_CEP_FLG_RESTORE_NEXT_OUTPUT;
					}

				sel_ix = ix;
				}

			if (++ ix == tmax_tcp_cepid)
				ix = 0;
			} while (ix != last_ix);

		/* 次回は、処理した通信端点を後回しにする。*/
		last_ix = sel_ix;
		}
	}

#endif	/* of #ifdef SUPPORT_TCP */
