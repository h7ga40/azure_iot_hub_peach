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
 *  デバッグコンソール
 */

#include <stdlib.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
#include <syssvc/logtask.h>
#include "kernel/kernel_impl.h"
#include "kernel/task.h"
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#include "../kernel/jsp_kernel.h"
#include "../kernel/task.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/ppp_var.h>
#include <net/ppp_fsm.h>
#include <net/ppp_lcp.h>
#include <net/ppp_ipcp.h>
#include <net/ppp_modem.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/if_ether.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <netinet6/nd6.h>

#include <net/if_var.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/wwws.h>
#include <netapp/tcp_echo_srv1.h>
#include <netapp/tcp_echo_srv2.h>
#include <netapp/tcp_echo_cli.h>
#include <netapp/tcp_discard_cli.h>
#include <netapp/udp6_echo_srv.h>
#include <netapp/udp4_echo_srv.h>
#include <netapp/udp6_echo_cli.h>
#include <netapp/udp4_echo_cli.h>
#include <netapp/udp_discard_cli.h>
#include <netapp/resolver.h>
#include <netapp/dbg_cons.h>
#include <netapp/dhcp6_cli.h>
#include <netapp/dhcp4_cli.h>
#include "lcd.h"

#ifdef USE_DBG_CONS

/*
 *  dbg_cons_task_status -- タスクの状態の出力
 */

static const char task_stat_str[][sizeof("SUSPENDED")] = {
	"DORMANT",		/* 休止状態		*/
	"RUNNABLE",		/* 実行できる状態	*/
	"WAINTING",		/* 待ち状態		*/
	"SUSPENDED",		/* 強制待ち状態		*/
	"SLEEP",		/* 起床待ち状態		*/
	"WOBJ",			/* 同期・通信オブジェクトに対する待ち状態	*/
	"WOBJCB",		/* 共通部分の待ちキューにつながっている。	*/
	};

void
dbg_cons_task_status (ID portid, char *line)
{
	TCB	*tcb;
	int_t	ix, st, sx;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "TASK Status\n"
	            "ID PI State\n");
	for (ix = TMIN_TSKID; ix <= tmax_tskid; ix ++) {
		tcb = get_tcb(ix);
		cons_printf(portid, "%2d %2d ", ix, tcb->priority);
		st = tcb->tstat;
		for (sx = 0; st; sx ++) {
			if (st & 0x01) {
				cons_printf(portid, "%s", task_stat_str[sx]);
				if (st & 0xfe)
					cons_printf(portid, "|");
				}
			st >>= 1;
			}
		cons_putchar(portid, '\n');
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#ifdef SUPPORT_TCP

/*
 *  tcp_status -- TCP の状態の出力
 */

static const char tcp_fsm_str[][sizeof("ESTABLISHED")] = {
	"CLOSED",		/* クローズ			*/
	"LISTEN",		/* 受動オープン			*/
	"SYN_SENT",		/* 能動オープン、SYN 送信済み	*/
	"SYN_RECVD",		/* 受動オープン、SYN 受信済み	*/
	"ESTABLISHED",		/* コネクション開設完了		*/
	"CLOSE_WAIT",		/* 相手から FIN 受信、APP の終了待ち	*/
	"FIN_WAIT_1",		/* APP が終了、FIN 送信済み、ACK 待ち	*/
	"CLOSING",		/* 同時クローズ、FIN 交換済み、ACK 待ち	*/
	"LAST_ACK",		/* APP が終了、ACK 待ち			*/
	"FIN_WAIT_2",		/* 相手からの FIN 待ち			*/
	"TIME_WAIT",		/* 相手からの FIN 受信済み、時間待ち	*/
	};

#if defined(SUPPORT_INET6)

static void
tcp_cep_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	int		len;

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	T_TCP_TWCEP*	twcep;

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	cons_printf(portid,
	            "TCP CEP Status\n"
	            "ID  Snd-Q Rcv-Q Foreign Address                               State\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++)
		if (!VALID_TCP_CEP(cep)) {
			cons_printf(portid,
			            "%2d%c%c     0     0"
			            " -                                            "
			            " INVALID\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			}
		else if (cep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2d%c%c     0     0"
			            " -                                            "
			            " CLOSED\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			}
		else if (cep->fsm_state == TCP_FSM_LISTEN) {
			cons_printf(portid,
			            "%2d%c%c     0     0"
			            " -                                            "
			            " LISTEN\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			}
		else {
			cons_printf(portid,
			            "%2d%c%c %5d %5d ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4',
			            cep->swbuf_count, cep->rwbuf_count);

#if defined(SUPPORT_INET4)

			if (TCP_IS_CEP_IPV4(cep)) {
				T_IN4_ADDR	addr;

				addr = ntohl(cep->dstaddr.ipaddr.s6_addr32[3]);
				len  = put_ipv4addr(portid, &addr, 0);
				}
			else
				len  = put_ipv6addr(portid, &cep->dstaddr.ipaddr, 0);

#else	/* of #if defined(SUPPORT_INET4) */

			len  = put_ipv6addr(portid, &cep->dstaddr.ipaddr, 0);

#endif	/* of #if defined(SUPPORT_INET4) */

			cons_putchar(portid, '.');
			len += cons_putnumber(portid, cep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 44 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (cep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[cep->fsm_state],
				                    cep->timer[TCP_TIM_2MSL] / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[cep->fsm_state]);
			}

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	for (twcep = tcp_twcep; twcep < &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep ++) {
		if (twcep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2dTW     0     0"
			            " -                                            "
			            " CLOSED\n",
			            GET_TCP_TWCEPID(twcep));
			}
		else {
			cons_printf(portid, "%2dTW     0     0 ", GET_TCP_TWCEPID(twcep));

#if defined(SUPPORT_INET4)

			if (TCP_IS_CEP_IPV4(twcep)) {
				T_IN4_ADDR	addr;

				addr = ntohl(twcep->dstaddr.ipaddr.s6_addr32[3]);
				len  = put_ipv4addr(portid, &addr, 0);
				}
			else
				len  = put_ipv6addr(portid, &twcep->dstaddr.ipaddr, 0);

#else	/* of #if defined(SUPPORT_INET4) */

			len  = put_ipv6addr(portid, &twcep->dstaddr.ipaddr, 0);

#endif	/* of #if defined(SUPPORT_INET4) */

			cons_putchar(portid, '.');
			len += cons_putnumber(portid, twcep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 44 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (twcep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[twcep->fsm_state],
				                    twcep->timer_2msl / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[twcep->fsm_state]);
			}
		}

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	}

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

static void
tcp_cep_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	uint_t		len;

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	T_TCP_TWCEP*	twcep;

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	cons_printf(portid,
	            "TCP CEP Status\n"
	            "ID  Snd-Q Rcv-Q Local Address         Foreign Address       State\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++)
		if (!VALID_TCP_CEP(cep)) {
			cons_printf(portid,
			            "%2d%c%c     0     0"
			            " -                    "
			            " -                    "
			            " INVALID\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			}
		else if (cep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2d%c%c     0     0"
			            " -                    "
			            " -                    "
			            " CLOSED\n",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			}
		else if (cep->fsm_state == TCP_FSM_LISTEN) {
			cons_printf(portid,
			            "%2d%c%c     0     0 ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : '.',
			            TCP_IS_CEP_IPV6(cep) ? '6' : '4');
			len  = PUT_IPADDR(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cons_printf(portid,
			            " -                    "
			            " LISTEN\n");
			}
		else {
			cons_printf(portid,
			            "%2d%c %5d %5d ",
			            GET_TCP_CEPID(cep),
			            DYNAMIC_TCP_CEP(cep) ? 'D' : ' ',
			            cep->swbuf_count, cep->rwbuf_count);
			len  = PUT_IPADDR(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			len  = PUT_IPADDR(portid, &cep->dstaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, cep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (cep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[cep->fsm_state],
				                    cep->timer[TCP_TIM_2MSL] / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[cep->fsm_state]);
			}

#if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0

	for (twcep = tcp_twcep; twcep < &tcp_twcep[NUM_TCP_TW_CEP_ENTRY]; twcep ++) {
		if (twcep->fsm_state == TCP_FSM_CLOSED) {
			cons_printf(portid,
			            "%2dTW     0     0"
			            " -                    "
			            " -                    "
			            " CLOSED\n",
			            GET_TCP_TWCEPID(twcep));
			}
		else {
			cons_printf(portid, "%2dTW     0     0 ", GET_TCP_TWCEPID(twcep));
			len  = PUT_IPADDR(portid, &twcep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, twcep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			len  = PUT_IPADDR(portid, &twcep->dstaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, twcep->dstaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 20 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			if (twcep->fsm_state == TCP_FSM_TIME_WAIT)
				cons_printf(portid, " %s(%d)\n", tcp_fsm_str[twcep->fsm_state],
				                    twcep->timer_2msl / TCP_SLOW_HZ);
			else
				cons_printf(portid, " %s\n", tcp_fsm_str[twcep->fsm_state]);
			}
		}

#endif	/* of #if defined(NUM_TCP_TW_CEP_ENTRY) && NUM_TCP_TW_CEP_ENTRY > 0 */

	}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

#if TNUM_TCP6_REPID > 0

static void
tcp6_rep_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	T_TCP6_REP*	rep;
	ID		repid;
	int_t		cnt;
	uint_t		len;

	cons_printf(portid,
	            "TCP6 REP Status\n"
	            "ID  Local Address         CEP\n");
	for (repid = TMIN_TCP6_REPID; repid <= tmax_tcp6_repid; repid ++) {
		rep = GET_TCP6_REP(repid);
		cons_printf(portid, "%2d%c ", GET_TCP6_REPID(rep), DYNAMIC_TCP_REP(rep) ? 'D' : ' ');
		if (VALID_TCP_REP(rep)) {
			len  = PUT_IPADDR(portid, &rep->myaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, rep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cnt = 0;
			for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {
				if (TCP_IS_CEP_IPV6(cep) && cep->rep == rep) {
					if (cnt > 0)
						cons_printf(portid, ",%d", GET_TCP_CEPID(cep));
					else
						cons_printf(portid, "%d", GET_TCP_CEPID(cep));
					cnt ++;
					}
				}
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}
	}

#endif	/* of #if TNUM_TCP6_REPID > 0 */

#if TNUM_TCP4_REPID > 0

static void
tcp4_rep_status (ID portid, char *line)
{
	T_TCP_CEP*	cep;
	T_TCP4_REP*	rep;
	ID		repid;
	int_t		cnt;
	uint_t		len;

	cons_printf(portid,
	            "TCP4 REP Status\n"
	            "ID  Local Address         CEP\n");
	for (repid = TMIN_TCP4_REPID; repid <= tmax_tcp4_repid; repid ++) {
		rep = GET_TCP4_REP(repid);
		cons_printf(portid, "%2d%c ", GET_TCP4_REPID(rep), DYNAMIC_TCP_REP(rep) ? 'D' : ' ');
		if (VALID_TCP_REP(rep)) {
			len  = put_ipv4addr(portid, &rep->myaddr.ipaddr, 0);
			cons_putchar(portid, ':');
			len += cons_putnumber(portid, rep->myaddr.portno, 10, radhex, 0, false, ' ');
			for (len = 21 - len; len -- > 0; )
				cons_putchar(portid, ' ');
			cnt = 0;
			for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

				if (TCP_IS_CEP_IPV4(cep) && cep->rep4 == rep) {

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

				if (TCP_IS_CEP_IPV4(cep) && cep->rep == rep) {

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

					if (cnt > 0)
						cons_printf(portid, ",%d", GET_TCP_CEPID(cep));
					else
						cons_printf(portid, "%d", GET_TCP_CEPID(cep));
					cnt ++;
					}
				}
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}
	}

#endif	/* of #if TNUM_TCP4_REPID > 0 */

static void
tcp_status (ID portid, char *line)
{
	WAI_NET_CONS_PRINTF();

	tcp_cep_status(portid, line);

#if TNUM_TCP6_REPID > 0
	tcp6_rep_status(portid, line);
#endif

#if TNUM_TCP4_REPID > 0
	tcp4_rep_status(portid, line);
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

/*
 *  udp6_status -- UDP の状態の出力
 */

#if TNUM_UDP6_CEPID > 0

extern const ID tmax_tcp_cepid;

static void
udp6_status (ID portid, char *line)
{
	T_UDP6_CEP*	cep;
	ID		cepid;
	uint_t		len;

	cons_printf(portid,
	            "UDP6 CEP Status\n"
	            "ID  Local Address\n");

	for (cepid = TMIN_UDP6_CEPID; cepid <= tmax_udp6_cepid; cepid ++) {
		cep = GET_UDP6_CEP(cepid);
		cons_printf(portid, "%2d%c ", GET_UDP6_CEPID(cep), DYNAMIC_UDP_CEP(cep) ? 'D' : ' ');
		if (VALID_UDP_CEP(cep)) {
			len  = put_ipv6addr(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}

	SIG_NET_CONS_PRINTF();
	}

#endif	/* of #if TNUM_UDP6_CEPID > 0 */

/*
 *  udp4_status -- UDP の状態の出力
 */

#if TNUM_UDP4_CEPID > 0

static void
udp4_status (ID portid, char *line)
{
	T_UDP4_CEP*	cep;
	ID		cepid;
	uint_t		len;

	cons_printf(portid,
	            "UDP4 CEP Status\n"
	            "ID  Local Address\n");

	for (cepid = TMIN_UDP4_CEPID; cepid <= tmax_udp4_cepid; cepid ++) {
		cep = GET_UDP4_CEP(cepid);
		cons_printf(portid, "%2d%c ", GET_UDP4_CEPID(cep), DYNAMIC_UDP_CEP(cep) ? 'D' : ' ');
		if (VALID_UDP_CEP(cep)) {
			len  = put_ipv4addr(portid, &cep->myaddr.ipaddr, 0);
			cons_putchar(portid, '.');
			len += cons_putnumber(portid, cep->myaddr.portno, 10, radhex, 0, false, ' ');
			}
		else
			cons_printf(portid, "INVALID");
		cons_putchar(portid, '\n');
		}

	SIG_NET_CONS_PRINTF();
	}

#endif	/* of #if TNUM_UDP4_CEPID > 0 */

static void
udp_status (ID portid, char *line)
{
	WAI_NET_CONS_PRINTF();

#if TNUM_UDP6_CEPID > 0
	udp6_status(portid, line);
#endif

#if TNUM_UDP4_CEPID > 0
	udp4_status(portid, line);
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #ifdef SUPPORT_UDP */

#ifdef SUPPORT_ETHER

#if defined(SUPPORT_INET6)

/*
 * ifa6_status -- 近隣キャッシュ状態の出力
 */

static const char nd_state_str[][sizeof("INCOMPLETE")] = {
	"FREE",			/* 未使用		*/
	"NO STATE",		/* 状態未定義		*/
	"INCOMPLETE",		/* 未解決		*/
	"REACHABLE",		/* 到達可能		*/
	"STALE",		/* 到達可能性は未確認	*/
	"DELAY",		/* 到達可能性の確認待機	*/
	"PROBE",		/* 到達可能性を確認中	*/
	};

static void
ifa6_status (ID portid, char *line)
{
	const T_LLINFO_ND6	*cache;
	SYSTIM			now;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "ND Cache Status\n"
	            "IX Expire State      MAC Address       IP Address\n");

	/* expire の単位は [us]。*/
	get_tim(&now);
	cache = nd6_get_cache();
	for (ix = 0; ix < NUM_ND6_CACHE_ENTRY; ix ++) {
		if (cache[ix].state != ND6_LLINFO_FREE) {
			cons_printf(portid, "%2d %6ld %10s %M %lI\n",
			            ix, 
			            cache[ix].state == ND6_LLINFO_STALE ||
			            (uint32_t)((int32_t)(cache[ix].expire - now) <= 0
			                 ? 0 : (int32_t)(cache[ix].expire - now) / SYSTIM_HZ),
			            nd_state_str[cache[ix].state],
			            &cache[ix].ifaddr,
			            &cache[ix].addr);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

/*
 *  ifa4_status -- ARP キャッシュ出力
 */

static void
ifa4_status (ID portid, char *line)
{
	const T_ARP_ENTRY	*cache;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "ARP Cache\n"
	            "IX Expire MAC Address       IP Address\n");

	/* expire の単位は [us]。*/
	cache = arp_get_cache();
	for (ix = 0; ix < NUM_ARP_ENTRY; ix ++) {
		if (cache[ix].expire != 0)
			cons_printf(portid, "%2d %6d %M %hI\n",
			                    ix, cache[ix].expire / NET_TIMER_HZ,
			                    cache[ix].mac_addr, &cache[ix].ip_addr);
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #ifdef SUPPORT_ETHER */

#if NET_COUNT_ENABLE

/*
 *  net_count_struct -- ネットワーク統計情報
 */

static void
net_count_struct (ID portid, char *name, T_NET_COUNT *item)
{
	cons_printf(portid, "\n%s\n", name);
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	item->in_octets);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",	item->out_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		item->in_packets);
	cons_printf(portid, "\t送信バケット数\t%lu\n",		item->out_packets);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",	item->in_err_packets);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",	item->out_err_packets);
	}

/*
 *  net_count -- ネットワーク統計情報
 */

static void
net_count (ID portid, char *line)
{
	const T_NET_BUF_ENTRY	*tbl;
	SYSTIM			now;
	int_t			ix;

	WAI_NET_CONS_PRINTF();
	get_tim(&now);
	cons_printf(portid, "ネットワーク統計情報\t経過時間[us]\t%lu\t", now);
	if (now > (1000 * 3600 * 24))
		cons_printf(portid, "%3lu日 %2lu時間 %2lu分 %2lu秒\n",
		             now / (1000 * 3600 * 24),
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);
	else
		cons_printf(portid, "%2lu時間 %2lu分 %2lu秒\n",
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);

#ifdef SUPPORT_PPP

	net_count_struct(portid, "HDLC", &net_count_hdlc);
	cons_printf(portid, "\nPPP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp.in_octets);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",	net_count_ppp.out_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp.in_packets);
	cons_printf(portid, "\t送信バケット数\t%lu\n",		net_count_ppp.out_packets);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",	net_count_ppp.in_err_packets);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",	net_count_ppp.out_err_packets);
	cons_printf(portid, "\tnet_buf 割り当て失敗数\t%lu\n",	net_count_ppp_no_bufs);

	cons_printf(portid, "\nLCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_lcp_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_lcp_in_packets);

	cons_printf(portid, "\nIPCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_ipcp_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_ipcp_in_packets);

	cons_printf(portid, "\nPAP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",	net_count_ppp_upap_in_octets);
	cons_printf(portid, "\t受信バケット数\t%lu\n",		net_count_ppp_upap_in_packets);

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	net_count_struct(portid, "イーサネット", &net_count_ether);

	cons_printf(portid, "\nイーサネット・ネットワークインタフェース\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_IN_OCTETS]);
	cons_printf(portid, "\t受信バケット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーバケット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_IN_ERR_PACKETS]);
	cons_printf(portid, "\tnet_buf 割り当て失敗数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_NO_BUFS]);
	cons_printf(portid, "\t受信セマフォ資源返却上限オーバー数\t%lu\n",	net_count_ether_nic[NC_ETHER_NIC_RXB_QOVRS]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_OUT_OCTETS]);
	cons_printf(portid, "\t送信バケット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーバケット数\t%lu\n",			net_count_ether_nic[NC_ETHER_NIC_OUT_ERR_PACKETS]);
	cons_printf(portid, "\tコリジョン数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_COLS]);
	cons_printf(portid, "\t送信セマフォ資源返却上限オーバー数\t%lu\n",	net_count_ether_nic[NC_ETHER_NIC_TXB_QOVRS]);
	cons_printf(portid, "\t送信タイムアウト数	\t%lu\n",		net_count_ether_nic[NC_ETHER_NIC_TIMEOUTS]);
	cons_printf(portid, "\tリセット数\t%lu\n",				net_count_ether_nic[NC_ETHER_NIC_RESETS]);

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(SUPPORT_INET6)

	cons_printf(portid, "\nIPv6\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_ip6[NC_IP6_IN_OCTETS]);
	cons_printf(portid, "\t受信データグラム数\t%lu\n",		net_count_ip6[NC_IP6_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーデータグラム数\t%lu\n",	net_count_ip6[NC_IP6_IN_ERR_PACKETS]);
	cons_printf(portid, "\t長さエラー数\t%lu\n",			net_count_ip6[NC_IP6_IN_ERR_SHORT]);
	cons_printf(portid, "\tバージョンエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_VER]);
	cons_printf(portid, "\tアドレスエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_ADDR]);
	cons_printf(portid, "\tプロトコルエラー数\t%lu\n",		net_count_ip6[NC_IP6_IN_ERR_PROTO]);
	cons_printf(portid, "\t分割受信数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_IN]);
	cons_printf(portid, "\t分割受信フラグメント数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_FRAGS]);
	cons_printf(portid, "\t分割受信再構成成功数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_OK]);
	cons_printf(portid, "\t分割受信破棄数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_IN_DROP]);
	cons_printf(portid, "\t分割受信バッファり当て失敗数\t%lu\n",	net_count_ip6[NC_IP6_FRAG_IN_NO_BUF]);
	cons_printf(portid, "\t分割受信タイムアウト数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_IN_TMOUT]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_ip6[NC_IP6_OUT_OCTETS]);
	cons_printf(portid, "\t送信データグラム数\t%lu\n",		net_count_ip6[NC_IP6_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーデータグラム数\t%lu\n",	net_count_ip6[NC_IP6_OUT_ERR_PACKETS]);
	cons_printf(portid, "\t分割送信数\t%lu\n",			net_count_ip6[NC_IP6_FRAG_OUT]);
	cons_printf(portid, "\t分割送信フラグメント数\t%lu\n",		net_count_ip6[NC_IP6_FRAG_OUT_FRAGS]);

	cons_printf(portid, "\nICMPv6\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_OCTETS]);
	cons_printf(portid, "\t受信パケット数\t%lu\n",			net_count_icmp6[NC_ICMP6_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーパケット数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_ERR_PACKETS]);
	cons_printf(portid, "\t受信チックサムエラー数\t%lu\n",		net_count_icmp6[NC_ICMP6_IN_ERR_CKSUM]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_icmp6[NC_ICMP6_OUT_OCTETS]);
	cons_printf(portid, "\t送信パケット数\t%lu\n",			net_count_icmp6[NC_ICMP6_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーパケット数\t%lu\n",		net_count_icmp6[NC_ICMP6_OUT_ERR_PACKETS]);

	cons_printf(portid, "\n近隣探索\n");
	cons_printf(portid, "\tルータ通知受信数\t%lu\n",		net_count_nd6[NC_ND6_RA_IN_PACKETS]);
	cons_printf(portid, "\tルータ要請送信数\t%lu\n",		net_count_nd6[NC_ND6_RS_OUT_PACKETS]);
	cons_printf(portid, "\t近隣通知受信数\t%lu\n",			net_count_nd6[NC_ND6_NA_IN_PACKETS]);
	cons_printf(portid, "\t近隣通知送信数\t%lu\n",			net_count_nd6[NC_ND6_NA_OUT_PACKETS]);
	cons_printf(portid, "\t近隣要請受信数\t%lu\n",			net_count_nd6[NC_ND6_NS_IN_PACKETS]);
	cons_printf(portid, "\t近隣要請送信数\t%lu\n",			net_count_nd6[NC_ND6_NS_OUT_PACKETS]);
	cons_printf(portid, "\t重複アドレス検出送信数\t%lu\n",		net_count_nd6[NC_ND6_DAD_OUT_PACKETS]);

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#ifdef SUPPORT_ETHER

	net_count_struct(portid, "ARP", &net_count_arp);

#endif	/* of #ifdef SUPPORT_ETHER */

	cons_printf(portid, "\nIPv4\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_ip4[NC_IP4_IN_OCTETS]);
	cons_printf(portid, "\t受信データグラム数\t%lu\n",		net_count_ip4[NC_IP4_IN_PACKETS]);
	cons_printf(portid, "\t受信エラーデータグラム数\t%lu\n",	net_count_ip4[NC_IP4_IN_ERR_PACKETS]);
	cons_printf(portid, "\tチェックサムエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_CKSUM]);
	cons_printf(portid, "\t長さエラー数\t%lu\n",			net_count_ip4[NC_IP4_IN_ERR_SHORT]);
	cons_printf(portid, "\tバージョンエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_VER]);
	cons_printf(portid, "\tアドレスエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_ADDR]);
	cons_printf(portid, "\tプロトコルエラー数\t%lu\n",		net_count_ip4[NC_IP4_IN_ERR_PROTO]);
	cons_printf(portid, "\tオプション入力数\t%lu\n",		net_count_ip4[NC_IP4_OPTS]);
	cons_printf(portid, "\t分割受信数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_IN]);
	cons_printf(portid, "\t分割受信フラグメント数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_FRAGS]);
	cons_printf(portid, "\t分割受信再構成成功数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_OK]);
	cons_printf(portid, "\t分割受信破棄数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_IN_DROP]);
	cons_printf(portid, "\t分割受信バッファり当て失敗数\t%lu\n",	net_count_ip4[NC_IP4_FRAG_IN_NO_BUF]);
	cons_printf(portid, "\t分割受信タイムアウト数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_IN_TMOUT]);
	cons_printf(portid, "\t送信オクテット数\t%lu\n",		net_count_ip4[NC_IP4_OUT_OCTETS]);
	cons_printf(portid, "\t送信データグラム数\t%lu\n",		net_count_ip4[NC_IP4_OUT_PACKETS]);
	cons_printf(portid, "\t送信エラーデータグラム数\t%lu\n",	net_count_ip4[NC_IP4_OUT_ERR_PACKETS]);
	cons_printf(portid, "\t分割送信数\t%lu\n",			net_count_ip4[NC_IP4_FRAG_OUT]);
	cons_printf(portid, "\t分割送信フラグメント数\t%lu\n",		net_count_ip4[NC_IP4_FRAG_OUT_FRAGS]);

	net_count_struct(portid, "ICMP", &net_count_icmp4);

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

#ifdef SUPPORT_TCP

	cons_printf(portid, "\nTCP\n");
	cons_printf(portid, "\t受信オクテット数\t%lu\n",		net_count_tcp[NC_TCP_RECV_OCTETS]);
	cons_printf(portid, "\t受信データオクテット数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DATA_OCTETS]);
	cons_printf(portid, "\t受信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_SEGS]);
	cons_printf(portid, "\t受信データセグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DATA_SEGS]);
	cons_printf(portid, "\t受信、緊急セグメント数\t%lu\n",		net_count_tcp[NC_TCP_RECV_URG_SEGS]);
	cons_printf(portid, "\t受信、ヘッダ不正数\t%lu\n",		net_count_tcp[NC_TCP_RECV_BAD_HEADERS]);
	cons_printf(portid, "\t受信、チェックサム不正数\t%lu\n",	net_count_tcp[NC_TCP_RECV_BAD_CKSUMS]);
	cons_printf(portid, "\t受信、ACK 数\t%lu\n",			net_count_tcp[NC_TCP_RECV_ACKS]);
	cons_printf(portid, "\t受信、多重 ACK 数\t%lu\n",		net_count_tcp[NC_TCP_RECV_DUP_ACKS]);
	cons_printf(portid, "\t受信、RST 数\t%lu\n",			net_count_tcp[NC_TCP_RECV_RSTS]);
	cons_printf(portid, "\t受信、多重数\t%lu\n",			net_count_tcp[NC_TCP_RECV_DUP_SEGS]);
	cons_printf(portid, "\t受信、破棄数\t%lu\n",			net_count_tcp[NC_TCP_RECV_DROP_SEGS]);
	cons_printf(portid, "\t受信キュー解放数\t%lu\n",		net_count_tcp[NC_TCP_FREE_RCV_QUEUE]);
	cons_printf(portid, "\t送信データオクテット数\t%lu\n",		net_count_tcp[NC_TCP_SEND_DATA_OCTETS]);
	cons_printf(portid, "\t送信制御セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_CNTL_SEGS]);
	cons_printf(portid, "\t送信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_SEGS]);
	cons_printf(portid, "\t再送信セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_REXMIT_SEGS]);
	cons_printf(portid, "\t送信データセグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_DATA_SEGS]);
	cons_printf(portid, "\t送信、緊急セグメント数\t%lu\n",		net_count_tcp[NC_TCP_SEND_URG_SEGS]);
	cons_printf(portid, "\t送信、ACK 数\t%lu\n",			net_count_tcp[NC_TCP_SEND_ACKS]);
	cons_printf(portid, "\t送信、RST 数\t%lu\n",			net_count_tcp[NC_TCP_SEND_RSTS]);
	cons_printf(portid, "\tRTT 更新数\t%lu\n",			net_count_tcp[NC_TCP_RTT_UPDATES]);
	cons_printf(portid, "\t受動オープン数\t%lu\n",			net_count_tcp[NC_TCP_ACCEPTS]);
	cons_printf(portid, "\t能動オープン数\t%lu\n",			net_count_tcp[NC_TCP_CONNECTS]);

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

	net_count_struct(portid, "UDP",  &net_count_udp);

#endif	/* of #ifdef SUPPORT_UDP */

	tbl = nbuf_get_tbl();
	cons_printf(portid, "\nネットワークバッファ\n\tサイズ\t用意数\t使用中数\t割当要求数\t割当数\t割当てエラー数\n");
	for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
		cons_printf(portid, "\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
		                    tbl[ix].size,
		                    tbl[ix].prepares,
		                    tbl[ix].busies,
		                    tbl[ix].requests,
		                    tbl[ix].allocs,
		                    tbl[ix].errors);
		}

	cons_printf(portid, "\nネットワーク統計情報\t経過時間[us]\t%lu\t", now);
	if (now > (1000 * 3600 * 24))
		cons_printf(portid, "%3lu日 %2lu時間 %2lu分 %2lu秒\n",
		             now / (1000 * 3600 * 24),
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);
	else
		cons_printf(portid, "%2lu時間 %2lu分 %2lu秒\n",
		            (now / (1000 * 3600)) % 24,
		            (now / (1000 * 60  )) % 60,
		            (now /  1000        ) % 60);

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#if defined(SUPPORT_INET4)

#ifdef IP4_CFG_FRAGMENT

/*
 *  nbuf_stat_ip4_frag_queue -- IPv4 再構成キュー内ネットワークバッファ情報
 */

static void
nbuf_stat_ip4_frag_queue (ID portid)
{
	const T_NET_BUF_ENTRY	*tbl;
	const T_NET_BUF		*nbuf, **queue;
	int_t			ix, fix, count;

	cons_printf(portid, "\nIPv4再構成キュー内\nIndex\tTime");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; )
		cons_printf(portid, "\t%d", tbl[ix].size);
	cons_putchar(portid, '\n');

	queue = ip_get_frag_queue();
	for (fix = 0; fix < NUM_IP4_FRAG_QUEUE; fix ++) {
		if (queue[fix] != NULL) {
			cons_printf(portid, "%d\t%d", fix, GET_QIP4_HDR(queue[fix])->ttl);
			for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
				count = 0;
				for (nbuf = queue[fix]; nbuf != NULL; nbuf = GET_QIP4_HDR(nbuf)->next)
					if (nbuf->idix == ix)
						count ++;
				cons_printf(portid, "\t%d", count);
				}
			cons_putchar(portid, '\n');
			}
		}
	}

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#ifdef IP6_CFG_FRAGMENT

/*
 *  nbuf_stat_ip6_frag_queue -- IPv6 再構成キュー内ネットワークバッファ情報
 */

static void
nbuf_stat_ip6_frag_queue (ID portid)
{
	const T_NET_BUF_ENTRY	*tbl;
	const T_NET_BUF		*nbuf, **queue;
	int_t			ix, fix, count;

	cons_printf(portid, "\nIPv6再構成キュー内\nIndex\tTime");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; )
		cons_printf(portid, "\t%d", tbl[ix].size);
	cons_putchar(portid, '\n');

	queue = ip6_get_frag_queue();
	for (fix = 0; fix < NUM_IP6_FRAG_QUEUE; fix ++) {
		if (queue[fix] != NULL) {
			cons_printf(portid, "%d\t%d", fix, GET_QIP6_HDR(queue[fix])->ftim);
			for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
				count = 0;
				for (nbuf = queue[fix]; nbuf != NULL; nbuf = GET_QIP6_HDR(nbuf)->next_frag)
					if (nbuf->idix == ix)
						count ++;
				cons_printf(portid, "\t%d", count);
				}
			cons_putchar(portid, '\n');
			}
		}
	}

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #if NET_COUNT_ENABLE */

/*
 *  nbuf_status -- ネットワークバッファ情報
 */

static void
nbuf_status (ID portid, char *line)
{
	SYSTIM		now;

#if NET_COUNT_ENABLE
	const T_NET_BUF_ENTRY	*tbl;
	int_t			ix;
#endif	/* of #if NET_COUNT_ENABLE */

#ifdef SUPPORT_TCP
	T_NET_BUF		*nbuf;
	T_TCP_CEP*		cep;
	T_TCP_Q_HDR*		tqhdr;

#ifdef TCP_CFG_SWBUF_CSAVE
	T_TCP_HDR*		tcph;
#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */
#endif	/* of #ifdef SUPPORT_TCP */

	WAI_NET_CONS_PRINTF();
	get_tim(&now);
	cons_printf(portid, "ネットワークバッファ情報\t経過時間[us]\t%u\n", now);

#if NET_COUNT_ENABLE

	cons_printf(portid, "\nネットワークバッファ\n\tサイズ\t用意数\t使用中数\t割当要求数\t割当数\t割当てエラー数\n");
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
		cons_printf(portid, "\t%u\t%u\t%u\t%u\t%u\t%u\n",
		                    tbl[ix].size,
		                    tbl[ix].prepares,
		                    tbl[ix].busies,
		                    tbl[ix].requests,
		                    tbl[ix].allocs,
		                    tbl[ix].errors);
		}

#endif	/* of #if NET_COUNT_ENABLE */

#ifdef SUPPORT_TCP

	cons_printf(portid, "\nCEP内\nCEPID\n");

	for (cep = tcp_cep; cep < &tcp_cep[tmax_tcp_cepid]; cep ++) {
		cons_printf(portid, "%2d", GET_TCP_CEPID(cep));
		for (nbuf = cep->reassq; nbuf != NULL; nbuf = tqhdr->next) {
			tqhdr = GET_TCP_Q_HDR(nbuf, GET_TCP_HDR_OFFSET(nbuf));
			cons_printf(portid, "[a:%6lu-%6lu] ",
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff,
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff + tqhdr->slen);
			}

#ifdef TCP_CFG_RWBUF_CSAVE
		for (nbuf = cep->rwbufq; nbuf != NULL; nbuf = tqhdr->next) {
			tqhdr = GET_TCP_Q_HDR(nbuf, GET_TCP_HDR_OFFSET(nbuf));
			cons_printf(portid, "[r:%6lu-%6lu] ",
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff,
			            ntohl(tqhdr->seq) - cep->irs + tqhdr->soff + tqhdr->slen);
			}
#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE */

#ifdef TCP_CFG_SWBUF_CSAVE
		if (cep->swbufq != NULL) {
			tcph = GET_TCP_HDR(cep->swbufq, GET_TCP_HDR_OFFSET(cep->swbufq));
#if 0
			cons_printf(portid, "[s:%6lu-%6lu] ",
			            ntohl(tcph->seq) - cep->iss,
			            ntohl(tcph->seq) - cep->iss + (GET_IP_SDU_SIZE(cep->swbufq) - 
			                                           GET_TCP_HDR_SIZE(cep->swbufq, GET_TCP_HDR_OFFSET(cep->swbufq))));
#else
			cons_printf(portid, "[s:%6lu-%6lu,%08x,%08x] ",
			            ntohl(tcph->seq) - cep->iss,
			            ntohl(tcph->seq) - cep->iss + (GET_IP_SDU_SIZE(cep->swbufq) - 
			                                           GET_TCP_HDR_SIZE(cep->swbufq, GET_TCP_HDR_OFFSET(cep->swbufq))),
			            ntohl(tcph->seq), cep->iss);
#endif
			}
#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

		cons_putchar(portid, '\n');
		}

#endif	/* of #ifdef SUPPORT_TCP */

#if NET_COUNT_ENABLE

#if defined(SUPPORT_INET4)

#ifdef IP4_CFG_FRAGMENT

	nbuf_stat_ip4_frag_queue(portid);

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

#ifdef IP6_CFG_FRAGMENT

	nbuf_stat_ip6_frag_queue(portid);

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #if NET_COUNT_ENABLE */

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#ifdef USE_RESOLVER

/*
 *  show_dns_soa -- DNS の SOA RDATA を表示する。
 */

static ER_UINT
show_dns_soa (ID portid, uint8_t *msg, ER_UINT length, uint_t offset)
{
	T_RSLV_SOA	soa;
	ER_UINT		error;
	uint_t		rn_offset;

	if ((error = dns_analyze_soa(&soa, offset, msg, length)) < 0)
		return error;

	cons_printf(portid, "    mname:   ");
	rn_offset = show_dns_domain_name(portid, msg, offset);
	cons_putchar(portid, '\n');
	cons_printf(portid, "    rname:   ");
	show_dns_domain_name(portid, msg, rn_offset);
	cons_putchar(portid, '\n');

	cons_printf(portid, "    serial:  %d\n", soa.serial);
	cons_printf(portid, "    refresh: %d\n", soa.refresh);
	cons_printf(portid, "    retry:   %d\n", soa.retry);
	cons_printf(portid, "    expirel: %d\n", soa.expire);
	cons_printf(portid, "    minimum: %d\n", soa.minimum);

	return E_OK;
	}

/*
 *  show_dns_qdsection -- DNS の Question セクションを表示する。
 */

static ER_UINT
show_dns_qdsection (ID portid, uint8_t *msg, ER_UINT length, T_RSLV_DNS_MSG *rslv)
{
	T_RSLV_QD	qd;
	ER_UINT		offset, error;
	int		scount;

	cons_printf(portid, "question   section: %d\n", rslv->dns_hdr.qdcount);
	offset = rslv->qd_offset;
	for (scount = 1; scount <= rslv->dns_hdr.qdcount; scount ++) {
		if ((error = dns_analyze_qd(&qd, offset, msg, length)) < 0)
			return error;

		cons_printf(portid, "%2d: ", scount);
		show_dns_domain_name(portid, msg, offset);
		cons_printf(portid, "\n    type: %-4s, class: %2s\n", dns_strtype(qd.type), dns_strclass(qd.class));
		offset = error;
		}

	return E_OK;
	}

/*
 *  show_dns_section -- DNS の各セクションを表示する。
 */

static ER_UINT
show_dns_section (ID portid, uint8_t *msg, ER_UINT length, uint_t scount, uint_t offset, char *title)
{
	T_RSLV_RR	rr;
	T_IN4_ADDR	in4_addr;
	ER_UINT		error;
	int		count, dcount, col;
	T_IN6_ADDR	in6_addr;

	cons_printf(portid, "%10s section: %d\n", title, scount);
	for (count = 1; count <= scount; count ++) {
		if ((error = dns_analyze_rr(&rr, offset, msg, length)) < 0)
			return error;

		cons_printf(portid, "%2d: ", count);
		show_dns_domain_name(portid, msg, offset);
		cons_printf(portid, "\n    type: %-4s, class: %2s, TTL: %2d, len: %3d, offset: 0x%02x\n",
		            dns_strtype(rr.type), dns_strclass(rr.class), rr.ttl, rr.rdlength, rr.rdata_offset);

		switch (rr.type) {
		case DNS_TYPE_A:
			memcpy((void*)&in4_addr, (void*)(msg + rr.rdata_offset), sizeof(in4_addr));
			in4_addr = ntohl(in4_addr);
			cons_printf(portid, "    IPv4 addr: %hI\n", &in4_addr);
			break;
		case DNS_TYPE_NS:
			cons_printf(portid, "    host: ");
			show_dns_domain_name(portid, msg, rr.rdata_offset);
			cons_putchar(portid, '\n');
			break;
		case DNS_TYPE_CNAME:
			cons_printf(portid, "    host: ");
			show_dns_domain_name(portid, msg, rr.rdata_offset);
			cons_putchar(portid, '\n');
			break;
		case DNS_TYPE_SOA:
			show_dns_soa(portid, msg, length, rr.rdata_offset);
			break;
		case DNS_TYPE_PTR:
			cons_printf(portid, "     PTR: ");
			show_dns_domain_name(portid, msg, rr.rdata_offset);
			cons_putchar(portid, '\n');
			break;
		case DNS_TYPE_AAAA:
			memcpy((void*)&in6_addr, (void*)(msg + rr.rdata_offset), sizeof(in6_addr));
			cons_printf(portid, "    IPv6 addr: %lI\n", &in6_addr);
			break;
		default:
			cons_printf(portid, "    data: ");
			col = 32;
			for (dcount = 0; dcount < rr.rdlength; dcount ++) {
				cons_printf(portid, "%02x", *(msg + rr.rdata_offset + dcount));
				if (-- col == 0) {
					cons_printf(portid, "\n          ");
					col = 32;
					}
				}
			cons_putchar(portid, '\n');
			break;
			}
		}

	return E_OK;
	}
/*
 *  dns_info -- DNS 情報の表示
 */

static void
dns_info (ID portid)
{
#if defined(SUPPORT_INET6)
	T_IN6_ADDR	in6_addr;
#endif
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	in4_addr;
#endif

#if defined(SUPPORT_INET6)

	cons_printf(portid, "domain name:     %s\n", dns_in6_get_dname());

#else	/* of #if defined(SUPPORT_INET6) */

	cons_printf(portid, "domain name:     %s\n", dns_in4_get_dname());

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET6)
	dns_in6_get_addr(&in6_addr);
	cons_printf(portid, "IPv6 DNS server: ");
	if (IN6_IS_ADDR_UNSPECIFIED(&in6_addr))
		cons_printf(portid, "not available.\n");
	else
		cons_printf(portid, "%lI.\n", &in6_addr);
#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
	dns_in4_get_addr(&in4_addr);
	cons_printf(portid, "IPv4 DNS server: ");
	if (in4_addr == IPV4_ADDRANY)
		cons_printf(portid, "not available.\n");
	else
		cons_printf(portid, "%hI.\n", &in4_addr);
#endif	/* of #if defined(SUPPORT_INET4) */
	}

/*
 *  name_lookup -- ホスト名－IP アドレス変換
 */

const char *rcode_str[] = {
	"no error",
	"format error",
	"server error",
	"name error",
	"not implement",
	"refused",
	};

static void
name_lookup (ID portid, char *line)
{
	static char hostname[DBG_LINE_SIZE + 1];

	T_RSLV_DNS_MSG	rslv;
	ER_UINT		length, offset;
	ER		error;
	uint_t		flags = 0;
	uint8_t 	*msg;

	/* コマンドのオプションを設定する。*/
	line = skip_blanks(resolv_options(&flags, line, DEFAULT_API_PROTO));
	if ((flags & (DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_PROTO_IPV4)) == 0) {
		cons_printf(portid, "DNS server not available.\n");
		return;
		}

	/* 照会するホスト名・IP アドレスを解析する。*/
	resolv_hoststr(&flags, hostname, sizeof(hostname), line);

	/* 正引きでも逆引きでもプロトコル上は正引きを指定する。*/
	flags |= DNS_LUP_OPCODE_FORWARD;

	/* IPv6 アドレス、または IPv4 アドレスが指定された時は、照会タイプは PTR に設定する。*/
	if (((flags & DNS_LUP_FLAGS_NAME_MASK) == DNS_LUP_FLAGS_NAME_IPV6) ||
	    ((flags & DNS_LUP_FLAGS_NAME_MASK) == DNS_LUP_FLAGS_NAME_IPV4))
		flags = (flags & ~DNS_LUP_FLAGS_QTYPE_MASK) | DNS_LUP_FLAGS_QTYPE_PTR;

	if ((error = tget_mpf(MPF_RSLV_SRBUF, (void*)&msg, TMO_FEVR)) != E_OK) {
		cons_printf(portid, "get buffer error: %s.\n", itron_strerror(error));
		return;
		}

	if ((length = dns_lookup_host(flags | DNS_LUP_FLAGS_MSG, line, msg, DNS_UDP_MSG_LENGTH, &rslv)) < 0) {
		//cons_printf(portid, "error: %s.\n", itron_strerror(length));
		goto err_ret;
		}

	dly_tsk(1 * 1000);
	cons_printf(portid, "DNS header: flags: ");
	if (rslv.dns_hdr.code & (DNS_QR_RESPONSE | DNS_AUTHORITATIVE     |
	                         DNS_TRUN_CATION | DNS_RECURSION_DESIRED | DNS_RECURSION_AVAILABLE)) {
		cons_printf(portid, (rslv.dns_hdr.code & DNS_QR_RESPONSE)         ? "QR," : "");
		cons_printf(portid, (rslv.dns_hdr.code & DNS_AUTHORITATIVE)       ? "AA," : "");
		cons_printf(portid, (rslv.dns_hdr.code & DNS_TRUN_CATION)         ? "TC," : "");
		cons_printf(portid, (rslv.dns_hdr.code & DNS_RECURSION_DESIRED)   ? "RD," : "");
		cons_printf(portid, (rslv.dns_hdr.code & DNS_RECURSION_AVAILABLE) ? "RA," : "");
		cons_printf(portid, " ");
		}
	cons_printf(portid, "opcode: ");
	cons_printf(portid, (rslv.dns_hdr.code & DNS_OPCODE_REVERSE)      ? "RV"  : "FW");
	cons_printf(portid, (rslv.dns_hdr.code & DNS_OPCODE_STATUS)       ? ",ST" : "");
	cons_printf(portid, ", rcode: %s.\n",
	            (rslv.dns_hdr.code & DNS_RCODE_MASK) > DNS_RCODE_REFUSED
	            	? "6" : rcode_str[rslv.dns_hdr.code & DNS_RCODE_MASK]);

	if ((offset = show_dns_qdsection(portid, msg, length, &rslv)) < 0) {
		cons_printf(portid, "msg error: %s.\n", itron_strerror(offset));
		}
	if ((offset = show_dns_section(portid, msg, length, rslv.dns_hdr.ancount, rslv.an_offset, "answer")) < 0) {
		cons_printf(portid, "msg error: %s.\n", itron_strerror(offset));
		}
	if ((offset = show_dns_section(portid, msg, length, rslv.dns_hdr.nscount, rslv.ns_offset, "authority")) < 0) {
		cons_printf(portid, "msg error: %s.\n", itron_strerror(offset));
		}
	if ((offset = show_dns_section(portid, msg, length, rslv.dns_hdr.arcount, rslv.ar_offset, "additional")) < 0) {
		cons_printf(portid, "msg error: %s.\n", itron_strerror(offset));
		}

err_ret:
	if ((error = rel_mpf(MPF_RSLV_SRBUF, msg)) != E_OK)
		cons_printf(portid, "release buffer error: %s.\n", itron_strerror(error));
	return;
	}

#endif	 /* of #ifdef USE_RESOLVER */

#if defined(DHCP6_CLI_CFG)

/*
 *  dhcp6c_info -- DHCPv6 クライアント情報の表示
 */

static void
dhcp6c_info (ID portid)
{
	T_IN6_ADDR	svaddr;
	SYSTIM		bind_start;
	ER		error;
	uint32_t	expire, renew, rebind, deprefer;

	if ((error = dhcp6c_get_info(&svaddr, &expire, &renew, &rebind, &deprefer, &bind_start)) == E_OK) {
		cons_printf(portid, "      DHCPv6 server: %lI.\n", &svaddr);

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
	
		cons_printf(portid, "      DHCPv6 mode:   stateless.\n");

#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL

		cons_printf(portid, "      DHCPv6 mode:   statefull,\n");
		cons_printf(portid, "        Renew:       %u:%02u:%02u, Deprefer: %u:%02u:%02u,\n",
		                       renew / 3600, (   renew / 60) % 60,    renew % 60,
		                    deprefer / 3600, (deprefer / 60) % 60, deprefer % 60);
		cons_printf(portid, "        Rebind:      %u:%02u:%02u, Expire:   %u:%02u:%02u.\n",
		                      rebind / 3600, (  rebind / 60) % 60,   rebind % 60,
		                      expire / 3600, (  expire / 60) % 60,   expire % 60);

#else	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

#endif	/* of #if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS */

		}
	else if (error == E_OBJ)
		cons_printf(portid, "      DHCPv6 server: not available.\n");
	}

#endif	/* of #if defined(DHCP6_CLI_CFG) */

#if defined(DHCP4_CLI_CFG)

/*
 *  dhcp4c_info -- DHCPv4 クライアント情報の表示
 */

static void
dhcp4c_info (ID portid)
{
	T_IN4_ADDR	svaddr;
	SYSTIM		bind_start;
	ER		error;
	uint32_t	expire, renew, rebind;

	if ((error = dhcp4c_get_info(&svaddr, &expire, &renew, &rebind, &bind_start)) == E_OK) {
		cons_printf(portid, "      DHCPv4 server: %hI,\n", &svaddr);
		cons_printf(portid, "        Renew:       %u:%02u:%02u,\n", 
		                      renew  / 3600, (renew  / 60) % 60, renew % 60);
		cons_printf(portid, "        Rebind:      %u:%02u:%02u, Expire:   %u:%02u:%02u.\n",
		                      rebind / 3600, (rebind / 60) % 60, rebind % 60,
		                      expire / 3600, (expire / 60) % 60, expire % 60);
		}
	else if (error == E_OBJ)
		cons_printf(portid, "      DHCPv4 server: not available.\n");
	}

#endif	/* of #if defined(DHCP4_CLI_CFG) */

#if defined(DHCP6_CLI_CFG) || defined(DHCP4_CLI_CFG)

/*
 *  dhcpc -- DHCP クライアント情報の表示と制御
 */

static void
dhcpc (ID portid, char *line)
{
	WAI_NET_CONS_PRINTF();

	switch (*line) {

#if defined(DHCP6_CLI_CFG)
	case '6':			/* DHCPv6 の制御 */
		switch (*(line + 1)) {
		case 'r':		/* アドレス情報を解放する。*/
			dhcp6c_rel_info();
			break;

		case 'n':		/* アドレス情報を再取得する。*/
			dhcp6c_renew_info();
			break;

		default:
			dhcp6c_info(portid);
			break;
			}
		break;
#endif	/* of #if defined(DHCP6_CLI_CFG) */

#if defined(DHCP4_CLI_CFG)
	case '4':		/* DHCPv4 の制御 */
		switch (*(line + 1)) {
		case 'r':		/* アドレス情報を解放する。*/
			dhcp4c_rel_info();
			break;

		case 'n':		/* アドレス情報を再取得する。*/
			dhcp4c_renew_info();
			break;

		default:
			dhcp4c_info(portid);
			break;
			}
		break;
#endif	/* of #if defined(DHCP4_CLI_CFG) */

	default:

#if defined(DHCP6_CLI_CFG)
		dhcp6c_info(portid);
#endif

#if defined(DHCP4_CLI_CFG)
		dhcp4c_info(portid);
#endif
		break;
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(DHCP6_CLI_CFG) || defined(DHCP4_CLI_CFG) */

#if defined(SUPPORT_INET6)

#if NUM_ND6_DEF_RTR_ENTRY > 0

/*
 *  defrtrlist_status -- ディフォルトルータ・リスト状態の出力
 */

static void
defrtrlist_status (ID portid, char *line)
{
	const T_DEF_ROUTER	*dr;
	SYSTIM			now;
	int_t			rix, pix, pcnt, ptitle;
	uint_t			count, mask;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Default Router List Status\n"
	            "IX Expire Lifetime flags prefix");
	for (ptitle = NUM_ND6_PREFIX_ENTRY * 2 - sizeof("prefix"); ptitle -- > 0; )
		cons_printf(portid, " ");
	cons_printf(portid, " IP Address\n");

	/* expire の単位は [us]。*/
	get_tim(&now);
	dr = nd6_get_drl(&count);
	for (rix = 0; rix < count; rix ++) {
		if (dr[rix].state == ND6_DEF_RTR_BUSY) {
			cons_printf(portid, "%2d", rix);
			if (dr[rix].lifetime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "  INFIN    INFIN");
			else
				cons_printf(portid, " %6d %8d",
				            (uint32_t)(dr[rix].expire - now) <= 0
				                ? 0 : (dr[rix].expire - now) / SYSTIM_HZ,
				            dr[rix].lifetime / SYSTIM_HZ);
			cons_printf(portid, "    %02x", dr[rix].flags);
			pcnt = NUM_ND6_PREFIX_ENTRY;
			ptitle = NUM_ND6_PREFIX_ENTRY * 2 - sizeof(" prefix");
			mask = 1;
			for (pix = 0; pix < NUM_ND6_PREFIX_ENTRY; pix ++) {
				if ((dr[rix].plistmap & mask) != 0) {
					if (pcnt == NUM_ND6_PREFIX_ENTRY)
						cons_printf(portid, " ");
					else
						cons_printf(portid, ",");
					cons_printf(portid, "%d", pix);
					pcnt --;
					ptitle -= 2;
					}
				mask <<= 1;
				}
			while (pcnt --) {
				cons_printf(portid, "  ");
				ptitle -= 2;
				}
			while (ptitle -- > 0)
				cons_printf(portid, " ");
			cons_printf(portid, " %lI\n",  &dr[rix].addr);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

/*
 *  prefixlist_status -- プレフィックス・リスト状態の出力
 */

static void
prefixlist_status (ID portid, char *line)
{
	const T_ND6_PREFIX	*pr;
	SYSTIM			now;
	int_t			rix, pix, rcnt, rtitle;
	uint_t			mask;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "Prefix List Status\n"
	            "IX  Expire Lifetime flags router");
	for (rtitle = NUM_ND6_DEF_RTR_ENTRY * 2 - sizeof("router"); rtitle -- > 0; )
		cons_printf(portid, " ");
	cons_printf(portid, " Len Prefix\n");

	/* expire の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;
	pr = nd6_get_prl();
	for (pix = 0; pix < NUM_ND6_PREFIX_ENTRY; pix ++) {
		if ((pr[pix].flags & ND6_PREFIX_STATE_BUSY) != 0) {
			cons_printf(portid, "%2d", pix);
			if (pr[pix].vltime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "  INFIN    INFIN");
			else
				cons_printf(portid, " %7d %8d",
				            (uint32_t)(pr[pix].expire - now) <= 0
				                ? 0 : (pr[pix].expire - now),
				             pr[pix].vltime);
			cons_printf(portid, "    %02x", pr[pix].flags);
			rcnt = NUM_ND6_DEF_RTR_ENTRY;
			rtitle = NUM_ND6_DEF_RTR_ENTRY * 2 - sizeof(" router");
			mask = 1;
			for (rix = 0; rix < NUM_ND6_DEF_RTR_ENTRY; rix ++) {
				if ((pr[pix].routermap & mask) != 0) {
					if (rcnt == NUM_ND6_DEF_RTR_ENTRY)
						cons_printf(portid, " ");
					else
						cons_printf(portid, ",");
					cons_printf(portid, "%d", rix);
					rcnt --;
					rtitle -= 2;
					}
				mask <<= 1;
				}
			while (rcnt --) {
				cons_printf(portid, "  ");
				rtitle -= 2;
				}
			while (rtitle -- > 0)
				cons_printf(portid, " ");
			cons_printf(portid, "%4d %lI\n",  pr[pix].prefix_len, &pr[pix].prefix);
			}
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_ND6_DEF_RTR_ENTRY > 0 */

#if NUM_IN6_ROUTE_ENTRY > 0

/*
 *  routing6_table_status -- 経路表（IPv6）の出力
 */

static void
routing6_table_status (ID portid, char *line)
{
	SYSTIM	now;
	int_t	ix;

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "\nRouting Table Status (IPv6)\n"
	            "IX Expire flags Prefix Target                         Gateway\n");

	for (ix = 0; ix < NUM_IN6_STATIC_ROUTE_ENTRY; ix ++) {
		cons_printf(portid, "%2d STATIC     - %6d %30lI %lI\n",
		            ix,
		             routing6_tbl[ix].prefix_len,
		            &routing6_tbl[ix].target,
		            &routing6_tbl[ix].gateway);
		            
		}

	/* expire の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;

	for ( ; ix < NUM_IN6_ROUTE_ENTRY; ix ++)
		if (routing6_tbl[ix].flags & IN_RTF_DEFINED)
			cons_printf(portid, "%2d %6d    %02x %6d %30lI %lI\n",
			            ix,
			            (uint32_t)(routing6_tbl[ix].expire - now) <= 0
			                ? 0 : routing6_tbl[ix].expire - now,
			             routing6_tbl[ix].flags,
			             routing6_tbl[ix].prefix_len,
			            &routing6_tbl[ix].target,
			            &routing6_tbl[ix].gateway);

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_IN6_ROUTE_ENTRY > 0 */

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#if NUM_IN4_ROUTE_ENTRY > 0

/*
 *  routing4_table_status -- 経路表（IPv4）の出力
 */

static void
routing4_table_status (ID portid, char *line)
{
	int_t	ix;

	if ('0' <= *(line = skip_blanks(line)) && *line <= '9') {
		T_IN4_ADDR	target, mask, gateway;

		ix = atoi(line);
		while ('0' <= *line && *line <= '9')
			line ++;
		line = get_ipv4addr(&target,  skip_blanks(line));
		line = get_ipv4addr(&mask,    skip_blanks(line));
		       get_ipv4addr(&gateway, skip_blanks(line));
		in4_add_route(ix, target, mask, gateway);
		}

	WAI_NET_CONS_PRINTF();
	cons_printf(portid,
	            "\nRouting Table Status (IPv4)\n"
	            "IX Target          Mask            Gateway         Expire\n");

	for (ix = 0; ix < NUM_IN4_ROUTE_ENTRY; ix ++) {
		if ((routing4_tbl[ix].flags & IN_RTF_DEFINED) == 0)
			;
		else if ((routing4_tbl[ix].flags & IN_RTF_REDIRECT) != 0)
			cons_printf(portid, "%2d %15hI %15hI %15hI %4d\n",
			            ix,
			            &routing4_tbl[ix].target,
			            &routing4_tbl[ix].mask,
			            &routing4_tbl[ix].gateway,
			            &routing4_tbl[ix].expire);
		else
			cons_printf(portid, "%2d %15hI %15hI %hI\n",
			            ix,
			            &routing4_tbl[ix].target,
			            &routing4_tbl[ix].mask,
			            &routing4_tbl[ix].gateway);
		            
		}

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if NUM_IN4_ROUTE_ENTRY > 0 */

#endif	/* of #if defined(SUPPORT_INET4) */

/*
 *  routing_status -- 経路情報
 */

static void
routing_status (ID portid, char *line)
{
	switch (*line) {

#if defined(SUPPORT_INET6) && NUM_ND6_DEF_RTR_ENTRY > 0

	case 'l':		/* ディフォルトルータ・リスト情報 */
		defrtrlist_status(portid, line + 1);
		break;

	case 'p':		/* プレフィックス・リスト情報 */
		prefixlist_status(portid, line + 1);
		break;

#endif	/* #if defined(SUPPORT_INET6) && NUM_ND6_DEF_RTR_ENTRY > 0 */

	default:

		/* 経路表情報 */

#if defined(SUPPORT_INET6) && NUM_IN6_ROUTE_ENTRY > 0
		routing6_table_status(portid, line);
#endif

#if defined(SUPPORT_INET4) && NUM_IN4_ROUTE_ENTRY > 0
		routing4_table_status(portid, line);
#endif

		break;
		}
	}

/*
 *  dbg_cons_network_status -- ネットワークの状態の出力
 */

void
dbg_cons_network_status (ID portid, char *line)
{
	switch (*line) {

#ifdef SUPPORT_ETHER

	case 'a':		/* IF アドレス情報 */

#if defined(SUPPORT_INET6)
		ifa6_status(portid, line + 1);
#endif
#if defined(SUPPORT_INET4)
		ifa4_status(portid, line + 1);
#endif

		break;

#endif	/* of #ifdef SUPPORT_ETHER */

	case 'b':		/* ネットワークバッファ情報 */
		nbuf_status(portid, line + 1);
		break;

#if NET_COUNT_ENABLE

	case 'c':		/* ネットワーク統計情報 */
		net_count(portid, line + 1);
		break;

#endif	/* of #if NET_COUNT_ENABLE */

	case 'r':		/* 経路情報 */
		routing_status(portid, line + 1);
		break;

#ifdef USE_RESOLVER

	case 's':		/* name lookup */
		if (*(line = skip_blanks(line + 1)) == '\0')
			dns_info(portid);
		else
			name_lookup(portid, line);
		break;

#endif	 /* of #ifdef USE_RESOLVER */

#ifdef SUPPORT_TCP

	case 't':		/* TCP status */
		tcp_status(portid, line + 1);
		break;

#endif	 /* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

	case 'u':		/* UDP status */
		udp_status(portid, line + 1);
		break;

#endif	 /* of #ifdef SUPPORT_UDP */

	default:
		cons_printf(portid, "no such command: '%c%c'.\n", *(line - 1), *line);
		SIG_NET_CONS_PRINTF();
		FLUSH_SND_BUFF();
		break;
		}
	}

/*
 *  dbg_cons_ifconfig -- ネットワークインタフェースの構成情報
 */

void
dbg_cons_ifconfig (ID portid, char *line)
{
#ifdef SUPPORT_ETHER

	T_IF_SOFTC	*ic;

	WAI_NET_CONS_PRINTF();
	ic = IF_ETHER_NIC_GET_SOFTC();
	cons_printf(portid, "ether: %M\n", ic->ifaddr.lladdr);
	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();

#endif	/* of #ifdef SUPPORT_ETHER */
	}

#if defined(NETAPP_IP6_CFG)

/*
 *  dbg_cons_ifconfig6 -- ネットワークインタフェース（IPv6）の構成情報
 */

void
dbg_cons_ifconfig6 (ID portid, char *line)
{
#ifdef SUPPORT_ETHER

	T_IFNET		*ifp = IF_GET_IFNET();
	int_t		ix;
	SYSTIM		now;

	WAI_NET_CONS_PRINTF();

	/* expire と vltime の単位は [s]。*/
	get_tim(&now);
	now /= SYSTIM_HZ;

	cons_printf(portid, "IPv6:\nIX   Expire Preffered RTR PFX Flags IP Address\n");
	for (ix = 0; ix < NUM_IN6_IFADDR_ENTRY; ix ++) {
		if (ifp->in6_ifaddrs[ix].flags & IN6_IFF_DEFINED) {
			cons_printf(portid, " %d", ix);
			if (ifp->in6_ifaddrs[ix].lifetime.vltime == ND6_INFINITE_LIFETIME)
				cons_printf(portid, "    INFIN     INFIN");
			else
				cons_printf(portid, " %8d %9d",
				                    (uint32_t)(ifp->in6_ifaddrs[ix].lifetime.expire - now) <= 0 
				                         ? 0 : ifp->in6_ifaddrs[ix].lifetime.expire - now,
				                    (uint32_t)(ifp->in6_ifaddrs[ix].lifetime.preferred - now) <= 0
				                         ? 0 : ifp->in6_ifaddrs[ix].lifetime.preferred - now);
			if (ifp->in6_ifaddrs[ix].router_index == IN6_RTR_IX_UNREACH)
				cons_printf(portid, "   -");
			else
				cons_printf(portid, " %3d", ifp->in6_ifaddrs[ix].router_index);
			if (ifp->in6_ifaddrs[ix].prefix_index == ND6_PREFIX_IX_INVALID)
				cons_printf(portid, "   -");
			else
				cons_printf(portid, " %3d", ifp->in6_ifaddrs[ix].prefix_index);
			cons_printf(portid, "    %02x %lI\n",
			                     ifp->in6_ifaddrs[ix].flags,
			                    &ifp->in6_ifaddrs[ix].addr);
			}
		}

#ifdef DHCP6_CLI_CFG
	dhcp6c_info(portid);
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();

#endif	/* of #ifdef SUPPORT_ETHER */
	}

#endif	/* of #if defined(NETAPP_IP6_CFG) */

#if defined(NETAPP_IP4_CFG)

/*
 *  dbg_cons_ifconfig4 -- ネットワークインタフェース（IPv4）の構成情報
 */

void
dbg_cons_ifconfig4 (ID portid, char *line)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_IN4_ADDR	bc;

#ifdef SUPPORT_ETHER

	if (*(line = skip_blanks(line))) {
		T_IN4_ADDR	addr, mask;

		get_ipv4addr(&mask, skip_blanks(get_ipv4addr(&addr, line)));
		in4_add_ifaddr(addr, mask);
		}

#endif	/* of #ifdef SUPPORT_ETHER */

	WAI_NET_CONS_PRINTF();

	bc = (ifp->in4_ifaddr.addr & ifp->in4_ifaddr.mask) | ~ifp->in4_ifaddr.mask;
	cons_printf(portid,
	            "IPv4: %hI, mask: %hI, broadcast: %hI\n",
	            &ifp->in4_ifaddr.addr,
	            &ifp->in4_ifaddr.mask,
	            &bc);

#ifdef DHCP4_CLI_CFG
	dhcp4c_info(portid);
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#endif	/* of #if defined(NETAPP_IP4_CFG) */

/*
 *  dbg_cons_show_config -- コンフィギュレーション設定の表示
 */

void
dbg_cons_show_config (ID portid, char *line)
{
#if defined(SUPPORT_ETHER)
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	addr;
#endif
#endif	/* of #if defined(SUPPORT_ETHER) */

	cons_printf(portid, "DHCP:\n");
	cons_printf(portid, "  DHCP6_CLI_CFG: ");

#if defined(DHCP6_CLI_CFG)

	cons_printf(portid, "On\n           Mode: ");

#if DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATELESS
	cons_printf(portid, "STATELESS\n");
#elif DHCP6_CLI_CFG_MODE == DHCP6_CLI_CFG_STATEFULL
	cons_printf(portid, "STATEFULL\n");
#endif

#else	/* of #if define(DHCP6_CLI_CFG) */

	cons_printf(portid, "Off\n");

#endif	/* of #if define(DHCP6_CLI_CFG) */

	cons_printf(portid, "  DHCP4_CLI_CFG: ");

#if defined(DHCP4_CLI_CFG)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "Resolver:\n");
	cons_printf(portid, "  USE_RESOLVER: ");

#if defined(USE_RESOLVER)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#if defined(SUPPORT_TCP)

	cons_printf(portid, "TCP:\n");

	cons_printf(portid, "  TCP_CFG_OPT_MSS:      ");
#if defined(TCP_CFG_OPT_MSS)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_DELAY_ACK:    ");
#if defined(TCP_CFG_DELAY_ACK)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_ALWAYS_KEEP:  ");
#if defined(TCP_CFG_ALWAYS_KEEP)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_RWBUF_CSAVE:  ");
#if defined(TCP_CFG_RWBUF_CSAVE_ONLY)
	cons_printf(portid, "Only, ");
#if defined(TCP_CFG_RWBUF_CSAVE_MAX_QUEUES)
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: %2d\n",
	                     TCP_CFG_RWBUF_CSAVE_MAX_QUEUES);
#else
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: Undefined\n");
#endif
#elif defined(TCP_CFG_RWBUF_CSAVE)
	cons_printf(portid, "On,   ");
#if defined(TCP_CFG_RWBUF_CSAVE_MAX_QUEUES)
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: %2d\n",
	                     TCP_CFG_RWBUF_CSAVE_MAX_QUEUES);
#else
	cons_printf(portid, "TCP_CFG_RWBUF_CSAVE_MAX_QUEUES: Undefined\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_SWBUF_CSAVE:  ");
#if defined(TCP_CFG_SWBUF_CSAVE_ONLY)
	cons_printf(portid, "Only, TCP_CFG_SWBUF_CSAVE_MAX_SIZE: %4d\n"
	                    "                        "
	                    "      TCP_CFG_SWBUF_CSAVE_MIN_SIZE: %4d\n",
	                    TCP_CFG_SWBUF_CSAVE_MAX_SIZE, 
                            TCP_CFG_SWBUF_CSAVE_MIN_SIZE);
#elif defined(TCP_CFG_SWBUF_CSAVE)
	cons_printf(portid, "On,   TCP_CFG_SWBUF_CSAVE_MAX_SIZE: %4d\n"
	                    "                        "
	                    "      TCP_CFG_SWBUF_CSAVE_MIN_SIZE: %4d\n",
	                    TCP_CFG_SWBUF_CSAVE_MAX_SIZE, 
                            TCP_CFG_SWBUF_CSAVE_MIN_SIZE);
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_NON_BLOCKING: ");
#if defined(TCP_CFG_NON_BLOCKING)
#if defined(USE_TCP_NON_BLOCKING)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_NON_BLOCKING_COMPAT14: ");
#if defined(TCP_CFG_NON_BLOCKING_COMPAT14)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  TCP_CFG_EXTENTIONS:   ");
#if defined(TCP_CFG_EXTENTIONS)
#if defined(USE_TCP_EXTENTIONS)
	cons_printf(portid, "Use\n");
#if defined(SUPPORT_INET6)
	cons_printf(portid, "  NUM_VRID_TCP6_REPS:   %3d\n", NUM_VRID_TCP6_REPS);
	cons_printf(portid, "  NUM_VRID_TCP6_CEPS:   %3d\n", NUM_VRID_TCP6_CEPS);
#endif
#if defined(SUPPORT_INET4)
	cons_printf(portid, "  NUM_VRID_TCP4_REPS:   %3d\n", NUM_VRID_TCP4_REPS);
	cons_printf(portid, "  NUM_VRID_TCP4_CEPS:   %3d\n", NUM_VRID_TCP4_CEPS);
#endif
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  USE_COPYSAVE_API:     ");
#if defined(USE_COPYSAVE_API)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "Off\n");
#endif

#if NUM_TCP_TW_CEP_ENTRY > 0
	cons_printf(portid, "  TCP_TW_CEP:           On, NUM_TCP_TW_CEP_ENTRY:     %d\n", NUM_TCP_TW_CEP_ENTRY);
#else
	cons_printf(portid, "  TCP_TW_CEP:           Off\n");
#endif

#if defined(TCP_CFG_TRACE)
	cons_printf(portid, "  TCP_CFG_TRACE:        On, TCP_CFG_TRACE_LPORTNO:    %d\n", TCP_CFG_TRACE_LPORTNO);
#if defined(SUPPORT_INET4)
	addr = TCP_CFG_TRACE_IPV4_RADDR;
	cons_printf(portid, "                            TCP_CFG_TRACE_IPV4_RADDR: %hI\n", &addr);
#endif
	cons_printf(portid, "                            TCP_CFG_TRACE_RPORTNO:    %d\n", TCP_CFG_TRACE_RPORTNO);
#else
	cons_printf(portid, "  TCP_CFG_TRACE:        Off\n");
#endif

	cons_printf(portid, "  MAX_TCP_SND_SEG:      %4d\n", MAX_TCP_SND_SEG);
	cons_printf(portid, "  DEF_TCP_SND_SEG:      %4d\n", DEF_TCP_SND_SEG);
	cons_printf(portid, "  MAX_TCP_RCV_SEG:      %4d\n", MAX_TCP_RCV_SEG);
	cons_printf(portid, "  DEF_TCP_RCV_SEG:      %4d\n", DEF_TCP_RCV_SEG);
	cons_printf(portid, "  MAX_TCP_REALLOC_SIZE: %4d\n", MAX_TCP_REALLOC_SIZE);

#endif	/* of #if defined(SUPPORT_TCP) */

#if defined(SUPPORT_UDP)

	cons_printf(portid, "UDP:\n");

	cons_printf(portid, "  UDP_CFG_IN_CHECKSUM:  ");
#if defined(UDP_CFG_IN_CHECKSUM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_OUT_CHECKSUM: ");
#if defined(UDP_CFG_OUT_CHECKSUM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_NON_BLOCKING: ");
#if defined(UDP_CFG_NON_BLOCKING)
#if defined(USE_UDP_NON_BLOCKING)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_NON_BLOCKING_COMPAT14: ");
#if defined(UDP_CFG_NON_BLOCKING_COMPAT14)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  UDP_CFG_EXTENTIONS:   ");
#if defined(UDP_CFG_EXTENTIONS)
#if defined(USE_UDP_EXTENTIONS)
	cons_printf(portid, "Use\n");
#else
	cons_printf(portid, "On\n");
#endif
	cons_printf(portid, "  NUM_VRID_UDP4_CEPS:   %3d\n", NUM_VRID_UDP4_CEPS);
	cons_printf(portid, "  NUM_VRID_UDP6_CEPS:   %3d\n", NUM_VRID_UDP6_CEPS);
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  NUM_DTQ_UDP_RCVQ:     %3d\n", NUM_DTQ_UDP_RCVQ);
	cons_printf(portid, "  NUM_DTQ_UDP_OUTPUT:   %3d\n", NUM_DTQ_UDP_OUTPUT);

#endif	/* of #if defined(SUPPORT_UDP) */

	cons_printf(portid, "ICMP:\n");

	cons_printf(portid, "  ICMP_CFG_CALLBACK_ECHO_REPLY: ");
#if defined(ICMP_CFG_CALLBACK_ECHO_REPLY)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ICMP_REPLY_ERROR:             ");
#if defined(ICMP_REPLY_ERROR)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#if defined(NUM_REDIRECT_ROUTE_ENTRY)
#if NUM_REDIRECT_ROUTE_ENTRY > 0
	cons_printf(portid, "  TMO_IN_REDIRECT: %4d[s]\n", TMO_IN_REDIRECT / NET_TIMER_HZ);
#endif
#endif	/* of #if defined(NUM_REDIRECT_ROUTE_ENTRY) */

#if defined(SUPPORT_INET6)

	cons_printf(portid, "IPv6:\n");

	cons_printf(portid, "  IP6_IPV4_MAPPED_ADDR:   ");
#if defined(IP6_CFG_IPV4_MAPPED_ADDR)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  Routing Table:\n");
	cons_printf(portid, "    NUM_IN6_STATIC_ROUTE_ENTRY:   %d\n", NUM_IN6_STATIC_ROUTE_ENTRY);
	cons_printf(portid, "    NUM_IN6_REDIRECT_ROUTE_ENTRY: %d\n", NUM_IN6_REDIRECT_ROUTE_ENTRY);

	cons_printf(portid, "ND:\n");
	cons_printf(portid, "  TMO_ND6_RTR_SOL_DELAY:    %5d[us]\n", TMO_ND6_RTR_SOL_DELAY);
	cons_printf(portid, "  TMO_ND6_RTR_SOL_INTERVAL: %5d[us]\n", TMO_ND6_RTR_SOL_INTERVAL);
	cons_printf(portid, "  NUM_IP6_DAD_COUNT:        %5d\n",     NUM_IP6_DAD_COUNT);
	cons_printf(portid, "  NUM_ND6_CACHE_ENTRY:      %5d\n",     NUM_ND6_CACHE_ENTRY);

#if defined(SUPPORT_ETHER)

	cons_printf(portid, "  NUM_ND6_DEF_RTR_ENTRY     %5d\n",     NUM_ND6_DEF_RTR_ENTRY);
	cons_printf(portid, "  NUM_ND6_RTR_SOL_RETRY     %5d\n",     NUM_ND6_RTR_SOL_RETRY);

	cons_printf(portid, "  IP6_CFG_AUTO_LINKLOCAL: ");
#if defined(IP6_CFG_AUTO_LINKLOCAL)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_ETHER) */

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

	cons_printf(portid, "IPv4:\n");

	cons_printf(portid, "  IP4_CFG_FRAGMENT: ");

#if defined(IP4_CFG_FRAGMENT)
	cons_printf(portid, "On\n");
	cons_printf(portid, "    NUM_IP4_FRAG_QUEUE:       %4d\n", NUM_IP4_FRAG_QUEUE);
	cons_printf(portid, "    IP4_CFG_FRAG_REASSM_SIZE: %4d\n", IP4_CFG_FRAG_REASSM_SIZE);
#else
	cons_printf(portid, "Off\n");
#endif	/* of #if defined(IP4_CFG_FRAGMENT) */

#if defined(SUPPORT_ETHER)
	addr = IPV4_ADDR_LOCAL;
	cons_printf(portid, "  IPV4_ADDR_LOCAL:      %hI\n", &addr);
	addr = IPV4_ADDR_LOCAL_MASK;
	cons_printf(portid, "  IPV4_ADDR_LOCAL_MASK: %hI\n", &addr);
	addr = IPV4_ADDR_DEFAULT_GW;
	cons_printf(portid, "  IPV4_ADDR_DEFAULT_GW: %hI\n", &addr);
#endif	/* of #if defined(SUPPORT_ETHER) */

	cons_printf(portid, "  Routing Table:\n");
	cons_printf(portid, "    NUM_IN4_STATIC_ROUTE_ENTRY:   %d\n", NUM_IN4_STATIC_ROUTE_ENTRY);
	cons_printf(portid, "    NUM_IN4_REDIRECT_ROUTE_ENTRY: %d\n", NUM_IN4_REDIRECT_ROUTE_ENTRY);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET4) && defined(SUPPORT_ETHER)

	cons_printf(portid, "ARP:\n");
	cons_printf(portid, "  ARP_CACHE_KEEP: %4d[s]\n", ARP_CACHE_KEEP / NET_TIMER_HZ);

#endif	/* of #if defined(SUPPORT_INET4) && defined(SUPPORT_ETHER) */

#if defined(SUPPORT_ETHER)

	cons_printf(portid, "Ethernet:\n");

	cons_printf(portid, "  NUM_DTQ_ETHER_OUTPUT:    %3d\n", NUM_DTQ_ETHER_OUTPUT);

	cons_printf(portid, "  ETHER_CFG_ACCEPT_ALL:    ");
#if defined(ETHER_CFG_ACCEPT_ALL)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_UNEXP_WARNING: ");
#if defined(ETHER_CFG_UNEXP_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_802_WARNING:   ");
#if defined(ETHER_CFG_802_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_CFG_MCAST_WARNING: ");
#if defined(ETHER_CFG_MCAST_WARNING)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	cons_printf(portid, "  ETHER_COLLECT_ADDR:      ");
#if defined(ETHER_COLLECT_ADDR)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_ETHER) */

#if defined(SUPPORT_PPP)

	cons_printf(portid, "PPP:\n");

	cons_printf(portid, "  HDLC_PORTNO: %d", HDLC_PORTID);

	cons_printf(portid, "  PPP_IDLE_TIMEOUT: ");
#if defined(PPP_IDLE_TIMEOUT)
	cons_printf(portid, "  %4d[s]\n", PPP_IDLE_TIMEOUT / NET_TIMER_HZ);
#else
	cons_printf(portid, "  Off\n");
#endif

	cons_printf(portid, "  PPP_CFG_MODEM: ");
#if defined(PPP_CFG_MODEM)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

#endif	/* of #if defined(SUPPORT_PPP) */

	cons_printf(portid, "Network Buffer:\n");

#if defined(NUM_MPF_NET_BUF_64)
	cons_printf(portid, "  NUM_MPF_NET_BUF_64:        %2d\n", NUM_MPF_NET_BUF_64);
#endif

#if defined(NUM_MPF_NET_BUF_CSEG)
	cons_printf(portid, "  NUM_MPF_NET_BUF_CSEG:      %2d\n", NUM_MPF_NET_BUF_CSEG);
#endif

#if defined(NUM_MPF_NET_BUF_128)
	cons_printf(portid, "  NUM_MPF_NET_BUF_128:       %2d\n", NUM_MPF_NET_BUF_128);
#endif

#if defined(NUM_MPF_NET_BUF_256)
	cons_printf(portid, "  NUM_MPF_NET_BUF_256:       %2d\n", NUM_MPF_NET_BUF_256);
#endif

#if defined(NUM_MPF_NET_BUF_512)
	cons_printf(portid, "  NUM_MPF_NET_BUF_512:       %2d\n", NUM_MPF_NET_BUF_512);
#endif

#if defined(NUM_MPF_NET_BUF_IP_MSS)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IP_MSS:    %2d\n", NUM_MPF_NET_BUF_IP_MSS);
#endif

#if defined(NUM_MPF_NET_BUF_1024)
	cons_printf(portid, "  NUM_MPF_NET_BUF_1024:      %2d\n", NUM_MPF_NET_BUF_1024);
#endif

#if defined(NUM_MPF_NET_BUF_IPV6_MMTU)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IPV6_MMTU: %2d\n", NUM_MPF_NET_BUF_IPV6_MMTU);
#endif

#if defined(NUM_MPF_NET_BUF_IF_PDU)
	cons_printf(portid, "  NUM_MPF_NET_BUF_IF_PDU:    %2d\n", NUM_MPF_NET_BUF_IF_PDU);
#endif

#if defined(NUM_MPF_NET_BUF_REASSM)
	cons_printf(portid, "  NUM_MPF_NET_BUF_REASSM:    %2d\n", NUM_MPF_NET_BUF_REASSM);
#endif

#if defined(NUM_MPF_NET_BUF6_65536)
	cons_printf(portid, "  NUM_MPF_NET_BUF6_65536:    %2d\n", NUM_MPF_NET_BUF6_65536);
#endif

	cons_printf(portid, "Network Counter Mask: %08x\n", NET_COUNT_ENABLE);

	cons_printf(portid, "SUPPORT_MIB: ");
#if defined(SUPPORT_MIB)
	cons_printf(portid, "On\n");
#else
	cons_printf(portid, "Off\n");
#endif

	SIG_NET_CONS_PRINTF();
	FLUSH_SND_BUFF();
	}

#ifdef USE_PING

/*
 *  ping コマンド
 */

void
dbg_cons_ping_comd (ID portid, char *line)
{
	int_t	tmo, size;
	char	apip = DEFAULT_API_PROTO;

#if defined(SUPPORT_INET6)
	T_IN6_ADDR	addr;
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	addr4;
#endif
#else
	T_IN4_ADDR	addr;
#endif

#if defined(PING_CFG_I6RLP)

	static const char i6rlp_pmtu_str1[] = " FF1E::1:2 1 1452";
	static const char i6rlp_pmtu_str2[] = " FF1E::1:2 1 1352";
	static const char i6rlp_pmtu_str3[] = " fe80::0200:00ff:fe00:0100 1 2";

	if      (apip == '1')
		strcpy(line, i6rlp_pmtu_str1);
	else if (apip == '2')
		strcpy(line, i6rlp_pmtu_str2);
	else if (apip == '3')
		strcpy(line, i6rlp_pmtu_str3);

#endif	/* of #ifdef PING_CFG_I6RLP */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if ('0' <= *line && *line <= '9') {
		if (*line == '6')
			apip = API_PROTO_IPV6;
		if (*line == '4')
			apip = API_PROTO_IPV4;
		line ++;
		}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	if ((line = lookup_ipaddr(&addr, line, apip)) == NULL) {
		syslog(LOG_NOTICE, "[PING] unknown host.");
		return;
		}

	line = skip_blanks(line);
	if ('0' <= *line && *line <= '9')
		line = get_int(&tmo, line);
	else
		tmo = 3;

	line = skip_blanks(line);
	if ('0' <= *line && *line <= '9')
		line = get_int(&size, line);
	else
		size = 64;

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

	if (apip == API_PROTO_IPV6) {
		syslog(LOG_NOTICE, "[PING6] size: %d, tmo: %d, host: %s", size, tmo, ipv62str(NULL, &addr));
		ping6(&addr, (uint_t)tmo, (uint_t)size);
		}
	else {
		addr4 = ntohl(addr.s6_addr32[3]);
		syslog(LOG_NOTICE, "[PING4] size: %d, tmo: %d, host: %s", size, tmo, ip2str(NULL, &addr4));
		ping4(&addr4, (uint_t)tmo, (uint_t)size);
		}

#else /* of #if defined(SUPPORT_INET4) */

	syslog(LOG_NOTICE, "[PING6] size: %d, tmo: %d, host: %s", size, tmo, ipv62str(NULL, &addr));
	ping6(&addr, (uint_t)tmo, (uint_t)size);

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */


	syslog(LOG_NOTICE, "[PING4] size: %d, tmo: %d, host: %s", size, tmo, ip2str(NULL, &addr));
	ping4(&addr, (uint_t)tmo, (uint_t)size);

#endif	/* of #if defined(SUPPORT_INET6) */

	}

#endif	/* of #ifdef USE_PING */

#ifdef USE_DBG_CONS_PARSER

/*
 *  デバッグコマンド解析
 */

void
dbg_parse (ID portid, char *line)
{
	int_t	cepid;
	ER	error = E_OK;

#if defined(SUPPORT_UDP) && defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	int_t	udp_cep_ipproto;
#endif

	switch (*line) {

	case 'c':		/* cancel CEP */
		switch (*(line + 1)) {

		case 'f':	/* show configurations */
			dbg_cons_show_config(portid, line + 1);
			break;

#ifdef SUPPORT_TCP

		case 't':	/* cancel TCP */
			cepid = atoi(line = skip_blanks(line + 2));
			while ('0' <= *line && *line <= '9')
				line ++;
			if ((error = tcp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
				cons_printf(portid, "[TCP CAN CEP] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef SUPPORT_TCP */

#ifdef SUPPORT_UDP

		case 'u':	/* cancel UDP */

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

			udp_cep_ipproto = (*(line + 2) == '4' ? API_PROTO_IPV4 : API_PROTO_IPV6);
			cepid = atoi(line = skip_blanks(line + 3));
			while ('0' <= *line && *line <= '9')
				line ++;

#if TNUM_UDP6_CEPID > 0

			if (udp_cep_ipproto == API_PROTO_IPV6) {
				if ((error = udp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
					cons_printf(portid, "[UDP CAN CEP] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				}

#endif	/* of #if TNUM_UDP6_CEPID > 0 */

#if TNUM_UDP4_CEPID > 0

			if (udp_cep_ipproto == API_PROTO_IPV4) {
				if ((error = udp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
					cons_printf(portid, "[UDP CAN CEP] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				}

#endif	/* of #if TNUM_UDP4_CEPID > 0 */

#else	/* of #if defined(SUPPORT_INET4) */

#if TNUM_UDP6_CEPID > 0

			cepid = atoi(line = skip_blanks(line + 2));
			while ('0' <= *line && *line <= '9')
				line ++;

			if ((error = udp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
				cons_printf(portid, "[UDP CAN CEP] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}

#endif	/* of #if TNUM_UDP6_CEPID > 0 */

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#if TNUM_UDP4_CEPID > 0

			cepid = atoi(line = skip_blanks(line + 2));
			while ('0' <= *line && *line <= '9')
				line ++;

			if ((error = udp_can_cep((ID)cepid, atoi(skip_blanks(line)))) != E_OK && error != E_RLWAI) {
				cons_printf(portid, "[UDP CAN CEP] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}

#endif	/* of #if TNUM_UDP4_CEPID > 0 */

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

			break;

#endif	/* of #ifdef SUPPORT_UDP */

			}

		break;

	case 'd':	/* discard client */
		switch (*(line + 1)) {

#if defined(SUPPORT_PPP)

		case 'c':		/* disconnect */
			if (lcp_fsm.state != PPP_FSM_STOPPED)
				lcp_close();
			break;

#endif	/* of #if defined(SUPPORT_PPP) */

#if defined(SUPPORT_ETHER) && defined(USE_NET_CONS)

		case 'c':		/* disconnect */
			discon_net_cons();
			break;

#endif	/* of #if defined(SUPPORT_ETHER) && defined(USE_NET_CONS) */

#if defined(DHCP6_CLI_CFG) || defined(DHCP4_CLI_CFG)

		case 'h':		/* DHCP client */
			dhcpc(portid, line + 2);
			break;

#endif	/* of #if defined(DHCP6_CLI_CFG) || defined(DHCP4_CLI_CFG) */

#ifdef USE_TCP_DISCARD_CLI

		case 't':	/* TCP discard client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel TCP discard client */
				tcp_discard_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_TCP_DISCARD_CLI, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[TCP DISCARD CLI] error: %s.\n",
					       itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_TCP_DISCARD_CLI */

#ifdef USE_UDP_DISCARD_CLI

		case 'u':	/* UDP discard client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel UDP discard client */
				udp_discard_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_UDP_DISCARD_CLI, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[UDP DISCARD CLI] error: %s.\n",
					       itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_UDP_DISCARD_CLI */

			}

		break;

#if defined(USE_TCP_ECHO_CLI) || defined(USE_UDP_ECHO_CLI)

	case 'e':	/* echo client */
		switch (*(line + 1)) {

#ifdef USE_TCP_ECHO_CLI

		case 't':	/* TCP echo client */
			line += 2;
			switch (*line) {

			case 's':	/* cancel TCP echo */
				tcp_echo_cli_valid = false;
				break;

			default:
				if ((error = psnd_dtq(DTQ_TCP_ECHO_CLI_SND, (intptr_t)line)) != E_OK) {
					cons_printf(portid, "[TCP ECHO CLI] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;

				}
			break;

#endif	/* of #ifdef USE_TCP_ECHO_CLI */

#if defined(USE_UDP_ECHO_CLI)

		case 'u':	/* UDP echo client */
			switch (*(line + 2)) {

			case 's':	/* cancel UDP echo */

#if defined(USE_UDP6_ECHO_CLI_TSK)

#if defined(USE_UDP4_ECHO_CLI_TSK)

				if (*(line + 3) == '4')
					udp4_echo_cli_valid = false;
				else
					udp6_echo_cli_valid = false;

#else	/* of #if defined(USE_UDP4_ECHO_CLI_TSK) */

				udp6_echo_cli_valid = false;

#endif	/* of #if defined(USE_UDP4_ECHO_CLI_TSK) */

#else	/* of #if defined(USE_UDP6_ECHO_CLI_TSK) */

#if defined(USE_UDP4_CLI_TSK)

				udp4_echo_cli_valid = false;

#endif	/* of #if defined(USE_UDP4_CLI_TSK) */

#endif	/* of #if defined(USE_UDP6_ECHO_CLI_TSK) */


				break;

			default:

				line += 2;

#if defined(USE_UDP6_ECHO_CLI_TSK)

#if defined(USE_UDP4_ECHO_CLI_TSK)

				if (*line == '4') {
					line ++;
					error = psnd_dtq(DTQ_UDP4_ECHO_CLI, (intptr_t)line);
					}
				else {
					if (*line && *line != ' ')
						line ++;
					error = psnd_dtq(DTQ_UDP6_ECHO_CLI, (intptr_t)line);
					}

#else	/* of #if defined(USE_UDP4_ECHO_CLI_TSK) */

				if (*line && *line != ' ')
					line ++;
				error = psnd_dtq(DTQ_UDP6_ECHO_CLI, (intptr_t)line);

#endif	/* of #if defined(USE_UDP4_ECHO_CLI_TSK) */

#else	/* of #if defined(USE_UDP6_ECHO_CLI_TSK) */

				if (*line && *line != ' ')
					line ++;
				error = psnd_dtq(DTQ_UDP4_ECHO_CLI, (intptr_t)line);

#endif	/* of #if defined(USE_UDP6_ECHO_CLI_TSK) */

				if (error != E_OK) {
					cons_printf(portid, "[UDP ECHO CLI] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}

				break;

				}
			break;

#endif	/* of #if defined(USE_UDP_ECHO_CLI) */

			}

		break;

#endif	/* of #if defined(USE_TCP_ECHO_CLI) || defined(USE_UDP_ECHO_CLI) */

	case 'i':

		if (*(line + 1) == 'f') {

			dbg_cons_ifconfig (portid, line + 2);

#if defined(NETAPP_IP6_CFG)
			dbg_cons_ifconfig6(portid, line + 2);
#endif

#if defined(NETAPP_IP4_CFG)
			dbg_cons_ifconfig4(portid, line + 2);
#endif

			}

#ifdef SUPPORT_PPP

#ifdef PPP_CFG_MODEM

		else if (lcp_fsm.state == PPP_FSM_STOPPED)
			dial();		/* initiate */

#else	/* of #ifdef PPP_CFG_MODEM */

		else if (lcp_fsm.state == PPP_FSM_STOPPED)
			lcp_open(PPP_OPEN_ACTIVE);

#endif	/* of #ifdef PPP_CFG_MODEM */

#endif	/* of #ifdef SUPPORT_PPP */

		break;

	case 'n':		/* network status */
		dbg_cons_network_status(portid, line + 1);
		break;

	case 'p':		/* ping or task status */

		if (*(line + 1) == 's')
			dbg_cons_task_status(portid, skip_blanks(line + 2));

#ifdef USE_PING

		else
			dbg_cons_ping_comd(portid, line + 1);

#endif	/* of #ifdef USE_PING */

		break;

	case 'r':		/* release wait task */
		if ((error = rel_wai(atoi(skip_blanks(line + 1)))) != E_OK) {
			cons_printf(portid, "[REL WAI TSK] error: %s.\n", itron_strerror(error));
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();
			}
		break;

	case 't':		/* terminate (delete) REP/CEP */
		switch (*(line + 1)) {

#ifdef USE_TCP_EXTENTIONS

		case 't':		/* TCP REP */
	
			error = tcp_del_rep(atoi(skip_blanks(line + 2)));
			if (error != E_OK) {
				cons_printf(portid, "[TCP DEL REP] ID: %d, error: %s.\n", 
				                    atoi(skip_blanks(line + 2)),
				                    itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

		case 'u':		/* UDP CEP */

#if defined(USE_UDP6_ECHO_SRV_TSK)

#if defined(USE_UDP4_ECHO_SRV_TSK)

			if (*(line + 2) == '4') {
				if ((error = udp_del_cep(atoi(skip_blanks(line + 3)))) != E_OK) {
					cons_printf(portid, "[UDP DEL CEP] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				}
			else {
				if ((error = udp6_del_cep(atoi(skip_blanks(line + 2)))) != E_OK) {
					cons_printf(portid, "[UDP DEL CEP] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				}

#else	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

			if ((error = udp6_del_cep(atoi(skip_blanks(line + 2)))) != E_OK) {
				cons_printf(portid, "[UDP DEL CEP] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}

#endif	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

#else	/* of #if defined(USE_UDP6_ECHO_SRV_TSK) */

#if defined(USE_UDP4_ECHO_SRV_TSK)

			if ((error = udp_del_cep(atoi(skip_blanks(line + 2)))) != E_OK) {
				cons_printf(portid, "[UDP DEL CEP] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}

#endif	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

#endif	/* of #if defined(USE_UDP6_ECHO_SRV_TSK) */

			break;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

			}
		break;

#if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS)

	case 'w':		/* wakeup server */
				/* wake up task */
		switch (*(line + 1)) {

#ifdef USE_TCP_EXTENTIONS

		case 't':		/* tcp server */
			switch (*(line + 2)) {

#ifdef USE_TCP_DISCARD_SRV
			case 'd':		/* tcp discard server */
				if ((error = wup_tsk(TCP_DISCARD_SRV_TASK)) != E_OK) {
					//cons_printf(portid, "[WUP TCP DISCARD SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #ifdef USE_TCP_DISCARD_SRV */

#if defined(USE_TCP_ECHO_SRV1)
			case 'e':		/* tcp echo server */
				if ((error = wakeup_tcp_echo_srv((*(line + 3) == '4' ? API_PROTO_IPV4 : API_PROTO_IPV6))) != E_OK) {
					//cons_printf(portid, "[WUP TCP ECHO SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #if defined(USE_TCP_ECHO_SRV1) */

#if defined(USE_TCP_ECHO_SRV2)
			case 'e':		/* tcp echo server */
				if ((error = wup_tsk(TCP_ECHO_SRV_TASK2)) != E_OK) {
					//cons_printf(portid, "[WUP TCP ECHO SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #if defined(USE_TCP_ECHO_SRV2) */

#ifdef USE_WWW_SRV
			case 'w':		/* WWW server */
				if ((error = wakeup_www_srv((*(line + 3) == '4' ? API_PROTO_IPV4 : API_PROTO_IPV6))) != E_OK) {
					//cons_printf(portid, "[WUP WWW SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}
				break;
#endif	/* of #ifdef USE_TCP_DISCARD_SRV */

				}
			break;

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

		case 'u':		/* udp server */
			switch (*(line + 2)) {

#if !defined(USE_UDP_CALL_BACK)

			case 'e':		/* udp echo server */

#if defined(USE_UDP6_ECHO_SRV_TSK)

#if defined(USE_UDP4_ECHO_SRV_TSK)

				if (*(line + 3) == '4') {
					if ((error = wup_tsk(UDP4_ECHO_SRV_TASK)) != E_OK) {
						cons_printf(portid, "[WUP UDP4 ECHO SRV] error: %s.\n", itron_strerror(error));
						SIG_NET_CONS_PRINTF();
						FLUSH_SND_BUFF();
						}
					}
				else {
					if ((error = wup_tsk(UDP6_ECHO_SRV_TASK)) != E_OK) {
						cons_printf(portid, "[WUP UDP6 ECHO SRV] error: %s.\n", itron_strerror(error));
						SIG_NET_CONS_PRINTF();
						FLUSH_SND_BUFF();
						}
					}

#else	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

				if ((error = wup_tsk(UDP6_ECHO_SRV_TASK)) != E_OK) {
					cons_printf(portid, "[WUP UDP6 ECHO SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}

#endif	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

#else	/* of #if defined(USE_UDP6_ECHO_SRV_TSK) */

#if defined(USE_UDP4_ECHO_SRV_TSK)

				if ((error = wup_tsk(UDP4_ECHO_SRV_TASK)) != E_OK) {
					cons_printf(portid, "[WUP UDP4 ECHO SRV] error: %s.\n", itron_strerror(error));
					SIG_NET_CONS_PRINTF();
					FLUSH_SND_BUFF();
					}

#endif	/* of #if defined(USE_UDP4_ECHO_SRV_TSK) */

#endif	/* of #if defined(USE_UDP6_ECHO_SRV_TSK) */

				break;

#endif	/* of #if !defined(USE_UDP_CALL_BACK) */

				}
			break;

#endif	/* of #ifdef USE_UDP_EXTENTIONS */

		default:
			if ((error = wup_tsk(atoi(skip_blanks(line + 1)))) != E_OK) {
				cons_printf(portid, "[WUP TSK] error: %s.\n", itron_strerror(error));
				SIG_NET_CONS_PRINTF();
				FLUSH_SND_BUFF();
				}
			break;

			}
		break;

#else	/* of #if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS) */

	case 'w':		/* wake up task */
		if ((error = wup_tsk(atoi(skip_blanks(line + 1)))) != E_OK) {
			cons_printf(portid, "[WUP TSK] error: %s.\n", itron_strerror(error));
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();
			}
		break;

#endif	/* of #if defined(USE_TCP_EXTENTIONS) || defined(USE_UDP_EXTENTIONS) */

	default:

		if (*line) {

#ifdef USE_DBG_CONS_EXTRA_PARSE

			//cons_printf(portid, "extra command: '%c'.\n", *line);
			dbg_cons_extra_parse(portid, line);

#else	/* of #ifdef USE_DBG_CONS_EXTRA_PARSE */

			//cons_printf(portid, "command: '%c'.\n", *line);
			cons_printf(portid, "no such command: '%c'.\n", *line);
			SIG_NET_CONS_PRINTF();
			FLUSH_SND_BUFF();

#endif	/* of #ifdef USE_DBG_CONS_EXTRA_PARSE */

			}
		break;
		}
	}

#endif	/* of #ifdef USE_DBG_CONS_PARSER */

/*
 *  デバッグコンソールタスク
 */

void
dbg_con_task (intptr_t exinf)
{
	static char line[DBG_LINE_SIZE + 1];

	ID	tskid;

#ifdef USE_LCD
#ifdef SUPPORT_ETHER

#if 0
#if defined(SUPPORT_INET6)

	uint32_t	lllen;
	const uint8_t	*lladdr;

#endif	/* of #if defined(SUPPORT_INET6) */
#endif

	lcd_initialize(0);

#if 0
#if defined(SUPPORT_INET6)

	dly_tsk(1 * 1000);
	lladdr = IF_ETHER_NIC_GET_SOFTC()->ifaddr.lladdr;
	for (lllen = ETHER_ADDR_LEN; lllen --; )
		lcd_printf(LCD_PORTID, "%02x", *lladdr ++);
	lcd_putc(LCD_PORTID, '\n');

#else	/* of #if defined(SUPPORT_INET6) */

	if ((TINET_PRVER & UINT_C(0x0f)) > 0)
		lcd_printf(LCD_PORTID, "TINET-%d.%d.%d\n",
		       (TINET_PRVER   >> 12) & UINT_C(0x0f),
		       (TINET_PRVER   >>  4) & UINT_C(0x0f),
		        TINET_PRVER          & UINT_C(0x0f));
	else
		lcd_printf(LCD_PORTID, "TINET-%d.%d\n",
		       (TINET_PRVER   >> 12) & UINT_C(0x0f),
		       (TINET_PRVER   >>  4) & UINT_C(0x0f));

#endif	/* of #if defined(SUPPORT_INET6) */
#endif

#if 0
#if defined(SUPPORT_INET4)

	dly_tsk(1 * 1000);
	lcd_puts(LCD_PORTID, ip2str(NULL, in4_get_ifaddr(0)));
	lcd_putc(LCD_PORTID, '\n');

#endif	/* of #if defined(SUPPORT_INET4) */
#endif

#endif	/* of #ifdef SUPPORT_ETHER */
#endif	/* of #ifdef USE_LCD */

	get_tid(&tskid);
	cons_printf(CONSOLE_PORTID, "[CONSOLE:%d] started.\n", tskid);
	serial_ctl_por(CONSOLE_PORTID,
	               IOCTL_ECHO  | IOCTL_CRLF  |
	               IOCTL_FCSND | IOCTL_FCANY | IOCTL_FCRCV);
	netapp_srand (tskid);
	while (true) {
		cons_getline(CONSOLE_PORTID, line, DBG_LINE_SIZE);
		dbg_parse(CONSOLE_PORTID, line);
		}
	}

#endif	/* of #ifdef USE_DBG_CONS */
