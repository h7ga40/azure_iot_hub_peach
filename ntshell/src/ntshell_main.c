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

/* 
 *  サンプルプログラム(1)の本体
 */

#include "shellif.h"
#include <kernel.h>
#include <t_stdlib.h>
#include <sil.h>
#include <setjmp.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#include "kernel_cfg.h"
#include "ffarch.h"
#include "ff.h"
#include "core/ntshell.h"
#include "core/ntlibc.h"
#include "util/ntstdio.h"
#include "usrcmd.h"
#include "util/ntopt.h"
#include "ntshell_main.h"
#include "fdtable.h"
#ifndef NTSHELL_NO_SOCKET
#include <tinet_config.h>
#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <tinet_nic_defs.h>
#include <tinet_cfg.h>
#include <netinet/in_var.h>
#include <net/ethernet.h>
#include <net/if6_var.h>
#include <net/net.h>
#include <net/if_var.h>
#include <netinet/udp_var.h>
#include "netapp/dhcp4_cli.h"
#include "ntp_cli.h"
#endif

char command[NTOPT_TEXT_MAXLEN];

extern uint8_t mac_addr[6];
const struct utsname host_name = {
	"TOPPERS/ASP3",
	TARGET_NAME,
	"3.2.0",
	"3.2.0",
	TARGET_NAME,
	"toppers.jp"
};

int shell_uname(struct utsname *uts)
{
	memcpy(uts, &host_name, sizeof(host_name));
	return 0;
}

enum ntshell_state_t {
	ntshell_state_start,
	ntshell_state_idle,
#ifndef NTSHELL_NO_SOCKET
	ntshell_state_start_dhcp,
#endif
	ntshell_state_servey_cmd,
};

struct ntshell_obj_t {
	ntshell_t ntshell;
	int timer;
	enum ntshell_state_t state;
	SYSTIM prev, now;
	task_base_t **tasks;
	int task_count;
#ifndef NTSHELL_NO_SOCKET
	uint8_t link;
	uint8_t link_up;
	uint8_t up;
	uint8_t dhcp;
#endif
	int event_req, event_res;
};
struct ntshell_obj_t ntshell_obj;

#ifndef NTSHELL_NO_SOCKET
static void netif_link_callback(T_IFNET *ether);
static void ntshell_change_netif_link(uint8_t link_up, uint8_t up);
#endif
static void ntshell_initialize(struct ntshell_obj_t *obj);
static void ntshell_finalize(struct ntshell_obj_t *obj);
static int ntshell_get_timer(struct ntshell_obj_t *obj);
static void ntshell_progress(struct ntshell_obj_t *obj, int interval);
static void ntshell_timeout(struct ntshell_obj_t *obj, bool_t wakeup);
static int execute_command(struct ntshell_obj_t *obj, int wait);
static int usrcmd_ntopt_callback(long *args, void *extobj);

extern void stdio_open(ID portid);

extern bool_t dhcp_enable;
extern int ntshell_exit;
int shellcmd_exit_code;
volatile int shellcmd_state;
jmp_buf shellcmd_exit;

void ntshell_task_init(task_base_t **tasks, int task_count)
{
	ntshell_obj.tasks = tasks;
	ntshell_obj.task_count = task_count;
}

int uart_read(char *buf, int cnt, void *extobj)
{
	struct ntshell_obj_t *obj = (struct ntshell_obj_t *)extobj;
	int result;
	ER ret;
	int timer;
	bool_t wakeup = false;

	obj->prev = obj->now;

	if (obj->event_req != obj->event_res) {
		obj->event_res = obj->event_req;
		wakeup = true;
#ifndef NTSHELL_NO_SOCKET
		if (obj->dhcp) {
			obj->dhcp = 0;
			obj->state = ntshell_state_start_dhcp;
		}
#endif
		obj->timer = 0;
	}

	/* タイマー取得 */
	timer = ntshell_get_timer(obj);

	/* 待ち */
	ret = serial_trea_dat(SIO_PORTID, buf, cnt, timer);
	if ((ret < 0) && (ret != E_OK) && (ret != E_TMOUT) && (ret != E_RLWAI)) {
		syslog(LOG_NOTICE, "tslp_tsk ret: %s %d", itron_strerror(ret), timer);
		ntshell_exit = 1;
		return -1;
	}
	if (ret != E_TMOUT)
		wakeup = true;
	result = (int)ret;

	ret = get_tim(&obj->now);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "get_tim ret: %s", itron_strerror(ret));
		ntshell_exit = 1;
		return -1;
	}

	/* 時間経過 */
	int elapse = obj->now - obj->prev;
	ntshell_progress(obj, elapse);

	/* タイムアウト処理 */
	ntshell_timeout(obj, wakeup);

	return result;
}

int uart_write(const char *buf, int cnt, void *extobj)
{
	return serial_wri_dat(SIO_PORTID, buf, cnt);
}

int cmd_execute(const char *text, void *extobj)
{
	if (text != NULL) {
		ntlibc_strlcpy(command, text, sizeof(command));
	}
	ER ret;
	ID tskid = 0;

	ret = get_tid(&tskid);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "get_tid %d", ret);
	}

	return execute_command(&ntshell_obj, tskid == NTSHELL_TASK);
}

/*
 *  ntshellタスク
 */
void ntshell_task(intptr_t exinf)
{
	struct ntshell_obj_t *obj = (struct ntshell_obj_t *)&ntshell_obj;

	/* 初期化 */
	ffarch_init();

	stdio_open(SIO_PORTID);

	main_initialize();

	ntshell_initialize(obj);

	ntshell_init(&obj->ntshell, uart_read, uart_write, cmd_execute, obj);
	ntshell_set_prompt(&obj->ntshell, "NTShell>");
	ntshell_execute(&obj->ntshell);

	ntshell_finalize(obj);
}

/*
 * 初期化
 */
static void ntshell_initialize(struct ntshell_obj_t *obj)
{
	ER ret;
	ID tskid = 0;

	ret = get_tid(&tskid);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "get_tid %d", ret);
		return;
	}

	obj->timer = 100000;
	obj->state = ntshell_state_start;

	for (int i = 0; i < obj->task_count; i++) {
		task_base_t *task = obj->tasks[i];
		task->on_start(task, tskid);
	}

	ret = get_tim(&obj->now);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "get_tim");
		ext_tsk();
		return;
	}
}

static void ntshell_finalize(struct ntshell_obj_t *obj)
{
	for (int i = 0; i < obj->task_count; i++) {
		task_base_t *task = obj->tasks[i];
		task->on_end(task);
	}

	main_finalize();
}

/*
 * タイマー取得
 */
static int ntshell_get_timer(struct ntshell_obj_t *obj)
{
	int timer = obj->timer;

	for (int i = 0; i < obj->task_count; i++) {
		task_base_t *task = obj->tasks[i];
		int timer2 = task->get_timer(task);
		if ((timer == -1) || ((timer2 != -1) && (timer > timer2)))
			timer = timer2;
	}

	return timer;
}

/*
 * 時間経過
 */
static void ntshell_progress(struct ntshell_obj_t *obj, int elapse)
{
	if (obj->timer != TMO_FEVR) {
		obj->timer -= elapse;
		if (obj->timer < 0) {
			obj->timer = 0;
		}
	}

	for (int i = 0; i < obj->task_count; i++) {
		task_base_t *task = obj->tasks[i];
		task->progress(task, elapse);
	}
}

/*
 * タイムアウト処理
 */
static void ntshell_timeout(struct ntshell_obj_t *obj, bool_t wakeup)
{
	ER ret;
	uint32_t event = wakeup ? NTSHELL_EVENT_WAKEUP : 0;

	if (!wakeup && (obj->timer != 0))
		return;

	switch (obj->state) {
	case ntshell_state_start:
#ifndef NTSHELL_NO_SOCKET
		ether_set_link_callback(netif_link_callback);
#endif
		obj->state = ntshell_state_idle;
		obj->timer = TMO_FEVR;
		break;
#ifndef NTSHELL_NO_SOCKET
	case ntshell_state_start_dhcp:
		ret = dhcp4c_renew_info();
		if (ret == E_OK) {
			obj->state = ntshell_state_idle;
			obj->timer = TMO_FEVR;
		}
		else {
			obj->state = ntshell_state_start_dhcp;
			obj->timer = 1000000;
		}
		break;
#endif
	default:
		obj->state = ntshell_state_idle;
		obj->timer = TMO_FEVR;
		break;
	}
#ifndef NTSHELL_NO_SOCKET
	if (obj->link) {
		obj->link = 0;
		event |= NTSHELL_EVENT_NETIF_CHANGED;
		if (obj->link_up)
			event |= NTSHELL_EVENT_LINK_UP;
		if (obj->up)
			event |= NTSHELL_EVENT_UP;

		if (obj->link_up && obj->up)
			ntp_cli_execute();

		ntshell_change_netif_link(obj->link_up, obj->up);
	}
#endif
	for (int i = 0; i < obj->task_count; i++) {
		task_base_t *task = obj->tasks[i];
		task->process(task, event);
	}
}

#ifndef NTSHELL_NO_SOCKET
void netif_link_callback(T_IFNET *ether)
{
	struct ntshell_obj_t *obj = (struct ntshell_obj_t *)&ntshell_obj;
	uint8_t link_up = (ether->flags & IF_FLAG_LINK_UP) != 0;
	uint8_t up = (ether->flags & IF_FLAG_UP) != 0;
	bool_t wake = false;

	if (dhcp_enable) {
		if (!link_up)
			dhcp4c_rel_info();
		else if (!up) {
			wake = dhcp4c_renew_info() != E_OK;
			if (wake) {
				obj->dhcp = 1;
			}
		}
	}
	else {
		up = link_up;
	}

	if ((obj->link_up != link_up) || (obj->up != up)) {
		obj->link = 1;
		wake = true;
	}

	obj->link_up = link_up;
	obj->up = up;

	if (wake && (obj->event_req == obj->event_res)) {
		obj->event_req++;
		rel_wai(NTSHELL_TASK);
	}
}
#endif

/*
 *  shellcmdタスク
 */
void shellcmd_task(intptr_t exinf)
{
	shellcmd_state = 1;

	if (setjmp(shellcmd_exit) == 0) {
		shellcmd_exit_code = ntopt_parse(command, usrcmd_ntopt_callback, NULL);
	}

	fflush(stdout);
	clean_fd();

	shellcmd_state = 2;
}
#ifndef NTSHELL_NO_SOCKET
void ntshell_change_netif_link(uint8_t link_up, uint8_t up)
{
	FLGPTN flgptn;
	T_RTSK rtsk;
	ER ret;

	ret = ref_tsk(SHELLCMD_TASK, &rtsk);
	if ((ret != E_OK) || (rtsk.tskstat == TTS_DMT))
		return;

	FD_SET(0, (fd_set *)&flgptn);

	set_flg(FLG_SELECT_WAIT, flgptn);
}
#endif
static int usrcmd_ntopt_callback(long *args, void *extobj)
{
	const cmd_table_t *p = cmd_table_info.table;
	int result = 0;
	int found = 0;

	if (*args == 0)
		return result;

	if (strcmp((const char *)args[1], "help") == 0) {
		found = 1;
		result = usrcmd_help(args[0], (char **)&args[1]);
	}
	else for (int i = 0; i < cmd_table_info.count; i++) {
		if (strcmp((const char *)args[1], p->cmd) == 0) {
			found = 1;
			result = p->func(args[0], (char **)&args[1]);
			break;
		}
		p++;
	}

	if ((found == 0) && (((const char *)args[1])[0] != '\0'))
		printf("Unknown command found.\n");

	return result;
}

int usrcmd_help(int argc, char **argv)
{
	const cmd_table_t *p = cmd_table_info.table;
	for (int i = 0; i < cmd_table_info.count; i++) {
		fwrite(p->cmd, strlen(p->cmd), 1, stdout);
		fwrite("\t:", strlen("\t:"), 1, stdout);
		puts(p->desc);
		p++;
	}
	return 0;
}

void shell_abort()
{
	shellcmd_exit_code = -1;
	longjmp(shellcmd_exit, 1);
}

void shell_exit(int exitcd)
{
	shellcmd_exit_code = exitcd;
	longjmp(shellcmd_exit, 1);
}

void shell_exit_group(int exitcd)
{
	shellcmd_exit_code = exitcd;
	longjmp(shellcmd_exit, 1);
}

void stdio_input(unsigned char c);

int execute_command(struct ntshell_obj_t *obj, int wait)
{
	T_RTSK rtsk;
	ER ret;

	ret = ter_tsk(SHELLCMD_TASK);
	if ((ret != E_OK) && (ret != E_OBJ)) {
		syslog(LOG_ERROR, "ter_tsk => %d", ret);
	}

	if (ret == E_OK)
		tslp_tsk(100000);

	clean_fd();

	shellcmd_state = 0;
	ret = act_tsk(SHELLCMD_TASK);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "act_tsk => %d", ret);
	}

	if (wait == 0)
		return 0;

	do {
		obj->state = ntshell_state_servey_cmd;
		obj->timer = 100000;

		char c;
		if (obj->ntshell.func_read(&c, sizeof(c), obj->ntshell.extobj) == 1) {
			// Ctrl+Cが押された場合
			if (c == 0x03) {
				// コマンドタスクを終了
				ret = ter_tsk(SHELLCMD_TASK);
				if (ret != E_OK) {
					syslog(LOG_ERROR, "ter_tsk => %d", ret);
				}
				shellcmd_exit_code = -1;
				shellcmd_state = 4;
				if (ret == E_OK)
					tslp_tsk(100000);
				break;
			}

			stdio_input(c);
		}

		ret = ref_tsk(SHELLCMD_TASK, &rtsk);
		if ((ret != E_OK) || (rtsk.tskstat == TTS_DMT))
			shellcmd_state = 3;
	} while(shellcmd_state == 1);

	obj->state = ntshell_state_idle;
	obj->timer = TMO_FEVR;

	clean_fd();

	return shellcmd_exit_code;
}

int shell_clock_getres(clockid_t clk_id, struct timespec *res)
{
	if ((clk_id != CLOCK_REALTIME) && (clk_id != CLOCK_MONOTONIC))
		return -EINVAL;

	memset(&res->tv_sec, 0xFF, sizeof(res->tv_sec));
	res->tv_nsec = 0;

	return 0;
}

int shell_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	SYSTIM now = 0;

	if ((clk_id != CLOCK_REALTIME) && (clk_id != CLOCK_MONOTONIC))
		return -EINVAL;

	get_tim(&now);
	tp->tv_sec = now / 1000000;
	tp->tv_nsec = (now % 1000000) * 1000;

	return 0;
}

int shell_clock_settime(clockid_t clk_id, const struct timespec *tp)
{
	if ((clk_id != CLOCK_REALTIME) && (clk_id != CLOCK_MONOTONIC))
		return -EINVAL;

	SYSTIM time;
	ER ret;

	time = (tp->tv_sec * 1000000ll) + (tp->tv_nsec / 1000ll);

	ret = set_tim(time);
	if (ret != E_OK) {
		return -EPERM;
	}

	return 0;
}

sigset_t g_sigmask;

int shell_sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict old)
{
	if (old != NULL)
		memcpy(old, &g_sigmask, sizeof(sigset_t));

	switch (how) {
	case SIG_BLOCK:
		for (int i = 0; i < sizeof(g_sigmask.__bits) / sizeof(g_sigmask.__bits[0]); i++) {
			g_sigmask.__bits[i] |= set->__bits[i];
		}
		break;
	case SIG_UNBLOCK:
		for (int i = 0; i < sizeof(g_sigmask.__bits) / sizeof(g_sigmask.__bits[0]); i++) {
			g_sigmask.__bits[i] &= ~set->__bits[i];
		}
		break;
	case SIG_SETMASK:
		memcpy(&g_sigmask, set, sizeof(sigset_t));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

// musl-1.1.18\src\internal\ksigaction.h
struct k_sigaction {
	void(*handler)(int);
	unsigned long flags;
	void(*restorer)(void);
	unsigned mask[2];
};

struct k_sigaction sigtable[7];

int shell_sigaction(int sig, const struct k_sigaction *__restrict sa,
	struct k_sigaction *__restrict old, size_t size)
{
	struct k_sigaction *sat;

	switch(sig){
	case SIGALRM:
		sat = &sigtable[0];
		break;
	case SIGFPE:
		sat = &sigtable[1];
		break;
	case SIGILL:
		sat = &sigtable[2];
		break;
	case SIGSEGV:
		sat = &sigtable[3];
		break;
	case SIGBUS:
		sat = &sigtable[4];
		break;
	case SIGABRT:
		sat = &sigtable[5];
		break;
	case SIGPIPE:
		sat = &sigtable[6];
		break;
	default:
		return -EINVAL;
	}

	if (old != NULL)
		memcpy(old, sat, offsetof(struct k_sigaction, mask) + size);

	memcpy(sat, sa, offsetof(struct k_sigaction, mask) + size);

	return 0;
}

int shell_madvise(void *a, size_t b, int c)
{
	return 0;
}

int shell_gettid()
{
	ID tskid;
	ER ret;

	ret = get_tid(&tskid);
	if (ret != E_OK)
		return -1;

	return tskid;
}

int shell_tkill(int tid, int sig)
{
	if ((tid == SHELLCMD_TASK) && (sig == SIGABRT)) {
		shell_abort();
	}

	no_implement("tkill");
	return -1;
}

int shell_kill(int pid, int sig)
{
	DebugBreak();
	return -1;
}

int shell_gettimeofday(struct timeval *tv, void *tzvp)
{
	SYSTIM time;
	if (!tv) return 0;
	get_tim(&time);
	tv->tv_sec = time / 1000000;
	tv->tv_usec = time - (tv->tv_sec * 1000000);
	return 0;
}

int shell_nanosleep(const struct timespec *req, struct timespec *rem)
{
	ER ret;
	TMO tmo;
	SYSTIM prev, now, diff;

	if ((req == NULL) || (req->tv_nsec < 0) || (req->tv_nsec >= 1000000000))
		return -EINVAL;

	get_tim(&prev);

	tmo = req->tv_sec * 1000000 + req->tv_nsec / 1000;
	ret = tslp_tsk(tmo);
	if (ret == E_OK) {
		if (rem != NULL) {
			get_tim(&now);
			diff = now - prev;
			rem->tv_sec = diff / 1000000ll;
			rem->tv_nsec = (diff - (rem->tv_sec * 1000000ll)) * 1000ll;
		}
		return 0;
	}
	else if (ret == E_TMOUT) {
		if (rem != NULL) {
			rem->tv_sec = 0;
			rem->tv_nsec = 0;
		}
		return 0;
	}

	return -EFAULT;
}

ssize_t shell_getrandom(void *buf, size_t buflen, unsigned int flags)
{
	SYSTIM now;
	int32_t i;
	int *output = (int *)buf;
	size_t sz = buflen / 4;

	get_tim(&now);
	srand(now);

	for (i = 0; i < sz; i++)
		output[i] = rand();

	for (i = 4 * sz; i < buflen; i++)
		((char *)buf)[i] = rand();

	return buflen;
}
