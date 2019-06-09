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
 *	@(#)tcp_timer.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/tcp_timer.h,v 1.13.2.1 1999/08/29 16:29:56 peter Exp $
 */
 
#ifndef _TCP_TIMER_H_
#define _TCP_TIMER_H_

/* TCP タイマ変数 */

//#define NUM_TCP_TIMERS		4	/* TCP のタイマ数	*/

#define TCP_TIM_REXMT		0	/* 再送タイマ		*/
#define TCP_TIM_PERSIST		1	/* 持続タイマ		*/
#define TCP_TIM_KEEP		2	/* 保留 (keep alive) タイマ*/
#define TCP_TIM_2MSL		3	/* 2MSL タイマ		*/

/* MSL: Maximum Segment Lifetime, 最大セグメント生存時間 		*/

/*
 *  TCP/IP のタイマの定義
 */

#define TCP_SLOW_TMO		(ULONG_C(500000)/NET_TIMER_CYCLE)	/* 500,000 [us]	*/
#define TCP_FAST_TMO		(ULONG_C(200000)/NET_TIMER_CYCLE)	/* 200,000 [us]	*/

#define TCP_SLOW_HZ		(ULONG_C(1000000)/TCP_SLOW_TMO/NET_TIMER_CYCLE)	/* Hz	*/

/* TCP タイマ値 */

#define TCP_TVAL_MSL		(TCP_SLOW_HZ * 30)
					/* 最大セグメント生存時間	*/

#define TCP_TVAL_SRTT_BASE	0	/* 滑らかな往復時間の基準値*/

#define TCP_TVAL_RTO_BASE	(TCP_SLOW_HZ * 3)
		/* 情報が得られない場合の再送タイムアウトの基準値	*/

#define TCP_TVAL_MIN_PERSIST	(TCP_SLOW_HZ * 5)
				/* 持続タイムアウトの最小値	*/
#define TCP_TVAL_MAX_PERSIST	(TCP_SLOW_HZ * 60)
				/* 持続タイムアウトの最大値	*/

#define TCP_TVAL_KEEP_INIT	(TCP_SLOW_HZ * 75)
					/* 保留タイマの初期値	*/
#define TCP_TVAL_KEEP_IDLE	(TCP_SLOW_HZ * 2 * 60 * 60)
					/* 保留タイマ起動までの標準値*/
#define TCP_TVAL_KEEP_INTERVAL	(TCP_SLOW_HZ * 75)
					/* 保留タイマインターバルの標準値*/
#define TCP_TVAL_KEEP_COUNT	8	/* 保留タイムアウトの最大値*/

#define TCP_TVAL_MIN		(TCP_SLOW_HZ * 3)
					/* 時間の最小単位	*/
#define TCP_TVAL_MAX_REXMT	(TCP_SLOW_HZ * 64)
					/* 最大再送時間		*/

/*
 *  以下の変数の値はマクロで設定する。
 *
 *  tcp_maixidle   = tcp_keepcnt * tcp_keepintvl
 *                 = TCP_TVAL_KEEP_COUNT * TCP_TVAL_KEEP_INTERVAL
 *  tcp_keepcnt    = TCP_TVAL_KEEP_COUNT
 *  tcp_keepintval = TCP_TVAL_KEEP_INTERVAL
 *
 */

/*
 *  その他の定数
 */

#define TCP_MAX_REXMT_SHIFT	12	/* 最大再送回数		*/

/*
 *  関数
 */

extern void tcp_slow_timo (void *ignore);
extern void tcp_fast_timo (void *ignore);

extern T_TCP_TIME tcp_range_set (T_TCP_TIME value, T_TCP_TIME tvmin, T_TCP_TIME tvmax);

#endif	/* of #ifndef _TCP_TIMER_H_ */
