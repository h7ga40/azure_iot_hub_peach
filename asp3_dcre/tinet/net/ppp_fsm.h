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
 * fsm.h - {Link, IP} Control Protocol Finite State Machine definitions.
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
 *
 * $FreeBSD: src/usr.sbin/pppd/fsm.h,v 1.6.2.1 1999/08/29 15:47:00 peter Exp $
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
 *
 * $FreeBSD: src/usr.sbin/ppp/fsm.h,v 1.23 1999/12/27 11:54:53 brian Exp $
 *
 *	TODO:
 */

#ifndef _PPP_FSM_H_
#define _PPP_FSM_H_

#ifdef SUPPORT_PPP

/*
 *  状態管理構造体
 */

struct t_ppp_fsm_callbacks;

typedef struct t_fsm {
	struct t_ppp_fsm_callbacks	*callbacks;	/* コールバック関数*/
	uint16_t	proto;		/* プロトコル				*/
	uint8_t		state;		/* FSM の状態				*/
	uint8_t		id;		/* 現在の ID				*/
	uint8_t		reqid;		/* 要求 ID				*/
	uint8_t		seen_ack;	/* 要求に対し正常な ACK/NAK/REJ を受信	*/
	int8_t		failures;	/* 黙認した NAK 数			*/
	int8_t		retrans;	/* Configure-Request または、		*/
					/* Terminate-Request の再送回数		*/
	} T_PPP_FSM;

#define T_PPP_FSM_DEFINED

/*
 *  コールバック関数構造体
 */

typedef struct t_ppp_fsm_callbacks {
	void (*resetci)	(T_PPP_FSM *);	/* 自構成情報をリセットする。	*/
	int_t  (*cilen)	(T_PPP_FSM *);	/* 構成情報の長さを返す。		*/
	void (*addci)	(T_PPP_FSM *, T_NET_BUF *);
					/* 自構成情報を追加する。		*/
	bool_t (*ackci)	(T_PPP_FSM *, T_NET_BUF *);
					/* ACK を受信したときの処理	*/
	int_t  (*nakci)	(T_PPP_FSM *, T_NET_BUF *);
					/* NAK を受信したときの処理	*/
	int_t  (*rejci)	(T_PPP_FSM *, T_NET_BUF *);
					/* REJ を受信したときの処理	*/
	int_t  (*reqci)	(T_PPP_FSM *, T_NET_BUF *, T_NET_BUF *);
					/* 相手の構成情報を確認する。	*/
	void (*up)	(T_PPP_FSM *);	/* リンクの接続を確立する。	*/
	void (*down)	(T_PPP_FSM *);	/* リンクの接続を解放する。	*/
	void (*starting)(T_PPP_FSM *);	/* 下位層を開始する。		*/
	void (*finished)(T_PPP_FSM *);	/* 下位層を終了する。		*/
	void (*retrans)	(T_PPP_FSM *);	/* 再送する。			*/
					/* 不明な CP を受信したときの処理	*/
	bool_t  (*extcode)(T_PPP_FSM *, uint8_t, uint8_t, T_NET_BUF *);
	} T_PPP_FSM_CALLBACKS;

/*
 *  FSM の状態
 */

#define PPP_FSM_INITIAL		0	/* 未接続、未オープン	*/
#define PPP_FSM_STARTING	1	/* 未接続、オープン	*/
#define PPP_FSM_CLOSED		2	/* 接続、  未オープン	*/
#define PPP_FSM_STOPPED		3	/* オープン、切断待ち	*/
#define PPP_FSM_CLOSING		4	/* 切断中、未オープン	*/
#define PPP_FSM_STOPPING	5	/* 切断中、オープン	*/
#define PPP_FSM_REQSENT		6	/* Config REQ 送信済み	*/
#define PPP_FSM_ACKRCVD		7	/* Config ACK 受信済み	*/
#define PPP_FSM_ACKSENT		8	/* Config ACK 送信済み	*/
#define PPP_FSM_OPENED		9	/* 接続、通信可能	*/

/*
 *  オープン時の動作モード
 */

#define PPP_OPEN_PASSIVE	false	/* 受動オープン		*/
#define PPP_OPEN_ACTIVE		true	/* 能動オープン		*/

/*
 *  再送、タイムアウトの既定値
 */

#define DEF_PPP_TIMEOUT		(3*NET_TIMER_HZ)
					/* RFC1661 では  3 秒			*/
#define MAX_PPP_TERMINATES	2	/* Terminate-Requests の最大再送回数	*/
					/* RFC1661 では  2 回			*/
#define MAX_PPP_CONFIGURES	10	/* Configure-Requests の最大再送回数	*/
					/* RFC1661 では 10 回			*/
#define MAX_PPP_FAILURES	5	/* Configure-NAK の最大黙認回数		*/
					/* RFC1661 では  5 回			*/

/*
 *  関数
 */

extern void fsm_init    (T_PPP_FSM *fsm);
extern void fsm_lowerup (T_PPP_FSM *fsm);
extern void fsm_lowerdown(T_PPP_FSM *fsm);
extern void fsm_open    (T_PPP_FSM *fsm);
extern void fsm_close	(T_PPP_FSM *fsm);
extern void fsm_input	(T_PPP_FSM *fsm, T_NET_BUF *data);
extern void fsm_protreject(T_PPP_FSM *fsm);

/*
 *  変数
 */

extern uint8_t ppp_open_mode;

#endif	/* of #ifdef SUPPORT_PPP */

#endif	/* of #ifndef _PPP_FSM_H_ */
