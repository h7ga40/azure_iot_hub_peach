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
 * ipcp.c - PPP IP Control Protocol.
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
 *	PPP IP Control Protocol (IPCP) Module
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
 * $FreeBSD: src/usr.sbin/ppp/ipcp.c,v 1.90.2.4 2000/08/19 09:30:03 brian Exp $
 *
 *	TODO:
 *		o Support IPADDRS properly
 *		o Validate the length in IpcpDecodeConfig
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
#include <net/net_buf.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_ipcp.h>
#include <net/ppp_modem.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_timer.h>
#include <netinet/udp_var.h>

#include <net/if_var.h>

#ifdef SUPPORT_PPP

/*
 *  関数
 */

static void ipcp_init (void);
static void ipcp_input (T_NET_BUF *input);
static void ipcp_protrej (void);
static void ipcp_lowerup (void);
static void ipcp_lowerdown (void);
static void ipcp_open (void);
static void ipcp_close (void);

/*
 * FSM から呼出されるコールバック関
 */

static void  ipcp_resetci(T_PPP_FSM *fsm);	/* 自構成情報をリセットする。	*/
static int_t ipcp_cilen	(T_PPP_FSM *fsm);	/* 構成情報の長さを返す。		*/
static void  ipcp_addci	(T_PPP_FSM *fsm, T_NET_BUF *output);
						/* 自構成情報を追加する。		*/
static bool_t  ipcp_ackci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* ACK を受信したときの処理	*/
static bool_t  ipcp_nakci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* NAK を受信したときの処理	*/
static bool_t  ipcp_rejci	(T_PPP_FSM *fsm, T_NET_BUF *input);
						/* REJ を受信したときの処理	*/
static int_t ipcp_reqci	(T_PPP_FSM *fsm, T_NET_BUF *input, T_NET_BUF *output);
						/* 相手の構成情報を確認する。	*/
static void ipcp_up	(T_PPP_FSM *fsm);	/* リンク接続を確立する。		*/
static void ipcp_down	(T_PPP_FSM *fsm);	/* リンク接続を解放する。		*/
static void ipcp_finished(T_PPP_FSM *fsm);	/* 下位層を終了する。		*/

/*
 *  変数
 */

static T_IFNET ipcp_local_def_cfg = {		/* 自構成情報の規定値		*/
	NULL,
	{
		IPV4_ADDR_LOCAL,		/* IPv4 アドレス			*/
		UINT_C(0),			/* サブネットマスク		*/
		},
	};

static T_IFNET ipcp_remote_def_cfg = {		/* 相手の構成情報の規定値	*/
	NULL,
	{
		IPV4_ADDR_REMOTE,		/* IPv4 アドレス			*/
		UINT_C(0),			/* サブネットマスク		*/
		}
	};

static T_IFNET ipcp_local_neg_cfg;		/* ネゴ中の自構成情報		*/

static T_IFNET ipcp_remote_neg_cfg;		/* ネゴ中の相手の構成情報		*/

/*
 *  全域変数
 */

T_PPP_FSM_CALLBACKS ipcp_callbacks = {
	ipcp_resetci,			/* 自構成情報をリセットする。	*/
	ipcp_cilen,			/* 構成情報の長さを返す。		*/
	ipcp_addci,			/* 自構成情報を追加する。		*/
	ipcp_ackci,			/* ACK を受信したときの処理	*/
	ipcp_nakci,			/* NAK を受信したときの処理	*/
	ipcp_rejci,			/* REJ を受信したときの処理	*/
	ipcp_reqci,			/* 相手の構成情報を確認する。	*/
	ipcp_up,			/* リンク接続を確立する。		*/
	ipcp_down,			/* リンク接続を解放する。		*/
	NULL,				/* 下位層を開始する。		*/
	ipcp_finished,			/* 下位層を終了する。		*/
	NULL,				/* 再送する。			*/
	NULL,				/* 不明な CP を受信したときの処理	*/
	};

T_PPP_FSM ipcp_fsm = {
	&ipcp_callbacks,		/* コールバック関数		*/
	PPP_IPCP,			/* プロトコル			*/
	};

T_PPP_PROTENT ipcp_protent = {
	PPP_IPCP,
	ipcp_init,			/* 初期化			*/
	ipcp_input,			/* 入力				*/
	ipcp_protrej,			/* Proto-REJ 受信処理		*/
	ipcp_lowerup,			/* 下位層を起動する		*/
	ipcp_lowerdown,			/* 下位層を停止する		*/
	ipcp_open,			/* オープンする			*/
	ipcp_close,			/* クローズする			*/
	ip_input,			/* データ入力、IPv4 入力	*/
	};

T_IFNET ipcp_local_ack_cfg;		/* 相手に許可されたの自構成情報	*/
T_IFNET ipcp_remote_ack_cfg;		/* 相手に許可した構成情報	*/

/*
 *  ppp_get_ifnet -- ネットワークインタフェース構造体を返す。
 */

T_IFNET *
ppp_get_ifnet (void)
{
	return &ipcp_local_ack_cfg;
	}

/*
 *  wait_ipcp -- IP 接続完了まで待つ。
 *
 *	戻り値: 接続に失敗したら E_OBJ を返す。
 */

ER
wait_ipcp (void)
{
#ifdef PPP_CFG_MODEM
	ER error;
#endif	/* of #ifdef PPP_CFG_MODEM */

	if (ipcp_fsm.state <= PPP_FSM_STOPPED) {

#ifdef PPP_CFG_MODEM

		/* モデムの接続完了まで待つ。*/
		if ((error = wait_modem()) != E_OK)
			return error;

#else	/* of #ifdef PPP_CFG_MODEM */

		/*
		 * 受動オープンし、相手からの接続待ちなら LCP を一度クローズし、
		 * 能動オープン状態で、再オープンする。
		 */
		lcp_close();
		lcp_open(PPP_OPEN_ACTIVE);

#endif	/* of #ifdef PPP_CFG_MODEM */

		}

	if (ipcp_fsm.state != PPP_FSM_OPENED) {
		/* IP 接続完了まで待つ。*/
		wai_sem(SEM_IPCP_READY);
		if (ipcp_fsm.state != PPP_FSM_OPENED)
			return E_OBJ;
		}

	return E_OK;
	}

/*
 *  ipcp_init -- IPCP モジュールの初期化
 */

static void
ipcp_init (void)
{
	fsm_init(&ipcp_fsm);
	}

/*
 *  ipcp_input -- IPCP 入力
 */

static void
ipcp_input (T_NET_BUF *input)
{
	NET_COUNT_PPP_IPCP(net_count_ppp_ipcp_in_octets,  input->len);
	NET_COUNT_PPP_IPCP(net_count_ppp_ipcp_in_packets, 1);
	fsm_input(&ipcp_fsm, input);
	}

/*
 *  ipcp_resetci -- IPCPの構成情報をリセットする。
 */

static void
ipcp_resetci (T_PPP_FSM *fsm)
{
	/* 自構成情報の初期設定 */
	ipcp_local_neg_cfg = ipcp_local_def_cfg;
	memset((void*)&ipcp_local_ack_cfg, 0, sizeof(T_IFNET));

	/* 相手の構成情報の初期設定 */
	ipcp_remote_neg_cfg = ipcp_remote_def_cfg;
	memset((void*)&ipcp_remote_ack_cfg, 0, sizeof(T_IFNET));
	}

/*
 *  ipcp_cilen -- 構成情報の長さを返す。
 */

static int_t
ipcp_cilen (T_PPP_FSM *fsm)
{
	int_t cilen = 0;

	cilen += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);	/* IPv4 アドレス */

	return cilen;
	}

/*
 *  ipcp_addci -- IPCPの構成情報を追加する。
 */

static void
ipcp_addci (T_PPP_FSM *fsm, T_NET_BUF *output)
{
	uint8_t *buf;

	buf = output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);

	/* IPv4 アドレスオプションを追加する。 */
	*buf ++ = IPCP_CIT_ADDR;
	*buf ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
	ahtonl(buf, ipcp_local_neg_cfg.in4_ifaddr.addr);
	buf += sizeof(uint32_t);
	}

/*
 *  ipcp_ackci -- ACK を受信したときの処理
 *
 *	戻り値:	true  ACK は正常
 *		false ACK は異常
 */

static bool_t
ipcp_ackci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	uint8_t		*buf;
	uint16_t	len;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	/*
	 *  構成情報は送信した順に完全に一致しなければならない。
	 *  従って、ipcp_addci で作成した順に解析する。
	 */

	/* ADDR オプションを解析する。 */
	if (len < sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)	||
	    *buf ++ != IPCP_CIT_ADDR			||
	    *buf ++ != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)||
	    nahcmpl(buf, ipcp_local_neg_cfg.in4_ifaddr.addr))
		return false;
	buf += sizeof(uint32_t);
	len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);

	return true;
	}

/*
 *  ipcp_nakci -- NAK を受信したときの処理
 *
 *	戻り値:	1  NAK は正常
 *		0  NAK は異常
 */

static int_t
ipcp_nakci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_IFNET		new_cfg;
	uint32_t	cilong;
	uint16_t	len;
	uint8_t		*buf, cilen;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	new_cfg = ipcp_local_neg_cfg;

	/* ADDR オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == IPCP_CIT_ADDR				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)) {
	    	ntoahl(cilong, buf + 2);
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		new_cfg.in4_ifaddr.addr = cilong;
		}

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
	if (fsm->state != PPP_FSM_OPENED)
		ipcp_local_neg_cfg = new_cfg;
               		
	return 1;
	}

/*
 *  ipcp_rejci -- REJ を受信したときの処理
 *
 *	戻り値:	1  REJ は正常
 *		0  REJ は異常
 */

static int_t
ipcp_rejci (T_PPP_FSM *fsm, T_NET_BUF *input)
{
	T_IFNET		new_cfg;
	uint8_t		*buf;
	uint16_t	len;

	buf = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	len = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));

	new_cfg = ipcp_local_neg_cfg;

	/* ADDR オプションを解析する。 */
	if (len >= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)		&&
	    * buf      == IPCP_CIT_ADDR				&&
	    *(buf + 1) == sizeof(T_PPP_CI_HDR) + sizeof(uint32_t)) {
		buf += sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		len -= sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
		new_cfg.in4_ifaddr.addr = 0;
		}

	/* 長さが 0 でなければエラー */
	if (len != 0)
		return 0;

	/* 新しいオプションを設定する。*/
	if (fsm->state != PPP_FSM_OPENED)
		ipcp_local_neg_cfg = new_cfg;

	return 1;
	}

/*
 *  ipcp_reqci -- 相手の構成情報を解析する。
 */

static int_t
ipcp_reqci (T_PPP_FSM *fsm, T_NET_BUF *input, T_NET_BUF *output)
{
	uint32_t	addr;
	uint16_t	code, final, len, cilen;
	uint8_t		*np, *rp, *ap, *cp, type = 0;

	rp = ap = input->buf +  sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
				/* rp: REJ、ap: ACK する CI のポインタ	*/
				/* 拒否する場合は、引数 buf に上書きする。	*/
	len   = input->len - (sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR));
	np    = output->buf + sizeof(T_PPP_HDR) + sizeof(T_PPP_CP_HDR);
	final = PPP_CONFACK;	/* 最終的に送信するコード */
	while (len > 0) {
		code = PPP_CONFACK;
		cp   = ap;	/* cp: 現在処理中の CI	*/

		if (len < sizeof(T_PPP_CI_HDR)) {

			/* 残りが CI ヘッダより短い */
			syslog(LOG_NOTICE, "[PPP/IPCP] bad CI len: %d.", len);
			cilen = len;	/* データ長が異常の場合の処置	*/
			len  = 0;	/* ループから抜ける。		*/
			code  = PPP_CONFREJ;
			goto endswitch;
			}

		type  = *ap ++;	/* CI の型		*/
		cilen = *ap ++;	/* CI の長さ		*/
		if (len < cilen) {

			/* 残りが CI 長より短い */
			syslog(LOG_NOTICE, "[PPP/IPCP] bad CI len: %d.", cilen);
			cilen = len;	/* データ長が異常の場合の処置	*/
			len  = 0;	/* ループから抜ける。		*/
			code  = PPP_CONFREJ;
			goto endswitch;
			}
		len -= sizeof(T_PPP_CI_HDR);

		/* CI の型により分岐する。*/
		switch (type) {

		case IPCP_CIT_ADDR:	/* IPv4 アドレス */

		 	/* CI 長が、ヘッダ + 4 オクテットでなければエラー */
		 	if (cilen != sizeof(T_PPP_CI_HDR) + sizeof(uint32_t))
				code = PPP_CONFREJ;
			else {
				ntoahl(addr, ap);
				if (addr != ipcp_remote_neg_cfg.in4_ifaddr.addr &&
				    (addr == 0 || ipcp_remote_neg_cfg.in4_ifaddr.addr != 0)) {
					*np ++ = IPCP_CIT_ADDR;
					*np ++ = sizeof(T_PPP_CI_HDR) + sizeof(uint32_t);
					ahtonl(np, ipcp_remote_neg_cfg.in4_ifaddr.addr);
					np  += sizeof(uint32_t);
					code = PPP_CONFNAK;
					}
#if 1	/* 要確認 */
				else if (addr == 0 && ipcp_remote_neg_cfg.in4_ifaddr.addr == 0)
#else
				else if (addr == 0 || ipcp_remote_neg_cfg.in4_ifaddr.addr == 0)
#endif
					code = PPP_CONFREJ;
				else
					ipcp_remote_neg_cfg.in4_ifaddr.addr = addr;
				}
			break;

		default:
			syslog(LOG_INFO, "[PPP/IPCP] unexp opt: %d.", type);
			code = PPP_CONFREJ;
			break;
			}
endswitch:

		if (code == PPP_CONFNAK) {
			/* CI の型が Magic Number の場合を除いて、NAK の回数が	*/
			/* 最大値 MAX_PPP_FAILURES  を超えたら拒否する。	*/
			if (fsm->failures >= MAX_PPP_FAILURES)
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
 *  ipcp_up -- リンク接続を確立する。
 */

static void
ipcp_up (T_PPP_FSM *fsm)
{
	/* 自構成情報の初期設定 */
	ipcp_local_ack_cfg = ipcp_local_neg_cfg;

	/*
	 *  サブネットマスクと
	 *  ローカル・ブロードキャスト IPv4 アドレスを設定する。
	 */
	ipcp_local_ack_cfg.in4_ifaddr.mask = MAKE_IPV4_LOCAL_MASK(ipcp_local_neg_cfg.in4_ifaddr.addr);

	/* 相手の構成情報の初期設定 */
	ipcp_remote_ack_cfg = ipcp_remote_neg_cfg;

	/*
	 *  サブネットマスクと
	 *  ローカル・ブロードキャスト IPv4 アドレスを設定する。
	 */
	ipcp_remote_ack_cfg.in4_ifaddr.mask = MAKE_IPV4_LOCAL_MASK(ipcp_remote_neg_cfg.in4_ifaddr.addr);

	sig_sem(SEM_IPCP_READY);

	syslog(LOG_NOTICE, "[PPP/IPCP] up, Local IPv4 Addr: %s, Remote IPv4 Addr: %s.",
	                   ip2str(NULL, &ipcp_local_neg_cfg.in4_ifaddr.addr),
	                   ip2str(NULL, &ipcp_remote_neg_cfg.in4_ifaddr.addr));
	}

/*
 *  ipcp_down -- リンク接続を解放する。
 */

static void
ipcp_down (T_PPP_FSM *fsm)
{
	sig_sem(SEM_IPCP_READY);
	syslog(LOG_NOTICE, "[PPP/IPCP] down.");
	}

/*
 *  ipcp_finished -- 下位層を終了する。
 */

static void
ipcp_finished (T_PPP_FSM *fsm)
{
	}

/*
 *  ipcp_protrej -- Proto-REJ を受信したときの処理
 */

static void
ipcp_protrej (void)
{
	fsm_lowerdown(&ipcp_fsm);
	}

/*
 *  ipcp_lowerup -- IPCP 下位層を起動する。
 */

static void
ipcp_lowerup (void)
{
	fsm_lowerup(&ipcp_fsm);
	}

/*
 *  ipcp_lowerdown -- IPCP 下位層を停止する。
 */

static void
ipcp_lowerdown (void)
{
	fsm_lowerdown(&ipcp_fsm);
	}

/*
 *  ipcp_open -- IPCP のオープン
 */

static void
ipcp_open (void)
{
	fsm_open(&ipcp_fsm);
	}

/*
 *  ipcp_close -- IPCP のクローズ
 */

static void
ipcp_close (void)
{
	fsm_close(&ipcp_fsm);
	}

#endif	/* fo #ifdef SUPPORT_PPP */
