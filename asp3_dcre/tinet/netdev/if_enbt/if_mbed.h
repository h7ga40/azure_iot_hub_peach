/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2009 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *  Copyright (C) 2014-2019 Cores Co., Ltd. Japan
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id$
 */

/*
 * Copyright (C) 1993, David Greenman. This software may be used, modified,
 *   copied, distributed, and sold, in both source and binary form provided
 *   that the above copyright and these terms are retained. Under no
 *   circumstances is the author responsible for the proper functioning
 *   of this software, nor does the author assume any responsibility
 *   for damages incurred with its use.
 */

#ifndef _IF_MBED_H_
#define _IF_MBED_H_

#include "ethernet_api.h"

/*
 *  NIC の選択マクロ
 */

#define IF_ETHER_NIC_GET_SOFTC()	if_mbed_get_softc()
#define IF_ETHER_NIC_WATCHDOG(i)	if_mbed_watchdog(i)
#define IF_ETHER_NIC_PROBE(i)		if_mbed_probe(i)
#define IF_ETHER_NIC_INIT(i)		if_mbed_init(i)
#define IF_ETHER_NIC_READ(i)		if_mbed_read(i)
#define IF_ETHER_NIC_RESET(i)		if_mbed_reset(i)
#define IF_ETHER_NIC_START(i,o)		if_mbed_start(i,o)

#define T_IF_ETHER_NIC_SOFTC		struct t_mbed_softc

/* IPv6 関係 */

#define IF_ETHER_NIC_IN6_IFID(i,a)	get_mac6_ifid(i,a)	/* インタフェース識別子の設定		*/
#define IF_ETHER_NIC_ADDMULTI(s)	if_mbed_addmulti(s)		/* マルチキャストアドレスの登録		*/

#define PHY_TASK_WAIT				200000
#define IF_MBED_PHY_PRIORITY			4
#define IF_MBED_PHY_STACK_SIZE			256

#if !defined(TOPPERS_MACRO_ONLY) && !defined(_MACRO_ONLY)

/*
 *  前方参照
 */

#ifndef T_IF_SOFTC_DEFINED

typedef struct t_if_softc T_IF_SOFTC;

#define T_IF_SOFTC_DEFINED

#endif	/* of #ifndef T_IF_SOFTC_DEFINED */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

/*
 *  関数
 */

extern T_IF_SOFTC *if_mbed_get_softc(void);
extern void if_mbed_watchdog(T_IF_SOFTC *ic);
extern void if_mbed_probe(T_IF_SOFTC *ic);
extern void if_mbed_init(T_IF_SOFTC *ic);
extern void if_mbed_reset(T_IF_SOFTC *ic);
extern T_NET_BUF *if_mbed_read(T_IF_SOFTC *ic);
extern void if_mbed_start(T_IF_SOFTC *ic, T_NET_BUF *output);
extern ER if_mbed_addmulti(T_IF_SOFTC *ic);
extern void if_mbed_eth_handler(void);
void if_mbed_phy_task(intptr_t arg);

#endif /* #if !defined(TOPPERS_MACRO_ONLY) && !defined(_MACRO_ONLY) */

#endif	/* of #ifndef _IF_MBED_H_ */
