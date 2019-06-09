/*
 * Copyright (c) 2004-2011 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1995-2003 by Internet Software Consortium
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Internet Systems Consortium, Inc.
 *   950 Charter Street
 *   Redwood City, CA 94063
 *   <info@isc.org>
 *   https://www.isc.org/
 *
 * This code is based on the original client state machine that was
 * written by Elliot Poger.  The code has been extensively hacked on
 * by Ted Lemon since then, so any mistakes you find are probably his
 * fault and not Elliot's.
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

#ifndef DHCP6_CLI_VAR_H
#define DHCP6_CLI_VAR_H

#ifdef DHCP6_CLI_CFG

/*
 *  DHCP クライアントコンテキスト構造体
 */

/* TIMER の定義 */

#define DHCP6C_TIMER_SND_SOL	0
#define DHCP6C_TIMER_SND_REQ	1
#define DHCP6C_TIMER_SND_REL	2
#define DHCP6C_TIMER_RCV_ADV	3
#define DHCP6C_TIMER_RCV_REPLY	4
#define DHCP6C_TIMER_RENEW	5
#define DHCP6C_TIMER_REBIND	6
#define DHCP6C_TIMER_EXPIRE	7
#define DHCP6C_TIMER_DEPREF	8
#define DHCP6C_TIMER_SND_REF	9
#define DHCP6C_TIMER_SND_IREQ	10
#define NUM_DHCP6C_TIMERS	11

#define DHCP6C_TIMER_HZ		1

/* 定数の定義 */

#define MAX_DUID_LEN		(ETHER_ADDR_LEN+8)

typedef struct dhcp6_cli_context {
	T_DHCP6_CLI_MSG		*rcv_lst;	/* 受信メッセージ構造体リスト		*/
	T_DHCP6_CLI_MSG		*val_lst;	/* 検証メッセージ構造体リスト		*/
	T_DHCP6_CLI_MSG		*prf_lst;	/* 推奨メッセージ構造体リスト		*/
	T_DHCP6_CLI_MSG		*snd_msg;	/* 送信メッセージ構造体			*/
	T_DHCP6_CLI_MSG		*adv_msg;	/* 通知されたリース・メッセージ構造体	*/
	T_DHCP6_CLI_MSG		*act_msg;	/* 有効なリース・メッセージ構造体	*/
	T_IF_SOFTC		*sc;		/* IF のソフトウェア情報		*/
	T_IPV6EP		dst;		/* 送信先のIPアドレス/ポート番号	*/
	SYSTIM			RT;		/* 再送信間隔				*/
	SYSTIM			IRT;		/* 初期再送信間隔			*/
	SYSTIM			MRT;		/* 最大再送信間隔			*/
	SYSTIM			MRD;		/* 最大再送信継続時間			*/
	SYSTIM			next_MRD;	/* 次の最大再送信継続時間		*/
	SYSTIM			timers[NUM_DHCP6C_TIMERS];	
						/* タイマー配列			*/
	ER_UINT			error;		/* エラー・データ長		*/
	ID			cepid;		/* CEP ID			*/
	uint32_t		xid;		/* トランザクション ID		*/
	uint16_t		flags;		/* 各種フラグ			*/
	uint16_t		max_prefer;	/* 受信メッセージの最大推奨度	*/
	uint8_t			duid[MAX_DUID_LEN];	
						/* client DHCP Unique ID	*/
	uint8_t			duid_len;	/* client DUID 長		*/
	uint8_t			fsm;		/* 現在の状態			*/
	uint8_t			MRC;		/* 最大再送信回数		*/
	uint8_t			txcount;	/* 再送信回数			*/
	uint8_t			refresh_type;	/* REFRESH のメッセージタイプ	*/
	} T_DHCP6_CLI_CONTEXT;

/* fsm（状態遷移）の定義 */

#define DHCP6_FSM_INIT		1
#define DHCP6_FSM_SELECT	2
#define DHCP6_FSM_BOUND		4
#define DHCP6_FSM_RENEW		5
#define DHCP6_FSM_REBIND	6
#define DHCP6_FSM_REQ_INFO	9	/* DHCPv6 の RFC3315 には無い。*/
#define DHCP6_FSM_INFORMED	10	/* DHCPv6 の RFC3315 には無い。*/
#define DHCP6_FSM_REL_INFO	11	/* DHCPv6 の RFC3315 には無い。*/
#define DHCP6_FSM_SLEEP		12	/* DHCPv6 の RFC3315 には無い。*/

/* flag の定義 */

#define DHCP6C_FLAG_TMOUT_SND_SOL	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_SND_SOL)
#define DHCP6C_FLAG_TMOUT_SND_REQ	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_SND_REQ)
#define DHCP6C_FLAG_TMOUT_SND_REL	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_SND_REL)
#define DHCP6C_FLAG_TMOUT_RCV_ADV	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_RCV_ADV)
#define DHCP6C_FLAG_TMOUT_RCV_REPLY	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_RCV_REPLY)
#define DHCP6C_FLAG_TMOUT_RENEW		(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_RENEW)
#define DHCP6C_FLAG_TMOUT_REBIND	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_REBIND)
#define DHCP6C_FLAG_TMOUT_EXPIRE	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_EXPIRE)
#define DHCP6C_FLAG_TMOUT_DEPREF	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_DEPREF)
#define DHCP6C_FLAG_TMOUT_SND_REF	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_SND_REF)
#define DHCP6C_FLAG_TMOUT_SND_IREQ	(DHCP6C_FLAG_TMOUT_TIMER|DHCP6C_TIMER_SND_IREQ)
#define DHCP6C_FLAG_TMOUT_TIMER		UINT_C(0x0010)
#define DHCP6C_FLAG_TMOUT_TIMER_MASK	UINT_C(0x000f)
#define DHCP6C_FLAG_TMOUT_MASK		UINT_C(0x001f)

#define DHCP6C_FLAG_RCV_MSG		UINT_C(0x0200)
#define DHCP6C_FLAG_RENEW		UINT_C(0x0400)

/* IAADDR 構造体の FLAG の定義（len に多重化する）*/

#define DHCP6C_IAA_FLAG_EXPIRED		UINT_C(0x8000)
#define DHCP6C_IAA_FLAG_DEPREFERD	UINT_C(0x4000)
#define DHCP6C_IAA_FLAG_MASK		UINT_C(0xc000)

/*
 *  関数シミュレーションマクロ
 */

/* 時間比較 */

#define TIMEC_GT(a,b)		((int32_t)((a)-(b))>0)
#define TIMEC_GE(a,b)		((int32_t)((a)-(b))>=0)

/* 時間定数の変換 */

#define SYSTIM2TIMER(t)		(((t)<SYSTIM_HZ/DHCP6C_TIMER_HZ)?(DHCP6C_TIMER_HZ):((t)*DHCP6C_TIMER_HZ)/SYSTIM_HZ)
#define SYSTIM2SEC(t)		(((t)<SYSTIM_HZ)?1:((t)/SYSTIM_HZ))
#define SEC2TIMER(t)		((t)*DHCP6C_TIMER_HZ)
#define SEC2SYSTIM(t)		((t)*SYSTIM_HZ)

#endif	/* of #ifdef DHCP6_CLI_CFG */

#endif	/* of #ifndef DHCP6_CLI_VAR_H */
