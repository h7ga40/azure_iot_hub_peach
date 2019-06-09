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

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>

#include <net/if_var.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/dhcp6_cli.h>
#include <netapp/dhcp6_cli_var.h>
#include <netapp/resolver.h>

/*
 *  DHCPv6 クライアントタスク
 */

#ifdef DHCP6_CLI_CFG

/*
 *  DHCP 動作モード
 *
 *    #define DHCP6_CLI_CFG_MODE で指定する。
 *
 */

#if !defined(DHCP6_CLI_CFG_MODE)
#define DHCP6_CLI_CFG_MODE	DHCP6_CLI_CFG_MODE_DEFAULT
#endif

/*
 *  変数
 */

static const T_IN6_ADDR in6_addr_all_DHCP_relay_servers =
	IPV6_ADDR_ALL_DHCP_RELAY_SERVERS_INIT;			/* リレー・サーバの IPv6 アドレス	*/
static T_DHCP6_CLI_CONTEXT	context;			/* クライアントコンテキスト	*/

#ifdef DHCP6_CLI_CFG_REQUIRED_OLIST
static uint8_t	required_plist[] = DHCP6_CLI_CFG_REQUIRED_OLIST;	/* 必須オプションリスト		*/
#endif

#ifdef DHCP6_CLI_CFG_REQUEST_OLIST
static uint8_t	request_plist[] = DHCP6_CLI_CFG_REQUEST_OLIST;		/* 要求オプションリスト		*/
#endif

/*
 *  dhcp6_rand -- RFC3315 に規定された乱数を発生する。
 */

static int32_t
dhcp6_rand (SYSTIM base)
{
	SYSTIM 		range, split;
	uint32_t	rval;

	split = (base - 1) / 10;
	if (split == 0)
		return 0;
	range = (split * 2) + 10;
	rval = netapp_rand() % range - split;

	return rval;
	}

/*
 *  init_duid -- クライアントの DUID を設定する。
 */

static void
init_duid (T_DHCP6_CLI_CONTEXT *ct)
{

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS

	/* 動作モードに STATELESS が指定された時は LL 形式にする。*/

	T_DHCP6_DUID_LL_HDR	*ll;

	ll = (T_DHCP6_DUID_LL_HDR*)&ct->duid[0];
	HOST2MSG16(ll->code, DUID_LL);
	HOST2MSG16(ll->type, DHCP6_HWA_TYPE_ETHER);
	memcpy(&ct->duid[sizeof(*ll)], &ct->sc->ifaddr.lladdr, sizeof(ct->sc->ifaddr.lladdr));
	ct->duid_len = sizeof(*ll) + sizeof(ct->sc->ifaddr.lladdr);	/* DIUD 長を記録する。*/

#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

	/*
	 *    動作モードに DHCP6_CLI_CFG_STATEFULL が指定された時は LLT 形式にする。
	 *    time メンバーには現在の時間[s]を設定する。
	 *    注意: 暫定的に get_tim() の値を使うが、要検討
	 */

	T_DHCP6_DUID_LLT_HDR	*llt;
	SYSTIM			time;

	get_tim(&time);
	time = SYSTIM2SEC(time) - DUID_TIME_EPOCH;
	llt = (T_DHCP6_DUID_LLT_HDR*)&ct->duid[0];
	HOST2MSG16(llt->code, DUID_LLT);
	HOST2MSG16(llt->type, DHCP6_HWA_TYPE_ETHER);
	HOST2MSG32(llt->time, time);
	memcpy(&ct->duid[sizeof(*llt)], &ct->sc->ifaddr.lladdr, sizeof(ct->sc->ifaddr.lladdr));
	ct->duid_len = sizeof(*llt) + sizeof(ct->sc->ifaddr.lladdr);	/* DIUD 長を記録する。*/

#else	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

	}

/*
 *  cancel_all_timers -- 全ての TIMER をキャンセルする。
 */

static void
cancel_all_timers (T_DHCP6_CLI_CONTEXT *ct)
{
	int ix;

	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	for (ix = NUM_DHCP6C_TIMERS; ix -- > 0; )
		ct->timers[ix] = 0;
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
	}

/*
 *  dhcpc_timer -- TIMER 管理
 */

static void
dhcpc_timer (T_DHCP6_CLI_CONTEXT *ct)
{
	int ix;

	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	for (ix = NUM_DHCP6C_TIMERS; ix -- > 0; ) {
		if (ct->timers[ix] != 0) {
			ct->timers[ix] --;
			if (ct->timers[ix] == 0) {
				ct->flags = (ct->flags & ~DHCP6C_FLAG_TMOUT_MASK) | DHCP6C_FLAG_TMOUT_TIMER | ix;
				syscall(sig_sem(SEM_DHCP6_CLI_READY));
				}
			}
		}
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 1秒毎にタイムアウトする。*/
	timeout((callout_func)dhcpc_timer, ct, NET_TIMER_HZ / DHCP6C_TIMER_HZ);
	}

/*
 *  find_option -- 指定したオプションを探索する。
 *
 *    注意: 引数 len が 0 の時はオプションサイズを検証しない。
 *          引数 len にはオプション全体のサイズを指定すること。
 */

static uint8_t *
find_option (uint8_t *msg, uint_t msg_len, uint_t code, uint_t len)
{
	T_DHCP6_OPTION	*hdr;
	uint8_t		*last;

	last = msg + msg_len;
	while (msg < last) {
		hdr = (T_DHCP6_OPTION*)msg;
		if (MSG2HOST16(hdr->code) == code && (len == 0 || (MSG2HOST16(hdr->len) + sizeof(*hdr)) == len))
			return msg;
		else
			msg += MSG2HOST16(hdr->len) + sizeof(*hdr);
		}
	return NULL;
	}

/*
 *  find_msg_option -- 指定したオプションを探索する。
 *
 *    注意: 引数 len が 0 の時はオプションサイズを検証しない。
 *          引数 len にはオプション全体のサイズを指定すること。
 */

static uint8_t *
find_msg_option (T_DHCP6_CLI_MSG *cli_msg, uint_t code, uint_t len)
{
	return find_option(cli_msg->msg.options, cli_msg->len, code, len);
	}

/*
 *  is_equal_sid -- メッセージ内の SERVER ID を比較する。
 */

static bool_t
is_equal_sid (T_DHCP6_CLI_MSG *msg1, T_DHCP6_CLI_MSG *msg2)
{
	T_DHCP6_OPT_SERVERID	*sid1;
	T_DHCP6_OPT_SERVERID	*sid2;

	if ((sid1 = (T_DHCP6_OPT_SERVERID*)find_msg_option(msg1, DHCP6_OPT_SERVERID, 0)) == NULL)
		return false;
	if ((sid2 = (T_DHCP6_OPT_SERVERID*)find_msg_option(msg2, DHCP6_OPT_SERVERID, 0)) == NULL)
		return false;
	if (MSG2HOST16(sid1->len) != MSG2HOST16(sid2->len))
		return false;

	return memcmp((uint8_t*)sid1 + sizeof(*sid1),
	              (uint8_t*)sid2 + sizeof(*sid2), MSG2HOST16(sid1->len)) == 0;
	}

/*
 *  validate_iaaddrs -- アドレス情報（IAADDR）を検証する。
 */

static ER
validate_iaaddrs (uint8_t *opt, uint_t len)
{
	T_DHCP6_OPT_IAADDR	*iaa;

	/* オプションが IAADDR であることを検証する。*/
	iaa = (T_DHCP6_OPT_IAADDR*)opt;
	if (MSG2HOST16(iaa->code) != DHCP6_OPT_IAADDR)
		return E_PAR;

	while (len > 0) {
		/* オプション長を検証する。*/
		if (len < sizeof(*iaa))
			return E_PAR;

		/* オプション長が T_DHCP6_OPT_IAADDR のサイズと一致していなければエラー */
		iaa = (T_DHCP6_OPT_IAADDR*)opt;
		if ((MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) < (sizeof(*iaa) - sizeof(T_DHCP6_OPTION)))
			return E_PAR;

		/*
		 *  perferred-lifetime と valid-lifetime の値が矛盾しているときはエラー
		 *  【RFC3315 (22.6) 参照】
		 */
		if (MSG2HOST32(iaa->prefer) > MSG2HOST32(iaa->valid))
			return E_PAR;

		len -= (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(T_DHCP6_OPTION);
		opt += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(T_DHCP6_OPTION);
		}

	return E_OK;
	}

/*
 *  validate_ia_na_pd -- IA_NA/IA_PD オプションを検証する。
 */

static ER
validate_ia_na_pd (T_DHCP6_CLI_MSG *msg, uint16_t code)
{
	T_DHCP6_OPT_IA_NA_PD	*na;
	uint8_t			*opt;

	/* IA_NA/IA_PD が無ければ検証しない。*/
	if ((opt = find_msg_option(msg, code, 0)) == NULL)
		return E_OK;

	/* オプション長を検証する。*/
	na = (T_DHCP6_OPT_IA_NA_PD*)opt;
	if (!(msg->len >= (opt - (uint8_t*)&msg->msg) + sizeof(*na) &&
	                         (MSG2HOST16(na->len) & ~DHCP6C_IAA_FLAG_MASK) >= sizeof(*na)))
		return E_PAR;

	/*
	 *  T1 と T2 の値が矛盾している時、またはアドレス情報が誤っている時はエラー
	 *  【RFC3315 (22.4) 参照】
	 */
	if (MSG2HOST32(na->renew) > 0 && MSG2HOST32(na->rebind) > 0 &&
	    MSG2HOST32(na->renew) >      MSG2HOST32(na->rebind))
		return E_PAR;

	if (validate_iaaddrs(opt + sizeof(*na), (MSG2HOST16(na->len) & ~DHCP6C_IAA_FLAG_MASK) - (sizeof(*na) - sizeof(T_DHCP6_OPTION))) != E_OK)
		return E_PAR;

	return E_OK;
	}

/*
 *  validate_ia_ta -- IA_TA オプションを検証する。
 */

static ER
validate_ia_ta (T_DHCP6_CLI_MSG *msg)
{
	T_DHCP6_OPT_IA_TA	*ta;
	uint8_t			*opt;

	/* IA_TA が無ければ検証しない。*/
	if ((opt = find_msg_option(msg, DHCP6_OPT_IA_TA, 0)) == NULL)
		return E_OK;

	/* オプション長を検証する。*/
	ta = (T_DHCP6_OPT_IA_TA	*)opt;
	if (!(msg->len >= (opt - (uint8_t*)&msg->msg) + sizeof(*ta) &&
	                         (MSG2HOST16(ta->len) & ~DHCP6C_IAA_FLAG_MASK) >= sizeof(*ta)))
		return E_PAR;

	/*
	 *  アドレス情報が誤っている時はエラー【RFC3315 (22.4) 参照】
	 */
	if (validate_iaaddrs(opt + sizeof(*ta), (MSG2HOST16(ta->len) & ~DHCP6C_IAA_FLAG_MASK) - (sizeof(*ta) - sizeof(T_DHCP6_OPTION))) != E_OK)
		return E_PAR;

	return E_OK;
	}
/*
 *  classify_reply -- 応答メッセージを分類する。
 */

static ER
classify_reply (T_DHCP6_CLI_MSG *msg)
{
	ER	error;

	/* IA_NA オプションを検証する。*/
	if ((error = validate_ia_na_pd(msg, DHCP6_OPT_IA_NA)) != E_OK)
		return error;

	/* IA_TA オプションを検証する。*/
	if ((error = validate_ia_ta(msg)) != E_OK)
		return error;

	/* IA_PD オプションを検証する。*/
	if ((error = validate_ia_na_pd(msg, DHCP6_OPT_IA_PD)) != E_OK)
		return error;

	/* サーバ DUID があることを検証する。*/
	if (find_msg_option(msg, DHCP6_OPT_SERVERID, 0) == NULL)
		return E_PAR;

	return E_OK;
	}

/*
 *  validate_reply -- 応答メッセージを検証する。
 */

static ER
validate_reply (T_DHCP6_CLI_CONTEXT *ct, T_DHCP6_CLI_MSG *msg)
{
	T_DHCP6_OPT_CLIENTID	*cid;
	uint8_t			*opt;

	/* サーバ DUID があることを検証する。*/
	if (find_msg_option(msg, DHCP6_OPT_SERVERID, 0) == NULL)
		return E_PAR;

	/* クライアント DUID があり、自分のクライアント DUID と同じことを検証する。*/
	if ((opt = find_msg_option(msg, DHCP6_OPT_CLIENTID, 0)) == NULL)
		return E_PAR;

	cid = (T_DHCP6_OPT_CLIENTID*)opt;
	if (MSG2HOST16(cid->len) != ct->duid_len)
		return E_PAR;
	if (memcmp(opt + sizeof(*cid), ct->duid, ct->duid_len) != 0)
		return E_PAR;

	return E_OK;
	}

/*
 *  get_status_code -- STATUS オプションの結果コードを獲得する。
 */

static ER_UINT
get_status_code (uint8_t *msg, uint_t msg_len)
{
	T_DHCP6_OPT_STATUS_CODE	*status;
	uint8_t			*opt;

	/* STATUS オプションが無ければ検証しない。*/
	if ((opt = find_option(msg, msg_len, DHCP6_OPT_STATUS_CODE, 0)) == NULL)
		return DHCP6_STAT_SUCCESS;

	/* オプション長を検証する。*/
	status = (T_DHCP6_OPT_STATUS_CODE*)opt;
	if (msg_len < sizeof(*status))
		return E_PAR;
	if (MSG2HOST16(status->len) < sizeof(*status) - sizeof(T_DHCP6_OPTION))
		return E_PAR;

	return MSG2HOST16(status->status);
	}

/*
 *  get_msg_status_code -- STATUS オプションの結果コードを獲得する。
 */

static ER_UINT
get_msg_status_code (T_DHCP6_CLI_MSG *cli_msg, uint_t *code)
{
	T_DHCP6_OPTION		*opt;
	T_DHCP6_OPT_IA_NA_PD	*ia;
	T_DHCP6_OPT_IA_TA	*ta;
	ER_UINT			status;
	uint8_t			*msg, *last;

	*code = 0;
	cli_msg->status = DHCP6_CLI_STAT_UNDEFINED;
	if ((status = get_status_code(cli_msg->msg.options, cli_msg->len)) == DHCP6_STAT_SUCCESS) {

		msg = cli_msg->msg.options;
		last = msg + (cli_msg->len - sizeof(T_DHCP6_MSG_HDR));
		while (msg < last) {

			opt = (T_DHCP6_OPTION*)msg;
			status = get_status_code(msg + sizeof(*opt), MSG2HOST16(opt->len));
			if (status != DHCP6_STAT_SUCCESS) {
				*code = MSG2HOST16(opt->code);
				break;
				}

			if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_NA || MSG2HOST16(opt->code) == DHCP6_OPT_IA_PD) {

				ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
				status = get_status_code(msg + sizeof(*opt), MSG2HOST16(ia->len));
				if (status != DHCP6_STAT_SUCCESS) {
					*code = MSG2HOST16(((T_DHCP6_OPTION*)((uint8_t*)opt + sizeof(*opt)))->code);
					break;
					}
				}

			else if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_TA) {

				ta = (T_DHCP6_OPT_IA_TA*)opt;
				status = get_status_code(msg + sizeof(*opt), MSG2HOST16(ta->len));
				if (status != DHCP6_STAT_SUCCESS) {
					*code = MSG2HOST16(((T_DHCP6_OPTION*)((uint8_t*)opt + sizeof(*opt)))->code);
					break;
					}
				}

			msg += MSG2HOST16(opt->len) + sizeof(*opt);
			}
		cli_msg->status = status;
		}
	
	return status;
	}

/*
 *  validate_rcv_msg -- 受信メッセージを検証する。
 */

static ER
validate_rcv_msg (T_DHCP6_CLI_CONTEXT *ct, T_DHCP6_CLI_MSG *msg)
{
	ER	error;
	uint_t	code;

	/* メッセージのタイプが一致していることを検証する。*/
	switch (ct->fsm) {

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	case DHCP6_FSM_SELECT:
		if (msg->msg.hdr.type != DHCP6_REPLY)
			return E_PAR;
		break;
	case DHCP6_FSM_INFORMED:
		return E_PAR;
		break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	case DHCP6_FSM_INIT:
		if (msg->msg.hdr.type != DHCP6_ADVERTISE)
			return E_PAR;
		break;

	case DHCP6_FSM_SELECT:
	case DHCP6_FSM_RENEW:
	case DHCP6_FSM_REBIND:
	case DHCP6_FSM_REL_INFO:
		if (msg->msg.hdr.type != DHCP6_REPLY)
			return E_PAR;
		break;

	case DHCP6_FSM_BOUND:
		return E_PAR;
		break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

	default:
		syslog(LOG_NOTICE, "[DHCP6C] error, unknown state: %d.", ct->fsm);
		return E_PAR;
		break;
		}

	/* 受信メッセージを検証する。*/
	if ((error = validate_reply(ct, msg)) != E_OK)
		return error;

	/* 受信メッセージを分類する。*/
	if ((error = classify_reply(msg)) != E_OK)
		return error;

	/* 受信したメッセージ内の STATUS CODE オプションを検証する。*/
	error = get_msg_status_code(msg, &code);
	if (error != E_OK)
		return error > 0 ? E_PAR : error;

	return E_OK;
	}

/*
 *  eval_ia_prefer -- IA_NA/IA_TA/IA_PD の推奨度を評価する。
 */

static void
eval_ia_prefer (T_DHCP6_CLI_MSG *cli_msg)
{
	T_DHCP6_OPTION		*opt;
	T_DHCP6_OPT_IA_NA_PD	*ia;
	T_DHCP6_OPT_IA_TA	*ta;
	T_DHCP6_OPT_IAADDR	*iaa;
	uint8_t		*msg, *last;
	uint_t		aoff, asize;

	/* 無効の場合は終了する。*/
	if (cli_msg == NULL)
		return;

	msg = cli_msg->msg.options;
	last = msg + (cli_msg->len - sizeof(T_DHCP6_MSG_HDR));
	while (msg < last) {
		opt = (T_DHCP6_OPTION*)msg;

		if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_NA || MSG2HOST16(opt->code) == DHCP6_OPT_IA_PD) {

			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			aoff  = sizeof(*ia);
			asize = MSG2HOST16(ia->len) + sizeof(*opt);
			while (aoff < asize) {
				cli_msg->prefer += DHCP6_CLI_CFG_PREFER_IAADDR;
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				aoff += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(*iaa);
				}
			cli_msg->prefer = (cli_msg->prefer + DHCP6_CLI_CFG_PREFER_IA) | DHCP6_ClI_MSG_PREFER_ADDR;
			}

		else if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_TA) {

			ta = (T_DHCP6_OPT_IA_TA*)opt;
			aoff  = sizeof(*ta);
			asize = MSG2HOST16(ta->len) + sizeof(*opt);
			while (aoff < asize) {
				cli_msg->prefer += DHCP6_CLI_CFG_PREFER_IAADDR;
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				aoff += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(*iaa);
				}
			cli_msg->prefer = (cli_msg->prefer + DHCP6_CLI_CFG_PREFER_IA) | DHCP6_ClI_MSG_PREFER_ADDR;
			}

		msg += MSG2HOST16(opt->len) + sizeof(*opt);
		}

	}

/*
 *  eval_prefer -- 受信メッセージの推奨度を評価する。
 */

static void
eval_prefer (T_DHCP6_CLI_MSG *msg)
{
	uint8_t			*opt;
	int			ix;

	/* 無効の場合は終了する。*/
	if (msg == NULL)
		return;

	/* PREFERENCE オプションを評価する。*/
	if ((opt = find_msg_option(msg, DHCP6_OPT_PREFERENCE, sizeof(T_DHCP6_OPT_PREFERENCE))) != NULL)
		msg->prefer = MSG2HOST16(((T_DHCP6_OPT_PREFERENCE*)opt)->prefer) | DHCP6_ClI_MSG_PREFER_OPTION;
	else
		msg->prefer = 1;

#if defined(DHCP6_CLI_CFG_REQUIRED_OLIST)

	/* 必須オプションを検証する。*/
	for (ix = sizeof(required_plist); ix -- > 0; ) {
		if (find_msg_option(msg, required_plist[ix], 0) == NULL)
			msg->prefer = 0;
		}
	msg->prefer |= DHCP6_ClI_MSG_PREFER_REQUIRED;

#endif	/* of #if defined(DHCP6_CLI_CFG_REQUIRED_OLIST) */

#if defined(DHCP6_CLI_CFG_REQUEST_OLIST)

	/* 要求オプションを検証する。*/
	for (ix = sizeof(request_plist); ix -- > 0; ) {
		if (find_msg_option(msg, request_plist[ix], 0) != NULL)
			msg->prefer = (msg->prefer + 1) | DHCP6_ClI_MSG_PREFER_REQUEST;
		}

#endif	/* of #if defined(DHCP6_CLI_CFG_REQUEST_OLIST) */
	}

/*
 *  eval_rcv_msg -- 受信メッセージを評価する。
 */

static ER
eval_rcv_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG	*evl, *msg;
	SYSTIM		time;

	/* メッセージを受信していなければ中止する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	if (ct->val_lst == NULL) {
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return E_OK;
		}

	/* 検証メッセージリストから評価メッセージを取り出す。*/
	evl         = ct->val_lst;
	ct->val_lst = ct->val_lst->next;
	evl->next   = NULL;
	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

	/* 受信時間を設定する。*/
	get_tim(&time);
	evl->rcv_time = time;

	/* 評価メッセージを検証する。*/
	if (validate_rcv_msg(ct, evl) != E_OK) {

		/* エラーが発生した評価メッセージを受信メッセージ構造体リストに戻す。*/
		goto rcv_ret;
		}

	/* REL_INFO 状態の時の処理 */
	if (ct->fsm == DHCP6_FSM_REL_INFO) {
		syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
		if (evl->status == DHCP6_STAT_SUCCESS) {
			/*
			 *  STATUS CODE が DHCP6_STAT_SUCCESS なら、
			 *  評価メッセージを推奨メッセージリストに設定して終了する。
			 */
			evl->next   = ct->prf_lst;
			ct->prf_lst = evl;
			}
		else {
			/* 評価メッセージを受信メッセージ構造体リストに戻して終了する。*/
			evl->next   = ct->rcv_lst;
			ct->rcv_lst = evl;
			}
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return E_OK;
		}

	/* 評価メッセージの推奨度を評価する。*/
	eval_prefer(evl);
	eval_ia_prefer(evl);

	/* 推奨メッセージリストが空なら評価メッセージを移して終了する。*/
	if (ct->prf_lst == NULL) {
		ct->max_prefer = evl->prefer & DHCP6_ClI_MSG_PREFER_VALUE_MASK;
		ct->prf_lst = evl;
		return E_OK;
		}

	/* 評価メッセージの推奨度と推奨メッセージリストの最大推奨度を比較する。*/
	if ((evl->prefer & DHCP6_ClI_MSG_PREFER_VALUE_MASK) > ct->max_prefer) {

		/* 
		 *  評価メッセージの推奨度が推奨メッセージリストの最大推奨度より高ければ、
		 *  推奨メッセージリストを受信メッセージ構造体リストに戻し、
		 *  評価メッセージを推奨メッセージリストに移す。
		 */
		ct->max_prefer = evl->prefer & DHCP6_ClI_MSG_PREFER_VALUE_MASK;
		syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
		if (ct->rcv_lst == NULL)
			ct->rcv_lst = ct->prf_lst;
		else {
			for (msg = ct->rcv_lst; msg->next != NULL; msg = msg->next)
				;
			msg->next = ct->prf_lst;
			}
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		ct->prf_lst = evl;
		return E_OK;
		}

	else if ((evl->prefer & DHCP6_ClI_MSG_PREFER_VALUE_MASK) == ct->max_prefer) {
		for (msg = ct->prf_lst; msg != NULL; msg = msg->next) {
			if (is_equal_sid(evl, msg)) {

				/*
				 *  推奨メッセージリストに同じ DHCP6_OPT_SERVERID のメッセージがあれば、
				 *  評価メッセージを受信メッセージ構造体リストに戻す。
				 */
				goto rcv_ret;
				}
			}

		/* 
		 *  評価メッセージを推奨メッセージリストに移す。
		 */
		evl->next   = ct->prf_lst;
		ct->prf_lst = evl;
		return E_OK;
		}
	
	/*else */
		/* 
		 *  評価メッセージの推奨度が推奨メッセージリストの最大推奨度より低ければ、
		 *  評価メッセージを受信メッセージ構造体リストに戻す。
		 */

rcv_ret:
	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	evl->next   = ct->rcv_lst;
	ct->rcv_lst = evl;
	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
	return E_OK;
	}

/*
 *  select_msg -- 推奨メッセージリストの先頭のメッセージを選択する。
 */

static T_DHCP6_CLI_MSG*
select_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG		*msg, *select;

	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	if (ct->prf_lst == NULL) {
		/* メッセージを受信していなければ中止する。*/
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return NULL;
		}

	select = ct->prf_lst;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS

	if ((select->prefer & DHCP6_ClI_MSG_PREFER_REQUIRED) == 0) {
		/* 必須オプションが無ければ選択しない。*/
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return NULL;
		}

#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

	if (((select->prefer & DHCP6_ClI_MSG_PREFER_REQUIRED) == 0) ||
	    ((select->prefer & DHCP6_ClI_MSG_PREFER_ADDR)     == 0)) {
		/* 必須オプションとアドレスが無ければ選択しない。*/
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return NULL;
		}

#else	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

	/* 推奨メッセージリストの先頭のメッセージを選択する。*/
	ct->prf_lst  = ct->prf_lst->next;
	select->next = NULL;

	/* 選択されなかった検証メッセージを受信メッセージ構造体リストに戻す。*/
	if (ct->rcv_lst == NULL)
		ct->rcv_lst = ct->prf_lst;
	else {
		for (msg = ct->rcv_lst; msg->next != NULL; msg = msg->next)
			;
		msg->next = ct->prf_lst;
		}
	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
	return select;
	}

/*
 *  advance_retrans -- 再送信タイムアウトを更新する。
 */

static void
advance_retrans (T_DHCP6_CLI_CONTEXT *ct)
{
	/*
	 *  再送信間隔（RT）を更新する。
	 *  再送信毎に約 2倍するが、
	 *  最大送信間隔（MRT）を超えないようにする。
	 */
	ct->RT += ct->RT + dhcp6_rand(ct->RT);
	if ((ct->MRT != 0) && (ct->RT > ct->MRT))
		ct->RT = ct->MRT + dhcp6_rand(ct->MRT);

	/* 再送信回数を更新する。*/
	ct->txcount ++;
	}

/*
 *  init_retrans -- 再送信回数、トランザクション ID、その他を設定する。
 */

static void
init_retrans (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 送信回数をリセットする。*/
	ct->txcount = 0;

	/* 再送信間隔を設定する。*/
	ct->RT = ct->IRT + dhcp6_rand(ct->IRT);

	/* トランザクション ID を設定する。*/
	ct->xid = netapp_rand() % 0x00ffffff;

	/* 受信メッセージの最大推奨度をリセットする。*/
	ct->max_prefer = 0;
	}

/*
 *  setup_msg_header -- メッセージのヘッダ部分を作成する。
 */

static uint8_t *
setup_msg_header (T_DHCP6_CLI_CONTEXT *ct, uint_t type)
{
	memset((void*)&ct->snd_msg->msg, sizeof(ct->snd_msg->msg), 0);
	ct->snd_msg->len = sizeof(ct->snd_msg->msg);

	/* ヘッダ部を設定する。*/
	ct->snd_msg->msg.hdr.type   = type;
	ct->snd_msg->msg.hdr.xid[0] = (ct->xid >> 16) & 0xff;
	ct->snd_msg->msg.hdr.xid[1] = (ct->xid >>  8) & 0xff;
	ct->snd_msg->msg.hdr.xid[2] =  ct->xid        & 0xff;

	return (uint8_t*)&ct->snd_msg->msg + sizeof(ct->snd_msg->msg.hdr);
	}

/*
 *  common_options -- 標準オプションを追加する。
 */

static uint8_t *
common_options (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t type, T_DHCP6_CLI_MSG *lease)
{
	T_DHCP6_OPT_CLIENTID	*cid;
	T_DHCP6_OPT_SERVERID	*sid;

#if defined(DHCP6_CLI_CFG_REQUIRED_OLIST) || defined(DHCP6_CLI_CFG_REQUEST_OLIST)
	T_DHCP6_OPT_ORO		*oro;
#endif

	uint8_t			*opt;
	uint_t			opt_len;
	int_t			ix;

#if defined(DHCP6_CLI_CFG_RAPID_COMMIT)

	/* Rapid-commit の指定 */
	if (type == DHCP6_SOLICIT)
		;

#endif	/* of #if defined(DHCP6_CLI_CFG_RAPID_COMMIT) */

	/* クライアント DUID を追加する。*/
	cid = (T_DHCP6_OPT_CLIENTID*)msg;
	HOST2MSG16(cid->code, DHCP6_OPT_CLIENTID);
	HOST2MSG16(cid->len,  ct->duid_len);
	memcpy(msg + sizeof(T_DHCP6_OPT_CLIENTID), ct->duid, ct->duid_len);
	msg += ct->duid_len + sizeof(T_DHCP6_OPT_CLIENTID);

	/* サーバ DUID を追加する。*/
	if (lease == NULL) {
		if (!(type == DHCP6_SOLICIT || type == DHCP6_INFO_REQ))
			return NULL;
		}
	else if (!(type == DHCP6_CONFIRM || type == DHCP6_REBIND)) {
		if ((opt = find_msg_option(lease, DHCP6_OPT_SERVERID, 0)) != NULL) {
			sid = (T_DHCP6_OPT_SERVERID*)opt;
			opt_len = MSG2HOST16(sid->len);
			memcpy(msg, opt, opt_len + sizeof(*sid));
			msg += opt_len + sizeof(*sid);
			}
		}

#if defined(DHCP6_CLI_CFG_REQUIRED_OLIST) || defined(DHCP6_CLI_CFG_REQUEST_OLIST)

	/* 必須・要求オプションを追加する。*/
	oro = (T_DHCP6_OPT_ORO*)msg;
	HOST2MSG16(oro->code, DHCP6_OPT_ORO);
	HOST2MSG16(oro->len, 0);
	msg += sizeof(oro->code) + sizeof(oro->len);

#if defined(DHCP6_CLI_CFG_REQUIRED_OLIST)

	/* 必須オプションを追加する。*/
	HOST2MSG16(oro->len, MSG2HOST16(oro->len) + sizeof(required_plist) * 2);
	for (ix = sizeof(required_plist); ix -- > 0; ) {
		host2msg16(msg, required_plist[ix]);
		msg += sizeof(uint16_t);
		}

#endif	/* of #if defined(DHCP6_CLI_CFG_REQUIRED_OLIST) */

#if defined(DHCP6_CLI_CFG_REQUEST_OLIST)

	/* 要求オプションを追加する。*/
	HOST2MSG16(oro->len, MSG2HOST16(oro->len) + sizeof(request_plist) * 2);
	for (ix = sizeof(request_plist); ix -- > 0; ) {
		host2msg16(msg, request_plist[ix]);
		msg += sizeof(uint16_t);
		}

#endif	/* of #if defined(DHCP6_CLI_CFG_REQUEST_OLIST) */

#endif	/* of #if defined(DHCP6_CLI_CFG_REQUIRED_OLIST) || defined(DHCP6_CLI_CFG_REQUEST_OLIST) */

	return msg;
	}

/*
 *  rel_lease -- リースされたオプションを解析して値を解放する。
 */

static void
rel_lease (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPTION			*hdr;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

	T_DHCP6_OPT_IA_NA_PD		*ia;
	T_DHCP6_OPT_IAADDR		*iaa;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL */

	uint8_t		*opt, *last;

	/* 有効なリース・メッセージ構造体が無ければ何もしない。*/
	if (ct->act_msg == NULL)
		return;

	opt  = ct->act_msg->msg.options;
	last = opt + (ct->act_msg->len - sizeof(T_DHCP6_MSG_HDR));
	while (opt < last) {
		hdr = (T_DHCP6_OPTION*)opt;
		switch (MSG2HOST16(hdr->code)) {

		case DHCP6_OPT_NAME_SERVERS:
			dns_in6_set_addr(NULL);

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
			syslog(LOG_NOTICE, "[DHCP6C] release DNS server addr.");
#endif
			break;

		case DHCP6_OPT_DOMAIN_SEARCH:
			dns_in6_set_dname(NULL, 0);
			break;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

		case DHCP6_OPT_IA_NA:
			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			if (MSG2HOST16(ia->len) > sizeof(*iaa)) {
				iaa = (T_DHCP6_OPT_IAADDR*)(opt + sizeof(*ia));
				in6_del_ifaddr(&iaa->addr);
				}
			break;

		case DHCP6_OPT_IA_TA:
			break;

		case DHCP6_OPT_IA_PD:
			break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL */

		default:
			break;
			}
		opt += MSG2HOST16(hdr->len) + sizeof(*hdr);
		}
	}

/*
 *  set_lease -- リースされたオプションを解析して値を設定する。
 */

static void
set_lease (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPTION			*hdr;
	T_DHCP6_OPT_NAME_SERVERS	*ns;
	T_DHCP6_OPT_DOMAIN_SEARCH	*ds;
	const uint8_t			*dname;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

	T_DHCP6_OPT_IA_NA_PD		*ia;
	T_DHCP6_OPT_IAADDR		*iaa;
	SYSTIM				time, delay;
	ER				error;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL */

	uint8_t		*opt, *last;

	opt  = ct->act_msg->msg.options;
	last = opt + (ct->act_msg->len - sizeof(T_DHCP6_MSG_HDR));
	while (opt < last) {
		hdr = (T_DHCP6_OPTION*)opt;
		switch (MSG2HOST16(hdr->code)) {

		case DHCP6_OPT_NAME_SERVERS:
			ns = (T_DHCP6_OPT_NAME_SERVERS*)opt;
			dns_in6_set_addr(&ns->addrs[0]);

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
			syslog(LOG_NOTICE, "[DHCP6C] set DNS server addr: %s.", ipv62str(NULL, &ns->addrs[0]));
#endif
			break;

		case DHCP6_OPT_DOMAIN_SEARCH:
			ds = (T_DHCP6_OPT_DOMAIN_SEARCH*)opt;
			dname = dns_in6_set_dname(ds->name, MSG2HOST16(ds->len));
			break;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

		case DHCP6_OPT_IA_NA:
			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			if (MSG2HOST16(ia->len) > sizeof(*iaa)) {
				get_tim(&time);
				delay = time - ct->act_msg->rcv_time;
				iaa = (T_DHCP6_OPT_IAADDR*)(opt + sizeof(*ia));
				error = in6_upd_ifaddr(&iaa->addr, 64, MSG2HOST32(iaa->valid)  - SYSTIM2SEC(delay),
				                                       MSG2HOST32(iaa->prefer) - SYSTIM2SEC(delay));
				if (error == E_OK)
					syslog(LOG_NOTICE, "[DHCP6C] set IPv6 addr: %s.", ipv62str(NULL, &iaa->addr));
				else
					syslog(LOG_NOTICE, "[DHCP6C] set IPv6 addr, error: %s.", itron_strerror(error));
				}
			break;

		case DHCP6_OPT_IA_TA:
			break;

		case DHCP6_OPT_IA_PD:
			break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL */

		default:
			break;
			}
		opt += MSG2HOST16(hdr->len) + sizeof(*hdr);
		}
	}

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

/*
 *  start_req_info -- INFO-REQUEST を開始する。
 *
 *    注意: DHCPv6 の RFC3315 には無い
 */

static void
start_req_info (T_DHCP6_CLI_CONTEXT *ct)
{
	/* タイムアウトを設定する。【RFC3315 (18.1.5) 参照】*/
	ct->IRT = TMO_INF_TIMEOUT;
	ct->MRT = TMO_INF_MAX_RT;
	ct->MRC = 0;

	/* DHCP6_CLI_CFG_ONE_TRYが指定されていなければ標準タイムアウトまで再送する。*/
#if defined(DHCP6_CLI_CFG_ONE_TRY)
	ct->MRD = 0;
#else
	ct->MRD = TMO_DHCP6C_NORMAL;
#endif

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* REPLY メッセージの受信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RCV_REPLY] = SYSTIM2TIMER(ct->MRD);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/*
	 *  INFO-REQUEST メッセージ送信遅延時間を設定する。
	 *  ただし、TMO_INF_MAX_DELAY が 1[s]なので、dly_tsk()を使用する。
	 */
	dly_tsk(netapp_rand() % TMO_INF_MAX_DELAY);

	/* SELECT 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_SELECT;

	/* INFO-REQUEST メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_IREQ;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  start_informed -- INFORMED 状態に遷移する。
 *
 *    注意: DHCPv6 の RFC3315 には無い
 */

static void
start_informed (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL) {
		ct->fsm = DHCP6_FSM_SLEEP;
		return;
		}

	/* リース情報を設定する。*/
	set_lease(ct);

	/* INFORMED 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_INFORMED;

	//ct->flags |= DHCP6C_FLAG_STAY_STATE;
	}

/*
 *  start_rel_info -- アドレス情報を解放する。
 *
 *    注意: DHCPv6 の RFC3315 には無い
 */

static void
start_rel_info (T_DHCP6_CLI_CONTEXT *ct)
{
	/* リース情報を解放する。*/
	rel_lease(ct);

	/* SLEEP に遷移する。*/
	ct->fsm = DHCP6_FSM_SLEEP;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  select_info_reply -- 最適な REPLY (INFO-REQUEST への応答) メッセージを選択する。
 */

static bool_t
select_info_reply (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG		*msg;

	/* 最適な REPLY メッセージを選択する。*/
	if ((msg = select_msg(ct)) == NULL)
		return false;
	
	ct->act_msg = msg;
	return true;
	}

/*
 *  setup_info_req_msg -- INFO-REQUEST メッセージを作成する。
 */

static ER
setup_info_req_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	uint8_t	*msg;

	/* メッセージのヘッダ部分を作成する。*/
	if ((msg = setup_msg_header(ct, DHCP6_INFO_REQ)) == NULL)
		return E_PAR;

	if ((msg = common_options(ct, msg, DHCP6_INFO_REQ, NULL)) == NULL)
		return E_PAR;

	/* メッセージ構造体長を設定する。*/
	ct->snd_msg->len = msg - (uint8_t*)&ct->snd_msg->msg;

	/* 送信先アドレスを設定する。*/
	ct->dst.ipaddr	= in6_addr_all_DHCP_relay_servers;
	return E_OK;
	}

/*
 *  send_info_req_msg -- INFO-REQUEST メッセージを送信する。
 */

static void
send_info_req_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	ER	error;


	/* REPLY メッセージの選択が終了していれば、BOUND 状態に遷移する。*/
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	if (select_info_reply(ct)) {

		/* REPLY メッセージ受信タイマーを停止する。*/
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_RCV_REPLY] = 0;
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

		/* INFORMED 状態に遷移する。*/
		start_informed(ct);
		return;
		}

	/* 再送信回数を確認する。*/
	if ((ct->MRC != 0) && (ct->txcount > ct->MRC)) {
		ct->error  = E_TMOUT;
		return;
		}

	/* INFO-REQUEST メッセージを作成する。*/
	if ((error = setup_info_req_msg(ct)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, setup request message: %s.", itron_strerror(error));
		ct->error = error;
		return;
		}

	/* INFO-REQUEST メッセージを送信する。*/
	error = UDP_SND_DAT(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, send info-request message: %s.", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 再送信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_IREQ] = SYSTIM2TIMER(ct->RT);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 再送信タイムアウトを更新する。*/
	advance_retrans(ct);
	}

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

/*
 *  select_reply -- 最適な REPLY メッセージを選択する。
 */

static bool_t
select_reply (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG		*msg;

	/* 最適な REPLY メッセージを選択する。*/
	if ((msg = select_msg(ct)) == NULL)
		return false;

	if (ct->adv_msg != NULL) {
		eval_prefer(ct->adv_msg);
		eval_ia_prefer(ct->adv_msg);
		}

	/*
	 *  最適な REPLY メッセージの推奨度が ADVERTISE メッセージ推奨度の
	 *  1/2 以下の時は受信メッセージ構造体リストに戻す。
	 */
	if ((ct->adv_msg != NULL) && (ct->max_prefer < (ct->adv_msg->prefer & DHCP6_ClI_MSG_PREFER_VALUE_MASK) / 2)) {
		syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
		msg->next = ct->rcv_lst;
		ct->rcv_lst = msg;
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		}
	else {

		/*
		 *  ct->act_msg にある有効な元 REPLY メッセージを
		 *  受信メッセージ構造体リストに戻して、
		 *  最適な REPLY メッセージを act に移す。
		 */
		if (ct->act_msg != NULL) {
			syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
			ct->act_msg->next = ct->rcv_lst;
			ct->rcv_lst = ct->act_msg;
			syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
			}
		ct->act_msg = msg;
		}

	return ct->act_msg != NULL;
	}

/*
 *  select_adv -- 最適な ADVERTISE メッセージを選択する。
 */

static bool_t
select_adv (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 最適な ADVERTISE メッセージを adv に移す。*/
	if ((ct->adv_msg = select_msg(ct)) != NULL) {

		/*
		 *  最適な ADVERTISE メッセージのサーバの IPv6 アドレスを
		 *  次から送信するアドレスに設定する。
		 */
		ct->dst = ct->adv_msg->srv;
		return true;
		}
	else
		return false;
	}

/*
 *  eval_expire -- 推奨有効時間と有効時間を決定する。
 */

static void
eval_expire (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPTION		*opt;
	T_DHCP6_OPT_IA_NA_PD	*ia;
	T_DHCP6_OPT_IAADDR	*iaa;
	SYSTIM		time;
	uint32_t	depref, expire,
			high, low, renew, rebind,
			tval, iaa_high, iaa_low;
	uint8_t		*msg, *last;
	uint_t		aoff, asize, addrs;

	/* タイマーを停止する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RENEW]  = 0;
	ct->timers[DHCP6C_TIMER_REBIND] = 0;
	ct->timers[DHCP6C_TIMER_DEPREF] = 0;
	ct->timers[DHCP6C_TIMER_EXPIRE] = 0;
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
	
	addrs = high = expire = 0;
	low = renew = rebind = depref = DHCP6_MAX_TIME;

	msg = ct->act_msg->msg.options;
	last = msg + (ct->act_msg->len - sizeof(T_DHCP6_MSG_HDR));
	get_tim(&time);
	while (msg < last) {
		opt = (T_DHCP6_OPTION*)msg;

		/* IA_NA/IA_PD オプションであれば、有効時間を更新する。*/
		if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_NA || MSG2HOST16(opt->code) == DHCP6_OPT_IA_PD) {

			//dump_msg_option("evl_expire1", msg);
			iaa_high = 0;
			iaa_low  = DHCP6_MAX_TIME;

			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			aoff  = sizeof(*ia);
			asize = MSG2HOST16(ia->len) + sizeof(*opt);
			while (aoff < asize) {

				/* 最短の推奨有効時間を更新する。*/
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				//dump_msg_option("evl_depref1", (uint8_t*)iaa);
				if (!(MSG2HOST16(iaa->len) & DHCP6C_IAA_FLAG_DEPREFERD)) {
					if (MSG2HOST32(iaa->prefer) == DHCP6_MAX_TIME)
						tval = DHCP6_MAX_TIME;
					else
						tval = MSG2HOST32(iaa->prefer) + SYSTIM2SEC(ct->act_msg->rcv_time);
					if (tval < depref)
						depref = tval;
					}

				/* IAADDR の有効時間を更新する。*/
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				if (!(MSG2HOST16(iaa->len) & DHCP6C_IAA_FLAG_EXPIRED)) {
					if (MSG2HOST32(iaa->valid) == DHCP6_MAX_TIME)
						tval = DHCP6_MAX_TIME;
					else
						tval = MSG2HOST32(iaa->valid) + SYSTIM2SEC(ct->act_msg->rcv_time);
					tval -= SYSTIM2SEC(ct->act_msg->rcv_time);
					if (tval > iaa_high)
						iaa_high = tval;
					if (tval < iaa_low)
						iaa_low  = tval;
					addrs ++;
					}

				aoff += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(*iaa);
				}

			/* iaa_low と iaa_high/2 の最長の方を有効時間として選択する。*/
			if (iaa_low <= (iaa_high / 2))
				expire = iaa_high;
			else
				expire = iaa_low;

			/* expire が 最大値（無制限）か 1 以下なら既定値に設定する。*/
			if ((expire == DHCP6_MAX_TIME) || (expire <= 1))
				expire  = SYSTIM2SEC(TMO_DHCP6C_REQ_LEASE) / 2;
			else
				expire /= 2;

			/* renew を決定する。*/
			if (MSG2HOST32(ia->renew) == 0)
				tval = expire + SYSTIM2SEC(ct->act_msg->rcv_time);
			else if (MSG2HOST32(ia->renew) == DHCP6_MAX_TIME)
				tval = DHCP6_MAX_TIME;
			else
				tval = MSG2HOST32(ia->renew) + SYSTIM2SEC(ct->act_msg->rcv_time);
			if (tval < renew)
				renew = tval;

			/* rebind を決定する。*/
			if (MSG2HOST32(ia->rebind) == 0)
				tval = (expire + expire / 2) + SYSTIM2SEC(ct->act_msg->rcv_time);
			else if (MSG2HOST32(ia->rebind) == DHCP6_MAX_TIME)
				tval = DHCP6_MAX_TIME;
			else
				tval = MSG2HOST32(ia->rebind) + SYSTIM2SEC(ct->act_msg->rcv_time);
			if (tval < rebind)
				rebind = tval;

			/* 最短有効時間と最長有効時間を更新する。*/
			iaa_low  += SYSTIM2SEC(ct->act_msg->rcv_time);
			iaa_high += SYSTIM2SEC(ct->act_msg->rcv_time);
			if (iaa_low < low)
				low = iaa_low;
			if (iaa_high > high)
				high = iaa_high;
			}

		msg += MSG2HOST16(opt->len) + sizeof(*opt);
		}

	/* アドレスが送られていなければ、SLEEP 状態に遷移する。*/
	if (addrs == 0) {
		ct->fsm    = DHCP6_FSM_SLEEP;
		}

	/* 状態に依存する処理 */
	switch (ct->fsm) {

	case DHCP6_FSM_BOUND:

		/* RENEW タイムアウトの設定 */
		if ((rebind > SYSTIM2SEC(time)) && (renew < rebind)) {
			ct->next_MRD = SEC2SYSTIM(rebind) - time;
			syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
			ct->timers[DHCP6C_TIMER_RENEW] = SEC2TIMER(renew) - SYSTIM2TIMER(time);
			syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
			break;
			}
		/* break; */	/* 下に抜ける。*/

	case DHCP6_FSM_RENEW:

		/* REBIND タイムアウトの設定 */
		ct->MRD = SEC2SYSTIM(rebind);
		if (rebind != DHCP6_MAX_TIME) {
			ct->next_MRD = SEC2SYSTIM(high) - time;
			syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
			ct->timers[DHCP6C_TIMER_REBIND] = SEC2TIMER(rebind) - SYSTIM2TIMER(time);
			syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
			}
		break;

	case DHCP6_FSM_REBIND:

		ct->next_MRD = (SEC2TIMER(high) - time) - time;
		break;

	default:
		break;
		}

	/* DEPREF タイムアウトの設定 */
	if (depref != DHCP6_MAX_TIME) {
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_DEPREF] = SEC2TIMER(depref) - SYSTIM2TIMER(time);
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
		}

	/* EXPIRE タイムアウトの設定 */
	if (low != DHCP6_MAX_TIME) {
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_EXPIRE] = SEC2TIMER(low) - SYSTIM2TIMER(time);
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));
		}
	}

/*
 *  start_init -- INIT 状態に遷移する。
 */

static void
start_init (T_DHCP6_CLI_CONTEXT *ct)
{
	/* タイムアウトを設定する。【RFC3315 (17.1.2) 参照】*/
	ct->IRT = TMO_SOL_TIMEOUT;
	ct->MRT = TMO_SOL_MAX_RT;
	ct->MRC = 0;

	/* DHCP6_CLI_CFG_ONE_TRYが指定されていなければ標準タイムアウトまで再送する。*/
#if defined(DHCP6_CLI_CFG_ONE_TRY)
	ct->MRD = 0;
#else
	ct->MRD = TMO_DHCP6C_NORMAL;
#endif

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* RFC3315 (17.1.2) の規定に従ってタイムアウトを調整する。*/
	if (ct->RT <= ct->IRT)
		ct->RT = ct->IRT + (ct->IRT - ct->RT);
	if (ct->RT <= ct->IRT)
		ct->RT = ct->IRT + SEC2SYSTIM(1);

	/* ADVERTISE メッセージの受信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RCV_ADV] = SYSTIM2TIMER(ct->MRD);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/*
	 *  SOLICIT メッセージ送信遅延時間を設定する。
	 *  ただし、TMO_SOL_MAX_DELAY が 1[s]なので、dly_tsk()を使用する。
	 */
	dly_tsk(netapp_rand() % TMO_SOL_MAX_DELAY);

	/* INIT 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_INIT;

	/* SOLICIT メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_SOL;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  start_select -- REQUEST メッセージを送信して SELECT 状態に遷移する。
 */

static void
start_select (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 有効な ADVERTISE メッセージを受信していなければ SLEEP 状態に遷移する。*/
	if (ct->adv_msg == NULL) {
		ct->fsm = DHCP6_FSM_SLEEP;
		return;
		}

	/* タイムアウトを設定する。【RFC3315 (18.1.1) 参照】*/
	ct->IRT = TMO_REQ_TIMEOUT;
	ct->MRT = TMO_REQ_MAX_RT;
	ct->MRC = TMO_REQ_MAX_RC;
	ct->MRD = 0;

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* REPLY メッセージの受信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RCV_REPLY] = SYSTIM2TIMER(ct->MRD);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* SELECT 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_SELECT;

	/* REQUEST メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_REQ;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  start_bound -- BOUND 状態に遷移する。
 */

static void
start_bound (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL)
		ct->fsm = DHCP6_FSM_SLEEP;

	/* BOUND 状態に遷移する。*/
	ct->fsm    = DHCP6_FSM_BOUND;

	/* リース情報を設定する。*/
	set_lease(ct);

	/* 有効時間を決定する。*/
	eval_expire(ct);

	}
/*
 *  start_renew -- RENEW 状態に遷移する。
 */

static void
start_renew (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL) {
		ct->fsm = DHCP6_FSM_SLEEP;
		return;
		}

	/* タイムアウトを設定する。【RFC3315 (18.1.3) 参照】*/
	ct->IRT = TMO_REN_TIMEOUT;
	ct->MRT = TMO_REN_MAX_RT;
	ct->MRC = 0;
	ct->MRD = ct->next_MRD;

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* REFRESH メッセージタイプを設定する。*/
	ct->refresh_type = DHCP6_RENEW;

	/* REPLY メッセージの受信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RCV_REPLY] = SYSTIM2TIMER(ct->MRD);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* RENEW 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_RENEW;

	/* REFRESH メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_REF;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  start_rebind -- REBIND 状態に遷移する。
 */

static void
start_rebind (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL) {
		ct->fsm = DHCP6_FSM_SLEEP;
		return;
		}

	/* タイムアウトを設定する。【RFC3315 (18.1.4) 参照】*/
	ct->IRT = TMO_REB_TIMEOUT;
	ct->MRT = TMO_REB_MAX_RT;
	ct->MRC = 0;
	ct->MRD = ct->next_MRD;

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* REFRESH メッセージタイプを設定する。*/
	ct->refresh_type = DHCP6_REBIND;

	/* REPLY メッセージの受信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_RCV_REPLY] = SYSTIM2TIMER(ct->MRD);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 送信先アドレスを設定する。*/
	ct->dst.ipaddr	= in6_addr_all_DHCP_relay_servers;

	/* REBIND 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_REBIND;

	/* REFRESH メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_REF;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

/*
 *  start_rel_info -- アドレス情報を解放する。
 *
 *    注意: DHCPv6 の RFC3315 には無い
 */

static void
start_rel_info (T_DHCP6_CLI_CONTEXT *ct)
{
	/* 全ての TIMER をキャンセルする。*/
	cancel_all_timers(ct);

	/* リース情報を解放する。*/
	rel_lease(ct);

	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL) {
		ct->fsm = DHCP6_FSM_SLEEP;
		return;
		}

	/* タイムアウトを設定する。【RFC3315 (18.1.1) 参照】*/
	ct->IRT = TMO_REL_TIMEOUT;
	ct->MRT = 0;
	ct->MRC = TMO_REL_MAX_RC;
	ct->MRD = 0;

	/* 再送回数とトランザクション ID を設定する。*/
	init_retrans(ct);

	/* REL_INFO 状態に遷移する。*/
	ct->fsm = DHCP6_FSM_REL_INFO;

	/* RELEASE メッセージを送信する。*/
	ct->flags |= DHCP6C_FLAG_TMOUT_SND_REL;
	syscall(sig_sem(SEM_DHCP6_CLI_READY));
	}

#if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_TA) || defined(DHCP6_CLI_CFG_IA_PD)

/*
 *  solicit_ia_option -- SOLICIT メッセージの IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。
 */

static void
solicit_ia_option (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t code)
{
	T_DHCP6_OPT_IA_TA	*ia;
	uint_t			off, len;

	/* オプションコードを設定する。*/
	ia = (T_DHCP6_OPT_IA_TA*)msg;
	HOST2MSG16(ia->code, code);

	/* IAID を設定する。*/
	if (sizeof(ct->sc->ifaddr.lladdr) > sizeof(uint32_t)) {
		off = sizeof(ct->sc->ifaddr.lladdr) - 4;
		len = 4;
		}
	else {
		off = 0;
		len = sizeof(ct->sc->ifaddr.lladdr);
		}
	memcpy((uint8_t*)&ia->iaid, ((uint8_t*)ct->sc->ifaddr.lladdr) + off, len);
	}

/*
 *  solicit_iaaddr_option -- SOLICIT メッセージの IAADDR オプションを追加する。
 */

static uint_t
solicit_iaaddr_option (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t code)
{
	T_DHCP6_OPT_IAADDR	*iaa;
	uint32_t		t1, t2;
	uint8_t			*opt;
	uint_t			len;

	/*
	 *  有効なリース・メッセージが残っていて、
	 *  指定されたオプション（IA_NA/IA_TA/IA_PD）がある場合は
	 *  同じアドレスを要求する。
	 */
	if ((ct->act_msg == NULL) || (opt = find_msg_option(ct->act_msg, code, 0)) == NULL)
		return 0;

	/* オプションコードを設定する。*/
	iaa = (T_DHCP6_OPT_IAADDR*)msg;
	HOST2MSG16(iaa->code, DHCP6_OPT_IAADDR);

	/* IAADDR オプションを除いたオプション長を設定する。*/
	HOST2MSG16(iaa->len, sizeof(*iaa) - sizeof(T_DHCP6_OPTION));

	/* アドレスを設定する。*/
	len = code == DHCP6_OPT_IA_TA ? sizeof(T_DHCP6_OPT_IA_TA)
	                              : sizeof(T_DHCP6_OPT_IA_NA_PD);
	memcpy(&iaa->addr, opt + sizeof(T_DHCP6_OPTION) + len, sizeof(iaa->addr));

	/* T1、T2 を設定する。*/
	t1 = TMO_DHCP6C_REQ_LEASE / 2;
	t2 = t1 + (t1 / 2);
	HOST2MSG32(iaa->prefer, t1);
	HOST2MSG32(iaa->valid,  t2);

	return sizeof(*iaa);
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_TA) || defined(DHCP6_CLI_CFG_IA_PD) */

/*
 *  solicit_ia_na_pd -- SOLICIT メッセージの IA_NA/IA_PD オプションを追加する。
 */

#if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_PD)

static uint8_t *
solicit_ia_na_pd (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t code)
{
	T_DHCP6_OPT_IA_NA_PD	*ia;
	uint32_t		t1, t2;
	uint_t			len;

	/* IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。*/
	solicit_ia_option(ct, msg, code);
	ia = (T_DHCP6_OPT_IA_NA_PD*)msg;

	/* T1、T2 を設定する。*/
	t1 = TMO_DHCP6C_REQ_LEASE / 2;
	t2 = t1 + (t1 / 2);
	HOST2MSG32(ia->renew,  SYSTIM2SEC(t1));
	HOST2MSG32(ia->rebind, SYSTIM2SEC(t2));

	/*
	 *  有効なリース・メッセージが残っていて、
	 *  指定されたオプション（IA_NA/IA_PD）がある場合は
	 *  同じアドレスを要求する。
	 */
	msg += sizeof(*ia);
	len  = solicit_iaaddr_option(ct, msg, code);

	/* IA_NAIA_TA/IA_PD オプションを除いたオプション長を設定する。*/
	HOST2MSG16(ia->len, (sizeof(*ia) + len) - sizeof(T_DHCP6_OPTION));

	return msg + len;
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_PD) */

/*
 *  solicit_ia_ta -- SOLICIT メッセージの IA_TA オプションを追加する。
 */

#if defined(DHCP6_CLI_CFG_IA_TA)

static uint8_t *
solicit_ia_ta (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg)
{
	T_DHCP6_OPT_IA_TA	*ia;

	/* IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。*/
	solicit_ia_option(ct, msg, DHCP6_OPT_IA_TA);

	/* IA_NAIA_TA/IA_PD オプションを除いたオプション長を設定する。*/
	ia = (T_DHCP6_OPT_IA_TA*)msg;
	HOST2MSG16(ia->len, sizeof(*ia) - sizeof(T_DHCP6_OPTION));

	/*
	 *  有効なリース・メッセージが残っていて、
	 *  指定されたオプション（IA_TA）がある場合は
	 *  同じアドレスを要求する。
	 */
	msg += sizeof(*ia);
	return msg + solicit_iaaddr_option(ct, msg, code);
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_TA) */

/*
 *  setup_solicit_msg -- SOLICIT メッセージを作成する。
 */

static ER
setup_solicit_msg (T_DHCP6_CLI_CONTEXT *ct, T_DHCP6_CLI_MSG *lease)
{
	uint8_t	*head, *msg;

	/* メッセージのヘッダ部分を作成する。*/
	if ((head = setup_msg_header(ct, DHCP6_SOLICIT)) == NULL)
		return E_PAR;

	if ((msg = common_options(ct, head, DHCP6_SOLICIT, lease)) == NULL)
		return E_PAR;

#if defined(DHCP6_CLI_CFG_IA_NA)

	/* IA_NA を追加する。*/
	msg = solicit_ia_na_pd(ct, msg, DHCP6_OPT_IA_NA);
#endif

#if defined(DHCP6_CLI_CFG_IA_TA)

	/* IA_TA を追加する。*/
	msg = solicit_ia_ta(ct, msg);
#endif

#if defined(DHCP6_CLI_CFG_IA_PD)

	/* IA_PD を追加する。*/
	msg = solicit_ia_na_pd(ct, msg, DHCP6_OPT_IA_PD);
#endif

	/* メッセージ構造体長を設定する。*/
	ct->snd_msg->len = msg - (uint8_t*)&ct->snd_msg->msg;

	/* 送信先アドレスを設定する。*/
	ct->dst.ipaddr	= in6_addr_all_DHCP_relay_servers;
	return E_OK;
	}

/*
 *  send_solicit_msg -- SOLICIT メッセージを送信する。
 */

static void
send_solicit_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	ER	error;

	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	ct->error = E_OK;

	/* ADVERTISE メッセージの選択が終了していれば、SELECT 状態に遷移する。*/
	if (select_adv(ct)) {

		/* ADVERTISE メッセージ受信タイマーを停止する。*/
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_RCV_ADV] = 0;
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

 		/* REQUEST メッセージを送信して SELECT 状態に遷移する。*/
 		start_select(ct);
		return;
		}

	/* 再送信回数を確認する。*/
	if ((ct->MRC != 0) && (ct->txcount > ct->MRC)) {
		ct->error  = E_TMOUT;
		return;
		}

	/* SOLICIT メッセージを作成する。*/
	if ((error = setup_solicit_msg(ct, NULL)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, setup solicit message: %s.", itron_strerror(error));
		ct->error = error;
		return;
		}

	/* SOLICIT メッセージを送信する。*/
	error = UDP_SND_DAT(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, send solicit message: %s.", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 再送信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_SOL] = SYSTIM2TIMER(ct->RT);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 再送信タイムアウトを更新する。*/
	advance_retrans(ct);
	}

/*
 *  request_ia_option -- REQUEST メッセージの IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。
 */

#if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_TA) || defined(DHCP6_CLI_CFG_IA_PD)

static uint8_t *
request_ia_option (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t code, uint8_t *lease)
{
	T_DHCP6_OPT_IA_NA_PD	*dst, *src;

	dst = (T_DHCP6_OPT_IA_NA_PD*)msg;
	src = (T_DHCP6_OPT_IA_NA_PD*)lease;

	/* オプションコードを設定する。*/
	HOST2MSG16(dst->code, code);

	/* IA_NAIA_TA/IA_PD オプションを除いたオプション長を設定する。*/
	HOST2MSG16(dst->len, sizeof(*dst) - (sizeof(dst->code) + sizeof(dst->len)));

	/* ADVERTISE メッセージから IA ID をコピーする。*/
	memcpy(&dst->iaid, &src->iaid, sizeof(src->iaid));

	return msg + sizeof(uint16_t) * 2 + sizeof(uint32_t);
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_TA) || defined(DHCP6_CLI_CFG_IA_PD) */

/*
 *  request_ia_na_pd -- REQUEST メッセージの IA_NA/IA_PD オプションを追加する。
 */

#if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_PD)

static uint8_t *
request_ia_na_pd (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t msg_type, uint_t opt_code, T_DHCP6_CLI_MSG *lease)
{
	T_DHCP6_OPT_IA_NA_PD	*dia,  *sia;
	T_DHCP6_OPT_IAADDR	*diaa, *siaa;
	uint8_t			*opt;
	uint32_t		t1, t2;
	uint_t			ix, nsiaa;

	/* ADVERTISE メッセージに指定されたオプションがあることを確認する。*/
	if ((opt = find_msg_option(lease, opt_code, 0)) == NULL)
		return msg;

	/* IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。*/
	dia = (T_DHCP6_OPT_IA_NA_PD*)msg;
	sia = (T_DHCP6_OPT_IA_NA_PD*)opt;
	msg = request_ia_option(ct, msg, opt_code, opt);

	/* T1、T2 を決定する。*/
	switch (msg_type) {
	case DHCP6_REQUEST:
	case DHCP6_RENEW:
	case DHCP6_REBIND:
		t1 = SYSTIM2TIMER(TMO_DHCP6C_REQ_LEASE);
		t2 = t1 + (t1 / 2);
		break;

	case DHCP6_CONFIRM:
	case DHCP6_RELEASE:
	case DHCP6_DECLINE:
		t1 = t2 = 0;
		break;

	default:
		syslog(LOG_NOTICE, "[DHCP6C] unkown msg_type in setup IA_NA/IA_PD for request message: %d.", msg_type);
		return NULL;
		break;
		}

	/* T1、T2 を設定する。*/
	HOST2MSG32(dia->renew,  t1);
	HOST2MSG32(dia->rebind, t2);
	msg += sizeof(dia->renew) + sizeof(dia->rebind);

	/* IAADDR オプションを設定する。*/
	nsiaa = ((MSG2HOST16(sia->len) & ~DHCP6C_IAA_FLAG_MASK) - (sizeof(*sia) - (sizeof(sia->code) + sizeof(sia->len)))) / sizeof(*siaa);

	for (ix = 0; ix <nsiaa; ix++) {
		diaa = (T_DHCP6_OPT_IAADDR*)(msg + sizeof(*diaa) * ix);
		siaa = (T_DHCP6_OPT_IAADDR*)((opt + sizeof(*sia)) + sizeof(*siaa) * ix);

		/* アドレスの有効期限が切れていればスキップする。*/
		if (MSG2HOST16(siaa->len) & DHCP6C_IAA_FLAG_EXPIRED)
			continue;

		/* コードと長さを設定する。*/
		memcpy(&diaa->code, &siaa->code, sizeof(siaa->code));
		HOST2MSG16(diaa->len, MSG2HOST16(siaa->len) & ~DHCP6C_IAA_FLAG_MASK);
		msg += sizeof(dia->code) + sizeof(dia->len);

		/* IPv6 アドレスをコピーする。*/
		memcpy(&diaa->addr, &siaa->addr, sizeof(siaa->addr));
		msg += sizeof(diaa->addr);

		/* T1、T2 を決定する。*/
		switch (msg_type) {
		case DHCP6_REQUEST:
		case DHCP6_RENEW:
		case DHCP6_REBIND:
			t1 = SYSTIM2TIMER(TMO_DHCP6C_REQ_LEASE);
			t2 = t1 + 300;
			break;

		case DHCP6_CONFIRM:
		case DHCP6_RELEASE:
		case DHCP6_DECLINE:
			t1 = t2 = 0;
			break;

		default:
			syslog(LOG_NOTICE, "[DHCP6C] unknown msg_type in setup IA_NA/IA_PD for request: %d.", msg_type);
			return NULL;
			break;
			}

		/* T1、T2 を設定する。*/
		HOST2MSG32(diaa->prefer, t1);
		HOST2MSG32(diaa->valid,  t2);
		msg += sizeof(diaa->prefer) + sizeof(diaa->valid);

		/* IAADDR に付加オプションを追加する【先送り】。*/
		}

	HOST2MSG16(((T_DHCP6_OPTION*)dia)->len, 
	MSG2HOST16(((T_DHCP6_OPTION*)dia)->len) + (msg - (uint8_t*)dia) - sizeof(*dia));
	return msg;
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_NA) || defined(DHCP6_CLI_CFG_IA_PD) */

/*
 *  request_ia_ta -- REQUEST メッセージの IA_TA オプションを追加する。
 */

#if defined(DHCP6_CLI_CFG_IA_TA)

static uint8_t *
request_ia_ta (T_DHCP6_CLI_CONTEXT *ct, uint8_t *msg, uint_t msg_type, T_DHCP6_CLI_MSG *lease)
{
	T_DHCP6_OPT_IA_TA	*dia,  *sia;
	T_DHCP6_OPT_IAADDR	*diaa, *siaa;
	uint8_t			*opt;
	uint32_t		t1, t2;
	uint_t			ix, nsiaa;

	/* ADVERTISE メッセージに IA_TA オプションがあることを確認する。*/
	if ((opt = find_msg_option(lease, DHCP6_OPT_IA_TA, 0)) == NULL)
		return msg;

	/* IA_NA/IA_TA/IA_PD オプションの共通部分を追加する。*/
	dia = (T_DHCP6_OPT_IA_TA*)msg;
	sia = (T_DHCP6_OPT_IA_TA*)opt;
	msg = request_ia_option(ct, msg, DHCP6_OPT_IA_TA, opt);

	/* IAADDR オプションを設定する。*/
	nsiaa = ((MSG2HOST16(sia->len) & ~DHCP6C_IAA_FLAG_MASK) - (sizeof(*sia) - (sizeof(sia->code) + sizeof(sia->len)))) / sizeof(*siaa);

	for (ix = 0; ix <nsiaa; ix++) {
		diaa = (T_DHCP6_OPT_IAADDR*)(msg + sizeof(*diaa) * ix);
		siaa = (T_DHCP6_OPT_IAADDR*)((opt + sizeof(*sia)) + sizeof(*siaa) * ix);

		/* アドレスの有効期限が切れていればスキップする。*/
		if (MSG2HOST16(siaa->len) & DHCP6C_IAA_FLAG_EXPIRED)
			continue;

		/* コードと長さを設定する。*/
		memcpy(diaa->code, siaa->code, sizeof(siaa->code));
		diaa->len  = HOST2MSG16(MSG2HOST16(siaa->len) & ~DHCP6C_IAA_FLAG_MASK);
		msg += sizeof(dia->code) + sizeof(dia->len);

		/* IPv6 アドレスをコピーする。*/
		memcpy(diaa->addr, siaa->addr, sizeof(siaa->addr));
		msg += sizeof(diaa->addr);

		/* T1、T2 を決定する。*/
		switch (msg_type) {
		case DHCP6_REQUEST:
		case DHCP6_RENEW:
		case DHCP6_REBIND:
			t1 = SYSTIM2TIMER(TMO_DHCP6C_REQ_LEASE);
			t2 = t1 + 300;
			break;

		case DHCP6_CONFIRM:
		case DHCP6_RELEASE:
		case DHCP6_DECLINE:
			t1 = t2 = 0;
			break;

		default:
			syslog(LOG_NOTICE, "[DHCP6C] unknown msg_type in setup IA_TA for request message: %d.", msg_type);
			return NULL;
			break;
			}

		/* T1、T2 を設定する。*/
		HOST2MSG32(diaa->prefer, t1);
		HOST2MSG32(diaa->valid,  t2);
		msg += sizeof(diaa->prefer) + sizeof(diaa->valid);

		/* IAADDR に付加オプションを追加する【先送り】。*/
		}

	return msg;
	}

#endif	/* of #if defined(DHCP6_CLI_CFG_IA_TA) */

/*
 *  setup_request_msg -- REQUEST メッセージを作成する。
 */

static ER
setup_request_msg (T_DHCP6_CLI_CONTEXT *ct, T_DHCP6_CLI_MSG *lease, uint_t type)
{
	uint8_t	*msg;

	/* メッセージのヘッダ部分を作成する。*/
	if ((msg = setup_msg_header(ct, type)) == NULL)
		return E_PAR;

	if ((msg = common_options(ct, msg, type, lease)) == NULL)
		return E_PAR;

#if defined(DHCP6_CLI_CFG_IA_NA)

	/* IA_NA を追加する。*/
	msg = request_ia_na_pd(ct, msg, type, DHCP6_OPT_IA_NA, lease);
#endif

#if defined(DHCP6_CLI_CFG_IA_TA)

	/* IA_TA を追加する。*/
	msg = request_ia_ta(ct, msg, type, lease);
#endif

#if defined(DHCP6_CLI_CFG_IA_PD)

	/* IA_PD を追加する。*/
	msg = request_ia_na_pd(ct, msg, type, DHCP6_OPT_IA_PD, lease);
#endif

	/* メッセージ構造体長を設定する。*/
	ct->snd_msg->len = msg - (uint8_t*)&ct->snd_msg->msg;
	return E_OK;
	}

/*
 *  send_request_msg -- REQUEST メッセージを送信する。
 */

static void
send_request_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	ER	error;

	ct->error = E_OK;
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;

	/* ADVERTISE メッセージを受信していなければエラー */
	if (ct->adv_msg == NULL) {
		ct->error  = E_OBJ;
		return;
		}

	/* REPLY メッセージの選択が終了していれば、BOUND 状態に遷移する。*/
	if (select_reply(ct)) {

		/* REPLY メッセージ受信タイマーを停止する。*/
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_RCV_REPLY] = 0;
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	 	/* ADVERTISE メッセージを受信メッセージ構造体リストに戻す。*/
		syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
		ct->adv_msg->next = ct->rcv_lst;
		ct->rcv_lst = ct->adv_msg;
		ct->adv_msg = NULL;
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

		/* BOUND 状態に遷移する。*/
		start_bound(ct);
		return;
		}

	/* 再送信回数を確認する。*/
	if ((ct->MRC != 0) && (ct->txcount > ct->MRC)) {
		ct->error  = E_TMOUT;
		return;
		}

	/* REQUEST メッセージを作成する。*/
	if ((error = setup_request_msg(ct, ct->adv_msg, DHCP6_REQUEST)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, setup request message: %s.", itron_strerror(error));
		ct->error = error;
		return;
		}

	/* REQUEST メッセージを送信する。*/
	error = UDP_SND_DAT(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, send request message: %s.", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 再送信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_REQ] = SYSTIM2TIMER(ct->RT);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 再送信タイムアウトを更新する。*/
	advance_retrans(ct);
	}

/*
 *  send_refresh_msg -- REFRESH メッセージを送信する。
 */

static void
send_refresh_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPT_UNICAST	*uc;
	ER			error;
	uint8_t			*opt;

	/* 有効なリース情報がなければエラー */
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	if (ct->act_msg == NULL)
		ct->error = E_OBJ;

	/* REFRESH タイプが RENEW または REBIND でなければエラー */
	if (!((ct->refresh_type == DHCP6_RENEW) || (ct->refresh_type == DHCP6_REBIND)))
		ct->error = E_PAR;

	/* REPLY メッセージの選択が終了していれば、BOUND 状態に遷移する。*/
	if (select_reply(ct)) {

		/* REPLY メッセージ受信タイマーを停止する。*/
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_RCV_REPLY] = 0;
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

		/* リース情報を解放する。*/
		rel_lease(ct);

		/* BOUND 状態に遷移する。*/
		start_bound(ct);
		return;
		}

	/* 再送信回数を確認する。*/
	if ((ct->MRC != 0) && (ct->txcount > ct->MRC)) {
		eval_expire(ct);
		return;
		}

	/*
	 *  サーバから UNICAST オプションを受信したときは
	 *  指定したアドレスを送信先アドレスに設定する。
	 */
	if ((opt = find_msg_option(ct->act_msg, DHCP6_OPT_UNICAST, sizeof(T_DHCP6_OPT_UNICAST))) != NULL) {
		uc = (T_DHCP6_OPT_UNICAST*)opt;
		ct->dst.ipaddr = uc->addr;
		}

	/* REFRESH (REQUEST) メッセージを作成する。*/
	if ((error = setup_request_msg(ct, ct->act_msg, ct->refresh_type)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, setup refresh message: %s.", itron_strerror(error));
		ct->error = error;
		return;
		}

	/* REFRESH メッセージを送信する。*/
	error = UDP_SND_DAT(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, send refresh message: %s.", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 再送信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_REF] = SYSTIM2TIMER(ct->RT);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 再送信タイムアウトを更新する。*/
	advance_retrans(ct);
	}

/*
 *  setup_release_msg -- RELEASE メッセージを作成する。
 */

static ER
setup_release_msg (T_DHCP6_CLI_CONTEXT *ct, T_DHCP6_CLI_MSG *lease, uint_t type)
{
	uint8_t	*msg;

	/* メッセージのヘッダ部分を作成する。*/
	if ((msg = setup_msg_header(ct, type)) == NULL)
		return E_PAR;

	if ((msg = common_options(ct, msg, type, lease)) == NULL)
		return E_PAR;

#if defined(DHCP6_CLI_CFG_IA_NA)

	/* IA_NA を追加する。*/
	msg = request_ia_na_pd(ct, msg, type, DHCP6_OPT_IA_NA, lease);
#endif

#if defined(DHCP6_CLI_CFG_IA_PD)

	/* IA_PD を追加する。*/
	msg = request_ia_na_pd(ct, msg, type, DHCP6_OPT_IA_PD, lease);
#endif

	/* メッセージ構造体長を設定する。*/
	ct->snd_msg->len = msg - (uint8_t*)&ct->snd_msg->msg;
	return E_OK;
	}

/*
 *  send_release_msg -- RELEASE メッセージを送信する。
 */

static void
send_release_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	ER	error;

	/* 有効な REPLY メッセージを受信していなければエラー */
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	if (ct->act_msg == NULL)
		ct->error = E_OBJ;

	/* 有効な REPLY メッセージを受信していれば、SLEEP 状態に遷移する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	if (ct->prf_lst != NULL) {

		/* 再送信タイマーを停止する。*/
		syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
		ct->timers[DHCP6C_TIMER_RCV_REPLY] = 0;
		syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

		/* 有効なリース・メッセージ構造体を解放する。*/
		if (ct->act_msg != NULL) {
			syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)ct->act_msg));
			ct->act_msg = NULL;
			}

		/* SLEEP 状態に遷移する。*/
		ct->fsm    = DHCP6_FSM_SLEEP;
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
		return;
		}
	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

	/* 再送信回数を確認する。*/
	if ((ct->MRC != 0) && (ct->txcount > ct->MRC)) {
		ct->error  = E_TMOUT;
		return;
		}

	/* RELEASE メッセージを作成する。*/
	if ((error = setup_release_msg(ct, ct->act_msg, DHCP6_RELEASE)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, setup release message: %s.", itron_strerror(error));
		ct->error = error;
		return;
		}

	/* RELEASE メッセージを送信する。*/
	error = UDP_SND_DAT(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, send release message: %s.", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 再送信タイムアウトを設定する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_REL] = SYSTIM2TIMER(ct->RT);
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	/* 再送信タイムアウトを更新する。*/
	advance_retrans(ct);
	}

/*
 *  advertise_msg_timeout -- ADVERTISE メッセージのタイムアウト処理
 */

static void
advertise_msg_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	/* SOLICIT メッセージ送信タイマーを停止する。*/
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_SOL] = 0;
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

	if (select_adv(ct)) {
		/*
		 *  ADVERTISE メッセージの選択が終了していれば、
 		 *  REQUEST メッセージを送信して SELECT 状態に遷移する。
 		 */
 		start_select(ct);
		}
	else
		ct->error  = E_TMOUT;
	}

/*
 *  renew_timeout -- RENEW タイムアウト処理
 */

static void
renew_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	/* RENEW 状態に遷移する。*/
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	start_renew(ct);
	}

/*
 *  rebind_timeout -- REBIND タイムアウト処理
 */

static void
rebind_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	/* REBIND 状態に遷移する。*/
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	start_rebind(ct);
	}

/*
 *  depref_timeout -- DEPREF タイムアウト処理
 */

static void
depref_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPTION		*opt;
	T_DHCP6_OPT_IA_NA_PD	*ia;
	T_DHCP6_OPT_IAADDR	*iaa;
	SYSTIM		time;
	uint8_t		*msg, *last;
	uint_t		aoff, asize;

	/* 有効なリースが無ければ処理終了 */
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	if (ct->act_msg == NULL) {
		return;
		}

	get_tim(&time);
	msg = ct->act_msg->msg.options;
	last = msg + (ct->act_msg->len - sizeof(T_DHCP6_MSG_HDR));
	while (msg < last) {

		/* IA_NA/IA_PD オプションであれば、推奨有効時間を確認する。*/
		opt = (T_DHCP6_OPTION*)msg;
		if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_NA || MSG2HOST16(opt->code) == DHCP6_OPT_IA_PD) {

			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			asize = MSG2HOST16(ia->len) + sizeof(*opt);
			for (aoff  = sizeof(*ia); aoff < asize;
				                  aoff += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(*iaa)) {

				/* すでに推奨有効時間を経過していれば処理しない。*/
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				if ((MSG2HOST16(iaa->len) & DHCP6C_IAA_FLAG_DEPREFERD))
					continue;

				/* 推奨有効時間経過していれば FLAG を設定する。*/
				if (TIMEC_GE(time, ct->act_msg->rcv_time + MSG2HOST32(iaa->prefer))) {
					HOST2MSG16(iaa->len, MSG2HOST16(iaa->len) | DHCP6C_IAA_FLAG_DEPREFERD);
					if (MSG2HOST16(ia->code) == DHCP6_OPT_IA_NA)
						syslog(LOG_NOTICE, "[DHCP6C] address depreferd: %s.", ipv62str(NULL, &iaa->addr));
					else
						syslog(LOG_NOTICE, "[DHCP6C] prefix  depreferd: %s.", ipv62str(NULL, &iaa->addr));

					/*【DNS の処理が必要】*/
					}
				}
			}
		msg += MSG2HOST16(opt->len) + sizeof(*opt);
		}

	eval_expire(ct);
	}

/*
 *  expire_timeout -- EXPIRE タイムアウト処理
 */

static void
expire_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_OPTION		*opt;
	T_DHCP6_OPT_IA_NA_PD	*ia;
	T_DHCP6_OPT_IAADDR	*iaa;
	SYSTIM		time;
	uint8_t		*msg, *last;
	uint_t		aoff, asize, addrs = 0;

	/* 有効なリースが無ければ処理終了 */
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	if (ct->act_msg == NULL) {
		return;
		}

	get_tim(&time);
	msg = ct->act_msg->msg.options;
	last = msg + (ct->act_msg->len - sizeof(T_DHCP6_MSG_HDR));
	while (msg < last) {

		/* IA_NA/IA_PD オプションであれば、有効時間を確認する。*/
		opt = (T_DHCP6_OPTION*)msg;
		if (MSG2HOST16(opt->code) == DHCP6_OPT_IA_NA || MSG2HOST16(opt->code) == DHCP6_OPT_IA_PD) {

			ia = (T_DHCP6_OPT_IA_NA_PD*)opt;
			asize = MSG2HOST16(ia->len) + sizeof(*opt);
			for (aoff  = sizeof(*ia); aoff < asize;
				                  aoff += (MSG2HOST16(iaa->len) & ~DHCP6C_IAA_FLAG_MASK) + sizeof(*iaa)) {

				/* すでに有効時間を経過していれば処理しない。*/
				iaa = (T_DHCP6_OPT_IAADDR*)((uint8_t*)opt + aoff);
				if ((MSG2HOST16(iaa->len) & DHCP6C_IAA_FLAG_EXPIRED))
					continue;

				/* 有効時間経過していれば FLAG を設定する。*/
				if (TIMEC_GE(time, ct->act_msg->rcv_time + MSG2HOST32(iaa->valid))) {
					HOST2MSG16(iaa->len, MSG2HOST16(iaa->len) | DHCP6C_IAA_FLAG_EXPIRED);
					if (MSG2HOST16(ia->code) == DHCP6_OPT_IA_NA)
						syslog(LOG_NOTICE, "[DHCP6C] address expired: %s.", ipv62str(NULL, &iaa->addr));
					else
						syslog(LOG_NOTICE, "[DHCP6C] prefix  expired: %s.", ipv62str(NULL, &iaa->addr));

					/*【DNS の処理が必要】*/
					continue;
					}
				addrs ++;
				}
			}
		msg += MSG2HOST16(opt->len) + sizeof(*opt);
		}

	/* 有効なアドレスが全て無くなった時は INIT 状態に遷移する。*/
	if (addrs == 0) {

	 	/* ACTIVE リースを受信メッセージ構造体リストに戻す。*/
		syslog(LOG_NOTICE, "[DHCP6C] all address expired.");
		syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
		ct->act_msg->next = ct->rcv_lst;
		ct->rcv_lst = ct->act_msg;
		ct->act_msg = NULL;
		syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

		/* 全ての TIMER をキャンセルする。*/
		cancel_all_timers(ct);

		/* INIT 状態に遷移する。*/
		start_init(ct);
		return;
		}

	eval_expire(ct);
	}

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

/*
 *  reply_msg_timeout -- REPLY メッセージのタイムアウト処理
 */

static void
reply_msg_timeout (T_DHCP6_CLI_CONTEXT *ct)
{
	/* SOLICIT メッセージ送信タイマーを停止する。*/
	ct->flags &= ~DHCP6C_FLAG_TMOUT_MASK;
	syscall(wai_sem(SEM_DHCP6_CLI_TIMER));
	ct->timers[DHCP6C_TIMER_SND_REQ] = 0;
	ct->timers[DHCP6C_TIMER_SND_REF] = 0;
	syscall(sig_sem(SEM_DHCP6_CLI_TIMER));

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	/* STATELES モードではSLEEP に遷移する。*/
	ct->error = E_TMOUT;
	ct->fsm   = DHCP6_FSM_SLEEP;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	/* RENEW メッセージの選択が終了していれば、BOUND 状態に遷移する。*/
	if (select_reply(ct)) {

		/* リース情報を解放する。*/
		rel_lease(ct);

		/* BOUND 状態に遷移する。*/
		start_bound(ct);
		}
	else if (ct->fsm == DHCP6_FSM_RENEW || ct->fsm == DHCP6_FSM_REBIND)
		eval_expire(ct);
	else {
		/* SLEEP に遷移する。*/
		ct->error = E_TMOUT;
		ct->fsm   = DHCP6_FSM_SLEEP;
		}

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */
	}

/*
 *  dispatch_timeout -- タイムアウトした時の処理
 */

static ER
dispatch_timeout (T_DHCP6_CLI_CONTEXT *ct)
{

	switch (ct->flags & DHCP6C_FLAG_TMOUT_TIMER_MASK) {

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	case DHCP6C_TIMER_SND_IREQ:
		send_info_req_msg(ct);
		break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA

	case DHCP6C_TIMER_SND_SOL:
		send_solicit_msg(ct);
		break;

	case DHCP6C_TIMER_SND_REQ:
		send_request_msg(ct);
		break;

	case DHCP6C_TIMER_SND_REF:
		send_refresh_msg(ct);
		break;

	case DHCP6C_TIMER_SND_REL:
		send_release_msg(ct);
		break;

	case DHCP6C_TIMER_RCV_ADV:
		advertise_msg_timeout(ct);
		break;

	case DHCP6C_TIMER_RENEW:
		renew_timeout(ct);
		break;

	case DHCP6C_TIMER_REBIND:
		rebind_timeout(ct);
		break;

	case DHCP6C_TIMER_DEPREF:
		depref_timeout(ct);
		break;

	case DHCP6C_TIMER_EXPIRE:
		expire_timeout(ct);
		break;

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL || DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_RA */

	case DHCP6C_TIMER_RCV_REPLY:
		reply_msg_timeout(ct);
		break;

	default:
		syslog(LOG_NOTICE, "[DHCP6C] error, unknown timeout: %d.", ct->flags & DHCP6C_FLAG_TMOUT_TIMER_MASK);
		break;
		}

	return ct->error;
	}

/*
 *  dispatch_event -- イベント毎の処理
 */

static ER
dispatch_event (T_DHCP6_CLI_CONTEXT *ct)
{
	ER	error = E_OK;

	while (ct->fsm != DHCP6_FSM_SLEEP && error == E_OK) {

		/* メッセージの送受信とタイムアウトを待つ。*/
		syscall(wai_sem(SEM_DHCP6_CLI_READY));

		if (ct->flags & DHCP6C_FLAG_RCV_MSG) {
			while (ct->val_lst != NULL) {
				error = eval_rcv_msg(ct);
				}
			ct->flags &= ~DHCP6C_FLAG_RCV_MSG;
			}

		if (ct->flags & DHCP6C_FLAG_TMOUT_MASK)
			error = dispatch_timeout(ct);

		if (ct->error != E_OK)
			error = ct->error;
		}

	return error;
	}

/*
 *  init_context -- DHCPv6 クライアントコンテキスト構造体を初期化する。
 */

static void
init_context (T_DHCP6_CLI_CONTEXT *ct, ID cepid)
{
  	memset(ct, 0, sizeof(*ct));
	ct->cepid	= cepid;
	ct->sc		= IF_ETHER_NIC_GET_SOFTC();
	ct->dst.portno	= DHCP6_SRV_CFG_PORTNO;
	}

/*
 *  rel_cli_msg -- メッセージ構造体を解放する。
 */

static void
rel_cli_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG	*ptr, *cell;

	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	for (ptr = ct->rcv_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)cell));
		}

	for (ptr = ct->val_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)cell));
		}

	for (ptr = ct->prf_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)cell));
		}

	if (ct->snd_msg != NULL)
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)ct->snd_msg));
	if (ct->adv_msg != NULL)
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)ct->adv_msg));
	ct->rcv_lst = ct->prf_lst = ct->val_lst = ct->snd_msg = ct->adv_msg = NULL;

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS

	/* ステートレスの時は有効なリース・メッセージ構造体も解放する。*/
	if (ct->act_msg != NULL) {
		syscall(rel_mpf(MPF_DHCP6_CLI_MSG, (void*)ct->act_msg));
		ct->act_msg = NULL;
		}

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
	}

/*
 *  init_cli_msg -- 各メッセージ構造体を初期化する。
 */

static ER
init_cli_msg (T_DHCP6_CLI_CONTEXT *ct)
{
	T_DHCP6_CLI_MSG	*cell;
	ER		error;
	int		count;

	/* 送信メッセージ構造体を獲得する。*/
	if ((error = tget_mpf(MPF_DHCP6_CLI_MSG, (void*)&ct->snd_msg, TMO_DHCP6C_MPF_GET)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP6C] error, tget_mpf() for send: %s.", itron_strerror(error));
		return error;
		}

	/*
	 *  有効なリース・メッセージ構造体がある場合は、
	 *  獲得する受信メッセージ構造体リストの構造体を 1減らす。
	 */
	if (ct->act_msg == NULL)
		count = NUM_DHCP6_CLI_MSG_LIST;
	else
		count = NUM_DHCP6_CLI_MSG_LIST - 1;

	/* 受信メッセージ構造体を獲得する。*/
	syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
	while (count --> 0) {
		if ((error = tget_mpf(MPF_DHCP6_CLI_MSG, (void*)&cell, TMO_DHCP6C_MPF_GET)) != E_OK) {
			syslog(LOG_NOTICE, "[DHCP6C] error, tget_mpf() for receive: %s.", itron_strerror(error));
			break;
			}
		cell->next  = ct->rcv_lst;
		ct->rcv_lst = cell;
		}
	syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

	return E_OK;
	}

/*
 *  dhcp6c_renew_info -- ネットワーク情報を再取得する。
 */

ER
dhcp6c_renew_info (void)
{

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS

	if (context.fsm == DHCP6_FSM_INFORMED) {

		/* REL_INFO 状態に遷移した後に SLEEP を解除するように設定する。*/
		context.flags |= DHCP6C_FLAG_RENEW;
		start_rel_info(&context);
		return E_OK;
		}
	else if (context.fsm == DHCP6_FSM_SLEEP) {

		/* SLEEP を解除する。*/
		wup_tsk(DHCP6_CLI_TASK);
		return E_OK;
		}
	else
		return E_OBJ;

#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

	if (context.fsm == DHCP6_FSM_BOUND) {

		/* RENEW 状態に遷移する。*/
		start_renew(&context);
		return E_OK;
		}
	else if (context.fsm == DHCP6_FSM_SLEEP) {

		/* SLEEP を解除する。*/
		wup_tsk(DHCP6_CLI_TASK);
		return E_OK;
		}
	else
		return E_OBJ;

#else	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

	}

/*
 *  dhcp6c_rel_info -- DHCP で設定されたネットワーク情報を解放する。
 */

ER
dhcp6c_rel_info (void)
{
	/* REL_INFO 状態に遷移する。*/
	start_rel_info(&context);
	return E_OK;
	}

/*
 *  dhcp6c_get_info -- DHCP で設定されたネットワーク情報を返す。
 */

ER
dhcp6c_get_info (T_IN6_ADDR *addr, uint32_t *expire, uint32_t *renew,
                                   uint32_t *rebind, uint32_t *deprefer, SYSTIM *bind_start)
{
	if (addr   == NULL || expire ==   NULL || renew == NULL || 
	    rebind == NULL || deprefer == NULL || bind_start == NULL)
		return E_PAR;
	else if (context.act_msg == NULL)
		return E_OBJ;
	else {
		//            context.timers[DHCP6C_TIMER_EXPIRE],
		//            context.timers[DHCP6C_TIMER_RENEW],
		//            context.timers[DHCP6C_TIMER_REBIND],
		//            context.timers[DHCP6C_TIMER_DEPREF]);

		*addr = context.act_msg->srv.ipaddr;
		*expire		= context.timers[DHCP6C_TIMER_EXPIRE];
		*renew		= context.timers[DHCP6C_TIMER_RENEW];
		*rebind		= context.timers[DHCP6C_TIMER_REBIND];
		*deprefer	= context.timers[DHCP6C_TIMER_DEPREF];
		*bind_start	= context.act_msg->rcv_time;
		return E_OK;
		}
	}

/*
 *  コールバック関数
 */

ER
callback_nblk_dhcp6_cli (ID cepid, FN fncd, void *p_parblk)
{
	T_DHCP6_CLI_MSG	*msg;
	ER_UINT		len;

	len = *(ER_UINT*)p_parblk;
	if (len < 0 && len != E_RLWAI) {
		/* E_RLWAI 以外で、0 以下の場合は、エラーコード */
		syslog(LOG_NOTICE, "[DHCP6C(CBR)] error: %s, fncd: %s.", itron_strerror(len), in_strtfn(fncd));
		}
	else {
		if (fncd == TEV_UDP_RCV_DAT) {

			/* 受信メッセージリストが空の時は、受信しないでデータグラムを捨てる。*/
			if (context.rcv_lst == NULL) 
				syslog(LOG_NOTICE, "[DHCP6C(CBR)] receive buffer all busy.");
			else {
				/* メッセージを受信するメッセージ構造体を準備する。*/
				syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
				msg = context.rcv_lst;
				context.rcv_lst = context.rcv_lst->next;
				syscall(sig_sem(SEM_DHCP6_CLI_LOCK));

				/* メッセージを受信する。*/
				len = UDP_RCV_DAT(context.cepid, &msg->srv, &msg->msg, sizeof(msg->msg), TMO_POL);

				syscall(wai_sem(SEM_DHCP6_CLI_LOCK));
				msg->len = len;
				if (len >= 0) {
					/* 受信したメッセージを検証メッセージリストに移す。*/
					msg->next = context.val_lst;
					context.val_lst = msg;
					context.flags |= DHCP6C_FLAG_RCV_MSG;
					}
				else {
					/* メッセージ構造体を受信メッセージリストに戻す。*/
					context.error = len;
					msg->next = context.rcv_lst;
					context.rcv_lst = msg;
					}
				syscall(sig_sem(SEM_DHCP6_CLI_LOCK));
				}
			}
		syscall(sig_sem(SEM_DHCP6_CLI_READY));
		}
	return E_OK;
	}
/*
 *  dhcp6_cli_task -- DHCPv6 クライアントタスク
 */

void
dhcp6_cli_task (intptr_t exinf)
{
	T_DHCP6_CLI_CONTEXT	*ct;
	ID			tskid;
	ER			error;

	dly_tsk(1000);
	get_tid(&tskid);

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
	syslog(LOG_NOTICE, "[DHCP6C:%d] started in stateless.", tskid);
#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL
	syslog(LOG_NOTICE, "[DHCP6C:%d] started in statefull.", tskid);
#else
#endif

	dly_tsk(500);
	ct = &context;

	/* DHCP クライアントコンテキスト構造体を初期化する。*/
	init_context(ct, (ID)exinf);

	while (true) {

		/* 1秒毎にタイムアウトするようにタイマーを設定する。*/
		timeout((callout_func)dhcpc_timer, ct, NET_TIMER_HZ / DHCP6C_TIMER_HZ);

		/* クライアント DUID を設定する。*/
		init_duid(ct);

		/* メッセージ構造体を初期化する。*/
		if ((error = init_cli_msg(ct))  == E_OK) {

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
			start_req_info(ct);
#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL
			start_init(ct);
#else
#endif

			/* メインループ */
			error = dispatch_event(ct);
			}

		/* 全ての TIMER をキャンセルする。*/
		cancel_all_timers(ct);

		/* タイマーを停止する。*/
		untimeout((callout_func)dhcpc_timer, ct);

		/* メッセージ構造体を解放する。*/
		rel_cli_msg(ct);

		/* リースを解放後、再取得が指定されていなければ休止する。*/
		if (!(ct->flags & DHCP6C_FLAG_RENEW)) {

			/* 休止する。*/
			if (error == E_OK)
				syslog(LOG_NOTICE, "[DHCP6C] lease released, go to sleep.");
			else {
				syslog(LOG_NOTICE, "[DHCP6C] server not available, go to sleep, error: %s.", itron_strerror(error));
				ct->fsm = DHCP6_FSM_SLEEP;
				}
			slp_tsk();
			}
		ct->flags = 0;
		ct->error = E_OK;
		}
	}

#endif	/* of #ifdef DHCP6_CLI_CFG */
