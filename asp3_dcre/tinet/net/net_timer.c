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

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

/*
 *  変数
 */

static T_NET_CALLOUT callout[NUM_NET_CALLOUT];

/*
 *  呼出しエントリ登録
 */

void
timeout (callout_func func, void *arg, RELTIM timeout)
{
	int_t ix;

	/* 呼出しエントリを獲得する */
	syscall(wai_sem(SEM_CALL_OUT_LOCK));
	for (ix = NUM_NET_CALLOUT; callout[-- ix].func != NULL; )
		if (ix == 0) {
			syslog(LOG_WARNING, "[NET] callout busy, size=%d.", NUM_NET_CALLOUT);
			syscall(sig_sem(SEM_CALL_OUT_LOCK));
			return;
			}

	/* 呼出しエントリを設定する */
	callout[ix].func	= func;
	callout[ix].arg		= arg;
	callout[ix].timeout	= timeout;
	syscall(sig_sem(SEM_CALL_OUT_LOCK));
	}

/*
 *  呼出しエントリ削除
 */

void
untimeout (callout_func func, void *arg)
{
	int_t ix;

	/* エントリを探す */
	for (ix = NUM_NET_CALLOUT; ix -- > 0; ) {
		syscall(wai_sem(SEM_CALL_OUT_LOCK));
		if (callout[ix].func == func && callout[ix].arg == arg && callout[ix].timeout > 0) {
			callout[ix].func = NULL;
			syscall(sig_sem(SEM_CALL_OUT_LOCK));
			break;
			}
		syscall(sig_sem(SEM_CALL_OUT_LOCK));
		}
	}

/*
 *  ネットワークタイマタスク
 *
 *    注意: タスクコンテキストで実行する必要があるため，
 *          周期ハンドラは使用できない。
 */

static char const tinet_banner[] =
"\n"
"TINET %d.%d.%d (" __DATE__ ", " __TIME__ ")\n"
"Copyright (C) 2001-2012 by Dep. of Computer Science and Engineering\n"
"                    Tomakomai National College of Technology, JAPAN\n";

void
net_timer_task(intptr_t exinf)
{
	ID		tskid;
	int_t		ix;
	callout_func	func;
	void 		*arg;

	get_tid(&tskid);
	syslog(LOG_NOTICE, tinet_banner,
	       (TINET_PRVER   >> 12) & UINT_C(0x0f),
	       (TINET_PRVER   >>  4) & UINT_C(0x0f),
	        TINET_PRVER          & UINT_C(0x0f));
	syslog(LOG_NOTICE, "[NET/TIMER:%d] started.", tskid);

	/* IP を初期化する。*/

#if defined(_IP6_CFG)
	ip6_init();
#endif

#if defined(_IP4_CFG)
	ip_init();
#endif

#ifdef SUPPORT_TCP

	/* TCP 出力タスクを起動する */
	syscall(act_tsk(TCP_OUTPUT_TASK));

#endif	/* of #ifdef SUPPORT_TCP */

#if defined(SUPPORT_UDP) && defined(UDP_CFG_NON_BLOCKING)

	/* UDP 出力タスクを起動する */
	syscall(act_tsk(UDP_OUTPUT_TASK));

#endif	/* of #if defined(SUPPORT_UDP) && defined(UDP_CFG_NON_BLOCKING) */

	while (true) {
		/* タイムアウトしたエントリを呼出す。*/
		syscall(wai_sem(SEM_CALL_OUT_TIMEOUT));
		for (ix = NUM_NET_CALLOUT; ix -- > 0; ) {
			syscall(wai_sem(SEM_CALL_OUT_LOCK));
			if (callout[ix].func != NULL && callout[ix].timeout == 0) {
				func = callout[ix].func;
				arg  = callout[ix].arg;
				callout[ix].func = NULL;
				syscall(sig_sem(SEM_CALL_OUT_LOCK));
				(func)(arg);
				}
			else
				syscall(sig_sem(SEM_CALL_OUT_LOCK));
			}
		}
	}

/*
 *  ネットワークタイマハンドラ
 *
 *    NET_TIMER_CYCLE (100ms) 毎に呼出される。
 */

void
net_timer_handler(intptr_t exinf)
{
	int_t	ix, count = 0;

	for (ix = NUM_NET_CALLOUT; ix -- > 0; ) {
		if (callout[ix].func != NULL && callout[ix].timeout > 0) {
			callout[ix].timeout --;
			if (callout[ix].timeout == 0)
				count ++;
			}
		}

	if (count)
		syscall(isig_sem(SEM_CALL_OUT_TIMEOUT));
	}
