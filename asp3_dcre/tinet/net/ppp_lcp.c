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
 * lcp.c - PPP Link Control Protocol.
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
 *	      PPP Link Control Protocol (LCP) Module
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
 * $FreeBSD: src/usr.sbin/ppp/lcp.c,v 1.81.2.3 2000/08/19 09:30:04 brian Exp $
 *
 */

#include <string.h>

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
#include <net/ppp_auth.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>

#ifdef SUPPORT_PPP

/*
 *  関数
 */

static void lcp_init    (void);
static void lcp_protrej (void);
static void lcp_input   (T_NET_BUF *input);

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

static void send_echo_req  (T_PPP_FSM *fsm);
static void recv_echo_rep  (T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input);
static void echo_timeout   (void *arg);
static void echo_lowerup   (void);
static void echo_lowerdown (void);

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

/*
 * FSM から呼出されるコールバック関数
 */

static void  lcp_resetci(T_PPP_FSM *fsm);	/* 自構成情報をリセットする。	*/
static int_t lcp_cilen	(T_PPP_FSM *fsm);	/* 構成情報の長さを返す。		*/
static void  lcp_addci	(T_PPP_FSM *fsm, T_NET_BUF *output);
						/* 自構成情報を追加する。		*/
static bool_t lcp_ackci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* ACK を受信したときの処理	*/
static bool_t lcp_nakci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* NAK を受信したときの処理	*/
static bool_t lcp_rejci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* REJ を受信したときの処理	*/
static int_t lcp_reqci	(T_PPP_FSM *fsm, T_NET_BUF *input, T_NET_BUF *output);
						/* 相手の構成情報を確認する。	*/
static void lcp_up	(T_PPP_FSM *fsm);	/* リンク接続を確立する。		*/
static void lcp_starting(T_PPP_FSM *fsm);	/* 下位層を開始する。		*/
static bool_t lcp_extcode	(T_PPP_FSM *fsm, uint8_t code, uint8_t id, T_NET_BUF *input);
						/* 不明な CP を受信したときの処理	*/
static void lcp_rprotrej(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* Proto-REJ を受信したときの処理	*/
/*
 *  変数
 */

static T_LCP_CONFIGS lcp_def_cfg = {		/* 構成情報の初期設定値		*/
	DEF_REMOTE_ACCM,			/* ACCM				*/

#ifdef LCP_CFG_MAGIC

	ULONG_C(0),				/* マジック番号			*/

#endif	/* of #ifdef LCP_CFG_MAGIC */

	DEF_PPP_MTU,				/* MRU				*/
	UINT_C(0),				/* オプションフラグ		*/
	};

static T_LCP_CONFIGS lcp_local_def_cfg = {	/* 自構成情報の規定値		*/
	DEF_LOCAL_ACCM,				/* ACCM				*/

#ifdef LCP_CFG_MAGIC

	ULONG_C(0),				/* マジック番号			*/

#endif	/* of #ifdef LCP_CFG_MAGIC */

	DEF_PPP_MTU,				/* MRU				*/
	DEF_LCP_LOCAL_CFGS,			/* オプションフラグ		*/
	};

static T_LCP_CONFIGS lcp_local_neg_cfg;		/* ネゴ中の自構成情報		*/

static T_LCP_CONFIGS lcp_remote_neg_cfg;	/* ネゴ中の相手の構成情報		*/

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

static uint8_t echo_pending  = UINT_C(0);
static uint8_t echo_number   = UINT_C(0);
static uint8_t timer_running = false;

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

/*
 *  全域変数
 */

T_PPP_FSM_CALLBACKS lcp_callbacks = {
	lcp_resetci,			/* 自構成情報をリセットする。	*/
	lcp_cilen,			/* 構成情報の長さを返す。		*/
	lcp_addci,			/* 自構成情報を追加する。		*/
	lcp_ackci,			/* ACK を受信したときの処理	*/
	lcp_nakci,			/* NAK を受信したときの処理	*/
	lcp_rejci,			/* REJ を受信したときの処理	*/
	lcp_reqci,			/* 相手の構成情報を確認する。	*/
	lcp_up,				/* リンク接続を確立する。		*/
	lcp_down,			/* リンク接続を解放する。		*/
	lcp_starting,			/* 下位層を開始する。		*/
	lcp_finished,			/* 下位層を終了する。		*/
	NULL,				/* 再送する。			*/
	lcp_extcode,			/* 不明な CP を受信したときの処理	*/
	};

T_PPP_FSM lcp_fsm = {
	&lcp_callbacks,			/* コールバック関数		*/
	PPP_LCP,			/* プロトコル			*/
	};

T_PPP_PROTENT lcp_protent = {
	PPP_LCP,
	lcp_init,			/* 初期化			*/
	lcp_input,			/* 入力				*/
	lcp_protrej,			/* Proto-REJ 受信処理		*/
	NULL,				/* 下位層を起動する		*/
	lcp_lowerdown,			/* 下位層を停止する		*/
	NULL,				/* オープンする			*/
	lcp_close,			/* クローズする			*/
	NULL,				/* データ入力			*/
	};

T_LCP_CONFIGS lcp_local_ack_cfg;	/* 相手に許可されたの自構成情報	*/
T_LCP_CONFIGS lcp_remote_ack_cfg;	/* 相手に許可した構成情報		*/

/*
 *  lcp_init -- LCP モジュールの初期化
 */

static void
lcp_init (void)
{
	fsm_init(&lcp_fsm);
	}

/*
 *  lcp_input -- LCP 入力
 */

static void
lcp_input (T_NET_BUF *input)
{
	NET_COUNT_PPP_LCP(net_count_ppp_lcp_in_octets,  input->len);
	NET_COUNT_PPP_LCP(net_count_ppp_lcp_in_packets, 1);
	fsm_input(&lcp_fsm, input);
	}

/*
 *  lcp_resetci -- LCPの構成情報をリセットする。
 */

static void
lcp_resetci (T_PPP_FSM *fsm)
{
#ifdef LCP_CFG_MAGIC

	SYSTIM	time;

#endif	/* of #ifdef LCP_CFG_MAGIC */

	/* 自構成情報の初期設定 */
	lcp_local_neg_cfg	= lcp_local_def_cfg;
	lcp_local_ack_cfg	= lcp_def_cfg;

	/* 相手の構成情報の初期設定 */
	lcp_remote_neg_cfg	= lcp_def_cfg;
	lcp_remote_ack_cfg	= lcp_def_cfg;

#ifdef LCP_CFG_MAGIC

	syscall(get_tim(&time));
	net_srand((unsigned int)time);
	lcp_local_neg_cfg.magic = net_rand();

#endif	/* of #ifdef LCP_CFG_MAGIC */

	auth_reset();
	}

/*
 *  lcp_cilen -- 構成情報の長さを返す。
 */

static int_t
lcp_cilen (T_PPP_FSM *fsm)
{
	int_t cilen = 0;

	if (lcp_local_neg_cfg.options & LCP_CFG_MRU)
		cilen += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);	/* MRU   */

	if (lcp_local_neg_cfg.options & LCP_CFG_ACCM)
		cilen += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);	/* ACCM  */

#ifdef LCP_CFG_MAGIC

	if (lcp_local_neg_cfg.options & LCP_CFG_MAGIC)
		cilen += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);	/* Magic */

#endif	/* of #ifdef LCP_CFG_MAGIC */

#ifdef LCP_CFG_PAP

	if (lcp_local_neg_cfg.options & LCP_CFG_PAP)
		cilen += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);	/* PAP   */

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

	if (lcp_local_neg_cfg.options & LCP_CFG_PCOMP)
		cilen += sizeof(T_PPP_CI_HDR);			/* プロトコル圧縮 */

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

	if (lcp_local_neg_cfg.options & LCP_CFG_ACCOMP)
		cilen += sizeof(T_PPP_CI_HDR);			/* アドレス・制御圧縮 */

#endif	/* of #ifdef LCP_CFG_ACCOMP */

	return cilen;
	}

/*
 *  lcp_addci -- LCPの構成情報を追加する。
 */

static void
lcp_addci (T_PPP_FSM *fsm, T_NET_BUF *output)
{
	uint8_t *buf;

	buf = output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);

	/* MRU オプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_MRU) {
		*buf ++ = LCP_CIT_MRU;
		*buf ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		*((uint16_t*)buf) = htons(lcp_local_neg_cfg.MRU);
		buf += sizeof(uint16_t);
		}

	/* ACCM オプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_ACCM) {
		*buf ++ = LCP_CIT_ACCM;
		*buf ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		ahtonl(buf, lcp_local_neg_cfg.ACCM);
		buf += sizeof(uint32_t);
		}

#ifdef LCP_CFG_MAGIC

	/* Magic Number オプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_MAGIC) {
		*buf ++ = LCP_CIT_MAGIC;
		*buf ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		ahtonl(buf, lcp_local_neg_cfg.magic);
		buf += sizeof(uint32_t);
		}

#endif	/* of #ifdef LCP_CFG_MAGIC */

#ifdef LCP_CFG_PAP

	/* 認証プロトコルオプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_PAP) {
		*buf ++ = LCP_CIT_AUTH;
		*buf ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		*((uint16_t*)buf) = htons(PPP_PAP);
		buf += sizeof(uint16_t);
		}

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

	/* プロトコル圧縮オプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_PCOMP) {
		*buf ++ = LCP_CIT_PCOMP;
		*buf ++ = sizeof(T_PPP_CI_HDR);
		}

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

	/* アドレス・制御圧縮オプションを追加する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_ACCOMP) {
		*buf ++ = LCP_CIT_ACCOMP;
		*buf ++ = sizeof(T_PPP_CI_HDR);
		}

#endif	/* of #ifdef LCP_CFG_ACCOMP */
	}

/*
 *  lcp_ackci -- ACK を受信したときの処理
 *
 *	戻り値:	TURE  ACK は正常
 *		false ACK は異常
 */

static bool_t
lcp_ackci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	uint8_t		*buf;
	uint16_t	len;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	/*
	 *  構成情報は送信した順に完全に一致しなければならない。
	 *  従って、lcp_addci で作成した順に解析する。
	 */

	/* MRU オプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_MRU) {
		if (len < sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	||
		    *buf ++ != LCP_CIT_MRU			||
		    *buf ++ != sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)||
		    *((uint16_t*)buf) != htons(lcp_local_neg_cfg.MRU)	)
			return false;
		buf += sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		}

	/* ACCM オプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_ACCM) {
		if (len < sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	||
		    *buf ++ != LCP_CIT_ACCM			||
		    *buf ++ != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)||
		    nahcmpl(buf, lcp_local_neg_cfg.ACCM)        )
			return false;
		buf += sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		}

#ifdef LCP_CFG_MAGIC

	/* Magic Number オプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_MAGIC) {
		if (len < sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	||
		    *buf ++ != LCP_CIT_MAGIC			||
		    *buf ++ != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)||
		    nahcmpl(buf, lcp_local_neg_cfg.magic)       )
			return false;
		buf += sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		}

#endif	/* of #ifdef LCP_CFG_MAGIC */

#ifdef LCP_CFG_PAP

	/* 認証プロトコルオプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_PAP) {
		if (len < sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	||
		    *buf ++ != LCP_CIT_AUTH			||
		    *buf ++ != sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)||
		    *((uint16_t*)buf) != htons(PPP_PAP)		)
			return false;
		buf += sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		}

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

	/* プロトコル圧縮オプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_PCOMP) {
		if (len < sizeof(T_PPP_CI_HDR)			||
		    *buf ++ != LCP_CIT_PCOMP			||
		    *buf ++ != sizeof(T_PPP_CI_HDR))
			return false;
		len -= sizeof(T_PPP_CI_HDR);
		}

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

	/* アドレス・制御圧縮オプションを解析する。 */
	if (lcp_local_neg_cfg.options & LCP_CFG_ACCOMP) {
		if (len < sizeof(T_PPP_CI_HDR)			||
		    *buf ++ != LCP_CIT_ACCOMP			||
		    *buf ++ != sizeof(T_PPP_CI_HDR))
			return false;
		len -= sizeof(T_PPP_CI_HDR);
		}

#endif	/* of #ifdef LCP_CFG_ACCOMP */
	return true;
	}

/*
 *  lcp_nakci -- NAK を受信したときの処理
 *
 *	戻り値:	1  NAK は正常
 *		0  NAK は異常
 */

static int_t
lcp_nakci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_LCP_CONFIGS	new_cfg;
	bool_t		loop = false;	/* ループバックをチェック */
	uint32_t	cilong;
	uint16_t	cishort, len;
	uint8_t		*buf, cilen;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	new_cfg = lcp_local_neg_cfg;

	/* MRU オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)		&&
	    * buf      == LCP_CIT_MRU				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_MRU)) {
	    	cishort = ntohs(*((uint16_t*)(buf + 2)));
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		if (cishort <= lcp_local_neg_cfg.MRU || cishort <= DEF_PPP_MTU)
			new_cfg.MRU = cishort;
		}

	/* ACCM オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == LCP_CIT_ACCM				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_ACCM)) {
	    	ntoahl(cilong, buf + 2);
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		new_cfg.ACCM |= cilong;
		}

#ifdef LCP_CFG_MAGIC

	/* Magic Number オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == LCP_CIT_MAGIC				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_MAGIC)) {
	    	ntoahl(cilong, buf + 2);
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		new_cfg.magic = cilong;
		loop = true;
		}

#endif	/* of #ifdef LCP_CFG_MAGIC */

#ifdef LCP_CFG_PAP

	/* 認証プロトコルオプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)		&&
	    * buf      == LCP_CIT_AUTH				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_PAP)) {
	    	cishort = ntohs(*((uint16_t*)(buf + 2)));
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		if (cishort == PPP_PAP)
			new_cfg.options &= ~LCP_CFG_PAP;
		else
			return 0;
		}

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

	/* プロトコル圧縮オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR)				&&
	    * buf      == LCP_CIT_PCOMP				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR)			&&
	    (lcp_local_neg_cfg.options & LCP_CFG_PCOMP)) {
		buf += sizeof(T_PPP_CI_HDR);
		len -= sizeof(T_PPP_CI_HDR);
		new_cfg.options &= ~LCP_CFG_PCOMP;
		}

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

	/* アドレス・制御圧縮オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR)				&&
	    * buf      == LCP_CIT_ACCOMP			&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR)			&&
	    (lcp_local_neg_cfg.options & LCP_CFG_ACCOMP)) {
		buf += sizeof(T_PPP_CI_HDR);
		len -= sizeof(T_PPP_CI_HDR);
		new_cfg.options &= ~LCP_CFG_ACCOMP;
		}

#endif	/* of #ifdef LCP_CFG_ACCOMP */

	/* 後は無視する。*/
	while (len > sizeof(T_PPP_CI_HDR)) {
		cilen = *(buf + 1);
		if (len < cilen)
			return 0;
		buf += cilen;
		len -= cilen;
		}

	/* 長さが 0 でなければエラー */
	if (len != 0)
		return 0;

	/* 新しいオプションを設定する。*/
	if (fsm->state != PPP_FSM_OPENED) {
		if (loop) {
			syslog(LOG_WARNING, "[PPP/LCP] line looped.");
			lcp_close();
			}
		lcp_local_neg_cfg = new_cfg;
		}
	return 1;
	}

/*
 *  lcp_rejci -- REJ を受信したときの処理
 *
 *	戻り値:	1  REJ は正常
 *		0  REJ は異常
 */

static int_t
lcp_rejci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_LCP_CONFIGS	new_cfg;
	uint32_t	cilong;
	uint16_t	cishort, len;
	uint8_t		*buf;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	new_cfg = lcp_local_neg_cfg;

	/* MRU オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)		&&
	    * buf      == LCP_CIT_MRU					&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_MRU)) {
	    	cishort = ntohs(*((uint16_t*)(buf + 2)));
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		if (cishort != lcp_local_neg_cfg.MRU)
			return 0;
		else
			new_cfg.options &= ~LCP_CFG_MRU;
		}
		
	/* ACCM オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == LCP_CIT_ACCM					&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_ACCM)) {
	    	ntoahl(cilong, buf + 2);
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		if (cilong != lcp_local_neg_cfg.ACCM)
			return 0;
		else
			new_cfg.options &= ~LCP_CFG_ACCM;
		}

#ifdef LCP_CFG_MAGIC

	/* Magic Number オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == LCP_CIT_MAGIC					&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_MAGIC)) {
	    	ntoahl(cilong, buf + 2);
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		if (cilong != lcp_local_neg_cfg.magic)
			return 0;
		else
			new_cfg.options &= ~LCP_CFG_MAGIC;
		}

#endif	/* of #ifdef LCP_CFG_MAGIC */

#ifdef LCP_CFG_PAP

	/* 認証プロトコルオプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)		&&
	    * buf      == LCP_CIT_AUTH					&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint16_t)	&&
	    (lcp_local_neg_cfg.options & LCP_CFG_PAP)) {
	    	cishort = ntohs(*((uint16_t*)(buf + 2)));
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
		if (cishort != PPP_PAP)
			return 0;
		else
			new_cfg.options &= ~LCP_CFG_PAP;
		}

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

	/* プロトコル圧縮オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR)				&&
	    * buf      == LCP_CIT_PCOMP				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR)			&&
	    (lcp_local_neg_cfg.options & LCP_CFG_PCOMP)) {
		buf += sizeof(T_PPP_CI_HDR);
		len -= sizeof(T_PPP_CI_HDR);
		new_cfg.options &= ~LCP_CFG_PCOMP;
		}

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

	/* アドレス・制御圧縮オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR)				&&
	    * buf      == LCP_CIT_ACCOMP			&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR)			&&
	    (lcp_local_neg_cfg.options & LCP_CFG_ACCOMP)) {
		buf += sizeof(T_PPP_CI_HDR);
		len -= sizeof(T_PPP_CI_HDR);
		new_cfg.options &= ~LCP_CFG_ACCOMP;
		}

#endif	/* of #ifdef LCP_CFG_ACCOMP */

	/* 長さが 0 でなければエラー */
	if (len != 0)
		return 0;

	/* 新しいオプションを設定する。*/
	if (fsm->state != PPP_FSM_OPENED)
		lcp_local_neg_cfg = new_cfg;

	return 1;
	}

/*
 *  lcp_reqci -- 相手の構成情報を解析する。
 */

static int_t
lcp_reqci (T_PPP_FSM *fsm, T_NET_BUF *input, T_NET_BUF *output)
{
#if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC

	uint32_t	cilong;

#endif	/* of #if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC */

	uint16_t	code, final, len, cilen, cishort;
	uint8_t		*np, *rp, *ap, *cp, type = 0;

	rp = ap = input->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
				/* rp: REJ、ap: ACK する CI のポインタ		*/
				/* 拒否する場合は、引数 input に上書きする。	*/
	len   = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
	np    = output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	final = PPP_CONFACK;	/* 最終的に送信するコード */
	while (len > 0) {
		code = PPP_CONFACK;
		cp   = ap;	/* cp: 現在処理中の CI	*/

		if (len < sizeof(T_PPP_CI_HDR)) {

			/* 残りが CI ヘッダより短い */
			syslog(LOG_WARNING, "[PPP/LCP] bad CI len: %d.", len);
			cilen = len;	/* データ長が異常の場合の処置	*/
			len   = 0;	/* ループから抜ける。		*/
			code  = PPP_CONFREJ;
			goto endswitch;
			}

		type  = *ap ++;	/* CI の型		*/
		cilen = *ap ++;	/* CI の長さ		*/
		if (len < cilen) {

			/* 残りが CI 長より短い */
			syslog(LOG_WARNING, "[PPP/LCP] bad CI len: %d.", cilen);
			cilen = len;	/* データ長が異常の場合の処置	*/
			len   = 0;	/* ループから抜ける。		*/
			code  = PPP_CONFREJ;
			goto endswitch;
			}
		len -= sizeof(T_PPP_CI_HDR);

		/* CI の型により分岐する。*/
		switch (type) {

		case LCP_CIT_MRU:	/* 最大受信量 (Maximum Receive Unit) */

#if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MRU) == 0

			code = PPP_CONFREJ;

#else	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MRU) == 0 */

		 	/* CI 長が、ヘッダ + 2 オクテットでなければ REJ */
		 	if (cilen != sizeof(T_PPP_CI_HDR) + sizeof(uint16_t))
				code = PPP_CONFREJ;
			{
				cishort = ntohs(*(uint16_t*)ap);

				/* 最小受信量より小さければ NAK */
				if (cishort < MIN_PPP_MTU) {
					*np ++ = LCP_CIT_MRU;
					*np ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint16_t);
					*((uint16_t*)np) = htons(MIN_PPP_MTU);
					np  += sizeof(uint16_t);
					code = PPP_CONFNAK;
					}
				else {
					lcp_remote_neg_cfg.MRU = cishort;
					lcp_remote_neg_cfg.options |= LCP_CFG_MRU;
					}
				}

#endif	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MRU) == 0 */

			break;

		case LCP_CIT_ACCM:	/* Async Control Character Map */

#if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCM) == 0

			code = PPP_CONFREJ;

#else	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCM) == 0 */

		 	/* CI 長が、ヘッダ + 4 オクテットでなければ REJ */
		 	if (cilen != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t))
				code = PPP_CONFREJ;
			else {
				ntoahl(lcp_remote_neg_cfg.ACCM, ap);
				lcp_remote_neg_cfg.options |= LCP_CFG_ACCM;
				}

#endif	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCM) == 0 */

			break;

		case LCP_CIT_MAGIC:	/* マジック番号 */

#if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC) == 0

			code = PPP_CONFREJ;

#else	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC) == 0 */

		 	/* CI 長が、ヘッダ + 4 オクテットでなければ REJ */
		 	if (cilen != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t))
				code = PPP_CONFREJ;
			else {
				ntoahl(cilong, ap);

				/* 番号が自分と同じなら NAK */
				if (cilong == lcp_local_neg_cfg.magic) {
					cilong = net_rand();
					*np ++ = LCP_CIT_MAGIC;
					*np ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
					ahtonl(np, cilong);
					np  += sizeof(uint32_t);
					code = PPP_CONFNAK;
					}
				else {
					lcp_remote_neg_cfg.magic = cilong;
					lcp_remote_neg_cfg.options |= LCP_CFG_MAGIC;
					}
				}

#endif	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC) == 0 */

			break;

#ifdef LCP_CFG_PAP
		/* CHAP は実装予定	*/

		case LCP_CIT_AUTH:	/* 認証プロトコル */

		 	/* CI 長が、ヘッダ + 2 オクテットより短いときはエラー */
		 	if (cilen < sizeof(T_PPP_CI_HDR) + sizeof(uint16_t))
				code = PPP_CONFREJ;
			else {
				cishort = ntohs(*(uint16_t*)ap);

				/* 認証プロトコルにより分岐する	*/
				if (cishort == PPP_PAP &&
				    cilen   == sizeof(T_PPP_CI_HDR) + sizeof(uint16_t) &&
			 	    (DEF_LCP_REMOTE_CFGS & LCP_CFG_PAP) != 0)
					lcp_remote_neg_cfg.options |= LCP_CFG_PAP;
				else
					code = PPP_CONFREJ;
				}
			break;

#endif	/* of #ifdef LCP_CFG_PAP */

#ifdef LCP_CFG_PCOMP

		case LCP_CIT_PCOMP:	/* プロトコル部圧縮は実装予定 */

#if (DEF_LCP_REMOTE_CFGS & LCP_CFG_PCOMP) == 0

			code = PPP_CONFREJ;

#else	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_PCOMP) == 0 */

		 	/* CI 長が、ヘッダ長だけでなければ REJ */
		 	if (cilen != sizeof(T_PPP_CI_HDR))
				code = PPP_CONFREJ;
			else
				lcp_remote_neg_cfg.options |= LCP_CFG_PCOMP;

#endif	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_PCOMP) == 0 */

			break;

#endif	/* of #ifdef LCP_CFG_PCOMP */

#ifdef LCP_CFG_ACCOMP

		case LCP_CIT_ACCOMP:	/* アドレス・制御部圧縮 */

#if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCOMP) == 0

			code = PPP_CONFREJ;

#else	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCOMP) == 0 */

		 	/* CI 長が、ヘッダ長だけでなければ REJ */
		 	if (cilen != sizeof(T_PPP_CI_HDR))
				code = PPP_CONFREJ;
			else
				lcp_remote_neg_cfg.options |= LCP_CFG_ACCOMP;

#endif	/* of #if (DEF_LCP_REMOTE_CFGS & LCP_CFG_ACCOMP) == 0 */

			break;

#endif	/* of #ifdef LCP_CFG_ACCOMP */

		default:
			syslog(LOG_WARNING, "[PPP/LCP] unexp opt: %d.", type);
			code  = PPP_CONFREJ;
			break;
			}
endswitch:
		if (code == PPP_CONFNAK) {
			/* CI の型が Magic Number の場合を除いて、NAK の回数が	*/
			/* 最大値 MAX_PPP_FAILURES  を超えたら拒否する。	*/
			if (fsm->failures >= MAX_PPP_FAILURES && type != LCP_CIT_MAGIC)
				code = PPP_CONFREJ;
			else if (final != PPP_CONFREJ)
				final = PPP_CONFNAK;
			}

		/* この CI を拒否する */
		if (code == PPP_CONFREJ) {
			if (rp < cp)		/* 前に詰める。*/
				memcpy(rp, cp, cilen);
			rp += cilen;
			final = PPP_CONFREJ;
			}

		ap   += cilen - sizeof(T_PPP_CI_HDR);
		len  -= cilen - sizeof(T_PPP_CI_HDR);
		}

	/* 最終的な長さを調整する。*/
	switch (final) {
	case PPP_CONFNAK:
		output->len = np - output->buf;
		memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
		break;
	case PPP_CONFREJ:
		memcpy(output->buf, input->buf, output->len = rp - input->buf);
		break;
	case PPP_CONFACK:
		memcpy(output->buf, input->buf, input->len);
		break;
		}
	return final;
	}

/*
 *  lcp_up -- リンク接続を確立する (Phase: Establish に移行する)。
 */

static void
lcp_up (T_PPP_FSM *fsm)
{
	/* 自構成情報の設定 */
	lcp_local_ack_cfg = lcp_local_neg_cfg;

	/* 相手の構成情報の設定 */
	lcp_remote_ack_cfg = lcp_remote_neg_cfg;

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

	echo_lowerup();

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

	link_established();
	}

/*
 *  lcp_starting -- 下位層を開始する。
 */

static void
lcp_starting (T_PPP_FSM *fsm)
{
	link_required();
	}

/*
 *  lcp_extcode -- LCP 特有の CP を受信したときの処理
 */

static bool_t
lcp_extcode (T_PPP_FSM *fsm, uint8_t code, uint8_t id, T_NET_BUF *input)
{
#if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC

	T_NET_BUF *output;

#endif	/* of #if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC */

	switch (code) {
	case PPP_PROTREJ:
		lcp_rprotrej(fsm, input);
		break;

	case PPP_ECHOREQ:

#if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC

		if (fsm->state == PPP_FSM_OPENED) {
			syscall(get_net_buf(&output, sizeof(T_PPP_HDR)
			                           + sizeof(T_PPP_CP_HDR)
			                           + sizeof(uint32_t)));
			*(uint32_t*)(output->buf + sizeof(T_PPP_HDR)
			                   + sizeof(T_PPP_CP_HDR)) = lcp_local_ack_cfg.magic;
			IF_SET_PROTO(output, PPP_LCP);
			ppp_cp_output(PPP_ECHOREP, id, output);
			}

#endif	/* of #if DEF_LCP_REMOTE_CFGS & LCP_CFG_MAGIC */

		break;

	case PPP_ECHOREP:

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

		recv_echo_rep(fsm, id, input);

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

		break;

	case PPP_DISCREQ:
		break;

	default:
		return false;
		break;

		}
	return true;
	}

/*
 *  lcp_rprotrej -- Proto-REJ を受信したときの処理
 */

static void
lcp_rprotrej (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_PPP_PROTENT	*entry;
	int_t		ix;
	uint16_t	proto;

	if (input->len < sizeof(uint16_t)) {
		syslog(LOG_WARNING, "[PPP/LCP] bad len: %d in recv Proto-REJ.");
		return;
		}

	proto = ntohs(*GET_PPP_HDR(input));

	if (fsm->state != PPP_FSM_OPENED) {
		return;
		}

	/* 上位プロトコルを初期化する */
	for (ix = 0; (entry = protocols[ix]) != NULL; ix ++)
		if (entry->proto == proto) {
			(*entry->protrej)();
			return;
			}

	syslog(LOG_WARNING, "[PPP/LCP] unexp proto: %d in recv Proto-REJ.", proto);
	}

/*
 *  lcp_protrej -- Proto-REJ を受信したときの処理
 */

static void
lcp_protrej (void)
{
	fsm_protreject(&lcp_fsm);
	}

/*
 *  lcp_lowerup -- LCP 下位層を起動する。
 */

void
lcp_lowerup (void)
{
	fsm_lowerup(&lcp_fsm);
	}

/*
 *  lcp_lowerdown -- LCP 下位層を停止する。
 */

void
lcp_lowerdown (void)
{
	fsm_lowerdown(&lcp_fsm);
	}

/*
 *  lcp_open -- LCP のオープ
 */

void
lcp_open (uint8_t open_mode)
{
	ppp_open_mode = open_mode;	/* オープン時の動作モード	*/
	fsm_open(&lcp_fsm);
	}

/*
 *  lcp_close -- LCP のクローズ
 */

void
lcp_close (void)
{
	if (ppp_phase != PPP_PHASE_DEAD)
		ppp_phase = PPP_PHASE_TERMINATE;

	if (ppp_open_mode == PPP_OPEN_PASSIVE && lcp_fsm.state == PPP_FSM_STOPPED) {
		lcp_fsm.state = PPP_FSM_CLOSED;
		lcp_finished(&lcp_fsm);
		}
	else
		fsm_close(&lcp_fsm);
	}

/*
 *  lcp_sprotrej -- 認識できないプロトコルが送られてきたときの処理
 */

void
lcp_sprotrej (T_NET_BUF *input)
{
	T_NET_BUF	*output;
	/*
	 *  Protocol-Reject を送り返す。
	 */

	syscall(get_net_buf(&output, input->len + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));

	memcpy(output->buf, input->buf, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
	memcpy(output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR), input->buf, input->len);

	IF_SET_PROTO(output, PPP_LCP);
	ppp_cp_output(PPP_PROTREJ, ++ lcp_fsm.id, output);
	}

/*
 *  lcp_down -- リンク接続を解放する (Phase: Dead に移行する)。
 */

void
lcp_down (T_PPP_FSM *fsm)
{

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

	echo_lowerdown();

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

	link_down();
	}

/*
 *  lcp_finished -- 下位層を終了する。
 */

void
lcp_finished (T_PPP_FSM *fsm)
{
	link_terminated();
	}

#if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC

/*
 *  recv_echo_rep -- ECHOREP を受信したときの処理
 */

static void
recv_echo_rep(T_PPP_FSM *fsm, uint8_t id, T_NET_BUF *input)
{
	T_PPP_CP_HDR	*hdr;
	uint32_t	magic;

 	hdr = GET_PPP_CP_HDR(input);

 	/* Magic Number の長さ以下ならエラー */
 	if (hdr->len < sizeof(T_PPP_CP_HDR) + sizeof(uint32_t)) {
		syslog(LOG_WARNING, "[PPP/LCP] bad ECHOREP len: %d.", hdr->len);
		return;
		}

	ntoahl(magic, input->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

 	/* 受信した Magic Number が自分と同じならループしている。*/
	if ((lcp_local_ack_cfg.options & LCP_CFG_MAGIC) && magic == lcp_local_ack_cfg.magic) {
		syslog(LOG_WARNING, "[PPP/LCP] ECHOREP looped.");
		return;
		}

	echo_pending = 0;
	}

/*
 *  echo_timeout -- ECHO REQ/REP のタイムアウト処理
 */

static void
echo_timeout (void *arg)
{
	if (timer_running) {
		timer_running = false;
		send_echo_req((T_PPP_FSM *)arg);
		}
	}

/*
 *  send_echo_req -- ECHO REQ を送信する。
 */

static void
send_echo_req (T_PPP_FSM *fsm)
{
	T_NET_BUF	*input;

#if LCP_ECHO_FAILS > 0

	if (echo_pending >= LCP_ECHO_FAILS) {
		syslog(LOG_WARNING, "[PPP/LCP] no reply to ECHOREQ.");
		lcp_close();
		echo_pending = 0;
		}

#endif	/* of #if LCP_ECHO_FAILS > 0 */

	if (fsm->state == PPP_FSM_OPENED) {

		/* ECHOREQ を送信する。*/
		syscall(get_net_buf(&input, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR) + sizeof(uint32_t)));
		*(uint32_t*)(input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)) = lcp_local_ack_cfg.magic;
		IF_SET_PROTO(input, PPP_LCP);
		ppp_cp_output(PPP_ECHOREQ, echo_number ++, input);
		echo_pending ++;

		/* タイマーをスタートする。*/
		timeout((FP)echo_timeout, fsm, LCP_ECHO_INTERVAL);
		timer_running = true;
		}
	}

/*
 *  echo_lowerup -- ECHO REQ/REP の起動
 */

static void
echo_lowerup (void)
{
	echo_pending  = 0;
	echo_number   = 0;
	timer_running = false;

	send_echo_req(&lcp_fsm);
	}

/*
 *  echo_lowerdown -- ECHO REQ/REP の停止
 */

static void
echo_lowerdown (void)
{
	if (timer_running) {
		untimeout((FP)echo_timeout, &lcp_fsm);
		timer_running = false;
		}
	}

#endif	/* of #if DEF_LCP_LOCAL_CFGS & LCP_CFG_MAGIC */

#endif	/* fo #ifdef SUPPORT_PPP */
