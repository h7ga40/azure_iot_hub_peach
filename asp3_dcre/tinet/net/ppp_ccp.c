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
 * ccp.c - PPP Compression Control Protocol.
 *
 * Copyright (c) 1994 The Australian National University.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, provided that the above copyright
 * notice appears in all copies.  This software is provided without any
 * warranty, express or implied. The Australian National University
 * makes no representations about the suitability of this software for
 * any purpose.
 *
 * IN NO EVENT SHALL THE AUSTRALIAN NATIONAL UNIVERSITY BE LIABLE TO ANY
 * PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE AUSTRALIAN NATIONAL UNIVERSITY HAVE BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * THE AUSTRALIAN NATIONAL UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE AUSTRALIAN NATIONAL UNIVERSITY HAS NO
 * OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS,
 * OR MODIFICATIONS.
 */

/*
 *	   PPP Compression Control Protocol (CCP) Module
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
 * $FreeBSD: src/usr.sbin/ppp/ccp.c,v 1.54.2.2 2000/08/19 09:29:59 brian Exp $
 *
 *	TODO:
 *		o Support other compression protocols
 */

/*
 *  実装予定、現在はすべて拒否
 */

#include <t_services.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/net_buf.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_fsm.h>
#include <net/ppp_ccp.h>

/*
 *  関数
 */

static void ccp_init (void);
static void ccp_input (T_NET_BUF *nbuf);
static void ccp_protrej (void);
static void ccp_lowerup (void);
static void ccp_lowerdown (void);
static void ccp_open (void);
static void ccp_close (void);

/*
 * FSM から呼出されるコールバック関
 */

static int_t  ccp_reqci	(T_PPP_FSM *fsm, T_NET_BUF *nbuf);
					/* 相手の構成情報を確認する。	*/

/*
 *  全域変数
 */

T_PPP_FSM_CALLBACKS ccp_callbacks = {
	NULL,				/* 自構成情報をリセットする。	*/
	NULL,				/* 構成情報の長さを返す。		*/
	NULL,				/* 自構成情報を追加する。		*/
	NULL,				/* ACK を受信したときの処理	*/
	NULL,				/* NAK を受信したときの処理	*/
	NULL,				/* REJ を受信したときの処理	*/
	ccp_reqci,			/* 相手の構成情報を確認する。	*/
	NULL,				/* リンク接続を確立する。		*/
	NULL,				/* リンク接続を解放する。		*/
	NULL,				/* 下位層を開始する。		*/
	NULL,				/* 下位層を終了する。		*/
	NULL,				/* 再送する。			*/
	NULL,				/* 不明な CP を受信したときの処理	*/
	};

T_PPP_FSM ccp_fsm = {
	&ccp_callbacks,			/* コールバック関数		*/
	PPP_CCP,			/* プロトコル			*/
	};

T_PPP_PROTENT ccp_protent = {
	PPP_CCP,
	ccp_init,			/* 初期化			*/
	ccp_input,			/* 入力				*/
	ccp_protrej,			/* Proto-REJ 受信処理		*/
	ccp_lowerup,			/* 下位層を起動する		*/
	ccp_lowerdown,			/* 下位層を停止する		*/
	ccp_open,			/* オープンする			*/
	ccp_close,			/* クローズする			*/
	NULL,				/* データ入力			*/
	};

/*
 *  ccp_init -- CCP モジュールの初期化
 */

static void
ccp_init (void)
{
	fsm_init(&ccp_fsm);
	}

/*
 *  ccp_input -- CCP 入力
 */

static void
ccp_input (T_NET_BUF *nbuf)
{
	NET_COUNT_PPP_CCP(net_count_ppp_ccp_in_octets,  nbuf->len);
	NET_COUNT_PPP_CCP(net_count_ppp_ccp_in_packets, 1);
	fsm_input(&ccp_fsm, nbuf);
	}

/*
 *  ccp_reqci -- 相手の構成情報を解析する。
 */

static int_t
ccp_reqci (T_PPP_FSM *fsm, T_NET_BUF *nbuf)
{
/*
 *  実装予定、現在はすべてのオプションを拒否する。
 */
	return nbuf->len > (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR)) ? PPP_CONFREJ : PPP_CONFACK;
	}

/*
 *  ccp_protrej -- Proto-REJ を受信したときの処理
 */

static void
ccp_protrej (void)
{
	fsm_lowerdown(&ccp_fsm);
	}

/*
 *  ccp_lowerup -- CCP 下位層を起動する。
 */

static void
ccp_lowerup (void)
{
	fsm_lowerup(&ccp_fsm);
	}

/*
 *  ccp_lowerdown -- CCP 下位層を停止する。
 */

static void
ccp_lowerdown (void)
{
	fsm_lowerdown(&ccp_fsm);
	}

/*
 *  ccp_open -- CCP のオープ
 */

static void
ccp_open (void)
{
	fsm_open(&ccp_fsm);
	}

/*
 *  ccp_close -- CCP のクローズ
 */

static void
ccp_close (void)
{
	fsm_close(&ccp_fsm);
	}
