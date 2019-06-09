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
 * Copyright (c) 1989 Carnegie Mellon University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Carnegie Mellon University.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *	    Written by Toshiharu OHNO (tony-o@iij.ad.jp)
 *
 *   Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Internet Initiative Japan.  The name of the
 * IIJ may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _PPP_H_
#define _PPP_H_

/*
 *  PPP ヘッダのプロトコルフィールドの値
 */

#define PPP_IP		UINT_C(0x0021)		/* IPv4		*/
#define PPP_IPV6	UINT_C(0x0057)		/* IPv6		*/

#define PPP_IPCP	UINT_C(0x8021)		/* IPCP		*/
#define PPP_IPV6CP	UINT_C(0x8057)		/* IPV6CP	*/
#define PPP_CCP		UINT_C(0x80fd)		/* CCP		*/

#define PPP_LCP		UINT_C(0xc021)		/* LCP		*/
#define PPP_PAP		UINT_C(0xc023)		/* PAP		*/
#define PPP_LQR		UINT_C(0xc025)		/* LQR		*/
#define PPP_CHAP	UINT_C(0xc223)		/* CHAP		*/

#define GET_PPP_HDR(nbuf)	((T_PPP_HDR*)((nbuf)->buf))

/*
 *  リンクの状態
 */

#define PPP_PHASE_DEAD		0
#define PPP_PHASE_INITIALIZE	1
#define PPP_PHASE_DORMANT	2
#define PPP_PHASE_ESTABLISH	3
#define PPP_PHASE_AUTHENTICATE	4
#define PPP_PHASE_CALLBACK	5
#define PPP_PHASE_NETWORK	6
#define PPP_PHASE_TERMINATE	7
#define PPP_PHASE_HOLDOFF	8

/*
 *  PPP リンク制御 (CP) ヘッダ構造体
 */

typedef struct t_ppp_cp_hdr {
	uint8_t		code;		/* 制御コード	*/
	uint8_t		id;		/* 識別子	*/
	uint16_t	len;		/* 長さ		*/
	} __attribute__((packed, aligned(2)))T_PPP_CP_HDR;

#define GET_PPP_CP_HDR(nbuf)	((T_PPP_CP_HDR*)((nbuf)->buf + sizeof(T_PPP_HDR)))

/*
 *  PPP リンク制御 (CP) の制御コード
 */

#define PPP_VENDER		UINT_C(0)	/* Vender Specific		*/
#define PPP_CONFREQ		UINT_C(1)	/* Configuration Request	*/
#define PPP_CONFACK		UINT_C(2)	/* Configuration ACK		*/
#define PPP_CONFNAK		UINT_C(3)	/* Configuration NAK		*/
#define PPP_CONFREJ		UINT_C(4)	/* Configuration Reject		*/
#define PPP_TERMREQ		UINT_C(5)	/* Termination Request		*/
#define PPP_TERMACK		UINT_C(6)	/* Termination ACK		*/
#define PPP_CODEREJ		UINT_C(7)	/* Code Reject			*/

/*
 * 構成情報 (CI) ヘッダ
 */

typedef struct t_ppp_ci_hdr {
	uint8_t		type;		/* CI の型	*/
	uint8_t		len;		/* 長さ		*/
	} __attribute__((packed, aligned(2)))T_PPP_CI_HDR;

#endif	/* of #ifndef _PPP_H_ */
