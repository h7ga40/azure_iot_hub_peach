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
 *	@(#)ip.h	8.2 (Berkeley) 6/1/94
 * $FreeBSD: src/sys/netinet/ip.h,v 1.15.2.1 1999/08/29 16:29:38 peter Exp $
 */
 
#ifndef _IP4_H_
#define _IP4_H_

#include <netinet/ip6.h>

/*
 *  IPv4 の定義、RFC791 参照
 */

#define IPV4_VERSION		4

/*
 *  IPv4 ヘッダの定義
 */

typedef struct t_ip4_hdr {
	uint8_t		vhl;		/* version << 4 | header length >> 2	*/
					/* length は 4 octets 単位		*/
	uint8_t		type;		/* type of services			*/
	uint16_t	len;		/* total length				*/

	uint16_t	id;		/* identification			*/
	uint16_t	flg_off;	/* fragment flag & offset		*/

	uint8_t		ttl;		/* time to live				*/
	uint8_t		proto;		/* protocol				*/
	uint16_t	sum;		/* checksum				*/

	T_IN4_ADDR	src;		/* source address			*/
	T_IN4_ADDR	dst;		/* destination address			*/
	} __attribute__((packed, aligned(2)))T_IP4_HDR;

#define IP4_HDR_SIZE		(sizeof(T_IP4_HDR))

/*
 *  IPv4 ヘッダ構造体メンバアクセスマクロ
 */

#define IP4_MAKE_VHL(v,hl)	((uint8_t)(((uint8_t)(v)<<4)|(hl)))
#define IP4_VHL_V(vhl)		((uint8_t)((uint8_t)(vhl)>>4))
#define IP4_VHL_HL(vhl)		((uint8_t)((vhl)&0x0f))

#define IP4_MAKE_FLGOFF(f,o)	((uint16_t)((f)|(o)))
#define IP4_FLGOFF_FLG(f)	((uint16_t)((f)&~IP4_OFFMASK))
#define IP4_FLGOFF_OFF(o)	((uint16_t)((o)&IP4_OFFMASK))

/*
 *  IPv4 ヘッダのフィールドの値の定義
 */

#define IP4_VHL_BORING	UINT_C(0x45)	/* オプションがない VHL 値	*/
#define IP4_RF		UINT_C(0x8000)	/* off: 予約			*/
#define IP4_DF		UINT_C(0x4000)	/* off: 分割禁止			*/
#define IP4_MF		UINT_C(0x2000)	/* off: 分割あり			*/
#define IP4_OFFMASK	UINT_C(0x1fff)	/* オフセット部のマスク値		*/

/*
 *  制限値
 */

#define IP4_MAXPACKET	UINT_C(65535)	/* 最大パケットサイズ		*/
#define IP4_MAXTTL	UINT_C(255)	/* TTL 最大値			*/
#define IP4_DEFTTL	UINT_C(64)	/* TTL 規定値 (RFC1340)		*/
#define IP4_FRAGTTL	UINT_C(60)	/* 分割されたデータグラムの生存時間（秒）*/
#define IP4_MSS		UINT_C(576)	/* 最大セグメントサイズの規定値	*/

/* IPv4 と IPv6 をコンパイル時に選択するためのマクロ */

#if !defined(_IP6_CFG) && defined(_IP4_CFG)

#define IP_HDR_SIZE	IP4_HDR_SIZE
#define T_IP_HDR	T_IP4_HDR
#define IP_DEFTTL	IP4_DEFTTL

#endif	/* of #if !defined(_IP6_CFG) && defined(_IP4_CFG) */

#endif	/* of #ifndef _IP4_H_ */
