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
 *  UDP4 ECHO クライアント
 */

#include <string.h>

#if defined(TARGET_KERNEL_ASP)

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #if defined(TARGET_KERNEL_ASP) */

#if defined(TARGET_KERNEL_JSP)

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #if defined(TARGET_KERNEL_JSP) */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/udp4_echo_cli.h>

#if defined(USE_UDP4_ECHO_CLI_TSK)

/* echo サーバのポート番号 */

#define ECHO_SRV_PORTNO		UINT_C(7)

/* 送信間隔 */

#if defined(TOPPERS_S810_CLG3_85)

#define SLP_ITV		(30*SYSTIM_HZ)

#else	/* of #if defined(TOPPERS_S810_CLG3_85) */

#define SLP_ITV		(60*SYSTIM_HZ)

#endif	/* of #if defined(TOPPERS_S810_CLG3_85) */

/* 自動実行 */

#if defined(UDP4_ECHO_CLI_AUTO_RUN_STR)
#else
#if 0
#define UDP4_ECHO_CLI_AUTO_RUN_STR	"172.25.193.140 - 1000"
#endif
#endif

/*
 *  全域変数
 */

bool_t udp4_echo_cli_valid;

/*
 * 変数
 */

#define BUF_SIZE	2048

static char		udp_buf[BUF_SIZE];
static T_IPV4EP		dst;

#if defined(USE_UDP_NON_BLOCKING)

static ER_UINT		nblk_len;
static ID		nblk_tskid;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_udp4_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	nblk_len = *(ER_UINT*)p_parblk;
	syscall(wup_tsk(nblk_tskid));
	return E_OK;
	}

/*
 *  udp4_echo_cli -- ECHO/UDP サーバにメッセージを送信する (ノンブロッキングコール使用)。
 */

ER
udp4_echo_cli (ID cepid, T_IN4_ADDR *ipaddr, uint16_t portno, char *line)
{
	ER_UINT	error;
	SYSTIM	time;
	uint_t	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len         = strlen(line);
	get_tim(&time);
	get_tid(&nblk_tskid);
	if ((error = udp_snd_dat(cepid, &dst, line, len, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[UEC4:%02u SND(NBLK)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(nblk_len));
		return error;
		}
	else
		syslog(LOG_NOTICE, "[UEC4:%02u SND(NBLK)] send:  %7lu,  to: %s.%d\n"
		                   "                                    msg: %s",
		                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, line);

	syscall(slp_tsk());
	get_tim(&time);
	if (nblk_len < 0) {	/* 0 以下の場合は、エラーコード */
		syslog(LOG_NOTICE, "[UEC4:%02u CBR(NBLK)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(nblk_len));
		return nblk_len;
		}

	if ((error = udp_rcv_dat(cepid, &dst, udp_buf, sizeof(udp_buf), TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[UEC4:%02u RCV(NBLK)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(nblk_len));
		return error;
		}

	syscall(slp_tsk());
	get_tim(&time);
	if (nblk_len < 0) {	/* 0 以下の場合は、エラーコード */
		syslog(LOG_NOTICE, "[UEC4:%02u RCV(NBLK)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(nblk_len));
		return nblk_len;
		}
	else {
		udp_buf[nblk_len] = '\0';
		get_tim(&time);
		syslog(LOG_NOTICE, "[UEC4:%02u RCV(NBLK)] recv:  %7lu,from: %s.%d\n"
		                   "                                    msg: %s",
		                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, udp_buf);
		nblk_len = E_OK;
		}

	return nblk_len;
	}

#else	/* of #if defined(USE_UDP_NON_BLOCKING) */

#if defined(USE_UDP_CALL_BACK)

/*
 *  コールバック関数
 */

ER
callback_udp4_echo_cli (ID cepid, FN fncd, void *p_parblk)
{
	SYSTIM	time;
	ER_UINT	len;

	get_tim(&time);
	if ((len = udp_rcv_dat(cepid, &dst, udp_buf, sizeof(udp_buf), TMO_FEVR)) >= 0) {
		*(udp_buf + len) = '\0';
		syslog(LOG_NOTICE, "[UEC4:%02u RCV(CB)] recv:  %7lu,from: %s.%d\n"
		                   "                                  msg: %s",
		                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, udp_buf);
		}
	else
		syslog(LOG_NOTICE, "[UEC4:%02u RCV(CB)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(len));

	return E_OK;
	}

/*
 *  udp4_echo_cli -- ECHO/UDP サーバにメッセージを送信する (コールバック関数使用)。
 */

ER
udp4_echo_cli (ID cepid, T_IN4_ADDR *ipaddr, uint16_t portno, char *line)
{
	SYSTIM	time;
	ER_UINT	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len        = strlen(line);
	get_tim(&time);
	syslog(LOG_NOTICE, "[UEC4:%02u SND(CB)] send:  %7lu,  to: %s.%d\n"
	                   "                                  msg: %s",
	                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, line);
	if ((len = udp_snd_dat(cepid, &dst, line, len, TMO_FEVR)) < 0)
		syslog(LOG_NOTICE, "[UEC4:%02u SND(CB)] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(len));

	return E_OK;
	}

#else	/* of #if defined(USE_UDP_CALL_BACK) */

/*
 *  udp4_echo_cli -- ECHO/UDP サーバにメッセージを送信する (コールバック関数未使用)。
 */

ER
udp4_echo_cli (ID cepid, T_IN4_ADDR *ipaddr, uint16_t portno, char *line)
{
	SYSTIM	time;
	ER_UINT	len;

	dst.ipaddr = *ipaddr;
	dst.portno = portno;
	len         = strlen(line);
	get_tim(&time);
	syslog(LOG_NOTICE, "[UEC4:%02u SND] send:  %7lu,  to: %s.%d\n"
	                   "                              msg: %s",
	                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, line);
	if ((len = udp_snd_dat(cepid, &dst, line, len, TMO_FEVR)) < 0) {
		syslog(LOG_NOTICE, "[UEC4:%02u SND] error: %7lu,%s",
		                   cepid, time / SYSTIM_HZ, itron_strerror(len));
		return len;
		}

	if ((len = udp_rcv_dat(cepid, &dst, udp_buf, sizeof(udp_buf), 10*1000)) >= 0) {
		*(udp_buf + len) = '\0';
		get_tim(&time);
		syslog(LOG_NOTICE, "[UEC4:%02u RCV] recv:  %7lu,from: %s.%d\n"
		                   "                              msg: %s",
		                   cepid, time / SYSTIM_HZ, ip2str(NULL, &dst.ipaddr), dst.portno, udp_buf);
		return E_OK;
		}
	else {
		get_tim(&time);
		syslog(LOG_NOTICE, "[UEC4:%02u RCV] error: %7lu,%s",
		       cepid, time / SYSTIM_HZ, itron_strerror(len));
		return len;
		}
	}

#endif	/* of #if defined(USE_UDP_CALL_BACK) */

#endif	/* of #if defined(USE_UDP_NON_BLOCKING) */

/*
 *  getcomd -- コマンドを得る。
 */

#if defined(UDP4_ECHO_CLI_AUTO_RUN_STR)

static char *
getcomd (ID cepid, bool_t retry)
{
	ER		error;
	char		*line = NULL;
	static char	auto_run_str[] = UDP4_ECHO_CLI_AUTO_RUN_STR;
	static int_t	count = 0;

	if (retry || count == 0) {
		line = auto_run_str;
		dly_tsk(6 * 1000);
		}
	else {
		while ((error = rcv_dtq(DTQ_UDP4_ECHO_CLI, (intptr_t*)&line)) != E_OK) {
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] error:%s", cepid, itron_strerror(error));
			dly_tsk(SLP_ITV);
			}
		}

	count ++;
	return line;
	}

#else	/* of #if defined(UDP4_ECHO_CLI_AUTO_RUN_STR) */

static char *
getcomd (ID cepid, bool_t retry)
{
	ER	error;
	char	*line = NULL;

	while ((error = rcv_dtq(DTQ_UDP4_ECHO_CLI, (intptr_t*)&line)) != E_OK) {
		syslog(LOG_NOTICE, "[UEC4:%02u TSK] error:%s", cepid, itron_strerror(error));
		dly_tsk(SLP_ITV);
		}
	return line;
	}

#endif	/* of #if defined(UDP4_ECHO_CLI_AUTO_RUN_STR) */

#define MESSAGE_FORMAT	"MSG 1000000"

/*
 *  UDP4 ECHO クライアント送信タスク
 */

void
udp4_echo_cli_task (intptr_t exinf)
{
	static char msg[sizeof(MESSAGE_FORMAT)] = MESSAGE_FORMAT;

	ID		tskid, cepid;
	ER		error = E_OK;
	SYSTIM		time;
	T_IN4_ADDR	addr;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	T_IN6_ADDR	addr6;
#endif

	char		*line, *p;
	int_t		rep;
	uint32_t	count, msgno;
	bool_t		retry = false;
	uint16_t	portno = ECHO_SRV_PORTNO;

#if defined(USE_UDP_EXTENTIONS)
	T_UDP_CCEP	ccep;
#endif

	dly_tsk(10*1000);
	get_tid(&tskid);
	syslog(LOG_NOTICE, "[UDP4 ECHO CLI:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		line = skip_blanks(getcomd((ID)exinf, retry));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

		line = lookup_ipaddr(&addr6, line, API_PROTO_IPV4);
		if (line == NULL || !in6_is_addr_ipv4mapped(&addr6)) {
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] sleep %d.%03u[s], unknown host.",
			       (ID)exinf, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			dly_tsk(SLP_ITV);
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] resume.", (ID)exinf);
			retry = true;
			continue;
			}
		addr = ntohl(addr6.s6_addr32[3]);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		if ((line = lookup_ipaddr(&addr, line, DEFAULT_API_PROTO)) == NULL) {
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] sleep %d.%03u[s], unknown host.",
			       (ID)exinf, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			dly_tsk(SLP_ITV);
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] resume.", (ID)exinf);
			retry = true;
			continue;
			}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9') {	/* Port No */
			line = get_int(&rep, line);
			portno = (uint16_t)rep;
			}
		else if (*line == '-')
			line ++;

#if defined(USE_UDP_EXTENTIONS)

		ccep.cepatr = UINT_C(0);
		ccep.myaddr.portno = UDP_PORTANY;

#if defined(SUPPORT_INET6)

		memcpy(&ccep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));

#else	/* #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
		ccep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#endif	/* #if defined(SUPPORT_INET6) */

#if defined(USE_UDP_NON_BLOCKING)
		ccep.callback = (FP)callback_nblk_udp4_echo_cli;
#else
		ccep.callback = NULL;
#endif

		if ((error = alloc_udp4_cep(&cepid, tskid, &ccep)) != E_OK) {
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] CEP create error:%s", cepid, itron_strerror(error));
			continue;
			}

#else/* of #if defined(USE_UDP_EXTENTIONS) */

		cepid = (ID)exinf;

#endif/* of #if defined(USE_UDP_EXTENTIONS) */

		line = skip_blanks(line);
		if ('0' <= *line && *line <= '9') {				/* Repeat */
			line = get_int(&rep, line);
#if 0
			if (rep > 1000000)
				rep = 1000000;
#endif
			udp4_echo_cli_valid = true;
			count = 0;
			while (rep == 0 || count < rep) {
				if (!udp4_echo_cli_valid) {
					syslog(LOG_NOTICE, "[UEC4:%02u TSK] canceled.", cepid);
					break;
					}
					
				count ++;
				p = &msg[sizeof(MESSAGE_FORMAT)] - 1;
				for (msgno = count; msgno > 0; msgno /= 10)
					*(-- p) = msgno % 10 + '0';
				while (*(p - 2) != 'G')
					*(-- p) = ' ';

				if ((error = udp4_echo_cli(cepid, &addr, portno, msg)) != E_OK)
					dly_tsk(30 * SYSTIM_HZ);

				dly_tsk(25 * SYSTIM_HZ + netapp_rand() % (10 * SYSTIM_HZ));
				}
			}

		else								/* Single Message */
			error = udp4_echo_cli(cepid, &addr, portno, line);

		if (error != E_OK) {
			get_tim(&time);
			syslog(LOG_NOTICE, "[UEC4:%02u SND] error: %7lu,%s",
			                   cepid, time / SYSTIM_HZ, itron_strerror(error));
			}

#if defined(USE_UDP_EXTENTIONS)

		if ((error = free_udp4_cep(cepid, !(error == E_NOEXS || error == E_DLT))) != E_OK)
			syslog(LOG_NOTICE, "[UEC4:%02u TSK] CEP delete error:%s", cepid, itron_strerror(error));

#endif	/* of #if defined(USE_UDP_EXTENTIONS) */

		}
	}

#endif	/* of #if defined(USE_UDP4_ECHO_CLI_TSK) */
