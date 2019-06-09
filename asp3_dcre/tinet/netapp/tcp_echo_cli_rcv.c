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
 *  TCP ECHO クライアント受信タスク
 */

#include <stdlib.h>

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
#include <netapp/tcp_echo_cli.h>

#ifdef USE_TCP_ECHO_CLI

/* 表示 */

//#define SHOW_TIME_OUT
//#define SHOW_RCV_RANGE

/* タイムアウト */

#ifdef TOPPERS_S810_CLG3_85

#define CLS_TMO		TMO_FEVR					/* Close Wait は標準で 60秒 */
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(30*SYSTIM_HZ)

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

//#define CLS_TMO		TMO_FEVR				/* Close Wait は標準で 60秒 */
#define CLS_TMO		(70*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*10)
//#define RCV_TMO		TMO_FEVR
#define RCV_TMO		(40*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*20)

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

/*
 *  API IPプロトコルの指定
 */

#undef  DEFAULT_API_PROTO

#if defined(SUPPORT_INET6)
#if defined(SUPPORT_INET4)
#define DEFAULT_API_PROTO	API_PROTO_IPVn
#else
#define DEFAULT_API_PROTO	API_PROTO_IPV6
#endif
#else
#define DEFAULT_API_PROTO	API_PROTO_IPV4
#endif

/*
 *  TCP ECHO クライアント受信タスク
 */

#ifdef USE_TCP_NON_BLOCKING

#ifdef USE_COPYSAVE_API

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER		error;
	SYSTIM		time;
	uint32_t	total, rcount;
	uint16_t	roff, rlen;
	char		*rmsg, head, tail, apip = DEFAULT_API_PROTO;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP%c ECHO CLI (NBLK,CS) RCV:%d] started.", DEFAULT_API_PROTO, tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s",
			                   DEFAULT_API_PROTO, cepid, itron_strerror(error));
			}
                else {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

#ifdef USE_TCP_EXTENTIONS

			apip = API_PROTO_IPVn;

#else	/* of #ifdef USE_TCP_EXTENTIONS */

			if (cepid == TCP4_ECHO_CLI_CEPID)
				apip = API_PROTO_IPV4;
			else
				apip = API_PROTO_IPV6;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

			roff = rcount = total = 0;
			while (true) {
				if ((error = tcp_rcv_buf(cepid, (void**)&rmsg, TMO_NBLK)) != E_WBLK) {
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] error: %s", apip, cepid, itron_strerror(error));
					break;
					}

				/* 受信バッファの獲得が完了するまで待つ。*/
				syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
				if (tcp_echo_cli_nblk_error < 0)
					break;

				rlen = tcp_echo_cli_nblk_error;
				head = *rmsg;
				tail = *(rmsg + rlen - 1);
				if (rlen > 0) {
					roff  += rlen;
					total += rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] "
					                   "count: %7lu, len: %4u, off: %4u, data: %02x -> %02x",
					                   apip, cepid, rcount, rlen, roff, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;

				if ((error = tcp_rel_buf(cepid, rlen)) != E_OK)
					syslog(LOG_NOTICE, "[TEC%c:%02u REL] error: %s", 
					                   apip, cepid, itron_strerror(error));
				}

			if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK && error != E_OK)
				syslog(LOG_NOTICE, "[TEC%c:%02u CLS] error: %s", apip, cepid, itron_strerror(error));

			/* 切断が完了するまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC%c:%02u RCV] finsh: %7lu, ttl: %lu",
			                   apip, cepid, time / SYSTIM_HZ, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#else	/* of #ifdef USE_COPYSAVE_API */

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	static char rmsg[RCV_BUF_SIZE];

	ID		tskid, cepid;
	ER		error;
	SYSTIM		time;
	uint32_t	total, rcount;
	uint16_t	roff, rlen;
	char		head, tail, apip = DEFAULT_API_PROTO;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP%c ECHO CLI (NBLK) RCV:%d] started.", DEFAULT_API_PROTO, tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s",
			                   DEFAULT_API_PROTO, cepid, itron_strerror(error));
			}
                else {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

			if (cepid == TCP4_ECHO_CLI_CEPID)
				apip = API_PROTO_IPV4;
			else
				apip = API_PROTO_IPV6;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

			roff = rcount = total = 0;
			while (true) {
				if ((error = tcp_rcv_dat(cepid, rmsg, sizeof(rmsg), TMO_NBLK)) != E_WBLK) {
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] error: %s", apip, cepid, itron_strerror(error));
					break;
					}

				/* 受信が完了するまで待つ。*/
				syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
				if (tcp_echo_cli_nblk_error < 0)
					break;

				rlen = tcp_echo_cli_nblk_error;
				head = *rmsg;
				tail = *(rmsg + rlen - 1);
				rcount ++;
				if (rlen > 0) {
					roff  += rlen;
					total += rlen;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] "
					                   "count: %7lu, len: %4u, off: %4u, data: %02x -> %02x",
					                   apip, cepid, rcount, rlen, roff, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				}

			if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK && error != E_OK)
				syslog(LOG_NOTICE, "[TEC%c:%02u CLS] error: %s", apip, cepid, itron_strerror(error));

			/* 切断が完了するまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC%c:%02u RCV] finsh: %7lu, ttl: %lu",
			                   apip, cepid, time / SYSTIM_HZ, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_COPYSAVE_API

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time, tmo;
	uint32_t	total = 0, rcount;
	uint16_t	roff;
	char		*rmsg, apip = DEFAULT_API_PROTO;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP%c ECHO CLI (CS) RCV:%d] started.", DEFAULT_API_PROTO, tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s",
			                   DEFAULT_API_PROTO, cepid, itron_strerror(error));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

#ifdef USE_TCP_EXTENTIONS

			if (tcp6_is_cepid(cepid))
				apip = API_PROTO_IPV6;
			else
				apip = API_PROTO_IPV4;

#else	/* of #ifdef USE_TCP_EXTENTIONS */

			if (cepid == TCP4_ECHO_CLI_CEPID)
				apip = API_PROTO_IPV4;
			else
				apip = API_PROTO_IPV6;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

			}
		else {
			roff = rcount = total = 0;
			while (true) {
				tmo = RCV_TMO;
#ifdef SHOW_TIME_OUT
				syslog(LOG_NOTICE, "[TEC%c:%02u RCV] tmo:   %7lu", apip, cepid, tmo);;
#endif	/* of #ifdef SHOW_TIME_OUT */
				if ((rlen = tcp_rcv_buf(cepid, (void**)&rmsg, tmo)) < 0) {
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] error: %s",
					                   apip, cepid, itron_strerror(rlen));
					break;
					}
				else if (rlen > 0) {
					roff  += (uint16_t)rlen;
					total +=     rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] count: %7lu, len: %4u, data: %02x -> %02x",
					                   apip, cepid, rcount, (uint16_t)rlen, *rmsg, *(rmsg + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				if ((error = tcp_rel_buf(cepid, rlen)) != E_OK)
					syslog(LOG_NOTICE, "[TEC%c:%02u REL] error: %s", apip, cepid, itron_strerror(error));
				}

			tmo = CLS_TMO;
#ifdef SHOW_TIME_OUT
			syslog(LOG_NOTICE, "[TEC%c:%02u CLS] tmo:   %7lu", apip, cepid, tmo);;
#endif	/* of #ifdef SHOW_TIME_OUT */
			if ((error = tcp_cls_cep(cepid, tmo)) < 0)
				syslog(LOG_NOTICE, "[TEC%c:%02u CLS] error: %s", apip, cepid, itron_strerror(error));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC%c:%02u RCV] finsh: %7lu, ttl: %lu",
			                   apip, cepid, time / SYSTIM_HZ, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#else	/* of #ifdef USE_COPYSAVE_API */

void
tcp_echo_cli_rcv_task (intptr_t exinf)
{
	static char rmsg[RCV_BUF_SIZE];

	ID		tskid, cepid;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time, tmo;
	uint32_t	total, rcount;
	uint16_t	roff;
	char		apip = DEFAULT_API_PROTO;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP%c ECHO CLI RCV:%d] started.", DEFAULT_API_PROTO, tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s",
			                   DEFAULT_API_PROTO, cepid, itron_strerror(error));
			}
                else {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

			if (cepid == TCP4_ECHO_CLI_CEPID)
				apip = API_PROTO_IPV4;
			else
				apip = API_PROTO_IPV6;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

			roff = rcount = total = 0;
			while (true) {
#ifdef SHOW_TIME_OUT
				syslog(LOG_NOTICE, "[TEC%c:%02u RCV] tmo:   %7lu", apip, cepid, tmo);;
#endif	/* of #ifdef SHOW_TIME_OUT */
				tmo = RCV_TMO;
				if ((rlen = tcp_rcv_dat(cepid, rmsg, sizeof(rmsg), tmo)) < 0) {
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] error: %s", apip, cepid, itron_strerror(rlen));
					tcp_can_cep(cepid, TFN_TCP_ALL);
					break;
					}
				else if (rlen > 0) {
					roff  += (uint16_t)rlen;
					total +=     rlen;
					rcount ++;
#ifdef SHOW_RCV_RANGE
					syslog(LOG_NOTICE, "[TEC%c:%02u RCV] count: %7lu, len: %4u, data: %02x -> %02x",
					                   apip, cepid, rcount, (uint16_t)rlen, *rmsg, *(rmsg + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */
					}
				else
					break;
				}

#ifdef SHOW_TIME_OUT
			syslog(LOG_NOTICE, "[TEC%c:%02u CLS] tmo:   %7lu", apip, cepid, tmo);;
#endif	/* of #ifdef SHOW_TIME_OUT */
			tmo = CLS_TMO;
			if ((error = tcp_cls_cep(cepid, tmo)) < 0)
				syslog(LOG_NOTICE, "[TEC%c:%02u CLS] error: %s", apip, cepid, itron_strerror(error));

			get_tim(&time);
			syslog(LOG_NOTICE, "[TEC%c:%02u RCV] finsh: %7lu, ttl: %lu",
			                   apip, cepid, time / SYSTIM_HZ, total);
			}

		syscall(wup_tsk(TCP_ECHO_CLI_SND_TASK));
		}
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#endif	/* of #ifdef USE_TCP_ECHO_CLI */
