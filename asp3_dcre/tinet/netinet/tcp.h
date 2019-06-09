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
 *	@(#)tcp.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/tcp.h,v 1.10.2.1 1999/08/29 16:29:52 peter Exp $
 */
 
#ifndef _TCP_H_
#define _TCP_H_

/*
 *  TCP ヘッダ、RFC 793 参照
 */

typedef struct t_tcp_hdr {
	uint16_t	sport;		/* 送信元ポート番号			*/
	uint16_t	dport;		/* 宛先ポート番号			*/

	uint32_t	seq;		/* SEQ 番号				*/

	uint32_t	ack;		/* ACK 番号				*/

	uint8_t		doff;		/* データオフセット(上位4ビット)	*/
	uint8_t		flags;		/* フラグ(下位6ビット)			*/
	uint16_t	win;		/* ウィンドサイズ			*/

	uint16_t	sum;		/* チェックサム				*/
	uint16_t	urp;		/* 緊急ポインタ				*/
	} __attribute__((packed, aligned(2)))T_TCP_HDR;

#define TCP_HDR_SIZE			(sizeof(T_TCP_HDR))

/*
 *  TCP ヘッダ構造体メンバアクセスマクロ
 */

#define TCP_MAKE_DATA_OFF(o)	((uint8_t)(((uint8_t)(o) << 2) & 0xf0))
#define TCP_DATA_OFF(o)		(((uint8_t)(o) & UINT_C(0xf0)) >> 2)
#define TCP_HDR_LEN(o)		(TCP_DATA_OFF(o))

/*
 *  TCP ヘッダのフラグフィールドの値の定義
 */

#define TCP_FLG_FIN		UINT_C(0x01)	/* FIN:	最終			*/
#define TCP_FLG_SYN		UINT_C(0x02)	/* SYN:	同期			*/
#define TCP_FLG_RST		UINT_C(0x04)	/* RST:	コネクションのリセット	*/
#define TCP_FLG_PUSH		UINT_C(0x08)	/* PUSH:プッシュ機能		*/
#define TCP_FLG_ACK		UINT_C(0x10)	/* ACK:	肯定応答		*/
#define TCP_FLG_URG		UINT_C(0x20)	/* URG:	緊急情報		*/

/*
 *  TCP オプションの定義
 */

/* オプションの型 */

#define TCP_OPT_EOL		UINT_C(0)	/* 最終				*/
#define TCP_OPT_NOP		UINT_C(1)	/* 何もしない			*/
#define TCP_OPT_MAXSEG		UINT_C(2)	/* 最大セグメント長		*/
#define TCP_OPT_WINDOW		UINT_C(3)	/* ウィンドスケーリング		*/
#define TCP_OPT_SACK_PERM	UINT_C(4)	/* 選択 ACK 許可		*/
#define TCP_OPT_SACK		UINT_C(5)	/* 選択 ACK			*/
#define TCP_OPT_TIMESTAMP	UINT_C(8)	/* タイムスタンプ		*/
#define TCP_OPT_CC		UINT_C(11)	/* 接続カウント			*/
#define TCP_OPT_CCNEW		UINT_C(12)	/* 接続カウント			*/
#define TCP_OPT_CCECHO		UINT_C(13)	/* 接続カウント			*/

/* オプションの長さ */

#define TCP_OPT_LEN_MAXSEG	4		/* 最大セグメント長		*/
#define TCP_OPT_LEN_WINDOW	3		/* ウィンドスケーリング		*/
#define TCP_OPT_LEN_SACK_PERM	4		/* 選択 ACK 許可		*/
#define TCP_OPT_LEN_SACK	5		/* 選択 ACK			*/
#define TCP_OPT_LEN_TIMESTAMP	10		/* タイムスタンプ		*/
#define TCP_OPT_LEN_CC		6		/* 接続カウント			*/
#define TCP_OPT_LEN_CCNEW	6		/* 接続カウント			*/
#define TCP_OPT_LEN_CCECHO	6		/* 接続カウント			*/

/*
 *  TCP 最大セグメントサイズ
 */

/*
 *  IPv4 の TCP 最大セグメントサイズ
 *  IP4_MSS は 576 で、標準の IPv4 ヘッダ長 (20) と
 *  TCP ヘッダ長 (20) を引いた値 536 であるが、512 を標準とする。
 */

#define TCP_MSS			UINT_C(512)

/*
 *  IPv6 の TCP 最大セグメントサイズ
 *  IPV6_MMTU は 1280 で、標準の IPv6 ヘッダ長 (40) と
 *  TCP ヘッダ長 (20) を引いた値 1220 であるが、1024 を標準とする。
 */

#define TCP6_MSS		UINT_C(1024)

/*
 *  TCP 最小セグメントサイズ
 *
 *  256（最小リンク MTU）- 40 = 216
 */

#define TCP_MINMSS		UINT_C(216)

/*
 *  TCP 最大ウィンドウサイズ
 */

#define MAX_TCP_WIN_SIZE	UINT_C(65535)

#endif	/* of #ifndef _TCP_H_ */
