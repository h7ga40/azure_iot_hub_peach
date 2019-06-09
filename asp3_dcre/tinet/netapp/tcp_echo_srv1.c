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

#if defined(_NET_CFG_BYTE_ORDER)
#error "net/net.h included."
#endif

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

/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t tcp_echo_srv_swbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t tcp_echo_srv_rwbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_RWBUF_SIZE];
#endif

#ifndef USE_TCP_NON_BLOCKING

/*
 *  変数
 */

#ifdef USE_TCP_EXTENTIONS

T_TCP_ECHO_SRV_INFO tcp_echo_srv_info[NUM_TCP_ECHO_SRV_TASKS];

#else	/* of #ifdef USE_TCP_EXTENTIONS */

T_TCP_ECHO_SRV_INFO tcp_echo_srv_info[] = {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV)

	{ TCP4_ECHO_SRV_CEPID1, TCP4_ECHO_SRV_REPID },

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

	{ TCP_ECHO_SRV_CEPID1, TCP_ECHO_SRV_REPID },

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

#if NUM_TCP_ECHO_SRV_TASKS >= 2
	{ TCP_ECHO_SRV_CEPID2, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 3
	{ TCP_ECHO_SRV_CEPID3, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 4
	{ TCP_ECHO_SRV_CEPID4, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 5
	{ TCP_ECHO_SRV_CEPID5, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 6
	{ TCP_ECHO_SRV_CEPID6, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 7
	{ TCP_ECHO_SRV_CEPID7, TCP_ECHO_SRV_REPID },
#endif
#if NUM_TCP_ECHO_SRV_TASKS >= 8
	{ TCP_ECHO_SRV_CEPID8, TCP_ECHO_SRV_REPID },
#endif

	};

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

/*
 *  tcp_passive_open -- 受動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

static ER
tcp_passive_open (T_TCP_ECHO_SRV_INFO *info, char apip)
{
	ER		error = E_OK;

#if TNUM_TCP4_REPID > 0
	T_IPV4EP	dst4;
#endif

	if (apip == API_PROTO_IPV6) {

#if TNUM_TCP6_REPID > 0

		/* 受付口は IPv6 */
		if ((error = tcp6_acp_cep(info->cepid, info->repid, &info->dst, TMO_FEVR)) != E_OK)
			return error;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp6_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #if TNUM_TCP6_REPID > 0 */

		}
	else {

#if TNUM_TCP4_REPID > 0

		/* 受付口は IPv4 */
		if ((error = tcp_acp_cep(info->cepid, info->repid, &dst4, TMO_FEVR)) != E_OK)
			return error;
		in6_make_ipv4mapped (&info->dst.ipaddr, dst4.ipaddr);
		info->dst.portno = dst4.portno;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp4_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #if TNUM_TCP4_REPID > 0 */

		}

	return error;
	}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  tcp_passive_open -- 受動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

static ER
tcp_passive_open (T_TCP_ECHO_SRV_INFO *info, char apip)
{
	ER		error = E_OK;

	if ((error = TCP_ACP_CEP(info->cepid, info->repid, &info->dst, TMO_FEVR)) != E_OK)
		return error;

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(info->repid, true)) != E_OK)
		return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	return error;
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

#ifdef USE_COPYSAVE_API

/*
 *  tcp_echo_srv -- TCP エコーサーバ
 *
 *    USE_TCP_NON_BLOCKING	OFF
 *    USE_COPYSAVE_API		ON
 */

ER
tcp_echo_srv (uint_t six, char apip)
{
	ER		error = E_OK;
	ER_UINT		rblen, sblen;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	rlen, slen, soff, scount, rcount;
	char		*rbuf, *sbuf;

#ifdef SHOW_RCV_RANGE
	char		head, tail;
#endif

	if ((error = tcp_passive_open(&tcp_echo_srv_info[six], apip)) != E_OK) {
		syslog(LOG_NOTICE, "[TES%c:%02u OPN] error: %s",
		                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
		return error;
		}

	syscall(get_tim(&now));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if (apip == API_PROTO_IPV6)
		syslog(LOG_NOTICE, "[TES6:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ipv62str(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
		                   tcp_echo_srv_info[six].dst.portno);
	else {
		T_IN4_ADDR	addr;

		addr = ntohl(tcp_echo_srv_info[six].dst.ipaddr.s6_addr32[3]);
		syslog(LOG_NOTICE, "[TES4:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ip2str(NULL, &addr), tcp_echo_srv_info[six].dst.portno);
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	syslog(LOG_NOTICE, "[TES%c:%02u ACP] conct: %7lu,from: %s.%u",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
	                   IP2STR(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
	                   tcp_echo_srv_info[six].dst.portno);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	scount = rcount = total = 0;
	while (true) {
		if ((rblen = tcp_rcv_buf(tcp_echo_srv_info[six].cepid, (void**)&rbuf, RCV_TMO)) <= 0) {
			if (rblen != E_OK)
				syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s",
				                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(rblen));
			break;
			}

		rcount ++;

#ifdef SHOW_RCV_RANGE
		head = *rbuf;
		tail = *(rbuf + rblen - 1);

		//syslog(LOG_NOTICE, "[TES%c:%02u RCV] len:   %7lu",
		//                   apip, tcp_echo_srv_info[six].cepid, (uint16_t)rblen);
		syslog(LOG_NOTICE, "[TES%c:%02u RCV] rcount: %7lu, len: %6lu, data %02x -> %02x",
		                   apip, tcp_echo_srv_info[six].cepid, rcount, (uint16_t)rblen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */

		rlen   = (uint16_t)rblen;
		total +=     rblen;
		soff = 0;
		while (rlen > 0) {

			if ((sblen = tcp_get_buf(tcp_echo_srv_info[six].cepid, (void**)&sbuf, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TES%c:%02u GET] error: %s",
				                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(sblen));
				goto err_fin;
				}

			//syslog(LOG_NOTICE, "[TES%c:%02u SND] len:   %7lu",
			//                   apip, tcp_echo_srv_info[six].cepid, (uint16_t)sblen);
			scount ++;
			slen = rlen < (uint16_t)sblen ? rlen : (uint16_t)sblen;
			memcpy(sbuf, rbuf + soff, slen);
			if ((error = tcp_snd_buf(tcp_echo_srv_info[six].cepid, slen)) != E_OK) {
				syslog(LOG_NOTICE, "[TES%c:%02u SND] error: %s",
				                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES%c:%02u SND] scount: %4u, len: %4u",
			                   apip, tcp_echo_srv_info[six].cepid, scount, slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}

		if ((error = tcp_rel_buf(tcp_echo_srv_info[six].cepid, rblen)) < 0) {
			syslog(LOG_NOTICE, "[TES%c:%02u REL] error: %s",
			                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
			break;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(tcp_echo_srv_info[six].cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u SHT] error: %s",
		                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(tcp_echo_srv_info[six].cepid, CLS_TMO)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u CLS] error: %s",
		                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));

	syscall(get_tim(&now));
#if 1
	syslog(LOG_NOTICE, "[TES%c:%02u FIN] finsh: %7lu, ttl: %lu",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ, total);
#else
	syslog(LOG_NOTICE, "[TES%c:%02u FIN] finsh: %7lu, ttl: %lu, error: %s",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ, total, itron_strerror(error));
#endif

	return error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

/*
 *  tcp_echo_srv -- TCP エコーサーバ
 *
 *    USE_TCP_NON_BLOCKING	OFF
 *    USE_COPYSAVE_API		OFF
 */

ER
tcp_echo_srv (uint_t six, char apip)
{
	ID		cepid;
	ER		error = E_OK;
	ER_UINT		rlen, slen;
	SYSTIM		now;
	uint32_t	total;
	uint16_t	soff, scount, rcount;

	if ((error = tcp_passive_open(&tcp_echo_srv_info[six], apip)) != E_OK) {
		syslog(LOG_NOTICE, "[TES%c:%02u OPN] error: %s",
		                   apip, tcp_echo_srv_info[six].cepid, itron_strerror(error));
		return error;
		}
	
	syscall(get_tim(&now));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if (apip == API_PROTO_IPV6)
		syslog(LOG_NOTICE, "[TES6:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ipv62str(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
		                   tcp_echo_srv_info[six].dst.portno);
	else	{
		T_IN4_ADDR	addr;

		addr = ntohl(tcp_echo_srv_info[six].dst.ipaddr.s6_addr32[3]);
		syslog(LOG_NOTICE, "[TES4:%02u ACP] conct: %7lu,from: %s.%u",
		                   tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ip2str(NULL, &addr), tcp_echo_srv_info[six].dst.portno);
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	syslog(LOG_NOTICE, "[TES%c:%02u ACP] conct: %7lu,from: %s.%u",
	                   apip, tcp_echo_srv_info[six].cepid, now / SYSTIM_HZ,
	                   IP2STR(NULL, &tcp_echo_srv_info[six].dst.ipaddr),
	                   tcp_echo_srv_info[six].dst.portno);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	cepid =  tcp_echo_srv_info[six].cepid;
	scount = rcount = total = 0;
	while (true) {
		if ((rlen = tcp_rcv_dat(cepid, tcp_echo_srv_info[six].buffer, BUF_SIZE - 1, RCV_TMO)) <= 0) {
			if (rlen != E_OK)
				syslog(LOG_NOTICE, "[TES%c:%02u RCV] error: %s",
				                   apip, cepid, itron_strerror(rlen));
			break;
			}

		rcount ++;
#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES%c:%02u RCV] rcount: %4u, len: %4u, data %02x -> %02x",
		                   apip, cepid, rcount,
		                   (uint16_t)rlen, * tcp_echo_srv_info[six].tcp_echo_srv_info[six].buffer,
		                                   *(tcp_echo_srv_info[six].tcp_echo_srv_info[six].buffer + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		soff = 0;
		while (rlen > 0) {
			scount ++;
			if ((slen = tcp_snd_dat(cepid, &tcp_echo_srv_info[six].buffer[soff], rlen, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TES%c:%02u SND] error: %s",
				                   apip, cepid, itron_strerror(slen));
				goto err_fin;
				}
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES%c:%02u SND] scount: %4u, len: %4u",
			                   apip, cepid, scount, (uint16_t)slen);
#endif	/* of #ifdef SHOW_RCV_RANGE */

			rlen -= slen;
			soff += slen;
			}
		}
err_fin:

#ifdef USE_TCP_SHT_CEP
	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u SHT] error: %s", 
		                    apip, cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

	if ((error = tcp_cls_cep(cepid, CLS_TMO)) != E_OK)
		syslog(LOG_NOTICE, "[TES%c:%02u CLS] error: %s",
		                   apip, cepid, itron_strerror(error));

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES%c:%02u FIN] finsh: %7lu, ttl: %lu",
	                   apip, cepid, now / SYSTIM_HZ, total);

	return error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#endif	/* of #ifndef USE_TCP_NON_BLOCKING */

#ifndef USE_TCP_EXTENTIONS

/*
 *  tcp_echo_srv_task -- TCP エコーサーバタスク
 */

void
tcp_echo_srv_task(intptr_t exinf)
{
	ID	tskid, cepid;
	ER	error;
	uint_t	six;
	char	apip;

	six = INDEX_SRV_INFO((ID)exinf);

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV)

	if (six >= NUM_TCP4_ECHO_SRV_TASKS)
		apip = API_PROTO_IPV6;
	else
		apip = API_PROTO_IPV4;

#else	/* of #if defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

	apip = API_PROTO_IPV6;

#endif	/* of #if defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_SRV) */

#else	/* of #if defined(SUPPORT_INET6) */

	apip = API_PROTO_IPV4;

#endif	/* of #if defined(SUPPORT_INET6) */

	syscall(get_tid(&tskid));
	cepid = tcp_echo_srv_info[six].cepid;
	syslog(LOG_NOTICE, "[TCP%c ECHO SRV:%d,%d] started.", apip, tskid, cepid);

	while (true) {
		while ((error = tcp_echo_srv(six, apip)) == E_OK)
			;
		syslog(LOG_NOTICE, "[TES%c:%02u TSK] sleep 60[s], error: %s", apip, cepid, itron_strerror(error));
		tslp_tsk(60 * 1000 * 1000);
		syslog(LOG_NOTICE, "[TES%c:%02u TSK] resume.", apip, cepid);
		}
	}

#endif	/* of #ifndef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_ECHO_SRV1 */
