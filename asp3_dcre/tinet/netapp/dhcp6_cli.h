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

#ifndef DHCP6_CLI_H
#define DHCP6_CLI_H

#ifdef DHCP6_CLI_CFG

#include <netinet/in.h>
#include <netapp/dhcp6.h>

/* 
 *  DHCP クライアントのスタックサイズ
 */

#define DHCP6_CLI_STACK_SIZE		1024		/* DHCP クライアントタスクのスタックサイズ	*/

/* 
 *  DHCP クライアントの優先度
 */

#define DHCP6_CLI_MAIN_PRIORITY		5		/* DHCP クライアントタスクの優先度		*/

/* 
 *  コンパイル時コンフィギュレーション
 */

/*
 *  DHCPv6 動作モード
 *
 *    #define DHCP6_CLI_CFG_MODE で指定する。
 *    指定されていなければ、ステートレスに設定する。
 *
 */

#define DHCP6_CLI_CFG_STATELESS		0	/* ステートレスに設定する。		*/
#define DHCP6_CLI_CFG_STATEFULL		1	/* ステートフルに設定する。		*/
#define DHCP6_CLI_CFG_RA		2	/* RA の M/O ビットにより設定する。	*/
#define DHCP6_CLI_CFG_MODE_DEFAULT	DHCP6_CLI_CFG_STATELESS

/* IA の設定 */
#define DHCP6_CLI_CFG_IA_NA		1
//#define DHCP6_CLI_CFG_IA_TA		1
//#define DHCP6_CLI_CFG_IA_PD		1

/* Rapid-commit の指定 */
//#define DHCP6_CLI_CFG_RAPID_COMMIT	1

/* 推奨度の定義 */
#define DHCP6_CLI_CFG_PREFER_IA		50	/* IA_NA/TA/PD 当たりの推奨度	*/
#define DHCP6_CLI_CFG_PREFER_IAADDR	100	/* アドレス当たりの推奨度	*/

/* DHCPv6 クライアントメッセージの受信リストサイズ（2以上指定すること） */
#define NUM_DHCP6_CLI_MSG_LIST		3

/*
 *  DHCPv6 クライアントメッセージ構造体数【変更禁止】
 *  +1 は送信用
 */
#define NUM_MPF_DHCP6_CLI_MSG		(NUM_DHCP6_CLI_MSG_LIST+1)

/* DHCP 必須オプションリスト */

#ifndef DHCP6_CLI_CFG_REQUIRED_OLIST

#if defined(USE_RESOLVER)

#define DHCP6_CLI_CFG_REQUIRED_OLIST {	\
	DHCP6_OPT_NAME_SERVERS,		\
	}

#else	/* of #if defined(USE_RESOLVER) */

#endif	/* of #if defined(USE_RESOLVER) */

#endif	/* of #ifndef DHCP6_CLI_CFG_REQUIRED_OLIST */

/* DHCP 要求オプションリスト */

#ifndef DHCP6_CLI_CFG_REQUEST_OLIST

#if defined(USE_RESOLVER)

#define DHCP6_CLI_CFG_REQUEST_OLIST {	\
	DHCP6_OPT_DOMAIN_SEARCH,	\
	}

#else	/* of #if defined(USE_RESOLVER) */

#endif	/* of #if defined(USE_RESOLVER) */

#endif	/* of #ifndef DHCP6_CLI_CFG_REQUEST_OLIST */

/*
 *  タイムアウト定数
 *  RFC3315 に規定されたタイミング定数は dhcp6.h 参照
 */

/* 標準メッセージの再送信タイムアウト	*/
#define TMO_DHCP6C_NORMAL	ULONG_C(60*1000)

/* 標準要求リース時間			*/
#define TMO_DHCP6C_REQ_LEASE	ULONG_C(7200*1000)

/* メッセージ構造体獲得タイムアウト	*/
#define TMO_DHCP6C_MPF_GET	ULONG_C(1000)

/* メッセージ受信タイムアウト	*/
#define TMO_DHCP6C_RCV_DAT	ULONG_C(1000)

/*
 *  DHCPv6 クライアントメッセージ構造体
 */

typedef struct dhcp6_cli_msg {
	/* 4バイト境界にアラインする。*/
	T_DHCP6_MSG		msg;		/* DHCPv6 メッセージ構造体	*/
	struct dhcp6_cli_msg	*next;		/* リストの次のメッセージ構造体	*/
	SYSTIM			rcv_time;	/* 受信した時間			*/
	T_IPV6EP		srv;		/* サーバのIPv6アドレス/ポート番号 */
	uint16_t		len;		/* msg の有効長			*/
	uint16_t		prefer;		/* msg の推奨度			*/
	uint16_t		status;		/* STATUS CODE オプションの status */
	} T_DHCP6_CLI_MSG;

/* prefer のフラグ */

#define DHCP6_ClI_MSG_PREFER_ADDR	UINT_C(0x8000)	/* アドレス情報あり		*/
#define DHCP6_ClI_MSG_PREFER_REQUIRED	UINT_C(0x4000)	/* 必須オプションあり		*/
#define DHCP6_ClI_MSG_PREFER_REQUEST	UINT_C(0x2000)	/* 要求オプションあり		*/
#define DHCP6_ClI_MSG_PREFER_OPTION	UINT_C(0x1000)	/* PREFER オプションあり	*/
#define DHCP6_ClI_MSG_PREFER_FLAG_MASK	UINT_C(0xf000)
#define DHCP6_ClI_MSG_PREFER_VALUE_MASK	(~(DHCP6_ClI_MSG_PREFER_FLAG_MASK))

/*
 *  STATUS CODE オプションが無い時の値
 */

#define DHCP6_CLI_STAT_UNDEFINED	(-1)

/*
 *  コールバック関数
 */

extern ER callback_nblk_dhcp6_cli (ID cepid, FN fncd, void *p_parblk);

/*
 *  タスク
 */

extern void	dhcp6_cli_task(intptr_t exinf);

/*
 *  関数
 */

extern ER dhcp6c_rel_info (void);
extern ER dhcp6c_renew_info (void);
extern ER dhcp6c_get_info (T_IN6_ADDR *addr, uint32_t *expire, uint32_t *renew,
                                             uint32_t *rebind, uint32_t *deprefer, SYSTIM *bind_start);

#endif	/* of #ifdef DHCP6_CLI_CFG */

#endif	/* of #ifndef DHCP6_CLI_H */
