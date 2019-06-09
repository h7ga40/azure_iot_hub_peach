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
 *  TCP ECHO クライアント送信関数
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
#include <netapp/tcp_echo_cli.h>

#ifdef USE_TCP_ECHO_CLI

/* 表示 */

//#define SHOW_TIME_OUT
//#define SHOW_SND_COUNT

/* タイムアウト */

#ifdef TOPPERS_S810_CLG3_85

#define CON_TMO		TMO_FEVR
//#define CON_TMO		(60*SYSTIM_HZ)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(30*SYSTIM_HZ)

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

#define CON_TMO		TMO_FEVR
//#define CON_TMO		(60*SYSTIM_HZ)
//#define SND_TMO		TMO_FEVR
#define SND_TMO		(30*SYSTIM_HZ+(netapp_rand()%SYSTIM_HZ)*30)

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

#ifdef USE_TCP_NON_BLOCKING

/*
 *  変数
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

static T_IPV4EP	nblk_src4 = {
			IPV4_ADDRANY,
			TCP_PORTANY,
			};
static T_IPV4EP	nblk_dst4;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

static T_IPEP	nblk_src = {
			IP_ADDRANY,
			TCP_PORTANY,
			};
static T_IPEP	nblk_dst;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

/*
 *  tcp4_active_open -- 能動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	ON
 */

static ER
tcp4_active_open (ID cepid, T_IN4_ADDR ipaddr, uint16_t portno)
{
	ER		error;
	SYSTIM		time;

	nblk_dst4.ipaddr = ipaddr;
	nblk_dst4.portno = portno;

	if ((error = tcp_con_cep(cepid, &nblk_src4, &nblk_dst4, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TEC4:%02u CON] error: %s", cepid, itron_strerror(error));
		return error;
		}

	/* 接続が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

	if ((error = tcp_echo_cli_nblk_error) != E_OK) {
		syslog(LOG_NOTICE, "[TEC4:%02u CON] error: %s", cepid, itron_strerror(tcp_echo_cli_nblk_error));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TEC4:%02u CON] conct: %7lu,  to: %s.%u",
	                   cepid, time / SYSTIM_HZ, ip2str(NULL, &nblk_dst4.ipaddr), nblk_dst4.portno);
	return error;
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

/*
 *  tcp_active_open -- 能動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	ON
 */

static ER
tcp_active_open (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char apip)
{
	ER		error;
	SYSTIM		time;

	nblk_dst.ipaddr = *ipaddr;
	nblk_dst.portno = portno;

	if ((error = TCP_CON_CEP(cepid, &nblk_src, &nblk_dst, TMO_NBLK)) != E_WBLK) {
		if (error == E_PAR || error == E_ID)
			syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s, API: IPv%c, addr: %s",
			                   apip, cepid, itron_strerror(error), apip, IP2STR(NULL, ipaddr));
		else
			syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s",
			                   apip, cepid, itron_strerror(error));
		return error;
		}

	/* 接続が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_CLI_NBLK_READY));

	if ((error = tcp_echo_cli_nblk_error) != E_OK) {
		syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s",
		                   apip, cepid, itron_strerror(tcp_echo_cli_nblk_error));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TEC%c:%02u CON] conct: %7lu,  to: %s.%u",
	                   apip, cepid, time / SYSTIM_HZ,
	                   IP2STR(NULL, &nblk_dst.ipaddr), nblk_dst.portno);
	return error;
	}

/*
 *  tcp_echo_cli_snd -- ECHO/TCP サーバにメッセージを送信する。
 *
 *    USE_TCP_NON_BLOCKING	ON
 */

ER
tcp_echo_cli_snd (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char apip)
{
	static char smsg[SND_BUF_SIZE];

	ER_UINT		slen;
	ER		error;
	uint32_t	total;
	uint16_t	soff, echo, rep;
	char		*p, pat;

#ifdef SND_URG_DATA_SIZE
	int_t	urg = SND_URG_COUNT;
#endif

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

	if (apip == API_PROTO_IPV6) {
		if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
			return error;
		}
	else if (in6_is_addr_ipv4mapped(ipaddr)) {
		if ((error = tcp4_active_open(cepid, ntohl(ipaddr->s6_addr32[3]), portno)) != E_OK)
			return error;
		}
	else {
		if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
			return error;
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
		return error;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t)cepid)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s", apip, cepid, itron_strerror(error));
		goto cls_ret;
		}

	tcp_echo_cli_scount = total = 0;
	for (echo = NUM_ECHO; echo -- > 0; ) {
		soff = 0;
		while (soff < SND_BUF_SIZE) {
#ifdef SHOW_TIME_OUT
			syslog(LOG_NOTICE, "[TEC%c:%02u SND] tmo:   %7lu", apip, cepid, SND_TMO);;
#endif/* of #ifdef SHOW_TIME_OUT */
			if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TEC%c:%02u SND] error: %s",
				                   apip, cepid, itron_strerror(slen));
				error = slen;
				goto cls_ret;
				}
			soff  += (uint16_t)slen;
			total +=     slen;
			tcp_echo_cli_scount ++;
#ifdef SHOW_SND_COUNT
			syslog(LOG_NOTICE, "[TEC%c:%02u SND] count: %7lu, len: %4u, off: %4u",
			                   apip, cepid, tcp_echo_cli_scount, (uint16_t)slen, soff);
#endif/* of #ifdef SHOW_SND_COUNT */

#ifdef SND_URG_DATA_SIZE
			if (urg) {
				if (urg == 1) {
					if ((error = tcp_snd_oob(cepid, smsg, SND_URG_DATA_SIZE, TMO_NBLK)) != E_WBLK)
						syslog(LOG_NOTICE, "[TEC%c:%02u OOB] error: %s",
						                   apip, cepid, itron_strerror(error));
					}
				urg --;
				}
#endif/* of #ifdef SND_URG_DATA_SIZE */

#if defined(SND_DLY)
#if SND_DLY > 0
			syscall(dly_tsk(SND_DLY + netapp_rand() % SYSTIM_HZ));
#endif
#endif
			}
		}

cls_ret:
	if ((error = tcp_sht_cep(cepid)) < 0)
		syslog(LOG_NOTICE, "[TEC%c:%02u SHT] error: %s", apip, cepid, itron_strerror(error));

	/* 受信が完了するまで待つ。*/
	syscall(slp_tsk());

	return error;
	}

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

/*
 *  tcp4_active_open -- 能動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

static ER
tcp4_active_open (ID cepid, T_IN4_ADDR ipaddr, uint16_t portno)
{
	static T_IPV4EP src = {
		IPV4_ADDRANY,
		TCP_PORTANY,
		};

	T_IPV4EP	dst;
	ER		error;
	SYSTIM		time;

	dst.ipaddr = ipaddr;
	dst.portno = portno;

#ifdef SHOW_TIME_OUT
	syslog(LOG_NOTICE, "[TEC%c:%02u CON] tmo:   %7lu", API_PROTO_IPV4, cepid, CON_TMO);
#endif	/* of #ifdef SHOW_TIME_OUT */
	if ((error = tcp_con_cep(cepid, &src, &dst, CON_TMO)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s, addr: %s",
		                   API_PROTO_IPV4, cepid, itron_strerror(error), ip2str(NULL, &ipaddr));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TEC%c:%02u CON] conct: %7lu,  to: %s.%u",
	                   API_PROTO_IPV4, cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno);
	return error;
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

/*
 *  tcp_active_open -- 能動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

static ER
tcp_active_open (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char apip)
{
	static T_IPEP src = {
		IP_ADDRANY,
		TCP_PORTANY,
		};

	T_IPEP		dst;
	ER		error;
	SYSTIM		time;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;

#ifdef SHOW_TIME_OUT
	syslog(LOG_NOTICE, "[TEC%c:%02u CON] tmo:   %7lu", apip, cepid, CON_TMO);
#endif	/* of #ifdef SHOW_TIME_OUT */
	if ((error = TCP_CON_CEP(cepid, &src, &dst, CON_TMO)) != E_OK) {
		if (error == E_PAR || error == E_ID)
			syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s, API: IPv%c, addr: %s",
			                   apip, cepid, itron_strerror(error), apip, IP2STR(NULL, ipaddr));
		else
			syslog(LOG_NOTICE, "[TEC%c:%02u CON] error: %s", apip, cepid, itron_strerror(error));
		return error;
		}

	get_tim(&time);
	syslog(LOG_NOTICE, "[TEC%c:%02u CON] conct: %7lu,  to: %s.%u",
	                   apip, cepid, time / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	return error;
	}

/*
 *  tcp_echo_cli_snd -- ECHO/TCP サーバにメッセージを送信する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

ER
tcp_echo_cli_snd (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char apip)
{
	static char smsg[SND_BUF_SIZE];

	ER_UINT		slen;
	ER		error;
	uint32_t	total;
	uint16_t	soff, echo, rep;
	char		*p, pat;

#ifdef SND_URG_DATA_SIZE
	int_t	urg = SND_URG_COUNT;
#endif

	p = smsg;
	for (rep = NUM_REP_PAT; rep -- > 0; )
		for (pat = PAT_BEGIN; pat <= PAT_END; pat ++)
			*p ++ = pat;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI)

	if (apip == API_PROTO_IPV6) {
		if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
			return error;
		}
	else if (in6_is_addr_ipv4mapped(ipaddr)) {
		if ((error = tcp4_active_open(cepid, ntohl(ipaddr->s6_addr32[3]), portno)) != E_OK)
			return error;
		}
	else {
		if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
			return error;
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	if ((error = tcp_active_open(cepid, ipaddr, portno, apip)) != E_OK)
		return error;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_TCP4_ECHO_CLI) */

	if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_RCV, (intptr_t)cepid)) != E_OK) {
		syslog(LOG_NOTICE, "[TEC%c:%02u DTQ] error: %s", apip, cepid, itron_strerror(error));
		goto cls_ret;
		}

	tcp_echo_cli_scount = total = 0;
	for (echo = NUM_ECHO; echo -- > 0; ) {
		soff = 0;
		while (soff < SND_BUF_SIZE) {
#ifdef SHOW_TIME_OUT
			syslog(LOG_NOTICE, "[TEC%c:%02u SND] tmo:   %7lu", apip, cepid, SND_TMO);
#endif	/* of #ifdef SHOW_TIME_OUT */
			if ((slen = tcp_snd_dat(cepid, smsg + soff, sizeof(smsg) - soff, SND_TMO)) < 0) {
				syslog(LOG_NOTICE, "[TEC%c:%02u SND] error: %s", apip, cepid, itron_strerror(slen));
				error = slen;
				goto cls_ret;
				}
			soff  += (uint16_t)slen;
			total +=     slen;
			tcp_echo_cli_scount ++;
#ifdef SHOW_SND_COUNT
			syslog(LOG_NOTICE, "[TEC%c:%02u SND] count: %7lu, len: %4u",
			                   apip, cepid, tcp_echo_cli_scount, (uint16_t)slen);
#endif	/* of #ifdef SHOW_SND_COUNT */

#ifdef SND_URG_DATA_SIZE
			if (urg > 0) {
				if (urg == 1) {
#ifdef SHOW_TIME_OUT
					syslog(LOG_NOTICE, "[TEC%c:%02u SND] tmo:   %7lu", apip, cepid, SND_TMO);
#endif	/* of #ifdef SHOW_TIME_OUT */
					if ((slen = tcp_snd_oob(cepid, smsg, SND_URG_DATA_SIZE, SND_TMO)) >= 0)
						syslog(LOG_NOTICE, "[TEC%c:%02u SND] urg:   %7lu",
						                   apip, cepid, (uint16_6)slen);
					else
						syslog(LOG_NOTICE, "[TEC%c:%02u OOB] error: %s",
						                   apip, cepid, itron_strerror(slen));
					}
				urg --;
				}
#endif	/* of #ifdef SND_URG_DATA_SIZE */

#if defined(SND_DLY)
#if SND_DLY > 0
			syscall(dly_tsk(SND_DLY + netapp_rand() % SYSTIM_HZ));
#endif
#endif
			}
		}

cls_ret:
	if ((error = tcp_sht_cep(cepid)) < 0)
		syslog(LOG_NOTICE, "[TEC%c:%02u SHT] error: %s", apip, cepid, itron_strerror(error));

	/* 受信が完了するまで待つ。*/
	syscall(slp_tsk());

	return error;
	}

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#endif	/* of #ifdef USE_TCP_ECHO_CLI */
