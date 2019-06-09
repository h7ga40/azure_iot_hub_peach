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
 *  TCP ECHO サーバ #1
 *
 *    ・送受信タスク同一型
 *    ・ノンブロッキングコール
 *    ・省コピー API
 *    ・IPv4/IPv6
 *    ・緊急データの受信
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/tcp_echo_srv1.h>
#include <netapp/tcp_echo_srv1_var.h>

#ifdef USE_TCP_ECHO_SRV1

/*
 *  表示
 */

//#define SHOW_RCV_RANGE

/*
 *  コネクション切断方法の指定
 */

#define USE_TCP_SHT_CEP

/*
 *  タイムアウト
 */

#ifdef TOPPERS_S810_CLG3_85

#define CLS_TMO		TMO_FEVR		/* Close Wait は標準で 60秒 */
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(30*SYSTIM_HZ)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(30*SYSTIM_HZ)

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

//#define CLS_TMO		TMO_FEVR	/* Close Wait は標準で 60秒 */
#define CLS_TMO		(70*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*10)
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(30*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*30)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(40*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*20)

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

#ifdef USE_TCP_EXTENTIONS

/*
 *  変数
 */

typedef struct t_tcp_echo_srv_task_info {
	ID	taskid;		/* タスク ID		*/
	bool_t	active;		/* 動作状態		*/
	char	apip;		/* API IPプロトコル	*/
	} T_TCP_ECHO_SRV_TASK_INFO;

static T_TCP_ECHO_SRV_TASK_INFO tcp_echo_srv_task_info[NUM_TCP_ECHO_SRV_TASKS] = {

				{ TCP_ECHO_SRV_TASK1, false },

#if NUM_TCP_ECHO_SRV_TASKS >= 2
				{ TCP_ECHO_SRV_TASK2, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 3
				{ TCP_ECHO_SRV_TASK3, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 4
				{ TCP_ECHO_SRV_TASK4, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 5
				{ TCP_ECHO_SRV_TASK5, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 6
				{ TCP_ECHO_SRV_TASK6, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 7
				{ TCP_ECHO_SRV_TASK7, false },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 8
				{ TCP_ECHO_SRV_TASK8, false },
#endif

			};

/*
 *  get_tcp_rep -- TCP 受付口を獲得する。
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

static ER
get_tcp_rep (ID *repid, char apip)
{
	ID		tskid;
	T_TCP6_CREP	crep6;
	T_TCP_CREP	crep4;

	get_tid(&tskid);
	if (apip == API_PROTO_IPV6) {
		crep6.repatr = UINT_C(0);
		crep6.myaddr.portno = UINT_C(7);
		memcpy(&crep6.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
		return alloc_tcp6_rep(repid, tskid, &crep6);
		}
	else {
		crep4.repatr = UINT_C(0);
		crep4.myaddr.portno = UINT_C(7);
		crep4.myaddr.ipaddr = IPV4_ADDRANY;
		return alloc_tcp4_rep(repid, tskid, &crep4);
		}
	}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

static ER
get_tcp_rep (ID *repid, char apip)
{
	ID		tskid;
	T_TCPN_CREP	crep;

	get_tid(&tskid);

	crep.repatr = UINT_C(0);
	crep.myaddr.portno = UINT_C(7);

#if defined(SUPPORT_INET6)

	memcpy(&crep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));

#else	/* #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
	crep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#endif	/* #if defined(SUPPORT_INET6) */

	return ALLOC_TCP_REP(repid, tskid, &crep);
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  get_tcp_cep -- TCP 通信端点を獲得する。
 */

static ER
get_tcp_cep (ID *cepid, char apip)
{
	ID		tskid;
	T_TCP_CCEP	ccep;

	get_tid(&tskid);

	ccep.cepatr = UINT_C(0);
	ccep.sbufsz = TCP_ECHO_SRV_SWBUF_SIZE;
	ccep.rbufsz = TCP_ECHO_SRV_RWBUF_SIZE;

#ifdef TCP_CFG_SWBUF_CSAVE
	ccep.sbuf = NADR;
#else
	ccep.sbuf = tcp_echo_srv_swbuf;
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = tcp_echo_srv_rwbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
	ccep.callback = (FP)callback_nblk_tcp_echo_srv;
#else
	ccep.callback = NULL;
#endif

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if (apip == API_PROTO_IPV6)
		return alloc_tcp6_cep(cepid, tskid, &ccep);
	else
		return alloc_tcp4_cep(cepid, tskid, &ccep);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	return ALLOC_TCP_CEP(cepid, tskid, &ccep);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	}

/*
 *  wakeup_tcp_echo_srv -- TCP エコーサーバタスクの休止解除
 */

ER
wakeup_tcp_echo_srv (char apip)
{
	int	ix;

	for (ix = NUM_TCP_ECHO_SRV_TASKS; -- ix >= 0; )
		if (!tcp_echo_srv_task_info[ix].active) {
			syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] wake up.",
			                   apip, tcp_echo_srv_task_info[ix].taskid);
			tcp_echo_srv_task_info[ix].apip   = apip;
			tcp_echo_srv_task_info[ix].active = true;
			return syscall(wup_tsk(tcp_echo_srv_task_info[ix].taskid));
			}
	syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] wake up error: %s.",
			   apip, tcp_echo_srv_task_info[ix].taskid, itron_strerror(E_ID));
	return E_ID;
	}

/*
 *  tcp_echo_srv_task -- TCP エコーサーバタスク
 */

void
tcp_echo_srv_task (intptr_t exinf)
{
	ID	tskid;
	ER	error = E_OK;
	int	ix;
	char	apip;

	syscall(get_tid(&tskid));
	syslog(LOG_NOTICE, "[TCPn ECHO SRV(EXT):%d] started.", tskid);
	while (true) {
		syslog(LOG_NOTICE, "[TCPn ECHO SRV(EXT):%d] goto sleep.", tskid);
		syscall(slp_tsk());

		for (ix = NUM_TCP_ECHO_SRV_TASKS; -- ix >= 0; ) {
			if (tcp_echo_srv_task_info[ix].taskid == tskid) {
				break;
				}
			}

		apip = tcp_echo_srv_task_info[ix].apip;
		if ((error = get_tcp_cep(&tcp_echo_srv_info[ix].cepid, apip)) != E_OK) {
			syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] create CEP error: %s",
		                           apip, tskid, itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep(&tcp_echo_srv_info[ix].repid, apip)) != E_OK) {
				syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] create REP error: %s",
		                                   apip, tskid, itron_strerror(error));
				break;
				}
			else if ((error = tcp_echo_srv(ix, apip)) != E_OK) {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

				if (apip == API_PROTO_IPV6)
					error = free_tcp6_rep(tcp_echo_srv_info[ix].repid, error != E_DLT);
				else
					error = free_tcp4_rep(tcp_echo_srv_info[ix].repid, error != E_DLT);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

				error = FREE_TCP_REP(tcp_echo_srv_info[ix].repid, error != E_DLT);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

				if (error != E_OK) {
					syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] delete REP error: %s",
		                                           apip, tskid, itron_strerror(error));
					}

				break;
				}
			}

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

		if (apip == API_PROTO_IPV6)
			error = free_tcp6_cep(tcp_echo_srv_info[ix].cepid);
		else
			error = free_tcp4_cep(tcp_echo_srv_info[ix].cepid);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		error = FREE_TCP_CEP(tcp_echo_srv_info[ix].cepid);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		if (error != E_OK) {
			syslog(LOG_NOTICE, "[TCP%c ECHO SRV(EXT):%d] delete CEP error: %s",
		                           apip, tskid, itron_strerror(error));
			}

		for (ix = NUM_TCP_ECHO_SRV_TASKS; -- ix >= 0; ) {
			if (tcp_echo_srv_task_info[ix].taskid == tskid) {
				tcp_echo_srv_task_info[ix].active = false;
				break;
				}
			}

		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_ECHO_SRV1 */
