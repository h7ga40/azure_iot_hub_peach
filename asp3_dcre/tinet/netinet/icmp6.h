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

/*	$FreeBSD: src/sys/netinet/icmp6.h,v 1.2.2.3 2001/07/06 05:36:35 sumikawa Exp $	*/
/*	$KAME: icmp6.h,v 1.46 2001/04/27 15:09:48 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_icmp.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _IP_ICMP6_H_
#define _IP_ICMP6_H_

/*
 *  ICMPv6 ヘッダ
 */

typedef struct t_icmp6_hdr {
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		code;		/* サブ型		*/
	uint16_t	sum;		/* チェックサム		*/
	union {
		uint32_t	data32;
		uint16_t	data16[2];
		uint8_t		data8[4];
		} data;
	} __attribute__((packed, aligned(2))) T_ICMP6_HDR;

#define icmp6_data32			data.data32
#define icmp6_data16			data.data16
#define icmp6_data8			data.data8
#define icmp6_pptr			data.data32		/* パラメータ異常	*/
#define icmp6_mtu			data.data32		/* パラメータ異常	*/
#define icmp6_id			data.data16[0]		/* エコー要求と応答	*/
#define icmp6_seq			data.data16[1]		/* エコー要求と応答	*/

#define ICMP6_HDR_SIZE			(sizeof(T_ICMP6_HDR))
#define IF_IP6_ICMP6_HDR_OFFSET		(IF_IP6_HDR_SIZE)

#define GET_ICMP6_HDR(nbuf,ihoff)	((T_ICMP6_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_ICMP6_SDU(nbuf,ihoff)	((uint8_t*)((nbuf)->buf) + ihoff + ICMP6_HDR_SIZE)

#define GET_IP6_ICMP6_HDR_SIZE(nbuf)	(GET_IP6_HDR_SIZE(nbuf) + ICMP6_HDR_SIZE)
#define GET_IF_IP6_ICMP6_HDR_SIZE(nbuf)	(IF_HDR_SIZE + GET_IP6_ICMP6_HDR_SIZE(nbuf))

/*
 *  ICMP6 タイプ
 */

#define ICMP6_DST_UNREACH		UINT_C(1)	/* 宛先到達不可能		*/
#define ICMP6_PACKET_TOO_BIG		UINT_C(2)	/* パケットが大きすぎる。	*/
#define ICMP6_TIME_EXCEEDED		UINT_C(3)	/* TTL を超過した。		*/
#define ICMP6_PARAM_PROB		UINT_C(4)	/* IPv6 ヘッダが不正。		*/

#define ICMP6_ECHO_REQUEST		UINT_C(128)	/* エコー要求			*/
#define ICMP6_ECHO_REPLY		UINT_C(129)	/* エコー応答			*/

#define ICMP6_MEMBERSHIP_QUERY		UINT_C(130)	/* グループメンバ照会		*/
#define ICMP6_MEMBERSHIP_REPORT		UINT_C(131)	/* グループメンバ報告		*/
#define ICMP6_MEMBERSHIP_REDUCTION	UINT_C(132)	/* グループメンバ終了		*/

#define MLD_LISTENER_QUERY		UINT_C(130)	/* マルチキャスト受信者照会	*/
#define MLD_LISTENER_REPORT		UINT_C(131)	/* マルチキャスト受信者報告	*/
#define MLD_LISTENER_DONE		UINT_C(132)	/* マルチキャスト受信者終了	*/

#define ND_ROUTER_SOLICIT		UINT_C(133)	/* ルータ要請			*/
#define ND_ROUTER_ADVERT		UINT_C(134)	/* ルータ通知			*/
#define ND_NEIGHBOR_SOLICIT		UINT_C(135)	/* 近隣要請			*/
#define ND_NEIGHBOR_ADVERT		UINT_C(136)	/* 近隣通知			*/
#define ND_REDIRECT			UINT_C(137)	/* 向け直し			*/

#define ICMP6_ROUTER_RENUMBERING	UINT_C(138)	/* ルータ番号再設定		*/

#define ICMP6_WRU_REQUEST		UINT_C(139)	/* 要求ノード探索		*/
#define ICMP6_WRU_REPLY			UINT_C(140)	/* 応答ノード探索		*/

#define ICMP6_FQDN_QUERY		UINT_C(139)	/* FQDN 照会			*/
#define ICMP6_FQDN_REPLY		UINT_C(140)	/* FQDN 応答			*/

#define ICMP6_NI_QUERY			UINT_C(139)	/* ノード情報照会		*/
#define ICMP6_NI_REPLY			UINT_C(140)	/* ノード情報応答		*/

#define ICMP6_MAX_TYPE			UINT_C(203)	/* ICMP6 タイプの最大値		*/

/*
 *  ICMP6 コード
 */

/* 宛先到達不可能 */

#define ICMP6_DST_UNREACH_NOROUTE	UINT_C(0)	/* 到達ルートなし。		*/
#define ICMP6_DST_UNREACH_ADMIN		UINT_C(1)	/* 管理者により無効化中。	*/
#define ICMP6_DST_UNREACH_NOTEIGHBOR	UINT_C(2)	/* 近隣者ではなくなった。	*/
#define ICMP6_DST_UNREACH_BEYOND_SCOPE	UINT_C(2)	/* スコープを超えている。	*/
#define ICMP6_DST_UNREACH_ADDR		UINT_C(3)	/* 宛先アドレスに到達不可能。	*/
#define ICMP6_DST_UNREACH_NOPORT	UINT_C(4)	/* 宛先ポートに到達不可能。	*/

/* TTL 超過 */

#define ICMP6_TIME_EXCEED_TRANSIT	UINT_C(0)	/* 転送時に TTL==0 になった。	*/
#define ICMP6_TIME_EXCEED_REASSEMBLY	UINT_C(1)	/* IP 再構成でタイムアウトした。*/

/* ヘッダ不正 */

#define ICMP6_PARAMPROB_HEADER		UINT_C(0)	/* ヘッダに不正があった。	*/
#define ICMP6_PARAMPROB_NEXT_HEADER	UINT_C(1)	/* 次ヘッダが認識できない。	*/
#define ICMP6_PARAMPROB_OPTION		UINT_C(2)	/* オプションが認識できない。	*/

/* 内部使用 */
#define ND_REDIRECT_ONLINK		UINT_C(0)	/* オンリンクへの向け直し	*/
#define ND_REDIRECT_ROUTER		UINT_C(1)	/* ルータへの向け直し		*/

/*
 *  制限値
 */

							/* ICMPv6 の最大ペイロード長	*/
#define ICMPV6_PAYLOAD_MAXLEN		(IPV6_MMTU - (IP6_HDR_SIZE + ICMP6_HDR_SIZE))

/*
 *  近隣要請
 */

typedef struct t_neighbor_solicit_hdr {
	T_ICMP6_HDR	hdr;
	T_IN6_ADDR	target;
	/* この後にオプションが続く */
	} T_NEIGHBOR_SOLICIT_HDR;

#define NEIGHBOR_SOLICIT_HDR_SIZE	(sizeof(T_NEIGHBOR_SOLICIT_HDR))
#define IF_IP6_NEIGHBOR_SOLICIT_HDR_OFFSET	\
					(IF_IP6_HDR_SIZE)

#define GET_NEIGHBOR_SOLICIT_HDR(nbuf,nhoff)	\
					((T_NEIGHBOR_SOLICIT_HDR*)((uint8_t*)((nbuf)->buf) + nhoff))
#define GET_NEIGHBOR_SOLICIT_SDU(nbuf,nhoff)	\
					((uint8_t*)((nbuf)->buf) + nhoff + NEIGHBOR_SOLICIT_HDR_SIZE)

#define GET_IP6_NEIGHBOR_SOLICIT_HDR_SIZE(nbuf)		\
					(GET_IP6_HDR_SIZE(nbuf) + NEIGHBOR_SOLICIT_HDR_SIZE)
#define GET_IF_IP6_NEIGHBOR_SOLICIT_HDR_SIZE(nbuf)	\
					(GET_IP6_NEIGHBOR_SOLICIT_HDR_SIZE(nbuf) + IF_HDR_SIZE)

/*
 *  近隣通知
 */

typedef struct t_neighbor_advert_hdr {
	T_ICMP6_HDR	hdr;
	T_IN6_ADDR	target;
	/* この後にオプションが続く */
	} __attribute__((packed, aligned(2))) T_NEIGHBOR_ADVERT_HDR;

/* ICMP6 ヘッダのデータ部の定義 */

#define nd_na_flags_reserved		hdr.data.data32

#define NEIGHBOR_ADVERT_HDR_SIZE	(sizeof(T_NEIGHBOR_ADVERT_HDR))
#define IP6_NEIGHBOR_ADVERT_HDR_SIZE	(IP6_HDR_SIZE + NEIGHBOR_ADVERT_HDR_SIZE)
#define IF_IP6_NEIGHBOR_ADVERT_HDR_SIZE	\
					(IF_IP6_HDR_SIZE + NEIGHBOR_ADVERT_HDR_SIZE)
#define IF_IP6_NEIGHBOR_ADVERT_HDR_OFFSET	\
					(IF_IP6_HDR_SIZE)

#define GET_NEIGHBOR_ADVERT_HDR(nbuf,nhoff)	\
					((T_NEIGHBOR_ADVERT_HDR*)((uint8_t*)((nbuf)->buf) + nhoff))
#define GET_NEIGHBOR_ADVERT_SDU(nbuf,nhoff)	\
					((uint8_t*)((nbuf)->buf) + nhoff + NEIGHBOR_ADVERT_HDR_SIZE)

#define GET_IP6_NEIGHBOR_ADVERT_HDR_SIZE(nbuf)		\
					(GET_IP6_HDR_SIZE(nbuf) + NEIGHBOR_ADVERT_HDR_SIZE)
#define GET_IF_IP6_NEIGHBOR_ADVERT_HDR_SIZE(nbuf)	\
					(GET_IP6_NEIGHBOR_ADVERT_HDR_SIZE(nbuf) + IF_HDR_SIZE)

/*
 *  近隣通知フラグ、nd_na_flags_reserved (hdr.data32) に設定する。
 */

#if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN

#define ND_NA_FLG_OVERRIDE		ULONG_C(0x20000000)	/* 近隣キャッシュ上書き	*/
#define ND_NA_FLG_SOLICITED		ULONG_C(0x40000000)	/* 近隣要請への応答	*/
#define ND_NA_FLG_ROUTER		ULONG_C(0x80000000)	/* ルータ通知		*/

#elif _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN

#define ND_NA_FLG_OVERRIDE		ULONG_C(0x00000020)	/* 近隣キャッシュ上書き	*/
#define ND_NA_FLG_SOLICITED		ULONG_C(0x00000040)	/* 近隣要請への応答	*/
#define ND_NA_FLG_ROUTER		ULONG_C(0x00000080)	/* ルータ通知		*/

#endif	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

/*
 *  ルータ通知
 *
 *    RFC4291: IP Version 6 Addressing Architecture
 *    RFC4941: Privacy Extensions for Stateless Address Autoconfiguration in IPv6
 *    RFC6106: IPv6 Router Advertisement Options for DNS Configuration
 */

typedef struct t_router_advert_hdr {
	T_ICMP6_HDR	hdr;
	uint32_t	reachable;		/* [us] 到達可能時間	*/
	uint32_t	retransmit;		/* [us] 近隣要請送信間隔	*/
	/* この後にオプションが続く */
	} __attribute__((packed, aligned(2))) T_ROUTER_ADVERT_HDR;

/* ICMP6 ヘッダのデータ部の定義 */

#define nd_ra_current_hoplimit		hdr.data.data8[0]
#define nd_ra_flags			hdr.data.data8[1]
#define nd_ra_router_lifetime		hdr.data.data16[1]	/* [s] ルータ有効時間	*/

#define ROUTER_ADVERT_HDR_SIZE		(sizeof(T_ROUTER_ADVERT_HDR))
#define IP6_ROUTER_ADVERT_HDR_SIZE	(IP6_HDR_SIZE + ROUTER_ADVERT_HDR_SIZE)
#define IF_IP6_ROUTER_ADVERT_HDR_SIZE	(IF_IP6_HDR_SIZE + ROUTER_ADVERT_HDR_SIZE)
#define IF_IP6_ROUTER_ADVERT_HDR_OFFSET	(IF_IP6_HDR_SIZE)

#define GET_ROUTER_ADVERT_HDR(nbuf,nhoff)	\
					((T_ROUTER_ADVERT_HDR*)((uint8_t*)((nbuf)->buf) + nhoff))
#define GET_ROUTER_ADVERT_SDU(nbuf,nhoff)	\
					((uint8_t*)((nbuf)->buf) + nhoff + ROUTER_ADVERT_HDR_SIZE)

#define GET_IP6_ROUTER_ADVERT_HDR_SIZE(nbuf)	\
					(GET_IP6_HDR_SIZE(nbuf) + ROUTER_ADVERT_HDR_SIZE)
#define GET_IF_IP6_ROUTER_ADVERT_HDR_SIZE(nbuf)	\
					(GET_IP6_ROUTER_ADVERT_HDR_SIZE(nbuf) + IF_HDR_SIZE)

/*
 *  ルータ通知ヘッダのフラグ
 */

#define ND_RA_FLAG_MANAGED		UINT_C(0x80)
#define ND_RA_FLAG_OTHER		UINT_C(0x40)
#define ND_RA_FLAG_HA			UINT_C(0x20)

/*
 *  M: RA以外の方法で自動的にアドレスを設定することを許可する。
 *  O: RA以外の方法で自動的にアドレス以外の情報を設定することをを許可する。
 *
 *   M   O
 *  ON  ON	アドレスとそれ以外の情報をDHCPv6で設定する（statefull）
 *  ON  OFF	アドレスはDHCPv6で設定するが、それ以外は手動で設定する。
 *  OFF ON	アドレスは RA で設定するが、それ以外の情報をDHCPv6で設定する（stateless）
 *  OFF OFF	DHCPv6 は使用しない。
 */

/*
 *  ルータ要請
 */

typedef struct t_router_solicit_hdr {
	T_ICMP6_HDR	hdr;
	} __attribute__((packed, aligned(2))) T_ROUTER_SOLICIT_HDR;

/* ICMP6 ヘッダのデータ部の定義 */

#define nd_rs_reserved			hdr.data.data32

#define ROUTER_SOLICIT_HDR_SIZE		(sizeof(T_ROUTER_SOLICIT_HDR))
#define IP6_ROUTER_SOLICIT_HDR_SIZE	(IP6_HDR_SIZE + ROUTER_SOLICIT_HDR_SIZE)
#define IF_IP6_ROUTER_SOLICIT_HDR_SIZE	(IF_IP6_HDR_SIZE + ROUTER_SOLICIT_HDR_SIZE)
#define IF_IP6_ROUTER_SOLICIT_HDR_OFFSET	\
					(IF_IP6_HDR_SIZE)

#define GET_ROUTER_SOLICIT_HDR(nbuf,nhoff)	\
					((T_ROUTER_SOLICIT_HDR*)((uint8_t*)((nbuf)->buf) + nhoff))
#define GET_ROUTER_SOLICIT_SDU(nbuf,nhoff)	\
					((uint8_t*)((nbuf)->buf) + nhoff + ROUTER_SOLICIT_HDR_SIZE)

#define GET_IP6_ROUTER_SOLICIT_HDR_SIZE(nbuf)		\
					(GET_IP6_HDR_SIZE(nbuf) + ROUTER_SOLICIT_HDR_SIZE)
#define GET_IF_IP6_ROUTER_SOLICIT_HDR_SIZE(nbuf)	\
					(GET_IP6_ROUTER_SOLICIT_HDR_SIZE(nbuf) + IF_HDR_SIZE)

/*
 *  近隣探索オプション
 */

/* 近隣探索ヘッダ */

typedef struct t_neighbor_hdr {
	T_ICMP6_HDR	hdr;
	T_IN6_ADDR	target;
	} __attribute__((packed, aligned(2))) T_NEIGHBOR_HDR;

/* 近隣探索オプションヘッダ */

typedef struct t_nd_opt_hdr {
	uint8_t		type;			/* オプションタイプ			*/
	uint8_t		len;			/* オプション長、単位は 8 バイト	*/
	} __attribute__((packed, aligned(2))) T_ND_OPT_HDR;

#define ND_OPT_HDR_SIZE			(sizeof(T_ND_OPT_HDR))

#define ND_OPT_SOURCE_LINKADDR		UINT_C(1)	/* 送信先ネットワークインタフェースのアドレス	*/
#define ND_OPT_TARGET_LINKADDR		UINT_C(2)	/* 対象ネットワークインタフェースのアドレス	*/
#define ND_OPT_PREFIX_INFORMATION	UINT_C(3)	/* プレフィックス情報				*/
#define ND_OPT_REDIRECTED_HEADER	UINT_C(4)	/* リダイレクト・ヘッダ				*/
#define ND_OPT_MTU			UINT_C(5)	/* MTU						*/

#define ND_OPT_PREFIX_INFO_COUNT	UINT_C(6)	/* プレフィックス情報の個数			*/
#define ND_OPT_OFF_ARRAY_SIZE		UINT_C(7)	/* ND オプションのオフセットを記録する配列サイズ*/

#define ND_OPT_OFF_ARRAY_IX(t)		((t)-1)		/* オフセット配列のインデックス			*/

/* プレフィックスオプション */

typedef struct t_nd_opt_prefix_info {
	uint8_t		type;		/* 近隣探索オプションタイプ = 3	*/
	uint8_t		len;		/* オプション長 = 8×4=32	*/
	uint8_t		prefix_len;	/* プレフィックス長		*/
	uint8_t		flags;		/* フラグ			*/
	uint32_t	valid;		/* [s] 有効時間			*/
	uint32_t	preferred;	/* [s] 推奨有効時間		*/
	uint32_t	reserved;
	T_IN6_ADDR	prefix;		/* プレフィックス		*/
	} __attribute__((packed, aligned(2))) T_ND_OPT_PREFIX_INFO;

#define ND_OPT_PI_FLAG_ONLINK	UINT_C(0x80)		/* オンリンクプレフィックス	*/
#define ND_OPT_PI_FLAG_AUTO	UINT_C(0x40)		/* ステートレス・アドレス自動設定*/

/* MTU オプションヘッダ */

typedef struct t_nd_opt_mtu_hdr {
	uint8_t		type;		/* オプションタイプ		*/
	uint8_t		len;		/* オプション長、単位は 8 バイト*/
	uint16_t	reserved;
	uint32_t	mtu;		/* MTU				*/
	} __attribute__((packed, aligned(2))) T_ND_OPT_MTU_HDR;

/*
 *  向け直し
 */

typedef struct t_nd_redirect_hdr {
	T_ICMP6_HDR	hdr;
	T_IN6_ADDR	target;		/* ルータのアドレス		*/
	T_IN6_ADDR	dst;		/* 向け直しの終点アドレス		*/
	/* この後にオプションが続く */
	} __attribute__((packed, aligned(2))) T_ND_REDIRECT_HDR;

#define ND_REDIRECT_HDR_SIZE		(sizeof(T_ND_REDIRECT_HDR))

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

typedef struct t_icmp6_ifstat {
	uint32_t	ipv6IfIcmpInMsgs;
	uint32_t	ipv6IfIcmpInErrors;
	uint32_t	ipv6IfIcmpInDestUnreachs;
/*	uint32_t	ipv6IfIcmpInAdminProhibs;*/
	uint32_t	ipv6IfIcmpInTimeExcds;
	uint32_t	ipv6IfIcmpInParmProblems;
	uint32_t	ipv6IfIcmpInPktTooBigs;
	uint32_t	ipv6IfIcmpInEchos;
	uint32_t	ipv6IfIcmpInEchoReplies;
/*	uint32_t	ipv6IfIcmpInRouterSolicits;*/
	uint32_t	ipv6IfIcmpInRouterAdvertisements;
	uint32_t	ipv6IfIcmpInNeighborSolicits;
	uint32_t	ipv6IfIcmpInNeighborAdvertisements;
	uint32_t	ipv6IfIcmpInRedirects;
/*	uint32_t	ipv6IfIcmpInGroupMembQueries;*/
/*	uint32_t	ipv6IfIcmpInGroupMembResponses;*/
/*	uint32_t	ipv6IfIcmpInGroupMembReductions;*/
	uint32_t	ipv6IfIcmpOutMsgs;
	uint32_t	ipv6IfIcmpOutErrors;
	uint32_t	ipv6IfIcmpOutDestUnreachs;
/*	uint32_t	ipv6IfIcmpOutAdminProhibs;*/
/*	uint32_t	ipv6IfIcmpOutTimeExcds;*/
	uint32_t	ipv6IfIcmpOutParmProblems;
	uint32_t	ipv6IfIcmpOutPktTooBigs;
	uint32_t	ipv6IfIcmpOutEchos;
	uint32_t	ipv6IfIcmpOutEchoReplies;
	uint32_t	ipv6IfIcmpOutRouterSolicits;
/*	uint32_t	ipv6IfIcmpOutRouterAdvertisements;*/
	uint32_t	ipv6IfIcmpOutNeighborSolicits;
	uint32_t	ipv6IfIcmpOutNeighborAdvertisements;
/*	uint32_t	ipv6IfIcmpOutRedirects;*/
/*	uint32_t	ipv6IfIcmpOutGroupMembQueries;*/
/*	uint32_t	ipv6IfIcmpOutGroupMembResponses;*/
/*	uint32_t	ipv6IfIcmpOutGroupMembReductions;*/
} T_ICMP6_IFSTAT;

extern T_ICMP6_IFSTAT icmp6_ifstat;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  関数
 */

extern uint_t icmp6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern void icmp6_error (T_NET_BUF *input, uint8_t type, uint8_t code, uint32_t param);

#ifdef ICMP_CFG_CALLBACK_ECHO_REPLY

extern void icmp6_echo_reply (T_NET_BUF *input, uint_t ihoff);

#endif	/* of #ifdef ICMP_CFG_CALLBACK_ECHO_REPLY */

#endif	/* of #ifndef _IP_ICMP6_H_ */
