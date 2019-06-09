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
 * upap.c - User/Password Authentication Protocol.
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
 *			PPP PAP Module
 *
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993-94, Internet Initiative Japan, Inc.
 *		     All rights reserverd.
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
 * $FreeBSD: src/usr.sbin/ppp/pap.c,v 1.42.2.1 2000/08/19 09:30:05 brian Exp $
 *
 *	TODO:
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
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_hdlc.h>
#include <net/ppp_auth.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_upap.h>

#ifdef SUPPORT_PPP

/*
 *  関数
 */

static void upap_init (void);
static void upap_input (T_NET_BUF *input);
static void upap_protrej (void);
static void upap_lowerup (void);
static void upap_lowerdown (void);
static bool_t compare (uint8_t *rstr, uint8_t *lstr, uint8_t rlen);

#ifdef AUTH_CFG_CLIENT

static void upap_sauthreq (void);
static void upap_rauthack (void);
static void upap_rauthnak (void);

#if defined(DEF_PAP_TIMEOUT)

static void upap_timeout(void *arg);

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

static void upap_rauthreq (T_NET_BUF *input);
static void upap_sresp    (uint8_t code, uint8_t id);

#if defined(DEF_PAP_REQTIME)

static void upap_reqtimeout(void *arg);

#endif	/* of #if defined(DEF_PAP_REQTIME) */

#endif	/* of #ifdef AUTH_CFG_SERVER */

/*
 *  全域変数
 */

T_PPP_PROTENT pap_protent = {
	PPP_PAP,
	upap_init,			/* 初期化			*/
	upap_input,			/* 入力				*/
	upap_protrej,			/* Proto-REJ 受信処理		*/
	upap_lowerup,			/* 下位層を起動する		*/
	upap_lowerdown,			/* 下位層を停止する		*/
	NULL,				/* オープンする			*/
	NULL,				/* クローズする			*/
	NULL,				/* データ入力			*/
	};

/*
 *  変数
 */

#ifdef AUTH_CFG_CLIENT

static uint8_t client_state;
static uint8_t client_cp_id;

#if defined(DEF_PAP_TIMEOUT)

static uint8_t client_rexmt = 0;

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

static uint8_t server_state;

#endif	/* of #ifdef AUTH_CFG_SERVER */

/*
 *  upap_init -- UPAP モジュールの初期化
 */

static void
upap_init (void)
{
#ifdef AUTH_CFG_CLIENT

	client_cp_id = 0;
	client_state = PAP_CS_INIT;

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

	server_state = PAP_SS_INIT;

#endif	/* of #ifdef AUTH_CFG_SERVER */
	}

/*
 *  upap_input -- UPAP 入力
 */

static void
upap_input (T_NET_BUF *input)
{
	T_PPP_CP_HDR	*hdr;

	NET_COUNT_PPP_PAP(net_count_ppp_upap_in_octets,  input->len);
	NET_COUNT_PPP_PAP(net_count_ppp_upap_in_packets, 1);

 	/* PPP リンク制御 (CP) ヘッダより短ければエラー */
 	if (input->len < (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR))) {
 		syslog(LOG_WARNING, "[PPP/PAP] short hdr: %d.", input->len);
 		return;
 		}

 	hdr = GET_PPP_CP_HDR(input);

 	/* ヘッダの長さと入力データサイズが一致しなければエラー */
 	if (hdr->len != input->len - sizeof(T_PPP_HDR)) {
 		syslog(LOG_WARNING, "[PPP/PAP] bad len: %d.", hdr->len);
 		return;
 		}
 
 	/*
 	 *  制御コードにより適当な関数を呼出す
 	 */

	switch (hdr->code) {

#ifdef AUTH_CFG_CLIENT

	case PAP_AUTHACK:
		upap_rauthack();
		break;

	case PAP_AUTHNAK:
		upap_rauthnak();
		break;

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

	case PAP_AUTHREQ:
		upap_rauthreq(input);
		break;

#endif	/* of #ifdef AUTH_CFG_SERVER */

		}
	}

/*
 *  upap_protrej -- Proto-REJ を受信したときの処理
 */

static void
upap_protrej (void)
{
#ifdef AUTH_CFG_CLIENT

	if (client_state == PAP_CS_AUTHREQ)
	 	syslog(LOG_WARNING, "[PPP/PAP] proto-rej recved.");

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

	if (server_state == PAP_SS_LISTEN) {
	 	syslog(LOG_WARNING, "[PPP/PAP] proto-rej recved.");
		lcp_close();
		}

#endif	/* of #ifdef AUTH_CFG_SERVER */

	upap_lowerdown();
	}

/*
 *  upap_lowerup -- UPAP 下位層を起動する。
 */

static void
upap_lowerup (void)
{
#ifdef AUTH_CFG_CLIENT

	if (client_state == PAP_CS_INIT)
		client_state = PAP_CS_CLOSED;
	else if (client_state == PAP_CS_PENDING)
		upap_sauthreq();

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

	if (server_state == PAP_SS_INIT)
		server_state = PAP_SS_CLOSED;
	else if (server_state == PAP_SS_PENDING) {
		server_state = PAP_SS_LISTEN;

#if defined(DEF_PAP_REQTIME)

		timeout((FP)upap_reqtimeout, NULL, DEF_PAP_REQTIME);

#endif	/* of #if defined(DEF_PAP_REQTIME) */

		}

#endif	/* of #ifdef AUTH_CFG_SERVER */
	}

/*
 *  upap_lowerdown -- UPAP 下位層を停止する。
 */

static void
upap_lowerdown (void)
{
#ifdef AUTH_CFG_CLIENT

#if defined(DEF_PAP_TIMEOUT)

	if (client_state == PAP_CS_AUTHREQ)
		untimeout((FP)upap_timeout, NULL);

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

	client_state = PAP_CS_INIT;

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

#if defined(DEF_PAP_REQTIME)

	if (server_state == PAP_SS_LISTEN)
		untimeout((FP)upap_reqtimeout, NULL);

#endif	/* of #if defined(DEF_PAP_REQTIME) */

	server_state = PAP_SS_INIT;

#endif	/* of #ifdef AUTH_CFG_SERVER */
	}
#ifdef AUTH_CFG_CLIENT

/*
 *  クライアントモードで PAP 認証を開始する。
 */

void
upap_auth_client (void)
{
#if defined(DEF_PAP_TIMEOUT)

	client_rexmt = 0;

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

	if (client_state == PAP_CS_INIT || client_state == PAP_CS_PENDING) {
		client_state = PAP_CS_PENDING;
		return;
		}

	upap_sauthreq();
	}

/*
 *  upap_rauthack -- 認証 ACK 処理
 */

static void
upap_rauthack (void)
{
	if (client_state == PAP_CS_AUTHREQ) {
		client_state = PAP_CS_OPEN;
		network_phase();
		}
	}

/*
 *  upap_rauthnak -- 認証 NAK 処理
 */

static void
upap_rauthnak (void)
{
 	syslog(LOG_WARNING, "[PPP/PAP] auth-req NAKed.");
	client_state = PAP_CS_BADAUTH;
	}

/*
 *  upap_sauthreq -- 認証要求処理
 */

static void
upap_sauthreq (void)
{
	T_NET_BUF	*output;
	uint8_t		*data;

	syscall(get_net_buf(&output, sizeof(T_PPP_HDR)        + sizeof(T_PPP_CP_HDR)
	                           + sizeof(AUTH_REMOTE_USER) + sizeof(AUTH_REMOTE_PASSWD)));

	data = output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);

	/* ユーザ名を設定する。*/
	*data = sizeof(AUTH_REMOTE_USER) - 1;
	strcpy(data + 1, AUTH_REMOTE_USER);

	/* パスワードを設定する。*/
	*(data + sizeof(AUTH_REMOTE_USER)) = sizeof(AUTH_REMOTE_PASSWD) - 1;
	strcpy(data + sizeof(AUTH_REMOTE_USER) + 1, AUTH_REMOTE_PASSWD);

	/* 送信する */
	IF_SET_PROTO(output, PPP_PAP);
	ppp_cp_output(PAP_AUTHREQ, ++ client_cp_id, output);

#if defined(DEF_PAP_TIMEOUT)

	timeout((FP)upap_timeout, NULL, DEF_PAP_TIMEOUT);
	client_rexmt ++;

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

	client_state = PAP_CS_AUTHREQ;
	}

#if defined(DEF_PAP_TIMEOUT)

/*
 *  upap_timeout -- タイムアウト処理
 */

static void
upap_timeout (void *arg)
{
	if (client_state != PAP_CS_AUTHREQ)
		return;

	if (client_rexmt >= MAX_PAP_REXMT) {
 		syslog(LOG_WARNING, "[PPP/PAP] no reply auth-req.");
		client_state = PAP_CS_BADAUTH;
		return;
		}

	upap_sauthreq();
	}

#endif	/* of #if defined(DEF_PAP_TIMEOUT) */

#endif	/* of #ifdef AUTH_CFG_CLIENT */

#ifdef AUTH_CFG_SERVER

/*
 *  サーバモードで PAP 認証を開始する。
 */

void
upap_auth_server (void)
{
	if (server_state == PAP_SS_INIT || server_state == PAP_SS_PENDING) {
		server_state = PAP_SS_PENDING;
		return;
		}

	server_state = PAP_SS_LISTEN;

#if defined(DEF_PAP_REQTIME)

	timeout((FP)upap_reqtimeout, NULL, DEF_PAP_REQTIME);

#endif	/* of #if defined(DEF_PAP_REQTIME) */
	}

/*
 *  upap_rauthreq -- 認証要求応答処理
 */

static void
upap_rauthreq (T_NET_BUF *input)
{
	int16_t	cplen;
	uint8_t	*data, *user, ulen, plen, code, id;

	if (server_state < PAP_SS_LISTEN)
		return;

	/*
	 *  再要求があったときの処理
	 */
	id = GET_PPP_CP_HDR(input)->id;
	if (server_state == PAP_SS_OPEN) {
		upap_sresp(PAP_AUTHACK, id);
		return;
		}

	if (server_state == PAP_SS_BADAUTH) {
		upap_sresp(PAP_AUTHNAK, id);
		return;
		}

	cplen = GET_PPP_CP_HDR(input)->len;
	data  = input->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);

	/*
	 *  ユーザ名を特定する。
	 */
	ulen  = *data;
	if (cplen < sizeof(T_PPP_CP_HDR) + ulen + sizeof(uint8_t)) {
 		syslog(LOG_WARNING, "[PPP/PAP] bad req len: %d.", cplen);
 		return;
		}
	user = ++ data;
	data += ulen;

	/*
	 *  パスワードを特定する。
	 */
	plen  = *data;
	if (cplen < sizeof(T_PPP_CP_HDR) + ulen + plen + sizeof(uint8_t) * 2) {
 		syslog(LOG_WARNING, "[PPP/PAP] bad req len: %d.", cplen);
 		return;
		}

	/*
	 *  ユーザ名とパスワードをチェックする。
	 */
	if (compare(user,     AUTH_LOCAL_USER,   ulen) &&
	    compare(data + 1, AUTH_LOCAL_PASSWD, plen))
		code = PAP_AUTHACK;
	else
		code = PAP_AUTHNAK;

	upap_sresp(code, id);

	if (code == PAP_AUTHACK) {
		network_phase();
		server_state = PAP_SS_OPEN;
		}
	else {
		lcp_close();
		server_state = PAP_SS_BADAUTH;
		}

#if defined(DEF_PAP_REQTIME)

	untimeout((FP)upap_reqtimeout, NULL);

#endif	/* of #if defined(DEF_PAP_REQTIME) */
	}

/*
 *  upap_sresp -- 応答を返す。
 */

static void
upap_sresp (uint8_t code, uint8_t id)
{
	T_NET_BUF *output;

	/* 送信する */
	syscall(get_net_buf(&output, sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)));
	IF_SET_PROTO(output, PPP_PAP);
	ppp_cp_output(code, id, output);
	}

/*
 *  compare -- ユーザ名とパスワードの比較
 */

static bool_t
compare (uint8_t *rstr, uint8_t *lstr, uint8_t rlen)
{
	while (rlen -- > 0) {
		if (*rstr != *lstr)
			return false;
		rstr ++;
		lstr ++;
		}
	return *lstr ? false : true;
	}

#if defined(DEF_PAP_REQTIME)

/*
 *  upap_reqtimeout -- 要求タイムアウト処理
 */

static void
upap_reqtimeout (void *arg)
{
	if (server_state == PAP_SS_LISTEN) {
		lcp_close();
		server_state = PAP_SS_BADAUTH;
		}
	}

#endif	/* of #if defined(DEF_PAP_REQTIME) */

#endif	/* of #ifdef AUTH_CFG_SERVER */

#endif	/* fo #ifdef SUPPORT_PPP */
