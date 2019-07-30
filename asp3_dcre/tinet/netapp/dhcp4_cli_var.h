/*
 * Copyright 2004 Henning Brauer <henning@openbsd.org>
 * Copyright (c) 1995, 1996, 1997, 1998, 1999
 * The Internet Software Consortium.    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of The Internet Software Consortium nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INTERNET SOFTWARE CONSORTIUM AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE INTERNET SOFTWARE CONSORTIUM OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This software has been written for the Internet Software Consortium
 * by Ted Lemon <mellon@fugue.com> in cooperation with Vixie
 * Enterprises.  To learn more about the Internet Software Consortium,
 * see ``http://www.vix.com/isc''.  To learn more about Vixie
 * Enterprises, see ``http://www.vix.com''.
 *
 * This client was substantially modified and enhanced by Elliot Poger
 * for use on Linux while he was working on the MosquitoNet project at
 * Stanford.
 *
 * The current version owes much to Elliot's Linux enhancements, but
 * was substantially reorganized and partially rewritten by Ted Lemon
 * so as to use the same networking framework that the Internet Software
 * Consortium DHCP server uses.   Much system-specific configuration code
 * was moved into a shell script so that as support for more operating
 * systems is added, it will not be necessary to port and maintain
 * system-specific configuration code to these operating systems - instead,
 * the shell script can invoke the native tools to accomplish the same
 * purpose.
 */
/*
 * WIDE Project DHCP Implementation
 * Copyright (c) 1995-1997 Akihiro Tominaga
 * Copyright (c) 1995-1997 WIDE Project
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided the following conditions
 * are satisfied,
 *
 * 1. Both the copyright notice and this permission notice appear in
 *    all copies of the software, derivative works or modified versions,
 *    and any portions thereof, and that both notices appear in
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by WIDE Project and
 *      its contributors.
 * 3. Neither the name of WIDE Project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPER ``AS IS'' AND WIDE
 * PROJECT DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES
 * WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. ALSO, THERE
 * IS NO WARRANTY IMPLIED OR OTHERWISE, NOR IS SUPPORT PROVIDED.
 *
 * Feedback of the results generated from any improvements or
 * extensions made to this software would be much appreciated.
 * Any such feedback should be sent to:
 * 
 *  Akihiro Tominaga
 *  WIDE Project
 *  Keio University, Endo 5322, Kanagawa, Japan
 *  (E-mail: dhcp-dist@wide.ad.jp)
 *
 * WIDE project has the rights to redistribute these changes.
 */
/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2011 by Dep. of Computer Science and Engineering
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
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
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

#ifndef DHCP4_CLI_VAR_H
#define DHCP4_CLI_VAR_H

/*
 *  DHCP クライアントコンテキスト構造体
 */

/* TIMER の定義 */

#define DHCP4C_TIMER_SND_DISC	0
#define DHCP4C_TIMER_SND_REQ	1
#define DHCP4C_TIMER_SND_REL	2
#define DHCP4C_TIMER_RCV_OFFER	3
#define DHCP4C_TIMER_RCV_ACK	4
#define DHCP4C_TIMER_RENEW	5
#define DHCP4C_TIMER_REBIND	6
#define DHCP4C_TIMER_EXPIRE	7
#define NUM_DHCP4C_TIMERS	8

#define DHCP4C_TIMER_HZ		1

typedef struct dhcp4_cli_context {
	T_DHCP4_CLI_MSG	*rcv_lst;	/* 受信メッセージ構造体リスト		*/
	T_DHCP4_CLI_MSG	*val_lst;	/* 検証メッセージ構造体リスト		*/
	T_DHCP4_CLI_MSG	*prf_lst;	/* 推奨メッセージ構造体リスト		*/
	T_DHCP4_CLI_MSG	*snd_msg;	/* 送信メッセージ構造体			*/
	T_DHCP4_CLI_MSG	*off_msg;	/* 通知されたリース・メッセージ構造体	*/
	T_DHCP4_CLI_MSG	*act_msg;	/* 有効なリース・メッセージ構造体	*/
	T_IF_SOFTC	*sc;		/* IF のソフトウェア情報		*/
	T_IPV4EP	dst;		/* 送信先のIPアドレス/ポート番号	*/
	SYSTIM		snd_start;	/* 送信を開始した時間			*/
	SYSTIM		bind_start;	/* BINDを開始した時間			*/
	SYSTIM		expire;		/* リースの有効時間（ms）		*/
	SYSTIM		renew;		/* RENEW（T1）までの時間（ms）	*/
	SYSTIM		rebind;		/* REBIND（T2）までの時間（ms）	*/
	SYSTIM		timers[NUM_DHCP4C_TIMERS];	
					/* タイマー配列			*/
	RELTIM		interval;	/* 送信間隔			*/
	ER_UINT		error;		/* エラー・データ長		*/
	ID			tskid;		/* タスクID */
	ID			cepid;		/* CEP ID			*/
	uint32_t	xid;		/* トランザクション ID		*/
	uint16_t	flags;		/* 各種フラグ			*/
	uint16_t	max_prefer;	/* 受信メッセージの最大推奨度	*/
	uint16_t	secs;		/* seconds since boot began	*/
	uint8_t		fsm;		/* 現在の状態			*/
	uint8_t		req;
	uint8_t		sig;
	int timer;
	} T_DHCP4_CLI_CONTEXT;

/* fsm（状態遷移）の定義 */

#define DHCP4_FSM_INIT		1
#define DHCP4_FSM_SELECT	2
#define DHCP4_FSM_REQUEST	3
#define DHCP4_FSM_BOUND		4
#define DHCP4_FSM_RENEW		5
#define DHCP4_FSM_REBIND	6
#define DHCP4_FSM_INIT_REBOOT	7
#define DHCP4_FSM_REBOOT	8
#define DHCP4_FSM_REL_INFO	11	/* DHCPv4 の RFC2131 には無い。*/
#define DHCP4_FSM_SLEEP		12	/* DHCPv4 の RFC2131 には無い。*/

/* flag の定義 */

#define DHCP4C_FLAG_TMOUT_SND_DISC	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_SND_DISC)
#define DHCP4C_FLAG_TMOUT_SND_REQ	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_SND_REQ)
#define DHCP4C_FLAG_TMOUT_SND_REL	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_SND_REL)
#define DHCP4C_FLAG_TMOUT_RCV_OFFER	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_RCV_OFFER)
#define DHCP4C_FLAG_TMOUT_RCV_ACK	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_RCV_ACK)
#define DHCP4C_FLAG_TMOUT_EXPIRE	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_EXPIRE)
#define DHCP4C_FLAG_TMOUT_RENEW		(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_RENEW)
#define DHCP4C_FLAG_TMOUT_REBIND	(DHCP4C_FLAG_TMOUT_TIMER|DHCP4C_TIMER_REBIND)
#define DHCP4C_FLAG_TMOUT_TIMER		UINT_C(0x0010)
#define DHCP4C_FLAG_TMOUT_TIMER_MASK	UINT_C(0x000f)
#define DHCP4C_FLAG_TMOUT_MASK		UINT_C(0x001f)

#define DHCP4C_FLAG_RCV_MSG		UINT_C(0x0200)
#define DHCP4C_FLAG_RENEW		UINT_C(0x0400)

/*
 *  関数シミュレーションマクロ
 */

/* タイムアウトの計算 */

#define DHCP4C_TMOC(t)		(((t)-1000)+dhcp_rand()%2000)

/* 時間比較 */

#define TIMEC_GT(a,b)		((int32_t)((a)-(b))>0)
#define TIMEC_GE(a,b)		((int32_t)((a)-(b))>=0)

/* 時間定数の変換 */

#define SYSTIM2TIMER(t)		(((t)<SYSTIM_HZ/DHCP4C_TIMER_HZ)?(DHCP4C_TIMER_HZ):((t)*DHCP4C_TIMER_HZ)/SYSTIM_HZ)
#define SYSTIM2SEC(t)		(((t)<SYSTIM_HZ)?1:((t)/SYSTIM_HZ))
#define SEC2TIMER(t)		((t)*DHCP4C_TIMER_HZ)
#define SEC2SYSTIM(t)		((t)*SYSTIM_HZ)

#endif	/* of #ifndef DHCP4_CLI_VAR_H */
