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

/*	$FreeBSD: src/sys/netinet6/nd6.h,v 1.10 2002/04/19 04:46:23 suz Exp $	*/
/*	$KAME: nd6.h,v 1.76 2001/12/18 02:10:31 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
#ifndef _ND6_H_
#define _ND6_H_

/*
 *  ネットワークインタフェース構造体（T_IFNET）のフラグ
 */

#define ND6_IFF_IFDISABLED	UINT_C(0x01)	/* インタフェースを無効にする。*/

/*
 *  近隣キャッシュ 
 */

typedef struct t_llinfo_nd6 {
	T_IN6_ADDR	addr;
	T_NET_BUF	*hold;
	TMO		tmout;
	SYSTIM		expire;		/* [us] 有効時間が切れる時刻	*/
	T_IF_ADDR	ifaddr;
	uint8_t		state;
	uint8_t		asked;
	uint8_t		flags;
	} T_LLINFO_ND6;

/* 近隣キャッシュの状態 */

#define ND6_LLINFO_FREE			0	/* 未使用		*/
#define ND6_LLINFO_NO_STATE		1	/* 状態未定義		*/
#define ND6_LLINFO_INCOMPLETE		2	/* 未解決		*/
#define ND6_LLINFO_REACHABLE		3	/* 到達可能		*/
#define ND6_LLINFO_STALE		4	/* 到達可能性は未確認	*/
#define ND6_LLINFO_DELAY		5	/* 到達可能性の確認待機	*/
#define ND6_LLINFO_PROBE		6	/* 到達可能性を確認中	*/

#define ND6_IS_LLINFO_PROBREACH(n)	((n)->state > ND6_LLINFO_INCOMPLETE)

/* 近隣キャッシュの制御ビット */

#define ND6_LLIF_ROUTER			UINT_C(0x80)	/* ルータ		*/
#define ND6_LLIF_ASKED			UINT_C(0x40)	/* 通知済み		*/
#define ND6_LLIF_STATE_MASK		UINT_C(0x07)	/* 状態マスク		*/

/* 近隣探索キャッシュ、有効時間の定義 */

#define ND6_SLOW_TMO			((60*60*1000)/NET_TIMER_HZ)
							/* 1 [時間]				*/
							/* 近隣探索キャッシュ更新タイムアウト	*/

#define ND6_MAX_DAD_TRY			15		/* 重複アドレス検出の再送回数		*/
#define ND6_MAX_MCAST_QUERY		3		/* マルチキャストアドレス要請の再送回数	*/
#define ND6_MAX_UCAST_QUERY		3		/* ユニキャストアドレス要請の再送回数	*/

#define ND6_GCOLLECTION_TIME		(60*60*24*SYSTIM_HZ)
							/* ゴミ拾い周期、1 日			*/
#define	ND6_DELAY_TIME			(5*SYSTIM_HZ)	/* 確認待機遅延				*/
#define ND6_FIRST_DAD_DELAY_TIME	(1*SYSTIM_HZ)	/* 最初の重複アドレス検出送信遅延	*/
#define ND6_DAD_DELAY_TIME		(5*SYSTIM_HZ/10)
							/* 重複アドレス検出送信遅延		*/

/* 近隣探索用タイマの設定 */

#define ND6_TIMER_TMO			((SYSTIM_HZ/NET_TIMER_CYCLE)/5)	/* 0.2[s] 間隔		*/

/*
 *  到達可能状態の有効時間
 */

#define	ND6_DEF_REACHABLE_TIME		(30*SYSTIM_HZ)	/* 到達可能状態の有効時間の既定値		*/
#define	ND6_MAX_REACHABLE_TIME		(60*60*SYSTIM_HZ)
							/* 到達可能状態の有効時間の既定値		*/
#define	ND6_RECALC_REACHTM_INTERVAL	(60*60*2*SYSTIM_HZ)
							/* 到達可能状態の有効時間再計算間隔	*/

#define	ND6_MIN_REACHTM_RAND_FACTOR	512		/* 最小乱数補正値、1,024 * 0.5		*/

#if 1							/* タイマーの起動間隔を考慮し、規定より短く設定	*/
#define	ND6_MAX_REACHTM_RAND_FACTOR	1484		/* 最小乱数補正値、1,024 * 1.45			*/
#else
#define	ND6_MAX_REACHTM_RAND_FACTOR	1536		/* 最小乱数補正値、1,024 * 1.5			*/
#endif

/* 到達可能状態の有効時間の計算マクロ */

#define ND6_CALC_REACH_TIME(b)		\
	 ((ND6_MIN_REACHTM_RAND_FACTOR * ((uint32_t)(b) >> 10)) + \
	  (net_rand() & ((ND6_MAX_REACHTM_RAND_FACTOR - ND6_MIN_REACHTM_RAND_FACTOR) * ((uint32_t)(b) >> 10))))

/*
 *  近隣探索の送信間隔
 */

#define ND6_DEF_RETRANS_TIME		(1*SYSTIM_HZ)

/*
 *  ルータ通知により変更されることがある値
 */

#if NUM_ND6_DEF_RTR_ENTRY > 0

/* ルータ通知を受信する場合は、通知された値 */

#define	ND6_REACHABLE_TIME		nd6_reachable_time
#define ND6_RETRANS_TIME		nd6_retrans_time

#else	/* #if NUM_ND6_DEF_RTR_ENTRY > 0 */

/* ルータ通知を受信しない場合の値は既定値 */

#define	ND6_REACHABLE_TIME		ND6_DEF_REACHABLE_TIME
#define ND6_RETRANS_TIME		ND6_DEF_RETRANS_TIME

#endif	/* #if NUM_ND6_DEF_RTR_ENTRY > 0 */

/*
 *  ディフォルトルータ・リスト
 */

typedef struct t_def_router {
	T_IN6_ADDR	addr;
	SYSTIM		expire;		/* [us] 有効時間が切れる時刻	*/
	uint32_t	lifetime;	/* [us] 有効時間			*/
	uint16_t	plistmap;	/* プレフィックスマップ		*/
	uint8_t		flags;		/* ルータ通知のフラグ		*/
	uint8_t		state;
	} T_DEF_ROUTER;

/* ディフォルトルータ・リストの状態 */

#define ND6_DEF_RTR_BUSY		0xff	/* 使用中		*/

/* プレフィックスのインデックス */

#define ND6_PREFIX_IX_INVALID		0xff

/*
 *  プレフィックス・リスト
 */

typedef struct t_nd6_prefix {
	T_IN6_ADDR	prefix;		/* プレフィックス		*/
	uint32_t	expire;		/* [s] 有効時間が切れる時刻	*/
	uint32_t	preferred;	/* [s] 推奨有効時間が切れる時刻	*/
	uint32_t	vltime;		/* [s] 有効時間			*/
	uint32_t	pltime;		/* [s] 推奨有効時間		*/
	uint16_t	routermap;	/* ルータマップ			*/
	uint8_t		flags;		/* フラグ			*/
	uint8_t		prefix_len;	/* プレフィックス長		*/
	} T_ND6_PREFIX;

/* フラグ */

#define ND6_PREFIX_FLAG_ONLINK		UINT_C(0x80)	/* オンリンクプレフィックス	*/
#define ND6_PREFIX_FLAG_AUTO		UINT_C(0x40)	/* ステートレス・アドレス自動設定*/

#define ND6_PREFIX_STATE_BUSY		UINT_C(0x01)	/* 使用中		*/
#define ND6_PREFIX_STATE_ONLINK		UINT_C(0x02)	/* オンリンク状態	*/
#define ND6_PREFIX_STATE_DETACHED	UINT_C(0x04)	/* 非使用状態	*/

/*
 *  変数
 */

extern SYSTIM	nd6_base_reachable_time;
extern SYSTIM	nd6_reachable_time;
extern SYSTIM	nd6_recalc_reachtm_interval;
extern SYSTIM	nd6_retrans_time;

/*
 *  関数
 */

extern void nd6_ifattach (T_IFNET *ifp);
extern void nd6_ns_output (T_IFNET *ifp, const T_IN6_ADDR *daddr,
                           const T_IN6_ADDR *taddr, T_LLINFO_ND6 *ln, bool_t dad);
extern void nd6_na_output (T_IFNET *ifp, const T_IN6_ADDR *daddr,
                           const T_IN6_ADDR *taddr, uint32_t flags, bool_t tlladdr);
extern void nd6_dad_start (T_IFNET *ifp, T_IN6_IFADDR *ia, int_t *tick);
extern ER nd6_output (T_IFNET *ifp, T_NET_BUF *output,
                      const T_IN6_ADDR *dst, T_LLINFO_ND6 *ln, TMO tmout);
extern void nd6_na_input (T_NET_BUF *input, uint_t off);
extern void nd6_ns_input (T_NET_BUF *input, uint_t off);
extern void nd6_ra_input (T_NET_BUF *input, uint_t off);
extern ER nd6_storelladdr (T_IF_ADDR *out, const T_IN6_ADDR *dst, T_IF_ADDR *ifa);
extern ER nd6_options (uint8_t *opt, void *nh, uint_t len);
extern T_LLINFO_ND6 *nd6_lookup (const T_IN6_ADDR *addr, bool_t create);
extern ER nd6_output_hold (T_IFNET *ifp, T_LLINFO_ND6 *ln);
extern T_LLINFO_ND6 *nd6_cache_lladdr (T_IFNET *ifp, const T_IN6_ADDR *from, 
                                       T_IF_ADDR *lladdr, uint8_t type, uint8_t code);
extern void nd6_defrtrlist_timer (void);
extern void nd6_prelist_timer (void);
extern void nd6_rtrsol_ctl (void);
extern const T_IN6_ADDR *nd6_router_lookup (void);
extern T_DEF_ROUTER *nd6_defrtrlist_lookup (const T_IN6_ADDR *src);
extern void nd6_defrtrlist_del (T_DEF_ROUTER *dr);
extern void nd6_timer (void);
extern const T_DEF_ROUTER *nd6_get_drl (uint_t *count);
extern const T_ND6_PREFIX *nd6_get_prl (void);
extern const T_LLINFO_ND6 *nd6_get_cache (void);
extern ER nd6_prefix_onlink (T_ND6_PREFIX *pr);
extern ER nd6_prefix_offlink (T_ND6_PREFIX *pr);
extern T_ND6_PREFIX *nd6_onlink_prefix_lookup (const T_IN6_ADDR *dst);

#endif	/* of #ifndef _ND6_H_ */
