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
#include <stdlib.h>
#include <stdbool.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#include "kernel_cfg.h"
#include "main.h"
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
#include "ffarch.h"
#include "ff.h"
#include <stdio.h>
#include "usrcmd.h"
#include "fdtable.h"
#include "gpio_api.h"
#include "ntshell_main.h"
#include "ntp_cli.h"
#include "netcmd.h"
#include "client.h"

ID ws_api_mailboxid = MAIN_DATAQUEUE;
#ifndef NOUSE_MPF_NET_BUF
ID ws_mempoolid = MPF_NET_BUF_256;
#endif

bool_t dhcp_enable = true;

uint8_t mac_addr[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0x01};
PRI main_task_priority = MAIN_PRIORITY + 1;

static void netif_link_callback(T_IFNET *ether);
extern int execute_command(int wait);

enum main_state_t {
	main_state_start,
	main_state_idle,
	main_state_start_dhcp,
};

struct main_t {
	int timer;
	enum main_state_t state;
	SYSTIM prev, now;
	int dhcp_req, dhcp_res;
};
struct main_t main_obj;

static void main_initialize();
static int main_get_timer();
static void main_progress(int interval);
static void main_timeout();

extern int ntshell_exit;

int uart_read(char *buf, int cnt, void *extobj)
{
	struct main_t *obj = (struct main_t *)extobj;
	int result;
	ER ret;
	int timer;

	obj->prev = obj->now;

	if (main_obj.dhcp_req != main_obj.dhcp_res) {
		main_obj.dhcp_res = main_obj.dhcp_req;
		main_obj.state = main_state_start_dhcp;
		main_obj.timer = 0;
	}

	/* タイマー取得 */
	timer = main_get_timer();

	/* 待ち */
	ret = serial_trea_dat(SIO_PORTID, buf, cnt, timer);
	if ((ret < 0) && (ret != E_OK) && (ret != E_TMOUT) && (ret != E_RLWAI)) {
		syslog(LOG_NOTICE, "tslp_tsk ret: %s %d", itron_strerror(ret), timer);
		ntshell_exit = 1;
		return -1;
	}
	result = (int)ret;

	ret = get_tim(&obj->now);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "get_tim ret: %s", itron_strerror(ret));
		ntshell_exit = 1;
		return -1;
	}

			/* 時間経過 */
	int elapse = obj->now - obj->prev;
	main_progress(elapse);

	/* タイムアウト処理 */
	main_timeout();

	return result;
}

int uart_write(const char *buf, int cnt, void *extobj)
{
	return serial_wri_dat(SIO_PORTID, buf, cnt);
}

ntshell_t ntshell;

/*
 * メインタスク
 */
void main_task(intptr_t exinf)
{
	main_initialize();

	ntshell_init(&ntshell, uart_read, uart_write, cmd_execute, &main_obj);
	ntshell_set_prompt(&ntshell, "NTShell>");
	ntshell_execute(&ntshell);
}

int wolfSSL_Debugging_ON(void);

/*
 * 初期化
 */
static void main_initialize()
{
	ER ret;

	//wolfSSL_Debugging_ON();

	ntshell_task_init(SIO_PORTID);

	/* 初期化 */
	ffarch_init();

	main_obj.timer = 100000;
	main_obj.state = main_state_start;

	gpio_t led_blue, led_green, led_red, sw;
	gpio_init_out(&led_blue, LED_BLUE);
	gpio_init_out(&led_green, LED_GREEN);
	gpio_init_out(&led_red, LED_RED);
	gpio_init_in(&sw, USER_BUTTON0);

	bool_t exec = gpio_read(&sw) == 1;

	gpio_write(&led_blue, 1);
	gpio_write(&led_green, exec ? 1 : 0);
	gpio_write(&led_red, 0);

	if (!dhcp_enable) {
		dhcp4c_rel_info();
		in4_add_ifaddr(IPV4_ADDR_STAIC_LOCAL, IPV4_ADDR_STAIC_LOCAL_MASK);
	}

	gpio_write(&led_green, 0);

	iothub_client_init();

	ret = get_tim(&main_obj.now);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "get_tim");
		ext_tsk();
		return;
	}
}

/*
 * タイマー取得
 */
static int main_get_timer()
{
	int timer = main_obj.timer;

	return timer;
}

/*
 * 時間経過
 */
static void main_progress(int interval)
{
	if (main_obj.timer != TMO_FEVR) {
		main_obj.timer -= interval;
		if (main_obj.timer < 0) {
			main_obj.timer = 0;
		}
	}
}

/*
 * タイムアウト処理
 */
static void main_timeout()
{
	ER ret;

	if (main_obj.timer != 0)
		return;

	switch (main_obj.state) {
	case main_state_start:
		ether_set_link_callback(netif_link_callback);
		main_obj.state = main_state_idle;
		main_obj.timer = TMO_FEVR;
		break;
	case main_state_start_dhcp:
		ret = dhcp4c_renew_info();
		if (ret == E_OK) {
			main_obj.state = main_state_idle;
			main_obj.timer = TMO_FEVR;
		}
		else {
			main_obj.state = main_state_start_dhcp;
			main_obj.timer = 1000000;
		}
		break;
	default:
		main_obj.state = main_state_idle;
		main_obj.timer = TMO_FEVR;
		break;
	}
}

/* MACアドレスの設定時に呼ばれる */
void mbed_mac_address(char *mac)
{
	memcpy(mac, mac_addr, 6);
}

static void netif_link_callback(T_IFNET *ether)
{
	uint8_t link_up = (ether->flags & IF_FLAG_LINK_UP) != 0;
	uint8_t up = (ether->flags & IF_FLAG_UP) != 0;
	ER ret;

	if (dhcp_enable) {
		if (!link_up)
			dhcp4c_rel_info();
		else if (!up) {
			ret = dhcp4c_renew_info();
			if ((ret != E_OK) && (main_obj.dhcp_req == main_obj.dhcp_res)) {
				main_obj.dhcp_req++;
				rel_wai(MAIN_TASK);
			}
		}
	}
	else {
		up = link_up;
	}

	if (link_up && up)
		ntp_cli_execute();

	ntshell_change_netif_link(link_up, up);
}

int custom_rand_generate_seed(uint8_t* output, int32_t sz)
{
	SYSTIM now;
	int32_t i;

	get_tim(&now);
	srand(now);

	for (i = 0; i < sz; i++)
		output[i] = rand();

	return 0;
}

extern int curl_main(int argc, char **argv);
extern int iothub_client_main(int argc, char **argv);

static const cmd_table_t cmdlist[] = {
	{"cd", "change directory", usrcmd_cd },
	{"ls", "list files", usrcmd_ls },
	{"cp", "copy file", usrcmd_cp },
	{"rm", "remove file", usrcmd_rm },
	{"mv", "move file", usrcmd_mv },
	{"mkdir", "Make directory", usrcmd_mkdir},
	{"hexdump", "Hex dump", usrcmd_hexdump},
	{"date", "print date and time", usrcmd_date},
	{"curl", "Command lines or scripts to transfer data", curl_main},
	{"ping", "ping", usrcmd_ping},
	{"dhcpc", "DHCP Client rel/renew/info", usrcmd_dhcp4c},
	{"dnsc", "DNS client", usrcmd_dnsc },
	{"ntpc", "NTP client", usrcmd_ntpc },
	{"iothub", "Asure IoT Hub Client", iothub_client_main},
	{"dps_csgen", "Generate a connection string", dps_csgen_main},
	{"set_cs", "Set connection string", set_cs_main},
	{"info", "This is a description text string for info command.", usrcmd_info},
	{"exit", "Exit Natural Tiny Shell", usrcmd_exit},
};
cmd_table_info_t cmd_table_info = { cmdlist, sizeof(cmdlist) / sizeof(cmdlist[0]) };
