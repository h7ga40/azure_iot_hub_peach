/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2018-2019 Cores Co., Ltd. Japan
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

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include <t_stdlib.h>

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>

#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_var.h>

#include "netapp/dhcp4_cli.h"
#include "netapp/dhcp4_cli_var.h"
#include "ntp_cli.h"
#include "kernel_cfg.h"
#include "tinet_cfg.h"
#include <stdio.h>

T_DHCP4_CLI_CONTEXT *dhcp4_cli_initialize(ID tskid, ID cepid);
int dhcp4_cli_get_timer(T_DHCP4_CLI_CONTEXT *ct);
void dhcp4_cli_progress(T_DHCP4_CLI_CONTEXT *ct, int elapse);
void dhcp4_cli_wakeup(T_DHCP4_CLI_CONTEXT *ct);
void dhcp4_cli_timeout(T_DHCP4_CLI_CONTEXT *ct);

int net_misc_get_timer(T_DHCP4_CLI_CONTEXT *ct, T_NTP_CLI_CONTEXT *nc)
{
	int timer = dhcp4_cli_get_timer(ct);

	if ((nc->timer != TMO_FEVR) && ((timer == TMO_FEVR) || (timer > nc->timer))) {
		timer = nc->timer;
	}

	return timer;
}

extern T_NTP_CLI_CONTEXT ntp_cli;

/*
 *  DHCP, NTP クライアント タスク
 */

void
net_misc_task (intptr_t exinf)
{
	T_DHCP4_CLI_CONTEXT *ct;
	T_NTP_CLI_CONTEXT *nc = &ntp_cli;
	ER		error = E_OK, ret;
	SYSTIM		prev, time;
	int timer;

	get_tid(&nc->tskid);
	syslog(LOG_NOTICE, "[NET MISC:%d,%d] started.", nc->tskid, (ID)exinf);

	/* 初期化 */
	ct = dhcp4_cli_initialize(nc->tskid, UDP4_DHCP_CLI_CEPID);
	ntp_cli_initialize(nc, NTP_CLI_CEPID);

	ret = get_tim(&time);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "[NET MISC,%d] get_tim error: %7lu,%s",
			nc->cepid, time / SYSTIM_HZ, itron_strerror(ret));
		return;
	}

	while (true) {
		prev = time;

		/* タイマー取得 */
		timer = net_misc_get_timer(ct, nc);

		/* 待ち */
		error = tslp_tsk(timer);
		if ((error != E_OK) && (error != E_TMOUT)) {
			syslog(LOG_NOTICE, "[NET MISC,%d] tslp_tsk error: %s %d",
				nc->cepid, itron_strerror(error), timer);
			break;
		}

		ret = get_tim(&time);
		if (ret != E_OK) {
			syslog(LOG_NOTICE, "[NET MISC,%d] get_tim error: %s",
				nc->cepid, itron_strerror(ret));
			break;
		}

		/* 時間経過 */
		int elapse = time - prev;
		dhcp4_cli_progress(ct, elapse);
		ntp_cli_progress(nc, elapse);

		/* 起こされた場合 */
		if (error == E_OK) {
			dhcp4_cli_wakeup(ct);
			ntp_cli_wakeup(nc);
		}

		/* タイムアウト処理 */
		dhcp4_cli_timeout(ct);
		ntp_cli_timeout(nc);
	}
}
