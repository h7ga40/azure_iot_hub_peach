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
 *  ping6 -- ICMPv6 ECHO メッセージを送信する。
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

#include <netinet6/nd6.h>

#include <net/if_var.h>

#ifdef USE_PING

#if defined(SUPPORT_INET6)

/*
 *  変数
 */

#if 0
static uint16_t	curr_icmp6_id;
static uint16_t	curr_icmp6_size;
#endif

static uint16_t	send_icmp6_id = 0;
static uint16_t	reply_count;
static SYSTIM	icmp6_start_time;

/*
 *  icmp6_reply_timeout -- ICMP6 REPLY タイムアウト処理
 */

static void
icmp6_reply_timeout (void *arg)
{
	if (reply_count == 0)
		syslog(LOG_NOTICE, "[PING6] request timeout.");
	}

/*
 *  icmp6_echo_reply -- ICMP6 ECHO REPLY メッセージを処理する。
 */

void
icmp6_echo_reply (T_NET_BUF *input, uint_t ihoff)
{
	SYSTIM	time;

	syscall(get_tim(&time));
	untimeout(icmp6_reply_timeout, NULL);
	syslog(LOG_NOTICE, "[PING6] reply: %5ld [ms] from: %s",
	                   (time - icmp6_start_time) * 1000 / SYSTIM_HZ,
	                   ipv62str(NULL, &GET_IP6_HDR(input)->src));
	reply_count ++;
	}

/*
 *  ping6 -- ICMP ECHO メッセージを送信する。
 */

void
ping6 (T_IN6_ADDR *addr, uint_t tmo, uint_t size)
{
	T_IFNET		*ifp = IF_GET_IFNET();
	T_ICMP6_HDR	*icmp6h;
	T_NET_BUF	*output;
	T_IN6_IFADDR	*ia;
	int_t		ix;
	uint16_t	ipflags = 0;
	uint8_t		*sdu;

	/*
	 *  宛先アドレスにふさわしい送信元アドレスを、
	 *  ネットワークインタフェースから探索して利用する。
	 */
	if ((ia = in6_ifawithifp(ifp, addr)) == NULL)
		return;

	if (IN6_IS_ADDR_MULTICAST(addr))
		ipflags = IPV6_OUT_SET_HOP_LIMIT(IPV6_OUT_FLG_HOP_LIMIT, IPV6_DEFHLIM);

	/* 送信用の IP データグラムを獲得する。*/
	if (in6_get_datagram(&output, ICMP6_HDR_SIZE + size, 0,
	                     addr, &ia->addr, IPPROTO_ICMPV6,
	                     IPV6_DEFHLIM, NBA_SEARCH_ASCENT, TMO_FEVR) != E_OK)
		return;

	/* ICMP ヘッダを設定する。*/
	icmp6h           = GET_ICMP6_HDR(output, IF_IP6_ICMP6_HDR_OFFSET);
	icmp6h->type     = ICMP6_ECHO_REQUEST;
	icmp6h->code     = 0;
	icmp6h->icmp6_id = htons(send_icmp6_id);
	icmp6h->icmp6_seq= 0;

	/* データを設定する。*/
	sdu = GET_ICMP6_SDU(output, IF_IP6_ICMP6_HDR_OFFSET) + size;
	for (ix = size; ix -- > 0; )
		*-- sdu = ('0' + send_icmp6_id + ix) & 0xff;

	/* チェックサムを計算する。*/
	icmp6h->sum = 0;
	icmp6h->sum = in6_cksum(output, IPPROTO_ICMPV6,
	                        (uint8_t*)icmp6h - output->buf, ICMP6_HDR_SIZE + size);

	/* 応答チェック用の変数を設定する。*/
	reply_count = 0;

#if 0
	curr_icmp6_id   = send_icmp6_id ++;
	curr_icmp6_size = size;
#endif

#if 0
	syslog(LOG_NOTICE, "[PING6] send: TMO:%d, SIZE:%d, to: %s, from %s",
	                   tmo, size,
	                   ipv62str(NULL, addr), ipv62str(NULL, &ia->addr));
#endif

	/* 送信後、現在の時間を記憶し、タイムアウトを設定する。*/
	ip6_output(output, ipflags, TMO_FEVR);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutMsgs, 1);
	NET_COUNT_MIB(icmp6_ifstat.ipv6IfIcmpOutEchos, 1);

	syscall(get_tim(&icmp6_start_time));
	timeout(icmp6_reply_timeout, NULL, tmo * NET_TIMER_HZ);

	}

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifdef USE_PING */
