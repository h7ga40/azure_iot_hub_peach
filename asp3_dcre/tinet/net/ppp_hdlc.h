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
 * $FreeBSD: src/usr.sbin/ppp/hdlc.h,v 1.21 1999/12/30 13:27:15 brian Exp $
 *
 *	TODO:
 */

#ifndef _PPP__HDLC_H_
#define _PPP__HDLC_H_

#ifdef SUPPORT_PPP

/*
 *  RFC1171 (PPP) の HDLC 部の定義
 */

/*
 *  既定値
 */

#define HDLC_FLAG_SEQ		UINT_C(0x7e)		/* Flag Sequence			*/
#define HDLC_CNTL_ESC		UINT_C(0x7d)		/* Control Escape octet			*/
#define HDLC_ADDR_ALL		UINT_C(0xff)		/* Address: All-Stations		*/
#define HDLC_CNTL_UI		UINT_C(0x03)		/* Control: UI				*/
#define HDLC_ESC_MASK		UINT_C(0x20)		/* Escape Mask				*/
#define HDLC_INIT_FCS		UINT_C(0xffff)		/* Initial FCS value			*/
#define HDLC_INIT_FCS_AC	UINT_C(0x3de3)		/* Initial FCS value with		*/
							/* Address and Control: UI		*/
#define HDLC_GOOD_FCS		UINT_C(0xf0b8)		/* Good final FCS value			*/

/*
 *  関数
 */

extern ER HDLC_write (T_NET_BUF *data);
extern ER HDLC_read (T_NET_BUF *data, uint16_t size);
extern void HDLC_dummy_read (void);

#ifdef PPP_CFG_MODEM

extern int_t HDLC_raw_getoctet (void);
extern void HDLC_raw_putoctet (uint8_t ch);

#endif	/* of #ifdef PPP_CFG_MODEM */

#endif	/* of #ifdef SUPPORT_PPP */

#endif	/* of #ifndef _PPP__HDLC_H_ */
