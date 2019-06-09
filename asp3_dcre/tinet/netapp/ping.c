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
 *  ping -- ICMP ECHO メッセージを送信する。
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/ppp_var.h>
#include <net/ppp_ipcp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>

#ifdef USE_PING

#if defined(SUPPORT_INET4)

/*
 *  変数
 */

#if 0
static uint16_t	curr_icmp_id;
static uint16_t	curr_icmp_len;
#endif

static uint16_t	icmp_id = 0;
static SYSTIM	icmp_start_time;

/*
 *  icmp_reply_timeout -- ICMP REPLY タイムアウト処理
 */

static void
icmp_reply_timeout (void *arg)
{
	syslog(LOG_NOTICE, "[PING4] request timeout.");
	}

/*
 *  icmp_echo_reply -- ICMP ECHO REPLY メッセージを処理する。
 */

void
icmp_echo_reply (T_NET_BUF *input, uint_t ihoff)
{
	SYSTIM		time;
	T_IN4_ADDR	addr;

	syscall(get_tim(&time));
	addr = ntohl(GET_IP4_HDR(input)->src);
	untimeout(icmp_reply_timeout, NULL);
	syslog(LOG_NOTICE, "[PING4] reply: %5ld [ms] from: %s",
	                   (time - icmp_start_time) * 1000 / SYSTIM_HZ,
	                   ip2str(NULL, &addr));
	}

/*
 *  ping4 -- ICMP ECHO メッセージを送信する。
 */

void
ping4 (T_IN4_ADDR *addr, uint_t tmo, uint_t len)
{
	T_ICMP4_HDR	*icmp4h;
	T_NET_BUF	*output;
	uint8_t		*sdu;
	int_t		ix;

	/* 送信用の IP データグラムを獲得する。*/
	if (in4_get_datagram(&output, ICMP4_HDR_SIZE + len, 0,
	                     addr, NULL, IPPROTO_ICMP,
	                     IP4_DEFTTL, NBA_SEARCH_ASCENT, TMO_FEVR) != E_OK)
		return;

	/* ICMP ヘッダを設定する。*/
	icmp4h			= GET_ICMP4_HDR(output, IF_IP4_ICMP4_HDR_OFFSET);
	icmp4h->type		= ICMP4_ECHO_REQUEST;
	icmp4h->code		= 0;
	icmp4h->data.id_seq.id	= htons(icmp_id);
	icmp4h->data.id_seq.seq	= 0;

	/* データを設定する。*/
	sdu = GET_ICMP4_SDU(output, IF_IP4_ICMP4_HDR_OFFSET) + len;
	for (ix = len; ix -- > 0; )
		*-- sdu = ('0' + icmp_id + ix) & 0xff;

	/* チェックサムを計算する。*/
	icmp4h->sum = 0;
	icmp4h->sum = in_cksum(icmp4h, ICMP4_HDR_SIZE + len);

	/* 応答チェック用の変数を設定する。*/
#if 0
	curr_icmp_id  = icmp_id ++;
	curr_icmp_len = len;
#endif

	NET_COUNT_MIB(icmp_stats.icmpOutMsgs, 1);
	NET_COUNT_MIB(icmp_stats.icmpOutEchos, 1);

	/* 送信後、現在の時間を記憶し、タイムアウトを設定する。*/
	ip_output(output, TMO_FEVR);

	syscall(get_tim(&icmp_start_time));
	timeout(icmp_reply_timeout, NULL, tmo * NET_TIMER_HZ);

	}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #ifdef USE_PING */
