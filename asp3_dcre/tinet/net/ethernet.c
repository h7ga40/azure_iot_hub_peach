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
 *  Ethernet 処理タスクの本体
 */

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_llc.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>

#include <netinet6/if6_ether.h>
#include <netinet6/nd6.h>

#include <net/if_var.h>

#ifdef SUPPORT_ETHER

/*
 *  変数
 */

#if defined(_IP4_CFG)

const uint8_t ether_broad_cast_addr[ETHER_ADDR_LEN] = {
	UINT_C(0xff), UINT_C(0xff), UINT_C(0xff),
	UINT_C(0xff), UINT_C(0xff), UINT_C(0xff)
	};

#endif	/* of #if defined(_IP4_CFG) */

/*
 *  変数
 */

/* ネットワークインタフェース構造体 */

static T_IFNET ether_ifnet;

#ifdef SUPPORT_MIB

/*  SNMP の 管理情報ベース (MIB) */

T_IF_STATS if_stats;

#endif	/* of #ifdef SUPPORT_MIB */

#ifdef ETHER_CFG_802_WARNING

/*
 *  関数
 */

static void ieee_802_input (T_NET_BUF *input);

/*
 *  ieee_802_input -- IEEE 802.3 入力処理
 */

static void
ieee_802_input (T_NET_BUF *input)
{
	T_8023MAC_HDR	*mhdr;
	T_8022SNAP_HDR	*shdr;

	mhdr = GET_8023MAC_HDR(input);
	shdr = GET_8022SNAP_HDR(input);
	syslog(LOG_WARNING, "[ETHER] unexp 802.3 from: %s, proto: 0x%04x.",
	                    mac2str(NULL, mhdr->shost), ntohs(*(uint16_t*)&shdr->type));
	}

#endif	/* of #ifdef ETHER_CFG_802_WARNING */

/*
 *  ether_srand -- 乱数を初期値を返す。
 */

uint32_t
ether_srand (void)
{
	T_IF_SOFTC	*ic;
	uint32_t	rval;

	ic = IF_ETHER_NIC_GET_SOFTC();
	rval  = (ic->ifaddr.lladdr[2] << 24)
	      + (ic->ifaddr.lladdr[3] << 16)
	      + (ic->ifaddr.lladdr[4] <<  8)
	      + (ic->ifaddr.lladdr[5]      );

#ifdef ETHER_CFG_COLLECT_ADDR

	rval += (ether_collect_addr.lladdr[2] << 24)
	      + (ether_collect_addr.lladdr[3] << 16)
	      + (ether_collect_addr.lladdr[4] <<  8)
	      + (ether_collect_addr.lladdr[5]      );

#endif	/* of #ifdef ETHER_CFG_COLLECT_ADDR */

	return rval;
	}

/*
 *  ether_get_ifnet -- ネットワークインタフェース構造体を返す。
 */

T_IFNET *
ether_get_ifnet (void)
{
	return &ether_ifnet;
	}

#if defined(_IP6_CFG)

/*
 *  ether_in6_resolve_multicast -- イーサネットのマルチキャストアドレスへの変換
 */

ER
ether_in6_resolve_multicast (T_ETHER_ADDR *ifaddr, const T_IN6_ADDR *maddr)
{
	/* マルチキャストアドレスかチェックする。*/
	if (!IN6_IS_ADDR_MULTICAST(maddr))
		return E_PAR;

	/* イーサネットのマルチキャストアドレスへの変換 */
	ether_map_ipv6_multicast(ifaddr, maddr);

	return E_OK;
	}

#endif	/* of #if defined(_IP6_CFG) */

/**
 * ether_set_link_up -- リンクしたときにNICから呼び出される
 */
void
ether_set_link_up()
{
	if (!(ether_ifnet.flags & IF_FLAG_LINK_UP)) {
		ether_ifnet.flags |= IF_FLAG_LINK_UP;

		if (ether_ifnet.flags & IF_FLAG_UP) {
#if defined(_IP4_CFG) && defined(SUPPORT_ETHER)
			/* ARP再構築？ */
#endif /* #if defined(_IP4_CFG) && defined(SUPPORT_ETHER) */

#ifdef SUPPORT_IGMP
			/* IGMP参加 */
#endif /* #ifdef SUPPORT_IGMP */
			}
		if (ether_ifnet.link_callback) {
			(ether_ifnet.link_callback)(&ether_ifnet);
			}
		}
	}

/**
 * ether_set_link_up -- リンクが切断したときにNICから呼び出される
 */
void
ether_set_link_down()
{
	if (ether_ifnet.flags & IF_FLAG_LINK_UP) {
		ether_ifnet.flags &= ~IF_FLAG_LINK_UP;
		if (ether_ifnet.link_callback) {
			(ether_ifnet.link_callback)(&ether_ifnet);
			}
		}
	}

/*
 * ether_set_up -- DHCPでアドレスが設定されたとき呼び出される
 */

void
ether_set_up()
{
	if (!(ether_ifnet.flags & IF_FLAG_UP)) {
		ether_ifnet.flags |= IF_FLAG_UP;
		if (ether_ifnet.link_callback) {
			(ether_ifnet.link_callback)(&ether_ifnet);
			}
		}
	}

/*
 * ether_set_down -- DHCPでアドレスが解放されたとき呼び出される
 */

void
ether_set_down()
{
	if (ether_ifnet.flags & IF_FLAG_UP) {
		ether_ifnet.flags &= ~IF_FLAG_UP;
		if (ether_ifnet.link_callback) {
			(ether_ifnet.link_callback)(&ether_ifnet);
			}
		}
	}

/*
 * ether_set_link_callback -- リンク状態変化時のコールバック登録
 */

void
ether_set_link_callback(ether_status_callback_fn link_callback)
{
	ether_ifnet.link_callback = link_callback;
	if (ether_ifnet.link_callback) {
		(ether_ifnet.link_callback)(&ether_ifnet);
		}
	}

/*
 *  ether_raw_output -- Ethernet インタフェースの出力関数、MAC アドレス解決無し
 */

ER
ether_raw_output (T_NET_BUF *output, TMO tmout)
{
	ER error = E_OK;

	/* Ethernet 出力キューに投入する。*/
	if ((error = tsnd_dtq(DTQ_ETHER_OUTPUT, (intptr_t)output, tmout)) != E_OK) {
		NET_COUNT_ETHER(net_count_ether.out_err_packets, 1);
		NET_COUNT_MIB(if_stats.ifOutDiscards, 1);

		/* IF でネットワークバッファを開放しないフラグをチェックする。*/
		if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
			syscall(rel_net_buf(output));
			}
		else {
			output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;
			}
		IF_ETHER_NIC_RESET(IF_ETHER_NIC_GET_SOFTC());
		}
	return error;
	}

/*
 *  ether_output -- Ethernet インタフェースの出力関数、MAC アドレス解決有り
 */

ER
ether_output (T_NET_BUF *output, const void *dst, T_IF_ADDR *gw, TMO tmout)
{
	T_IF_SOFTC	*ic;
	ER		error = E_OK;
	T_ETHER_HDR *eth;

	/* 送信元 MAC アドレスを設定する。*/
	ic = IF_ETHER_NIC_GET_SOFTC();
	eth = GET_ETHER_HDR(output);
	memcpy(eth->shost, ic->ifaddr.lladdr, ETHER_ADDR_LEN);

	switch(ntohs(eth->type)) {

#if defined(_IP4_CFG)

	case ETHER_TYPE_IP:		/* IPv4	*/
#ifdef ETHER_CFG_MULTICAST
		/* マルチキャストアドレスの場合 */
		if(IN4_IS_ADDR_MULTICAST(*(uint32_t*)dst)){
			/* イーサネットのマルチキャストアドレスへの変換 */
			ether_map_ipv4_multicast((T_ETHER_ADDR *)&eth->dhost, *(T_IN4_ADDR*)dst);
			error = ether_raw_output(output, tmout);
			break;
			}
#endif
		if (arp_resolve(&ic->ifaddr, output, *(uint32_t*)dst)) {	/* true ならアドレス解決済 */
			error = ether_raw_output(output, tmout);
			}
		break;

#endif	/* of #if defined(_IP4_CFG) */

#if defined(_IP6_CFG)

	case ETHER_TYPE_IPV6:		/* IPv6	*/
		error = nd6_storelladdr((T_ETHER_ADDR*)eth->dhost,
		                        (const T_IN6_ADDR*)dst, gw);
		if (error == E_OK)
			error = ether_raw_output(output, tmout);

		/* IF でネットワークバッファを開放しないフラグをチェックする。*/
		else if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
			syscall(rel_net_buf(output));
			}
		else {
			output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;
			}
		break;

#endif	/* of #if defined(_IP6_CFG) */

	default:
		NET_COUNT_MIB(if_stats.ifOutErrors, 1);

		/* IF でネットワークバッファを開放しないフラグをチェックする。*/
		if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
			syscall(rel_net_buf(output));
			}
		else {
			output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;
			}
		error = E_PAR;
		break;
		}

	if (error != E_OK)
		NET_COUNT_ETHER(net_count_ether.out_err_packets, 1);
	return error;
	}

/*
 *  Ethernet 出力タスク
 */

void
ether_output_task(intptr_t exinf)
{
	T_IF_SOFTC	*ic;
	T_NET_BUF	*output;
	ID		tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[ETHER OUTPUT:%d] started.", tskid);

	ic = IF_ETHER_NIC_GET_SOFTC();

	while (true) {
		while (rcv_dtq(DTQ_ETHER_OUTPUT, (intptr_t*)&output) == E_OK) {
			NET_COUNT_ETHER(net_count_ether.out_octets,  output->len);
			NET_COUNT_MIB(if_stats.ifOutOctets, output->len + 8);
			NET_COUNT_ETHER(net_count_ether.out_packets, 1);

#ifdef SUPPORT_MIB
			if ((*(GET_ETHER_HDR(output)->dhost) & ETHER_MCAST_ADDR) == 0) {
				NET_COUNT_MIB(if_stats.ifOutUcastPkts, 1);
				}
			else {
				NET_COUNT_MIB(if_stats.ifOutNUcastPkts, 1);
				}
#endif	/* of #ifdef SUPPORT_MIB */

			syscall(wai_sem(ic->semid_txb_ready));

			IF_ETHER_NIC_START(ic, output);

#ifndef ETHER_NIC_CFG_RELEASE_NET_BUF

			if ((output->flags & NB_FLG_NOREL_IFOUT) == 0) {
				syscall(rel_net_buf(output));
				}
			else {
				output->flags &= (uint8_t)~NB_FLG_NOREL_IFOUT;

#ifdef SUPPORT_TCP
				sig_sem(SEM_TCP_POST_OUTPUT);
#endif	/* of #ifdef SUPPORT_TCP */
				}

#endif	/* of #ifndef ETHER_NIC_CFG_RELEASE_NET_BUF */

			}
		}
	}

/*
 *  Ethernet 入力タスク
 */

void
ether_input_task(intptr_t exinf)
{
	T_IF_SOFTC	*ic;
	T_NET_BUF	*input;
	T_ETHER_HDR	*eth;
	ID		tskid;
	uint16_t	proto;
	uint8_t		rcount = 0;

	/* ネットワークインタフェース管理を初期化する。*/
	ifinit();

	/* イーサネットネットワークインタフェース管理を初期化する。*/

#if defined(_IP4_CFG)

	ether_ifnet.in4_ifaddr.addr = IPV4_ADDR_LOCAL;		/* IPv4 アドレス		*/
	ether_ifnet.in4_ifaddr.mask = IPV4_ADDR_LOCAL_MASK;	/* サブネットマスク		*/

#endif	/* of #if defined(_IP4_CFG) */

	/* NIC を初期化する。*/
	ic = IF_ETHER_NIC_GET_SOFTC();
	IF_ETHER_NIC_PROBE(ic);
	IF_ETHER_NIC_INIT(ic);

	/* Ethernet 出力タスクを起動する */
	syscall(act_tsk(ETHER_OUTPUT_TASK));

	/* ネットワークタイマタスクを起動する */
	syscall(act_tsk(NET_TIMER_TASK));

	get_tid(&tskid);

	syslog(LOG_NOTICE, "[ETHER INPUT:%2d] started on MAC Addr: %s.",
	                   tskid, mac2str(NULL, ic->ifaddr.lladdr));

#if defined(_IP4_CFG)

	/* ARP を初期化する。*/
	arp_init();

#endif	/* of #if defined(_IP4_CFG) */

	ether_ifnet.ic = ic;

	/* 乱数生成を初期化する。*/
	net_srand(0);

	while (true) {
		syscall(wai_sem(ic->semid_rxb_ready));
		if ((input = IF_ETHER_NIC_READ(ic)) != NULL) {
			NET_COUNT_ETHER(net_count_ether.in_octets,  input->len);
			NET_COUNT_MIB(if_stats.ifInOctets, input->len + 8);
			NET_COUNT_ETHER(net_count_ether.in_packets, 1);
			eth = GET_ETHER_HDR(input);
			proto = ntohs(eth->type);

			/* 乱数生成を初期化する。*/
			if (rcount == 0) {

#ifdef ETHER_CFG_COLLECT_ADDR
				memcpy(ether_collect_addr.lladdr, eth->shost, 
				sizeof(ether_collect_addr.lladdr));
#endif	/* of #ifdef ETHER_CFG_COLLECT_ADDR */

				net_srand(0);
				}
			rcount ++;


#ifdef SUPPORT_MIB
			if ((*eth->dhost & ETHER_MCAST_ADDR) == 0) {
				NET_COUNT_MIB(if_stats.ifInUcastPkts, 1);
				}
			else {
				NET_COUNT_MIB(if_stats.ifInNUcastPkts, 1);
				}
#endif	/* of #ifdef SUPPORT_MIB */

#if defined(_IP4_CFG) && defined(ETHER_CFG_ACCEPT_ALL)

			if ((*eth->dhost & ETHER_MCAST_ADDR) && *eth->dhost != 0xff) {

#ifdef ETHER_CFG_MCAST_WARNING

				if (proto <= 1500)
					proto = ntohs(*(uint16_t*)&(GET_8022SNAP_HDR(input)->type));
				syslog(LOG_WARNING, "[ETHER] mcast addr  from: %s, to: %s, proto: 0x%04x.",
				                    mac2str(NULL, eth->shost), mac2str(NULL, eth->dhost), proto);

#endif	/* of #ifdef ETHER_CFG_MCAST_WARNING */

				NET_COUNT_ETHER(net_count_ether.in_err_packets, 1);
				NET_COUNT_MIB(if_stats.ifInErrors, 1);
				syscall(rel_net_buf(input));
				continue;
				}

#endif	/* of #if defined(_IP4_CFG) && defined(ETHER_CFG_ACCEPT_ALL) */

			switch(proto) {

#if defined(_IP4_CFG)

			case ETHER_TYPE_IP:		/* IP	*/
				ip_input(input);
				break;

			case ETHER_TYPE_ARP:		/* ARP	*/
				arp_input(&ic->ifaddr, input);
				break;

#endif	/* of #if defined(_IP4_CFG) */

#if defined(_IP6_CFG)

			case ETHER_TYPE_IPV6:		/* IPv6	*/
				ip6_input(input);
				break;

#endif	/* of #if defined(_IP6_CFG) */

			default:

#ifdef ETHER_CFG_UNEXP_WARNING

#ifdef ETHER_CFG_802_WARNING

				if (proto <= 1500)
					ieee_802_input(input);
				else
					syslog(LOG_NOTICE, "[ETHER] unexp proto from: %s, proto: 0x%04x.",
					                   mac2str(NULL, GET_ETHER_HDR(input)->shost), proto);

#else	/* of #ifdef ETHER_CFG_802_WARNING */

				syslog(LOG_INFO, "[ETHER] unexp proto from: %s, proto: 0x%04x.",
				                   mac2str(NULL, GET_ETHER_HDR(input)->shost), proto);

#endif	/* of #ifdef ETHER_CFG_802_WARNING */

#endif	/* of #ifdef ETHER_CFG_UNEXP_WARNING */

				NET_COUNT_ETHER(net_count_ether.in_err_packets, 1);
				NET_COUNT_MIB(if_stats.ifUnknownProtos, 1);
				syscall(rel_net_buf(input));
				break;
				}
			}
		}
	}

#endif /* of #ifdef SUPPORT_ETHER */
