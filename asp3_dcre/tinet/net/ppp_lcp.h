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
 * lcp.h - Link Control Protocol definitions.
 *
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
 *
 * $FreeBSD: src/usr.sbin/pppd/lcp.h,v 1.6.2.1 1999/08/29 15:47:03 peter Exp $
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
 *
 * $FreeBSD: src/usr.sbin/ppp/lcp.h,v 1.23.2.1 2000/08/19 09:30:04 brian Exp $
 *
 *	TODO:
 */

#ifndef _PPP_LCP_H_
#define _PPP_LCP_H_

#ifdef SUPPORT_PPP

/*
 *  LCP 特有の PPP リンク制御 (CP) の制御コード
 */

#define PPP_PROTREJ		UINT_C(8)	/* Protocol Reject	*/
#define PPP_ECHOREQ		UINT_C(9)	/* Echo Request		*/
#define PPP_ECHOREP		UINT_C(10)	/* Echo Reply		*/
#define PPP_DISCREQ		UINT_C(11)	/* Discard Request	*/

/*
 *  LCP の構成情報 (CI)
 */

/*
 *  CI の型
 */

#define LCP_CIT_MRU		UINT_C(1)	/* 最大受信量 (Maximum Receive Unit)	*/
#define LCP_CIT_ACCM		UINT_C(2)	/* Async Control Character Map		*/
#define LCP_CIT_AUTH		UINT_C(3)	/* 認証プロトコル			*/
#define LCP_CIT_QUALITY		UINT_C(4)	/* 品質プロトコルは未実装			*/
#define LCP_CIT_MAGIC		UINT_C(5)	/* マジック番号				*/
#define LCP_CIT_PCOMP		UINT_C(7)	/* プロトコル部圧縮は実装予定		*/
#define LCP_CIT_ACCOMP		UINT_C(8)	/* アドレス・制御部圧縮は実装予定		*/
#define LCP_CIT_FCS_ALTER	UINT_C(9)	/* FCS-Alternatives は未実装		*/
#define LCP_CIT_SELF_PAD	UINT_C(10)	/* Self-Describing-Padding は未実装	*/
#define LCP_CIT_CALLBACK	UINT_C(13)	/* Callback は未実装			*/
#define LCP_CIT_CFRAMES		UINT_C(15)	/* Compound-Frames は未実装		*/

/*
 *  LCP の構成情報オプション
 */

typedef struct t_lcp_configs {
	uint32_t	ACCM;		/* ACCM			*/

#ifdef LCP_CFG_MAGIC

	uint32_t	magic;		/* マジック番号		*/

#endif	/* of #ifdef LCP_CFG_MAGIC */

	uint16_t	MRU;		/* MRU			*/
	uint16_t	options;	/* オプションフラグ	*/
	} __attribute__((packed, aligned(2)))T_LCP_CONFIGS;

/*
 *  変数
 */

extern T_PPP_FSM	lcp_fsm;
extern T_PPP_PROTENT	lcp_protent;
extern T_LCP_CONFIGS	lcp_local_ack_cfg;	/* 相手に許可されたの自構成情報	*/
extern T_LCP_CONFIGS	lcp_remote_ack_cfg;	/* 相手に許可した構成情報		*/

/*
 *  関数
 */

extern void lcp_lowerup (void);
extern void lcp_lowerdown (void);
extern void lcp_down (T_PPP_FSM *fsm);
extern void lcp_finished (T_PPP_FSM *fsm);
extern void lcp_open (uint8_t open_mode);
extern void lcp_close (void);
extern void lcp_sprotrej (T_NET_BUF *data);

#endif	/* of #ifdef SUPPORT_PPP */

#endif	/* of #ifndef _PPP_LCP_H_ */
