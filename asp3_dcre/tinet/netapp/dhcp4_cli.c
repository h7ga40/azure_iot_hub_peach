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
#include <netinet/if_ether.h>

#include <net/if_var.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/dhcp4_cli.h>
#include <netapp/dhcp4_cli_var.h>
#include <netapp/resolver.h>
#include "lcd.h"

/*
 *  DHCP クライアントタスク
 */

#ifdef DHCP4_CLI_CFG

/*
 *  変数
 */

static T_DHCP4_CLI_CONTEXT	context;			/* クライアントコンテキスト	*/
static uint8_t	magicno[DHCP4_MAGIC_LEN] = DHCP4_RFC1048_MAGIC;	/* magic 番号（RFC1048）*/

/* 必須オプションリスト */
#ifdef DHCP4_CLI_CFG_REQUIRED_OLIST
static uint8_t	required_plist[] = DHCP4_CLI_CFG_REQUIRED_OLIST;
#endif

/* 要求オプションリスト */
#ifdef DHCP4_CLI_CFG_REQUEST_OLIST
static uint8_t	request_plist[] = DHCP4_CLI_CFG_REQUEST_OLIST;
#endif

#ifdef SEM_DHCP4_CLI_TIMER
#define DHCP4_CLI_TIMER_LOCK() do { syscall(wai_sem(SEM_DHCP4_CLI_TIMER)); } while(0)
#define DHCP4_CLI_TIMER_UNLOCK() do { syscall(sig_sem(SEM_DHCP4_CLI_TIMER)); } while(0)
#else
#define DHCP4_CLI_TIMER_LOCK() do { } while(0)
#define DHCP4_CLI_TIMER_UNLOCK() do { } while(0)
#endif

#ifdef SEM_DHCP4_CLI_READY
#define DHCP4_CLI_READY_WAIT(ct) do { syscall(wai_sem(SEM_DHCP4_CLI_READY)); } while(0)
#define DHCP4_CLI_READY_SIGNAL(ct) do { syscall(sig_sem(SEM_DHCP4_CLI_READY)); } while(0)
#else
#define DHCP4_CLI_READY_WAIT(ct) do { } while(0)
#define DHCP4_CLI_READY_SIGNAL(ct) do { syscall(wup_tsk(ct->tskid)); } while(0)
#endif

/*
 *  dhcp4_cksum -- チェックサムの計算関数
 */

static uint32_t
dhcp4_cksum (void *data, uint_t len /*オクテット単位*/)
{
	uint32_t carry, sum = 0;

	for ( ; len > 0; len -= 2) {
		sum += (*(uint8_t*)data << 8) + *((uint8_t*)data + 1);
		data = (uint8_t*)data + 2;
		}

	while (sum >= 0x00010000) {
		carry = sum >> 16;
		sum   = (sum & 0xffff) + carry;
		}

	return sum;
	}

/*
 *  create_xid -- トランザクション ID を生成する。
 */

static uint32_t
create_xid (void)
{
	T_IF_SOFTC	*ic;
	SYSTIM		time;

	ic = IF_ETHER_NIC_GET_SOFTC();
	get_tim(&time);
	return (dhcp4_cksum((void*)&ic->ifaddr.lladdr, ETHER_ADDR_LEN) << 16)
	      + dhcp4_cksum((void*)&time, sizeof(time));
	}

/*
 *  find_option_sub -- 指定したオプションを探索する。
 *
 *    注意: 引数 size が 0 の時はオプションサイズを検証しない。
 */

static uint8_t *
find_option_sub (T_DHCP4_CLI_MSG *msg, uint_t off, uint_t len, 
                                      uint_t tag, uint_t size, bool_t overload)
{
	uint8_t	*opt, *tmp;

	opt = (uint8_t*)&msg->msg + off;
	while (opt < (uint8_t*)&msg->msg + off + len && *opt != DHCP4_OPT_END) {
		if (*opt == DHCP4_OPT_PAD)
			opt ++;
		else if ((*opt == DHCP4_OPT_OPT_OVERLOAD) && overload) {
			if (*(opt + 2) & DHCP4_OPTOL_FILE) {
				tmp = find_option_sub(msg, DHCP4_FILE_OFFSET, DHCP4_FILE_LEN, tag, len, false);
				if (tmp != NULL)
					return tmp;
				}
			if (*(opt + 2) & DHCP4_OPTOL_SNAME) {
				tmp = find_option_sub(msg, DHCP4_SNAME_OFFSET, DHCP4_SNAME_LEN, tag, len, false);
				if (tmp != NULL)
					return tmp;
				}
			opt += *(opt + 1) + 2;
			}
		else if (*opt == tag && (size == 0 || *(opt + 1) == size))
			return opt;
		else
			opt += *(opt + 1) + 2;
		}
	return NULL;
	}

/*
 *  find_option -- 指定したオプションを探索する。
 *
 *    注意: 引数 size が 0 の時はオプションサイズを検証しない。
 */

static uint8_t *
find_option (T_DHCP4_CLI_MSG *msg, uint_t tag, uint_t size)
{
	return find_option_sub(msg, DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN,
	                       msg->len - (DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN),
	                       tag, size, true);
	}

/*
 *  is_equal_sid -- メッセージ内の SERVER ID を比較する。
 */

static bool_t
is_equal_sid (T_DHCP4_CLI_MSG *msg1, T_DHCP4_CLI_MSG *msg2)
{
	uint8_t	*opt1, *opt2;

	if ((opt1 = find_option(msg1, DHCP4_OPT_SERVER_ID, sizeof(T_IN4_ADDR))) == NULL)
		return false;
	if ((opt2 = find_option(msg2, DHCP4_OPT_SERVER_ID, sizeof(T_IN4_ADDR))) == NULL)
		return false;

	return msg2host32(opt1) == msg2host32(opt2);
	}

/*
 *  timer_value -- メッセージから時間情報を取り出す。
 */

static uint32_t
timer_value (T_DHCP4_CLI_MSG *msg, uint_t tag, uint32_t def_value)
{
	uint8_t 	*opt;
	uint32_t	seconds;

	if ((opt = find_option(msg, tag, sizeof(uint32_t))) == NULL)
		return def_value;
	else {

		/* サーバからのリース時間は[s]。*/
		seconds = msg2host32(opt + 2);

		/* コンテキスト内のリース時間は SYSTIM_HZ。*/
		if (seconds == DHCP4_TIME_INFINITY)
			return DHCP4_TIME_INFINITY;
		else if (seconds > SYSTIM2SEC(DHCP4_TIME_INFINITY))
			return DHCP4_TIME_INFINITY;
		else
			return SEC2SYSTIM(seconds);
		}
	}

/*
 *  rel_addr -- IPv4 アドレス、サブネットマスク、静的経路表を解放する。
 */

static void
rel_addr (void)
{
	ER	error;
	uint_t	count = 0;

	if ((error = in4_add_ifaddr(IPV4_ADDRANY, IPV4_ADDRANY)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, IPv4 addr: %s", itron_strerror(error));
		count ++;
		}

	if ((error = in4_add_route(0, IPV4_ADDRANY, IPV4_ADDRANY, IPV4_ADDRANY)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, release routing table: %s", itron_strerror(error));
		count ++;
		}

	if ((error = in4_add_route(1, IPV4_ADDRANY, IPV4_ADDRANY, IPV4_ADDRANY)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, release routing table: %s", itron_strerror(error));
		count ++;
		}

	if ((error = in4_add_route(2, 0xffffffff, 0xffffffff, IPV4_ADDRANY)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, release routing table: %s", itron_strerror(error));
		count ++;
		}

	if (count == 0) {
		syslog(LOG_NOTICE, "[DHCP4C] release IPv4 addr.");

#if 0
#ifdef USE_LCD
		dly_tsk(1 * 1000);
		lcd_puts(LCD_PORTID, "0.0.0.0\n");
#endif	/* of #ifdef USE_LCD */
#endif
		ether_set_down();
		}
	}

/*
 *  rel_options -- リースされたオプションを解析して値を解放する。
 */

static void
rel_options (T_DHCP4_CLI_CONTEXT *ct, uint_t off, uint_t len, bool_t overload)
{
#if defined(USE_RESOLVER)
	T_IN4_ADDR	addr;
#endif

	uint8_t	*opt;

	opt = (uint8_t*)&ct->act_msg->msg + off;
	while (opt < (uint8_t*)&ct->act_msg->msg + off + len && *opt != DHCP4_OPT_END) {
		if (*opt == DHCP4_OPT_PAD)
			opt ++;
		else if ((*opt == DHCP4_OPT_OPT_OVERLOAD) && overload) {
			if (*(opt + 2) & DHCP4_OPTOL_FILE)
				rel_options(ct, DHCP4_FILE_OFFSET, DHCP4_FILE_LEN, false);
			if (*(opt + 2) & DHCP4_OPTOL_SNAME)
				rel_options(ct, DHCP4_SNAME_OFFSET, DHCP4_SNAME_LEN, false);
			opt += *(opt + 1) + 2;
			}
		else {

			switch (*opt) {

#if defined(USE_RESOLVER)

			case DHCP4_OPT_DNS_SERVER:
				addr = IPV4_ADDRANY;
				dns_in4_set_addr(&addr);
				break;

			case DHCP4_OPT_HOST_NAME:
				break;

			case DHCP4_OPT_DOMAIN_NAME:
				dns_in4_set_dname("", 0);
				break;

#else	/* of #if defined(USE_RESOLVER) */

			case DHCP4_OPT_DNS_SERVER:
			case DHCP4_OPT_HOST_NAME:
			case DHCP4_OPT_DOMAIN_NAME:

#endif	/* of #if defined(USE_RESOLVER) */

			case DHCP4_OPT_SUBNET_MASK:
			case DHCP4_OPT_ROUTER:
			case DHCP4_OPT_LEASE_TIME:
			case DHCP4_OPT_RENEWAL_T1:
			case DHCP4_OPT_REBINDING_T2:
				break;

			default:
				break;
				}

			opt += *(opt + 1) + 2;
			}
		}
	}

/*
 *  rel_lease -- リースされた情報を解放する。
 */

static void
rel_lease (T_DHCP4_CLI_CONTEXT *ct)
{
	/* 有効なリース・メッセージ構造体が無ければ何もしない。*/
	if (ct->act_msg == NULL)
		return;

	/* リースされたオプションを解析して値を解放する。*/
	rel_options(ct, DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN,
	              ct->act_msg->len - (DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN), true);
	rel_addr();

	/* 有効なリース・メッセージ構造体を解放する。*/
	syscall(rel_mpf(MPF_DHCP4_CLI_MSG, (void*)ct->act_msg));
	ct->act_msg = NULL;
	}

/*
 *  set_addr -- IPv4 アドレス、サブネットマスク、静的経路表を設定する。
 */

static void
set_addr (T_DHCP4_CLI_CONTEXT *ct, uint8_t *mopt, uint8_t *ropt)
{
	ER		error;
	T_IN4_ADDR	addr, mask, router, net_addr;

	addr     = ntohl(ct->act_msg->msg.yiaddr);
	mask     = msg2host32(mopt + 2);
	net_addr = addr & mask;
	router   = msg2host32(ropt + 2);
	
	if ((error = in4_add_ifaddr(addr, mask)) != E_OK)
		syslog(LOG_NOTICE, "[DHCP4C] error, set IPv4 addr: %s", itron_strerror(error));

	if ((error = in4_add_route(0, IPV4_ADDRANY, IPV4_ADDRANY, router)) != E_OK)
		syslog(LOG_NOTICE, "[DHCP4C] error, set routing table: %s", itron_strerror(error));

	if ((error = in4_add_route(1, net_addr, mask, IPV4_ADDRANY)) != E_OK)
		syslog(LOG_NOTICE, "[DHCP4C] error, set routing table: %s", itron_strerror(error));

	if ((error = in4_add_route(2, 0xffffffff, 0xffffffff, IPV4_ADDRANY)) != E_OK)
		syslog(LOG_NOTICE, "[DHCP4C] error, set routing table: %s", itron_strerror(error));

	if (error == E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] set IPv4 addr: %s.", ip2str(NULL, &addr));

#ifdef USE_LCD
		dly_tsk(1 * 1000);
		lcd_puts(LCD_PORTID, ip2str(NULL, &addr));
		lcd_putc(LCD_PORTID, '\n');
#endif	/* of #ifdef USE_LCD */
		ether_set_up();
		}
	}

/*
 *  set_options -- リースされたオプションを解析して値を設定する。
 */

static void
set_options (T_DHCP4_CLI_CONTEXT *ct, uint_t off, uint_t len, bool_t overload)
{
#if defined(USE_RESOLVER)
	T_IN4_ADDR	addr;
#endif

	uint8_t	*opt, *mopt = NULL, *ropt = NULL;

	opt = (uint8_t*)&ct->act_msg->msg + off;
	while (opt < (uint8_t*)&ct->act_msg->msg + off + len && *opt != DHCP4_OPT_END) {
		if (*opt == DHCP4_OPT_PAD)
			opt ++;
		else if ((*opt == DHCP4_OPT_OPT_OVERLOAD) && overload) {
			if (*(opt + 2) & DHCP4_OPTOL_FILE)
				set_options(ct, DHCP4_FILE_OFFSET, DHCP4_FILE_LEN, false);
			if (*(opt + 2) & DHCP4_OPTOL_SNAME)
				set_options(ct, DHCP4_SNAME_OFFSET, DHCP4_SNAME_LEN, false);
			opt += *(opt + 1) + 2;
			}
		else {

			switch (*opt) {
			case DHCP4_OPT_SUBNET_MASK:
				mopt = opt;
				break;

			case DHCP4_OPT_ROUTER:
				ropt = opt;
				break;

#if defined(USE_RESOLVER)

			case DHCP4_OPT_DNS_SERVER:
				addr = msg2host32(opt + 2);
				dns_in4_set_addr(&addr);
				break;

			case DHCP4_OPT_HOST_NAME:
				break;

			case DHCP4_OPT_DOMAIN_NAME:
				dns_in4_set_dname(opt + 2, *(opt + 1));
				break;

#else	/* of #if defined(USE_RESOLVER) */

			case DHCP4_OPT_DNS_SERVER:
			case DHCP4_OPT_HOST_NAME:
			case DHCP4_OPT_DOMAIN_NAME:

#endif	/* of #if defined(USE_RESOLVER) */

			case DHCP4_OPT_LEASE_TIME:
			case DHCP4_OPT_RENEWAL_T1:
			case DHCP4_OPT_REBINDING_T2:
				break;

			default:
				break;
				}

			opt += *(opt + 1) + 2;
			}
		}
	if (!(mopt == NULL || ropt == NULL))
		set_addr(ct, mopt, ropt);
	}

/*
 *  set_lease -- リースされた情報を設定する。
 */

static void
set_lease (T_DHCP4_CLI_CONTEXT *ct)
{
	/* リースされたオプションを解析して値を設定する。*/
	set_options(ct, DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN,
	              ct->act_msg->len - (DHCP4_OPTION_OFFSET + DHCP4_MAGIC_LEN), true);
	}

/*
 *  validate_duplicate -- 指定された IPv4 アドレスが使われていないか検証する。
 *
 *    使われていない時は true を返す。
 */

static bool_t
validate_duplicate (T_DHCP4_CLI_MSG *msg)
{
	T_IN4_ADDR	yiaddr;

	/* 指定された IPv4 アドレスが ADDR ANY なら使われていないと判断する。*/
	yiaddr = ntohl(msg->msg.yiaddr);
	if (yiaddr == IPV4_ADDRANY)
		return true;

	/* ARP キャッシュにあれば使われていると判断する。*/
	if (arp_lookup(yiaddr, false) != NULL)
		return false;

	/*
	 *  ARP 要求を出力して TMO_DHCP4C_ARP_WAIT 待ち、
	 *  ARP キャッシュになければ使われていないと判断する。
	 */
	arp_request(&IF_ETHER_NIC_GET_SOFTC()->ifaddr, yiaddr);
	dly_tsk(TMO_DHCP4C_ARP_WAIT);
	return arp_lookup(yiaddr, false) == NULL;
	}

/*
 *  validate_rcv_msg -- 受信メッセージを検証する。
 */

static ER
validate_rcv_msg (T_DHCP4_CLI_CONTEXT *ct, T_DHCP4_CLI_MSG *msg)
{
	uint8_t 	*opt;

	/* packet type が DHCP4_REPLY であることを確認する。*/
	if (!(msg->msg.op == DHCP4_REPLY))
		return false;

	/* メッセージタイプオプションがあることを確認する。*/
	if ((opt = find_option(msg, DHCP4_OPT_MSG_TYPE, 1)) == NULL)
		return E_PAR;

	/* メッセージのタイプが一致していることを検証する。*/
	msg->type = *(opt + 2);
	switch (ct->fsm) {

	case DHCP4_FSM_SELECT:
		if (!(msg->type == DHCP4_MTYPE_OFFER))
			return E_PAR;
		break;

	case DHCP4_FSM_REQUEST:
	case DHCP4_FSM_REBOOT:
	case DHCP4_FSM_RENEW:
	case DHCP4_FSM_REBIND:
	case DHCP4_FSM_REL_INFO:
		if (!((msg->type == DHCP4_MTYPE_ACK) || (msg->type == DHCP4_MTYPE_NAK)))
			return E_PAR;
		break;

	case DHCP4_FSM_INIT:
	case DHCP4_FSM_BOUND:
	case DHCP4_FSM_INIT_REBOOT:
		return E_PAR;
		break;

	default:
		syslog(LOG_NOTICE, "[DHCP4C] error, unknown state: %d.", ct->fsm);
		return E_PAR;
		break;
		}

	/* マジック番号を検証する。*/
	if (memcmp((uint8_t*)&msg->msg.options, magicno, DHCP4_MAGIC_LEN))
		return E_PAR;

	/* トランザクション ID が異なっていればエラー */
	if (ntohl(msg->msg.xid) != ct->xid)
		return E_PAR;

	/* ハードウェアアドレス長が 6（MAC アドレス長）でなければエラー */
	if (msg->msg.hlen != ETHER_ADDR_LEN)
		return E_PAR;

	/* ハードウェアアドレスが異なっていればエラー */
	if (memcmp(&msg->msg.chaddr, &ct->sc->ifaddr.lladdr, sizeof(ct->sc->ifaddr.lladdr)))
		return E_PAR;

	/*
	 *  メッセージタイプが OFFER の場合は、
	 *  ARP を使用して、指定された IPv4 アドレスが使われていないか検証する。
	 *  使われている時は、他の IPv4 アドレスの提供を待つ。
	 */
	if (msg->type == DHCP4_MTYPE_OFFER) {
		if (!validate_duplicate(msg))
			return E_PAR;
		}

	return E_OK;
	}

/*
 *  eval_prefer -- 受信メッセージの推奨度を評価する。
 */

static void
eval_prefer (T_DHCP4_CLI_MSG *msg)
{
	int	ix;

	/* 無効の場合は終了する。*/
	if (msg == NULL)
		return;

	/* 指定された IPv4 アドレスが ADDR ANY なら推奨度を 0に設定する。*/
	if (ntohl(msg->msg.yiaddr) == IPV4_ADDRANY)
		msg->prefer = 0;
	else
		msg->prefer = DHCP4_CLI_CFG_PREFER_ADDR;

#if defined(DHCP4_CLI_CFG_REQUEST_OLIST)

	/* 要求オプションを検証する。*/
	for (ix = sizeof(request_plist); ix -- > 0; ) {
		if (find_option(msg, request_plist[ix], 0) != NULL)
			msg->prefer ++;
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUEST_OLIST) */

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST)

	/* 必須オプションを検証する。*/
	for (ix = sizeof(required_plist); ix -- > 0; ) {
		if (find_option(msg, required_plist[ix], 0) == NULL)
			msg->prefer = 0;
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) */
	}

/*
 *  eval_rcv_msg -- 受信メッセージを評価する。
 */

static ER
eval_rcv_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG	*evl, *msg;

	/* メッセージを受信していなければ中止する。*/
	syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
	if (ct->val_lst == NULL) {
		syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
		return E_OK;
		}

	/* 検証メッセージリストから評価メッセージを取り出す。*/
	evl         = ct->val_lst;
	ct->val_lst = ct->val_lst->next;
	evl->next   = NULL;
	syscall(sig_sem(SEM_DHCP4_CLI_LOCK));

	/* 評価メッセージを検証する。*/
	if (validate_rcv_msg(ct, evl) != E_OK) {

		/* エラーが発生した評価メッセージを受信メッセージ構造体リストに戻す。*/
		goto rcv_ret;
		}

	/* 受信したメッセージが NAK の時の処理 */
	if (evl->type == DHCP4_MTYPE_NAK) {

		/* リースされた情報を解放する。*/
		rel_lease(ct);

		/* 内部エラーを設定して終了する。*/
		return E_PAR;
		}

	/* REL_INFO 状態の時の処理 */
	//cons_printf(CONSOLE_PORTID, "evl_rcv_msg3[s=%d]\n", ct->fsm);
	if (ct->fsm == DHCP4_FSM_REL_INFO) {
		syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
		evl->next   = ct->prf_lst;
		ct->prf_lst = evl;
		syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
		return E_OK;
		}

	/* 評価メッセージの推奨度を評価する。*/
	eval_prefer(evl);

	/* 推奨メッセージリストが空なら評価メッセージを移して終了する。*/
	if (ct->prf_lst == NULL) {
		ct->max_prefer = evl->prefer;
		ct->prf_lst = evl;
		return E_OK;
		}

	/* 評価メッセージの推奨度と推奨メッセージリストの最大推奨度を比較する。*/
	if (evl->prefer > ct->max_prefer) {

		/* 
		 *  評価メッセージの推奨度が推奨メッセージリストの最大推奨度より高ければ、
		 *  推奨メッセージリストを受信メッセージ構造体リストに戻し、
		 *  評価メッセージを推奨メッセージリストに移す。
		 */
		ct->max_prefer = evl->prefer;
		syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
		if (ct->rcv_lst == NULL)
			ct->rcv_lst = ct->prf_lst;
		else {
			for (msg = ct->rcv_lst; msg->next != NULL; msg = msg->next)
				;
			msg->next = ct->prf_lst;
			}
		syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
		ct->prf_lst = evl;
		return E_OK;
		}

	else if (evl->prefer == ct->max_prefer) {
		for (msg = ct->prf_lst; msg != NULL; msg = msg->next) {
			if (is_equal_sid(evl, msg)) {

				/*
				 *  推奨メッセージリストに同じ DHCP4_OPT_SERVER_ID のメッセージがあれば、
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
	syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
	evl->next   = ct->rcv_lst;
	ct->rcv_lst = evl;
	syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
	return E_OK;
	}

/*
 *  select_msg -- 推奨メッセージを選択する。
 */

static T_DHCP4_CLI_MSG*
select_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG		*msg, *select;

	syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
	if (ct->prf_lst == NULL) {
		/* メッセージを受信していなければ中止する。*/
		syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
		return NULL;
		}

	/* 推奨メッセージリストの先頭のメッセージを選択する。*/
	select = ct->prf_lst;
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
	syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
	return select;
	}

/*
 *  select_ack -- ACK メッセージを検証する。
 *
 *    受け入れることが可能の時は true を返す。
 */

static bool_t
select_ack (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG	*msg;

	if ((msg = select_msg(ct)) == NULL)
		return false;

	/* 有効なリース・メッセージが残っていれば、受信メッセージ構造体リストに戻す。*/
	if (ct->act_msg != NULL) {
		syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
		ct->act_msg->next = ct->rcv_lst;
		ct->rcv_lst = ct->act_msg;
		syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
		}

	/* 有効なリース・メッセージに設定する。*/
	ct->act_msg = msg;
	return true;
	}

/*
 *  select_offer -- OFFER メッセージを検証する。
 *
 *    受け入れることが可能の時は true を返す。
 */

static bool_t
select_offer (T_DHCP4_CLI_CONTEXT *ct)
{
	return (ct->off_msg = select_msg(ct)) != NULL;
	}

/*
 *  cancel_all_timers -- 全ての TIMER をキャンセルする。
 */

static void
cancel_all_timers (T_DHCP4_CLI_CONTEXT *ct)
{
	int ix;

	DHCP4_CLI_TIMER_LOCK();
	for (ix = NUM_DHCP4C_TIMERS; ix -- > 0; )
		ct->timers[ix] = 0;
	DHCP4_CLI_TIMER_UNLOCK();
	}


/*
 *  start_select -- DISCOVER メッセージを送信して SELECT 状態に遷移する。
 */

static void
start_select (T_DHCP4_CLI_CONTEXT *ct)
{
	/* 送信を開始した時間を記録する。*/
	get_tim(&ct->snd_start);

	/* OFFER メッセージ受信タイムアウトを設定する。*/
	DHCP4_CLI_TIMER_LOCK();
	ct->timers[DHCP4C_TIMER_RCV_OFFER] = SYSTIM2TIMER(TMO_DHCP4C_OFFER);
	DHCP4_CLI_TIMER_UNLOCK();

	/* 送信先アドレスにローカルネットワークへのブロードキャストを設定する。*/
	ct->dst.ipaddr = IPV4_ADDRBROADCAST;

	/* 送信間隔をリセットする。*/
	ct->interval = 0;

	/* SELECT 状態に遷移する。*/
	ct->fsm = DHCP4_FSM_SELECT;

	/* DISCOVER メッセージを送信する。*/
	ct->flags |= DHCP4C_FLAG_TMOUT_SND_DISC;
	DHCP4_CLI_READY_SIGNAL(ct);
	}

/*
 *  start_bound -- BOUND 状態に遷移する。
 */

static void
start_bound (T_DHCP4_CLI_CONTEXT *ct)
{
	/* リースされた情報を設定する。*/
	set_lease(ct);

	/* リースの有効時間を設定する。*/
	ct->expire = timer_value(ct->act_msg, DHCP4_OPT_LEASE_TIME, TMO_DHCP4C_DEF_LEASE);

	/* リース時間が TMO_DHCP4C_MIN_LEASE 以下の時は、TMO_DHCP4C_MIN_LEASE に設定する。*/
	if (ct->expire < TMO_DHCP4C_MIN_LEASE)
		ct->expire = TMO_DHCP4C_MIN_LEASE;

	/* RENEW 状態までの時間を設定する。*/
	ct->renew = timer_value(ct->act_msg, DHCP4_OPT_RENEWAL_T1, ct->expire / 2);

	/* REBIND 状態までの時間を設定する。*/
	ct->rebind = timer_value(ct->act_msg, DHCP4_OPT_REBINDING_T2, ct->renew + ct->renew / 2 + ct->renew / 4);

	/* EXPIRE、RENEW、REBIND タイマーを設定する。*/
	DHCP4_CLI_TIMER_LOCK();
	ct->timers[DHCP4C_TIMER_EXPIRE] = SYSTIM2TIMER(ct->expire);		/* EXPIRE      までの時間 */
	ct->timers[DHCP4C_TIMER_REBIND] = SYSTIM2TIMER(ct->rebind);		/* REBIND（T2）までの時間 */
	ct->timers[DHCP4C_TIMER_RENEW]  = SYSTIM2TIMER(ct->renew);		/* RENEW （T1）までの時間 */
	DHCP4_CLI_TIMER_UNLOCK();

	/* BOUND 状態に遷移する。*/
	ct->fsm = DHCP4_FSM_BOUND;

	/* BIND を開始した時間を記録する。*/
	get_tim(&ct->bind_start);
	}

/*
 *  start_send_request -- REQUEST メッセージを送信して、指定された状態に遷移する。
 */

static void
start_send_request (T_DHCP4_CLI_CONTEXT *ct, uint_t fsm)
{
	T_IFNET	*ifp = IF_GET_IFNET();

	/* 送信を開始した時間を記録する。*/
	get_tim(&ct->snd_start);

	/* 遷移する状態が RENEW 以外では ACK/NAK メッセージ受信タイムアウトを設定する。*/
	if (fsm != DHCP4_FSM_RENEW) { 
		DHCP4_CLI_TIMER_LOCK();
		ct->timers[DHCP4C_TIMER_RCV_ACK] = SYSTIM2TIMER(TMO_DHCP4C_ACK);
		DHCP4_CLI_TIMER_UNLOCK();
		}

	/*
	 *  遷移する状態が RENEW で、
	 *  SERVER ID オプションがあればローカルネットワークへのブロードキャストを送信する。
	 */
	if ((fsm == DHCP4_FSM_RENEW) &&
	    (find_option(ct->act_msg, DHCP4_OPT_SERVER_ID, sizeof(T_IN4_ADDR)) !=  NULL))
		ct->dst.ipaddr = (ifp->in4_ifaddr.addr &  ifp->in4_ifaddr.mask) | ~ifp->in4_ifaddr.mask;
	else
		ct->dst.ipaddr = IPV4_ADDRBROADCAST;

	/* 送信間隔をリセットする。*/
	ct->interval = 0;

	/* 指定された状態に遷移する。*/
	ct->fsm = fsm;

	/* REQUEST メッセージを送信する。*/
	ct->flags |= DHCP4C_FLAG_TMOUT_SND_REQ;
	DHCP4_CLI_READY_SIGNAL(ct);
	}

/*
 *  start_rel_info -- アドレス情報を解放する。
 *
 *    注意: DHCPv4 の RFC2131 には無い
 */

static void
start_rel_info (T_DHCP4_CLI_CONTEXT *ct)
{
	/* 全ての TIMER をキャンセルする。*/
	cancel_all_timers(ct);

	/* 有効なリース情報が無ければ SLEEP に遷移する。*/
	if (ct->act_msg == NULL) {
		ct->fsm = DHCP4_FSM_SLEEP;
		return;
		}

	/* 送信先アドレスにローカルネットワークへのブロードキャストを設定する。*/
	ct->dst.ipaddr = IPV4_ADDRBROADCAST;

	/* RELEASE メッセージを送信する。*/
	ct->flags |= DHCP4C_FLAG_TMOUT_SND_REL;
	DHCP4_CLI_READY_SIGNAL(ct);
	}

/*
 *  setup_dhcp_com_msg -- DHCP メッセージの共通部分を作成する。
 */

static uint8_t *
setup_dhcp_com_msg (T_DHCP4_CLI_CONTEXT *ct, uint_t op, uint_t type)
{
	uint8_t	*option;

	memset((void*)&ct->snd_msg->msg, 0, sizeof(ct->snd_msg->msg));
	ct->snd_msg->len = sizeof(ct->snd_msg->msg);

	/* ヘッダ部を設定する。*/
	ct->snd_msg->msg.op = op;
	ct->snd_msg->msg.htype = DHCP4_HWA_TYPE_ETHER;
	ct->snd_msg->msg.hlen = ETHER_ADDR_LEN;
	HOST2MSG32(ct->snd_msg->msg.xid, ct->xid);

	/* MAC アドレスを設定する。*/
	memcpy(&ct->snd_msg->msg.chaddr, &ct->sc->ifaddr.lladdr, ETHER_ADDR_LEN);

	/*
	 *  オプション部を設定する。
	 */

	/* RFC1048 に定義されているマジック番号を設定する。*/
	memcpy(&ct->snd_msg->msg.options, magicno, DHCP4_MAGIC_LEN);
	option = ct->snd_msg->msg.options + DHCP4_MAGIC_LEN;

	/* DHCP メッセージタイプを設定する。*/
	*(option ++) = DHCP4_OPT_MSG_TYPE;
	*(option ++) = 1;
	*(option ++) = type;

	return option;
	}

/*
 *  setup_release_msg -- RELEASE メッセージを作成する。
 */

static ER
setup_release_msg (T_DHCP4_CLI_CONTEXT *ct, T_DHCP4_CLI_MSG *ref)
{
	uint8_t *opt, *ref_opt;

	/* 有効なリース・メッセージがなければエラー */
	if (ref == NULL)
		return E_PAR;

	ct->xid = create_xid();
	if ((opt = setup_dhcp_com_msg(ct, DHCP4_REQUEST, DHCP4_MTYPE_RELEASE)) == NULL)
		return E_PAR;

	/*
	 *  サーバ ID（server-ip）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	if ((ref_opt = find_option(ref, DHCP4_OPT_SERVER_ID, sizeof(T_IN4_ADDR))) == NULL)
		return E_PAR;
	*(opt ++) = *(ref_opt ++);
	*(opt ++) = *(ref_opt ++);
	memcpy(opt, ref_opt, *(ref_opt - 1));
	opt += *(ref_opt - 1);

	/* 
	 *  クライアント ID（MAC アドレス）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	*(opt ++) = DHCP4_OPT_CLIENT_ID;
	*(opt ++) = ETHER_ADDR_LEN + 1;
	*(opt ++) = DHCP4_HWA_TYPE_ETHER;
	memcpy(opt, &ct->sc->ifaddr.lladdr, ETHER_ADDR_LEN);
	opt += ETHER_ADDR_LEN;

	/* オプションの終了を設定する。*/
	*(opt ++) = DHCP4_OPT_END;

	/*
	 *  クライアント IP アドレス（ciaddr）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	HOST2MSG32(ct->snd_msg->msg.ciaddr, IF_GET_IFNET()->in4_ifaddr.addr);

	/* メッセージ長を設定する。*/
	ct->snd_msg->len = sizeof(ct->snd_msg->msg) - DHCP4_DFLT_OPT_LEN
	             + (opt - (uint8_t*)&ct->snd_msg->msg.options);
	return E_OK;
	}


/*
 *  setup_request_msg -- REQUEST メッセージを作成する。
 */

static ER
setup_request_msg (T_DHCP4_CLI_CONTEXT *ct, T_DHCP4_CLI_MSG *ref)
{
	uint8_t *opt, *ref_opt;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST)
	uint_t	len = 0;
#endif

	/* 参照するメッセージがなければエラー */
	if (ref == NULL)
		return E_PAR;

	ct->xid = create_xid();
	if ((opt = setup_dhcp_com_msg(ct, DHCP4_REQUEST, DHCP4_MTYPE_REQUEST)) == NULL)
		return E_PAR;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST)

	/* 追加のオプションを設定する。*/

	*opt  = DHCP4_OPT_REQ_PARAM;
	 opt += 2;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST)

	/* 要求オプションリストを設定する。*/
	if (sizeof(required_plist) > 0) {
		memcpy(opt, required_plist, sizeof(required_plist));
		opt += sizeof(required_plist);
		len += sizeof(required_plist);
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) */

#if defined(DHCP4_CLI_CFG_REQUEST_OLIST)

	/* 要求オプションリストを設定する。*/
	if (sizeof(request_plist) > 0) {
		memcpy(opt, request_plist, sizeof(request_plist));
		opt += sizeof(request_plist);
		len += sizeof(request_plist);
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUEST_OLIST) */

	*(opt - len - 1) = len;

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST) */

	/*
	 *  サーバ ID（server-ip）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	if (ct->fsm == DHCP4_FSM_REQUEST) {
		if ((ref_opt = find_option(ref, DHCP4_OPT_SERVER_ID, sizeof(T_IN4_ADDR))) == NULL)
			return E_PAR;
		*(opt ++) = *(ref_opt ++);
		*(opt ++) = *(ref_opt ++);
		memcpy(opt, ref_opt, *(ref_opt - 1));
		opt += *(ref_opt - 1);
		}

	/*
	 *  要求 IP アドレス（requested-ip）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 *  RFC2131 では、DHCP4_FSM_RENEW の時 MUST NOT になっているが、
	 *  MUST のようである。
	 */
	*(opt ++) = DHCP4_OPT_REQ_IPV4;
	*(opt ++) = sizeof(ref->msg.yiaddr);
	if (ct->fsm == DHCP4_FSM_REQUEST || ct->fsm == DHCP4_FSM_INIT_REBOOT ||
	    ct->fsm == DHCP4_FSM_RENEW   || ct->fsm == DHCP4_FSM_REBIND)
	 	/* 参照メッセージと同じ IP アドレス（requested-ip）を要求する。*/
		memcpy(opt, (uint8_t*)&ref->msg.yiaddr, sizeof(ref->msg.yiaddr));
	else
		host2msg32(opt, IPV4_ADDRANY);
	opt += sizeof(ref->msg.yiaddr);

	/* 
	 *  クライアント ID（MAC アドレス）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	*(opt ++) = DHCP4_OPT_CLIENT_ID;
	*(opt ++) = ETHER_ADDR_LEN + 1;
	*(opt ++) = DHCP4_HWA_TYPE_ETHER;
	memcpy(opt, &ct->sc->ifaddr.lladdr, ETHER_ADDR_LEN);
	opt += ETHER_ADDR_LEN;

	/* オプションの終了を設定する。*/
	*(opt ++) = DHCP4_OPT_END;

	/*
	 *  クライアント IP アドレス（ciaddr）を設定する。
	 *  RFC2131 4.4.1 Initialization and allocation of network address 参照
	 */
	if (ct->fsm == DHCP4_FSM_RENEW || ct->fsm == DHCP4_FSM_REBIND)
		HOST2MSG32(ct->snd_msg->msg.ciaddr, IF_GET_IFNET()->in4_ifaddr.addr);
	else
		HOST2MSG32(ct->snd_msg->msg.ciaddr, IPV4_ADDRANY);

	/* メッセージ長を設定する。*/
	ct->snd_msg->len = sizeof(ct->snd_msg->msg) - DHCP4_DFLT_OPT_LEN
	             + (opt - (uint8_t*)&ct->snd_msg->msg.options);
	return E_OK;
	}

/*
 *  setup_discover_msg -- DISCOVER メッセージを作成する。
 */

static ER
setup_discover_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	uint8_t *opt;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST)
	uint_t	len = 0;
#endif

	ct->xid = create_xid();
	if ((opt = setup_dhcp_com_msg(ct, DHCP4_REQUEST, DHCP4_MTYPE_DISCOVER)) == NULL)
		return E_PAR;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST)

	/* 追加のオプションを設定する。*/

	*opt  = DHCP4_OPT_REQ_PARAM;
	 opt += 2;

#if defined(DHCP4_CLI_CFG_REQUIRED_OLIST)

	/* 要求オプションリストを設定する。*/
	if (sizeof(required_plist) > 0) {
		memcpy(opt, required_plist, sizeof(required_plist));
		opt += sizeof(required_plist);
		len += sizeof(required_plist);
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) */

#if defined(DHCP4_CLI_CFG_REQUEST_OLIST)

	/* 要求オプションリストを設定する。*/
	if (sizeof(request_plist) > 0) {
		memcpy(opt, request_plist, sizeof(request_plist));
		opt += sizeof(request_plist);
		len += sizeof(request_plist);
		}

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUEST_OLIST) */

	*(opt - len - 1) = len;

#endif	/* of #if defined(DHCP4_CLI_CFG_REQUIRED_OLIST) || defined(DHCP4_CLI_CFG_REQUEST_OLIST) */

	/* 既にアドレス情報がリースされていれば、同じ IP アドレスを要求する。*/
	if (ct->act_msg != NULL) {
		*(opt ++) = DHCP4_OPT_REQ_IPV4;
		*(opt ++) = sizeof(ct->act_msg->msg.yiaddr);
		memcpy(opt, (uint8_t*)&ct->act_msg->msg.yiaddr, sizeof(ct->act_msg->msg.yiaddr));
		opt += sizeof(ct->act_msg->msg.yiaddr);
		}

	/* クライアント ID（MAC アドレス）を設定する。*/
	*(opt ++) = DHCP4_OPT_CLIENT_ID;
	*(opt ++) = ETHER_ADDR_LEN + 1;
	*(opt ++) = DHCP4_HWA_TYPE_ETHER;
	memcpy(opt, &ct->sc->ifaddr.lladdr, ETHER_ADDR_LEN);
	opt += ETHER_ADDR_LEN;

	/* オプションの終了を設定する。*/
	*(opt ++) = DHCP4_OPT_END;

	/* 送信先アドレスを設定する。*/
	ct->dst.ipaddr = IPV4_ADDRBROADCAST;

	/* メッセージ長を設定する。*/
	ct->snd_msg->len = sizeof(ct->snd_msg->msg) - DHCP4_DFLT_OPT_LEN
	             + (opt - (uint8_t*)&ct->snd_msg->msg.options);
	return E_OK;
	}

/*
 *  ack_msg_timeout -- ACK/NAK メッセージ受信タイムアウト処理
 */

static void
ack_msg_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	/* ACK メッセージを受信していれば。BOUND 状態に遷移する。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;
	if (select_ack(ct)) {

		/* REQUEST メッセージ送信タイマーを停止する。*/
		DHCP4_CLI_TIMER_LOCK();
		ct->timers[DHCP4C_TIMER_SND_REQ] = 0;
		DHCP4_CLI_TIMER_UNLOCK();

		/*
		 *  状態が REQUEST の時は、
		 *  OFFER メッセージを受信メッセージ構造体リストに戻す。
		 */
		if (ct->fsm == DHCP4_FSM_REQUEST && ct->off_msg != NULL) {
			syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
			ct->off_msg->next = ct->rcv_lst;
			ct->rcv_lst = ct->off_msg;
			syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
			ct->off_msg = NULL;
			}

		/* BOUND 状態に遷移する。*/
		start_bound(ct);
		return;
		}
	else {

		/* リース情報を解放する。*/
		rel_lease(ct);

		ct->error = E_TMOUT;
		}
	}

/*
 *  send_request_msg -- REQUEST メッセージを送信する。
 */

static void
send_request_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG	*ref;
	ER		error;
	SYSTIM		time;
	RELTIM		interval;
	uint16_t	secs;

	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* ACK メッセージを受信していれば。BOUND 状態に遷移する。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;
	if (select_ack(ct)) {

		/* ACK/NAK メッセージ受信タイマーを停止する。*/
		DHCP4_CLI_TIMER_LOCK();
		ct->timers[DHCP4C_TIMER_RCV_ACK] = 0;
		DHCP4_CLI_TIMER_UNLOCK();

		/*
		 *  状態が REQUEST の時は、
		 *  OFFER メッセージを受信メッセージ構造体リストに戻す。
		 */
		if (ct->fsm == DHCP4_FSM_REQUEST && ct->off_msg != NULL) {
			syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
			ct->off_msg->next = ct->rcv_lst;
			ct->rcv_lst = ct->off_msg;
			syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
			ct->off_msg = NULL;
			}

		/* BOUND 状態に遷移する。*/
		start_bound(ct);
		return;
		}

	/* REQUEST メッセージを作成する。*/
	if (ct->fsm == DHCP4_FSM_REQUEST)
		ref = ct->off_msg;
	else
		ref = ct->act_msg;
	if ((error = setup_request_msg(ct, ref)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, setup request message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 送信を開始してからの時間を得る。*/
	get_tim(&time);
	interval = time - ct->snd_start;

	/*
	 * 送信を開始してからの時間（秒）をメッセージに設定する。
	 * 最大値は 65535。
	 */
	if (ct->fsm == DHCP4_FSM_REQUEST)
		secs = ct->secs;
	else {
		if (SYSTIM2SEC(interval) < 65536)
			secs = SYSTIM2SEC(interval);
		else
			secs = 65535;
		}
	HOST2MSG16(ct->snd_msg->msg.secs, secs);

	/* ノンブロッキングコールでメッセージを送信する。*/
	error = udp_snd_dat(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, send request message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 送信間隔を設定する。*/
	if (ct->interval == 0)
		/* 未設定 */
		ct->interval = TMO_DHCP4C_INIT_INTVL;
	else
		/* バックオフアルゴリズムで 2倍にした（乱数使用）送信間隔を加算する。*/
		ct->interval += (netapp_rand() >> 2) % (ct->interval << 1);
	
	/* 送信周期の制限値を超えないようにする。*/
	if (ct->interval > TMO_DHCP4C_MAX_BACKOFF)
		ct->interval = (TMO_DHCP4C_MAX_BACKOFF >> 1) + ((netapp_rand() >> 2) % TMO_DHCP4C_MAX_BACKOFF);

	/* リース時間のタイムアウト【先送り】 */
	if (ct->fsm != DHCP4_FSM_REQUEST &&
	    TIMEC_GT(ct->interval + time, ct->expire + ct->bind_start))
		ct->interval = ct->expire + ct->bind_start - time + SYSTIM_HZ;

	/* 再送信タイムアウトを設定する。*/
	DHCP4_CLI_TIMER_LOCK();
	ct->timers[DHCP4C_TIMER_SND_REQ] = SYSTIM2TIMER(ct->interval);
	DHCP4_CLI_TIMER_UNLOCK();
	}

/*
 *  send_release_msg -- RELEASE メッセージを送信する。
 */

static void
send_release_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	ER		error;

	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* RELEASE メッセージを作成する。*/
	if ((error = setup_release_msg(ct, ct->act_msg)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, setup release message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* リース情報を解放する。*/
	rel_lease(ct);

	/* ノンブロッキングコールでメッセージを送信する。*/
	error = udp_snd_dat(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, send release message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* メッセージ送信待ちを行う。*/
	DHCP4_CLI_READY_WAIT(ct);

	/* SLEEP 状態に遷移する。*/
	ct->fsm = DHCP4_FSM_SLEEP;
	}

/*
 *  offer_msg_timeout -- OFFER メッセージ受信タイムアウト処理
 */

static void
offer_msg_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	/* OFFER メッセージを受信していれば。REQUEST 状態に遷移する。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;
	if (select_offer(ct)) {

		/* DISCOVER メッセージ送信タイマーを停止する。*/
		DHCP4_CLI_TIMER_LOCK();
		ct->timers[DHCP4C_TIMER_SND_DISC] = 0;
		DHCP4_CLI_TIMER_UNLOCK();

		/* REQUEST メッセージを送信して REQUEST 状態に遷移する。*/
		start_send_request(ct, DHCP4_FSM_REQUEST);
		}
	else
		ct->error = E_TMOUT;
	}

/*
 *  send_discover_msg -- DISCOVERメッセージを送信する。
 */

static void
send_discover_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	ER	error;
	SYSTIM	time;
	RELTIM	interval;

	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* OFFER メッセージを受信していれば。REQUEST 状態に遷移する。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;
	if (select_offer(ct)) {

		/* OFFER メッセージ受信タイマーを停止する。*/
		DHCP4_CLI_TIMER_LOCK();
		ct->timers[DHCP4C_TIMER_RCV_OFFER] = 0;
		DHCP4_CLI_TIMER_UNLOCK();

		/* REQUEST メッセージを送信して REQUEST 状態に遷移する。*/
		start_send_request(ct, DHCP4_FSM_REQUEST);
		return;
		}

	/* DISCOVER メッセージを作成する。*/
	if ((error = setup_discover_msg(ct)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, setup discover message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 送信を開始してからの時間を得る。*/
	get_tim(&time);
	interval = time - ct->snd_start;

	/*
	 * 送信を開始してからの時間（秒）をメッセージに設定する。
	 * 最大値は 65535。
	 */
	if (SYSTIM2SEC(interval) < 65536)
		ct->secs = SYSTIM2SEC(interval);
	else
		ct->secs = 65535;
	HOST2MSG16(ct->snd_msg->msg.secs, ct->secs);

	/* ノンブロッキングコールでメッセージを送信する。*/
	error = udp_snd_dat(ct->cepid, &ct->dst, &ct->snd_msg->msg, ct->snd_msg->len, TMO_NBLK);
	if (error != E_WBLK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, send discover message: %s", itron_strerror(error));
		ct->error  = error;
		return;
		}

	/* 送信間隔を設定する。*/
	if (ct->interval == 0)
		/* 未設定 */
		ct->interval = TMO_DHCP4C_INIT_INTVL;
	else
		/* バックオフアルゴリズムで 2倍にした（乱数使用）送信間隔を加算する。*/
		ct->interval += (netapp_rand() >> 2) % (ct->interval << 1);
	
	/* 送信周期の制限値を超えないようにする。*/
	if (ct->interval > TMO_DHCP4C_MAX_BACKOFF)
		ct->interval = (TMO_DHCP4C_MAX_BACKOFF >> 1) + ((netapp_rand() >> 2) % TMO_DHCP4C_MAX_BACKOFF);

	/* OFFER メッセージの受信タイムアウトを超えないように調整する。*/
	if (TIMEC_GT(time + ct->interval, ct->snd_start + TMO_DHCP4C_OFFER))
		ct->interval = (TMO_DHCP4C_INIT_INTVL + TMO_DHCP4C_OFFER) - time + 1;

	/* 再送信タイムアウトを設定する。*/
	DHCP4_CLI_TIMER_LOCK();
	ct->timers[DHCP4C_TIMER_SND_DISC] = SYSTIM2TIMER(ct->interval);
	DHCP4_CLI_TIMER_UNLOCK();
	}

#ifdef DHCP4_CLI_TASK

/*
 *  dhcpc_timer -- TIMER 管理
 */

static void
dhcpc_timer (T_DHCP4_CLI_CONTEXT *ct)
{
	int ix;

	DHCP4_CLI_TIMER_LOCK();
	for (ix = NUM_DHCP4C_TIMERS; ix -- > 0; ) {
		if (ct->timers[ix] != 0) {
			ct->timers[ix] --;
			if (ct->timers[ix] == 0) {
				ct->flags = (ct->flags & ~DHCP4C_FLAG_TMOUT_MASK) | DHCP4C_FLAG_TMOUT_TIMER | ix;
				DHCP4_CLI_READY_SIGNAL(ct);
				}
			}
		}
	DHCP4_CLI_TIMER_UNLOCK();

	/* 1秒毎にタイムアウトする。*/
	timeout((callout_func)dhcpc_timer, ct, NET_TIMER_HZ);
	}

#endif

/*
 *  expire_timeout -- EXPIRE タイムアウト処理
 */

static void
expire_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	//ER	error;

	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* 状態が REQUEST の時は何もしない。*/
	if (ct->fsm == DHCP4_FSM_REQUEST)
		return;

	/* 全ての TIMER をキャンセルする。*/
	cancel_all_timers(ct);

	/* リースされた情報を解放する。*/
	rel_lease(ct);

	ct->error = E_TMOUT;
	}

/*
 *  rebind_timeout -- REBIND タイムアウト処理
 */

static void
rebind_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* 全ての TIMER をキャンセルする。*/
	cancel_all_timers(ct);

	/* REQUEST メッセージを送信して REBIND 状態に遷移する。*/
	start_send_request(ct, DHCP4_FSM_REBIND);
	}

/*
 *  renew_timeout -- RENEW タイムアウト処理
 */

static void
renew_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	/* タイムアウトフラグをクリアーする。*/
	ct->flags &= ~DHCP4C_FLAG_TMOUT_MASK;

	/* REQUEST メッセージを送信して RENEW 状態に遷移する。*/
	start_send_request(ct, DHCP4_FSM_RENEW);
	}

/*
 *  dispatch_timeout -- タイムアウトした時の処理
 */

static ER
dispatch_timeout (T_DHCP4_CLI_CONTEXT *ct)
{
	switch (ct->flags & DHCP4C_FLAG_TMOUT_TIMER_MASK) {

	case DHCP4C_TIMER_SND_DISC:
		send_discover_msg(ct);
		break;

	case DHCP4C_TIMER_SND_REQ:
		send_request_msg(ct);
		break;

	case DHCP4C_TIMER_RCV_OFFER:
		offer_msg_timeout(ct);
		break;

	case DHCP4C_TIMER_RCV_ACK:
		ack_msg_timeout(ct);
		break;

	case DHCP4C_TIMER_RENEW:
		renew_timeout(ct);
		break;

	case DHCP4C_TIMER_REBIND:
		rebind_timeout(ct);
		break;

	case DHCP4C_TIMER_EXPIRE:
		expire_timeout(ct);
		break;

	case DHCP4C_TIMER_SND_REL:
		send_release_msg(ct);
		break;

	default:
		break;
		}

	return ct->error;
	}

#ifdef DHCP4_CLI_TASK

/*
 *  dispatch_event -- イベント毎の処理
 */

static ER
dispatch_event (T_DHCP4_CLI_CONTEXT *ct)
{
	ER	error = E_OK;

	while (ct->fsm != DHCP4_FSM_SLEEP && error == E_OK) {

		/* メッセージの受信とタイムアウトを待つ。*/
		DHCP4_CLI_READY_WAIT(ct);

		if (ct->flags & DHCP4C_FLAG_RCV_MSG) {
			while (ct->val_lst != NULL) {
				error = eval_rcv_msg(ct);
				}
			ct->flags &= ~DHCP4C_FLAG_RCV_MSG;
			}
		if (ct->flags & DHCP4C_FLAG_TMOUT_MASK)
			error = dispatch_timeout(ct);

		if (ct->error != E_OK)
			error = ct->error;
		}

	return error;
	}

#endif

/*
 *  init_context -- DHCP クライアントコンテキスト構造体を初期化する。
 */

static void
init_context (T_DHCP4_CLI_CONTEXT *ct, ID tskid, ID cepid)
{
  	memset(ct, 0, sizeof(*ct));
	ct->tskid	= tskid;
	ct->cepid	= cepid;
	ct->sc		= IF_ETHER_NIC_GET_SOFTC();
	ct->dst.portno	= DHCP4_SRV_CFG_PORTNO;
	ct->dst.ipaddr	= IPV4_ADDRANY;
	}

/*
 *  rel_cli_msg -- メッセージ構造体を解放する。
 */

static void
rel_cli_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG	*ptr, *cell;

	for (ptr = ct->rcv_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP4_CLI_MSG, (void*)cell));
		}

	for (ptr = ct->val_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP4_CLI_MSG, (void*)cell));
		}

	for (ptr = ct->prf_lst; ptr != NULL; ) {
		cell = ptr;
		ptr = ptr->next;
		syscall(rel_mpf(MPF_DHCP4_CLI_MSG, (void*)cell));
		}

	if (ct->snd_msg != NULL)
		syscall(rel_mpf(MPF_DHCP4_CLI_MSG, (void*)ct->snd_msg));
	ct->rcv_lst = ct->prf_lst = ct->val_lst = ct->snd_msg = NULL;
	}

/*
 *  init_cli_msg -- 各メッセージ構造体を初期化する。
 */

static ER
init_cli_msg (T_DHCP4_CLI_CONTEXT *ct)
{
	T_DHCP4_CLI_MSG	*cell;
	ER		error;
	int		count;

	/* 送信メッセージ構造体を獲得する。*/
	if ((error = tget_mpf(MPF_DHCP4_CLI_MSG, (void*)&ct->snd_msg, TMO_DHCP4C_MPF_GET)) != E_OK) {
		syslog(LOG_NOTICE, "[DHCP4C] error, tget_mpf() for send: %s.", itron_strerror(error));
		return error;
		}

	/*
	 *  有効なリース・メッセージ構造体がある場合は、
	 *  獲得する受信メッセージ構造体リストの構造体を 1減らす。
	 */
	if (ct->act_msg == NULL)
		count = NUM_DHCP4_MSG_LIST;
	else
		count = NUM_DHCP4_MSG_LIST - 1;

	/* 受信メッセージ構造体を獲得する。*/
	while (count --> 0) {
		if ((error = tget_mpf(MPF_DHCP4_CLI_MSG, (void*)&cell, TMO_DHCP4C_MPF_GET)) != E_OK) {
			syslog(LOG_NOTICE, "[DHCP4C] error, tget_mpf() for receive: %s.", itron_strerror(error));
			break;
			}
		cell->next  = ct->rcv_lst;
		ct->rcv_lst = cell;
		}

	return E_OK;
	}

/*
 *  dhcp4c_renew_info -- ネットワーク情報を再取得する。
 */

ER
dhcp4c_renew_info (void)
{
	if (context.fsm == DHCP4_FSM_BOUND) {

		/* REL_INFO 状態に遷移した後に SLEEP を解除するように設定する。*/
		context.flags |= DHCP4C_FLAG_RENEW;
		start_rel_info(&context);
		return E_OK;
		}
	else if (context.fsm == DHCP4_FSM_SLEEP) {

		/* SLEEP を解除する。*/
		context.fsm = DHCP4_FSM_WAKE;
		context.timer = 0;
		wup_tsk(context.tskid);
		return E_OK;
		}
	else
		return E_OBJ;
	}

/*
 *  dhcp4c_rel_info -- DHCP で設定されたネットワーク情報を解放する。
 */

ER
dhcp4c_rel_info (void)
{
	/* REL_INFO 状態に遷移する。*/
	start_rel_info(&context);
	return E_OK;
	}

/*
 *  dhcp4c_get_info -- DHCP で設定されたネットワーク情報を返す。
 */

ER
dhcp4c_get_info (T_IN4_ADDR *addr, uint32_t *expire, uint32_t *renew,
                                   uint32_t *rebind, SYSTIM *bind_start)
{
	if (addr  == NULL || expire == NULL || 
	    renew == NULL || rebind == NULL || bind_start == NULL)
		return E_PAR;
	else if (context.act_msg == NULL)
		return E_OBJ;
	else {
		*addr		= context.act_msg->srv.ipaddr;
		*expire		= context.timers[DHCP4C_TIMER_EXPIRE];
		*renew		= context.timers[DHCP4C_TIMER_RENEW];
		*rebind		= context.timers[DHCP4C_TIMER_REBIND];
		*bind_start	= context.bind_start;
		return E_OK;
		}
	}

/*
 *  コールバック関数
 */

ER
callback_nblk_dhcp4_cli (ID cepid, FN fncd, void *p_parblk)
{
	T_DHCP4_CLI_MSG	*msg;
	ER_UINT		len;

	len = *(ER_UINT*)p_parblk;
	if (len < 0 && len != E_RLWAI) {
		/* E_RLWAI 以外で、0 以下の場合は、エラーを意味している。*/
		syslog(LOG_NOTICE, "[DHCPC(CBR)] error: %s, fncd: %s", itron_strerror(len), in_strtfn(fncd));
		}
	else {
		if (fncd == TEV_UDP_RCV_DAT) {

			/* 受信メッセージリストが空の時は、受信しないでデータグラムを捨てる。*/
			if (context.rcv_lst == NULL) 
				syslog(LOG_NOTICE, "[DHCP4C(CBR)] receive buffer all busy.");
			else {
				/* メッセージを受信するメッセージ構造体を準備する。*/
				syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
				msg = context.rcv_lst;
				context.rcv_lst = context.rcv_lst->next;
				syscall(sig_sem(SEM_DHCP4_CLI_LOCK));

				/* メッセージを受信する。*/
				len = udp_rcv_dat(context.cepid, &msg->srv, &msg->msg, sizeof(msg->msg), TMO_POL);

				syscall(wai_sem(SEM_DHCP4_CLI_LOCK));
				msg->len = len;
				if (len >= 0) {
					/* 受信したメッセージを検証メッセージリストに移す。*/
					msg->next = context.val_lst;
					context.val_lst = msg;
					context.flags |= DHCP4C_FLAG_RCV_MSG;
					}
				else {
					/* メッセージ構造体を受信メッセージリストに戻す。*/
					context.error = len;
					msg->next = context.rcv_lst;
					context.rcv_lst = msg;
					}
				syscall(sig_sem(SEM_DHCP4_CLI_LOCK));
				}
			}
		DHCP4_CLI_READY_SIGNAL((&context));
		}
	return E_OK;
	}

#ifdef DHCP4_CLI_TASK

/*
 *  dhcp4_cli_task -- DHCP クライアントタスク
 */

void
dhcp4_cli_task (intptr_t exinf)
{
	T_DHCP4_CLI_CONTEXT	*ct;
	ID			tskid;
	ER			error = E_OK;

	dly_tsk(1000);
	get_tid(&tskid);
	syslog(LOG_NOTICE, "[DHCP4C:%d,%d] started.", tskid, (ID)exinf);
	dly_tsk(500);
	ct = &context;

	/* DHCP クライアントコンテキスト構造体を初期化する。*/
	init_context(ct, tskid, (ID)exinf);
	ct->fsm = DHCP4_FSM_SLEEP;

	while (true) {

		/* リースを解放後、再取得が指定されていなければ休止する。*/
		if (!(ct->flags & DHCP4C_FLAG_RENEW)) {

			/* 休止する。*/
			if (error == E_OK) {
				if (ct->fsm != DHCP4_FSM_SLEEP)
					syslog(LOG_NOTICE, "[DHCP4C] lease released, go to sleep.");
			}
			else {
				syslog(LOG_NOTICE, "[DHCP4C] server not available, go to sleep, error: %s.", itron_strerror(error));
				ct->fsm = DHCP4_FSM_SLEEP;
				}
			slp_tsk();
			}
		ct->flags = 0;
		ct->error = E_OK;

		/* 1秒毎にタイムアウトするようにタイマーを設定する。*/
		timeout((callout_func)dhcpc_timer, ct, NET_TIMER_HZ / DHCP4C_TIMER_HZ);

		/* メッセージ構造体を初期化する。*/
		if ((error = init_cli_msg(ct)) == E_OK) {

			/* SELECT 状態に遷移する。*/
			start_select(ct);

			/* メインループ */
			error = dispatch_event(ct);
			}

		/* 全ての TIMER をキャンセルする。*/
		cancel_all_timers(ct);

		/* タイマーを停止する。*/
		untimeout((callout_func)dhcpc_timer, ct);

		/* メッセージ構造体を解放する。*/
		rel_cli_msg (ct);
		}
	}

#else

T_DHCP4_CLI_CONTEXT *
dhcp4_cli_initialize(ID tskid, ID cepid)
{
	T_DHCP4_CLI_CONTEXT *ct = &context;
	/* DHCP クライアントコンテキスト構造体を初期化する。*/
	init_context(ct, tskid, cepid);
	ct->fsm = DHCP4_FSM_SLEEP;
	ct->timer = TMO_FEVR;
	return ct;
	}

int
dhcp4_cli_get_timer(T_DHCP4_CLI_CONTEXT *ct)
{
	if (ct->fsm == DHCP4_FSM_SLEEP)
		return TMO_FEVR;

	return ct->timer;
	}

void
dhcp4_cli_progress(T_DHCP4_CLI_CONTEXT *ct, int elapse)
{
	int ix;

	if ((ct->fsm == DHCP4_FSM_SLEEP) || (ct->timer == TMO_FEVR))
		return;

	ct->timer -= elapse;
	if (ct->timer > 0)
		return;

	/* 1秒毎にタイムアウトするようにタイマーを設定する。*/
	ct->timer = 1000 * 1000;

	for (ix = NUM_DHCP4C_TIMERS; ix-- > 0; ) {
		if (ct->timers[ix] != 0) {
			ct->timers[ix] --;
			if (ct->timers[ix] <= 0) {
				ct->timers[ix] = 0;
				ct->flags = (ct->flags & ~DHCP4C_FLAG_TMOUT_MASK) | DHCP4C_FLAG_TMOUT_TIMER | ix;
				DHCP4_CLI_READY_SIGNAL(ct);
				}
			}
		}
	}

void
dhcp4_cli_wakeup(T_DHCP4_CLI_CONTEXT *ct)
{
	if (ct->fsm == DHCP4_FSM_WAKE) {
		ct->flags = 0;
		ct->error = E_OK;

		/* メッセージ構造体を初期化する。*/
		if ((ct->error = init_cli_msg(ct)) != E_OK)
			return;

		ct->timer = 1000 * 1000;

		/* SELECT 状態に遷移する。*/
		start_select(ct);
		}

	if (ct->flags & DHCP4C_FLAG_RCV_MSG) {
		while (ct->val_lst != NULL) {
			ct->error = eval_rcv_msg(ct);
			}
		ct->flags &= ~DHCP4C_FLAG_RCV_MSG;
		}
	}

void
dhcp4_cli_timeout(T_DHCP4_CLI_CONTEXT *ct)
{
	if (ct->flags & DHCP4C_FLAG_TMOUT_MASK)
		ct->error = dispatch_timeout(ct);

	if (ct->error != E_OK || ct->fsm == DHCP4_FSM_SLEEP) {
		/* 全ての TIMER をキャンセルする。*/
		cancel_all_timers(ct);
		ct->timer = TMO_FEVR;

		/* メッセージ構造体を解放する。*/
		rel_cli_msg(ct);

		/* リースを解放後、再取得が指定されていなければ休止する。*/
		if (!(ct->flags & DHCP4C_FLAG_RENEW)) {

			/* 休止する。*/
			if (ct->error == E_OK) {
				if (ct->fsm != DHCP4_FSM_SLEEP)
					syslog(LOG_NOTICE, "[DHCP4C] lease released, go to sleep.");
			}
			else {
				syslog(LOG_NOTICE, "[DHCP4C] server not available, go to sleep, error: %s.", itron_strerror(ct->error));
				ct->error = E_OK;
				ct->fsm = DHCP4_FSM_SLEEP;
				}
			}
		}
	}

#endif /* DHCP4_CLI_TASK */
#endif	/* of #ifdef DHCP4_CLI_CFG */
