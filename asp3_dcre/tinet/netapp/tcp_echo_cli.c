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
 *  TCP ECHO クライアント
 *
 *    ・送受信タスク分離型
 *    ・ノンブロッキングコール
 *    ・省コピー API
 *    ・IPv4
 *    ・IPv6
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

/* echo サーバのポート番号 */

#define ECHO_SRV_PORTNO		UINT_C(7)

/* 緊急データの送信 */

#ifdef USE_TCP_EXTENTIONS

//#define SND_URG_DATA_SIZE	4
#define SND_URG_COUNT		10

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

/* 送信間隔 */

#ifdef TOPPERS_S810_CLG3_85

//#define SND_DLY		(500)
#define START_DLY	(10*SYSTIM_HZ)
#define SND_ITV		(5*SYSTIM_HZ)
#define SLP_ITV		(30*SYSTIM_HZ)

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

//#define SND_DLY		(500)
#define START_DLY	(10*SYSTIM_HZ)
#define SND_ITV		(5*SYSTIM_HZ)
#define SLP_ITV		(60*SYSTIM_HZ)

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

/* 自動実行 */

#if defined(TCP_ECHO_CLI_AUTO_RUN_STR)
#else
#if 0
#if defined(SUPPORT_INET6)
#define TCP_ECHO_CLI_AUTO_RUN_STR	"fd90:cce5:25f6:ff81:201:2ff:fe81:e7c9 - 0"
#else
#define TCP_ECHO_CLI_AUTO_RUN_STR	"172.25.129.140 - 0"
#endif
#endif
#endif

#if defined(SUPPORT_INET6)
#if defined(SUPPORT_INET4)
#define API_PROTO		'n'
#else
#define API_PROTO		'6'
#endif
#else
#define API_PROTO		'4'
#endif

/*
 *  全域変数
 */

bool_t	tcp_echo_cli_valid;
uint_t	tcp_echo_cli_scount;

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t tcp_echo_cli_swbuf[TCP_ECHO_CLI_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t tcp_echo_cli_rwbuf[TCP_ECHO_CLI_RWBUF_SIZE];
#endif

#ifdef USE_TCP_NON_BLOCKING

/*
 *  全域変数
 */

ER_UINT	tcp_echo_cli_nblk_error;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;

	tcp_echo_cli_nblk_error = *(ER*)p_parblk;

	switch (fncd) {

	case TFN_TCP_CON_CEP:
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_RCV_BUF:
		if (tcp_echo_cli_nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC%c:%02u CBN] error: %s",
			                   API_PROTO, cepid, itron_strerror(tcp_echo_cli_nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_CLS_CEP:
		if (tcp_echo_cli_nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC%c:%02u CBN] error: %s",
			                   API_PROTO, cepid, itron_strerror(tcp_echo_cli_nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_RCV_DAT:
		if (tcp_echo_cli_nblk_error < 0)
			syslog(LOG_NOTICE, "[TEC%c:%02u CBN] error: %s",
			                   API_PROTO, cepid, itron_strerror(tcp_echo_cli_nblk_error));
		syscall(sig_sem(SEM_TCP_ECHO_CLI_NBLK_READY));
		break;

	case TFN_TCP_SND_OOB:
		if (tcp_echo_cli_nblk_error > 0)
			syslog(LOG_NOTICE, "[TEC%c:%02u CBN] snd urg: %4u",
			                   API_PROTO, cepid, tcp_echo_cli_nblk_error);
		else
			syslog(LOG_NOTICE, "[TEC%c:%02u CBN] error: %s",
			                   API_PROTO, cepid, itron_strerror(tcp_echo_cli_nblk_error));
		break;

	case TFN_TCP_ACP_CEP:
	case TFN_TCP_SND_DAT:
	case TFN_TCP_GET_BUF:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  getcomd -- コマンドを得る。
 */

#ifdef TCP_ECHO_CLI_AUTO_RUN_STR

static char *
getcomd (ID cepid, bool_t retry, char *apip)
{
	ER		error;
	char		*line = NULL;
	static char	auto_run_str[] = TCP_ECHO_CLI_AUTO_RUN_STR;
	static int_t	count = 0;

	if (retry || (count == 0)) {
		line = auto_run_str;
		dly_tsk(3 * 1000);
		}
	else {
		while ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t*)&line)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", API_PROTO, cepid, itron_strerror(error));
			dly_tsk(SLP_ITV);
			}
		}

	count ++;
	return line;
	}

#else	/* of #ifdef TCP_ECHO_CLI_AUTO_RUN_STR */

static char *
getcomd (ID cepid, bool_t retry, char *apip)
{
	ER	error;
	char	*line = NULL;

	while ((error = rcv_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t*)&line)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", API_PROTO, cepid, itron_strerror(error));
		dly_tsk(SLP_ITV);
		}

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

	if (*line == '6') {
		line ++;
		*apip = API_PROTO_IPV6;
		}
	else if (*line == '4') {
		line ++;
		*apip = API_PROTO_IPV4;
		}
	else {
		*apip= DEFAULT_API_PROTO;
		if (*line && *line != ' ')
			line ++;
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	*apip= DEFAULT_API_PROTO;
	if (*line && *line != ' ')
		line ++;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	return line;
	}

#endif	/* of #ifdef TCP_ECHO_CLI_AUTO_RUN_STR */

/*
 *  TCP ECHO クライアント送信タスク
 */

/*
 *  IP アドレスの指定
 */

#define ADDR_IPVn	'n'
#define ADDR_IPV6	'6'
#define ADDR_IPV4	'4'

void
tcp_echo_cli_snd_task (intptr_t exinf)
{
	ID		tskid, cepid;
	ER		error;
	T_IN_ADDR	addr;
	char		*line;
	int_t		rep, count;
	bool_t		retry = false;
	uint16_t	portno;
	char		apip;

#ifdef USE_TCP_EXTENTIONS

	T_TCP_CCEP	ccep;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef TCP_ECHO_CLI_AUTO_RUN_STR
	dly_tsk(START_DLY);
#endif
	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP%c ECHO CLI SND:%d,%d] started.", DEFAULT_API_PROTO, tskid, (ID)exinf);
	while (true) {
		line = skip_blanks(getcomd((ID)exinf, retry, &apip));

#if defined(SUPPORT_INET6) && !defined(SUPPORT_INET4) && !defined(API_CFG_IP4MAPPED_ADDR)

		if (apip == API_PROTO_IPV4) {
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: E_PAR", API_PROTO, (ID)exinf);
			retry = true;
			continue;
			}

#endif	/* of #if defined(SUPPORT_INET6) && !defined(SUPPORT_INET4) && !defined(API_CFG_IP4MAPPED_ADDR) */

		if ((line = lookup_ipaddr(&addr, line, apip)) == NULL) {
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] sleep %d.%03u[s], unknown host.",
			       API_PROTO, (ID)exinf, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			dly_tsk(SLP_ITV);
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] resume.", API_PROTO, (ID)exinf);
			retry = true;
			continue;
			}
		else
			retry = false;

#if defined(SUPPORT_INET6)

#if defined(USE_TCP4_ECHO_CLI)

#else	/* of #if defined(USE_TCP4_ECHO_CLI) */

		if (in6_is_addr_ipv4mapped(&addr))
			apip = API_PROTO_IPV4;
		else
			apip = API_PROTO_IPV6;

#endif	/* of #if defined(USE_TCP4_ECHO_CLI) */

#else	/* of #if defined(SUPPORT_INET6) */

		if (apip == API_PROTO_IPV6) {
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: E_PAR", API_PROTO, (ID)exinf);
			retry = true;
			continue;
			}

#endif	/* of #if defined(SUPPORT_INET6) */

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9') {	/* Port No */
			line = get_int(&rep, line);
			portno = (uint16_t)rep;
			}
		else {
			line ++;
			portno = ECHO_SRV_PORTNO;
			}

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9')	/* Repeat */
			line = get_int(&rep, line);
		else {
			line ++;
			rep = 1;
			}

#ifdef USE_TCP_EXTENTIONS

		ccep.cepatr = 0;
		ccep.sbufsz = TCP_ECHO_CLI_SWBUF_SIZE;
		ccep.rbufsz = TCP_ECHO_CLI_RWBUF_SIZE;

#ifdef TCP_CFG_SWBUF_CSAVE
		ccep.sbuf = NADR;
#else
		ccep.sbuf = tcp_echo_cli_swbuf;
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
		ccep.rbuf = NADR;
#else
		ccep.rbuf = tcp_echo_cli_rwbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
		ccep.callback = (FP)callback_nblk_tcp_echo_cli;
#else
		ccep.callback = NULL;
#endif

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

		if (apip == API_PROTO_IPV6) {
			if ((error = alloc_tcp6_cep(&cepid, tskid, &ccep)) != E_OK) {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", apip, cepid, itron_strerror(error));
				continue;
				}
			}
		else {
			if ((error = alloc_tcp4_cep(&cepid, tskid, &ccep)) != E_OK) {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", apip, cepid, itron_strerror(error));
				continue;
				}
			}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

		if ((error = ALLOC_TCP_CEP(&cepid, tskid, &ccep)) != E_OK) {
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", apip, cepid, itron_strerror(error));
			continue;
			}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

#else	/* of #ifdef USE_TCP_EXTENTIONS */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

		if (apip == API_PROTO_IPV6)
			cepid = (ID)exinf;
		else
			cepid = TCP4_ECHO_CLI_CEPID;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

		cepid = (ID)exinf;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		tcp_echo_cli_valid = true;
		count = 0;
		while (rep == 0 || count < rep) {
			if (!tcp_echo_cli_valid) {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] canceled.", apip, cepid);
				break;
				}

			count ++;
			if (rep == 0) {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] start:       repeat: %d", apip, cepid, count);
				error = tcp_echo_cli_snd(cepid, &addr, portno, apip);
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] finsh:       repeat: %d", apip, cepid, count);
				}
			else {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] start:       repeat: %d/%d", apip, cepid, count, rep);
				error = tcp_echo_cli_snd(cepid, &addr, portno, apip);
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] finsh:       repeat: %d/%d", apip, cepid, count, rep);
				}

			if (error != E_OK) {
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] sleep %d.%03u[s], error: %s",
				                   apip, cepid, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ, itron_strerror(error));
				tslp_tsk(SLP_ITV);
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] resume.", apip, cepid);
				}

#if defined(SND_ITV)
#if SND_ITV > 0
			if (count < rep) {
				uint_t itv;
				
				itv = SND_ITV;
				syslog(LOG_NOTICE, "[TEC%c:%02u TSK] interval: %d[s].", apip, cepid, itv / SYSTIM_HZ);
				syscall(dly_tsk(SND_ITV));
				}
#endif
#endif
			}

#ifdef USE_TCP_EXTENTIONS

		if ((error = FREE_TCP_CEP(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TEC%c:%02u TSK] error: %s", apip, cepid, itron_strerror(error));

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		}
	}

#endif	/* of #ifdef USE_TCP_ECHO_CLI */
