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
 *
 *	@(#)tcp_input.c	8.12 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_input.c,v 1.82.2.3 1999/10/14 11:49:38 des Exp $
 */

#include <string.h>

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
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>

#ifdef SUPPORT_TCP

/*
 *  戻り値
 *
 *      RET_OK		正常
 *	RET_DROP	エラー、セグメントを破棄する。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 */

#define RET_OK		(0)
#define RET_NEED_OUTPUT	(1)
#define RET_RETURN	(2)
#define RET_DROP	(-1)
#define RET_RST_DROP	(-2)

/*
 *  関数
 */

static void close_connection	(T_TCP_CEP *cep, bool_t *needoutput);
static void set_rexmt_timer	(T_TCP_CEP *cep,  T_TCP_TIME rtt);
static uint8_t reassemble	(T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, uint8_t flags);
static ER drop_after_ack	(T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff);
static ER listening		(T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, T_TCP_SEQ iss);
static ER proc_ack1		(T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, bool_t *needoutput);
static ER proc_ack2		(T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, bool_t *needoutput);
static ER syn_sent		(T_TCP_HDR *tcph,  T_TCP_CEP *cep);
static void trim_length		(T_TCP_HDR *tcph,  T_TCP_CEP *cep);
static void parse_option	(T_TCP_HDR *tcph,  T_TCP_CEP *cep);
static bool_t update_wnd	(T_TCP_HDR *tcph,  T_TCP_CEP *cep);
static void proc_urg		(T_TCP_HDR *tcph,  T_TCP_CEP *cep);

/*
 *  parse_option -- TCP ヘッダのオプションを解析する。
 */

static void
parse_option (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{
	uint8_t	*opt, type = 0;
	uint_t	len, ol, ssize;

	opt = (uint8_t*)tcph + TCP_HDR_SIZE;
	len = (uint_t)(TCP_HDR_LEN(tcph->doff) - TCP_HDR_SIZE);
	while (len > 0 && (type = *opt) != TCP_OPT_EOL) {
		if (type == TCP_OPT_NOP)
			ol = 1u;
		else {
			if (len < 2)
				break;
			ol = *(opt + 1);
			if (ol < 2 || ol > len)
				break;
			}
		switch (type) {
		case TCP_OPT_NOP:
			break;
		case TCP_OPT_MAXSEG:
			if ((ol == TCP_OPT_LEN_MAXSEG) && (tcph->flags & TCP_FLG_SYN)) {
				ssize = ntohs(*(uint16_t*)(opt + 2));
				if (ssize > MAX_TCP_SND_SEG)
					cep->maxseg = MAX_TCP_SND_SEG;
				else if (ssize < TCP_MINMSS)
					cep->maxseg = TCP_MINMSS;
				else
					cep->maxseg = ssize;
				}
			break;

		case TCP_OPT_WINDOW:
		case TCP_OPT_SACK_PERM:
		case TCP_OPT_TIMESTAMP:
		case TCP_OPT_CC:
		case TCP_OPT_CCNEW:
		case TCP_OPT_CCECHO:
			syslog(LOG_INFO, "[TCP] unsup opt: %d.", type);
			break;

		default:
			syslog(LOG_NOTICE, "[TCP] unexp opt: %d.", type);
			break;
			}
		opt += ol;
		len -= ol;
		}
	}

/*
 *  set_rexmt_timer -- 新しい往復時間を収集し、再送タイマを更新する。
 */

static void
set_rexmt_timer (T_TCP_CEP *cep, T_TCP_TIME rtt)
{
	T_TCP_TIME delta;

	NET_COUNT_TCP(net_count_tcp[NC_TCP_RTT_UPDATES], 1);
	if (cep->srtt != 0) {
		/*
		 *  srtt: 平滑化された RTT
		 *
		 *  計測された RTT (rtt) と現在の平滑化された RTT (srtt) の差 (delta) を求める。
		 *
		 *  delta は 2 ビット左シフト ( 4 倍) した値で保持する。
		 *  srtt  は 5 ビット左シフト (32 倍) した値で保持されている。
		 *
		 *    delta = rtt / 8 - srtt / 8
		 *
		 *  新しい srtt は
		 *
		 *    srtt = rtt / 8 + srtt * 7 / 8
		 *         = srtt + (rtt / 8 - srtt / 8)
		 *
		 *  で計算する。
		 *  このため、rtt を 2 ビット左シフトし、srtt を (5 - 2) ビット右シフトして delta を求める。
		 */
		delta = ((rtt - 1) << TCP_DELTA_SHIFT) - (cep->srtt >> (TCP_SRTT_SHIFT - TCP_DELTA_SHIFT));
		cep->srtt += delta;
		if (cep->srtt <= 0)
			cep->srtt = 1;

		/*
		 *  delta の絶対値 | delta | を求める。
		 */
		if (delta < 0)
			delta = - delta;

		/*
		 *  rttvar: 平滑化された分散
		 *
		 *  rttvar は 4 ビット左シフト (16 倍) した値で保持されている。
		 *
		 *    delta = |delta| / 4 - rttvar / 4
		 *
		 *  新しい rttvar は
		 *
		 *    rttvar = |delta|/ 4 + rttvar * 3 /4
		 *           = rttvar + (|delta| / 4 - rttvar / 4)
		 *
		 *  で計算する。
		 */
		delta -= cep->rttvar >> (TCP_RTTVAR_SHIFT - TCP_DELTA_SHIFT);
		cep->rttvar += delta;
		if (cep->rttvar <= 0)
			cep->rttvar = 1;
		}
	else {
		/*
		 *  まだ srtt の設定が行われていないときは、今回計測された RTT を使用する。
		 *  平滑化された RTT (srtt) には、RTT を 5 ビット左シフト (32倍) した値。
		 *  平滑化された分散 (rttvar) には、RTT の 1/2 を 4 ビット左シフト (16倍) した値。
		 */
		cep->srtt   = rtt <<  TCP_SRTT_SHIFT;
		cep->rttvar = rtt << (TCP_RTTVAR_SHIFT - 1);
		}

	/*
	 *  rtt の測定を終了し、再送回数をリセットする。
	 */
	cep->rtt = cep->rxtshift = 0;

	/*
	 *  RTT に許される最小値 と rtt + 2 の大きな値の方を再送タイムアウトの最小値にする。
	 */
	if (rtt + 2 < TCP_TVAL_MIN)
		cep->rxtcur = tcp_range_set(tcp_rexmt_val(cep),
		                            (T_TCP_TIME)TCP_TVAL_MIN,
		                            (T_TCP_TIME)TCP_TVAL_MAX_REXMT);
	else
		cep->rxtcur = tcp_range_set(tcp_rexmt_val(cep),
		                            (T_TCP_TIME)(rtt + 2),
		                            (T_TCP_TIME)TCP_TVAL_MAX_REXMT);
	}

/*
 *  reassemble -- 受信セグメントを再構成する。順番通りに受信したときの処理
 */

static uint8_t
reassemble (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, uint8_t flags)
{
	T_TCP_Q_HDR 	*qhdr;
	T_TCP_HDR	*tcph;

	tcph = GET_TCP_HDR(input, thoff);
	if (tcph->sum > cep->rbufsz - cep->rwbuf_count) {
		/*
		 *  受信ウィンドバッファに空きがないときは破棄する。
		 */
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DROP_SEGS], 1);
		syscall(rel_net_buf(input));
		cep->flags |= TCP_CEP_FLG_ACK_NOW;
		flags &= ~TCP_FLG_FIN;
		}
	else if (tcph->seq == cep->rcv_nxt &&
	         cep->reassq == NULL &&
	         cep->fsm_state == TCP_FSM_ESTABLISHED) {
		/*
		 *  順番通りにセグメントを受信した時の処理
		 *  受信セグメントの並べ替えは不要なので
		 *  そのまま受信ウィンドバッファに書き込む。
		 */

#ifdef TCP_CFG_DELAY_ACK

		cep->flags |= TCP_CEP_FLG_DEL_ACK;

#else/* of #ifdef TCP_CFG_DELAY_ACK */

		cep->flags |= TCP_CEP_FLG_ACK_NOW;

#endif/* of #ifdef TCP_CFG_DELAY_ACK */

		qhdr = GET_TCP_Q_HDR(input, thoff);

		/*  TCP ヘッダの位置を保存する。*/
		SET_IP_TCP_Q_HDR_OFFSET(input, thoff);

		/* SDU のオフセット（元はウィンドサイズ）をリセットする。*/
		qhdr->soff = 0;

		/* データを受信ウィンドバッファに書き込む。*/
		TCP_WRITE_RWBUF(cep, input, thoff);
		}
	else {
		flags = tcp_write_raque(input, cep, thoff, flags);
		cep->flags |= TCP_CEP_FLG_ACK_NOW;
		}
	return flags;
	}

/*
 *  listening -- 受動オープンして，状態が LISTEN の処理
 *
 *    戻り値:
 *      RET_OK		正常
 *	RET_DROP	エラー、セグメントを破棄する。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 */

static ER
listening (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, T_TCP_SEQ iss)
{
	T_TCP_HDR	*tcph;

	tcph = GET_TCP_HDR(input, thoff);

	/* 
	 *  フラグに RST がセットされていれば破棄する。
	 */
	if (tcph->flags & TCP_FLG_RST)
		return RET_DROP;

	/*  
	 *  フラグに ACK がセットさてれいれば、
	 *  リセットを送って破棄する。
	 */
	if (tcph->flags & TCP_FLG_ACK)
		return RET_RST_DROP;

	/* 
	 *  フラグに SYN がセットされていなければれば破棄する。
	 */
	if ((tcph->flags & TCP_FLG_SYN) == 0)
		return RET_DROP;

	/*
	 *  受信可能な IP アドレスとポート番号であることを確認する。
	 */
	if (!tcp_is_addr_accept(input, thoff))
		return RET_DROP;

	/* 相手のアドレスを記録する。*/
	IN_COPY_TO_HOST(&cep->dstaddr.ipaddr, input);
	cep->dstaddr.portno = tcph->sport;

	/* オプションを処理する。*/
	parse_option(tcph, cep);

	/* シーケンス番号を初期化する。*/
	if (tcp_iss == 0)
		tcp_init_iss();

	/* 自分のシーケンス番号の初期値を記録する。*/
	if (iss != 0)
		cep->iss = iss;
	else
		cep->iss = tcp_iss;

	tcp_iss += TCP_ISS_INCR() / 4;

	/* 相手のシーケンス番号の初期値を記録する。*/
	cep->irs = tcph->seq;

	/* 送受信シーケンス番号を初期化する。*/
	init_send_seq(cep);
	init_receive_seq(cep);

	/* 送信ウインドサイズを設定する。*/
	cep->snd_wnd = tcph->win;

	/* 最終設定 */
	cep->flags    |= TCP_CEP_FLG_ACK_NOW;
	cep->fsm_state = TCP_FSM_SYN_RECVD;
	cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_INIT;

	return RET_OK;
	}

/*
 *  syn_sent -- 能動オープンして、状態が SYN 送信済の処理
 *
 *    戻り値:
 *      RET_OK		正常
 *	RET_DROP	エラー、セグメントを破棄する。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 */

static ER
syn_sent (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{
	ER error = RET_OK;

	/*
	 *  相手から受信確認が送られて来ても、
	 *
	 *    ACK <= iss && 送信した最大 SEQ (snd_max) < ACK
	 *
	 *  なら、リセットを送ってセグメントを破棄する。
	 */
	if ((tcph->flags & TCP_FLG_ACK) &&
	    (SEQ_LE(tcph->ack, cep->iss) || SEQ_GT(tcph->ack, cep->snd_max)))
		return RET_RST_DROP;

	/*
	 *  RST/ACK フラグの応答があれば、ポートが開いていない
	 *  ことを意味している。
	 */
	if (tcph->flags & TCP_FLG_RST) {
		if (tcph->flags & TCP_FLG_ACK) {
			cep->net_error = EV_CNNRF;
			cep = tcp_drop(cep, E_CLS);
			}
		return RET_DROP;
		}

	/*
	 *  SYN フラグがなければセグメントを破棄する。
	 */
	if ((tcph->flags & TCP_FLG_SYN) == 0)
		return RET_DROP;

	cep->snd_wnd = tcph->win;	/* snd_wnd: 相手の受信可能ウィンドサイズ	*/
	cep->irs     = tcph->seq;	/* irs:     相手のシーケンス番号の初期値	*/
	init_receive_seq(cep);		/* 送受信シーケンス番号を初期化する。		*/

	if (tcph->flags & TCP_FLG_ACK) {
		/*
		 *  ACK フラグがあるときの処理
		 *
		 *  受信を期待している最大の SEQ (rcv_adv) を
		 *  受信可能なウィンドサイズ (rcv_wnd) 分進める。
		 */
		cep->rcv_adv += cep->rcv_wnd;	/* rcv_adv: 受信を期待している最大の SEQ	*/
						/* rcv_wnd: 受信可能なウィンドサイズ		*/

		/* 未確認の最小送信 SEQ (snd_una) を SYN 分 (1 オクテット) 進める。*/
		cep->snd_una ++;

#ifdef TCP_CFG_DELAY_ACK

		if (tcph->sum != 0)		/* tcph->sum は SDU 長 */
			cep->flags |= TCP_CEP_FLG_DEL_ACK;
		else
			cep->flags |= TCP_CEP_FLG_ACK_NOW;

#else/* of #ifdef TCP_CFG_DELAY_ACK */

		cep->flags |= TCP_CEP_FLG_ACK_NOW;

#endif/* of #ifdef TCP_CFG_DELAY_ACK */

		if (cep->flags & TCP_CEP_FLG_NEED_FIN) {
			/*
			 *  CEP で FIN 送信が要求されていれば、
			 *  切断処理を開始し、
			 *  CEP の状態を FIN Wait 1 にする。
			 */
			cep->fsm_state = TCP_FSM_FIN_WAIT_1;
			cep->flags  &= ~TCP_CEP_FLG_NEED_FIN;
			tcph->flags &= ~TCP_FLG_SYN;
			}
		else {
			/*
			 *  相手から ACK が応答されたので、
			 *  CEP の状態を コネクション開設完了状態にする。
			 */
			cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_IDLE;
			cep->fsm_state  = TCP_FSM_ESTABLISHED;
			NET_COUNT_MIB(tcp_stats.tcpActiveOpens, 1);
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_ESTABLISHED));

#ifdef TCP_CFG_NON_BLOCKING

			if (cep->snd_nblk_tfn == TFN_TCP_CON_CEP) {

				/* 相手のアドレスをコピーする。*/

#if defined(_IP6_CFG) && defined(_IP4_CFG)

				if (cep->flags & TCP_CEP_FLG_IPV4) {
					(*cep->p_dstaddr4).ipaddr = ntohl(cep->dstaddr.ipaddr.s6_addr32[3]);
					(*cep->p_dstaddr4).portno = cep->dstaddr.portno;
					}
				else
					*cep->p_dstaddr = cep->dstaddr;

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				*cep->p_dstaddr = cep->dstaddr;

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, E_OK);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					ER	error = E_OK;

					NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					}
				else {
					syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
					error = RET_RST_DROP;
					}
				cep->p_dstaddr = NULL;
				cep->snd_tskid = TA_NULL;
				cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
				}
			else

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

				NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
			}
		}
	else {
		/* ACK フラグがないときは、ACK を送って、CEP の状態を SYN 受信済みにする。*/
		cep->flags |= TCP_CEP_FLG_ACK_NOW;
		cep->timer[TCP_TIM_REXMT] = 0;
		cep->fsm_state  = TCP_FSM_SYN_RECVD;
		}

	return error;
	}

/*
 *  trim_length -- 受信した SDU 長を調整する。
 */

static void
trim_length (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{
	tcph->seq ++;
	if (tcph->sum > cep->rcv_wnd) {		/* 注意: tcph->sum は SDU 長 */
		/*
		 *  SDU 長が受信ウィンドサイズより大きいときは、受信ウィンドサイズ以降は
		 *  破棄し、FIN に応答しないことで、破棄したデータを再送させる。
		 */
		tcph->sum    = (uint16_t)cep->rcv_wnd;
		tcph->flags &= ~TCP_FLG_FIN;
		}
	cep->snd_wl1 = tcph->seq - 1;		/* cep->snd_wl1: ウィンド更新 SEQ 番号	*/

#ifdef TCP_CFG_EXTENTIONS
	cep->rcv_up  = tcph->seq;		/* cep->rcv_up : 受信した緊急ポインタ	*/
#endif
	}

/*
 *  proc_ack2 -- ACK の処理 (2)
 *
 *    戻り値
 *
 *      RET_OK		正常
 *      RET_RETURN	正常、リターンする。
 *	RET_DROP	エラー、セグメントを破棄する。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 */

static ER
proc_ack2 (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, bool_t *needoutput)
{
	T_TCP_HDR	*tcph;
	ER		ret = RET_OK;
	uint32_t	acked;
	bool_t		ourfinisacked = false;

	tcph = GET_TCP_HDR(input, thoff);

	/*
	 *  相手に受信確認された ACK から、まだ確認されていない
	 *  最小送信 SEQ (snd_una) を引くと、送信ウィンドバッファから
	 *  削除してよいオクテット数 (acked) になる。
	 */
	acked = tcph->ack - cep->snd_una;
	NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_ACKS], 1);

	/*
	 *  往復時間計測 (rtt) が設定されていて、計測開始 SEQ より
	 *  後の ACK を受信したら、タイマバックオフをキャンセルし、
	 *  再送タイマを再設定する。
	 */
	if (cep->rtt && SEQ_GT(tcph->ack, cep->rtseq)) {
		set_rexmt_timer(cep, cep->rtt);
		}

	/*
	 *  全ての未確認データが ACK されたら、再送タイマを停止し、
	 *  再開を記憶する (さらに出力か持続)。
	 *  もし、ACK すべき、さらに多くのデータがあるなら、再送タイマに
	 *  現在の再送タイムアウトを設定する。
	 */
	if (tcph->ack == cep->snd_max) {	/* cep->snd_max: 送信した最大 SEQ */

#ifdef TCP_CFG_SWBUF_CSAVE

		/*
		 * 送信ウィンドバッファの省コピー機能が有効の場合は、
		 * 送信済みで、ACKが完了するまで再送タイマを変更しない。
		 */
		if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_ACKED)
			cep->timer[TCP_TIM_REXMT] = 0;

#else	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		cep->timer[TCP_TIM_REXMT] = 0;

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		*needoutput = true;
		}
	else if (cep->timer[TCP_TIM_PERSIST] == 0) {
	 	cep->timer[TCP_TIM_REXMT] = cep->rxtcur;	/* cep->rxtcur: 現在の再送タイムアウト */
		}

	/* 相手が受信確認したデータがあるときの処理 */
	if (acked) {
		uint32_t cw   = cep->snd_cwnd;	/* cep->snd_cwnd: 輻輳ウィンドサイズ	*/
		uint32_t incr = cep->maxseg;		/* cep->maxseg:   最大セグメントサイズ	*/

		/*
		 *  新たに相手が受信確認したデータがあったときは、
		 *  輻輳ウィンドサイズを大きくする。
		 *  輻輳ウィンドサイズ (snd_cwnd) が
		 *  輻輳ウィンドサイズのしきい値 (snd_ssthresh) より大きいときは
		 *  輻輳回避制御を行い。
		 *
		 *    snd_cwnd = snd_cwnd + maxseg * maxseg / snd_cwnd;
		 *
		 *  等しいか小さいときは、スロースタート制御を行う。
		 *
		 *    snd_cwnd = snd_cwnd + maxseg
		 *
		 */
		if (cw > cep->snd_ssthresh)
			/* 輻輳回避制御 */
			incr = incr * incr / cw;

		if (cw + incr < MAX_TCP_WIN_SIZE)
			cep->snd_cwnd = (uint16_t)(cw + incr);
		else
			cep->snd_cwnd = MAX_TCP_WIN_SIZE;

		/*
		 *  送信ウィンドバッファから、相手が受信確認したデータ数 (acked) のデータを削除する。
		 */
		if (acked > cep->swbuf_count) {
			cep->snd_wnd -= cep->swbuf_count;
			TCP_DROP_SWBUF(cep, (uint_t)cep->swbuf_count);
			ourfinisacked = true;
			}
		else {
			cep->snd_wnd -= (uint16_t)acked;
			TCP_DROP_SWBUF(cep, (uint_t)acked);
			ourfinisacked = false;
			}

		/* 送信ウィンドバッファに空きができたことを知らせる。*/
		syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));

		/*
		 *  送達確認されていない最小送信 SEQ (snd_una) を
		 *  今回送達確認された ACK まで進め、
		 *  次の送信データの SEQ (snd_nxt) も、新しい
		 *  送達確認されていない最小送信 SEQ (snd_una)
		 *  まで進める。
		 */
		cep->snd_una += acked;
		if (SEQ_LT(cep->snd_nxt, cep->snd_una))
			cep->snd_nxt = cep->snd_una;

		/*
		 *  状態により分岐
		 */
		switch (cep->fsm_state) {
		case TCP_FSM_FIN_WAIT_1:	/* APP が終了、FIN 送信済み、ACK 待ち */
			if (ourfinisacked) {
				cep->fsm_state = TCP_FSM_FIN_WAIT_2;
				cep->timer[TCP_TIM_2MSL] = TCP_TVAL_KEEP_COUNT * TCP_TVAL_KEEP_INTERVAL;
				}
			break;
		case TCP_FSM_CLOSING:		/* 同時クローズ、FIN 交換済み、ACK 待ち */
			if (ourfinisacked) {
				/*
				 *  送信した FIN が確認されていれば状態を変更し、
				 *  すべてのタイマをリセットした後、2MSL タイマを設定する。
				 */
				cep->fsm_state = TCP_FSM_TIME_WAIT;
				tcp_cancel_timers(cep);
				cep->timer[TCP_TIM_2MSL] = 2 * TCP_TVAL_MSL;
				}
			break;
		case TCP_FSM_LAST_ACK:		/* APP が終了、ACK 待ち */
			if (ourfinisacked) {
				/*
				 *  送信した FIN が確認されていれば、cep をクローズし、
				 *  セグメントを破棄する。
				 */
				cep = tcp_close(cep);
				ret = RET_DROP;
				}
			break;
		case TCP_FSM_TIME_WAIT:		/* 相手からの FIN 受信済み、時間待ち */
			/*
			 *  相手から FIN が再送された。もう一度2MSL タイマを設定し、
			 *  ACK 送信後、セグメントを破棄する。
			 */
			cep->timer[TCP_TIM_2MSL] = 2 * TCP_TVAL_MSL;
			return drop_after_ack(input, cep, thoff);
			}
		}

	return ret;
	}

/*
 *  proc_ack1 -- ACK の処理 (1)
 *
 *    戻り値:
 *      RET_OK		正常
 *      RET_RETURN	正常、リターンする。
 *	RET_DROP	エラー、セグメントを破棄する。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 *
 */

static ER
proc_ack1 (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, bool_t *needoutput)
{
	T_TCP_HDR *tcph = GET_TCP_HDR(input, thoff);

	switch (cep->fsm_state) {
	case TCP_FSM_SYN_RECVD:		/* SYN を受信し、SYN 送信済み	*/

		/* 状態を変更する。*/
		if (cep->flags & TCP_CEP_FLG_NEED_FIN) {
			cep->fsm_state  = TCP_FSM_FIN_WAIT_1;
			cep->flags &= ~TCP_CEP_FLG_NEED_FIN;
			}
		else {
			cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_IDLE;
			cep->fsm_state  = TCP_FSM_ESTABLISHED;

			/* TCP 通信端点からTCP 受付口を解放する。*/
			cep->rep = NULL;

#if defined(_IP6_CFG) && defined(_IP4_CFG)
			cep->rep4 = NULL;
#endif

			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_ESTABLISHED));

#ifdef TCP_CFG_NON_BLOCKING

			if (cep->rcv_nblk_tfn == TFN_TCP_ACP_CEP) {

				/* 相手のアドレスをコピーする。*/

#if defined(_IP6_CFG) && defined(_IP4_CFG)

				if (cep->flags & TCP_CEP_FLG_IPV4) {
					(*cep->p_dstaddr4).ipaddr = ntohl(cep->dstaddr.ipaddr.s6_addr32[3]);
					(*cep->p_dstaddr4).portno = cep->dstaddr.portno;
					}
				else
					*cep->p_dstaddr = cep->dstaddr;

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				*cep->p_dstaddr = cep->dstaddr;

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					NET_COUNT_MIB(tcp_stats.tcpPassiveOpens, 1);
					NET_COUNT_TCP(net_count_tcp[NC_TCP_ACCEPTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, E_OK);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					ER	error = E_OK;

					NET_COUNT_MIB(tcp_stats.tcpPassiveOpens, 1);
					NET_COUNT_TCP(net_count_tcp[NC_TCP_ACCEPTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					cep->p_dstaddr = NULL;
					cep->rcv_tskid = TA_NULL;
					cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
					}
				else {
					syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
					cep->p_dstaddr = NULL;
					cep->rcv_tskid = TA_NULL;
					cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
					return RET_RST_DROP;
					}
				}

			if (cep->snd_nblk_tfn == TFN_TCP_CON_CEP) {

				/* 相手のアドレスをコピーする。*/

#if defined(_IP6_CFG) && defined(_IP4_CFG)

				if (cep->flags & TCP_CEP_FLG_IPV4) {
					(*cep->p_dstaddr4).ipaddr = ntohl(cep->dstaddr.ipaddr.s6_addr32[3]);
					(*cep->p_dstaddr4).portno = cep->dstaddr.portno;
					}
				else
					*cep->p_dstaddr = cep->dstaddr;

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				*cep->p_dstaddr = cep->dstaddr;

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

				if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					NET_COUNT_MIB(tcp_stats.tcpActiveOpens, 1);
					NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, E_OK);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					ER	error = E_OK;

					NET_COUNT_MIB(tcp_stats.tcpActiveOpens, 1);
					NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					cep->p_dstaddr = NULL;
					cep->snd_tskid = TA_NULL;
					cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
					}
				else {
					syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
					cep->p_dstaddr = NULL;
					cep->snd_tskid = TA_NULL;
					cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
					return RET_RST_DROP;
					}
				}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			if (cep->rcv_tfn == TFN_TCP_ACP_CEP) {
				NET_COUNT_MIB(tcp_stats.tcpPassiveOpens, 1);
				NET_COUNT_TCP(net_count_tcp[NC_TCP_ACCEPTS], 1);
				}

			if (cep->snd_tfn == TFN_TCP_CON_CEP) {
				NET_COUNT_MIB(tcp_stats.tcpActiveOpens, 1);
				NET_COUNT_TCP(net_count_tcp[NC_TCP_CONNECTS], 1);
				}
			}

		/*
		 *  SDU がなく FIN がついていなければ、tcp_move_ra2rw() を呼出す。
		 */
		if (tcph->sum == 0 && (tcph->flags & TCP_FLG_FIN) == 0)		/* tcph->sum は SDU 長 */
			tcph->flags = tcp_move_ra2rw(cep, tcph->flags);
		
		cep->snd_wl1 = tcph->seq - 1;	/* snd_wl1: ウィンド更新 SEQ */

		/* break; 下に落ちる。*/
	
	case TCP_FSM_ESTABLISHED:	/* コネクション開設完了		*/
	case TCP_FSM_FIN_WAIT_1:	/* 終了して、FIN 送信済み		*/
	case TCP_FSM_FIN_WAIT_2:	/* 終了、FIN 伝達確認受信、FIN待ち*/
	case TCP_FSM_CLOSE_WAIT:	/* FIN 受信、クローズ待ち		*/
	case TCP_FSM_CLOSING:		/* 終了、FIN 交換済み、ACK 待ち	*/
	case TCP_FSM_LAST_ACK:		/* FIN 受信、終了、ACK 待ち	*/
	case TCP_FSM_TIME_WAIT:		/* 終了、時間待ち		*/

		if (SEQ_LE(tcph->ack, cep->snd_una)) {

			/*
			 *  受信確認 ACK が 未確認の最小送信 SEQ (snd_una) と同じか以前のときの処理
			 *  つまり、多重に ACK を受信したことを意味している。
			 */

			if (tcph->sum == 0 && tcph->win == cep->snd_wnd) {	/* tcph->sum は SDU 長 */

				/*
				 *  SDU がなく、相手のウィンドサイズが変更されていなければ、
				 *  すでに送信したセグメントの中で、ACK (tcph->ack) と
				 *  同じ SEQ から始まるセグメントが、途中で消失した可能性がある。
				 *  この場合は、高速再転送と高速リカバリを行う。
				 */
				NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DUP_ACKS], 1);

				if (cep->timer[TCP_TIM_REXMT] == 0 || tcph->ack != cep->snd_una) {

					/*
					 *  再送タイマがセットされていないとき、
					 *  または、ACK (tcph->ack) と未確認の最小送信 SEQが
					 *  一致しないときは、多重 ACK 数を 0 にする。
					 */
					cep->dupacks = 0;
					}

				else if (++ cep->dupacks == MAX_TCP_REXMT_THRESH) {

					/*
					 *  多重 ACK 数がしきい値 (標準 3) になったら
					 *  高速再転送処理を開始する。
					 */
					uint_t		win;

					/*
					 *  輻輳ウィンドサイズ(snd_cwnd)のしきい値を設定する。
					 *
					 *    相手の受信可能ウィンドサイズ (snd_wnd) か
					 *    輻輳ウィンドサイズ (snd_cwnd) の 1/2。
					 *    ただし、2 * maxseg 以上。
					 *
					 */
					if (cep->snd_wnd < cep->snd_cwnd)
						win = cep->snd_wnd / 2 / cep->maxseg;
					else
						win = cep->snd_cwnd / 2 / cep->maxseg;
					if (win < 2)
						win = 2;
					cep->snd_ssthresh = win * cep->maxseg;

					/* 再送タイマと往復時間をリセットする。*/
					cep->timer[TCP_TIM_REXMT] = 0;
					cep->rtt = 0;

					/* 消失したセグメントを送信する。*/
					cep->snd_old_nxt = cep->snd_nxt;
					cep->snd_nxt     = tcph->ack;
					cep->snd_cwnd    = cep->maxseg;

					/*
					 *  snd_nxt を元に戻すように設定して
					 *  送信を指示する。
					 */
					cep->flags |=  TCP_CEP_FLG_POST_OUTPUT |
					               TCP_CEP_FLG_FORCE       |
					               TCP_CEP_FLG_FORCE_CLEAR |
					               TCP_CEP_FLG_RESTORE_NEXT_OUTPUT;
					sig_sem(SEM_TCP_POST_OUTPUT);

					/* 輻輳ウィンドサイズを更新する。*/
					cep->snd_cwnd = (uint16_t)(cep->snd_ssthresh
					                   + cep->maxseg * cep->dupacks);
					
					return RET_DROP;
					}

				else if (cep->dupacks > MAX_TCP_REXMT_THRESH) {

					/*
					 *  多重 ACK 数がしきい値 (標準 3) を超えたら
					 *  輻輳ウィンドサイズを増加しながら再送する。
					 */
					cep->snd_cwnd += cep->maxseg;

					/* 送信を指示する。*/
					cep->flags |=  TCP_CEP_FLG_POST_OUTPUT;
					sig_sem(SEM_TCP_POST_OUTPUT);

					return RET_DROP;
					}
				} 
			else
				cep->dupacks = 0;
			break;
			}

		/*
		 *  受信確認 ACK が 未確認の最小送信 SEQ (snd_una) 以降のときの処理
		 */
		if (cep->dupacks >= MAX_TCP_REXMT_THRESH && cep->snd_cwnd > cep->snd_ssthresh)
		 	/*
			 *  高速再転送を行っていたときは、輻輳ウィンドサイズをしきい値まで戻す。
			 */
			cep->snd_cwnd = (uint16_t)cep->snd_ssthresh;
	
		cep->dupacks = 0;

		if (SEQ_GT(tcph->ack, cep->snd_max))
			/*
			 *  受信した ACK が送信した最大 SEQ を超えていたときの処理
			 */
			return drop_after_ack(input, cep, thoff);

		if (cep->flags & TCP_CEP_FLG_NEED_SYN) {
			/*
			 *  SYN 送信要求を取り消して、未確認の最小送信 SEQ を進める。
			 */
			cep->flags &= ~TCP_CEP_FLG_NEED_SYN;
			cep->snd_una ++;
			}
		
		return proc_ack2(input, cep, thoff, needoutput);
		break;
		}
	return RET_OK;
	}

/*
 *  update_wnd -- ウィンドサイズを更新する。
 *
 *    戻り値: 送信が必要なら true を返す。
 */

static bool_t
update_wnd (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{

	/*
	 *  更新条件
	 *
	 *    ACK フラグがセットされている &&
	 *    (前回ウィンドを更新した SEQ (snd_wl1) が SEQ より前 ||
	 *     前回ウィンドを更新した SEQ (snd_wl1) が SEQ と同じ &&
	 *     (前回ウィンドを更新した ACK (snd_wl2) が ACK より前 ||
	 *      (前回ウィンドを更新した ACK (snd_wl2) が ACK と同じ &&
	 *       WIN が相手の受信可能ウィンドサイズ (snd_wnd) より大きい
	 *       )
	 *      )
	 *     )
	 */
	if ((tcph->flags & TCP_FLG_ACK) &&
	    (SEQ_LT(cep->snd_wl1, tcph->seq) ||
	     (cep->snd_wl1 == tcph->seq &&
	      (SEQ_LT(cep->snd_wl2, tcph->ack) ||
	       (cep->snd_wl2 == tcph->ack && tcph->win > cep->snd_wnd))))) {

		cep->snd_wnd = tcph->win;
		cep->snd_wl1 = tcph->seq;
		cep->snd_wl2 = tcph->ack;

		if (cep->snd_wnd > cep->max_sndwnd)
			/* 今までの最大送信ウィンドサイズを更新する。*/
			cep->max_sndwnd = cep->snd_wnd;

#ifdef TCP_CFG_SWBUF_CSAVE

		if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_WOPEN_PEND) {

			/*
			 *  送信ウィンドバッファ用のネットワークバッファ割り当て中で、
			 *  相手の受信ウィンドが空くのを待っているときの処理
			 */
			if (cep->snd_wnd > 0) {

				/*
				 *  相手の受信ウィンドが空いたときは、
				 *  送信ウィンドバッファ用のネットワークバッファ割り当てを再開する。
				 */
				cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK)
				                         |  TCP_CEP_FLG_WBCS_FREE |  TCP_CEP_FLG_POST_OUTPUT;
				sig_sem(SEM_TCP_POST_OUTPUT);
				}
			}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		return true;
		}
	else
		return false;
	}

/*
 *  proc_urg -- 緊急データつきのセグメントの処理
 */

#ifdef TCP_CFG_EXTENTIONS

static void
proc_urg (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{
	if ((tcph->flags & TCP_FLG_URG) && VALID_URG_POINTER(tcph->urp) &&
	    TCP_FSM_HAVE_RCVD_FIN(cep->fsm_state) == 0) {

		/* 緊急データつきのセグメントの処理 */

		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_URG_SEGS], 1);
		if (tcph->urp + cep->rwbuf_count > cep->rbufsz) {

			/*
			 *  緊急ポインタの位置が受信ウィンドバッファの
			 *  範囲を超えるときは何もしない。
			 */
			tcph->urp    = 0;
			tcph->flags &= ~TCP_FLG_URG;
			}

		if (SEQ_GT(tcph->seq + tcph->urp, cep->rcv_up))
			/* 緊急ポインタが更新されたときの処理 */
			cep->rcv_up = tcph->seq + tcph->urp;

		if ((tcph->flags & TCP_FLG_URG) && (tcph->urp + TCP_CFG_URG_OFFSET) < tcph->sum) {	/* tcph->sum は TCP の SDU 長 */

			/*
			 *  緊急ポインタの位置が、今回受信したセグメント内の場合は、
			 *  コールバック関数を呼び出す。
			 */
			cep->urg_tcph = tcph;
			if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

				(*cep->callback)(GET_TCP_CEPID(cep), TEV_TCP_RCV_OOB, (void*)(uint32_t)1);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				uint32_t	len = 1;

				(*cep->callback)(GET_TCP_CEPID(cep), TEV_TCP_RCV_OOB, (void*)&len);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

				}
			else {
				syslog(LOG_WARNING, "[TCP] no call back for OOB, CEP: %d.", GET_TCP_CEPID(cep));
				}

			if (cep->urg_tcph != NULL) {
				/* コールバック関数内で tcp_rcv_oob() を呼出さなかった。*/
				cep->urg_tcph = NULL;
				tcph->urp = 0;
				}
			else {
				/* 
				 *  コールバック関数内で tcp_rcv_oob() を呼出した時は、
				 *  SDU 長の補正値を設定する。
				 */
				tcph->urp = 1;
				}
			}
		else if (tcph->urp > 0) {
			tcph->urp = 0;
			}

		}
	else if (SEQ_GT(cep->rcv_nxt, cep->rcv_up)) {
		cep->rcv_up = cep->rcv_nxt;
		tcph->urp = 0;
		}
	}

#else	/* of #ifdef TCP_CFG_EXTENTIONS */

static void
proc_urg (T_TCP_HDR *tcph, T_TCP_CEP *cep)
{
	tcph->urp = 0;
	}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

/*
 *  drop_after_ack -- 受信セグメントを破棄した後、ACK を返す (注意: 名前とは合っていない)。
 *
 *    戻り値:
 *      RET_RETURN	正常、リターンする。
 *	RET_RST_DROP	エラー、RST を送信し、セグメントを破棄する。
 */

static ER
drop_after_ack (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff)
{
	T_TCP_HDR *tcph = GET_TCP_HDR(input, thoff);

	/*
	 *    SYN 受信状態で、ACK が送達確認されていない最小送信 SEQ (snd_una) より
	 *    前の値か、送信された最大 SEQ (snd_max) より後の値の場合は、相手に RST を
	 *    送って終了する。これは、"LAND" DoS 攻撃への防御であり、偽造された SYN
	 *    セグメントを送信しつづけるポート間での ACK ストームを防ぐ。
	 */
	if (cep->fsm_state == TCP_FSM_SYN_RECVD && (tcph->flags & TCP_FLG_ACK) &&
	    (SEQ_GT(cep->snd_una, tcph->ack) ||
	     SEQ_GT(tcph->ack, cep->snd_max)))
		return RET_RST_DROP;

	syscall(rel_net_buf(input));

	/* 送信を指示する。*/
	cep->flags |=  TCP_CEP_FLG_ACK_NOW | TCP_CEP_FLG_POST_OUTPUT;
	sig_sem(SEM_TCP_POST_OUTPUT);
	return RET_RETURN;
	}

/*
 *  close_connection -- コネクション開放処理、相手から FIN を受信した。
 */

static void
close_connection (T_TCP_CEP *cep, bool_t *needoutput)
{
	if (TCP_FSM_HAVE_RCVD_FIN(cep->fsm_state) == 0) {

#ifdef TCP_CFG_DELAY_ACK

		if (cep->flags & TCP_CEP_FLG_NEED_SYN)
			cep->flags |= TCP_CEP_FLG_DEL_ACK;
		else
			cep->flags |= TCP_CEP_FLG_ACK_NOW;

#else/* of #ifdef TCP_CFG_DELAY_ACK */

		cep->flags |= TCP_CEP_FLG_ACK_NOW;

#endif/* of #ifdef TCP_CFG_DELAY_ACK */

		cep->rcv_nxt ++;
		}

	switch (cep->fsm_state) {
	case TCP_FSM_SYN_RECVD:		/* SYN を受信し、SYN 送信済み	*/
	case TCP_FSM_ESTABLISHED:	/* コネクション開設完了		*/
		cep->fsm_state = TCP_FSM_CLOSE_WAIT;
		syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_RWBUF_READY));
		break;

	case TCP_FSM_FIN_WAIT_1:	/* APP が終了、FIN 送信済み、ACK 待ち */
		cep->fsm_state = TCP_FSM_CLOSING;
		break;

	case TCP_FSM_FIN_WAIT_2:	/* 相手からの FIN 待ち */
		cep->fsm_state = TCP_FSM_TIME_WAIT;
		tcp_cancel_timers(cep);
		cep->timer[TCP_TIM_2MSL] = 2 * TCP_TVAL_MSL;

		/*
		 *  FIN WAIT 2 状態では、
		 *  受信は可能であるが、すでに送信は終了している。
		 *  相手の送信も終了したので、入力タスクのみ起床する。
		 */
		syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

		/* 相手からの FIN に対して応答を返す。*/
		tcp_respond(NULL, cep, cep->rcv_nxt, cep->snd_una,
		            cep->rbufsz - cep->rwbuf_count, TCP_FLG_ACK);
		cep->flags &= ~TCP_CEP_FLG_ACK_NOW;
		*needoutput = false;

		/*
		 *  必要な情報を Time Wait 用 TCP 通信端点に移して、
		 *  標準の TCP 通信端点を開放する。
		 */
		tcp_move_twcep(cep);

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

		break;

	case TCP_FSM_TIME_WAIT:		/* 相手からの FIN 受信済み、時間待ち */
		cep->timer[TCP_TIM_2MSL] = 2 * TCP_TVAL_MSL;
		break;
		}
	}

/*
 *  tcp_input -- TCP の入力関数
 *
 *	注意: input には IF ヘッダと IP ヘッダが先頭にある。
 */

uint_t
tcp_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_TCP_HDR	*tcph;
	T_TCP_CEP	*cep = NULL;
	T_TCP_SEQ	iss = 0;
	ER		ret;
	bool_t		needoutput = false;
	int_t		rbfree;
	int32_t		todrop, win;
	uint16_t	seglen;
	uint8_t		flags;

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0
	T_TCP_TWCEP	*twcep;
#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_OCTETS],
	              input->len - GET_IF_IP_HDR_SIZE(input));
	NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_SEGS], 1);
	NET_COUNT_MIB(tcp_stats.tcpInSegs, 1);

	/* ヘッダ長をチェックする。*/
	if (input->len < IF_IP_TCP_HDR_SIZE(input)) {
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_BAD_HEADERS], 1);
		goto drop;
		}

	tcph = GET_TCP_HDR(input, *offp);

	seglen  = input->len - *offp;				/* TCP のセグメント長 */

	if (IN_CKSUM(input, IPPROTO_TCP, *offp, (uint_t)seglen) != 0) {
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_BAD_CKSUMS], 1);
		goto drop;
		}

	/* TCP ヘッダ長をチェックする。*/
	if (TCP_HDR_LEN(tcph->doff) < TCP_HDR_SIZE || TCP_HDR_LEN(tcph->doff) > seglen) {
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_BAD_HEADERS], 1);
		goto drop;
		}
	tcph->sum = seglen - TCP_HDR_LEN(tcph->doff);		/* ここから tcph->sum は TCP の SDU 長 */

	/*
	 *  SYN と FIN の両ビットがセットされていれば破棄する。nmap 等の対策
	 *  ただし、RFC1644 T/TCP 拡張機能と競合する。
	 */
	if ((tcph->flags & (TCP_FLG_SYN | TCP_FLG_FIN)) == (TCP_FLG_SYN | TCP_FLG_FIN))
		goto drop;

	/* ネットワークオーダーからホストオーダーに変換する。*/

	NTOHL(tcph->seq);
	NTOHL(tcph->ack);
	NTOHS(tcph->win);
	NTOHS(tcph->urp);
	NTOHS(tcph->sport);
	NTOHS(tcph->dport);

	/* SDU 長 より 緊急ポインタが大きい場合 */
	if (tcph->urp > tcph->sum)
		goto drop;

find_cep:

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	/*
	 *  状態が Time Wait 中の CEP を探索する。
	 */
	twcep = tcp_find_twcep(input, *offp);
	if (twcep != NULL) {

		if (tcph->flags & TCP_FLG_RST)		/* RST フラグを受信したときは無視する。*/
			goto drop;
		else {

			/*
			 *    TCP 通信端点が Time Wait の時、相手ホストからセグメントが来たときは、
			 *    相手ホストの FIN に対する自ホストの ACK セグメントが途中で
			 *    損失したことを意味しているので、ACK セグメントを再送する。
			 */

			/* ホストオーダーからネットワークオーダーに戻す。*/
			HTONS(tcph->sport);
			HTONS(tcph->dport);

			tcp_respond(input, NULL, twcep->rcv_nxt, twcep->snd_una, twcep->rbufsz - twcep->rwbuf_count, TCP_FLG_ACK);
			}
		return IPPROTO_DONE;
		}
	else
		/* 標準の TCP 通信端点を得る。*/
		cep = tcp_find_cep(input, *offp);

#else	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	/* TCP 通信端点を得る。*/
	cep = tcp_find_cep(input, *offp);

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	/*
	 *  TCP 通信端点がない場合と CEP の状態がクローズなら破棄する。
	 */
	if (cep == NULL) {
		syslog(LOG_INFO, "[TCP] unexp port: %d.", tcph->dport);
		goto reset_drop;
		}

#ifdef TCP_CFG_TRACE

	tcp_input_trace(input, cep);

#endif	/* of #ifdef TCP_CFG_TRACE */


	if (cep->fsm_state == TCP_FSM_CLOSED)
		goto drop;

	/*
	 *  コネクション開設済みでセグメントを受信したときは、
	 *  アイドル時間と生存確認タイマをリセットする。
	 */
	cep->idle = 0;
	if (TCP_FSM_HAVE_ESTABLISHED(cep->fsm_state)) {
		cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_IDLE;
		}

	/* CEP の状態が LISTEN 以外の時は、オプションを処理する。*/
	if (cep->fsm_state != TCP_FSM_LISTEN)
		parse_option(tcph, cep);

	/*
	 *  受信可能ウィンドサイズを計算する。
	 *
	 *  rcv_nxt:     受信を期待している最小の SEQ（これ以前は受信済み）
	 *  rcv_adv:     受信を期待している最大の SEQ
	 *  rbufsz:      受信ウィンドバッファサイズ
	 *  rwbuf_count:  受信ウィンドバッファにあるデータ量
	 *  tcph->sum:   今回受信した SDU サイズ
	 *
	 *  今回受信したセグメントを順序整列キューに連結する
	 *  可能性があるので tcph->sum を考慮する。
	 *
	 */
	win = cep->rbufsz - (cep->rwbuf_count + tcph->sum);
	if (win < 0)
		win = 0;
	if (win > (int32_t)(cep->rcv_adv - cep->rcv_nxt))
		cep->rcv_wnd = win;
	else
		cep->rcv_wnd = cep->rcv_adv - cep->rcv_nxt;

	/* CEP の状態により処理を行う。*/

	if (cep->fsm_state == TCP_FSM_LISTEN) {		/* 受動オープン (LISTEN) の処理。*/
		if ((ret = listening(input, cep, *offp, iss)) == RET_RST_DROP)
			goto reset_drop;
		else if (ret == RET_DROP)
			goto drop;
		trim_length(tcph, cep);			/* 受信した SDU 長を調整する。*/

		if (tcph->flags & TCP_FLG_ACK) {	/* ACK フラグの処理 */
			if ((ret = proc_ack2(input, cep, *offp, &needoutput)) == RET_DROP)
				goto drop;
			else if (ret == RET_RST_DROP)
				goto reset_drop;
			else if (ret == RET_RETURN)
				return IPPROTO_DONE;
			}
		}
	else if (cep->fsm_state == TCP_FSM_SYN_SENT) {	/* 能動オープン、SYN 送信済み	*/
		if ((ret = syn_sent(tcph, cep)) == RET_RST_DROP)
			goto reset_drop;
		else if (ret == RET_DROP)
			goto drop;
		trim_length(tcph, cep);			/* 受信した SDU 長を調整する。*/

		if (tcph->flags & TCP_FLG_ACK) {	/* ACK フラグの処理 */
			if ((ret = proc_ack2(input, cep, *offp, &needoutput)) == RET_DROP)
				goto drop;
			else if (ret == RET_RST_DROP)
				goto reset_drop;
			else if (ret == RET_RETURN)
				return IPPROTO_DONE;
			}
		}
	else {
		if (cep->fsm_state == TCP_FSM_SYN_RECVD) {	/* SYN を受信、SYN 送信済み	*/
			/*
			 *  相手から受信確認が送られて来ても、
			 *
			 *    ACK <= 未確認の最小送信 SEQ (snd_una) &&
			 *           送信した最大     SEQ (snd_max) < ACK
			 *
			 *  なら、リセットを送ってセグメントを破棄する。
			 */
			if ((tcph->flags & TCP_FLG_ACK) &&
			    (SEQ_LE(tcph->ack, cep->snd_una) ||
			     SEQ_GT(tcph->ack, cep->snd_max)))
			     	goto reset_drop;
			}

		/* 
		 *  RST フラグを受信したときの処理 (異常切断)
		 */
		if (tcph->flags & TCP_FLG_RST) {
			if (SEQ_GE(tcph->seq, cep->last_ack_sent) &&
			    SEQ_LT(tcph->seq, cep->last_ack_sent + cep->rcv_wnd)) {
				/*
				 *  受信したセグメントの SEQ が、最後に送信した ACK (last_ack_sent)
				 *  から、受信ウインドウサイズまでの間の処理
				 */
				switch (cep->fsm_state) {
				case TCP_FSM_SYN_RECVD:		/* SYN を受信し、SYN 送信済み		*/

					cep->net_error = EV_CNNRF;	/* 接続不能 */
					cep->error     = E_CLS;
					NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_RSTS], 1);
					NET_COUNT_MIB(tcp_stats.tcpAttemptFails, 1);
					cep = tcp_close(cep);
					break;

				case TCP_FSM_ESTABLISHED:	/* コネクション開設完了			*/
				case TCP_FSM_CLOSE_WAIT:	/* FIN 受信、クローズ待ち		*/
					NET_COUNT_MIB(tcp_stats.tcpEstabResets, 1);
					/* fallthrough */

				case TCP_FSM_FIN_WAIT_1:	/* 終了して、FIN 送信済み		*/
				case TCP_FSM_FIN_WAIT_2:	/* 終了、FIN 伝達確認受信、FIN待ち	*/

					cep->net_error = EV_CNRST;	/* 接続リセット */
					cep->error     = E_CLS;
					NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_RSTS], 1);
					/* no break; */

				case TCP_FSM_CLOSING:		/* 終了、FIN 交換済み、ACK 待ち	*/
				case TCP_FSM_LAST_ACK:		/* FIN 受信、終了、ACK 待ち	*/

					cep = tcp_close(cep);
					break;
					}
				}
			goto drop;
			}

		/*
		 *  CEP の状態が SYN を受信し、SYN 送信済みの場合は、
		 *  受信ウィンドに収まるようにデータを
		 *  調整する前に、この接続によるパケットかどうかを検証する。
		 *
		 *    受信した相手の SEQ < 相手の SEQ の初期値 (irs)
		 *
		 *  これは、"LAND" DoS 攻撃の防御である。
		 */
		if (cep->fsm_state == TCP_FSM_SYN_RECVD && SEQ_LT(tcph->seq, cep->irs)) {
			goto reset_drop;
			}

		/*
		 *  受信を期待している最小の SEQ (rcv_nxt) - 受信した相手の SEQ が
		 *  正なら、rcv_nxt 以前のデータはすでに受信しているので、その部分を
		 *  削除する。
		 *                           <---------- rcv_wnd --------->
		 *                           rcv_nxt                      rcv_nxt + rcv_wnd
		 *                           v                            v
		 *                      -----+----------------------------+-----
		 *                           |                            |
		 *                      -----+----------------------------+-----
		 *           +----------------------+
		 *           |***************|      |
		 *           +----------------------+
		 *           ^                      ^
		 *           seq                    seq + len
		 *           <---------------> 削除する。
		 */
		todrop = cep->rcv_nxt - tcph->seq;
		if (todrop > 0) {

			/*
			 *  SYN フラグがついているときは、その分 (1 オクテット)
			 *  SEQ を進め、緊急ポインタと削除する長さを調整する。
			 */
			if (tcph->flags & TCP_FLG_SYN) {
				tcph->flags &= ~TCP_FLG_SYN;
				tcph->seq ++;
				if (tcph->urp > 1)
					tcph->urp --;
				else
					tcph->flags &= ~TCP_FLG_URG;
				todrop --;
				}

			/*
			 *  削除する長さが SDU より長い、つまり、受信を期待している
			 *  最小の SEQ (rcv_nxt) に達していないか、
			 *  削除する長さが SDU と同じで、FIN フラグがついてなければ
			 *  全て削除する。
			 */
			if ( todrop >  tcph->sum ||		/* tcph->sum は TCP の SDU 長 */
			    (todrop == tcph->sum && (tcph->flags & TCP_FLG_FIN) == 0)) {
				tcph->flags &= ~TCP_FLG_FIN;
				cep->flags |= TCP_CEP_FLG_ACK_NOW;
				todrop = tcph->sum;		/* tcph->sum は TCP の SDU 長 */
				}

			/*
			 *  SDU を前に詰める。
			 */
			if (todrop < tcph->sum) {		/* tcph->sum は TCP の SDU 長 */
				memcpy(GET_TCP_SDU(input, *offp),
				       GET_TCP_SDU(input, *offp) + todrop, (size_t)(tcph->sum - todrop));
				}

			/*
			 *  SEQ と SDU 長を調整する。
			 */
			tcph->seq +=     todrop;
			tcph->sum -= (uint16_t)todrop;	/* tcph->sum は TCP の SDU 長 */

			/*
			 *  緊急ポインタを調整する。
			 */
			if (tcph->urp > todrop)
				tcph->urp -= (uint16_t)todrop;
			else {
				tcph->flags &= ~TCP_FLG_URG;
				tcph->urp = 0;
				}

			NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DUP_SEGS], 1);
			}

		/*  
		 *  もしユーザタスクが終了した後に、データを受信した
		 *  場合は、RST を送る。
		 */
		if (cep->fsm_state == TCP_FSM_LAST_ACK && tcph->sum > 0) {	/* tcph->sum は TCP の SDU 長 */
			cep = tcp_close(cep);
			goto reset_drop;
			}

		/*
		 *  受信セグメントが受信ウィンドを超える場合は、
		 *  超えた分を削る。
		 *
		 *       <---------- rcv_wnd --------->
		 *       rcv_nxt                      (rcv_nxt + rcv_wnd)
		 *       v                            v
		 *  -----+----------------------------+-----
		 *       |                            |
		 *  -----+----------------------------+-----
		 *                    +----------------------+
		 *                    |               |******|
		 *                    +----------------------+
		 *                    ^                      ^
		 *                    seq                    seq + len
		 *                                     <-----> 削除する。
		 */
		todrop = (tcph->seq + tcph->sum) - (cep->rcv_nxt + cep->rcv_wnd);	/* tcph->sum は TCP の SDU 長 */
		if (todrop > 0) {
			if (todrop > tcph->sum) {					/* tcph->sum は TCP の SDU 長 */
				/*
				 *  受信した SDU の全てが受信ウィンドを超える場合。
				 *
				 *  TIME_WAIT 中に、新たな接続要求を受信したら
				 *  古い接続を破棄し、新たな接続を開始する。
				 *  ただし、SEQ は前より進んでいなければならない。
				 */
				if ((tcph->flags & TCP_FLG_SYN) &&
				    cep->fsm_state == TCP_FSM_TIME_WAIT &&
				    SEQ_GT(tcph->seq, cep->rcv_nxt)) {
					iss = cep->snd_nxt + TCP_ISS_INCR();
					tcp_close(cep);
					syscall(dly_tsk(0));
					goto find_cep;
				    	}

				/*
				 *  受信ウィンドが 0 で、受信した SEQ と
				 *  受信を期待している最小の SEQ が一致したときは
				 *  ACK を返す。それ以外はデータを破棄し、ACK を返す。
				 */
				if (cep->rcv_wnd == 0 && (tcph->seq == cep->rcv_nxt || tcph->sum == 0)) {
					cep->flags |= TCP_CEP_FLG_ACK_NOW;
					}
				else if (drop_after_ack(input, cep, *offp) == RET_RST_DROP)
					goto reset_drop;
				else {
					return IPPROTO_DONE;
					}
				}
			tcph->sum -= (uint16_t)todrop;	/* tcph->sum は TCP の SDU 長 */
			tcph->flags &= ~(TCP_FLG_PUSH | TCP_FLG_FIN);
			}

		/*
		 *  もし、SYN がセットされていれば、
		 *  エラーなので RST を送り、接続を破棄する。
		 */
		if (tcph->flags & TCP_FLG_SYN) {
			cep->net_error = EV_CNRST;
			cep = tcp_drop(cep, E_CLS);
			goto reset_drop;
			}

		/*
		 *  もし、ACK がセットされていない場合は、
		 *  状態が SYN 受信済みか
		 *  SYN を送信しようとしていれば、処理を続けるが、
		 *  それ以外はセグメントを破棄して終了する。
		 */
		if ((tcph->flags & TCP_FLG_ACK) == 0) {
			if (!(cep->fsm_state == TCP_FSM_SYN_RECVD || (cep->flags & TCP_CEP_FLG_NEED_SYN)))
				goto drop;
			}
		else {
			/*
			 * ACK の処理
			 */
			ret = proc_ack1(input, cep, *offp, &needoutput);
			if (ret == RET_DROP)
				goto drop;
			else if (ret == RET_RST_DROP)
				goto reset_drop;
			else if (ret == RET_RETURN)
				return IPPROTO_DONE;
			}
		}

/* step 6 */

	/* 送信ウィンドを更新する。*/
	if (update_wnd(tcph, cep) == true)
		needoutput = true;

	/* 緊急データを処理する。*/
	proc_urg(tcph, cep);

/* do data */

	/*
	 *  SDU があるか、FIN を未受信の状態で、最初に FIN を受信したとき、
	 *  受信セグメントキューに net_buf を追加する。
	 *  それ以外の場合は、セグメントを破棄する。
	 */
	flags = tcph->flags;
	if ((tcph->sum > 0 || (flags & TCP_FLG_FIN)) &&		/* tcph->sum は TCP の SDU 長 */
	    TCP_FSM_HAVE_RCVD_FIN(cep->fsm_state) == 0) {
		flags = reassemble(input, cep, *offp, flags);
		}
	else {
		syscall(rel_net_buf(input));
		flags &= ~TCP_FLG_FIN;
		}

	/*
	 *  FIN を受信したらコネクションをクローズする。
	 */
	if (flags & TCP_FLG_FIN)
		close_connection(cep, &needoutput);

	/* 出力を行った後終了する。*/
	if (needoutput == true || (cep->flags & TCP_CEP_FLG_ACK_NOW)) {
		/* 送信を指示する。*/
		cep->flags |=  TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}

	return IPPROTO_DONE;

reset_drop:
	/*
	 *  RST 送信処理
	 */

	if ((tcph->flags & TCP_FLG_RST) || IN_IS_NET_ADDR_MULTICAST(input))
		goto drop;

	/* ホストオーダーからネットワークオーダーに戻す。*/

	HTONS(tcph->sport);
	HTONS(tcph->dport);

	if (cep == NULL)
		rbfree = 0;
	else
		rbfree = cep->rbufsz - cep->rwbuf_count;

	if (tcph->flags & TCP_FLG_ACK)
		tcp_respond(input, cep, 0, tcph->ack, rbfree, TCP_FLG_RST);
	else {
		if (tcph->flags & TCP_FLG_SYN)
			tcph->sum ++;		/* tcph->sum は SDU 長 */
		tcp_respond(input, cep, tcph->seq + tcph->sum, 0, rbfree, TCP_FLG_RST | TCP_FLG_ACK);
		}

	/* input は tcp_respoond で返却される。*/
	NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_RSTS], 1);
	NET_COUNT_MIB(tcp_stats.tcpOutRsts, 1);
	return IPPROTO_DONE;

drop:
	NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DROP_SEGS], 1);
	NET_COUNT_MIB(tcp_stats.tcpInErrs, 1);
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
	}

#endif	/* of #ifdef SUPPORT_TCP */
