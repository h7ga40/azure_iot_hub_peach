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

#ifndef _TINET_CONFIG_H_
#define _TINET_CONFIG_H_

/*
 *  TINET 1.2 との互換性のための定義
 */

/* TCPの受動オープンをサポートする【TINET 1.7 以降不要になった】。*/

#if 0
#define TCP_CFG_PASSIVE_OPEN
#endif

/* TCPのノンブロッキングコール機能を組込む。*/

#define TCP_CFG_NON_BLOCKING

/* UDPのノンブロッキングコール機能を組込む。*/

#define UDP_CFG_NON_BLOCKING

/*
 *  NIC、システム、CPU、応用プログラムに
 *  依存する定義ファイルをインクルードする。
 */

#if defined(TARGET_KERNEL_JSP)

#ifdef SUPPORT_ETHER

#include "tinet_nic_config.h"

#endif	/* of #ifdef SUPPORT_ETHER */

#include "tinet_sys_config.h"
#include "tinet_cpu_config.h"
#include "tinet_app_config.h"

#elif defined(TARGET_KERNEL_ASP)

#ifdef SUPPORT_ETHER

#include "tinet_nic_config.h"

#endif	/* of #ifdef SUPPORT_ETHER */

#include "tinet_target_config.h"
#include "tinet_app_config.h"

#endif	/* of #if defined(TARGET_KERNEL_JSP) */

/*
 *  TINET 1.1 との互換性のための定義
 */

#ifdef  TMO_ICMP4_OUTPUT
#define TMO_ICMP_OUTPUT	TMO_ICMP4_OUTPUT
#endif

/*
 *  TINET 1.2 との互換性のための定義
 */

#ifdef  NUM_MPF_NET_BUF_4096
#define NUM_MPF_NET_BUF_REASSM	NUM_MPF_NET_BUF_4096
#endif

/*
 *  TINET 1.3 との互換性のための定義

#ifdef  NUM_MPF_NET_BUF_REASSM
#define NUM_MPF_NET_BUF4_REASSM	NUM_MPF_NET_BUF_REASSM
#endif
 */

/*
 *  TINET 1.5 との互換性のための定義
 */

/* ルーティング表のエントリ数の定義 */

#define NUM_IN6_ROUTE_ENTRY			\
	(NUM_IN6_STATIC_ROUTE_ENTRY + NUM_IN6_REDIRECT_ROUTE_ENTRY)
					/* ルーティング表のエントリ数	*/

#define NUM_IN4_ROUTE_ENTRY		\
	(NUM_IN4_STATIC_ROUTE_ENTRY + NUM_IN4_REDIRECT_ROUTE_ENTRY)
					/* ルーティング表のエントリ数	*/

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

#else	/* #if defined(SUPPORT_INET4) */

#ifndef NUM_IN6_STATIC_ROUTE_ENTRY
#define NUM_IN6_STATIC_ROUTE_ENTRY	NUM_STATIC_ROUTE_ENTRY
#endif

#ifndef NUM_IN6_REDIRECT_ROUTE_ENTRY
#define NUM_IN6_REDIRECT_ROUTE_ENTRY	NUM_REDIRECT_ROUTE_ENTRY
#endif

#ifdef  NUM_ROUTE_ENTRY
#undef  NUM_ROUTE_ENTRY
#endif
#define NUM_ROUTE_ENTRY			NUM_IN6_ROUTE_ENTRY

#endif	/* #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#ifndef NUM_IN4_STATIC_ROUTE_ENTRY
#define NUM_IN4_STATIC_ROUTE_ENTRY	NUM_STATIC_ROUTE_ENTRY
#endif

#ifndef NUM_IN4_REDIRECT_ROUTE_ENTRY
#define NUM_IN4_REDIRECT_ROUTE_ENTRY	NUM_REDIRECT_ROUTE_ENTRY
#endif

#ifdef  NUM_ROUTE_ENTRY
#undef  NUM_ROUTE_ENTRY
#endif
#define NUM_ROUTE_ENTRY			NUM_IN4_ROUTE_ENTRY

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

/* DHCP の定義 */

#ifdef  DHCP4_CLI_CFG
#define DHCP_CFG
#endif

/*
 *  IPv6/IPv4 に関する定義
 */

/*
 *  指定されたマクロに対する TINET 内部のマクロの定義
 *
 *    SUPPORT_INET6		 .   .  ON  ON  ON  ON
 *    SUPPORT_INET4		ON  ON   .   .  ON  ON
 *    API_CFG_IP4MAPPED_ADDR	 .  ON   .  ON   .  ON
 *    ----------------------    ----------------------
 *    _IP6_CFG                   .   .  ON  ON  ON  ON
 *    _IP4_CFG                  ON  ON   .  ON  ON  ON
 *    
 */

#if !defined(SUPPORT_INET6) && defined(API_CFG_IP4MAPPED_ADDR)
#error option selection: !defined(SUPPORT_INET6) && defined(API_CFG_IP4MAPPED_ADDR)
#endif

#if defined(SUPPORT_INET6)
#define	_IP6_CFG
#endif

#if defined(SUPPORT_INET4) || (defined(SUPPORT_INET6) && defined(API_CFG_IP4MAPPED_ADDR))
#define _IP4_CFG
#endif

/*
 *  TCP に関する定義
 */

/*
 *  MAX_TCP_RCV_SEG: 受信セグメントサイズの最大値
 *
 *    注意: 変更すべきではない。
 */

#ifndef MAX_TCP_RCV_SEG
#define MAX_TCP_RCV_SEG		(IF_MTU - (IP_HDR_SIZE + TCP_HDR_SIZE))
#endif

/*
 *  DEF_TCP_SND_SEG: 送信セグメントサイズの規定値
 */

#ifndef DEF_TCP_SND_SEG
#define DEF_TCP_SND_SEG		MAX_TCP_SND_SEG
#endif

/*
 *  TCP 通信端点の送受信ウィンドバッファの省コピー機能
 */

#ifdef  TCP_CFG_SWBUF_CSAVE_ONLY
#ifndef TCP_CFG_SWBUF_CSAVE
#define TCP_CFG_SWBUF_CSAVE
#endif
#endif	/* of #ifdef  TCP_CFG_SWBUF_CSAVE_ONLY */

#ifdef  TCP_CFG_RWBUF_CSAVE_ONLY
#ifndef TCP_CFG_RWBUF_CSAVE
#define TCP_CFG_RWBUF_CSAVE
#endif
#endif	/* of #ifdef  TCP_CFG_RWBUF_CSAVE_ONLY */

/*
 *  TCP_CFG_URG_OFFSET: 緊急データの最後のバイトのオフセット
 *
 *    -1: BSD の実装、緊急ポインタは、緊急データの最後のバイトの次のバイトを差す。
 *     0: RFC1122 の規定、緊急ポインタは、緊急データの最後のバイトを差す。
 */

#ifndef TCP_CFG_URG_OFFSET
#define TCP_CFG_URG_OFFSET	-1
#endif

/*
 *  ARP に関する定義
 */

#ifndef ARP_CACHE_KEEP
#define ARP_CACHE_KEEP		(20*60*NET_TIMER_HZ)	/* ARP キャッシュのタイムアウト	*/
#endif

/*
 *  ループバックインタフェースに関する定義
 */

#ifndef LOMTU
#define LOMTU			1500	/* ループバックの MTU			*/
#endif

/*
 * TINET サポート関数に関する定義
 */

#ifndef NUM_IPV4ADDR_STR_BUFF		/* ip2str における IP アドレス文字列用バッファ数 */

#if defined(SUPPORT_PPP)

#define NUM_IPV4ADDR_STR_BUFF	2

#else	/* of #if defined(SUPPORT_PPP) */

#define NUM_IPV4ADDR_STR_BUFF	1

#endif	/* of #if defined(SUPPORT_PPP) */

#endif	/* of #ifndef NUM_IPV4ADDR_STR_BUFF */

#ifndef NUM_IPV6ADDR_STR_BUFF
#define NUM_IPV6ADDR_STR_BUFF	2	/* ip62str における IP アドレス文字列用バッファ数 */
#endif

#ifndef NUM_MACADDR_STR_BUFF		/* macstr における MAC アドレス文字列用バッファ数 */

#if defined(SUPPORT_INET4) && defined(ETHER_CFG_ACCEPT_ALL) && defined(ETHER_CFG_MCAST_WARNING)

#define NUM_MACADDR_STR_BUFF	2

#else	/* of #if defined(SUPPORT_INET4) && defined(ETHER_CFG_ACCEPT_ALL) && defined(ETHER_CFG_MCAST_WARNING) */

#define NUM_MACADDR_STR_BUFF	1

#endif	/* of #if defined(SUPPORT_INET4) && defined(ETHER_CFG_ACCEPT_ALL) && defined(ETHER_CFG_MCAST_WARNING) */

#endif	/* of #ifndef NUM_MACADDR_STR_BUFF */

/*
 *  TINET 内部のタイムアウト値の設定
 */

#ifndef TMO_IP4_FRAG_GET_NET_BUF
#define TMO_IP4_FRAG_GET_NET_BUF	ULONG_C(1000000)	/* [us]、IPv4 データグラム再構成用ネットワークバッファ獲得タイムアウト	*/
#endif

#ifndef TMO_IP6_FRAG_GET_NET_BUF
#define TMO_IP6_FRAG_GET_NET_BUF	ULONG_C(1000000)	/* [us]、IPv6 データグラム再構成用ネットワークバッファ獲得タイムアウト	*/
#endif

#ifndef TMO_PPP_GET_NET_BUF
#define TMO_PPP_GET_NET_BUF		ULONG_C(1000000)	/* [us]、PPP 受信用ネットワークバッファ獲得タイムアウト	*/
#endif

#ifndef TMO_ARP_GET_NET_BUF
#define TMO_ARP_GET_NET_BUF		ULONG_C(1000000)	/* [us]、ARP のネットワークバッファ獲得タイムアウト	*/
#endif

#ifndef TMO_TCP_GET_NET_BUF
#define TMO_TCP_GET_NET_BUF		ULONG_C(1000000)	/* [us]、TCP のネットワークバッファ獲得タイムアウト	*/
#endif

#ifndef TMO_TCP_OUTPUT
#define TMO_TCP_OUTPUT			ULONG_C(1000000)	/* [us]、TCP 出力タイムアウト				*/
#endif

#ifndef TMO_UDP_OUTPUT
#define TMO_UDP_OUTPUT			ULONG_C(1000000)	/* [us]、UDP 出力タイムアウト				*/
#endif

#ifndef TMO_ICMP_OUTPUT
#define TMO_ICMP_OUTPUT			ULONG_C(1000000)	/* [us]、ICMP 出力タイムアウト				*/
#endif

#ifndef TMO_IGMP_OUTPUT
#define TMO_IGMP_OUTPUT			ULONG_C(1000000)	/* [us]、IGMP 出力タイムアウト				*/
#endif

#ifndef TMO_ND6_NS_OUTPUT
#define TMO_ND6_NS_OUTPUT		ULONG_C(1000000)	/* [us]、近隣要請出力タイムアウト			*/
#endif

#ifndef TMO_ND6_NA_OUTPUT
#define TMO_ND6_NA_OUTPUT		ULONG_C(1000000)	/* [us]、近隣通知出力タイムアウト			*/
#endif

#ifndef TMO_ND6_RS_OUTPUT
#define TMO_ND6_RS_OUTPUT		ULONG_C(1000000)	/* [us]、ルータ要請出力タイムアウト			*/
#endif

#ifndef TMO_ARP_OUTPUT
#define TMO_ARP_OUTPUT			ULONG_C(1000000)	/* [us]、アドレス解決要求出力タイムアウト		*/
#endif

#ifndef TMO_ND6_RTR_SOL_DELAY
#define TMO_ND6_RTR_SOL_DELAY		ULONG_C(1000000)	/* [us]、ルータ要請出力遅延				*/
#endif

#ifndef TMO_ND6_RTR_SOL_INTERVAL
#define TMO_ND6_RTR_SOL_INTERVAL	ULONG_C(4000000)	/* [us]、ルータ要請出力間隔				*/
#endif

#ifndef TMO_IN_REDIRECT
					/* [s]、向け直しルーティングエントリタイムアウト*/
#define TMO_IN_REDIRECT			(10*60*NET_TIMER_HZ)
#endif

/*
 *  TINET 内部のタスクのスタックサイズの設定
 */

#ifndef TCP_OUT_TASK_STACK_SIZE
#define	TCP_OUT_TASK_STACK_SIZE	1024	/* TCP 出力タスクのスタックサイズ		*/
#endif

#ifndef UDP_OUT_TASK_STACK_SIZE
#define	UDP_OUT_TASK_STACK_SIZE	1024	/* UDP 出力タスクのスタックサイズ			*/
#endif

#ifndef PPP_INPUT_STACK_SIZE
#define	PPP_INPUT_STACK_SIZE	1024	/* PPP タスクのスタックサイズ		*/
#endif

#ifndef PPP_OUTPUT_STACK_SIZE
#define	PPP_OUTPUT_STACK_SIZE	1024	/* PPP 出力タスクのスタックサイズ		*/
#endif

#ifndef ETHER_INPUT_STACK_SIZE
#define	ETHER_INPUT_STACK_SIZE	1024	/* Ethernet 入力タスクのスタックサイズ	*/
#endif

#ifndef ETHER_OUTPUT_STACK_SIZE
#define	ETHER_OUTPUT_STACK_SIZE	1024	/* Ethernet 出力タスクのスタックサイズ	*/
#endif

#ifndef LOOP_INPUT_STACK_SIZE
#define	LOOP_INPUT_STACK_SIZE	1024	/* ループバック入力タスクのスタックサイズ	*/
#endif

#ifndef LOOP_OUTPUT_STACK_SIZE
#define	LOOP_OUTPUT_STACK_SIZE	1024	/* ループバック出力タスクのスタックサイズ	*/
#endif

#ifndef NET_TIMER_STACK_SIZE
#define	NET_TIMER_STACK_SIZE	1024	/* ネットワークタイマタスクのスタックサイズ */
#endif

/*
 *  TINET 内部のタスクの優先度の設定
 */

#ifndef TCP_OUT_TASK_PRIORITY
#define TCP_OUT_TASK_PRIORITY	5	/* TCP 出力タスクの優先度			*/
#endif

#ifndef UDP_OUT_TASK_PRIORITY
#define UDP_OUT_TASK_PRIORITY	5	/* UDP 出力タスクの優先度				*/
#endif

#ifndef PPP_INPUT_PRIORITY
#define PPP_INPUT_PRIORITY	5	/* PPP タスクの優先度			*/
#endif

#ifndef PPP_OUTPUT_PRIORITY
#define PPP_OUTPUT_PRIORITY	5	/* PPP タスクの優先度			*/
#endif

#ifndef ETHER_INPUT_PRIORITY
#define ETHER_INPUT_PRIORITY	5	/* Ethernet 入力タスクの優先度		*/
#endif

#ifndef ETHER_OUTPUT_PRIORITY
#define ETHER_OUTPUT_PRIORITY	5	/* Ethernet 出力タスクの優先度		*/
#endif

#ifndef LOOP_INPUT_PRIORITY
#define LOOP_INPUT_PRIORITY	5	/* ループバック入力タスクの優先度		*/
#endif

#ifndef LOOP_OUTPUT_PRIORITY
#define LOOP_OUTPUT_PRIORITY	5	/* ループバック出力タスクの優先度		*/
#endif

#ifndef NET_TIMER_PRIORITY
#define NET_TIMER_PRIORITY	5	/* ネットワークタイマタスクの優先度 */
#endif

/*
 *  TINET 内部のデータキューのエントリ数の設定
 */

/* UDP 受信キューサイズ */

#ifndef NUM_DTQ_UDP_RCVQ
#define NUM_DTQ_UDP_RCVQ	1	/* 非コールバック用	*/
#endif

#ifndef NUM_DTQ_UDP_OUTPUT
#define NUM_DTQ_UDP_OUTPUT	1	/* UDP 出力データキューサイズ			*/
#endif

#ifndef NUM_DTQ_PPP_OUTPUT
#define NUM_DTQ_PPP_OUTPUT	2	/* PPP 出力データキューサイズ、2 以上	*/
#endif

#ifndef NUM_DTQ_ETHER_OUTPUT
#define NUM_DTQ_ETHER_OUTPUT	2	/* Ethernet 出力データキューサイズ	*/
#endif

#ifndef NUM_DTQ_LOOP_INPUT
#define NUM_DTQ_LOOP_INPUT	0	/* ループバック入力データキューサイズ	*/
#endif

#ifndef NUM_DTQ_LOOP_OUTPUT
#define NUM_DTQ_LOOP_OUTPUT	0	/* ループバック出力データキューサイズ	*/
#endif

/*
 *  TINET プロトコルスタック内部のリネーム
 */

#include <net/net_rename.h>
#include <netinet/in_rename.h>
#include <netinet6/in6_rename.h>

#endif /* _TINET_CONFIG_H_ */
