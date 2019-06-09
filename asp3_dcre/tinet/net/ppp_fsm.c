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
 * fsm->c - {Link, IP} Control Protocol Finite State Machine.
 *
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *		PPP Finite State Machine for LCP/IPCP
 *
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Internet Initiative Japan, Inc.  The name of the
 * IIJ may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD: src/usr.sbin/ppp/fsm->c,v 1.52.2.3 2000/08/19 09:30:03 brian Exp $
 *
 *  TODO:
 */

#include <string.h>

#include <s_services.h>
#include <t_services.h>

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_timer.h>
#include <net/net_buf.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_hdlc.h>
#include <net/ppp_fsm.h>

#include <netinet/in.h>

#ifdef SUPPORT_PPP

/*
 *  全域変数
 */

uint8_t	ppp_open_mode = PPP_OPEN_PASSIVE;	/* オープン時の動作モード	*/

/*
 *  関数
 */

static void fsm_sconfreq	(T_PPP_FSM *fsm, int_t retansmit);
static void fsm_rconfreq	(T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input);
static void fsm_rconfack	(T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input);
static void fsm_rconfnakrej	(T_PPP_FSM *fsm, uint8_t code, uint8_t id, T_NET_BUF *input);
static void fsm_rtermreq	(T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input);
static void fsm_rtermack	(T_PPP_FSM *fsm, T_NET_BUF *input);
static void fsm_rcoderej	(T_PPP_FSM *fsm, T_NET_BUF *input);
static void fsm_stermreq	(T_PPP_FSM *fsm, uint8_t id);
static void fsm_timeout		(void *arg);

/*
 *  fsm_sconfreq -- Configure-Request を送信する。
 */

static void
fsm_sconfreq (T_PPP_FSM *fsm, int_t retransmit)
{
	T_NET_BUF	*output;
	uint16_t	len;

	if (!(fsm->state == PPP_FSM_REQSENT ||
	      fsm->state == PPP_FSM_ACKRCVD ||
	      fsm->state == PPP_FSM_ACKSENT)) {
		/* まだネゴをしていない。オプションをリセットする。 */
		if (fsm->callbacks->resetci)
			(*fsm->callbacks->resetci)(fsm);
		fsm->failures = 0;
		}
	if (retransmit == 0) {
		/* 新しいリクエスト。再送カウンタをリセットし、新しい ID を設定する。 */
		fsm->retrans = MAX_PPP_CONFIGURES;
		fsm->reqid   = ++ fsm->id;
		}

	fsm->seen_ack = 0;

	if (fsm->callbacks->cilen && fsm->callbacks->addci) {
		/* 構成情報パケットを作る。 */
		len = (*fsm->callbacks->cilen)(fsm);

		syscall(get_net_buf(&output, len + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
		(*fsm->callbacks->addci)(fsm, output);

		/* 構成情報パケットを送信する。 */
		IF_SET_PROTO(output, fsm->proto);
		ppp_cp_output(PPP_CONFREQ, fsm->reqid, output);

		-- fsm->retrans;
		timeout((FP)fsm_timeout, fsm, DEF_PPP_TIMEOUT);
		}
	}

/*
 *  fsm_rconfreq -- Configure-Request を受信したときの処理
 */

static void
fsm_rconfreq (T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input)
{
	T_NET_BUF	*output;
	uint16_t	code;

	switch (fsm->state) {
	case PPP_FSM_CLOSED:
		syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
		memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
		ppp_cp_output(PPP_TERMACK, id, output);
		return;
		break;

	case PPP_FSM_CLOSING:
	case PPP_FSM_STOPPING:
		return;
		break;

	case PPP_FSM_OPENED:
		/* 上位層を停止して、ネゴに移る。 */
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm_sconfreq(fsm, 0);		/* 最初の Configure-Request を送る。 */
		break;

	case PPP_FSM_STOPPED:
		/* 相手からネゴが開始された */
		fsm_sconfreq(fsm, 0);		/* 最初の Configure-Request を送る。 */
		fsm->state = PPP_FSM_REQSENT;
		break;

		}

	syscall(get_net_buf(&output, input->len));

	/* 上位プロトコルに要求された Configure-Request を送る。 */
	if (fsm->callbacks->reqci != NULL)	/* 構成情報をチェックする。 */
		code = (*fsm->callbacks->reqci)(fsm, input, output);
	else if (output->len > sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR))
		code = PPP_CONFREJ;
	else
		code = PPP_CONFACK;

	/* 相手に ACK, NAK または REJ を送信する。 */
	ppp_cp_output(code, id, output);

	if (code == PPP_CONFACK) {
		if (fsm->state == PPP_FSM_ACKRCVD) {
			untimeout((FP)fsm_timeout, fsm);
			fsm->state = PPP_FSM_OPENED;
			if (fsm->callbacks->up != NULL)
				(*fsm->callbacks->up)(fsm);
			}
		else
			fsm->state = PPP_FSM_ACKSENT;
		fsm->failures = 0;
		}
	else {
		/* ACK か REJ を送る */
		if (fsm->state != PPP_FSM_ACKRCVD)
			fsm->state = PPP_FSM_REQSENT;
		if (code == PPP_CONFNAK)
			fsm->failures ++;
		}
	}

/*
 *  fsm_rconfnakrej -- Configure-NAK/REJ を受信したときの処理
 */

static void
fsm_rconfnakrej (T_PPP_FSM *fsm, uint8_t code, uint8_t id, T_NET_BUF *input)
{
	int_t		ret = 0;
	T_NET_BUF	*output;

	/* 予期した ID でないか、NAK/REJ を予期していないときは終了する。*/
	if (id != fsm->reqid || fsm->seen_ack) {
		syslog(LOG_WARNING, "[PPP/FSM] bad ID or unexp NAK/REJ.");
		return;
		}

	if (code == PPP_CONFNAK) {
		if (fsm->callbacks->nakci != NULL && (ret = (*fsm->callbacks->nakci)(fsm, input)) == 0) {
			syslog(LOG_WARNING, "[PPP/FSM] bad NAK.");
			return;
			}
		}
	else {
		if (fsm->callbacks->rejci != NULL && (ret = (*fsm->callbacks->rejci)(fsm, input)) == 0) {
			syslog(LOG_WARNING, "[PPP/FSM] bad REJ.");
			return;
			}
		}

	fsm->seen_ack = 1;

	switch (fsm->state) {
	case PPP_FSM_CLOSED:
	case PPP_FSM_STOPPED:
		syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
		memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
		ppp_cp_output(PPP_TERMACK, id, output);
		return;
		break;

	case PPP_FSM_REQSENT:
	case PPP_FSM_ACKSENT:
		untimeout((FP)fsm_timeout, fsm);
		if (ret < 0)		/* 送信した Conf-Req が破損し空の NAK/REJ が届いた。*/
			fsm->state = PPP_FSM_STOPPED;
		else
			fsm_sconfreq(fsm, 0);
		break;

	case PPP_FSM_ACKRCVD:
		untimeout((FP)fsm_timeout, fsm);
		fsm_sconfreq(fsm, 0);
		fsm->state = PPP_FSM_REQSENT;
		break;

	case PPP_FSM_OPENED:
		/* 再起動し、ネゴをやり直す。*/
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm_sconfreq(fsm, 0);
		fsm->state = PPP_FSM_REQSENT;
		break;
		}
	}

/*
 *  fsm_rconfack -- Configure-ACK を受信したときの処理
 */

static void
fsm_rconfack (T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input)
{
	T_NET_BUF	*output;

	/* 予期した ID でないか、ACK を予期していないときは終了する。*/
	if (id != fsm->reqid || fsm->seen_ack) {
		syslog(LOG_WARNING, "[PPP/FSM] bad ID or unexp ACK.");
		return;
		}

	if (fsm->callbacks->ackci != NULL) {
		if ((*fsm->callbacks->ackci)(fsm, input) != true) {
			syslog(LOG_WARNING, "[PPP/FSM] bad ACK.");
			return;
			}
		}
	else if (input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)) != 0) {
		syslog(LOG_WARNING, "[PPP/FSM] bad ACK.");
		return;
		}

	fsm->seen_ack = 1;

	switch (fsm->state) {
	case PPP_FSM_CLOSED:
	case PPP_FSM_STOPPED:
		syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
		memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
		ppp_cp_output(PPP_TERMACK, id, output);
		break;

	case PPP_FSM_REQSENT:
		fsm->state = PPP_FSM_ACKRCVD;
		fsm->retrans = MAX_PPP_CONFIGURES;
		break;

	case PPP_FSM_ACKRCVD:
		untimeout((FP)fsm_timeout, fsm);
		fsm_sconfreq(fsm, 0);
		fsm->state = PPP_FSM_REQSENT;
		break;

	case PPP_FSM_ACKSENT:
		untimeout((FP)fsm_timeout, fsm);
		fsm->state = PPP_FSM_OPENED;
		fsm->retrans = MAX_PPP_CONFIGURES;
		if (fsm->callbacks->up != NULL)
			(*fsm->callbacks->up)(fsm);
		break;

	case PPP_FSM_OPENED:
		/* 再起動し、ネゴをやり直す。*/
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm_sconfreq(fsm, 0);
		fsm->state = PPP_FSM_REQSENT;
		break;

	default	:
		syslog(LOG_WARNING, "[PPP/FSM] unexp state: %d in recv Conf-ACK.", fsm->state);
		break;
		}

	}

/*
 *  fsm_rtermreq -- Terminate-Request を受信したときの処理
 */

static void
fsm_rtermreq (T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input)
{
	T_NET_BUF	*output;

	switch (fsm->state) {
	case PPP_FSM_ACKRCVD:
	case PPP_FSM_ACKSENT:
		fsm->state = PPP_FSM_REQSENT;
		break;

	case PPP_FSM_OPENED:
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm->retrans = 0;
		fsm->state = PPP_FSM_STOPPING;
		timeout((FP)fsm_timeout, fsm, DEF_PPP_TIMEOUT);
		break;
		}
	syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
	memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
	ppp_cp_output(PPP_TERMACK, id, output);
	}

/*
 *  fsm_rtermack -- Terminate-ACK を受信したときの処理
 */

static void
fsm_rtermack (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	switch (fsm->state) {
	case PPP_FSM_CLOSING:
		untimeout((FP)fsm_timeout, fsm);
#if 0	/* 本来の動作は CLOSED に遷移する。*/
		fsm->state = PPP_FSM_CLOSED;
#else	/* of #if 0 */
		fsm->state = PPP_FSM_STOPPED;
#endif	/* of #if 0 */
		if (fsm->callbacks->finished != NULL)
			(*fsm->callbacks->finished)(fsm);
		break;

	case PPP_FSM_STOPPING:
		untimeout((FP)fsm_timeout, fsm);
		fsm->state = PPP_FSM_STOPPED;
		if (fsm->callbacks->finished != NULL)
			(*fsm->callbacks->finished)(fsm);
		break;

	case PPP_FSM_ACKRCVD:
		fsm->state = PPP_FSM_REQSENT;
		break;

	case PPP_FSM_OPENED:
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm_sconfreq(fsm, 0);
		break;
		}
	}

/*
 *  fsm_rcoderej -- Code-REJ を受信したときの処理
 */

static void
fsm_rcoderej (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_PPP_CP_HDR	*hdr;

 	/* PPP リンク制御 (CP) ヘッダより短ければエラー */
 	if (input->len < sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)) {
 		syslog(LOG_WARNING, "[PPP/FSM] bad len: %d in recv Code-REJ.", input->len);
 		return;
 		}

 	hdr = GET_PPP_CP_HDR(input);

	syslog(LOG_WARNING, "[PPP/FSM] recv Code-REJ, code=%d, id=%d.", hdr->code, hdr->id);

	if (fsm->state == PPP_FSM_ACKRCVD)
		fsm->state = PPP_FSM_REQSENT;
	}

/*
 *  fsm_timeout -- タイムアウト処理
 */

static void
fsm_timeout (void *arg)
{
	T_PPP_FSM *fsm = (T_PPP_FSM*)arg;

	switch (fsm->state) {
	case PPP_FSM_CLOSING:
	case PPP_FSM_STOPPING:
		if (fsm->retrans <= 0) {
			/* 最大再送回数を超えたときの処理 */
#if 0	/* 本来の動作は CLOSED に遷移する。*/
			fsm->state = (fsm->state == PPP_FSM_CLOSING) ? PPP_FSM_CLOSED : PPP_FSM_STOPPED;
#else	/* of #if 0 */
			fsm->state = PPP_FSM_STOPPED;
#endif	/* of #if 0 */
			if (fsm->callbacks->finished != NULL)
				(*fsm->callbacks->finished)(fsm);
			}
		else {
			/* Terminate-Request を送信する。*/
			fsm_stermreq(fsm, ++ fsm->id);
			}
		break;
	case PPP_FSM_REQSENT:
	case PPP_FSM_ACKRCVD:
	case PPP_FSM_ACKSENT:
		if (fsm->retrans <= 0) {
			/* 最大再送回数を超えたときの処理 */
			syslog(LOG_WARNING, "[PPP/FSM] retrans Conf-Req.");
			fsm->state = PPP_FSM_STOPPED;
			if (ppp_open_mode != PPP_OPEN_PASSIVE && fsm->callbacks->finished)
				(*fsm->callbacks->finished)(fsm);
			}
		else {
			/* Configure-Request を再送する。*/
			if (fsm->callbacks->retrans != NULL)
				(*fsm->callbacks->retrans)(fsm);
			fsm_sconfreq(fsm, 1);
			if (fsm->state == PPP_FSM_ACKRCVD)
				fsm->state = PPP_FSM_REQSENT;
			}

		break;

	default	:
		syslog(LOG_WARNING, "[PPP/FSM] unexp state: %d in timeout.", fsm->state);
		break;
		}
	}

/*
 *  fsm_stermreq -- Terminate-Request を送信する。
 */

static void
fsm_stermreq (T_PPP_FSM *fsm, uint8_t id)
{
	T_NET_BUF	*input;

	syscall(get_net_buf(&input, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
	IF_SET_PROTO(input, fsm->proto);
	ppp_cp_output(PPP_TERMREQ, id, input);

	timeout((FP)fsm_timeout, fsm, DEF_PPP_TIMEOUT);
	fsm->retrans --;
	}

/*
 *  fsm_init -- FSM モジュールの初期化
 */

void
fsm_init (T_PPP_FSM *fsm)
{
	fsm->state = PPP_FSM_INITIAL;
	fsm->id	   = 0;
	}

/*
 *  fsm_lowerup -- FSM 下位層を起動する。
 */

void
fsm_lowerup (T_PPP_FSM *fsm)
{
	switch (fsm->state) {
	case PPP_FSM_INITIAL:
		fsm->state = PPP_FSM_CLOSED;
		break;

	case PPP_FSM_STARTING:
		if (ppp_open_mode == PPP_OPEN_PASSIVE)
			fsm->state = PPP_FSM_STOPPED;
        	else {
			fsm_sconfreq(fsm, 0);
			fsm->state = PPP_FSM_REQSENT;
			}
		break;

	default	:
		syslog(LOG_WARNING, "[PPP/FSM] unexp state: %d in lowerup", fsm->state);
		break;
		}
	}

/*
 *  fsm_lowerdown -- FSM 下位層を停止する。
 */

void
fsm_lowerdown (T_PPP_FSM *fsm)
{
	switch (fsm->state) {
	case PPP_FSM_CLOSED:
		fsm->state = PPP_FSM_INITIAL;
		break;

	case PPP_FSM_STOPPED:
		fsm->state = PPP_FSM_STARTING;
		if (fsm->callbacks->starting != NULL)
			(*fsm->callbacks->starting)(fsm);
		break;

	case PPP_FSM_CLOSING:
		fsm->state = PPP_FSM_INITIAL;
		untimeout((FP)fsm_timeout, fsm);
		break;

	case PPP_FSM_STOPPING:
	case PPP_FSM_REQSENT:
	case PPP_FSM_ACKRCVD:
	case PPP_FSM_ACKSENT:
		fsm->state = PPP_FSM_STARTING;
		untimeout((FP)fsm_timeout, fsm);
		break;

	case PPP_FSM_OPENED:
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);
		fsm->state = PPP_FSM_STARTING;
		break;

	default	:
		syslog(LOG_WARNING, "[PPP/FSM] unexp state: %d in lowerdown", fsm->state);
		break;
		}
	}

/*
 *  fsm_open -- FSM のオープン
 */

void
fsm_open (T_PPP_FSM *fsm)
{
	switch (fsm->state) {
	case PPP_FSM_INITIAL:
		fsm->state = PPP_FSM_STARTING;
		if (fsm->callbacks->starting != NULL)
			(*fsm->callbacks->starting)(fsm);
		break;

	case PPP_FSM_CLOSED:
		if (ppp_open_mode == PPP_OPEN_PASSIVE)
			fsm->state = PPP_FSM_STOPPED;
		else {
			fsm_sconfreq(fsm, 0);
			fsm->state = PPP_FSM_REQSENT;
			}
		break;

	case PPP_FSM_CLOSING:
		fsm->state = PPP_FSM_STOPPING;
		/* nobreak;	下に落ちる */

	case PPP_FSM_STOPPED:
	case PPP_FSM_OPENED:
		fsm_lowerdown(fsm);
		fsm_lowerup(fsm);
		break;
		}
	}

/*
 *  fsm_close -- リンクの切断を開始する。
 */

void
fsm_close (T_PPP_FSM *fsm)
{
	switch (fsm->state) {
	case PPP_FSM_STARTING:
		fsm->state = PPP_FSM_INITIAL;
		break;

#if 0	/* 本来の動作は CLOSED に遷移する。*/
	case PPP_FSM_STOPPED:
		fsm->state = PPP_FSM_CLOSED;
		break;
#endif	/* of #if 0 */

	case PPP_FSM_STOPPING:
		fsm->state = PPP_FSM_CLOSING;
		break;

	case PPP_FSM_REQSENT:
	case PPP_FSM_ACKRCVD:
	case PPP_FSM_ACKSENT:
	case PPP_FSM_OPENED:
		if (fsm->state != PPP_FSM_OPENED)
			untimeout((FP)fsm_timeout, fsm);
		else if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);

		/* タイムアウトを設定し、Terminate-Request を送信する。*/
		fsm->retrans = MAX_PPP_TERMINATES;
		fsm_stermreq(fsm, fsm->reqid = ++ fsm->id);
		fsm->state = PPP_FSM_CLOSING;
		break;
		}
	}

/*
 *  fsm_input -- 入力、PPP リンク制御 (CP) の制御コードにより適当な関数を呼出す
 */

void
fsm_input (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_PPP_CP_HDR	*hdr;
	T_NET_BUF	*output;

 	/* PPP リンク制御 (CP) ヘッダより短ければエラー */
 	if (input->len < (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR))) {
 		syslog(LOG_WARNING, "[PPP/FSM] short hdr: %d.", input->len);
 		return;
 		}

 	hdr = GET_PPP_CP_HDR(input);

 	/* ヘッダの長さと入力データサイズが一致しなければエラー */
 	if (hdr->len != input->len - sizeof(T_PPP_HDR)) {
 		syslog(LOG_WARNING, "[PPP/FSM] bad len: %d.", hdr->len);
 		return;
 		}
 
 	/* 状態が INITIAL か STARTING ならエラー */
 	if (fsm->state == PPP_FSM_INITIAL || fsm->state == PPP_FSM_STARTING) {
 		syslog(LOG_WARNING, "[PPP/FSM] unexp state: %d in input.", fsm->state);
 		return;
 		}
 
 	/*
 	 *  制御コードにより適当な関数を呼出す
 	 */

	switch (hdr->code) {
	case PPP_CONFREQ:
		fsm_rconfreq(fsm, hdr->id, input);
		break;

	case PPP_CONFACK:
		fsm_rconfack(fsm, hdr->id, input);
		break;

	case PPP_CONFNAK:
	case PPP_CONFREJ:
		fsm_rconfnakrej(fsm, hdr->code, hdr->id, input);
		break;

	case PPP_TERMREQ:
		fsm_rtermreq(fsm, hdr->id, input);
		break;

	case PPP_TERMACK:
		fsm_rtermack(fsm, input);
		break;

	case PPP_CODEREJ:
		fsm_rcoderej(fsm, input);
		break;

	default:
		if (fsm->callbacks->extcode == NULL ||
		    (*fsm->callbacks->extcode)(fsm, hdr->code, hdr->id, input) != true) {
			syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
			memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
			ppp_cp_output(PPP_CODEREJ, ++ fsm->id, output);
			}
		break;
		}
	}

/*
 *  fsm_protoreject -- Proto-REJ を受信したときの処理
 */

void
fsm_protreject (T_PPP_FSM *fsm)
{
	switch (fsm->state) {
	case PPP_FSM_CLOSING:
		untimeout((FP)fsm_timeout, fsm);

#if 0	/* 本来の動作は CLOSED に遷移する。*/
		/* 下に落ちる */
#else	/* of #if 0 */
		fsm->state = PPP_FSM_STOPPED;
		if (fsm->callbacks->finished != NULL)
			(*fsm->callbacks->finished)(fsm);
		break;
#endif	/* of #if 0 */

	case PPP_FSM_CLOSED:
		fsm->state = PPP_FSM_CLOSED;
		if (fsm->callbacks->finished != NULL)
			(*fsm->callbacks->finished)(fsm);
		break;

	case PPP_FSM_STOPPING:
	case PPP_FSM_REQSENT:
	case PPP_FSM_ACKRCVD:
	case PPP_FSM_ACKSENT:
		untimeout((FP)fsm_timeout, fsm);
		/* 下に落ちる */

	case PPP_FSM_STOPPED:
		fsm->state = PPP_FSM_STOPPED;
		if (fsm->callbacks->finished != NULL)
			(*fsm->callbacks->finished)(fsm);
		break;

	case PPP_FSM_OPENED:
		if (fsm->callbacks->down != NULL)
			(*fsm->callbacks->down)(fsm);

		/* タイムアウトを設定し、Terminate-Request を送信する。*/
		fsm->retrans = MAX_PPP_TERMINATES;
		fsm_stermreq(fsm, fsm->reqid = ++ fsm->id);
		fsm->state = PPP_FSM_STOPPING;
		break;

		}
	}

#endif	/* fo #ifdef SUPPORT_PPP */
