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

/*	$FreeBSD: src/sys/netinet/ip6.h,v 1.2.2.2 2001/07/03 11:01:46 ume Exp $	*/
/*	$KAME: ip6.h,v 1.18 2001/03/29 05:34:30 itojun Exp $	*/

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
 *	@(#)ip.h	8.1 (Berkeley) 6/10/93
 */
 
#ifndef _IP6_H_
#define _IP6_H_

/*
 *  IPv6 の定義、RFC2460 参照
 */

#define IPV6_VERSION	6

/*
 *  IPv6 ヘッダの定義
 */

typedef struct t_ip6_hdr {
	uint32_t	vcf;		/* Version:        4 bit		*/
					/* Traffic Class:  8 bit		*/
					/* Flow Label:    20 bit		*/
	uint16_t	plen;		/* Payload Length			*/
	uint8_t		next;		/* Next Header				*/
	uint8_t		hlim;		/* Hop Limit				*/
	T_IN6_ADDR 	src;		/* Source Address			*/
	T_IN6_ADDR 	dst;		/* Destination Address			*/
	} __attribute__((packed, aligned(2)))T_IP6_HDR;

#define IP6_HDR_SIZE		(sizeof(T_IP6_HDR))

/*
 *  IPv6 ヘッダ構造体メンバアクセスマクロ
 */

#define IP6_MAKE_VCF(v,cf)	(((uint32_t)(v)<<28)|(cf))
#define IP6_VCF_V(vcf)		((uint32_t)(vcf)>>28)
#define IP6_VCF_C(vcf)		(((uint32_t)(vcf)>>20)&ULONG_C(0xff))
#define IP6_VCF_F(vcf)		((uint32_t)(vcf)&ULONG_C(0x000fffff))
#define IP6_VCF_CF(vcf)		((uint32_t)(vcf)&ULONG_C(0x0fffffff))

/*
 *  拡張ヘッダ
 */

typedef struct t_ip6_ext_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		len;		/* 8 オクテット単位の長さ	*/
	} __attribute__((packed, aligned(2)))T_IP6_EXT_HDR;

/*
 *  中継点 (Hop-by-Hop) オプションヘッダ
 */

typedef struct t_ip6_hbh_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		len;		/* 8 オクテット単位の長さ。ただし、
					 * このオプションヘッダ全体のオクテット数を
					 * 8 で割った値の - 1 が入る。
					 */
	} __attribute__((packed, aligned(2)))T_IP6_HBH_HDR;

/*
 *  終点 (Destination) オプションヘッダ
 */

typedef struct t_ip6_dest_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		len;		/* 8 オクテット単位の長さ。ただし、
					 * このオプションヘッダ全体のオクテット数を
					 * 8 で割った値の - 1 が入る。
					 */
	} __attribute__((packed, aligned(2)))T_IP6_DEST_HDR;

/* 中継点 (Hop-by-Hop) オプション */

#define IP6OPT_PAD1		UINT_C(0x00)	/* 00 0 00000 */
#define IP6OPT_PADN		UINT_C(0x01)	/* 00 0 00001 */
#define IP6OPT_JUMBO		UINT_C(0xc2)	/* 11 0 00010 */
#define IP6OPT_NSAP_ADDR	UINT_C(0xc3)	/* 11 0 00011 */
#define IP6OPT_TUNNEL_LIMIT	UINT_C(0x04)	/* 00 0 00100 */
#define IP6OPT_RTR_ALERT	UINT_C(0x05)	/* 00 0 00101 */

#define IP6OPT_MINLEN		2	/* オプションの最短長		*/

/* 未知のオプションの処理の指定 */

#define IP6OPT_TYPE_SKIP	UINT_C(0x00)	/* 無視する。			*/
#define IP6OPT_TYPE_DISCARD	UINT_C(0x40)	/* データグラムを破棄する。	*/
#define IP6OPT_TYPE_FORCEICMP	UINT_C(0x80)	/* 破棄して、ICMP を返送する。	*/
#define IP6OPT_TYPE_ICMP	UINT_C(0xc0)
		/* 破棄して、マルチキャストでなければ ICMP を返送する。	*/

#define IP6OPT_TYPE(o)		((o)&UINT_C(0xc0))

/*
 *  経路制御ヘッダ
 */

typedef struct t_ip6_rt_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		len;		/* 8 オクテット単位の長さ。ただし、
					 * このオプションヘッダ全体のオクテット数を
					 * 8 で割った値の - 1 が入る。
					 */
	uint8_t		type;		/* 経路制御タイプ		*/
	uint8_t		segleft;	/* セグメントの残り		*/
	} __attribute__((packed, aligned(2)))T_IP6_RT_HDR;

/*
 *  タイプ 0 経路制御ヘッダ
 */

typedef struct t_ip6_rt0_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		len;		/* 8 オクテット単位の長さ。ただし、
					 * このオプションヘッダ全体のオクテット数を
					 * 8 で割った値の - 1 が入る。
					 */
	uint8_t		type;		/* 経路制御タイプ		*/
	uint8_t		segleft;	/* セグメントの残り		*/
	uint32_t	reserved;	/* 予約				*/
	} __attribute__((packed, aligned(2)))T_IP6_RT0_HDR;

/*
 *  断片ヘッダ
 */

typedef struct t_ip6_frag_hdr {
	uint8_t		next;		/* Next Header			*/
	uint8_t		reserved;
	uint16_t	off_flag;	/* オフセットとフラグ		*/
	uint32_t	ident;		/* 分割識別子			*/
	} __attribute__((packed, aligned(2)))T_IP6_FRAG_HDR;

/* 断片ヘッダのオフセットとフラグ */

#ifdef _NET_CFG_BYTE_ORDER

#if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN

#define IP6F_OFF_MASK		UINT_C(0xfff8)		/* オフセットを取出すマスク	*/
#define IP6F_RESERVED_MASK	UINT_C(0x0006)		/* 予約を取出すマスク		*/
#define IP6F_MORE_FRAG		UINT_C(0x0001)		/* 中間フラグメント		*/

#elif _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN

#define IP6F_OFF_MASK		UINT_C(0xf8ff)		/* オフセットを取出すマスク	*/
#define IP6F_RESERVED_MASK	UINT_C(0x0600)		/* 予約を取出すマスク		*/
#define IP6F_MORE_FRAG		UINT_C(0x0100)		/* 中間フラグメント		*/

#endif	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

#endif	/* of #ifdef _NET_CFG_BYTE_ORDER */

/*
 *  制限値
 */

#define IPV6_MAXHLIM	UINT_C(255)	/* ホップリミットの最大値				*/
#define IPV6_DEFHLIM	UINT_C(64)	/* ホップリミットの規定値				*/
#define IPV6_FRAGTTL	UINT_C(60)	/* 分割されたデータグラムの生存時間（秒）	*/
#define IPV6_MMTU	UINT_C(1280)	/* 最小 MTU。ペイロード (1024) + ヘッダ (256)	*/
#define IPV6_MAXPACKET	UINT_C(65535)	/* ジャンボペイロードを除く最大パケットサイズ	*/

/*
 *  IPv6 と IPv4 をコンパイル時に選択するためのマクロ
 */

/*
 *  ホップリミットの規定値は、ルータ通知により変更されることがある
 */

#if NUM_ND6_DEF_RTR_ENTRY > 0

/* ルータ通知を受信する場合は、通知された値 */

#define IP6_DEFTTL	ip6_defhlim

#else	/* #if NUM_ND6_DEF_RTR_ENTRY > 0 */

/* ルータ通知を受信しない場合の値は既定値 */

#define IP6_DEFTTL	IPV6_DEFHLIM

#endif	/* #if NUM_ND6_DEF_RTR_ENTRY > 0 */

/* IPv4 と IPv6 をコンパイル時に選択するためのマクロ */

#ifdef _IP6_CFG

#define IP_HDR_SIZE	IP6_HDR_SIZE
#define T_IP_HDR	T_IP6_HDR
#define IP_DEFTTL	IP6_DEFTTL

#endif	/* of #ifdef _IP6_CFG */

#endif	/* of #ifndef _IP6_H_ */
