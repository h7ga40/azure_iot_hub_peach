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
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Internet Initiative Japan.  The name of the
 * IIJ may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* 
 *  PPP 処理タスクの本体
 */

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_hdlc.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_ipv6cp.h>
#include <net/ppp_ipcp.h>
#include <net/ppp_ccp.h>
#include <net/ppp_upap.h>
#include <net/ppp_modem.h>

#ifdef SUPPORT_PPP

/*
 *  全域変数
 */

uint8_t	ppp_phase = PPP_PHASE_INITIALIZE;	/* リンクの状態		*/

/*
 *  プロトコル表
 */

T_PPP_PROTENT *protocols[] = {
	&lcp_protent,

#ifdef _IP6_CFG
	&ipv6cp_protent,
#endif	/* of #ifdef _IP6_CFG */

#ifdef _IP4_CFG
	&ipcp_protent,
#endif	/* of #ifdef _IP4_CFG */

#ifdef SUPPORT_CCP
	&ccp_protent,
#endif	/* of #ifdef SUPPORT_CCP */

#ifdef LCP_CFG_PAP
	&pap_protent,
#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_CHAP
	&chap_protent,
#endif	/* of #ifdef LCP_CFG_CHAP */

	NULL
	};

/*
 *  変数
 */

#ifdef PPP_IDLE_TIMEOUT

static bool_t idle = false;

#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

/*
 *  関数
 */

static void parse_input (T_NET_BUF *input);

#ifdef PPP_IDLE_TIMEOUT

static void idle_timeout (void *arg);

#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

/*
 *  PPP 入力の解析
 */

static void
parse_input(T_NET_BUF *input)
{
	T_PPP_PROTENT	*entry;
	int_t		ix;
	uint16_t	proto;

	NET_COUNT_PPP(net_count_ppp.in_octets,  input->len);
	NET_COUNT_PPP(net_count_ppp.in_packets, 1);

	/* フレーム長をチェックする */
	if (input->len < sizeof(T_PPP_HDR)) {	/* FreeBSD では PPP_HDRLEN は 4 */
		syslog(LOG_NOTICE, "[PPP] recv short frame.");
		goto buf_rel;
		}

	proto = ntohs(*GET_PPP_HDR(input));

	/* プロトコルが LCP 以外で、LCP がオープンされていなければエラー */
	if (proto != PPP_LCP && lcp_fsm.state != PPP_FSM_OPENED) {
		syslog(LOG_NOTICE, "[PPP] LCP not open.");
		goto buf_rel;
		}

	/* リンクが認証状態以前は、LCP、LQR、PAP、CHAP のみ受け付ける */
	if (ppp_phase <= PPP_PHASE_AUTHENTICATE &&
	    !(proto == PPP_LCP || proto == PPP_LQR ||
	      proto == PPP_PAP || proto == PPP_CHAP)) {
		syslog(LOG_NOTICE, "[PPP] disc proto: 0x%04x, phase: %d.", proto, ppp_phase);
		goto buf_rel;
		}

	/* 上位プロトコルの入力関数を呼出す */
	for (ix = 0; (entry = protocols[ix]) != NULL; ix ++) {
		if (entry->proto == proto && entry->input != NULL) {
			(*entry->input)(input);
			syscall(rel_net_buf(input));
			return;
			}
		if ((entry->proto & ~0x8000) == proto && entry->datainput != NULL) {
			(*entry->datainput)(input);
			return;
			}
		}

	syslog(LOG_INFO, "[PPP] unexp proto: 0x%04x.", proto);
	lcp_sprotrej(input);

buf_rel:
	NET_COUNT_PPP(net_count_ppp.in_err_packets, 1);
	syscall(rel_net_buf(input));
	}

/*
 *  ppp_output -- PPP インタフェースの出力関数
 */

ER
ppp_output (T_NET_BUF *output, TMO tmout)
{
	ER error = E_OK;

#ifdef PPP_CFG_MODEM

#if 0	/* 保留 */
	/* モデムの接続完了まで待つ。*/
	if ((error = wait_modem()) != E_OK)
		goto buf_ret;
#endif

#endif	/* of #ifdef PPP_CFG_MODEM */

#ifdef _IP6_CFG
	/* IPV6CP の接続完了まで待つ。*/
	if ((error = wait_ipv6cp()) != E_OK)
		goto buf_ret;
#endif	/* of #ifdef _IP6_CFG */

#ifdef _IP4_CFG
	/* IPCP の接続完了まで待つ。*/
	if ((error = wait_ipcp()) != E_OK)
		goto buf_ret;
#endif	/* of #ifdef _IP4_CFG */

#ifdef _IP4_CFG
	/* IPCP の接続完了まで待つ。*/
	if ((error = wait_ipcp()) != E_OK)
		goto buf_ret;
#endif	/* of #ifdef _IP4_CFG */

#ifdef PPP_IDLE_TIMEOUT
	wai_sem(SEM_IDLE_TIMEOUT);
	if (idle) {
		untimeout((FP)idle_timeout, NULL);
		idle = false;
		}
	sig_sem(SEM_IDLE_TIMEOUT);
#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

	/* PPP 出力キューに投入する。*/
	if ((error = tsnd_dtq(DTQ_PPP_OUTPUT, output, tmout)) != E_OK)
		goto buf_ret;

#ifdef PPP_IDLE_TIMEOUT
	wai_sem(SEM_IDLE_TIMEOUT);
	if (!idle && ppp_phase == PPP_PHASE_NETWORK) {
		timeout((FP)idle_timeout, NULL, PPP_IDLE_TIMEOUT);
		idle = true;
		}
	sig_sem(SEM_IDLE_TIMEOUT);
#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

	return error;

buf_ret:
	syscall(rel_net_buf(output));
	NET_COUNT_PPP(net_count_ppp.out_err_packets, 1);
	return error;
	}

/*
 *  ppp_cp_output -- CP フレームの送信
 */

void
ppp_cp_output (uint8_t code, uint8_t id, T_NET_BUF *output)
{
	T_PPP_CP_HDR	*cph;

	/* CP ヘッダを設定する */
	cph = GET_PPP_CP_HDR(output);
	cph->code = code;
	cph->id   = id;
	cph->len  = htons(output->len - sizeof(T_PPP_HDR));

	/* PPP 出力キューに投入する。*/
	if (snd_dtq(DTQ_PPP_OUTPUT, output) != E_OK) {
		syscall(rel_net_buf(output));
		NET_COUNT_PPP(net_count_ppp.out_err_packets, 1);
		}
	}

/*
 *  PPP 入力タスク
 */

void
ppp_input_task(intptr_t exinf)
{
	T_NET_BUF	*input;
	T_PPP_PROTENT	*proto;
	ID		tskid;
	int_t		ix;
	uint8_t		rcount = 0;

	/* ポートを初期設定する */
	syscall(serial_opn_por(HDLC_PORTID));
	syscall(serial_ctl_por(HDLC_PORTID, IOCTL_FCSND | IOCTL_FCRCV));

	get_tid(&tskid);

#ifdef PPP_CFG_MODEM

	init_modem();
	syslog(LOG_NOTICE, "[PPP INPUT:%d] started with modem on port %d.", tskid, HDLC_PORTID);

#else	/* of #ifdef PPP_CFG_MODEM */

	syslog(LOG_NOTICE, "[PPP INPUT:%d] started on port %d.", tskid, HDLC_PORTID);

#endif	/* of #ifdef PPP_CFG_MODEM */

	/* ネットワークタイマタスクを起動する */
	syscall(act_tsk(NET_TIMER_TASK));

	/* 上位プロトコルを初期化する */
	for (ix = 0; (proto = protocols[ix]) != NULL; ix ++)
		if (proto->init != NULL)
			(*proto->init)();

	lcp_lowerup();
	lcp_open(PPP_OPEN_PASSIVE);

	/* PPP 出力タスクを起動する */
	syscall(act_tsk(PPP_OUTPUT_TASK));

	/* 乱数生成を初期化する。*/
	net_srand(0);

	while (true) {
		if (tget_net_buf(&input, IF_PDU_SIZE, TMO_PPP_GET_NET_BUF) == E_OK) {
			while (HDLC_read(input, IF_PDU_SIZE) != E_OK)
				;
			if (input->len > 0) {

				/* 乱数生成を初期化する。*/
				if (rcount == 0)
					net_srand(input->len);
				rcount ++;

#ifdef PPP_IDLE_TIMEOUT
				wai_sem(SEM_IDLE_TIMEOUT);
				if (idle && ntohs(*GET_PPP_HDR(input)) != PPP_LCP) {
					untimeout((FP)idle_timeout, NULL);
					idle = false;
					}
				sig_sem(SEM_IDLE_TIMEOUT);
#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

				parse_input(input);
				}
			else
				syscall(rel_net_buf(input));

#ifdef PPP_IDLE_TIMEOUT
			wai_sem(SEM_IDLE_TIMEOUT);
			if (!idle && ppp_phase == PPP_PHASE_NETWORK) {
				timeout((FP)idle_timeout, NULL, PPP_IDLE_TIMEOUT);
				idle = true;
				}
			else if (idle && ppp_phase != PPP_PHASE_NETWORK) {
				untimeout((FP)idle_timeout, NULL);
				idle = false;
				}
			sig_sem(SEM_IDLE_TIMEOUT);
#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

			}
		else {
			HDLC_dummy_read();
			NET_COUNT_PPP(net_count_ppp.in_err_packets, 1);
			NET_COUNT_PPP(net_count_ppp_no_bufs, 1);
			}
		}
	}

/*
 *  PPP 出力タスク
 */

void
ppp_output_task(intptr_t exinf)
{
	T_NET_BUF	*output;
	ID		tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[PPP OUTPUT:%d] started.", tskid);

	while (true) {
		while (rcv_dtq(DTQ_PPP_OUTPUT, (intptr_t*)&output) == E_OK) {
			NET_COUNT_PPP(net_count_ppp.out_octets,  output->len);
			NET_COUNT_PPP(net_count_ppp.out_packets, 1);
			syscall(HDLC_write(output));
			syscall(rel_net_buf(output));
			}
		}
	}

#ifdef PPP_IDLE_TIMEOUT

/*
 *  アイドルタイムアウト処理
 */

static void
idle_timeout (void *arg)
{
	syslog(LOG_NOTICE, "[PPP] idel %d [s], disconnecting.", (uint16_t)(PPP_IDLE_TIMEOUT / NET_TIMER_HZ));
	lcp_close();
	}

#endif	/* of #ifdef PPP_IDLE_TIMEOUT */

#endif	/* fo #ifdef SUPPORT_PPP */
