/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
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

#ifndef _NTSHELL_MAIN_H_
#define _NTSHELL_MAIN_H_

/*
 *		ntshellタスクのヘッダファイル
 */

/*
 *  ターゲット依存の定義
 */
#include <kernel.h>

#include "usrcmd.h"
#include "core/ntshell.h"

/*
 *  各タスクの優先度の定義
 */

#define NTSHELL_PRIORITY	5		/* ntshellタスクの優先度 */
#define SHELLCMD_PRIORITY	8		/* shellcmdタスクの優先度 */

#define NTSHELL_STACK_SIZE	2048	/* ntshellタスクのスタック領域のサイズ */
#define SHELLCMD_STACK_SIZE	10240	/* shellcmdタスクのスタック領域のサイズ */

/*
 *  関数のプロトタイプ宣言
 */
#ifndef TOPPERS_MACRO_ONLY

typedef struct task_base_t {
	void (*on_start)(struct task_base_t *task, ID tskid);
	void (*on_end)(struct task_base_t *task);
	int (*get_timer)(struct task_base_t *task);
	void (*progress)(struct task_base_t *task, int elapse);
	void (*process)(struct task_base_t *task, uint32_t event);
	int timer;
	void *exinf;
} task_base_t;

typedef struct
{
	const cmd_table_t *table;
	int count;
} cmd_table_info_t;

extern cmd_table_info_t cmd_table_info;

extern void main_initialize();
extern void main_finalize();

/* ntshellの初期化 */
void sys_init(intptr_t exinf);

#define NTSHELL_EVENT_WAKEUP		0x00000001
#define NTSHELL_EVENT_NETIF_CHANGED	0x00000002
#define NTSHELL_EVENT_LINK_UP		0x00000004
#define NTSHELL_EVENT_UP			0x00000008

/* ntshellタスク初期化 */
void ntshell_task_init(task_base_t **tasks, int task_count);

/* ntshellタスク */
void ntshell_task(intptr_t exinf);

/* shellcmdタスク */
void shellcmd_task(intptr_t exinf);

/* コマンド実行 */
int cmd_execute(const char *text, void *extobj);

/* 
 * ノンブロッキングコールのコールバック関数
 */
extern ER socket_udp_callback(ID cepid, FN fncd, void *p_parblk);

#endif /* TOPPERS_MACRO_ONLY */

#endif	/* of #ifndef _NTSHELL_MAIN_H_ */
