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
 */

#ifndef _IF6_VAR_H_
#define _IF6_VAR_H_

/*
 *  ネットワークインタフェース構造体
 */

#if defined(_IP6_CFG)

#if defined(_IP4_CFG)

struct t_ifnet {
	T_IF_SOFTC	*ic;		/* ネットワークインタフェースのソフトウェア情報	*/
	T_IN6_IFADDR 	in6_ifaddrs[NUM_IN6_IFADDR_ENTRY];
					/* IPv6 アドレス情報				*/
	T_IN6_ADDR	in6_maddrs [MAX_IN6_MADDR_CNT];
					/* マルチキャスト IPv6 アドレス			*/
	T_IN4_IFADDR 	in4_ifaddr;	/* IPv4 アドレス情報				*/
	ether_status_callback_fn link_callback;
	uint8_t		flags;		/* フラグ					*/
	};

#else	/* of #if defined(_IP4_CFG) */

struct t_ifnet {
	T_IF_SOFTC	*ic;		/* ネットワークインタフェースのソフトウェア情報	*/
	T_IN6_IFADDR 	in6_ifaddrs[NUM_IN6_IFADDR_ENTRY];
					/* IPv6 アドレス情報				*/
	T_IN6_ADDR	in6_maddrs [MAX_IN6_MADDR_CNT];
					/* マルチキャスト IPv6 アドレス			*/
	ether_status_callback_fn link_callback;
	uint8_t		flags;		/* フラグ					*/
	};

#endif	/* of #if defined(_IP4_CFG) */

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

struct t_ifnet {
	T_IF_SOFTC	*ic;		/* ネットワークインタフェースのソフトウェア情報	*/
	T_IN4_IFADDR 	in4_ifaddr;	/* IPv4 アドレス情報				*/
	ether_status_callback_fn link_callback;
	uint8_t		flags;		/* フラグ					*/
	};

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

#if defined(_IP6_CFG)

/*
 *  関数シミュレーションマクロ
 */

#define ND6_IFPTOMAC(i)		(IF_SOFTC_TO_IFADDR((i)->ic))

/*
 *  関数
 */

extern ER if_addmulti (T_IFNET *ifp, void *maddr, uint8_t type);

#endif	/* of #if defined(_IP6_CFG) */

#endif	/* of #ifndef _IF6_VAR_H_ */
