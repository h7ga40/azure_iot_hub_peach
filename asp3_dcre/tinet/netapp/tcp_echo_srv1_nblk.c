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

#ifdef USE_TCP_NON_BLOCKING

/*
 *  変数
 */

#ifdef USE_TCP_EXTENTIONS

T_TCP_ECHO_SRV_INFO tcp_echo_srv_info[] = {

	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY1 },

#if NUM_TCP_ECHO_SRV_TASKS >= 2
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY2 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 3
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY3 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 4
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY4 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 5
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY5 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 6
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY6 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 7
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY7 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 8
	{ 0, 0, SEM_TCP_ECHO_SRV_NBLK_READY8 },
#endif

	};

#else	/* of #ifdef USE_TCP_EXTENTIONS */

T_TCP_ECHO_SRV_INFO tcp_echo_srv_info[] = {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV)

	{ TCP4_ECHO_SRV_CEPID1, TCP4_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY1 },

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

	{ TCP_ECHO_SRV_CEPID1, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY1 },

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

#if NUM_TCP_ECHO_SRV_TASKS >= 2
	{ TCP_ECHO_SRV_CEPID2, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY2 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 3
	{ TCP_ECHO_SRV_CEPID3, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY3 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 4
	{ TCP_ECHO_SRV_CEPID4, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY4 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 5
	{ TCP_ECHO_SRV_CEPID5, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY5 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 6
	{ TCP_ECHO_SRV_CEPID6, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY6 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 7
	{ TCP_ECHO_SRV_CEPID7, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY7 },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 8
	{ TCP_ECHO_SRV_CEPID8, TCP_ECHO_SRV_REPID, SEM_TCP_ECHO_SRV_NBLK_READY8 },
#endif

	};

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

/*
 *  find_srv_info -- 接続相手の情報構造体を探索する。
 */

static 
T_TCP_ECHO_SRV_INFO *find_srv_info (ID cepid)
{
	T_TCP_ECHO_SRV_INFO	*ri;

	for (ri = &tcp_echo_srv_info[sizeof(tcp_echo_srv_info) / sizeof(T_TCP_ECHO_SRV_INFO)]; ri --; )
		if (ri->cepid == cepid)
			return ri;
	return NULL;
	}

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_echo_srv (ID cepid, FN fncd, void *p_parblk)
{
	ER		error = E_OK;
	T_TCP_ECHO_SRV_INFO	*ri;

	if ((ri = find_srv_info(cepid)) == NULL) {
		syslog(LOG_NOTICE, "[TESn:%02u CBN] CEPID not found", cepid);
		return E_PAR;
		}

	switch (fncd) {

	case TFN_TCP_ACP_CEP:
		ri->error = *(ER*)p_parblk;
		syscall(sig_sem(ri->semid));
		break;

	case TFN_TCP_RCV_DAT:
		if ((ri->rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u CBN] error: %s", cepid, itron_strerror(ri->rlen));
		syscall(sig_sem(ri->semid));
		break;

	case TFN_TCP_SND_DAT:
		if ((ri->slen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u CBN] error: %s", cepid, itron_strerror(ri->slen));
		syscall(sig_sem(ri->semid));
		break;

	case TFN_TCP_CLS_CEP:
		if ((ri->error = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u CBN] error: %s", cepid, itron_strerror(ri->error));
		syscall(sig_sem(ri->semid));
		break;

	case TFN_TCP_RCV_BUF:
		if ((ri->rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u CBN] error: %s", cepid, itron_strerror(ri->rlen));
		syscall(sig_sem(ri->semid));
		break;

	case TFN_TCP_GET_BUF:
		if ((ri->slen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u CBN] error: %s", cepid, itron_strerror(ri->slen));
		syscall(sig_sem(ri->semid));
		break;

#ifdef USE_TCP_EXTENTIONS

	case TEV_TCP_RCV_OOB:
		if ((ri->rlen = *(ER*)p_parblk) < 0)
			syslog(LOG_NOTICE, "[TESn:%02u OOB] error: %s", cepid, itron_strerror(ri->rlen));
		else if (ri->rlen > 0) {
			char ch;

			if ((ri->rlen = tcp_rcv_oob(cepid, &ch, sizeof(ch))) > 0)
				syslog(LOG_NOTICE, "[TESn:%02u OOB] recv oob: 0x%02x", cepid, ch);
			else if (ri->rlen < 0)
				syslog(LOG_NOTICE, "[TESn:%02u OOB] error: %s", cepid, itron_strerror(ri->rlen));
			}
		break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	case TFN_TCP_CON_CEP:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

/*
 *  tcp_passive_open -- 受動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	ON
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

static ER
tcp_passive_open (T_TCP_ECHO_SRV_INFO *info, char apip)
{
	ER		error;
	T_IPV4EP	dst4;

	if (apip == API_PROTO_IPV6) {
		/* 受付口は IPv6 */
		if ((error = tcp6_acp_cep(info->cepid, info->repid, &info->dst, TMO_NBLK)) != E_WBLK)
			return error;

		/* 相手から接続されるまで待つ。*/
		syscall(wai_sem(info->semid));

		if (info->error != E_OK)
			return info->error;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp6_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		}
	else {
		/* 受付口は IPv4 */
		if ((error = tcp_acp_cep(info->cepid, info->repid, &dst4, TMO_NBLK)) != E_WBLK)
			return error;

		/* 相手から接続されるまで待つ。*/
		syscall(wai_sem(info->semid));

		if (info->error != E_OK)
			return info->error;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp4_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		in6_make_ipv4mapped (&info->dst.ipaddr, dst4.ipaddr);
		info->dst.portno = dst4.portno;

		}

	return E_OK;
	}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

static ER
tcp_passive_open (T_TCP_ECHO_SRV_INFO *info, char apip)
{
	ER		error;

	if ((error = TCP_ACP_CEP(info->cepid, info->repid, &info->dst, TMO_NBLK)) != E_WBLK)
		return error;

	/* 相手から接続されるまで待つ。*/
	syscall(wai_sem(info->semid));

	if (info->error != E_OK)
		return info->error;

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(info->repid, true)) != E_OK)
		return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	return E_OK;
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

#ifdef USE_COPYSAVE_API

/*
 *  tcp_echo_srv -- TCP エコーサーバ
 *
 *    USE_TCP_NON_BLOCKING	ON
 *    USE_COPYSAVE_API		ON
 */

ER
tcp_echo_srv (uint_t six, char apip)
{
#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	T_IN4_ADDR	ipv4addr;
#endif
	ER		error;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	rblen, sblen, rlen, slen, soff, scount, rcount;
	char		*rbuf, *sbuf, head, tail;

	if ((error = tcp_passive_open(&tcp_echo_srv_info[six], apip)) != E_OK)
		return error;

	syscall(get_tim(&now));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if (apip == API_PROTO_IPV6)
		syslog(LOG_NOTICE, "[TES6:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ipv62str(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
		                   tcp_echo_srv_info[six].dst.portno);
	else {
		ipv4addr = ntohl(tcp_echo_srv_info[six].dst.ipaddr.s6_addr32[3]);
		syslog(LOG_NOTICE, "[TES4:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ip2str(NULL, &ipv4addr),
		                   tcp_echo_srv_info[six].dst.portno);
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	syslog(LOG_NOTICE, "[TES%c:%02u ACP] conct: %7lu,from: %s.%u",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
	                   IP2STR(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
	                   tcp_echo_srv_info[six].dst.portno);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	rlen = scount = rcount = total = 0;
	while (true) {
		if ((error = tcp_rcv_buf(tcp_echo_srv_info[six].cepid, (void**)&rbuf, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s",
	                                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
			break;
			}

		/* 受信するまで待つ。*/
		syscall(wai_sem(tcp_echo_srv_info[six].semid));

		if (tcp_echo_srv_info[six].rlen < 0) {	/* エラー */
			syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s", 
	                                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(tcp_echo_srv_info[six].rlen));
			break;
			}
		else if (tcp_echo_srv_info[six].rlen == 0)	/* 受信終了 */
			break;

		rblen = tcp_echo_srv_info[six].rlen;

		/* バッファの残りにより、受信長を調整する。*/
		if (rblen > BUF_SIZE - rlen)
			rblen = BUF_SIZE - rlen;
		total += rblen;
		rlen   = rblen;

		head = *rbuf;
		tail = *(rbuf + rblen - 1);
		rcount ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES%c:%02u RCV] "
		                   "rcount: %7lu, len: %6lu, data %02x -> %02x",
	                           apip, tcp_echo_srv_info[six].cepid, rcount, rblen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		memcpy(tcp_echo_srv_info[six].buffer, rbuf, rblen);

		if ((error = tcp_rel_buf(tcp_echo_srv_info[six].cepid, rlen)) < 0) {
			syslog(LOG_NOTICE, "[TES%c:%02u REL] error: %s",
	                                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
			break;
			}

		soff = 0;
		while (rlen > 0) {

			if ((error = tcp_get_buf(tcp_echo_srv_info[six].cepid, (void**)&sbuf, TMO_NBLK)) != E_WBLK) {
				syslog(LOG_NOTICE, "[TES%c:%02u GET] error: %s",
	                                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
				goto err_fin;
				}

			/* 送信バッファの獲得が完了するまで待つ。*/
			syscall(wai_sem(tcp_echo_srv_info[six].semid));

			if (tcp_echo_srv_info[six].slen < 0) {
				syslog(LOG_NOTICE, "[TES%c:%02u GET] error: %s",
	                                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(tcp_echo_srv_info[six].slen));
				goto err_fin;
				}

			sblen = tcp_echo_srv_info[six].slen;
			scount ++;
			slen = sblen < rlen ? sblen : rlen;
			memcpy(sbuf, tcp_echo_srv_info[six].buffer + soff, slen);

			if ((error = tcp_snd_buf(tcp_echo_srv_info[six].cepid, slen)) != E_OK) {
				syslog(LOG_NOTICE, "[TES%c:%02u SND] error: %s",
	                                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES%c:%02u SND] scount: %7lu, len: %4u",
	                                   apip, cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(tcp_echo_srv_info[six].cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u SHT] error: %s",
	                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(tcp_echo_srv_info[six].cepid, TMO_NBLK)) != E_WBLK)
		syslog(LOG_NOTICE, "[TES%c:%02u CLS] error: %s",
	                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));

	/* 開放が完了するまで待つ。*/
	syscall(wai_sem(tcp_echo_srv_info[six].semid));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES%c:%02u FIN] finsh: %7lu, ttl: %lu",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ, total);

	return error == E_WBLK ? E_OK : error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

/*
 *  tcp_echo_srv -- TCP エコーサーバ
 *
 *    USE_TCP_NON_BLOCKING	ON
 *    USE_COPYSAVE_API		OFF
 */

ER
tcp_echo_srv (uint_t six, char apip)
{
	T_IN4_ADDR	ipv4addr;
	ER		error;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	rlen, slen, soff, scount, rcount;
	char		head, tail;

	if ((error = tcp_passive_open(&tcp_echo_srv_info[six], apip)) != E_OK)
		return error;

	syscall(get_tim(&now));
	if (apip == API_PROTO_IPV6)
		syslog(LOG_NOTICE, "[TES6:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ipv62str(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
		                   tcp_echo_srv_info[six].dst.portno);
	else {
		ipv4addr = ntohl(tcp_echo_srv_info[six].dst.ipaddr.s6_addr32[3]);
		syslog(LOG_NOTICE, "[TES4:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ip2str(NULL, &ipv4addr),
		                   tcp_echo_srv_info[six].dst.portno);
		}

	scount = rcount = total = 0;
	while (true) {
		if ((error = tcp_rcv_dat(tcp_echo_srv_info[six].cepid, tcp_echo_srv_info[six].buffer, BUF_SIZE - 1, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s",
	                                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
			break;
			}

		/* 受信完了まで待つ。*/
		syscall(wai_sem(tcp_echo_srv_info[six].semid));

		if (tcp_echo_srv_info[six].rlen < 0) {
			syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s",
	                                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(tcp_echo_srv_info[six].rlen));
			break;
			}
		else if (tcp_echo_srv_info[six].rlen == 0)
			break;

		rlen = tcp_echo_srv_info[six].rlen;
		head = * tcp_echo_srv_info[six].buffer;
		tail = *(tcp_echo_srv_info[six].buffer + rlen - 1);
		rcount ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES%c:%02u RCV] rcount: %4u, len: %4u, data %02x -> %02x",
	                           apip, tcp_echo_srv_info[six].cepid, rcount, rlen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		soff = 0;
		while (rlen > 0) {
			scount ++;
			if ((error = tcp_snd_dat(tcp_echo_srv_info[six].cepid, &tcp_echo_srv_info[six].buffer[soff], rlen, TMO_NBLK)) != E_WBLK) {
				syslog(LOG_NOTICE, "[TES%c:%02u SND] error: %s",
	                                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
				goto err_fin;
				}

			/* 送信完了まで待つ。*/
			syscall(wai_sem(tcp_echo_srv_info[six].semid));

			if (tcp_echo_srv_info[six].slen < 0) {
				syslog(LOG_NOTICE, "[TES%c:%02u SND] error: %s",
	                                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(tcp_echo_srv_info[six].slen));
				goto err_fin;
				}

			slen = tcp_echo_srv_info[six].slen;

#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES%c:%02u SND] scount: %4u, len: %4u",
	                                   apip, tcp_echo_srv_info[six].cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(tcp_echo_srv_info[six].cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u SHT] error: %s",
	                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(tcp_echo_srv_info[six].cepid, TMO_NBLK)) != E_WBLK)
		syslog(LOG_NOTICE, "[TES%c:%02u CLS] error: %s",
	                           apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));

	/* 開放が完了するまで待つ。*/
	syscall(wai_sem(tcp_echo_srv_info[six].semid));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES%c:%02u FIN] finsh: %7lu, ttl: %lu",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ, total);

	return error == E_WBLK ? E_OK : error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#endif	/* of #ifdef USE_TCP_ECHO_SRV1 */
