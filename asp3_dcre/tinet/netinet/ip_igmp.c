/*
*  TINET (TCP/IP Protocol Stack)
*
*  Copyright (C) 2001-2012 by Dep. of Computer Science and Engineering
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

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
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
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/udp_var.h>
#include <netinet/ip_igmp.h>

#include <net/if_var.h>
#include <net/net_var.h>

#ifdef SUPPORT_IGMP

static T_IGMP_ENTRY igmp_groups[NUM_IGMP_ENTRY];
static T_IN4_ADDR igmp_send_addrs[NUM_IGMP_ENTRY];

static bool_t igmp_send_report_v2(T_IN4_ADDR dst);
static int16_t igmp_get_timer(T_IGMP_ENTRY *entry);

ER igmp_set_loop(T_UDP_CEP *cep, uint8_t optval)
{
	switch (optval) {
	case 0:
		cep->igmp_loopback = false;
		return E_OK;
	case 1:
		cep->igmp_loopback = true;
		return E_OK;
	}

	return E_PAR;
}

ER igmp_get_loop(T_UDP_CEP *cep, uint8_t *optval)
{
	if (optval == NULL)
		return E_PAR;

	*optval = cep->igmp_loopback ? 1 : 0;

	return E_OK;
}

ER igmp_set_ttl(T_UDP_CEP *cep, uint8_t optval)
{
	cep->igmp_ttl = optval;

	return E_OK;
}

ER igmp_get_ttl(T_UDP_CEP *cep, uint8_t *optval)
{
	if (optval == NULL)
		return E_PAR;

	*optval = cep->igmp_ttl;

	return E_OK;
}

ER igmp_set_if(T_UDP_CEP *cep, T_IN4_ADDR *optval)
{
	if (optval == NULL)
		return E_PAR;

	cep->igmp_mcaddr = *optval;

	return E_OK;
}

ER igmp_get_if(T_UDP_CEP *cep, T_IN4_ADDR *optval)
{
	if (optval == NULL)
		return E_PAR;

	*optval = cep->igmp_mcaddr;

	return E_OK;
}

ER igmp_add_membership(T_UDP_CEP *cep, T_IP_MREQ *optval)
{
	T_IGMP_ENTRY *entry = NULL;
	int		i;
	T_IFNET	*ifp = IF_GET_IFNET();
	ER ret = E_NOMEM;

	if ((optval->imr_interface != IPV4_ADDRANY)
		&& (optval->imr_interface != ifp->in4_ifaddr.addr))
		return E_PAR;

	syscall(wai_sem(SEM_IGMP_GROUP_LOCK));

	for (i = 0; i < NUM_IGMP_ENTRY; i++) {
		entry = &igmp_groups[i];
		if (entry->timer == -1) {
			entry->timer = 0;
			entry->ip_addr = optval->imr_multiaddr;
			ret = E_OK;
			break;
		}
	}

	syscall(sig_sem(SEM_IGMP_GROUP_LOCK));

	return ret;
}

ER igmp_drop_membership(T_UDP_CEP *cep, T_IP_MREQ *optval)
{
	T_IGMP_ENTRY *entry = NULL;
	int		i;
	ER ret = E_PAR;
	T_IFNET	*ifp = IF_GET_IFNET();

	if ((optval->imr_interface != IPV4_ADDRANY)
		&& (optval->imr_interface != ifp->in4_ifaddr.addr))
		return E_PAR;

	syscall(wai_sem(SEM_IGMP_GROUP_LOCK));

	for (i = 0; i < NUM_IGMP_ENTRY; i++) {
		entry = &igmp_groups[i];
		if ((entry->timer >= 0) && (entry->ip_addr == optval->imr_multiaddr)) {
			entry->timer = -1;
			entry->ip_addr = 0;
			ret = E_OK;
			break;
		}
	}

	syscall(sig_sem(SEM_IGMP_GROUP_LOCK));

	return ret;
}

ER igmp_set_ip_msfilter(T_UDP_CEP *cep, const T_IP_MSFILTER *optval)
{
	return E_PAR;
}

ER igmp_get_ip_msfilter(T_UDP_CEP *cep, T_IP_MSFILTER *optval)
{
	return E_PAR;
}

uint_t igmp_input(T_NET_BUF **inputp, uint_t *offp, uint_t *nextp)
{
	T_NET_BUF	*input = *inputp;
	T_IGMPV2_HDR	*igmph;
	uint_t		len, align;
	T_IP4_HDR	*ip4h;
	int i;
	T_IGMP_ENTRY *entry;
	bool_t ret;

	NET_COUNT_IGMP(net_count_igmp.in_octets,
		input->len - GET_IF_IP4_HDR_SIZE(input));
	NET_COUNT_IGMP(net_count_igmp.in_packets, 1);

	/* ICMP ヘッダの長さをチェックする。*/
	if (input->len < IF_IP4_IGMP_HDR_SIZE) {
		NET_COUNT_IGMP(net_count_igmp.in_err_packets, 1);
		NET_COUNT_MIB(igmp_stats.igmpInErrors, 1);
		goto buf_rel;
	}

	igmph = (T_IGMPV2_HDR *)(input->buf + *offp);

	/* 4 オクテット境界のデータ長 */
	len = input->len - *offp;
	align = (len + 3) >> 2 << 2;

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset((uint8_t*)input->buf + input->len, 0, (size_t)(align - len));

	/* チェックサムを計算する。*/
	if (in_cksum(igmph, align) != 0) {
		NET_COUNT_IGMP(net_count_igmp.in_err_packets, 1);
		goto buf_rel;
	}

	/* メッセージの型により分岐する。*/
	switch (igmph->type) {
	case IGMP_MEMBERSHIP_QUERY:
		ip4h = GET_IP4_HDR(input);
		ret = false;

		/* 参加中のグループアドレスからのクエリーの場合 */
		syscall(wai_sem(SEM_IGMP_GROUP_LOCK));

		for (i = 0; i < NUM_IGMP_ENTRY; i++) {
			entry = &igmp_groups[i];
			if ((entry->timer >= 0) && (entry->ip_addr == igmph->addr)) {
				entry->resptime = igmph->time;
				ret = true;
				break;
			}
		}

		syscall(sig_sem(SEM_IGMP_GROUP_LOCK));

		if (ret) {
			/* メンバーシップリポートを送信 */
			igmp_send_report_v2(ip4h->src);
		}
		break;
	}

buf_rel:
	syscall(rel_net_buf(input));
	return IPPROTO_DONE;
}

static bool_t igmp_send_report_v2(T_IN4_ADDR dst)
{
	T_IGMPV2_HDR	*igmph;
	T_NET_BUF		*output;
	uint_t			len = 0, align;

	if (in4_get_datagram(&output, (uint_t)(IF_IP4_IGMP_HDR_SIZE + len), 0,
		&dst, NULL, IPPROTO_IGMP, IP4_DEFTTL,
		NBA_SEARCH_ASCENT, TMO_IGMP_OUTPUT) != E_OK)
		return false;

	/* IGMP ヘッダを設定する。*/
	igmph = GET_IGMPV2_HDR(output, IF_IP4_IGMP_HDR_OFFSET);
	igmph->type = IGMP_MEMBERSHIP_REPORT_V2;
	igmph->time = 0;
	igmph->addr = htonl(dst);

	/* 4 オクテット境界のデータ長 */
	align = (len + 3) >> 2 << 2;

	/* 4 オクテット境界までパディングで埋める。*/
	if (align > len)
		memset((uint8_t*)GET_IGMP_SDU(output, IF_IP4_IGMP_HDR_OFFSET) + len,
			0, (size_t)(align - len));

	/* チェックサムを計算する。*/
	igmph->sum = 0;
	igmph->sum = in_cksum(igmph, (uint_t)(IGMP_HDR_SIZE + align));

	/* 送信する。*/
	NET_COUNT_IGMP(net_count_igmp.out_octets,
		output->len - GET_IF_IP4_HDR_SIZE(output));
	NET_COUNT_IGMP(net_count_igmp.out_packets, 1);
	NET_COUNT_MIB(icmp_stats.icmpOutMsgs, 1);
	NET_COUNT_MIB(icmp_stats.icmpOutDestUnreachs, 1);
	ip_output(output, TMO_IGMP_OUTPUT);

	return true;
}

/*
*  igmp_timer -- IGMP 共通タイマー
*/
static void igmp_timer(void *ignore)
{
	T_IGMP_ENTRY *entry = NULL;
	int i, j = 0;

	/* memset(igmp_send_addrs, 0, sizeof(igmp_send_addrs)); */

	syscall(wai_sem(SEM_IGMP_GROUP_LOCK));

	for (i = 0; i < NUM_IGMP_ENTRY; i++) {
		entry = &igmp_groups[i];
		if (entry->timer > 0)
			entry->timer--;
		if (entry->timer == 0) {
			entry->timer = igmp_get_timer(entry);
			igmp_send_addrs[j++] = entry->ip_addr;
		}
	}

	syscall(sig_sem(SEM_IGMP_GROUP_LOCK));

	for (i = 0; i < j; i++) {
		igmp_send_report_v2(igmp_send_addrs[i]);
	}

	timeout(igmp_timer, NULL, NET_TIMER_HZ / 1);
}

/*
*  igmp_init -- IGMP 機能を初期化する。
*/

void igmp_init(void)
{
	int i;

	for (i = 0; i < NUM_IGMP_ENTRY; i++) {
		igmp_groups[i].timer = -1;
	}

	timeout(igmp_timer, NULL, NET_TIMER_HZ / 1);
}

bool_t igmp_is_joined_group(T_IN4_ADDR addr)
{
	T_IGMP_ENTRY *entry = NULL;
	int		i;
	bool_t ret = false;

	syscall(wai_sem(SEM_IGMP_GROUP_LOCK));

	for (i = 0; i < NUM_IGMP_ENTRY; i++) {
		entry = &igmp_groups[i];
		if ((entry->timer >= 0) && (entry->ip_addr == addr)) {
			ret = true;
			break;
		}
	}

	syscall(sig_sem(SEM_IGMP_GROUP_LOCK));

	return ret;
}

int16_t igmp_get_timer(T_IGMP_ENTRY *entry)
{
	int16_t time = entry->resptime;

	if (time == 0)
		time = 590;

	return (int16_t)(net_rand() % time) + 10;
}

#endif /* of #ifdef SUPPORT_IGMP */
