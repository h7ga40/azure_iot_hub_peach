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

#ifndef DHCP4_CLI_H
#define DHCP4_CLI_H

#include <netapp/dhcp4.h>

/* 
 *  DHCP クライアントのスタックサイズ
 */

#define DHCP4_CLI_STACK_SIZE		1024		/* DHCP クライアントタスクのスタックサイズ	*/

/* 
 *  DHCP クライアントの優先度
 */

#define DHCP4_CLI_MAIN_PRIORITY		5		/* DHCP クライアントタスクの優先度		*/

/* 推奨度の定義 */
#define DHCP4_CLI_CFG_PREFER_ADDR	100		/* アドレスの推奨度	*/

/* DHCP クライアントメッセージの受信リストサイズ（2以上指定すること） */
#define NUM_DHCP4_MSG_LIST		3

/*
 *  DHCP クライアントメッセージ構造体数【変更禁止】
 *  +1 は送信用
 */
#define NUM_MPF_DHCP4_CLI_MSG		(NUM_DHCP4_MSG_LIST+1)

/* 
 *  コンパイル時コンフィギュレーション
 */

/* DHCP 必須オプションリスト */

#ifndef DHCP4_CLI_CFG_REQUIRED_OLIST

#define DHCP4_CLI_CFG_REQUIRED_OLIST {	\
	DHCP4_OPT_SUBNET_MASK,		\
	DHCP4_OPT_ROUTER,		\
}

#endif	/* of #ifndef DHCP4_CLI_CFG_REQUIRED_OLIST */

/* DHCP 要求オプションリスト */

#ifndef DHCP4_CLI_CFG_REQUEST_OLIST

#if defined(USE_RESOLVER)

#define DHCP4_CLI_CFG_REQUEST_OLIST {	\
	DHCP4_OPT_DNS_SERVER,		\
	DHCP4_OPT_DOMAIN_NAME,		\
}

#endif	/* of #if defined(USE_RESOLVER) */

#endif	/* of #ifndef DHCP4_CLI_CFG_REQUEST_OLIST */

/*
 *  タイムアウト値
 */

/* 受信タイムアウト	*/
#define TMO_DHCP4C_OFFER	(ULONG_C(120)*SYSTIM_HZ)	/* OFFER メッセージ	*/
#define TMO_DHCP4C_ACK		(ULONG_C(60)*SYSTIM_HZ)	/* ACK/NAK メッセージ	*/

/* リース時間の既定値	*/
#define TMO_DHCP4C_DEF_LEASE	(ULONG_C(12*60*60)*SYSTIM_HZ)	/* 標準時間	*/
#define TMO_DHCP4C_MIN_LEASE	(ULONG_C(60)*SYSTIM_HZ)	/* 最小時間	*/

/* メッセージの送信周期	*/
#define TMO_DHCP4C_INIT_INTVL	(ULONG_C(3)*SYSTIM_HZ)		/* 初期値	*/
#define TMO_DHCP4C_MAX_BACKOFF	(ULONG_C(15)*SYSTIM_HZ)	/* 制限値	*/

/* DHCP サーバへの UDP 送信タイムアウト	*/
#define TMO_DHCP4C_UDP_SND	ULONG_C(1000000)		/* [us] */

/* メッセージ構造体獲得タイムアウト	*/
#define TMO_DHCP4C_MPF_GET	ULONG_C(1000000)		/* [us] */

/* ARP 要求を送信してからの待ち時間	*/
#define TMO_DHCP4C_ARP_WAIT	ULONG_C(1000000)		/* [us] */

/* RELEASE メッセージを送信してからの待ち時間	*/
#define TMO_DHCP4C_SND_REL_WAIT	ULONG_C(1000000)	/* [us] */

#ifndef _MACRO_ONLY

#ifdef T_IPV4EP_DEFINED

/*
 *  DHCP クライアントメッセージ構造体
 */

typedef struct dhcp4_cli_msg {
	/* 4バイト境界にアラインする。*/
	T_DHCP4_MSG		msg;		/* DHCP メッセージ構造体	*/
	struct dhcp4_cli_msg	*next;		/* リストの次のメッセージ構造体	*/
	T_IPV4EP		srv;		/* サーバのIPv4アドレス/ポート番号 */
	uint16_t		len;		/* msg の有効長			*/
	uint16_t		prefer;		/* msg の推奨度			*/
	uint8_t			type;		/* メッセージタイプ		*/
	} T_DHCP4_CLI_MSG;

#endif	/*of #ifdef T_IPV4EP_DEFINED */

/*
 *  コールバック関数
 */

extern ER callback_nblk_dhcp4_cli (ID cepid, FN fncd, void *p_parblk);

/*
 *  タスク
 */

extern void	dhcp4_cli_task(intptr_t exinf);

/*
 *  関数
 */

#ifdef T_IN4_ADDR_DEFINED

extern ER dhcp4c_rel_info (void);
extern ER dhcp4c_renew_info (void);
extern ER dhcp4c_get_info (T_IN4_ADDR *addr, uint32_t *expire, uint32_t *renew,
                                             uint32_t *rebind, SYSTIM *bind_start);

#endif	/* of #ifdef T_IN4_ADDR_DEFINED */

#endif	/* of #ifndef _MACRO_ONLY */

#endif	/* of #ifndef DHCP4_CLI_H */
