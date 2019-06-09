/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2009 by Dep. of Computer Science and Engineering
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

#ifndef _TINET_TARGET_CONFIG_H_
#define _TINET_TARGET_CONFIG_H_

/*
 *  TCP/IP に関する定義
 */

/* TCP に関する定義 */

/*
 *  MAX_TCP_SND_SEG: 送信セグメントサイズの最大値
 *
 *    相手から MSS オプションでセグメントサイズを指定されても、
 *    この値で、セグメントサイズを制限できる。
 */

#ifndef MAX_TCP_SND_SEG
#define MAX_TCP_SND_SEG		(IF_MTU - (IP_HDR_SIZE + TCP_HDR_SIZE))
#endif	/* of #ifndef MAX_TCP_SND_SEG */

/*
 *  DEF_TCP_RCV_SEG: 受信セグメントサイズの規定値
 */

#ifndef DEF_TCP_RCV_SEG
#define DEF_TCP_RCV_SEG		(IF_MTU - (IP_HDR_SIZE + TCP_HDR_SIZE))
#endif	/* of #ifndef DEF_TCP_RCV_SEG */

/* 
 *  セグメントの順番を入れ替えるときに新たにネットワークバッファを割当てて、
 *  データをコピーするサイズのしきい値
 */
#define MAX_TCP_REALLOC_SIZE	1024	

#define TCP_CFG_OPT_MSS		/* コネクション開設時に、セグメントサイズオプションをつけて送信する。*/
#define TCP_CFG_DELAY_ACK	/* ACK を遅らせるときはコメントを外す。			*/
#define TCP_CFG_ALWAYS_KEEP	/* 常にキープアライブする時はコメントを外す。		*/

/* UDP に関する定義 */

#define UDP_CFG_IN_CHECKSUM	/* UDP の入力チェックサムを行う場合はコメントを外す。	*/
#define UDP_CFG_OUT_CHECKSUM	/* UDP の出力チェックサムを行う場合はコメントを外す。	*/

/* ICMPv4/v6 に関する定義 */

#define ICMP_REPLY_ERROR		/* ICMP エラーメッセージを送信する場合はコメントを外す。*/

/* IPv4 に関する定義 */

#define IP4_CFG_FRAGMENT		/* データグラムの分割・再構成行う場合はコメントを外す。	*/
#define NUM_IP4_FRAG_QUEUE	2	/* データグラム再構成キューサイズ			*/
#define IP4_CFG_FRAG_REASSM_SIZE	4096	/* IPv4 再構成バッファサイズ			*/

/* IPv6 に関する定義 */

#define NUM_IP6_DAD_COUNT	1	/* 重複アドレス検出で送信する近隣要請の回数、		*/
					/*  0 を指定すると、重複アドレス検出を行わない。	*/
#define NUM_ND6_CACHE_ENTRY	10	/* 近隣キャッシュのエントリ数			*/

/*
 *  IPv6 用ホストキャッシュのエントリ数。
 *  0 を指定するとIPv6 用ホストキャッシュを組込まない。
 */
#define NUM_IN6_HOSTCACHE_ENTRY	4

#ifdef SUPPORT_ETHER

#define IP6_CFG_AUTO_LINKLOCAL		/* リンクローカルアドレスの自動設定を行う場合はコメントを外す。*/

/*
 *  ディフォルトルータリストに関する定義
 */

/*
 *  ディフォルトルータリストのエントリ数。
 *  最大値は 16、0 を指定するとルータ通知を受信しない。
 */
#define NUM_ND6_DEF_RTR_ENTRY		4

/*
 *  プレフィックスリストに関する定義。
 *  最大値は 16。
 */
#define NUM_ND6_PREFIX_ENTRY		4	/*  プレフィックスリストのエントリ数			*/

/*
 *  起動時のルータ要請出力回数。
 *  0 を指定するとルータ要請を出力しない。
 */
#define NUM_ND6_RTR_SOL_RETRY	3	

#define IP6_CFG_FRAGMENT		/* データグラムの分割・再構成行う場合はコメントを外す。	*/
#define NUM_IP6_FRAG_QUEUE	2	/* データグラム再構成キューサイズ			*/
#define IP6_CFG_FRAG_REASSM_SIZE	4096	/* IPv6 再構成バッファサイズ			*/

#endif	/* of #ifdef SUPPORT_ETHER */

/*
 *  データリンク層 (ネットワークインタフェース) に関する定義
 */

/*
 *  PPP、モデムに関する定義
 */

#define MODEM_CFG_DIAL		"ATD"	/* ダイアルコマンド文字列			*/
#define MODEM_CFG_RETRY_CNT	3	/* ダイアルリトライ回数			*/
#define MODEM_CFG_RETRY_WAIT	10000	/* ダイアルリトライまでの待ち時間 [ms]	*/

/*
 *  PPP、HDLC に関する定義
 */

#define DEF_LOCAL_ACCM		ULONG_C(0x000a0000)	/* 自分の ACCM、XON と XOFF のみ変換	*/
#define DEF_REMOTE_ACCM		ULONG_C(0xffffffff)	/* 相手の ACCM、初期値は全て変換		*/

/*
 *  PPP、LCP に関する定義
 */

#define LCP_CFG_MRU		UINT_C(0x0001)	/* MRU					*/
#define LCP_CFG_ACCM		UINT_C(0x0002)	/* ACCM					*/
#define LCP_CFG_MAGIC		UINT_C(0x0004)	/* マジック番号				*/
#define LCP_CFG_PCOMP		UINT_C(0x0008)	/* プロトコル部圧縮機能			*/
#define LCP_CFG_ACCOMP		UINT_C(0x0010)	/* アドレス・制御部圧縮			*/
#define LCP_CFG_PAP		UINT_C(0x0020)	/* PAP					*/
/*#define LCP_CFG_CHAP		UINT_C(0x0040)	   CHAP は実装予定			*/

#ifdef LCP_CFG_MAGIC

#define LCP_ECHO_INTERVAL	(20*NET_TIMER_HZ)	/* インターバル時間		*/
#define LCP_ECHO_FAILS		9			/* 失敗閾値			*/

#endif	/* of #ifdef LCP_CFG_MAGIC */

/*
 *  PPP、PAP に関する定義
 */

#define DEF_PAP_TIMEOUT		(3*NET_TIMER_HZ)
#define DEF_PAP_REQTIME		(30*NET_TIMER_HZ)	/* タイムアウト処理を行うときはコメントを外す。*/
#define MAX_PAP_REXMT		10			/* 認証要求の最大再送回数	*/

/*
 *  MBED Ethernet Controler に関する定義
 */

#define NUM_IF_MBED_TXBUF		NUM_OF_TX_DESCRIPTOR	/* 送信バッファ数			*/
#define NUM_IF_MBED_RXBUF		NUM_OF_RX_DESCRIPTOR	/* 受信バッファ数			*/

#define TMO_IF_MBED_GET_NET_BUF	1000	/* [us]、受信用 net_buf 獲得タイムアウト	*/
					/* [s]、 送信タイムアウト			*/
#define TMO_IF_MBED_XMIT		(2*IF_TIMER_HZ)

/*#define IF_MBED_CFG_ACCEPT_ALL		 マルチキャスト、エラーフレームも受信するときはコメントを外す。*/

/*
 *  Bluetooth USB PAN に関する定義
 */

#define NUM_IF_BTUSB_TXBUF		2	/* 送信バッファ数			*/
#define NUM_IF_BTUSB_RXBUF		2	/* 受信バッファ数			*/
#define IF_BTUSB_BUF_PAGE_SIZE	1518	/* バッファサイズ */

#define TMO_IF_BTUSB_GET_NET_BUF	1000	/* [us]、受信用 net_buf 獲得タイムアウト	*/
					/* [s]、 送信タイムアウト			*/
#define TMO_IF_BTUSB_XMIT		(2*IF_TIMER_HZ)

/*
 *  イーサネット出力時に、NIC で net_buf を開放する場合に指定する。
 *
 *  注意: 以下の指定は、指定例であり、if_mbed では、
 *        開放しないので、以下のコメントを外してはならない。
 */

/*#define ETHER_NIC_CFG_RELEASE_NET_BUF*/

/*
 *  MBED Ethernet Controler に関する定義
 */

#define INHNO_IF_MBED		ETHERI_IRQn		/* IRQ5 */
#define INTNO_IF_MBED		ETHERI_IRQn		/* IRQ5 */
#define INTATR_IF_MBED		(TA_NULL)		/* 割込み属性	*/
#define INTPRI_IF_MBED		(-1)			/* 割込み優先度	*/

/*
 *  ARP に関する定義
 */

#define NUM_ARP_ENTRY		10			/* ARP キャッシュエントリ数	*/

/*
 *  DHCP に関する定義
 *
 *    ・注意: TINET は、DHCP を実装していない。応用プログラムで、
 *            DHCP メッセージを受信するための定義である。
 *            また、現在は IPv4 のみ有効である。
 */

/*#define DHCP_CFG					 DHCP を処理する場合はコメントを外す。*/

/*
 *  Ethernet に関する定義
 */

/*#define ETHER_CFG_ACCEPT_ALL		 マルチキャスト、エラーフレームも受信するときはコメントを外す。	*/
/*#define ETHER_CFG_UNEXP_WARNING	 非サポートフレームの警告を表示するときはコメントを外す。		*/
/*#define ETHER_CFG_802_WARNING		 IEEE 802.3 フレームの警告を表示するときはコメントを外す。		*/
/*#define ETHER_CFG_MCAST_WARNING	 マルチキャストの警告を表示するときはコメントを外す。		*/
#define ETHER_CFG_MULTICAST			/* マルチキャストを送受信 */

/*
 *  アドレスリストに関する定義。
 */
#define NUM_IN6_IFADDR_ENTRY		5	/*  インタフェースのアドレスリストのエントリ数		*/

/*
 *  汎用ネットワークに関する定義
 */

/*
 *  ネットワークバッファに関する定義
 */

/* ネットワークバッファ数 */

#ifdef SUPPORT_PPP

/*
 *  PPP では、受信用の ネットワークバッファは PDU (1502) サイズを常に
 *  割り当てなければならないので PDU サイズを多めに確保する。
 */

#ifndef NUM_MPF_NET_BUF_CSEG
#define NUM_MPF_NET_BUF_CSEG	2	/* IF + IP + TCP、2 以上*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_CSEG */

#ifndef NUM_MPF_NET_BUF_64
#define NUM_MPF_NET_BUF_64	0	/* 64 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_64 */

#ifndef NUM_MPF_NET_BUF_128
#define NUM_MPF_NET_BUF_128	0	/* 128 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_128 */

#ifndef NUM_MPF_NET_BUF_256
#define NUM_MPF_NET_BUF_256	0	/* 256 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_256 */

#ifndef NUM_MPF_NET_BUF_512
#define NUM_MPF_NET_BUF_512	0	/* 512 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_512 */

#ifndef NUM_MPF_NET_BUF_1024
#define NUM_MPF_NET_BUF_1024	0	/* 1024 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_1024 */

#ifndef NUM_MPF_NET_BUF_IF_PDU
#define NUM_MPF_NET_BUF_IF_PDU	2	/* IF 最大 PDU サイズ	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_IF_PDU */

#ifndef NUM_MPF_NET_BUF4_REASSM

#if defined(SUPPORT_INET4) && defined(IP4_CFG_FRAGMENT)
#define NUM_MPF_NET_BUF4_REASSM	2	/* IPv4 再構成バッファサイズ	*/
#else
#define NUM_MPF_NET_BUF4_REASSM	0	/* IPv4 再構成バッファサイズ	*/
#endif

#endif	/* of #ifndef NUM_MPF_NET_BUF4_REASSM */

#ifndef NUM_MPF_NET_BUF6_REASSM

#if defined(SUPPORT_INET6) && defined(IP6_CFG_FRAGMENT)
#define NUM_MPF_NET_BUF6_REASSM	2	/* IPv6 再構成バッファサイズ	*/
#else
#define NUM_MPF_NET_BUF6_REASSM	0	/* IPv6 再構成バッファサイズ	*/
#endif

#endif	/* of #ifndef NUM_MPF_NET_BUF6_REASSM */

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

/*
 *  イーサネットの場合のネットワークバッファ数の割り当て
 */

/*
 *  注意!!
 *
 *  MBED Ethernet Controler のディバイスドライバ（if_mbed）の最低割当て長は
 *  60（アラインして 62）オクテットのため IF + IP +TCP よりは
 *  64 オクテットのネットワークバッファの方が最適である。
 */

#ifndef NUM_MPF_NET_BUF_CSEG
#define NUM_MPF_NET_BUF_CSEG	0	/* IF + IP + TCP	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_CSEG */

#ifndef NUM_MPF_NET_BUF_64
#define NUM_MPF_NET_BUF_64	2	/* 64 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_64 */

#ifndef NUM_MPF_NET_BUF_128
#define NUM_MPF_NET_BUF_128	0	/* 128 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_128 */

#ifndef NUM_MPF_NET_BUF_256
#define NUM_MPF_NET_BUF_256	0	/* 256 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_256 */

#ifndef NUM_MPF_NET_BUF_512
#define NUM_MPF_NET_BUF_512	0	/* 512 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_512 */

#if defined(SUPPORT_INET4)

#ifndef NUM_MPF_NET_BUF_IP_MSS
#define NUM_MPF_NET_BUF_IP_MSS	0	/* IF + 576 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_IP_MSS */

#endif	/* of #if defined(SUPPORT_INET4) */

#ifndef NUM_MPF_NET_BUF_1024
#define NUM_MPF_NET_BUF_1024	0	/* 1024 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_1024 */

#if defined(SUPPORT_INET6)

#ifndef NUM_MPF_NET_BUF_IPV6_MMTU
#define NUM_MPF_NET_BUF_IPV6_MMTU	0	/* IF + 1280	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_IPV6_MMTU */

#endif	/* of #if defined(SUPPORT_INET6) */

#ifndef NUM_MPF_NET_BUF_IF_PDU
#define NUM_MPF_NET_BUF_IF_PDU	4	/* IF 最大 PDU サイズ	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_IF_PDU */

#ifndef NUM_MPF_NET_BUF4_REASSM

#if defined(SUPPORT_INET4) && defined(IP4_CFG_FRAGMENT)
#define NUM_MPF_NET_BUF4_REASSM	2	/* IPv4 再構成バッファサイズ	*/
#else
#define NUM_MPF_NET_BUF4_REASSM	0	/* IPv4 再構成バッファサイズ	*/
#endif

#endif	/* of #ifndef NUM_MPF_NET_BUF4_REASSM */

#ifndef NUM_MPF_NET_BUF6_REASSM

#if defined(SUPPORT_INET6) && defined(IP6_CFG_FRAGMENT)
#define NUM_MPF_NET_BUF6_REASSM	2	/* IPv6 再構成バッファサイズ	*/
#else
#define NUM_MPF_NET_BUF6_REASSM	0	/* IPv6 再構成バッファサイズ	*/
#endif

#endif	/* of #ifndef NUM_MPF_NET_BUF6_REASSM */

#endif	/* of #ifdef SUPPORT_ETHER */

#ifdef SUPPORT_LOOP

#ifndef NUM_MPF_NET_BUF_CSEG
#define NUM_MPF_NET_BUF_CSEG	2	/* IF + IP + TCP	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_CSEG */

#ifndef NUM_MPF_NET_BUF_64
#define NUM_MPF_NET_BUF_64	0	/* 64 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_64 */

#ifndef NUM_MPF_NET_BUF_128
#define NUM_MPF_NET_BUF_128	0	/* 128 オクテット、2 以上	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_128 */

#ifndef NUM_MPF_NET_BUF_256
#define NUM_MPF_NET_BUF_256	0	/* 256 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_256 */

#ifndef NUM_MPF_NET_BUF_512
#define NUM_MPF_NET_BUF_512	0	/* 512 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_512 */

#ifndef NUM_MPF_NET_BUF_1024
#define NUM_MPF_NET_BUF_1024	0	/* 1024 オクテット	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_1024 */

#ifndef NUM_MPF_NET_BUF_IF_PDU
#define NUM_MPF_NET_BUF_IF_PDU	4	/* IF 最大 PDU サイズ	*/
#endif	/* of #ifndef NUM_MPF_NET_BUF_IF_PDU */

#endif	/* of #ifdef SUPPORT_PPP */

/*
 *  ネットワーク統計情報の計測
 *
 *  ネットワーク統計情報の計測を行う場合は、tinet/include/net/net.h
 *  で定義されているプロトコル識別フラグを指定する。
 */

#if 1

#ifdef SUPPORT_INET6

#ifdef SUPPORT_INET4

#define NET_COUNT_ENABLE	(0			\
				| PROTO_FLG_PPP_HDLC	\
				| PROTO_FLG_PPP_PAP	\
				| PROTO_FLG_PPP_LCP	\
				| PROTO_FLG_PPP_IPCP	\
				| PROTO_FLG_PPP		\
				| PROTO_FLG_LOOP	\
				| PROTO_FLG_ETHER_NIC	\
				| PROTO_FLG_ETHER	\
				| PROTO_FLG_IP6		\
				| PROTO_FLG_ICMP6	\
				| PROTO_FLG_ND6		\
				| PROTO_FLG_ARP		\
				| PROTO_FLG_IP4		\
				| PROTO_FLG_ICMP4	\
				| PROTO_FLG_TCP		\
				| PROTO_FLG_UDP		\
				| PROTO_FLG_NET_BUF	\
				)

#else	/* of #ifdef SUPPORT_INET4 */

#define NET_COUNT_ENABLE	(0			\
				| PROTO_FLG_PPP_HDLC	\
				| PROTO_FLG_PPP_PAP	\
				| PROTO_FLG_PPP_LCP	\
				| PROTO_FLG_PPP_IPCP	\
				| PROTO_FLG_PPP		\
				| PROTO_FLG_LOOP	\
				| PROTO_FLG_ETHER_NIC	\
				| PROTO_FLG_ETHER	\
				| PROTO_FLG_IP6		\
				| PROTO_FLG_ICMP6	\
				| PROTO_FLG_ND6		\
				| PROTO_FLG_TCP		\
				| PROTO_FLG_UDP		\
				| PROTO_FLG_NET_BUF	\
				)

#endif	/* of #ifdef SUPPORT_INET4 */

#else	/* of #ifdef SUPPORT_INET6 */

#ifdef SUPPORT_INET4

#define NET_COUNT_ENABLE	(0			\
				| PROTO_FLG_PPP_HDLC	\
				| PROTO_FLG_PPP_PAP	\
				| PROTO_FLG_PPP_LCP	\
				| PROTO_FLG_PPP_IPCP	\
				| PROTO_FLG_PPP		\
				| PROTO_FLG_LOOP	\
				| PROTO_FLG_ETHER_NIC	\
				| PROTO_FLG_ETHER	\
				| PROTO_FLG_ARP		\
				| PROTO_FLG_IP4		\
				| PROTO_FLG_ICMP4	\
				| PROTO_FLG_TCP		\
				| PROTO_FLG_UDP		\
				| PROTO_FLG_NET_BUF	\
				)

#endif	/* of #ifdef SUPPORT_INET4 */

#endif	/* of #ifdef SUPPORT_INET6 */

#else	/* of #if 0 */

#define NET_COUNT_ENABLE	(0			\
				)

#endif	/* of #if 0 */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  関数
 */

extern void mbed_bus_init (void);
extern void mbed_inter_init (void);

#endif	/* of #ifndef TOPPERS_MACRO_ONLY */

#endif /* _TINET_TARGET_CONFIG_H_ */
