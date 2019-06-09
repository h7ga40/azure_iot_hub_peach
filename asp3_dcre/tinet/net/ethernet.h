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
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Fundamental constants relating to ethernet.
 *
 * $FreeBSD: src/sys/net/ethernet.h,v 1.9.2.1 1999/08/29 16:28:13 peter Exp $
 *
 */

#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#ifdef SUPPORT_ETHER

/*
 *  フィールド長
 */

#define ETHER_ADDR_LEN		6	/* Ethernet (MAC) Address	*/
#define ETHER_TYPE_LEN		2	/* Type		*/
#define ETHER_CRC_LEN		0/*4*/	/* CRC		*/

/*
 *  フレーム長 (Ethernet ヘッダと CRC を含む)
 */

#define ETHER_MIN_LEN		64
#define ETHER_MAX_LEN		(IF_MTU + sizeof(T_ETHER_HDR) + ETHER_CRC_LEN)

/*
 *  Ethernet ヘッダ
 */

#ifndef IF_ETHER_NIC_HDR_ALIGN
#error IF_ETHER_NIC_HDR_ALIGN expected.
#endif	/* of #ifndef IF_ETHER_NIC_HDR_ALIGN */

#if defined(__RX)
#pragma pack
#elif defined(_MSC_VER)
#pragma pack(push, 1)
#endif

#if defined(__RX) || defined(_MSC_VER)

typedef struct t_ether_header {

#if IF_ETHER_NIC_HDR_ALIGN != 0

	uint8_t		align[IF_ETHER_NIC_HDR_ALIGN];	/* アライン調整 */

#endif	/* of #if IF_ETHER_NIC_HDR_ALIGN != 0 */

	uint8_t		dhost[ETHER_ADDR_LEN];
	uint8_t		shost[ETHER_ADDR_LEN];
	uint16_t	type;
	} T_ETHER_HDR;

#elif defined(TOPPERS_S810_CLG3_85)	/* of #if defined(__RX) */

typedef struct t_ether_header {

#if IF_ETHER_NIC_HDR_ALIGN != 0

	uint8_t		align[IF_ETHER_NIC_HDR_ALIGN];	/* アライン調整 */

#endif	/* of #if IF_ETHER_NIC_HDR_ALIGN != 0 */

	uint8_t		dhost[ETHER_ADDR_LEN];
	uint8_t		shost[ETHER_ADDR_LEN];
	uint16_t	type;
	} T_ETHER_HDR;

#else	/* of #if defined(__RX) */

typedef struct t_ether_header {

#if IF_ETHER_NIC_HDR_ALIGN != 0

	uint8_t		align[IF_ETHER_NIC_HDR_ALIGN];	/* アライン調整 */

#endif	/* of #if IF_ETHER_NIC_HDR_ALIGN != 0 */

	uint8_t		dhost[ETHER_ADDR_LEN];
	uint8_t		shost[ETHER_ADDR_LEN];
	uint16_t	type;
	} __attribute__((packed, aligned(2))) T_ETHER_HDR;

#endif	/* of #if defined(__RX) */

#define GET_ETHER_HDR(nbuf)		((T_ETHER_HDR*)((nbuf)->buf))

/*
 *  MAC アドレスの定義
 */

#define ETHER_MCAST_ADDR	UINT_C(0x01)	/* マルチキャスト・アドレス	*/
#define ETHER_LOCAL_ADDR	UINT_C(0x02)	/* ローカル・アドレス		*/

typedef struct t_ether_addr {
	uint8_t		lladdr[ETHER_ADDR_LEN];
	} __attribute__((packed, aligned(2))) T_ETHER_ADDR;

#if defined(__RX)
#pragma packoption
#elif defined(_MSC_VER)
#pragma pack(pop)
#endif

/*
 *  Type フィールド
 */

#define ETHER_TYPE_IP		UINT_C(0x0800)	/* IPv4	*/
#define ETHER_TYPE_ARP		UINT_C(0x0806)	/* ARP	*/
#define ETHER_TYPE_IPV6		UINT_C(0x86dd)	/* IPv6	*/

/*
 *  インタフェースの選択マクロ
 */

#define T_IF_HDR		T_ETHER_HDR		/* インタフェースのヘッダ				*/
#define T_IF_ADDR		T_ETHER_ADDR		/* インタフェースのアドレス			*/
#define IF_HDR_ALIGN		2			/* ヘッダのアライン単位				*/
#define IF_MTU			1500			/* インタフェースの MTU				*/
#define IF_MIN_LEN		ETHER_MIN_LEN		/* インターフェースフレームの最短長		*/

#define IF_OUTPUT(o,d,g,t)	ether_output(o,d,g,t)	/* インタフェースの出力関数			*/
#define IF_RAW_OUTPUT(o,t)	ether_raw_output(o,t)	/* インタフェースの出力関数、アドレス解決無し	*/
#define IF_SET_PROTO(b,p)	(GET_ETHER_HDR(b)->type = htons(p))
							/* インタフェースのプロトコル設定関数		*/
#define IF_SOFTC_TO_IFADDR(s)	((T_IF_ADDR*)(s)->ifaddr.lladdr)
							/* ソフトウェア情報から MAC アドレスを取り出す	*/
#define IF_GET_IFNET()		ether_get_ifnet()	/* ネットワークインタフェース構造体を返す。		*/
#define IF_TYPE			IFT_ETHER		/* インタフェースの型				*/
#define IF_SRAND()		ether_srand()	/* インタフェースの乱数初期値			*/

/* IPv4 関係 */

#define IF_PROTO_IP		ETHER_TYPE_IP		/* インタフェースの IPv4 プロトコル指定		*/
#define IF_PROTO_ARP		ETHER_TYPE_ARP		/* インタフェースの ARP プロトコル指定		*/

/* IPv6 関係 */

#define MAX_IF_MADDR_CNT	2			/* インタフェースのマルチキャストアドレス配列の最大サイズ	*/
#define IF_MADDR_INIT		{ { { 0, 0, 0, 0, 0, 0 } }, { { 0, 0, 0, 0, 0, 0 } } }
							/* インタフェースのマルチキャストアドレス配列の初期化	*/
#define IF_PROTO_IPV6		ETHER_TYPE_IPV6		/* インタフェースの IPv6 プロトコル指定			*/
#define IF_ADDMULTI(s)		IF_ETHER_NIC_ADDMULTI(s)
							/* マルチキャストアドレスの登録				*/
#define IF_IN6_NEED_CACHE(i)	(true)			/* 近隣探索キャッシュを使用する。				*/
#define IF_IN6_IFID(i,a)	IF_ETHER_NIC_IN6_IFID(i,a)
							/* インタフェース識別子の設定				*/
#define IF_IN6_RESOLVE_MULTICAST(i,m)	\
				ether_in6_resolve_multicast(i,m)
							/* インタフェースのマルチキャストアドレスへの変換		*/

#define IF_FLAG_UP				0x01U
#define IF_FLAG_LINK_UP			0x10U

/*
 *  前方参照
 */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

#ifndef T_IF_SOFTC_DEFINED

typedef struct t_if_softc T_IF_SOFTC;

#define T_IF_SOFTC_DEFINED

#endif	/* of #ifndef T_IF_SOFTC_DEFINED */

#ifndef T_IN4_ADDR_DEFINED

typedef uint32_t T_IN4_ADDR;

#define T_IN4_ADDR_DEFINED

#endif	/* of #ifndef T_IN4_ADDR_DEFINED */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

/*
 *  ネットワークインタフェースに依存しないソフトウェア情報
 */

#ifdef T_IF_ETHER_NIC_SOFTC

struct t_if_softc {
	T_IF_ADDR		ifaddr;			/* ネットワークインタフェースのアドレス	*/
	uint16_t		timer;			/* 送信タイムアウト			*/
	T_IF_ETHER_NIC_SOFTC	*sc;			/* ディバイス依存のソフトウェア情報	*/
	ID			semid_txb_ready;	/* 送信セマフォ				*/
	ID			semid_rxb_ready;	/* 受信セマフォ				*/

#ifdef _IP6_CFG

	T_IF_ADDR 	maddrs[MAX_IF_MADDR_CNT];	/* マルチキャストアドレスリスト	*/

#endif	/* of #ifdef _IP6_CFG */
	};

#endif	/* of #ifdef T_IF_ETHER_NIC_SOFTC */

/*
 *  変数
 */

#ifdef ETHER_CFG_COLLECT_ADDR

T_ETHER_ADDR ether_collect_addr;

#endif	/* of #ifdef ETHER_CFG_COLLECT_ADDR */

/*
 *  関数
 */

#ifdef _IP6_CFG

/* 前方参照 */

#ifndef T_IN6_ADDR_DEFINED

typedef struct t_in6_addr T_IN6_ADDR;

#define T_IN6_ADDR_DEFINED

#endif	/* of #ifndef T_IN6_ADDR_DEFINED */

extern ER ether_in6_resolve_multicast(T_ETHER_ADDR *ifaddr, const T_IN6_ADDR *maddr);

#endif	/* of #ifdef _IP6_CFG */

extern T_IFNET *ether_get_ifnet (void);
extern ER ether_output (T_NET_BUF *data, const void *dst, T_IF_ADDR *gw, TMO tmout);
extern ER ether_raw_output (T_NET_BUF *data, TMO tmout);
extern uint32_t ether_srand (void);

typedef void (*ether_status_callback_fn)(T_IFNET *ether);
extern void ether_set_link_callback(ether_status_callback_fn link_callback);
extern void ether_set_link_up();
extern void ether_set_link_down();
extern void ether_set_up();
extern void ether_set_down();

#endif	/* of #ifdef SUPPORT_ETHER */

#endif	/* of #ifndef _ETHERNET_H_ */
