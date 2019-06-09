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
 *  DISCARD サーバ
 *
 *    ・省コピー API
 *    ・IPv4
 *    ・IPv6
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/tcp_discard_srv.h>

#ifdef USE_TCP_DISCARD_SRV

/*
 *  表示
 */

//#define SHOW_RCV_RANGE
//#define SHOW_RCV_DATA

/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_RWBUF_CSAVE_ONLY
uint8_t tcp_discard_srv_rwbuf[TCP_DISCARD_SRV_RWBUF_SIZE];
#endif

/*
 *  TCP DISCARD サーバタスク
 */

#define BUF_SIZE	TCP_DISCARD_SRV_RWBUF_SIZE

#ifdef USE_COPYSAVE_API

static ER
tcp_discard_srv (ID cepid, ID repid)
{
	T_IPEP		dst;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	count;
	uint8_t		*buf;

#ifdef SHOW_RCV_DATA
	uint_t	ix;
#endif	/* of #ifdef SHOW_RCV_DATA */

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TDSn:%02u ACP] accept error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TDSn:%02u DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	count = total = 0;
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDSn:%02u RCV] conct: %7lu, from: %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while ((rlen = tcp_rcv_buf(cepid, (void*)&buf, TMO_FEVR)) > 0) {
		count ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] count: %6lu, len: %4u, data: %02x -> %02x",
		       cepid, count, (uint16_t)rlen, *buf, *(buf + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

#ifdef SHOW_RCV_DATA
		for (ix = 0; ix < rlen; ix ++)
			cons_putchar(CONSOLE_PORTID,  *(buf + ix));
#endif	/* of #ifdef SHOW_RCV_DATA */

		if ((error = tcp_rel_buf(cepid, rlen)) != E_OK) {
			syslog(LOG_NOTICE, "[TDSn:%02u RCV] rel buf error: %s",
			                    cepid, itron_strerror(error));
			rlen = 0;
			break;
			}
		total += rlen;
		}

	if (rlen != 0)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] recv buf error: %s", cepid, itron_strerror(rlen));

	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) != E_OK)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] close error: %s", cepid, itron_strerror(error));

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDSn:%02u RCV] finsh: %7lu, ttl: %lu",
	                   cepid, time / SYSTIM_HZ, count, total);

	return error;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

static ER
tcp_discard_srv (ID cepid, ID repid)
{
	static char buffer[BUF_SIZE];

	T_IPEP		dst;
	ER_UINT		rlen;
	ER		error;
	SYSTIM		time;
	uint32_t	total;
	uint16_t	count;
	uint8_t		*buf = buffer;

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] accept error: %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TDSn:%02u DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	count = total = 0;
	get_tim(&time);
	syslog(LOG_NOTICE, "[TDSn:%02u RCV] conct: %7lu, from: %s.%d",
	                   cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	while ((rlen = tcp_rcv_dat(cepid, buf, BUF_SIZE - 1, TMO_FEVR)) > 0) {
		count ++;

#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] count: %6lu, len: %4u, data: %02x -> %02x",
		       cepid, count, (uint16_t)rlen, *buf, *(buf + rlen - 1));
#endif	/* of #ifdef SHOW_RCV_RANGE */

		total += rlen;
		}
	if (rlen != 0)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] recv error: %s", cepid, itron_strerror(rlen));

	if ((error = tcp_sht_cep(cepid)) != E_OK)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] shutdown error: %s", cepid, itron_strerror(error));

	if ((error = tcp_cls_cep(cepid, TMO_FEVR)) != E_OK)
		syslog(LOG_NOTICE, "[TDSn:%02u RCV] close error: %s", cepid, itron_strerror(error));

	get_tim(&time);
	syslog(LOG_NOTICE, "[TDSn:%02u RCV] finsh: %7lu, ttl: %lu",
	                   cepid, time / SYSTIM_HZ, count, total);

	return error;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

#ifdef USE_TCP_EXTENTIONS

/*
 *  get_tcp_rep -- TCP 受付口を獲得する。
 */

static ER
get_tcp_rep (ID *repid)
{
	ID		tskid;
	T_TCPN_CREP	crep;

	get_tid(&tskid);

	crep.repatr = UINT_C(0);
	crep.myaddr.portno = UINT_C(9);

#if defined(SUPPORT_INET6)

	memcpy(&crep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));

#else	/* #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
	crep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#endif	/* #if defined(SUPPORT_INET6) */

	return ALLOC_TCP_REP(repid, tskid, &crep);
	}

/*
 *  get_tcp_cep -- TCP 通信端点とを獲得する。
 */

static ER
get_tcp_cep (ID *cepid)
{
	ID		tskid;
	T_TCP_CCEP	ccep;

	get_tid(&tskid);

	ccep.cepatr = UINT_C(0);
	ccep.sbuf = NADR;
	ccep.sbufsz = 0;
	ccep.rbufsz = TCP_DISCARD_SRV_RWBUF_SIZE;
	ccep.callback = NULL;

#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = tcp_discard_srv_rwbuf;
#endif

	return ALLOC_TCP_CEP(cepid, tskid, &ccep);
	}

/*
 *  tcp_discard_srv_task -- TCP エコーサーバタスク
 */

void
tcp_discard_srv_task (intptr_t exinf)
{
	ID	tskid, cepid, repid;
	ER	error = E_OK;

	syscall(get_tid(&tskid));

#if defined(SUPPORT_INET6)
#if defined(SUPPORT_INET4)
	syslog(LOG_NOTICE, "[TCPn DISCARD SRV:%d] started.", tskid);
#else
	syslog(LOG_NOTICE, "[TCP6 DISCARD SRV:%d] started.", tskid);
#endif
#else
	syslog(LOG_NOTICE, "[TCP4 DISCARD SRV:%d] started.", tskid);
#endif

	while (true) {

		syscall(slp_tsk());
		if ((error = get_tcp_cep (&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TDSn:00 EXT] CEP create error: %s", itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep (&repid)) != E_OK) {
				syslog(LOG_NOTICE, "[TDSn:00 EXT] REP create error: %s", itron_strerror(error));
				break;
				}
			else if ((error = tcp_discard_srv(cepid, repid)) != E_OK) {
				error = FREE_TCP_REP(repid, error != E_DLT);
				break;
				}
			}

		if ((error = FREE_TCP_CEP(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TDSn:%02u EXT] CEP delete error: %s", cepid, itron_strerror(error));

		}
	}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

void
tcp_discard_srv_task(intptr_t exinf)
{
	ID	tskid;

	get_tid(&tskid);

#if defined(SUPPORT_INET6)
#if defined(SUPPORT_INET4)
	syslog(LOG_NOTICE, "[TCPn DISCARD SRV:%d,%d] started.", tskid, (ID)exinf);
#else
	syslog(LOG_NOTICE, "[TCP6 DISCARD SRV:%d,%d] started.", tskid, (ID)exinf);
#endif
#else
	syslog(LOG_NOTICE, "[TCP4 DISCARD SRV:%d,%d] started.", tskid, (ID)exinf);
#endif

	while (true) {
		while (tcp_discard_srv((ID)exinf, TCP_DISCARD_SRV_REPID) == E_OK)
			;
		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_DISCARD_SRV */
