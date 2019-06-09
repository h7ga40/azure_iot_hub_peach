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

#ifndef _TINET_APP_CONFIG_H_
#define _TINET_APP_CONFIG_H_

/*
 *  インターネットプロトコルファミリーのサポート
 *
 *    Makefile で指定しても良い
 */

/*#define SUPPORT_INET4		TCP/IP,IPv4		*/
/*#define SUPPORT_INET6		TCP/IP,IPv6		*/
/*#define SUPPORT_TCP		TCP			*/
/*#define SUPPORT_UDP		UDP			*/

/*データリンク層のネットワークインタフェースを選択する	*/
/*いずれか一つだけ選択できる。				*/

/*#define SUPPORT_PPP		PointtoPointProtocol	*/
/*#define SUPPORT_LOOP		LocalLoopBack		*/
/*#define SUPPORT_ETHER		Ethernet		*/

/*#define SUPPORT_MIB		MIB(SNMP)		*/

/*
 *  TCP/IP に関する定義
 */

/* TCP に関する定義 */

/*
 *  TCP 通信端点の送受信ウィンドバッファの省コピー機能
 *    注意: Makefile で指定している。
 */
/*#define TCP_CFG_RWBUF_CSAVE_ONLY*/
			/* TCP 通信端点の受信ウィンドバッファの省コピー機能のみ有効にする。	*/
/*#define TCP_CFG_RWBUF_CSAVE*/
			/* TCP 通信端点の受信ウィンドバッファの省コピー機能を有効にする。	*/
/*#define TCP_CFG_SWBUF_CSAVE_ONLY*/
			/* TCP 通信端点の送信ウィンドバッファの省コピー機能のみ有効にする。	*/
/*#define TCP_CFG_SWBUF_CSAVE*/
			/* TCP 通信端点の送信ウィンドバッファの省コピー機能を有効にする。	*/

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能の、
 *  受信ウィンドバッファキューの最大エントリ数。
 *  ただし、正常に受信したセグメントも破棄するため、再送回数が増加する。
 *  また、指定しないと制限しない。
 */
/*#define TCP_CFG_RWBUF_CSAVE_MAX_QUEUES	2*/

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能で、
 *  送信ウィンドバッファに使用するネットワークバッファのサイズ。
 */
#define USE_TCP_MSS_SEG

#ifdef USE_TCP_MSS_SEG

#ifdef SUPPORT_INET6

#define TCP_CFG_SWBUF_CSAVE_MAX_SIZE	(IF_HDR_SIZE + IPV6_MMTU)	/* 最大サイズ */

#else	/* of #ifdef SUPPORT_INET6 */

#ifdef SUPPORT_INET4
#define TCP_CFG_SWBUF_CSAVE_MAX_SIZE	(IF_HDR_SIZE + IP4_MSS)		/* 最大サイズ */
#endif

#endif	/* of #ifdef SUPPORT_INET6 */

#else	/* of #ifdef USE_TCP_MSS_SEG */

#define TCP_CFG_SWBUF_CSAVE_MAX_SIZE	IF_PDU_SIZE			/* 最大サイズ */

#endif	/* of #ifdef USE_TCP_MSS_SEG */

#define TCP_CFG_SWBUF_CSAVE_MIN_SIZE	0				/* 最小サイズ */

/*
 *  TCP のノンブロッキングコール機能
 */

#ifdef UNDEF_TCP_CFG_NON_BLOCKING
#undef TCP_CFG_NON_BLOCKING
#endif

/*
 *  MAX_TCP_SND_SEG: 送信セグメントサイズの最大値
 *
 *    相手から MSS オプションでセグメントサイズを指定されても、
 *    この値で、セグメントサイズを制限できる。
 */

#ifdef USE_TCP_MSS_SEG

#ifdef MAX_TCP_SND_SEG
#undef MAX_TCP_SND_SEG
#endif

#ifdef SUPPORT_INET6

#define MAX_TCP_SND_SEG			TCP6_MSS

#else	/* of #ifdef SUPPORT_INET6 */

#ifdef SUPPORT_INET4
#define MAX_TCP_SND_SEG			TCP_MSS
#endif

#endif	/* of #ifdef SUPPORT_INET6 */

#endif	/* of #ifdef USE_TCP_MSS_SEG */

/*
 *  DEF_TCP_RCV_SEG: 受信セグメントサイズの規定値
 */

#ifdef USE_TCP_MSS_SEG

#ifdef DEF_TCP_RCV_SEG
#undef DEF_TCP_RCV_SEG
#endif

#ifdef SUPPORT_INET6

#define DEF_TCP_RCV_SEG			TCP6_MSS

#else	/* of #ifdef SUPPORT_INET6 */

#ifdef SUPPORT_INET4
#define DEF_TCP_RCV_SEG			TCP_MSS
#endif

#endif	/* of #ifdef SUPPORT_INET6 */

#endif	/* of #ifdef USE_TCP_MSS_SEG */

/*
 *  タスクからの Time Wait 状態の TCP 通信端点分離機能
 *
 *  Time Wait 状態の TCP 通信端点のエントリ数を指定する。
 *  指定しないか、0 を指定すると、
 *  タスクから Time Wait 状態の TCP 通信端点を分離する機能は組込まない。
 */
#if !defined(SUPPORT_TCP)
#define NUM_TCP_TW_CEP_ENTRY		0
#elif defined(USE_TCP_MSS_SEG)
#define NUM_TCP_TW_CEP_ENTRY		3
#else
#define NUM_TCP_TW_CEP_ENTRY		6
#endif

/*
 *  TCP ヘッダのトレース出力機能
 */
//#define TCP_CFG_TRACE

/*
 *  トレース出力対象のリモートホストの IPv4 アドレス
 *  IPV4_ADDRANY を指定すると、全てのホストを対象とする。
 */
#define TCP_CFG_TRACE_IPV4_RADDR	IPV4_ADDRANY

/*
 *  トレース出力対象のリモートホストのポート番号
 *  TCP_PORTANY を指定すると、全てのポート番号を対象にする。
 */
#define TCP_CFG_TRACE_RPORTNO		TCP_PORTANY
//#define TCP_CFG_TRACE_RPORTNO		UINT_C(7)

/*
 *  トレース出力対象のローカルホストのポート番号
 *  TCP_PORTANY を指定すると、全てのポート番号を対象にする。
 */
#define TCP_CFG_TRACE_LPORTNO		TCP_PORTANY
//#define TCP_CFG_TRACE_LPORTNO		UINT_C(7)

/*
 *  ITRON TCP/IP API、TCP の拡張機能
 */
#define TCP_CFG_EXTENTIONS

/*
 *  TCP_CFG_URG_OFFSET: 緊急データの最後のバイトのオフセット
 *
 *    -1: BSD の実装、緊急ポインタは、緊急データの最後のバイトの次のバイトを差す。
 *     0: RFC1122 の規定、緊急ポインタは、緊急データの最後のバイトを差す。
 */

//#define TCP_CFG_URG_OFFSET	-1
//#define TCP_CFG_URG_OFFSET	0

/* UDP に関する定義 */

/*
 *  UDP のノンブロッキングコール機能
 */

#ifdef UNDEF_UDP_CFG_NON_BLOCKING
#undef UDP_CFG_NON_BLOCKING
#endif

/*
 *  ITRON TCP/IP API、UDP の拡張機能
 */
#define UDP_CFG_EXTENTIONS

/* IPv6 に関する定義 */

#ifdef SUPPORT_PPP

#define NUM_IN6_STATIC_ROUTE_ENTRY	0
#define NUM_IN6_REDIRECT_ROUTE_ENTRY	0

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_LOOP

#define NUM_IN6_STATIC_ROUTE_ENTRY	0
#define NUM_IN6_REDIRECT_ROUTE_ENTRY	0

#endif	/* of #ifdef SUPPORT_LOOP */

#ifdef SUPPORT_ETHER

#define NUM_IN6_STATIC_ROUTE_ENTRY	0
#define NUM_IN6_REDIRECT_ROUTE_ENTRY	1

#endif	/* of #ifdef SUPPORT_ETHER */

/* IPv4 に関する定義 */

#ifdef SUPPORT_PPP

#if 1
#define IPV4_ADDR_LOCAL			MAKE_IPV4_ADDR(192,168,1,21)
#else
#define IPV4_ADDR_LOCAL			MAKE_IPV4_ADDR(0,0,0,0)		/* 相手に割り当ててもらう場合	*/
#endif

#if 1
#define IPV4_ADDR_REMOTE		MAKE_IPV4_ADDR(192,168,1,31)
#else
#define IPV4_ADDR_REMOTE		MAKE_IPV4_ADDR(0,0,0,0)		/* 相手に割り当ててもらう場合	*/
#endif

#define NUM_IN4_STATIC_ROUTE_ENTRY	1
#define NUM_IN4_REDIRECT_ROUTE_ENTRY	0

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

#define IPV4_ADDR_STAIC_LOCAL			MAKE_IPV4_ADDR(192,168,137,201)
#define IPV4_ADDR_STAIC_LOCAL_MASK		MAKE_IPV4_ADDR(255,255,255,0)
#define IPV4_ADDR_STAIC_DEFAULT_GW		MAKE_IPV4_ADDR(192,168,137,1)

#ifdef DHCP4_CLI_CFG

#define IPV4_ADDR_LOCAL			MAKE_IPV4_ADDR(0,0,0,0)
#define IPV4_ADDR_LOCAL_MASK	MAKE_IPV4_ADDR(0,0,0,0)
#define IPV4_ADDR_DEFAULT_GW	MAKE_IPV4_ADDR(0,0,0,0)

#else	/* of #ifdef DHCP4_CLI_CFG */

#define IPV4_ADDR_LOCAL			IPV4_ADDR_STAIC_LOCAL
#define IPV4_ADDR_LOCAL_MASK	IPV4_ADDR_STAIC_LOCAL_MASK
#define IPV4_ADDR_DEFAULT_GW	IPV4_ADDR_STAIC_DEFAULT_GW

#endif	/* of #ifdef DHCP4_CLI_CFG */

#define NUM_IN4_STATIC_ROUTE_ENTRY	3
#define NUM_IN4_REDIRECT_ROUTE_ENTRY	1

#endif	/* of #ifdef SUPPORT_ETHER */

#ifdef SUPPORT_LOOP

#define NUM_IN4_STATIC_ROUTE_ENTRY	1
#define NUM_IN4_REDIRECT_ROUTE_ENTRY	0

#endif	/* of #ifdef SUPPORT_LOOP */

/*
 *  データリンク層 (ネットワークインタフェース) に関する定義
 */

/*
 *  イーサネットに関する定義
 */

/*
 *  PPP に関する定義
 */

#define HDLC_PORTID			1	/* HDLC(PPP)に用いるシリアルポート番号	*/

/*#define PPP_IDLE_TIMEOUT		(180*NET_TIMER_HZ)*/
					/* 切断までのアイドル時間、		*/
					/* 切断しない場合はコメントアウトする。	*/

/*#define PPP_CFG_MODEM			 モデム接続の場合はコメントを外す。	*/

#define MODEM_CFG_INIT			"ATE1&D0&S0\\V0\\Q1S0=1"
					/* モデム初期化文字列			*/
					/* NTT DoComo モバイルアダプタ 96F 用	*/
					/*   E1: エコーバックあり			*/
					/*  &D0: ER信号無視			*/
					/*  &S0: DR信号常時 ON			*/
					/*  \V0: リザルトコード TYPE 1		*/
					/*  \Q1: XON/XOFF フロー制御		*/
					/* S0=1: 自動着信リング回数 = 1 回	*/

#define MODEM_CFG_PHONE_NUMBER		"090-xxxx-9242"

					/* 自構成情報の規定値			*/
#define DEF_LCP_LOCAL_CFGS		(LCP_CFG_MRU|LCP_CFG_ACCM|LCP_CFG_MAGIC|\
					 LCP_CFG_ACCOMP|LCP_CFG_PCOMP|LCP_CFG_PAP)

					/* 相手に許す構成情報の規定値		*/
#define DEF_LCP_REMOTE_CFGS		(LCP_CFG_MRU|LCP_CFG_ACCM|LCP_CFG_MAGIC|\
					 LCP_CFG_ACCOMP|LCP_CFG_PCOMP|LCP_CFG_PAP)

/*
 *  認証に関する定義
 */

#define AUTH_CFG_CLIENT		   	/* 認証クライアントモードの時はコメントを外す。	*/
#define AUTH_CFG_SERVER			/* 認証サーバモードの時はコメントを外す。		*/

#define AUTH_LOCAL_USER			"h8"		/* 自ホストへのログイン認証ユーザ名	*/
#define AUTH_LOCAL_PASSWD		"3048f"		/* 自ホストへのログイン認証パスワード	*/
#define AUTH_REMOTE_USER		"pen3"		/* 相手へのログイン認証ユーザ名		*/
#define AUTH_REMOTE_PASSWD		"600MHz"	/* 相手へのログイン認証パスワード		*/

/*
 *  ARP に関する定義
 */

/*#define ARP_CFG_CALLBACK_DUPLICATED*/	/* IP アドレス重複検出コールバック関数を	*/
					/* 定義する場合はコメントを外す。		*/

/*
 *  ICMPv4/v6 に関する定義
 */

#ifdef USE_PING

#define ICMP_CFG_CALLBACK_ECHO_REPLY	/* ICMP ECHO 応答を受信したとき呼出すコールバック関数を	*/
					/* 定義する場合はコメントを外す。			*/

#endif	/* of #ifdef USE_PING */

/*
 *  ネットワークバッファに関する定義
 */

#ifdef SUPPORT_ETHER

/*
 *  イーサネットの場合のネットワークバッファ数の割り当て
 */

/*
 *  64 オクテット
 *
 *    IPv4 でのみ割り当てる。
 *
 *    IPv6 では、TCP 制御セグメント（CSEG、SDU なし）サイズの
 *    ネットワークバッファ（IF + IPv6 + TCP = 74）以下の
 *    ネットワークバッファは不要である。
 */

#ifdef NUM_MPF_NET_BUF_64
#undef NUM_MPF_NET_BUF_64
#endif

#ifdef SUPPORT_INET4
#define NUM_MPF_NET_BUF_64		4
#else
#define NUM_MPF_NET_BUF_64		0
#endif

/*
 *  CSEG（IF + IP + TCP）
 *
 *    IPv6 でのみ割り当てる。
 *
 *    MBED Ethernet Controler のディバイスドライバ（if_mbed）の最低割当て長は、
 *    イーサネットフレームの最短長である 60（CRC の 4 オクテットを除き、
 *    更にアラインして 62）オクテットである。
 *    従って、IPv4 では、CSEG サイズのネットワークバッファ
 *   （IF + IPv4 + TCP = 54）は不要である。
 */

#ifdef NUM_MPF_NET_BUF_CSEG
#undef NUM_MPF_NET_BUF_CSEG
#endif

#ifdef SUPPORT_INET6
#define NUM_MPF_NET_BUF_CSEG		4
#else
#define NUM_MPF_NET_BUF_CSEG		0
#endif

/*
 *  128 オクテット
 *
 *    IPv4 で、TCP MSS（IP MSS、IF + 576 オクテット）の
 *    ネットワークバッファを割り当てる場合、
 *    128 オクテットのネットワークバッファは割り当てない。
 */

#ifdef NUM_MPF_NET_BUF_128
#undef NUM_MPF_NET_BUF_128
#endif

#if defined(USE_TCP_MSS_SEG)
#define NUM_MPF_NET_BUF_128		0
#else
#define NUM_MPF_NET_BUF_128		2
#endif

/*
 *  256 オクテット
 *
 *    IPv4 で、TCP MSS（IP MSS、IF + 576 オクテット）の
 *    ネットワークバッファを割り当てる場合、
 *    256 オクテットのネットワークバッファは割り当てない。
 */

#ifdef NUM_MPF_NET_BUF_256
#undef NUM_MPF_NET_BUF_256
#endif

/*#if defined(USE_TCP_MSS_SEG)
#define NUM_MPF_NET_BUF_256		0
#else
#define NUM_MPF_NET_BUF_256		2
#endif*/
#define NUM_MPF_NET_BUF_256		10

/*
 *  512 オクテット	
 *
 *    IPv4 で、TCP MSS（IP MSS、IF + 576 オクテット）の
 *    ネットワークバッファを割り当てる場合、
 *    512 オクテットのネットワークバッファは割り当てない。
 */

#ifdef NUM_MPF_NET_BUF_512
#undef NUM_MPF_NET_BUF_512
#endif

#if defined(USE_TCP_MSS_SEG)
#define NUM_MPF_NET_BUF_512		0
#else
#define NUM_MPF_NET_BUF_512		2
#endif

/*
 *  TCP MSS（IP MSS、IF + 576 オクテット）
 *
 *   ・IPv4 でのみ割り当てる。
 *   ・送受信ウィンドバッファの省コピー機能が有効で、
 *     省コピー API を使用するときは、+1 割り当てる。
 */

#ifdef NUM_MPF_NET_BUF_IP_MSS
#undef NUM_MPF_NET_BUF_IP_MSS
#endif

#if defined(SUPPORT_INET4) && defined(USE_TCP_MSS_SEG)

#if (defined(TCP_CFG_RWBUF_CSAVE)      || defined(TCP_CFG_SWBUF_CSAVE) ||	\
     defined(TCP_CFG_RWBUF_CSAVE_ONLY) || defined(TCP_CFG_SWBUF_CSAVE_ONLY)) && defined(USE_COPYSAVE_API)
#define NUM_MPF_NET_BUF_IP_MSS		6
#else
#define NUM_MPF_NET_BUF_IP_MSS		4	/* IF + 576 オクテット	*/
#endif

#else	/* of #if defined(SUPPORT_INET4) && defined(USE_TCP_MSS_SEG) */

#define NUM_MPF_NET_BUF_IP_MSS		0	/* IF + 576 オクテット	*/

#endif	/* of #if defined(SUPPORT_INET4) && defined(USE_TCP_MSS_SEG) */

/*
 *  1024 オクテット	
 *
 *    TCP MSS のネットワークバッファを割り当てる場合、
 *    1024 オクテットのネットワークバッファは割り当てない。
 */

#ifdef NUM_MPF_NET_BUF_1024
#undef NUM_MPF_NET_BUF_1024
#endif

#if defined(USE_TCP_MSS_SEG)
#define NUM_MPF_NET_BUF_1024		0	/* 1024 オクテット	*/
#else
#define NUM_MPF_NET_BUF_1024		2	/* 1024 オクテット	*/
#endif

/*
 *  TCP MSS（IPV6 MMTU、IF + 1280 オクテット）
 *
 *   ・IPv6 でのみ割り当てる。
 *   ・送受信ウィンドバッファの省コピー機能が有効で、
 *     省コピー API を使用するときは、+1 割り当てる。
 */

#ifdef NUM_MPF_NET_BUF_IPV6_MMTU
#undef NUM_MPF_NET_BUF_IPV6_MMTU
#endif

#if defined(SUPPORT_INET6) && (defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU))

#if (defined(TCP_CFG_RWBUF_CSAVE)      || defined(TCP_CFG_SWBUF_CSAVE) ||	\
     defined(TCP_CFG_RWBUF_CSAVE_ONLY) || defined(TCP_CFG_SWBUF_CSAVE_ONLY)) && defined(USE_COPYSAVE_API)
#define NUM_MPF_NET_BUF_IPV6_MMTU	6	/* IF + 1280	*/
#else
#define NUM_MPF_NET_BUF_IPV6_MMTU	4	/* IF + 1280	*/
#endif

#else	/* of #if defined(SUPPORT_INET6) && (defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU)) */

#define NUM_MPF_NET_BUF_IPV6_MMTU	0	/* IF + 1280	*/

#endif	/* of #if defined(SUPPORT_INET6) && (defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU)) */

/*
 *  IF 最大 PDU サイズ
 *
 *   ・TCP MSS のネットワークバッファを割り当てる場合、
 *     IF 最大 PDU サイズのネットワークバッファは割り当てない。
 *   ・送受信ウィンドバッファの省コピー機能が有効で、
 *     省コピー API を使用するときは、+1 割り当てる。
 */

#ifdef NUM_MPF_NET_BUF_IF_PDU
#undef NUM_MPF_NET_BUF_IF_PDU
#endif

#ifdef USE_TCP_MSS_SEG

#define NUM_MPF_NET_BUF_IF_PDU		0

#else	/* of #ifdef USE_TCP_MSS_SEG */

#if (defined(TCP_CFG_RWBUF_CSAVE) || defined(TCP_CFG_SWBUF_CSAVE)) && defined(USE_COPYSAVE_API)
#define NUM_MPF_NET_BUF_IF_PDU		12
#else
#define NUM_MPF_NET_BUF_IF_PDU		10
#endif

#endif	/* of #ifdef USE_TCP_MSS_SEG */

/*
 *  65536 オクテット
 *
 *  ・NET_BUF_CFG_LONG_LEN を定義したときのみ割り当てる。
 */

#ifdef NET_BUF_CFG_LONG_LEN

/* IPv6 用 */

#ifdef NUM_MPF_NET_BUF6_65536
#undef NUM_MPF_NET_BUF6_65536
#endif

#if defined(USE_TCP_MSS_SEG)
#define NUM_MPF_NET_BUF6_65536	0
#else
#define NUM_MPF_NET_BUF6_65536	4
#endif

#else	/* of ifdef NET_BUF_CFG_LONG_LEN */

#define NUM_MPF_NET_BUF6_65536	0

#endif	/* of ifdef NET_BUF_CFG_LONG_LEN */

#endif	/* of #ifdef SUPPORT_ETHER */

/*
 *  DNS サーバに関する定義
 */

/* DNS サーバの IP アドレス */

//#if !defined(DHCP6_CLI_CFG)

#define IPV6_ADDR_DNS_INIT	\
	{{{ UINT_C(0xfd), UINT_C(0x90), UINT_C(0xcc), UINT_C(0xe5), \
	    UINT_C(0x25), UINT_C(0xf6), UINT_C(0xff), UINT_C(0x81), \
	    UINT_C(0x02), UINT_C(0xa0), UINT_C(0x24), UINT_C(0xff), \
	    UINT_C(0xfe), UINT_C(0x56), UINT_C(0xc5), UINT_C(0xd6) }}}

//#endif	/* of #if !defined(DHCP6_CLI_CFG) */

//#if !defined(DHCP4_CLI_CFG)
#define IPV4_ADDR_DNS		MAKE_IPV4_ADDR(192,168,137,1)
//#endif

/* DOMAIN 名 */

#if !(defined(DHCP4_CLI_CFG) || defined(DHCP6_CLI_CFG))
#define RSLV_CFG_DNS_DOMAIN_NAME_STR	"jo.tomakomai-ct.ac.jp"
#endif

/*
 *  DHCP クライアントに関する定義
 */

/* DHCPv6 の動作モードの設定 */

#define DHCP6_CLI_CFG_MODE	DHCP6_CLI_CFG_STATELESS
//#define DHCP6_CLI_CFG_MODE	DHCP6_CLI_CFG_STATEFULL

#define ETHER_OUTPUT_PRIORITY	7	/* Ethernet 出力タスクの優先度		*/
#define TCP_OUT_TASK_PRIORITY	6	/* TCP 出力タスクの優先度			*/
#define NUM_DTQ_ETHER_OUTPUT	16	/* Ethernet 出力データキューサイズ	*/

#endif /* _TINET_APP_CONFIG_H_ */
