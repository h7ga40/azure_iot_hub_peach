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
 * if_ppp.h - Point-to-Point Protocol definitions.
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD: src/sys/net/if_ppp.h,v 1.13.2.1 1999/08/29 16:28:21 peter Exp $
 */

#ifndef _IF_PPP_H_
#define _IF_PPP_H_

#ifdef SUPPORT_PPP

/*
 *  PPP アドレスの定義（ダミー）
 */

typedef struct t_ppp_addr {
	uint8_t		lladdr[0];
	} T_PPP_ADDR;

/*
 *  インタフェースの選択マクロ
 */

#define T_IF_HDR		T_PPP_HDR		/* インタフェースのヘッダ		*/
#define T_IF_ADDR		T_PPP_ADDR		/* インタフェースのアドレス			*/
#define IF_HDR_ALIGN		2			/* ヘッダのアライン単位			*/
#define IF_MTU			DEF_PPP_MTU		/* インタフェースの MTU			*/

#define IF_OUTPUT(o,d,g,t)	ppp_output(o,t)		/* インタフェースの出力関数		*/
#define IF_RAW_OUTPUT(o,t)	ppp_output(o,t)		/* インタフェースの出力関数、アドレス解決無し*/
#define IF_SET_PROTO(b,p)	(*GET_PPP_HDR(b) = htons(p))
							/* インタフェースのプロトコル設定関数	*/
#define IF_SOFTC_TO_IFADDR(s)	((T_IF_ADDR*)(s)->ifaddr.lladdr)
							/* ソフトウェア情報から PPP アドレスを取り出す	*/
#define IF_GET_IFNET()		ppp_get_ifnet()		/* ネットワークインタフェース構造体を返す。*/
#define IF_SRAND()		(0)			/* インターフェースの乱数初期値			*/

/* IPv4 関係 */

#define IF_PROTO_IP		PPP_IP			/* インタフェースの IPv4 プロトコル指定	*/

/* IPv6 関係 */

#define MAX_IF_MADDR_CNT	0			/* インタフェースのマルチキャストアドレス配列の最大サイズ*/
#define IF_PROTO_IPV6		PPP_IPV6		/* インタフェースの IPv6 プロトコル指定	*/
#define IF_IN6_NEED_CACHE(i)	(false)			/* 近隣探索キャッシュを使用する。	*/
#define IF_IN6_IFID(i,a)	get_rand_ifid(i,a)	/* インタフェース識別子の設定		*/
#define IF_IN6_RESOLVE_MULTICAST(i,m)	{}		/* インタフェースのマルチキャストアドレスへの変換		*/

/*
 *  MTU (MRU) に関する定義
 */

#define DEF_PPP_MTU		1500	/* PPP の 標準 MTU	*/
#define MAX_PPP_MTU		16384	/* PPP の 最大 MTU	*/
#define MIN_PPP_MTU		128	/* PPP の 最小 MTU	*/

/*
 *  PPP ヘッダ
 */

typedef uint16_t T_PPP_HDR;				/* 上位プロトコル		*/

/*
 *  前方参照
 */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

#ifndef T_IF_SOFTC_DEFINED

typedef struct t_if_softc T_IF_SOFTC;

#define T_IF_SOFTC_DEFINED

#endif	/* of #ifndef T_IF_SOFTC_DEFINED */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

/*
 *  ネットワークインタフェースに依存しないソフトウェア情報
 */

struct t_if_softc {
	T_IF_ADDR	ifaddr;		/* ネットワークインタフェースのアドレス	*/
	};

/*
 *  関数
 */

extern T_IFNET *ppp_get_ifnet (void);
ER ppp_output (T_NET_BUF *data, TMO tmout);

#endif	/* of #ifdef SUPPORT_PPP */

#endif	/* of #ifndef _IF_PPP_H_ */
