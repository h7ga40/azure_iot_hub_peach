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
 * auth.c - PPP authentication and phase control.
 *
 * Copyright (c) 1993 The Australian National University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Australian National University.  The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
 *			PPP Secret Key Module
 *
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1994, Internet Initiative Japan, Inc. All rights reserverd.
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
 * $FreeBSD: src/usr.sbin/ppp/auth.c,v 1.50 1999/12/27 11:43:30 brian Exp $
 *
 *	TODO:
 *		o Implement check against with registered IP addresses.
 */

#include <s_services.h>
#include <t_services.h>

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_auth.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_upap.h>

#ifdef SUPPORT_PPP

/*
 *  定数
 */

#define PAP_PEND_SERVER		UINT_C(0x01)
#define PAP_PEND_CLIENT		UINT_C(0x02)
#define CHAP_PEND_SERVER	UINT_C(0x04)
#define CHAP_PEND_CLIENT	UINT_C(0x08)

/*
 *  network_phase -- コネクション確立
 */

void
network_phase (void)
{
	int_t		num, ix;
	T_PPP_PROTENT	*proto;

	ppp_phase = PPP_PHASE_NETWORK;

	/* 上位プロトコルをオープンする */
	num = 0;
	for (ix = 0; (proto = protocols[ix]) != NULL; ix ++)
#if 0	/* 保留、削除禁止 */
		if (proto->proto < 0xc000 && proto->open != NULL) {
#else	/* of #if 0 */
		if (proto->open != NULL) {
#endif	/* of #if 0 */
			(*proto->open)();
			if (proto->proto != PPP_CCP)
				num ++;
			}

	if (num == 0)
		/* 上位プロトコルがない */
		lcp_close();
	else
		syslog(LOG_NOTICE, "[PPP] up.");
	}

/*
 *  link_required -- リンクを開設する。
 */

void
link_required (void)
{
	}

/*
 *  link_terminated -- ログアウトして、リンクを切断する。
 */

void
link_terminated (void)
{
	ppp_phase     = PPP_PHASE_DEAD;
	ppp_open_mode = PPP_OPEN_PASSIVE;
	syslog(LOG_NOTICE, "[PPP] down.");
	}

/*
 *  link_down -- リンクを解放する。
 */

void
link_down (void)
{
	int_t		ix;
	T_PPP_PROTENT	*proto;

	/* 上位プロトコルを解放する */
	for (ix = 0; (proto = protocols[ix]) != NULL; ix ++) {
		if (proto->proto != PPP_LCP && proto->lowerdown != NULL)
			(*proto->lowerdown)();
		if (proto->proto < 0xc000 && proto->close != NULL)
			(*proto->close)();
		}

	if (ppp_phase != PPP_PHASE_DEAD)
		ppp_phase = PPP_PHASE_TERMINATE;
	}

/*
 *  link_established -- リンクの接続が確立した。
 */

void
link_established (void)
{
	int_t		ix;
	T_PPP_PROTENT	*proto;

#if defined(LCP_CFG_CHAP) || defined(LCP_CFG_PAP)

	int		auth;

#endif	/* of #if defined(LCP_CFG_CHAP) || defined(LCP_CFG_PAP) */

	/* 上位プロトコルを起動する */
	for (ix = 0; (proto = protocols[ix]) != NULL; ix ++)
		if (proto->lowerup != NULL)
			(*proto->lowerup)();

	/* 認証オプションを確認する。【未実装】*/

	ppp_phase = PPP_PHASE_AUTHENTICATE;

#if defined(LCP_CFG_CHAP) || defined(LCP_CFG_PAP)

	auth = 0;

#ifdef AUTH_CFG_SERVER

#if defined(LCP_CFG_CHAP)

	if (lcp_local_ack_cfg.options & LCP_CFG_CHAP) {
		chap_auth_server();
		auth |= CHAP_PEND_SERVER;
		}

#endif	/* of #if defined(LCP_CFG_CHAP) */

#if defined(LCP_CFG_PAP)
	
	if (lcp_local_ack_cfg.options & LCP_CFG_PAP) {
		upap_auth_server();
		auth |= PAP_PEND_SERVER;
		}

#endif	/* of #if defined(LCP_CFG_PAP) */

#endif	/* of #ifdef AUTH_CFG_SERVER */

#ifdef AUTH_CFG_CLIENT

#if defined(LCP_CFG_PAP)
	
	if (lcp_remote_ack_cfg.options & LCP_CFG_PAP) {
		upap_auth_client();
		auth |= PAP_PEND_CLIENT;
		}

#endif	/* of #if defined(LCP_CFG_PAP) */

#endif	/* of #ifdef AUTH_CFG_CLIENT */

	if (auth == 0)
		network_phase();

#else	/* of #if defined(LCP_CFG_CHAP) || defined(LCP_CFG_PAP) */

	network_phase();

#endif	/* of #if defined(LCP_CFG_CHAP) || defined(LCP_CFG_PAP) */

	}

/*
 *  auth_reset -- 認証オプションを再確認する。
 */

void
auth_reset (void)
{
	}

#endif	/* fo #ifdef SUPPORT_PPP */
