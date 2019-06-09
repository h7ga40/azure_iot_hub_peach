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
 *  ルーティング表
 */

#include <kernel.h>

#include <tinet_defs.h>
#include <tinet_config.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

/* IPv6 に関する定義 */

#ifdef _IP6_CFG

#if NUM_IN6_ROUTE_ENTRY > 0

T_IN6_RTENTRY routing6_tbl[NUM_IN6_ROUTE_ENTRY];

#endif	/* of #if NUM_IN6_ROUTE_ENTRY > 0 */

#endif	/* of #ifdef _IP6_CFG */

/* IPv4 に関する定義 */

#ifdef _IP4_CFG

#ifdef SUPPORT_LOOP

T_IN4_RTENTRY routing4_tbl[NUM_IN4_ROUTE_ENTRY] = {
	{ 0, 0, 0 },
	};

#endif	/* of #ifdef SUPPORT_LOOP */

#ifdef SUPPORT_PPP

T_IN4_RTENTRY routing4_tbl[NUM_IN4_ROUTE_ENTRY] = {
	{ 0, 0, 0 },
	};

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

T_IN4_RTENTRY routing4_tbl[NUM_IN4_ROUTE_ENTRY] = {

	/* 異なる LAN、default gateway による間接配送	*/
	{ 0,			0,			IPV4_ADDR_DEFAULT_GW	},

	/* 同一 LAN 内、直接配送				*/
	{ IPV4_ADDR_LOCAL &
	  IPV4_ADDR_LOCAL_MASK,	IPV4_ADDR_LOCAL_MASK,	0			},

	/* 同一 LAN 内へのブロードキャスト、直接配送	*/
	{ 0xffffffff,		0xffffffff,		0			},

	};

#endif	/* of #ifdef SUPPORT_ETHER */

#endif	/* of #ifdef _IP4_CFG */
