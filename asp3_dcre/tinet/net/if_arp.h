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
 * Copyright (c) 1986, 1993
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
 *	@(#)if_arp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/net/if_arp.h,v 1.10.2.3 1999/11/19 09:30:31 julian Exp $
 */

#ifndef _IF_ARP_H_
#define _IF_ARP_H_

#ifdef SUPPORT_ETHER

/*
 *  Address Resolution Protocol.
 *
 *  RFC 826 参照
 */

/*
 *  構造体
 */

/* ARP ヘッダ */

typedef struct t_arp_hdr {
	uint16_t	hrd_addr;	/* 物理アドレスフォーマット	*/
	uint16_t	proto;		/* プロトコル			*/
	uint8_t		hdr_len;	/* 物理アドレス長		*/
	uint8_t		proto_len;	/* プロトコル長			*/
	uint16_t	opcode;		/* 命令コード			*/
	} __attribute__((packed, aligned(2))) T_ARP_HDR;

#define ARP_HDR_SIZE			(sizeof(T_ARP_HDR))
#define IF_ARP_HDR_SIZE			(IF_HDR_SIZE + ARP_HDR_SIZE)

#define GET_ARP_HDR(nbuf)		((T_ARP_HDR*)((nbuf)->buf + IF_HDR_SIZE))

#define GET_IF_ARP_HDR_SIZE(nbuf)	(IF_HDR_SIZE + ARP_HDR_SIZE)

/* hrd_addr、物理アドレスフォーマットの値 */

#define ARPHRD_ETHER	1	/* Ethernet	*/

/* opcode, 命令コードの値 */

#define ARPOP_REQUEST		UINT_C(1)	/* アドレス解決要求		*/
#define ARPOP_REPLY		UINT_C(2)	/* アドレス解決応答		*/
#define ARPOP_REV_REQUEST	UINT_C(3)	/* プロトコル・アドレス解決要求	*/
#define ARPOP_REV_REPLY		UINT_C(4)	/* プロトコル・アドレス解決要求	*/
#define ARPOP_INV_REQUEST	UINT_C(8)	/* 終端識別要求			*/
#define ARPOP_INV_REPLY		UINT_C(9)	/* 終端識別要求			*/

/*
 *  イーサネットドライバモジュールで使用される ARP 情報
 */

typedef struct arpcom {
	uint8_t		mac_addr[ETHER_ADDR_LEN];
	} __attribute__((packed, aligned(2))) T_ARPCOM;

/*
 *  ARP キャッシュ更新タイマの定義
 */

#define ARP_TIMER_TMO		(60*NET_TIMER_HZ)	/* タイムアウト、60 [s]	*/

/*
 *  変数
 */

extern const uint8_t	ether_broad_cast_addr[ETHER_ADDR_LEN];

#endif	/* of #ifdef SUPPORT_ETHER */

#endif	/* of #ifndef _IF_ARP_H_ */
