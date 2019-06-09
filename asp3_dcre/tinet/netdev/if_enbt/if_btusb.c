/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2009 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *  Copyright (C) 2014-2017 by Cores Co., Ltd. Japan
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
 * Copyright (c) 1995, David Greenman
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * $FreeBSD: src/sys/i386/isa/if_ed.c,v 1.148.2.4 1999/09/25 13:08:18 nyan Exp $
 */

#define __BTSTACK_FILE__ "if_btusb.c"

#define CAST(type, val)		((type)(val))

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "kernel/kernel_impl.h"
#include "btstack/utils.h"
#include "bt_main.h"
#include "usb_hbth.h"
//#include "hal_cpu.h"
//#include "hal_tick.h"
#include "hci.h"
#include "debug.h"

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_timer.h>
#include <net/net_count.h>
#include <net/net_buf.h>
#include "if_mbed.h"
#include "if_btusb.h"

#include <string.h>

#ifdef _MSC_VER
#include <stdlib.h>
#endif

#ifndef TMO_IF_BTUSB_GET_NET_BUF
#define TMO_IF_BTUSB_GET_NET_BUF		ULONG_C(1000000)	/* [us] */
#endif

static void (*btstack_network_send_packet_callback)(uint16_t bnep_cid, const uint8_t * packet, uint16_t size);

int btstack_network_output_spos = 0;
int btstack_network_output_rpos = 0;
T_NET_BUF *btstack_network_output[NUM_IF_BTUSB_TXBUF];

/*
 *  ネットワークインタフェースのソフトウェア情報
 */

/* ネットワークインタフェースに依存するソフトウェア情報 */

//static int connection_count;


/*
 *  局所変数
 */

static void btusb_stop (T_BTUSB_SOFTC *sc);
static void btusb_init_sub (T_IF_SOFTC *ic);

#ifdef SUPPORT_INET6

static uint32_t ds_crc (uint8_t *addr);
static void ds_getmcaf (T_IF_SOFTC *ic, uint32_t *mcaf);

/*
 *  ds_crc -- イーサネットアドレスの CRC を計算する。
 */

#define POLYNOMIAL	0x04c11db6

static uint32_t
ds_crc (uint8_t *addr)
{
	uint32_t	crc = ULONG_C(0xffffffff);
	int_t		carry, len, bit;
	uint8_t		byte;

	for (len = ETHER_ADDR_LEN; len -- > 0; ) {
		byte = *addr ++;
		for (bit = 8; bit -- > 0; ) {
			carry   = ((crc & ULONG_C(0x80000000)) ? 1 : 0) ^ (byte & UINT_C(0x01));
			crc   <<= 1;
			byte   >>= 1;
			if (carry)
				crc = (crc ^ POLYNOMIAL) | carry;
		}
	}
	return crc;
}

#undef POLYNOMIAL

/*
 *  ds_getmcaf -- マルチキャストアドレスのリストからマルチキャストアドレス
 *                フィルタを計算する。
 */

static void
ds_getmcaf (T_IF_SOFTC *ic, uint32_t *mcaf)
{
	uint32_t	count, index;
	uint8_t		*af = (uint8_t*)mcaf;

	mcaf[0] = mcaf[1] = 0;

	for (count = MAX_IF_MADDR_CNT; count -- > 0; ) {
		index = ds_crc(ic->maddrs[count].lladdr) >> 26;
		af[index >> 3] |= 1 << (index & 7);
	}
}

/*
 * btusb_setrcr -- 受信構成レジスタ (RCR) を設定する。
 */

static void
btusb_setrcr (T_IF_SOFTC *ic)
{
}

/*
 * btusb_addmulti -- マルチキャストアドレスを追加する。
 */

ER
btusb_addmulti (T_IF_SOFTC *ic)
{
	btusb_setrcr(ic);
	return E_OK;
}

#endif	/* of #ifdef SUPPORT_INET6 */

T_BTUSB_SOFTC *get_softc(T_IF_SOFTC *ic, uint16_t bnep_cid)
{
	T_BTUSB_SOFTC *sc = ic->sc->btusb;

	for (int i = 0; i < MAX_PAN_CONNECTIONS; i++) {
		if (sc->bnep_cid == bnep_cid)
			return sc;
		sc++;
	}

	return NULL;
}

void btstack_network_init(void (*send_packet_callback)(uint16_t bnep_cid, const uint8_t * packet, uint16_t size))
{
	btstack_network_send_packet_callback = send_packet_callback;
}

void btstack_network_up(uint16_t bnep_cid, bd_addr_t local_addr, bd_addr_t remote_addr)
{
	T_IF_SOFTC *ic = &if_softc;
	T_BTUSB_SOFTC *sc = get_softc(ic, 0);

	if (sc == NULL) {
		log_info("btstack_network_up no space");
		return;
	}

	sc->bnep_cid = bnep_cid;

	/* MACアドレスを更新 */
	memcpy(sc->bd_addr, remote_addr, sizeof(sc->bd_addr));

	snprintf(sc->bnep_name, sizeof(sc->bnep_name), "BNEP%04x", bnep_cid);

	//connection_count++;
	//if (connection_count == 1)
	//	ether_set_link_up(ic);
}

const char * btstack_network_get_name(uint16_t bnep_cid)
{
	T_IF_SOFTC *ic = &if_softc;
	T_BTUSB_SOFTC *sc = get_softc(ic, bnep_cid);

	if (sc == NULL) {
		return "???";
	}

	return sc->bnep_name;
}

int btstack_network_down(uint16_t bnep_cid)
{
	T_IF_SOFTC *ic = &if_softc;
	T_BTUSB_SOFTC *sc = get_softc(ic, bnep_cid);

	if (sc == NULL) {
		log_info("btstack_network_down(bnep_cid=%d)", bnep_cid);
		return -1;
	}

	//connection_count--;
	//if (connection_count == 0)
	//	ether_set_link_down(ic);

	sc->bnep_cid = 0;

	return 0;
}

/*
 *  btusb_stop -- ネットワークインタフェースを停止する。
 * 
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void
btusb_stop (T_BTUSB_SOFTC *sc)
{
}

void btusb_bus_init() {}
void btusb_inter_init() {}

/*
 *  btusb_init_sub -- ネットワークインタフェースの初期化
 * 
 *    注意: NIC 割り込み禁止状態で呼び出すこと。
 */

static void
btusb_init_sub (T_IF_SOFTC *ic)
{
	/* ターゲット依存部の割込み初期化 */
	btusb_inter_init();
}

/*
 * btusb_reset -- ネットワークインタフェースをリセットする。
 */

void
btusb_reset (T_IF_SOFTC *ic)
{
	T_BTUSB_SOFTC *sc = ic->sc->btusb;

	NET_COUNT_ETHER_NIC(net_count_ether_nic[NC_ETHER_NIC_RESETS], 1);

	for (int i = 0; i < MAX_PAN_CONNECTIONS; i++) {
		if (sc->bnep_cid != 0)
			btusb_stop(sc);
		sc++;
	}

	btusb_init_sub(ic);
}

/*
 * btusb_watchdog -- ネットワークインタフェースのワッチドッグタイムアウト
 */

void
btusb_watchdog (T_IF_SOFTC *ic)
{
	btusb_reset(ic);
}

/*
 * btusb_probe -- ネットワークインタフェースの検出
 */

void
btusb_probe (T_IF_SOFTC *ic)
{
	/* ターゲット依存部のバスの初期化 */
	btusb_bus_init();
}

/*
 * btusb_init -- ネットワークインタフェースの初期化
 */

void
btusb_init (T_IF_SOFTC *ic)
{
	/* btusb_init 本体を呼び出す。*/
	btusb_init_sub(ic);
}

/*
 * btstack_network_process_packet -- btstackからフレームの受信
 */

void
btstack_network_process_packet(uint16_t bnep_cid, const uint8_t * packet, uint16_t size)
{
	T_IF_SOFTC *ic = &if_softc;
	T_BTUSB_SOFTC *sc = get_softc(ic, bnep_cid);
	T_NET_BUF *input = NULL;
	uint16_t align;
	uint8_t *dst;
	ER error;

	if (sc == NULL) {
		log_info("btstack_network_process_packet(bnep_cid=%d)", bnep_cid);
		return;
	}

	/*
	 *  +-----------+--------+---------+---------+
	 *  | Ehter HDR | IP HDR | TCP HDR | TCP SDU |
	 *  +-----------+--------+---------+---------+
	 *        14        20        20        n
	 *   <----------------- len ---------------->
	 *              ^
	 *              t_net_buf で 4 オクテット境界にアラインされている。
	 *
	 *  tcp_input と udp_input では、擬似ヘッダと SDU でチェックサムを
	 *  計算するが、n が 4 オクテット境界になるように SDU の後ろに 0 を
	 *  パッディングする。その分を考慮して net_buf を獲得しなければならない。
	 */
	align = ((((size - sizeof(T_IF_HDR)) + 3) >> 2) << 2) + sizeof(T_IF_HDR);

	if ((error = tget_net_buf(&input, align, TMO_IF_BTUSB_GET_NET_BUF)) == E_OK && input != NULL) {
		dst = input->buf + IF_ETHER_NIC_HDR_ALIGN;
		memcpy(dst, (void *)packet, size);

		if_mbed_start(ic, input);

		rel_net_buf(input);
	}

	/* BMEP側にも転送（マルチキャストと宛先MACアドレスがあるもの） */
	if ((error = tget_net_buf(&input, align, TMO_IF_BTUSB_GET_NET_BUF)) == E_OK && input != NULL) {
		dst = input->buf + IF_ETHER_NIC_HDR_ALIGN;
		memcpy(dst, (void *)packet, size);

		int rel = btusb_start(ic, input);
		if (rel) {
			rel_net_buf(input);
		}
	}
}

/*
 * btusb_start -- 送信フレームをバッファリングする。
 */

int
btusb_start (T_IF_SOFTC *ic, T_NET_BUF *output)
{
	T_BTUSB_SOFTC *sc = ic->sc->btusb;
	T_ETHER_HDR	*eth = GET_ETHER_HDR(output);
	bool_t send = (btstack_network_output_spos == btstack_network_output_rpos)
		&& (btstack_network_output[btstack_network_output_rpos] == NULL);
	bool_t enqueue = false;

	if ((btstack_network_send_packet_callback == NULL)
		|| ((btstack_network_output_spos == btstack_network_output_rpos)
			&& (btstack_network_output[btstack_network_output_rpos] != NULL))) {
		sig_sem(ic->semid_rxb_ready);
		return 1;
	}

	output->conn_pos = 0;

	if (send) {
		while (output->conn_pos < MAX_PAN_CONNECTIONS) {
			if ((sc->bnep_cid != 0)
				&& ((*eth->dhost & ETHER_MCAST_ADDR) || memcmp(eth->dhost, sc->bd_addr, sizeof(eth->dhost)) == 0)) {
				enqueue = true;
				break;
			}
			output->conn_pos++;
			sc++;
		}
	}
	else
		enqueue = true;

	if (enqueue) {
		loc_cpu();

		btstack_network_output[btstack_network_output_spos] = output;
		btstack_network_output_spos++;
		if (btstack_network_output_spos == NUM_IF_BTUSB_TXBUF)
			btstack_network_output_spos = 0;

		unl_cpu();
	}
	else {
		sig_sem(ic->semid_rxb_ready);
		return 1;
	}

	if (!send)
		return 0;

	btstack_network_send_packet_callback(sc->bnep_cid, output->buf, output->len);

	return 0;
}

/*
 * btstack_network_packet_sent -- バッファした送信フレームの送信が完了する。
 */

void
btstack_network_packet_sent(uint16_t bnep_cid)
{
	T_IF_SOFTC *ic = &if_softc;
	T_NET_BUF *output = btstack_network_output[btstack_network_output_rpos];
	T_BTUSB_SOFTC *sc;
	T_ETHER_HDR	*eth;

	while (output != NULL) {
		sc = &ic->sc->btusb[output->conn_pos];
		eth = GET_ETHER_HDR(output);

		for (;;) {
			output->conn_pos++;
			sc++;
			if (output->conn_pos >= MAX_PAN_CONNECTIONS)
				break;
			if ((sc->bnep_cid != 0)
				&& ((*eth->dhost & ETHER_MCAST_ADDR) || memcmp(eth->dhost, sc->bd_addr, sizeof(eth->dhost)) == 0)) {
				btstack_network_send_packet_callback(sc->bnep_cid, output->buf, output->len);
				return;
			}
		}

		rel_net_buf(output);

		loc_cpu();

		btstack_network_output[btstack_network_output_rpos] = NULL;
		btstack_network_output_rpos++;
		if (btstack_network_output_rpos == NUM_IF_BTUSB_TXBUF)
			btstack_network_output_rpos = 0;

		output = btstack_network_output[btstack_network_output_rpos];

		unl_cpu();

		sig_sem(ic->semid_rxb_ready);
	}
}

static void dummy_handler(void) {};
static void(*tick_handler)(void) = &dummy_handler;
void hal_tick_handler();

void hal_tick_init(void)
{
	//    timer_regist_userfunc(hal_tick_handler);
}

void hal_tick_set_handler(void(*handler)(void))
{
	if (handler == NULL) {
		tick_handler = &dummy_handler;
		return;
	}
	tick_handler = handler;
}

int  hal_tick_get_tick_period_in_ms(void)
{
	return 1;
}

void hal_tick_handler()
{
	tick_handler();
}

uint32_t hal_time_ms(void)
{
	SYSTIM time;
	get_tim(&time);
	return time / 1000;
}

void hal_cpu_disable_irqs(void)
{
	loc_cpu();
}

void hal_cpu_enable_irqs(void)
{
	unl_cpu();
}

int btstack_sleep = 0;

void hal_cpu_enable_irqs_and_sleep(void)
{
	unl_cpu();
	btstack_sleep = 1;
}

//#define PACKET_DUMP
static void usb_dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size);
static void(*usb_packet_handler)(uint8_t packet_type, uint8_t *packet, uint16_t size) = usb_dummy_handler;

void usb_bt_callback(uint8_t event_type, uint8_t *data, int size)
{
#ifdef PACKET_DUMP
	int len = size;

	switch (event_type) {
	case HCI_EVENT_PACKET:
		printf("EVT <= ");
		break;
	case HCI_ACL_DATA_PACKET:
		printf("ACL <= ");
		break;
	default:
		printf("??? <= ");
		break;
	}

	if (len > 256)
		len = 256;
	printf_hexdump(data, len);
#endif
	usb_packet_handler(event_type, data, size);
}

static void usb_init(const void *transport_config)
{

}

static int usb_open(void *transport_config)
{
	return 0;
}

static int usb_close(void *transport_config)
{
	return 0;
}

static int usb_send_packet(uint8_t packet_type, uint8_t *packet, int size)
{
	int r;
#ifdef PACKET_DUMP
	int len = size;
	if (len > 256)
		len = 256;
#endif
	switch (packet_type) {
	case HCI_COMMAND_DATA_PACKET:
#ifdef PACKET_DUMP
		printf("CMD => ");
		printf_hexdump(packet, len);
#endif
		r = bt_usb_hbth_control_write(packet, size);
		if (r < 0 || r != size) {
			log_error("Error submitting control transfer %d", r);
			return -1;
		}
		return 0;
	case HCI_ACL_DATA_PACKET:
#ifdef PACKET_DUMP
		printf("ACL => ");
		printf_hexdump(packet, len);
#endif
		if (size >= 1024)
			printf("HCI_ACL_DATA_PACKET size=%d\n", size);
		r = bt_usb_hbth_bulk_write(packet, size);
		if (r < 0) {
			log_error("Error submitting data transfer, %d", r);
			return -1;
		}
		return 0;
	default:
		return -1;
	}
}

static void usb_register_packet_handler(void(*handler)(uint8_t packet_type, uint8_t *packet, uint16_t size))
{
	log_info("registering packet handler.");
	usb_packet_handler = handler;
}

static const char * usb_get_transport_name(void){
	return "USB";
}

static void usb_dummy_handler(uint8_t packet_type, uint8_t *packet, uint16_t size)
{
}

static int usb_acl_out_active = 0;
static int usb_command_active = 0;

static int usb_can_send_packet_now(uint8_t packet_type)
{
	switch (packet_type) {
	case HCI_COMMAND_DATA_PACKET:
		return !usb_command_active;
	case HCI_ACL_DATA_PACKET:
		return !usb_acl_out_active;
	default:
		return 0;
	}
}

// single instance
static const hci_transport_t hci_transport_usb =
{
	/*.open                          =*/ usb_open,
	/*.close                         =*/ usb_close,
	/*.send_packet                   =*/ usb_send_packet,
	/*.register_packet_handler       =*/ usb_register_packet_handler,
	/*.get_transport_name            =*/ usb_get_transport_name,
	/*.set_baudrate                  =*/ NULL,
	/*.can_send_packet_now           =*/ NULL
};

// get usb singleton
const hci_transport_t *hci_transport_usb_instance()
{
	return &hci_transport_usb;
}
