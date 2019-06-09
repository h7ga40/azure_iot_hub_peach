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
 *  TCP DISCARD クライアント
 *
 *    ・ノンブロッキングコール
 *    ・IPv4
 *    ・IPv6
 */

#include <stdlib.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/tcp_discard_cli.h>

#ifdef USE_TCP_DISCARD_CLI

/* discard サーバのポート番号 */

#define DISCARD_SRV_PORTNO	UINT_C(9)

/* 表示 */

/*#define SHOW_RCV_RANGE*/

/* 送信間隔 */

#ifdef TOPPERS_S810_CLG3_85

#define START_DLY	(10*SYSTIM_HZ)
#define SND_ITV		(5*SYSTIM_HZ)
#define SLP_ITV		(30*SYSTIM_HZ)

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

#define START_DLY	(10*SYSTIM_HZ)
#define SND_ITV		(5*SYSTIM_HZ)
#define SLP_ITV		(60*SYSTIM_HZ)

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

/* 自動実行 */

#if defined(TCP_DISCARD_CLI_AUTO_RUN_STR)
#else
#if 0
#if defined(SUPPORT_INET6)
#define TCP_DISCARD_CLI_AUTO_RUN_STR	"fd90:cce5:25f6:ff81:201:2ff:fe81:e7c9 - 0"
#else
#define TCP_DISCARD_CLI_AUTO_RUN_STR	"172.25.129.142 - 0"
#endif
#endif
#endif

/*
 *  バッファサイズの定義
 */

#define NUM_DISCARD		5
#define NUM_REP_PAT		40
#define PAT_BEGIN		' '
#define PAT_END			'~'
#define SND_BUF_SIZE		((PAT_END - PAT_BEGIN + 1) * NUM_REP_PAT)

/*
 *  全域変数
 */

bool_t tcp_discard_cli_valid;

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE_ONLY
uint8_t tcp_discard_cli_swbuf[TCP_DISCARD_CLI_SWBUF_SIZE];
#endif

#ifdef USE_TCP_NON_BLOCKING

/*
 *  変数
 */

static T_IPEP		nblk_src = {
				IP_ADDRANY,
				TCP_PORTANY,
				};
static T_IPEP		nblk_dst;
static ER_UINT		nblk_error;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_discard_cli (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;

	nblk_error = *(ER*)p_parblk;

	switch (fncd) {

	case TFN_TCP_CON_CEP:
		syscall(sig_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		break;

	case TFN_TCP_CLS_CEP:
		if (nblk_error < 0)
			syslog(LOG_NOTICE, "[TDCn:%02u CBN] close error: %s", cepid, itron_strerror(nblk_error));
		syscall(sig_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		break;

	case TFN_TCP_ACP_CEP:
	case TFN_TCP_SND_DAT:
	case TFN_TCP_GET_BUF:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

/*
 *  send_tcp_discard -- DISCARD/TCP サーバにメッセージを送信する。
 */

static ER
send_tcp_discard (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[SND_BUF_SIZE];

	ER_UINT		slen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	soff, discard, rep, scount;
	uint8_t		pat, *p;

	nblk_dst.ipaddr = *ipaddr;
	nblk_dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &nblk_src, &nblk_dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TDCn:%02u SND] connect error: %s", cepid, itron_strerror(error));
		return error;
		}

	/* 接続が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));

	if (nblk_error != E_OK)
		syslog(LOG_NOTICE, "[TDCn:%02u SND] connect error: %s", cepid, itron_strerror(nblk_error));
	else {
		get_tim(&time);
		syslog(LOG_NOTICE, "[TDCn:%02u SND] conct: %7lu, to:   %s.%d",
		                   cepid, time / SYSTIM_HZ, IP2STR(NULL, ipaddr), nblk_dst.portno);

		scount = total = 0;
		for (discard = NUM_DISCARD; discard -- > 0; ) {
			soff = 0;
			while (soff < SND_BUF_SIZE) {
				if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, TMO_FEVR)) < 0) {
					syslog(LOG_NOTICE, "[TDCn:%02u SND] send error: %s",
					                   cepid, itron_strerror(slen));
					goto cls_ret;
					}
				soff  += (uint16_t)slen;
				total +=     slen;
				scount ++;

#ifdef SHOW_RCV_RANGE
				syslog(LOG_NOTICE, "[TDCn:%02u SND] send:  %7lu, len: %4u, off: %4u",
				                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

				syscall(dly_tsk(500 + netapp_rand() % SYSTIM_HZ));
				}
			}

	cls_ret:
		get_tim(&time);
		syslog(LOG_NOTICE, "[TDCn:%02u SND] finsh: %7lu, ttl: %lu",
		                   cepid, time / SYSTIM_HZ, scount, total);

		if ((error = tcp_sht_cep(cepid)) < 0)
			syslog(LOG_NOTICE, "[TDCn:%02u SND] shutdown error: %s", cepid, itron_strerror(error));

		if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK)
			syslog(LOG_NOTICE, "[TDCn:%02u SND] close error: %s", cepid, itron_strerror(error));

		/* 開放が完了するまで待つ。*/
		syscall(wai_sem(SEM_TCP_DISCARD_CLI_NBLK_READY));
		}

	return E_OK;
	}

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  send_tcp_discard -- DISCARD/TCP サーバにメッセージを送信する。
 */

static ER
send_tcp_discard (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno)
{
	static char smsg[SND_BUF_SIZE];
	static T_IPEP src = {
		IP_ADDRANY,
		TCP_PORTANY,
		};

	T_IPEP		dst;
	ER_UINT		slen;
	ER		error;
	SYSTIM		time;
	uint16_t	pat, soff, discard, rep, scount, total;
	uint8_t		*p;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

	if ((error = TCP_CON_CEP(cepid, &src, &dst, 60 * 1000)) != E_OK) {
		syslog(LOG_NOTICE, "[TDCn:%02u SND] connect error: %s", cepid, itron_strerror(error));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDCn:%02u SND] conct: %7lu, to:   %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);

	scount = total = 0;
	for (discard = NUM_DISCARD; discard -- > 0; ) {
		soff = 0;
		while (soff < SND_BUF_SIZE) {
			if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, TMO_FEVR)) < 0) {
				syslog(LOG_NOTICE, "[TDCn:%02u SND] send error: %s", cepid, itron_strerror(slen));
				goto cls_ret;
				}
			soff  += (uint16_t)slen;
			total +=     slen;
			scount ++;

#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TDCn:%02u SND] send:  %7lu, len: %4u, off: %4u",
			                   cepid, scount, (uint16_t)slen, soff);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			syscall(dly_tsk(500 + netapp_rand() % SYSTIM_HZ));
			}
		}

cls_ret:
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDCn:%02u SND] finsh: %7lu, ttl: %lu",
	                   cepid, time / SYSTIM_HZ, scount, total);

	if ((error = tcp_sht_cep(cepid)) < 0)
		syslog(LOG_NOTICE, "[TDCn:%02u SND] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) < 0)
		syslog(LOG_NOTICE, "[TDCn:%02u SND] close error: %s", cepid, itron_strerror(error));

	return E_OK;
	}

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  getcomd -- コマンドを得る。
 */

#ifdef TCP_DISCARD_CLI_AUTO_RUN_STR

static char *
getcomd (ID cepid, bool_t retry)
{
	ER		error;
	char		*line = NULL;
	static char	auto_run_str[] = TCP_DISCARD_CLI_AUTO_RUN_STR;
	static int_t	count = 0;

	if (retry || (count == 0)) {
		line = auto_run_str;
		dly_tsk(3 * 1000);
		}
	else {
		while ((error = rcv_dtq(DTQ_TCP_DISCARD_CLI, (intptr_t*)&line)) != E_OK) {
			syslog(LOG_NOTICE, "[TDCn:%02u TSK] error: %s", cepid, itron_strerror(error));
			dly_tsk(SLP_ITV);
			}
		}

	count ++;
	return line;
	}

#else	/* of #ifdef TCP_DISCARD_CLI_AUTO_RUN_STR */

static char *
getcomd (ID cepid, bool_t retry)
{
	ER	error;
	char	*line = NULL;

	while ((error = rcv_dtq(DTQ_TCP_DISCARD_CLI, (intptr_t*)&line)) != E_OK) {
		syslog(LOG_NOTICE, "[TDCn:%02u TSK] error: %s", cepid, itron_strerror(error));
		dly_tsk(SLP_ITV);
		}
	return line;
	}

#endif	/* of #ifdef TCP_DISCARD_CLI_AUTO_RUN_STR */

void
tcp_discard_cli_task (intptr_t exinf)
{
	ID		tskid, cepid;
	T_IN_ADDR	addr;
	ER		error;
	uint8_t		*line;
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

#if defined(SUPPORT_INET6)
	apip = API_PROTO_IPV6;
#if defined(SUPPORT_INET4)
	syslog(LOG_NOTICE, "[TCPn DISCARD CLI:%d,%d] started.", tskid, (ID)exinf);
#else
	syslog(LOG_NOTICE, "[TCP6 DISCARD CLI:%d,%d] started.", tskid, (ID)exinf);
#endif
#else
	apip = API_PROTO_IPV4;
	syslog(LOG_NOTICE, "[TCP4 DISCARD CLI:%d,%d] started.", tskid, (ID)exinf);
#endif

	while (true) {
		line = skip_blanks(getcomd((ID)exinf, retry));

		if ((line = lookup_ipaddr(&addr, line, apip)) == NULL) {
			syslog(LOG_NOTICE, "[TDCn:%02u TSK] sleep %d.%03u[s], unknown host.",
			       (ID)exinf, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			dly_tsk(SLP_ITV);
			syslog(LOG_NOTICE, "[TDCn:%02u TSK] resume.", (ID)exinf);
			retry = true;
			continue;
			}
		else
			retry = false;

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9') {	/* Port No */
			line = get_int(&rep, line);
			portno = (uint16_t)rep;
			}
		else {
			line ++;
			portno = DISCARD_SRV_PORTNO;
			}

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9')	/* Repeat */
			line = get_int(&rep, line);
		else
			rep = 1;

#ifdef USE_TCP_EXTENTIONS

		ccep.cepatr = 0;
		ccep.sbufsz = TCP_DISCARD_CLI_SWBUF_SIZE;
		ccep.rbufsz = 0;
		ccep.rbuf = NADR;

#ifdef TCP_CFG_SWBUF_CSAVE
		ccep.sbuf = NADR;
#else
		ccep.sbuf = tcp_discard_cli_swbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
		ccep.callback = (FP)callback_nblk_tcp_discard_cli;
#else
		ccep.callback = NULL;
#endif

		if ((error = ALLOC_TCP_CEP(&cepid, tskid, &ccep)) != E_OK) {
			syslog(LOG_NOTICE, "[TDCn:%02u TSK] CEP create error: %s", cepid, itron_strerror(error));
			continue;
			}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

		cepid = (ID)exinf;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		tcp_discard_cli_valid = true;
		count = 0;
		while (rep == 0 || count < rep) {
			if (!tcp_discard_cli_valid) {
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] canceled.", cepid);
				break;
				}

			count ++;
			if (rep == 0) {
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] start:       repeat: %d", cepid, count);
				error = send_tcp_discard(cepid, &addr, portno);
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] finsh:       repeat: %d", cepid, count);
				}
			else {
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] start:       repeat: %d/%d", cepid, count, rep);
				error = send_tcp_discard(cepid, &addr, portno);
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] finsh:       repeat: %d/%d", cepid, count, rep);
				}

			if (error != E_OK) {
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] sleep %d.%03u[s], error: %s",
				                   cepid, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ, itron_strerror(error));
				tslp_tsk(SLP_ITV);
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] resume.", cepid);
				}

#if defined(SND_ITV)
#if SND_ITV > 0
			if (count < rep) {
				uint_t itv;
				
				itv = SND_ITV;
				syslog(LOG_NOTICE, "[TDCn:%02u TSK] interval: %d[s].", cepid, itv / SYSTIM_HZ);
				syscall(dly_tsk(SND_ITV));
				}
#endif
#endif
			}

#ifdef USE_TCP_EXTENTIONS

		if ((error = FREE_TCP_CEP(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TDCn:%02u TSK] CEP delete error: %s", cepid, itron_strerror(error));

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		}
	}

#endif	/* of #ifdef USE_TCP_DISCARD_CLI */
