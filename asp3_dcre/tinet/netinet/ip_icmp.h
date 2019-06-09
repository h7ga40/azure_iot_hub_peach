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
 *	@(#)ip_icmp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/ip_icmp.h,v 1.12.2.2 1999/11/18 08:51:44 peter Exp $
 */
 
#ifndef _IP_ICMP_H_
#define _IP_ICMP_H_

#include <netinet/icmp6.h>

/*
 *  ICMP ヘッダの定義、RFC 792
 */

typedef struct t_icmp_hdr {
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		code;		/* サブ型		*/
	uint16_t	sum;		/* チェックサム		*/
	union {
		struct id_seq {
			uint16_t	id;
			uint16_t	seq;
			} id_seq;
		T_IN4_ADDR	addr;
		} data;
	} __attribute__((packed, aligned(2)))T_ICMP4_HDR;

#define ICMP4_HDR_SIZE			(sizeof(T_ICMP4_HDR))
#define IP4_ICMP4_HDR_SIZE		(IP4_HDR_SIZE + ICMP4_HDR_SIZE)
#define IF_IP4_ICMP4_HDR_SIZE		(IF_IP4_HDR_SIZE + ICMP4_HDR_SIZE)
#define IF_IP4_ICMP4_HDR_OFFSET		(IF_IP4_HDR_SIZE)

#define GET_ICMP4_HDR(nbuf,ihoff)	((T_ICMP4_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_ICMP4_SDU(nbuf,ihoff)	((uint8_t*)((nbuf)->buf) + ihoff + ICMP4_HDR_SIZE)

#define GET_ICMP4_HDR_OFFSET(nbuf)	(GET_IF_IP4_HDR_SIZE(nbuf))

#define GET_IP4_ICMP4_HDR_SIZE(nbuf)	(GET_IP4_HDR_SIZE(nbuf) + ICMP4_HDR_SIZE)
#define GET_IF_IP4_ICMP4_HDR_SIZE(nbuf)	(IF_HDR_SIZE + GET_IP4_ICMP4_HDR_SIZE(nbuf))

/*
 *  メッセージの型
 */

#define ICMP4_ECHO_REPLY	UINT_C(0)	/* エコー応答		*/
#define ICMP4_UNREACH		UINT_C(3)	/* エラー		*/
#define ICMP4_SOURCE_QUENCH	UINT_C(4)	/* 発信制限		*/
#define ICMP4_REDIRECT		UINT_C(5)	/* 方向変更		*/
#define ICMP4_ECHO_REQUEST	UINT_C(8)	/* エコー要求		*/
#define ICMP4_TIME_EXCEEDED	UINT_C(11)	/* 発信制限		*/
#define ICMP4_PARAMETER		UINT_C(12)	/* パラメータ異常	*/
#define ICMP4_TIME_REQUEST	UINT_C(13)	/* タイムスタンプ要求	*/
#define ICMP4_TIME_REPLY	UINT_C(14)	/* タイムスタンプ応答	*/
#define ICMP4_MASK_REQUEST	UINT_C(17)	/* アドレスマスク要求	*/
#define ICMP4_MASK_REPLY	UINT_C(18)	/* アドレスマスク応答	*/

/*
 *  サブ型
 */

/* ICMP4_UNREACH (3)	*/

#define ICMP4_UNREACH_NET		UINT_C(0)	/* ネットワーク		*/
#define ICMP4_UNREACH_HOST		UINT_C(1)	/* ホスト		*/
#define ICMP4_UNREACH_PROTOCOL		UINT_C(2)	/* プロトコル		*/
#define ICMP4_UNREACH_PORT		UINT_C(3)	/* ポート		*/
#define ICMP4_UNREACH_FLAG		UINT_C(4)	/* 分割			*/
#define ICMP4_UNREACH_SRC_FAIL		UINT_C(5)	/* ソース経路		*/
#define ICMP4_UNREACH_NET_UNKNOWN	UINT_C(6)	/* ネットワーク不明	*/
#define ICMP4_UNREACH_HOST_UNKNOWN	UINT_C(7)	/* ホスト不明		*/
#define ICMP4_UNREACH_ISOLATED		UINT_C(8)	/* 分離			*/
#define ICMP4_UNREACH_NET_PROHIB	UINT_C(9)	/* 			*/
#define ICMP4_UNREACH_HOST_PROHIB	UINT_C(10)	/* 			*/
#define ICMP4_UNREACH_TOS_NET		UINT_C(11)	/* ネットワークへの不正TOS*/
#define ICMP4_UNREACH_TOS_HOST		UINT_C(12)	/* ホストへの不正TOS	*/
#define ICMP4_UNREACH_FILTER_PROHIB	UINT_C(13)	/* 			*/
#define ICMP4_UNREACH_HOST_PRECEDENCE	UINT_C(14)	/* 			*/
#define ICMP4_UNREACH_PRECEDENCE_CUTOFF	UINT_C(15)	/* 			*/

/* ICMP4_REDIRECT (5)	*/

#define ICMP4_REDIRECT_NET		UINT_C(0)	/* ネットワーク		*/
#define ICMP4_REDIRECT_HOST		UINT_C(1)	/* ホスト		*/
#define ICMP4_REDIRECT_PORT_AND_NET	UINT_C(2)	/* ポート番号とネット	*/
#define ICMP4_REDIRECT_PORT_AND_HOST	UINT_C(3)	/* ポート番号とホスト	*/

/*
 *  全域変数
 */

/*
 *  関数シミュレーションマクロ
 */

#ifndef ICMP_REPLY_ERROR

#define _tinet_icmp_error(code,data)

#endif	/* of #ifdef ICMP_REPLY_ERROR */

/*
 *  関数
 */

extern uint_t icmp_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);

#ifdef ICMP_REPLY_ERROR

extern void icmp_error (uint8_t code, T_NET_BUF *data);

#endif	/* of #ifdef ICMP_REPLY_ERROR */

/*
 *  TINET 独自 API
 */

#ifdef ICMP_CFG_CALLBACK_ECHO_REPLY

extern void icmp_echo_reply (T_NET_BUF *input, uint_t ihoff);

#endif	/* of #ifdef ICMP_CFG_CALLBACK_ECHO_REPLY */

#endif	/* of #ifndef _IP_ICMP_H_ */
