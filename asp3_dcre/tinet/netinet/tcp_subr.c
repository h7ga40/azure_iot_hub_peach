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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tcp_subr.c	8.2 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_subr.c,v 1.49.2.4 1999/08/29 16:29:55 peter Exp $
 */

#include <stdarg.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
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
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>

#ifdef SUPPORT_TCP

/*
 *  全域変数
 */

/*
 *  TCP の SEQ の初期値、プログラムが起動して、最初に TCP セグメントを
 *  送受信するときに、get_tim() により初期化する。
 */
T_TCP_SEQ tcp_iss = 0;

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

T_TCP_STATS tcp_stats;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  局所変数
 */

static uint16_t tcp_port_auto = TCP_PORT_FIRST_AUTO;	/* 自動割り当て番号	*/

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

/*
 *  タスクからの Time Wait 状態 CEP 分離機能
 */

/*
 *  変数
 */

T_TCP_TWCEP tcp_twcep[NUM_TCP_TW_CEP_ENTRY];

/*
 *  tcp_move_twcep -- 必要な情報を Time Wait 用 TCP 通信端点に移して、
 *                    標準の TCP 通信端点を開放する。
 */

void
tcp_move_twcep (T_TCP_CEP *cep)
{
	T_TCP_TWCEP*	twcep;

	/* 空きの Time Wait 用 TCP 通信端点を探索する。*/
	for (twcep = &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep -- != tcp_twcep; ) {
		if (twcep->fsm_state != TCP_FSM_TIME_WAIT) {

			/*
			 *  通信端点をロックし、
			 *  必要な情報を Time Wait 用 TCP 通信端点に移す。
			 */
			syscall(wai_sem(cep->semid_lock));
			twcep->flags		= (uint8_t)cep->flags;
			twcep->rbufsz		= cep->rbufsz;
			twcep->dstaddr		= cep->dstaddr;
			twcep->myaddr		= cep->myaddr;
			twcep->snd_una		= cep->snd_una;
			twcep->rcv_nxt		= cep->rcv_nxt;
			twcep->rwbuf_count	= cep->rwbuf_count;
			twcep->fsm_state	= cep->fsm_state;
			twcep->timer_2msl	= cep->timer[TCP_TIM_2MSL];

			/* 通信端点をロックを解除する。*/
			syscall(sig_sem(cep->semid_lock));

			/* 標準 TCP 通信端点を開放する。*/
			tcp_close(cep);

			break;
			}
		}
	}

/*
 *  tcp_find_twcep -- ポート番号から Time Wait 用 TCP 通信端点を得る。
 */

T_TCP_TWCEP*
tcp_find_twcep (T_NET_BUF *input, uint_t off)
{
	T_TCP_TWCEP*	twcep;
	T_TCP_HDR	*tcph;

	tcph = GET_TCP_HDR(input, off);
	
	/*
	 *  状態が TIME WAIT で、
	 *  IP アドレスとポート番号が一致する通信端点を探索する。
	 */
	for (twcep = &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep -- != tcp_twcep; ) {
		if (twcep->fsm_state == TCP_FSM_TIME_WAIT                   &&
		    IN_IS_DSTADDR_ACCEPT    (&twcep->myaddr.ipaddr,  input) &&
		    IN_ARE_NET_SRCADDR_EQUAL(&twcep->dstaddr.ipaddr, input) &&
		    tcph->dport == twcep->myaddr.portno                     &&
		    tcph->sport == twcep->dstaddr.portno)
			return twcep;
		}

	return NULL;
	}

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

/*
 *  tcp_find_cep -- ポート番号から TCP 通信端点を得る。
 */

T_TCP_CEP*
tcp_find_cep (T_NET_BUF *input, uint_t off)
{
	T_TCP_CEP*	cep;
	T_TCP_HDR	*tcph;

	tcph = GET_TCP_HDR(input, off);
	
	/*
	 *  状態が SYN 送信済み以後は、
	 *  IP アドレスとポート番号が一致する TCP 通信端点を探索する。
	 */
	for (cep = &tcp_cep[tmax_tcp_cepid]; cep -- != tcp_cep; ) {
		if (cep->fsm_state >= TCP_FSM_SYN_SENT                    &&
		    IN_IS_DSTADDR_ACCEPT    (&cep->myaddr.ipaddr,  input) &&
		    IN_ARE_NET_SRCADDR_EQUAL(&cep->dstaddr.ipaddr, input) &&
		    tcph->dport == cep->myaddr.portno                     &&
		    tcph->sport == cep->dstaddr.portno)
			return cep;
		}

	/* IPv4 で受動オープン中の TCP 通信端点を先に探索する。*/	
	for (cep = &tcp_cep[tmax_tcp_cepid]; cep -- != tcp_cep; ) {
		if ((cep->flags & TCP_CEP_FLG_IPV4)                   &&
		     cep->fsm_state == TCP_FSM_LISTEN                 &&
		     GET_IP_VER(input) == IPV4_VERSION                &&
		     IN_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
		     tcph->dport == cep->myaddr.portno)
			return cep;
		}

	/* 受動オープン中の TCP 通信端点を探索する。*/	
	for (cep = &tcp_cep[tmax_tcp_cepid]; cep -- != tcp_cep; ) {

#if defined(_IP6_CFG) && defined(_IP4_CFG)

		if (cep->flags & TCP_CEP_FLG_IPV4) {
			if (cep->fsm_state == TCP_FSM_LISTEN                 &&
		            GET_IP_VER(input) == IPV4_VERSION                &&
			    IN_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
			    tcph->dport == cep->myaddr.portno)
				return cep;
			}
		else {

#if defined(API_CFG_IP4MAPPED_ADDR)

			if (cep->fsm_state == TCP_FSM_LISTEN                 &&
			    IN_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
			    tcph->dport == cep->myaddr.portno)
				return cep;

#else	/* of #if defined(API_CFG_IP4MAPPED_ADDR) */

			if (cep->fsm_state == TCP_FSM_LISTEN                   &&
			    INN6_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
			    tcph->dport == cep->myaddr.portno)
				return cep;

#endif	/* of #if defined(API_CFG_IP4MAPPED_ADDR) */

			}

#else	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */

		if (cep->fsm_state == TCP_FSM_LISTEN                 &&
		    IN_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
		    tcph->dport == cep->myaddr.portno)
			return cep;

#endif	/* of #if defined(_IP6_CFG) && defined(_IP4_CFG) */
		}

	return NULL;
	}

/*
 *  tcp_is_addr_accept -- 受信可能な IP アドレスとポート番号であることを確認する。
 */

bool_t
tcp_is_addr_accept (T_NET_BUF *input, uint_t off)
{
	T_TCP_HDR	*tcph;

	tcph = GET_TCP_HDR(input, off);

#if !defined(_IP6_CFG) && defined(_IP4_CFG) && defined(SUPPORT_LOOP)

	/*
	 *  次のときは破棄する。
	 *    ・ポート番号が同一で、送受信 IP アドレス が同一。
	 *      ただし、送信元 IP アドレスがローカルループバックなら良い。
	 *    ・マルチキャストアドレス
	 */

	if (tcph->dport == tcph->sport && 
	    (IN4_ARE_HDR_ADDR_EQUAL(input) && !IN4_ARE_NET_ADDR_EQUAL(&GET_IP4_HDR(input)->dst, &IPV4_ADDR_LOOPBACK)))
		return RET_DROP;

#else	/* of #if !defined(_IP6_CFG) && defined(_IP4_CFG) && defined(SUPPORT_LOOP) */

	/*
	 *  次のときは、受信可能ではない。
	 *    ・ポート番号が同一で、送受信 IP アドレス が同一。
	 *    ・マルチキャストアドレス
	 */
	if (tcph->dport == tcph->sport && IN_ARE_HDR_ADDR_EQUAL(input))
		return false;

#endif	/* of #if !defined(_IP6_CFG) && defined(_IP4_CFG) && defined(SUPPORT_LOOP) */

	if (IN_IS_NET_ADDR_MULTICAST(input))
		return false;
	else
		return true;
	}

/*
 *  tcp_free_reassq -- 受信再構成キューのネットワークバッファを解放する。
 *
 *    注意:
 *      必要であれば、この関数を呼び出す前に、通信端点をロックし、
 *      戻った後、解除する必要がある。
 */

void
tcp_free_reassq (T_TCP_CEP *cep)
{
	T_NET_BUF	*q, *nq;

	for (q = cep->reassq; q != NULL; q = nq) {
		nq = GET_TCP_Q_HDR(q, GET_IP_TCP_Q_HDR_OFFSET(q))->next;
		syscall(rel_net_buf(q));
		}
	cep->reassq  = NULL;
	NET_COUNT_TCP(net_count_tcp[NC_TCP_FREE_RCV_QUEUE],   1);
	}

/*
 *  tcp_alloc_auto_port -- 自動割り当てポート番号を設定する。
 */

void
tcp_alloc_auto_port (T_TCP_CEP *cep)
{
	int_t		ix;
	uint16_t	portno;

	while (true) {
		portno = tcp_port_auto ++;
		if (tcp_port_auto > TCP_PORT_LAST_AUTO)
			tcp_port_auto = TCP_PORT_FIRST_AUTO;

#if defined(TNUM_TCP6_REPID)
#if TNUM_TCP6_REPID > 0

		for (ix = tmax_tcp6_repid; ix -- > 0; ) {

#ifdef TCP_CFG_EXTENTIONS

			if (VALID_TCP_REP(&tcp6_rep[ix]) && tcp6_rep[ix].myaddr.portno == portno) {
				portno = TCP_PORTANY;
				break;
				}

#else	/* of #ifdef TCP_CFG_EXTENTIONS */

			if (tcp6_rep[ix].myaddr.portno == portno) {
				portno = TCP_PORTANY;
				break;
				}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

			}

#endif	/* of #if TNUM_TCP6_REPID > 0 */
#endif	/* of #if defined(TNUM_TCP6_REPID) */

#if defined(TNUM_TCP4_REPID)
#if TNUM_TCP4_REPID > 0

		for (ix = tmax_tcp4_repid; ix -- > 0; ) {

#ifdef TCP_CFG_EXTENTIONS

			if (VALID_TCP_REP(&tcp4_rep[ix]) && tcp4_rep[ix].myaddr.portno == portno) {
				portno = TCP_PORTANY;
				break;
				}

#else	/* of #ifdef TCP_CFG_EXTENTIONS */

			if (tcp4_rep[ix].myaddr.portno == portno) {
				portno = TCP_PORTANY;
				break;
				}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

			}

#endif	/* of #if TNUM_TCP4_REPID > 0 */
#endif	/* of #if defined(TNUM_TCP4_REPID) */

		if (portno != TCP_PORTANY) {

			syscall(wai_sem(SEM_TCP_CEP));
			for (ix = tmax_tcp_cepid; ix -- > 0; ) {
				if (VALID_TCP_CEP(&tcp_cep[ix]) && tcp_cep[ix].myaddr.portno == portno) {
					portno = TCP_PORTANY;
					break;
					}
				}

			if (portno != TCP_PORTANY) {
				cep->myaddr.portno = portno;
				syscall(sig_sem(SEM_TCP_CEP));
				return;
				}
			syscall(sig_sem(SEM_TCP_CEP));
			}
		}
	}

/*
 *  tcp_alloc_port -- 指定されたポート番号を設定する。
 */

ER
tcp_alloc_port (T_TCP_CEP *cep, uint16_t portno)
{
	int_t	ix;

	syscall(wai_sem(SEM_TCP_CEP));
	for (ix = tmax_tcp_cepid; ix -- > 0; )
		if (VALID_TCP_CEP(cep) && tcp_cep[ix].myaddr.portno == portno) {
			syscall(sig_sem(SEM_TCP_CEP));
			return E_PAR;
			}
	cep->myaddr.portno = portno;
	syscall(sig_sem(SEM_TCP_CEP));
	return E_OK;
	}

/*
 *  tcp_init -- TCP モジュールの初期化
 */

void
tcp_init (void)
{
	timeout(tcp_slow_timo, NULL, TCP_SLOW_TMO);

#ifdef TCP_CFG_DELAY_ACK

	timeout(tcp_fast_timo, NULL, TCP_FAST_TMO);

#endif/* of #ifdef TCP_CFG_DELAY_ACK */
	}

/*
 * tcp_init_iss シーケンス番号を初期化する。
 */

void
tcp_init_iss (void)
{
	SYSTIM now;

	syscall(get_tim(&now));
	net_srand(now);
	tcp_iss = net_rand();
	}

/*
 *  tcp_close -- コネクションを開放する。
 */

T_TCP_CEP *
tcp_close (T_TCP_CEP *cep)
{
	/* タイマーを停止する。*/
	tcp_cancel_timers(cep);

	/* 
	 *  通信端点をロックし、
	 *  受信再構成キューのネットワークバッファを解放する。
	 */
	syscall(wai_sem(cep->semid_lock));
	tcp_free_reassq(cep);
	syscall(sig_sem(cep->semid_lock));

	/* 状態を未使用にする。*/
	cep->fsm_state = TCP_FSM_CLOSED;

	/*
	 * 以下に関係しないフラグをクリアーする。
	 * ・送受信ウィンドバッファの省コピー機能
	 * ・動的な通信端点の生成・削除機能
	 * ・通信端点のネットワーク層プロトコル
	 */
	cep->flags &= TCP_CEP_FLG_NOT_CLEAR;

#ifdef TCP_CFG_NON_BLOCKING

	if (cep->snd_nblk_tfn != TFN_TCP_UNDEF || cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {
		/* ノンブロッキングコール */

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

		if (!IS_PTR_DEFINED(cep->callback))
			syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
		else {
			if (cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {
				ER_UINT len;

				switch (cep->rcv_nblk_tfn) {

				case TFN_TCP_ACP_CEP:
					/* TCP 通信端点からTCP 受付口を解放する。*/
					cep->rep = NULL;

#if defined(_IP6_CFG) && defined(_IP4_CFG)
					cep->rep4 = NULL;
#endif

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)E_CLS);
					break;

				case TFN_TCP_RCV_BUF:

					/* 受信ウィンドバッファの空アドレスを獲得する。*/
					len = TCP_GET_RWBUF_ADDR(cep, cep->rcv_p_buf);

					/* 異常切断等のエラーを設定する。*/
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else if (cep->error < 0)
						len = cep->error;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)(uint32_t)len);
					break;

				case TFN_TCP_RCV_DAT:

					/* 受信ウィンドバッファからデータを取り出す。*/
					len = TCP_READ_RWBUF(cep, cep->rcv_data, (uint_t)cep->rcv_len);

					/* 異常切断等のエラーを設定する。*/
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else if (cep->error != E_OK)
						len = cep->error;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)(uint32_t)len);
					break;

				case TFN_TCP_CLS_CEP:

					if (cep->error == E_TMOUT)
						(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)E_CLS);
					else
						(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)cep->error);
					break;

				default:
					syslog(LOG_WARNING, "[TCP] unexp TFN: %s.", in_strtfn((FN)cep->rcv_nblk_tfn));
					break;
					}

				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->rcv_tskid = TA_NULL;
				cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
				}

			if (cep->snd_nblk_tfn != TFN_TCP_UNDEF) {

				switch (cep->snd_nblk_tfn) {

				case TFN_TCP_CON_CEP:
					/* TCP 通信端点から TCP 受付口を解放する。*/
					cep->rep = NULL;

#if defined(_IP6_CFG) && defined(_IP4_CFG)
					cep->rep4 = NULL;
#endif

					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)E_CLS);
					break;

				case TFN_TCP_SND_DAT:
				case TFN_TCP_GET_BUF:
					if (cep->error == E_TMOUT)
						(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)E_CLS);
					else
						(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)cep->error);
					break;

				default:
					syslog(LOG_WARNING, "[TCP] unexp TFN: %s.", in_strtfn((FN)cep->snd_nblk_tfn));
					break;
					}

				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->snd_tskid = TA_NULL;
				cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
				}

			}

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

		if (!IS_PTR_DEFINED(cep->callback))
			syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
		else {
			if (cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {
				ER_UINT len;

				switch (cep->rcv_nblk_tfn) {

				case TFN_TCP_ACP_CEP:

					/* TCP 通信端点からTCP 受付口を解放する。*/
					cep->rep = NULL;

#if defined(_IP6_CFG) && defined(_IP4_CFG)
					cep->rep4 = NULL;
#endif

					/* 接続エラーを設定する。*/
					len      = E_CLS;
					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
					break;

				case TFN_TCP_RCV_BUF:

					/* 受信ウィンドバッファの空アドレスを獲得する。*/
					len = TCP_GET_RWBUF_ADDR(cep, cep->rcv_p_buf);

					/* 異常切断等のエラーを設定する。*/
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else if (cep->error < 0)
						len = cep->error;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
					break;

				case TFN_TCP_RCV_DAT:

					/* 受信ウィンドバッファからデータを取り出す。*/
					len = TCP_READ_RWBUF(cep, cep->rcv_data, (uint_t)cep->rcv_len);

					/* 異常切断等のエラーを設定する。*/
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else if (cep->error != E_OK)
						len = cep->error;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
					break;

				case TFN_TCP_CLS_CEP:
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else
						len = cep->error;
					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
					break;

				default:
					syslog(LOG_WARNING, "[TCP] unexp TFN: %s.", in_strtfn((FN)cep->rcv_nblk_tfn));
					break;
					}

				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->rcv_tskid = TA_NULL;
				cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
				}

			if (cep->snd_nblk_tfn != TFN_TCP_UNDEF) {
				ER_UINT len;

				switch (cep->snd_nblk_tfn) {

				case TFN_TCP_CON_CEP:

					/* TCP 通信端点からTCP 受付口を解放する。*/
					cep->rep = NULL;

#if defined(_IP6_CFG) && defined(_IP4_CFG)
					cep->rep4 = NULL;
#endif

					/* 接続エラーを設定する。*/
					len      = E_CLS;
					(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&len);
					break;

				case TFN_TCP_SND_DAT:
				case TFN_TCP_GET_BUF:
					if (cep->error == E_TMOUT)
						len = E_CLS;
					else
						len = cep->error;
					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
					break;

				default:
					syslog(LOG_WARNING, "[TCP] unexp TFN: %s.", in_strtfn((FN)cep->snd_nblk_tfn));
					break;
					}

				/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
				cep->snd_tskid = TA_NULL;
				cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
				}

			}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

		/* 
		 *  通信端点をロックし、
		 *  送受信ウィンドバッファキューのネットワークバッファを解放する。
		 */
		syscall(wai_sem(cep->semid_lock));
		TCP_FREE_RWBUFQ(cep);
		TCP_FREE_SWBUFQ(cep);
		syscall(sig_sem(cep->semid_lock));

		/* 未使用になったことを知らせる。*/
		syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
		}
	else {	/* 非ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
		cep->snd_tskid = cep->rcv_tskid = TA_NULL;
		cep->snd_tfn   = cep->rcv_tfn   = TFN_TCP_UNDEF;

		/* 
		 *  通信端点をロックし、
		 *  送受信ウィンドバッファキューのネットワークバッファを解放する。
		 */
		syscall(wai_sem(cep->semid_lock));
		TCP_FREE_RWBUFQ(cep);
		TCP_FREE_SWBUFQ(cep);
		syscall(sig_sem(cep->semid_lock));

		/* 未使用になったことを知らせる。*/
		syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));

		/*
		 * 入出力タスクを起床して、
		 * 送受信不可になったことを知らせる。
		 */
		syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));
		syscall(set_flg(cep->rcv_flgid, TCP_CEP_EVT_RWBUF_READY));

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	return NULL;
	}

/*
 *  tcp_drop -- TCP 接続を破棄する。
 */

T_TCP_CEP *
tcp_drop (T_TCP_CEP *cep, ER err)
{
#ifdef SUPPORT_MIB

	if ((cep->fsm_state == TCP_FSM_SYN_SENT) || (cep->fsm_state == TCP_FSM_SYN_RECVD)) {
		NET_COUNT_MIB(tcp_stats.tcpAttemptFails, 1);
		}

#endif	/* of #ifdef SUPPORT_MIB */

	cep->error = err;
	if (TCP_FSM_HAVE_RCVD_SYN(cep->fsm_state)) {
		cep->fsm_state = TCP_FSM_CLOSED;

		/* 送信と、送信後コネクションの切断を指示する。*/
		cep->flags |=  TCP_CEP_FLG_POST_OUTPUT | TCP_CEP_FLG_CLOSE_AFTER_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	else {
		cep = tcp_close(cep);
		}
	return cep;
	}

/*
 *  tcp_respond -- TCP の単一メッセージを送信する。
 */

void
tcp_respond (T_NET_BUF *output, T_TCP_CEP *cep,
             T_TCP_SEQ ack, T_TCP_SEQ seq, uint_t rbfree, uint8_t flags)
{
	T_TCP_HDR	*tcph;
	uint_t		win = 0;
	uint_t		hdr_offset;

	if ((flags & TCP_FLG_RST) == 0)
		win = rbfree;

	/*
	 *  output が NULL でなければ、これは入力したセグメントの
	 *  net_buf で、そのまま再利用する。
	 */
	if (output != NULL) {
		uint16_t	portno;

		/*
		 * IPv4 では、IP ヘッダのオプションを削除する。
		 * IPv6 では、拡張ヘッダを削除する。
		 */
		if (IP_REMOVE_OPTIONS(output) != E_OK) {
			syscall(rel_net_buf(output));
			return;
			}

		ip_exchg_addr(output);

#if defined(_IP6_CFG)

		/* トラヒッククラスとフローラベルをクリアする。*/
		SET_IP_CF(output, 0);

#endif	/* of #if defined(_IP6_CFG) */

		/* TCP SDU 長を 0 にする。*/
		SET_IP_SDU_SIZE(output, TCP_HDR_SIZE);

		tcph = GET_TCP_HDR(output, IF_IP_TCP_HDR_OFFSET(output));

		/* ポート番号を交換する。*/
		portno = tcph->sport;
		tcph->sport = tcph->dport;
		tcph->dport = portno;

		/* TCP ヘッダに情報を設定する。*/
		tcph->doff = TCP_MAKE_DATA_OFF(TCP_HDR_SIZE);
		}
	
	/* cep が NULL であれば、何もしないで終了する。*/
	else if (cep == NULL)
		return;
	else {
		if (tcpn_get_segment(&output, cep, 0,
		                    0, (uint_t)net_buf_max_siz(),
		                    NBA_SEARCH_ASCENT, TMO_TCP_GET_NET_BUF) != E_OK)
			return;
		tcph = GET_TCP_HDR(output, IF_IP_TCP_HDR_OFFSET(output));
		flags |= TCP_FLG_ACK;
		}

	tcph->seq   = htonl(seq);
	tcph->ack   = htonl(ack);
	tcph->win   = htons(win);
	tcph->flags = flags;
	tcph->urp   = tcph->sum = 0;

	/*
	 *  チェックサムを設定する。
	 */
	hdr_offset = IF_IP_TCP_HDR_OFFSET(output);
	tcph->sum = IN_CKSUM(output, IPPROTO_TCP, hdr_offset, 
	                     (uint_t)GET_TCP_HDR_SIZE(output, hdr_offset));

	/* ネットワークバッファ長を調整する。*/
	output->len = (uint16_t)GET_IF_IP_TCP_HDR_SIZE(output, hdr_offset);

#ifdef TCP_CFG_TRACE

	tcp_output_trace(output, cep);

#endif	/* of #ifdef TCP_CFG_TRACE */

	/* ネットワーク層 (IP) の出力関数を呼び出す。*/
	IP_OUTPUT(output, TMO_TCP_OUTPUT);
	}
#if 0
/*
 *  tcp_get_segment -- TCP セグメントを獲得し、ヘッダを設定する。
 *
 *    戻り値	エラーコード
 *    optlen	オプションサイズ、4 オクテット単位
 *    maxlen	最大セグメントサイズ（IF/IP/TCP ヘッダサイズを含まない）
 *    len	TCP SDU サイズ
 */

ER
tcp_get_segment (T_NET_BUF **nbuf, T_TCP_CEP *cep,
                 uint_t optlen, uint_t len, uint_t maxlen, ATR nbatr, TMO tmout)
{
	T_TCP_HDR	*tcph;
	ER		error;

	/* IP データグラムを獲得する。*/
	if ((error = IN_GET_DATAGRAM(nbuf,
	                             (uint_t)(TCP_HDR_SIZE + optlen + len),
	                             (uint_t)(TCP_HDR_SIZE + maxlen),
	                             &cep->dstaddr.ipaddr,
	                             &cep->myaddr.ipaddr,
	                             IPPROTO_TCP, IP_DEFTTL, nbatr, tmout)) != E_OK) {
		syslog(LOG_WARNING, "[TCP] NET BUF busy, len: %d, CEP: %d.",
		                    (uint16_t)(TCP_HDR_SIZE + optlen + len), GET_TCP_CEPID(cep));
		return error;
		}

	/* TCP ヘッダに情報を設定する。*/

	/* TCP ヘッダに情報を設定する。*/
	tcph = GET_TCP_HDR(*nbuf, IF_IP_TCP_HDR_OFFSET(*nbuf));
	tcph->sport	= htons(cep->myaddr.portno);
	tcph->dport	= htons(cep->dstaddr.portno);
	tcph->doff	= TCP_MAKE_DATA_OFF(TCP_HDR_SIZE + optlen);
	tcph->sum	= tcph->flags = 0;

	return E_OK;
	}
#endif
/*
 *  tcpn_get_segment -- TCP セグメントを獲得し、ヘッダを設定する。
 *
 *    戻り値	エラーコード
 *    optlen	オプションサイズ、4 オクテット単位
 *    maxlen	最大セグメントサイズ（IF/IP/TCP ヘッダサイズを含む）
 *    len	TCP SDU サイズ
 */

ER
tcpn_get_segment (T_NET_BUF **nbuf, T_TCP_CEP *cep,
                 uint_t optlen, uint_t len, uint_t maxlen, ATR nbatr, TMO tmout)
{
	T_TCP_HDR	*tcph;
	ER		error;

	/* IP データグラムを獲得する。*/
	if ((error = IN_GET_DATAGRAM(nbuf,
	                             (uint_t)(TCP_HDR_SIZE + optlen + len),
	                             (uint_t)(maxlen - IF_IP_NET_HDR_SIZE(&cep->dstaddr.ipaddr)),
	                             &cep->dstaddr.ipaddr,
	                             &cep->myaddr.ipaddr,
	                             IPPROTO_TCP, IP_DEFTTL, nbatr, tmout)) != E_OK) {
		syslog(LOG_WARNING, "[TCP] NET BUF busy,  len:%4d, CEP: %d.",
		                    (uint16_t)(TCP_HDR_SIZE + optlen + len), GET_TCP_CEPID(cep));
		return error;
		}

	/* TCP ヘッダに情報を設定する。*/

	/* TCP ヘッダに情報を設定する。*/
	tcph = GET_TCP_HDR(*nbuf, IF_IP_TCP_HDR_OFFSET(*nbuf));
	tcph->sport	= htons(cep->myaddr.portno);
	tcph->dport	= htons(cep->dstaddr.portno);
	tcph->doff	= TCP_MAKE_DATA_OFF(TCP_HDR_SIZE + optlen);
	tcph->sum	= tcph->flags = 0;

	return E_OK;
	}

/*
 *  tcp_can_send_more -- 送信できるか、通信端点の状態を見る。
 */

ER
tcp_can_send_more (T_TCP_CEP *cep, FN fncd, TMO tmout)
{
	ER	error;

	/* 送信できるか、CEP の FSM 状態を見る。*/
	if (!TCP_FSM_CAN_SEND_MORE(cep->fsm_state)) {
		if ((error = cep->error) == E_OK)
			error = E_OBJ;

#ifdef TCP_CFG_NON_BLOCKING

		/* タイムアウトをチェックする。*/
		if (tmout == TMO_NBLK) {	/* ノンブロッキングコール */

			if (!IS_PTR_DEFINED(cep->callback))
				error = E_OBJ;
			else {
				/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)error);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)&error);
#endif
				error = E_WBLK;
				}
			}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		}
	else {

#ifdef TCP_CFG_SWBUF_CSAVE

		error = E_OK;

#else	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		if (!IS_PTR_DEFINED(cep->sbuf)) {

#ifdef TCP_CFG_NON_BLOCKING

			/* タイムアウトをチェックする。*/
			if (tmout == TMO_NBLK) {	/* ノンブロッキングコール */
				if (!IS_PTR_DEFINED(cep->callback))
					error = E_OBJ;
				else {
					error = E_OBJ;

					/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
					(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)error);
#else
					(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)&error);
#endif
					error = E_WBLK;
					}
				}
			else

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

				error = E_OBJ;
			}
		else
			error = E_OK;

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		}

	return error;
	}

/*
 *  tcp_can_recv_more -- 受信できるか、通信端点の状態を見る。
 *
 *  注意: 戻り値
 *
 *	E_OK	受信可能
 *	E_OBJ	相手から切断されたか、エラーが発生した。
 */

ER
tcp_can_recv_more (ER *error, T_TCP_CEP *cep, FN fncd, TMO tmout)
{
	/*
	 *  受信できるか、fsm_state を見る。受信できない場合は
	 *  長さ 0、またはエラーを返す。
	 */
	if (!TCP_FSM_CAN_RECV_MORE(cep->fsm_state) &&
	    cep->rwbuf_count == 0 && cep->reassq == NULL) {
		*error = cep->error;

#ifdef TCP_CFG_NON_BLOCKING

		/* タイムアウトをチェックする。*/
		if (tmout == TMO_NBLK) {	/* ノンブロッキングコール */

			if (!IS_PTR_DEFINED(cep->callback))
				*error = E_OBJ;
			else {
				/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)*error);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)error);
#endif
				*error = E_WBLK;
				}
			}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/*
		 *  通信端点をロックして、
		 *  受信ウィンドバッファキューのネットワークバッファを解放する。
		 */
		syscall(wai_sem(cep->semid_lock));
		TCP_FREE_RWBUFQ(cep);
		syscall(sig_sem(cep->semid_lock));

		return E_OBJ;
		}
	else {

#ifndef TCP_CFG_RWBUF_CSAVE

		if (!IS_PTR_DEFINED(cep->rbuf)) {

#ifdef TCP_CFG_NON_BLOCKING

			/* タイムアウトをチェックする。*/
			if (tmout == TMO_NBLK) {	/* ノンブロッキングコール */
				if (!IS_PTR_DEFINED(cep->callback))
					*error = E_OBJ;
				else {
					*error = E_OBJ;

					/* コールバック関数を呼び出す。*/
#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
					(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)*error);
#else
					(*cep->callback)(GET_TCP_CEPID(cep), fncd, (void*)error);
#endif
					*error = E_WBLK;
					}
				}
			else

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

				*error = E_OBJ;

			return E_OBJ;
			}

#endif	/* of #ifndef TCP_CFG_RWBUF_CSAVE */

		return E_OK;
		}
	}

/*
 *  tcp_wait_rwbuf -- 受信ウィンドバッファにデータが入るのを待つ。
 */

ER
tcp_wait_rwbuf (T_TCP_CEP *cep, TMO tmout)
{
	ER	error;
	FLGPTN	flag;

	if (cep->rwbuf_count == 0) {
		/* 受信ウィンドバッファにデータがなければ、入力があるまで待つ。*/
		while (cep->rwbuf_count == 0) {
			if ((error = twai_flg(cep->rcv_flgid, TCP_CEP_EVT_RWBUF_READY, TWF_ORW, &flag, tmout)) != E_OK) {
				return error;
				}
			syscall(clr_flg(cep->rcv_flgid, (FLGPTN)(~TCP_CEP_EVT_RWBUF_READY)));

			/*
			 *  受信できるか、fsm_state を見る。受信できない状態で、
			 *  受信ウィンドバッファに文字がない場合は終了する。
			 */
			if (!TCP_FSM_CAN_RECV_MORE(cep->fsm_state) &&
			    cep->rwbuf_count == 0 && cep->reassq == NULL) {

				/*
				 *  通信端点をロックして、
				 *  受信ウィンドバッファキューのネットワークバッファを解放する。
				 */
				syscall(wai_sem(cep->semid_lock));
				TCP_FREE_RWBUFQ(cep);
				syscall(sig_sem(cep->semid_lock));

				return cep->error;
				}
			}
		}
	else {
		syscall(clr_flg(cep->rcv_flgid, (FLGPTN)(~TCP_CEP_EVT_RWBUF_READY)));
		}

	return E_OK;
	}

/*
 *  tcp_move_ra2rw -- 受信再構成キューで再構成したセグメントを受信ウィンドバッファに書き込む。
 */

uint8_t
tcp_move_ra2rw (T_TCP_CEP *cep, uint8_t flags)
{
	T_NET_BUF	*q;
	T_TCP_Q_HDR	*qhdr;

	/*
	 *  受信データを受信ウィンドバッファに書き込み、
	 *  rcv_nxt を完了した SEQ だけ進める。
	 */
	if (TCP_FSM_HAVE_ESTABLISHED(cep->fsm_state)) {
		while (cep->reassq != NULL) {
			q = cep->reassq;
			qhdr = GET_TCP_Q_HDR(q, GET_IP_TCP_Q_HDR_OFFSET(q));
			if (qhdr->seq != cep->rcv_nxt)
				break;

			/* 次のセグメントに進む。*/
			cep->reassq = qhdr->next;

			/* FIN フラグのみ残す。*/
			flags &= TCP_FLG_FIN;

			/* データを受信ウィンドバッファに書き込む。*/
			TCP_WRITE_RWBUF(cep, q, (uint_t)(GET_IP_TCP_Q_HDR_OFFSET(q)));
			}
		}
	if (cep->reassq != NULL) {
		cep->flags |= TCP_CEP_FLG_ACK_NOW;
		flags &= ~TCP_FLG_FIN;
		}
	return flags;
	}

/*
 *  tcp_write_raque -- 受信セグメントを再構成して、受信再構成キューに繋ぐ。
 */

uint8_t
tcp_write_raque (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, uint8_t flags)
{
	T_NET_BUF	*new;
	T_NET_BUF	*p, *q, *nq;
	T_TCP_Q_HDR	*qhdr, *inqhdr = GET_TCP_Q_HDR(input, thoff);
	int32_t		len;

	/*  TCP ヘッダの位置を保存する。*/
	SET_IP_TCP_Q_HDR_OFFSET(input, thoff);

	/*
	 *  MAX_TCP_REALLOC_SIZE 以下の場合は、新たにネットワークバッファを
	 *  割当てて、データをコピーする。
	 *  このとき、IP のオプション（拡張ヘッダ）と TCP のオプションは削除する。
	 */
	len  = IF_IP_TCP_HDR_SIZE(input) + inqhdr->slen;

	if (len <= MAX_TCP_REALLOC_SIZE) {

		/*
		 *  ネットワークバッファが確保できないときは割当てない。
		 */
		if (tget_net_buf(&new, (uint_t)len, TMO_TCP_GET_NET_BUF) != E_OK)
			new = NULL;
		}
	else
		new = NULL;

	/*
	 *    inqhdr->seq               
	 *    |
	 *    |<------- inqhdr->slen ------->|
	 *    v                              v
	 *    +------------------------------+
	 *    |             input            |
	 *    +------------------------------+
	 *        ^                  ^
	 *        |<----- slen ----->|
	 *        |
	 *        soff
	 *
	 */
	inqhdr->soff = 0;

	/*
	 *  今回追加するセグメントより前(p)と後(q)のセグメントを探す。
	 *
	 *    +-------------------------+
	 *    |            p            |
	 *    +-------------------------+
	 *    ^              +---------------------------+
	 *    p->seq         |            input          |
	 *                   +---------------------------+
	 *                   ^                      +-------------------------+
	 *                   inqhdr->seq            |            q            |
	 *                                          +-------------------------+
	 *                                          ^
	 *                                          q->seq
	 */
	for (q = cep->reassq, p = NULL; q != NULL; ) {
		qhdr = GET_TCP_Q_HDR(q, GET_IP_TCP_Q_HDR_OFFSET(q));
		if (SEQ_GT(qhdr->seq, inqhdr->seq))
			break;
		p = q;
		q = qhdr->next;
		}

	/*
	 *  前(p)のセグメントがあれば、既に、今回と同じセグメントが
	 *  あることもある。その場合は、その部分または全てを破棄する。
	 *
	 *    qhdr->seq
	 *    |
	 *    |<----- qhdr->slen ------>|
	 *    v                         v
	 *    +-------------------------+
	 *    |            p            |
	 *    +-------------------------+
	 *                    <--len -->
	 *                   +---------------------------+
	 *                   |            input          |
	 *                   +---------------------------+
	 *                   ^
	 *                   inqhdr->seq
	 */
	if (p != NULL) {
		qhdr = GET_TCP_Q_HDR(p, GET_IP_TCP_Q_HDR_OFFSET(p));
		len = qhdr->seq + qhdr->slen - inqhdr->seq;
		if (len > 0) {

			/*
			 *  len が正であれば、前(p) のセグメントと今回の
			 *  セグメントが重なっている。
			 */
			if (len >= inqhdr->slen) {

				/*
				 *  len が今回のセグメントと同じなら
				 *  まったく同じセグメントなので何もしない。
				 */
				if (new != NULL) {
					syscall(rel_net_buf(new));
					}

				syscall(rel_net_buf(input));
				return flags;
				}

			/*
			 *  追加するセグメントの SDU を len 分前に詰める。
			 */
			inqhdr->seq  += len;
			inqhdr->soff += (uint16_t)len;
			inqhdr->slen -= (uint16_t)len;
			}
		}

	/*
	 *  キューにつなぐ前に、新しいネットワークバッファにデータを移す。
	 *  TCP のオプションは削除する。
	 */
	if (new != NULL) {
		memcpy(new->buf, input->buf, (size_t)(thoff + TCP_HDR_SIZE));
		memcpy((uint8_t*)GET_TCP_HDR(new, thoff) + TCP_HDR_SIZE,
		                 GET_TCP_SDU(input, thoff), (size_t)(inqhdr->slen));
		syscall(rel_net_buf(input));
		input = new;
		inqhdr = GET_TCP_Q_HDR(input, thoff);
		inqhdr->doff = (uint8_t)TCP_MAKE_DATA_OFF(TCP_HDR_SIZE);
		}

	/*
	 *  重なる後(q)のセグメントを調整する。
	 *
	 *    inqhdr->seq
	 *    |
	 *    |<----- inqhdr->slen ---->|
	 *    v                         v
	 *    +-------------------------+
	 *    |          input          |
	 *    +-------------------------+
	 *                    <--len -->
	 *                   +---------------------------+
	 *                   |              q            |
	 *                   +---------------------------+
	 *                   ^
	 *                   qhdr->seq
	 */
	while (q != NULL) {
		qhdr = GET_TCP_Q_HDR(q, GET_IP_TCP_Q_HDR_OFFSET(q));
		len = inqhdr->seq + inqhdr->slen - qhdr->seq;
		if (len <= 0)
			/* len が負なら重なっていない。*/
			break;
		else if (len < qhdr->slen) {

			/*
			 *  後ろで重なっているセグメントの SDU を len 分前に詰める。
			 *
			 *    inqhdr->seq
			 *    |
			 *    |<----- inqhdr->slen ---->|
			 *    v                         v
			 *    +-------------------------+
			 *    |          input          |
			 *    +-------------------------+
			 *                    <--len -->
			 *            +------+---------------------------+
			 *            |      |              q            |
			 *            +------+---------------------------+
			 *                   ^--------->
			 *                   qhdr->seq
			 *                   qhdr->soff
			 */
			qhdr->seq  += len;
			qhdr->soff += (uint16_t)len;
			qhdr->slen -= (uint16_t)len;
			break;
			}
		nq = qhdr->next;
		if (p)
			GET_TCP_Q_HDR(p, GET_IP_TCP_Q_HDR_OFFSET(p))->next = nq;
		else
			cep->reassq = nq;
		syscall(rel_net_buf(q));
		q = nq;
		}

	if (p == NULL) {
		inqhdr->next = cep->reassq;
		cep->reassq = input;
		}
	else {
		inqhdr->next = GET_TCP_Q_HDR(p, GET_IP_TCP_Q_HDR_OFFSET(p))->next;
		GET_TCP_Q_HDR(p, GET_IP_TCP_Q_HDR_OFFSET(p))->next = input;
		}

	return tcp_move_ra2rw(cep, flags);
	}

/*
 *  tcp_rexmt_val -- 再送タイムアウト値を返す。
 */

T_TCP_TIME
tcp_rexmt_val (T_TCP_CEP *cep)
{
	T_TCP_TIME val;

	val = ((cep->srtt >> (TCP_SRTT_SHIFT - TCP_DELTA_SHIFT)) + cep->rttvar) >> TCP_DELTA_SHIFT;
	if (val < TCP_TVAL_MIN)
		return TCP_TVAL_MIN;
	else
		return val;
	}

/*
 *  tcp_init_cep -- 通信端点を初期化する。
 */

void
tcp_init_cep (T_TCP_CEP *cep)
{

#ifdef TCP_CFG_RWBUF_CSAVE
	/*
	 * 受信ウィンドバッファの省コピー機能を有効にした場合、
	 * 受信ウィンドバッファが全て解放されないことがあるので、
	 * ここで解放する。
	 */
	if (cep->rwbufq != NULL) {
		cep->rwbuf_count = 0;
		TCP_FREE_RWBUFQ(cep);
		}
#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE */


	memset((uint8_t*)cep + offsetof(T_TCP_CEP, timer), 0,
	       sizeof(T_TCP_CEP) - offsetof(T_TCP_CEP, timer));

	cep->sbuf_rptr	= cep->sbuf_wptr = cep->sbuf;
	cep->rbuf_rptr	= cep->rbuf_wptr = cep->rbuf;

	cep->maxseg	= DEF_TCP_SND_SEG;		/* 送信最大セグメントサイズ		*/
	cep->srtt	= TCP_TVAL_SRTT_BASE;		/* 滑らかな移動平均			*/
	cep->rttvar	= ((TCP_TVAL_RTO_BASE - TCP_TVAL_SRTT_BASE) << TCP_RTTVAR_SHIFT) / 4;
							/* 滑らかな分散				*/
	cep->rxtcur	= TCP_TVAL_RTO_BASE;		/* 現在の再送値				*/
	cep->snd_cwnd	=  MAX_TCP_WIN_SIZE;		/* 輻輳ウィンドサイズ			*/
	cep->snd_ssthresh= MAX_TCP_WIN_SIZE;		/* 輻輳ウィンドサイズ(snd_cwnd)の	制限値	*/

	/*
	 * 以下に関係しないフラグをクリアーする。
	 * ・送受信ウィンドバッファの省コピー機能
	 * ・動的な通信端点の生成・削除機能
	 * ・通信端点のネットワーク層プロトコル
	 */
	cep->flags &= TCP_CEP_FLG_NOT_CLEAR;

	/* セマフォを初期化する。*/
	sig_sem(cep->semid_lock);

	/* フラグを初期化する。*/
	syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));
	syscall(clr_flg(cep->rcv_flgid, TCP_CEP_EVT_RWBUF_READY));
	}

/*
 *  tcp_notify -- ICMP エラーの処理
 */

void
tcp_notify (T_NET_BUF *input, ER error)
{
	T_TCP_CEP	*cep;

	if ((cep = tcp_find_cep(input, GET_TCP_HDR_OFFSET(input))) != NULL) {

		/*
		 *  コネクション開設済で、ホスト到達不能エラーの場合は、
		 *  再送により処理する。
		 */
		if (cep->fsm_state == TCP_FSM_ESTABLISHED &&
		    (error == EV_NURCH || error == EV_HURCH || error == EV_HDOWN))
			return;

		/*
		 *  コネクション開設中、同じエラーを何度か受信した場合は、
		 *  待ち状態を解除し、対応する関数にエラーを返させる。
		 */
		if (cep->fsm_state < TCP_FSM_ESTABLISHED && cep->rxtshift > 3 && cep->net_error != E_OK) {
			cep->error = E_CLS;
			tcp_close(cep);
			}
		else {
			cep->net_error = error;

			/* 送信を指示する。*/
			cep->flags |=  TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);
			}
		}
	}

/*
 *  tcp_lock_cep -- TCP 通信端点をロックする。
 */

ER
tcp_lock_cep (ID cepid, T_TCP_CEP **p_cep, FN tfn)
{
	T_TCP_CEP	*cep;
	ER		error = E_OK;

	*p_cep = NULL;

	/* TCP 通信端点 ID をチェックする。*/
	if (!VALID_TCP_CEPID(cepid))
		return E_ID;

	/* TCP 通信端点を得る。*/
	cep = GET_TCP_CEP(cepid);

	/* TCP 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* TCP 通信端点をチェックする。*/
	if (!VALID_TCP_CEP(cep)) {
		syscall(sig_sem(cep->semid_lock));
		return E_NOEXS;
		}

	/*
	 *  API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if (IS_TFN_TCP_RCV(tfn)) {
		if (cep->rcv_tfn != TFN_TCP_UNDEF ||
		    (tfn == TFN_TCP_ACP_CEP && cep->snd_tfn == TFN_TCP_CON_CEP)) 
			error = E_OBJ;
		else {
			get_tid(&(cep->rcv_tskid));
			cep->rcv_tfn = tfn;
			}
		}
	else {
		if (cep->snd_tfn != TFN_TCP_UNDEF ||
		    (tfn == TFN_TCP_CON_CEP && cep->rcv_tfn == TFN_TCP_ACP_CEP) ||
		    (tfn == TFN_TCP_CON_CEP && cep->rcv_tfn == TFN_TCP_CLS_CEP))
			error = E_OBJ;
		else {
			get_tid(&(cep->snd_tskid));
			cep->snd_tfn = tfn;
			}
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	*p_cep = cep;
	return error;
	}

#ifdef TCP_CFG_TRACE

/*
 *  トレース出力に用いるシリアルポート番号
 */

#ifndef CONSOLE_PORTID
#define	CONSOLE_PORTID		LOGTASK_PORTID
#endif

/*
 *  コネクションの状態
 */

static const char tcp_strfsm[][3] = {
	{ 'C', 'D', '\0' },
	{ 'L', 'I', '\0' },
	{ 'S', 'S', '\0' },
	{ 'S', 'R', '\0' },
	{ 'E', 'S', '\0' },
	{ 'C', 'W', '\0' },
	{ 'F', '1', '\0' },
	{ 'C', 'G', '\0' },
	{ 'L', 'A', '\0' },
	{ 'F', '2', '\0' },
	{ 'T', 'W', '\0' },
	};

/*
 *  trace_putchar -- シリアルポートへの文字出力
 */

static void
trace_putchar (ID portid, char ch)
{
	serial_wri_dat(portid, &ch, 1);
	/*cpu_putc(ch);*/
	}

/*
 *  シリアルポートへの書式付文字列出力ライブラリ
 */

/*
 *  数値変換のための変換表
 */

static const char radhex[] = "0123456789abcdef";
static const char radHEX[] = "0123456789ABCDEF";

/*
 *  convert -- trace_printf の数値変換
 */

static int_t
convert (ID portid, long_t val, int_t radix,
         const char *radchar, int_t width, bool_t minus, char padchar)
{
	char	digits[24];
	int_t	ix, pad, pchars;

	ix = 0;
	do {
		if (minus)
			digits[ix ++] = radchar[val % radix];
		else
			digits[ix ++] = radchar[(ulong_t)val % radix];
		val /= radix;
		} while (val != 0);

	if (minus)
		digits[ix ++] = '-';

	if (width > ix)
		pchars = width;
	else
		pchars = ix;

	for (pad = ix; pad < width; pad ++)
		trace_putchar(portid, padchar);

	while (ix -- > 0)
		trace_putchar(portid, digits[ix]);

	return pchars;
	}

/*
 *  trace_printf -- シリアルポートへの書式付文字列出力
 */

static void
trace_printf (ID portid, const char *fmt, ...)
{
	va_list	ap;
	ulong_t	val;
	char	padchar, *str;
	int_t	ch, width, longflag;

	va_start(ap, fmt);
	while ((ch = *fmt ++) != '\0') {
		if (ch != '%') {		/* 書式指定以外 */
			trace_putchar(portid, (char)ch);
			continue;
			}

		width = longflag = 0;
		padchar = ' ';
		if ((ch = *fmt ++) == '0') {	/* 上位桁の 0 */
			padchar = '0';
			ch = *fmt ++;
			}

		while ('0' <= ch && ch <= '9') {	/* 出力幅 */
			width = width * 10 + ch - '0';
			ch = *fmt ++;
			}

		while (ch == 'l') {		/* long (long) の指定 */
			longflag ++;
			ch = *fmt ++;
			}

		switch (ch) {
		case 'd':
			val = longflag ? (ulong_t)va_arg(ap, long_t)
			               : (ulong_t)va_arg(ap, int_t);
			if (val >= 0)
				convert(portid,  val, 10, radhex, width, false, padchar);
			else
				convert(portid, -val, 10, radhex, width, true, padchar);
			break;

		case 'u':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			convert(portid, val, 10, radhex, width, false, padchar);
			break;

		case 'x':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			convert(portid, val, 16, radhex, width, false, padchar);
			break;

		case 'X':
			val = longflag ? (ulong_t)va_arg(ap, ulong_t)
			               : (ulong_t)va_arg(ap, uint_t);
			convert(portid, val, 16, radHEX, width, false, padchar);
			break;

		case 'c':
			ch = va_arg(ap, int_t);
			trace_putchar(portid, (char)ch);
			break;

		case 's':
			str = va_arg(ap, char*);
			while ((ch = *str ++) != '\0')
				trace_putchar(portid, (char)ch);
			break;

		case '%':
			trace_putchar(portid, '%');
			break;

		case '0':
			fmt --;
			break;

		default:
			break;
			}

		}
	va_end(ap);
	}

/*
 *  TCP フラグ文字列
 */

static char *
get_tcp_flag_str (char *buf, uint8_t flags)
{
	static char tcp_flag_str[] = "FSRPAU";

	int_t	bit, mask = 1;

	for (bit = 0; bit < sizeof(tcp_flag_str); bit ++) {
		buf[sizeof(tcp_flag_str) - bit - 1] = (char)((flags & mask) ? tcp_flag_str[bit] : '-');
		mask <<= 1;
		}
	buf[sizeof(tcp_flag_str)] = '\0';
	return buf;
	}

/*
 *  送信 TCP ヘッダのトレース出力
 */

void
tcp_output_trace (T_NET_BUF *output, T_TCP_CEP *cep)
{
	SYSTIM		time;
	T_TCP_HDR	*tcph;
	char		buf[9];

	if (!(TCP_CFG_TRACE_LPORTNO == TCP_PORTANY || cep->myaddr.portno  == TCP_CFG_TRACE_LPORTNO) ||
	    !(TCP_CFG_TRACE_RPORTNO == TCP_PORTANY || cep->dstaddr.portno == TCP_CFG_TRACE_RPORTNO))
		return;

#if defined(_IP4_CFG)

#if defined(_IP6_CFG)

	if (!((TCP_CFG_TRACE_IPV4_RADDR == IPV4_ADDRANY) ||
	     ((cep->flags & TCP_CEP_FLG_IPV4) && 
	      IN6_IS_ADDR_V4MAPPED(&cep->dstaddr.ipaddr) &&
	      (ntohl(cep->dstaddr.ipaddr.s6_addr32[3]) == TCP_CFG_TRACE_IPV4_RADDR))))
		return;

#else	/* of #if defined(_IP6_CFG) */

	if (!(TCP_CFG_TRACE_IPV4_RADDR == IPV4_ADDRANY || cep->dstaddr.ipaddr == TCP_CFG_TRACE_IPV4_RADDR))
		return;

#endif	/* of #if defined(_IP6_CFG) */

#endif	/* of #if defined(_IP4_CFG) */

	syscall(wai_sem(SEM_TCP_TRACE));
	syscall(get_tim(&time));
	tcph = GET_TCP_HDR(output, GET_TCP_HDR_OFFSET(output));
	if (time > 99999999)
		trace_printf(CONSOLE_PORTID, "=O%10d", time / 1000);
	else
		trace_printf(CONSOLE_PORTID, "=O%6d.%03d", time / 1000, time % 1000);
	if (cep == NULL)
		trace_printf(CONSOLE_PORTID, "=c:-- s:-- f:--------");
	else
		trace_printf(CONSOLE_PORTID, "=c:%2d s:%s f:%08x",
		                             GET_TCP_CEPID(cep),
		                             tcp_strfsm[cep->fsm_state], cep->flags);
	trace_printf(CONSOLE_PORTID, ":%s", get_tcp_flag_str(buf, tcph->flags));
	if (cep->fsm_state >= TCP_FSM_ESTABLISHED)
		trace_printf(CONSOLE_PORTID, " s:%10u a:%10u",
		                             ntohl(tcph->seq) - cep->iss,
		                             ntohl(tcph->ack) - cep->irs);
	else
		trace_printf(CONSOLE_PORTID, " s:%10u a:%10u",
		                             ntohl(tcph->seq), ntohl(tcph->ack));
	trace_printf(CONSOLE_PORTID, " w:%5d l:%4d>\n", 
	                             ntohs(tcph->win),
	                             GET_IP_SDU_SIZE(output) - TCP_HDR_LEN(tcph->doff));
	syscall(sig_sem(SEM_TCP_TRACE));
	}

/*
 *  受信 TCP ヘッダのトレース出力
 *
 *    注意: TCP ヘッダ内は、すでにホストバイトオーダ
 */

void
tcp_input_trace (T_NET_BUF *input, T_TCP_CEP *cep)
{
	SYSTIM		time;
	T_TCP_HDR	*tcph;
	char		buf[9];

	if (!(TCP_CFG_TRACE_LPORTNO == TCP_PORTANY || cep->myaddr.portno  == TCP_CFG_TRACE_LPORTNO) ||
	    !(TCP_CFG_TRACE_RPORTNO == TCP_PORTANY || cep->dstaddr.portno == TCP_CFG_TRACE_RPORTNO))
		return;

#if defined(_IP4_CFG)

#if defined(_IP6_CFG)

	if (!((TCP_CFG_TRACE_IPV4_RADDR == IPV4_ADDRANY) ||
	     ((cep->flags & TCP_CEP_FLG_IPV4) && 
	      IN6_IS_ADDR_V4MAPPED(&cep->dstaddr.ipaddr) &&
	      (ntohl(cep->dstaddr.ipaddr.s6_addr32[3]) == TCP_CFG_TRACE_IPV4_RADDR))))
		return;

#else	/* of #if defined(_IP6_CFG) */

	if (!(TCP_CFG_TRACE_IPV4_RADDR == IPV4_ADDRANY || cep->dstaddr.ipaddr == TCP_CFG_TRACE_IPV4_RADDR))
		return;

#endif	/* of #if defined(_IP6_CFG) */

#endif	/* of #if defined(_IP4_CFG) */

	syscall(wai_sem(SEM_TCP_TRACE));
	syscall(get_tim(&time));
	tcph = GET_TCP_HDR(input, GET_TCP_HDR_OFFSET(input));
	if (time > 99999999)
		trace_printf(CONSOLE_PORTID, "<I%10d", time / 1000);
	else
		trace_printf(CONSOLE_PORTID, "<I%6d.%03d", time / 1000, time % 1000);
	if (cep == NULL)
		trace_printf(CONSOLE_PORTID, "=c:-- s:-- f:--------");
	else
		trace_printf(CONSOLE_PORTID, "=c:%2d s:%s f:%08x",
		                             GET_TCP_CEPID(cep),
		                             tcp_strfsm[cep->fsm_state], cep->flags);
	trace_printf(CONSOLE_PORTID, ":%s", get_tcp_flag_str(buf, tcph->flags));
	if (cep->fsm_state >= TCP_FSM_SYN_RECVD)
		trace_printf(CONSOLE_PORTID, " a:%10u s:%10u", tcph->ack - cep->iss, tcph->seq - cep->irs);
	else
		trace_printf(CONSOLE_PORTID, " a:%10u s:%10u", tcph->ack, tcph->seq);
	trace_printf(CONSOLE_PORTID, " w:%5d l:%4d=\n", 
	                             tcph->win, GET_IP_SDU_SIZE(input) - TCP_HDR_LEN(tcph->doff));
	syscall(sig_sem(SEM_TCP_TRACE));
	}

#endif	/* of #ifdef TCP_CFG_TRACE */

#endif	/* of #ifdef SUPPORT_TCP */
