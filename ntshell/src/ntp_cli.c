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

/* 
 *  NTP クライアント
 */

#include <string.h>
#include <string.h>
#include <time.h>

#if defined(TARGET_KERNEL_ASP)

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include <stdio.h>

#endif	/* of #if defined(TARGET_KERNEL_ASP) */

#if defined(TARGET_KERNEL_JSP)

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #if defined(TARGET_KERNEL_JSP) */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include "ntp_cli.h"

/* NTP サーバのポート番号 */

#define NTP_SRV_PORTNO		UINT_C(123)

/* NTP サーバのURL */

#define NTP_SRV_URL		"ntp.nict.jp"

/* 送信間隔 */

#define SLP_ITV		(60*SYSTIM_HZ)

/* 関数 */

void ntp_cli_update_time(T_NTP_CLI_CONTEXT *nc);
ER ntp_cli_time_synchronization(T_NTP_CLI_CONTEXT *nc, ntp_mode_t mode);
void ntp_cli_read_time(T_NTP_CLI_CONTEXT *nc, struct timespec *tp);
void ntp_cli_write_time(T_NTP_CLI_CONTEXT *nc, struct timespec *tp);
void ntp_cli_read_data(T_NTP_CLI_CONTEXT *nc);

/*
 *  全域変数
 */

T_NTP_CLI_CONTEXT ntp_cli;

/* UNIX:1970/1/1 - NTP:1900/1/1 */
#define TIME_OFS (25567ll * 24 * 60 * 60)

void set_systime_to_ntptime(struct timespec *tp, T_NTP_TIMESTAMP *timestamp)
{
	timestamp->integer = htonl((uint32_t)(tp->tv_sec + TIME_OFS));
	timestamp->fraction = htonl((((unsigned long long)tp->tv_nsec) << 32) / 1000000000ll);
}

void set_ntptime_to_systime(struct timespec *tp, T_NTP_TIMESTAMP *timestamp)
{
	tp->tv_sec = ntohl(timestamp->integer) - TIME_OFS;
	tp->tv_nsec = ((unsigned long long)(ntohl(timestamp->fraction) * 1000000000ll)) >> 32;
}

void add_timestamp(T_NTP_TIMESTAMP *dst, T_NTP_TIMESTAMP *a, T_NTP_TIMESTAMP *b)
{
	uint32_t ai = ntohl(a->integer);
	uint32_t af = ntohl(a->fraction);
	uint32_t bi = ntohl(b->integer);
	uint32_t bf = ntohl(b->fraction);
	uint32_t f = af + bf;
	uint32_t o = ((f < af) || (f < bf)) ? 1u : 0;
	uint32_t i = ai + bi + o;
	dst->integer = htonl(i);
	dst->fraction = htonl(f);
}

void sub_timestamp(T_NTP_TIMESTAMP *dst, T_NTP_TIMESTAMP *a, T_NTP_TIMESTAMP *b)
{
	uint32_t ai = ntohl(a->integer);
	uint32_t af = ntohl(a->fraction);
	uint32_t bi = ntohl(b->integer);
	uint32_t bf = ntohl(b->fraction);
	uint32_t o = (af < bf) ? 1u : 0u;
	uint32_t f = af - bf;
	uint32_t i = ai - bi - o;
	dst->integer = htonl(i);
	dst->fraction = htonl(f);
}

void div2_timestamp(T_NTP_TIMESTAMP *a)
{
	uint32_t ai = ntohl(a->integer);
	uint32_t af = ntohl(a->fraction);

	if ((ai & 1) != 0) {
		ai >>= 1;
		af >>= 1;
		af |= 0x80000000u;
	}
	else {
		ai >>= 1;
		af >>= 1;
	}

	a->integer = htonl(ai);
	a->fraction = htonl(af);
}

#define NTP_POLL_ASYNC 10		// 1024秒毎に時刻問い合わせ
#define NTP_POLL_NORMAL 16		// 65536秒毎に時刻問い合わせ

void ntp_cli_initialize(T_NTP_CLI_CONTEXT *nc, ID cepid)
{
	nc->cepid = cepid;
	nc->state = NTP_CLI_STATE_ASYNC;
	nc->timer = TMO_FEVR;
	nc->snd_rmt.portno = NTP_SRV_PORTNO;
	nc->snd_rmt.ipaddr = IPV4_ADDRANY;
	nc->rcv_rmt.portno = 0;
	nc->rcv_rmt.ipaddr = IPV4_ADDRANY;
}

void ntp_cli_execute()
{
	ntp_cli.exe_flag = 1;
	syscall(wup_tsk(ntp_cli.tskid));
}

void ntp_cli_wakeup(T_NTP_CLI_CONTEXT *nc)
{
	int rcv_flag = nc->rcv_flag;
	nc->rcv_flag = 0;

	if (rcv_flag > 0) {
		ntp_cli_read_data(nc);
		return;
	}

	if (nc->exe_flag != 0) {
		nc->exe_flag = 0;

		nc->state = NTP_CLI_STATE_ASYNC;
		nc->timer = 0;
	}
}

void ntp_cli_read_data(T_NTP_CLI_CONTEXT *nc)
{
	struct timespec tp;
	T_NTP_MSG *ntp = &nc->ntp_msg;

	// NTP時刻応答の場合
	if ((ntp->mode == NTP_MODE_SERVER) || (ntp->mode == NTP_MODE_BROADCAST)) {
		switch (nc->state) {
		case NTP_CLI_STATE_RESOLVE_ADDR:
		case NTP_CLI_STATE_ASYNC:
			break;
		case NTP_CLI_STATE_REQUEST:
			// サーバーからの応答時刻を取得
			memcpy(&nc->receive_timestamp, &ntp->receive_timestamp, sizeof(nc->receive_timestamp));
			memcpy(&nc->transmit_timestamp, &ntp->transmit_timestamp, sizeof(nc->transmit_timestamp));

			// 現在時刻取得
			ntp_cli_read_time(nc, &tp);

			// NTP参照時刻に現在時刻を設定
			set_systime_to_ntptime(&tp, &nc->recv_resp_timestamp);

			// 時刻を更新
			ntp_cli_update_time(nc);

			nc->state = NTP_CLI_STATE_SYNC;
			nc->poll = NTP_POLL_NORMAL;
			nc->timer = (1 << nc->poll) * 1000 * 1000;
			break;
		case NTP_CLI_STATE_SYNC:
			break;
		}
	}
	// NTP時刻要求の場合
	else if (ntp->mode == NTP_MODE_CLIENT) {
		// 現在時刻取得
		ntp_cli_read_time(nc, &tp);

		// NTP受信・送信時刻に現在時刻を設定
		set_systime_to_ntptime(&tp, &nc->receive_timestamp);
		set_systime_to_ntptime(&tp, &nc->transmit_timestamp);

		// NTPパケット送信
		ntp_cli_time_synchronization(nc, NTP_MODE_SERVER);
	}
}

void ntp_cli_update_time(T_NTP_CLI_CONTEXT *nc)
{
	struct timespec tp;
	T_NTP_TIMESTAMP dt1, dt2, ofs;

	sub_timestamp(&dt1, &nc->receive_timestamp, &nc->originate_timestamp);
	sub_timestamp(&dt2, &nc->recv_resp_timestamp, &nc->transmit_timestamp);
	sub_timestamp(&ofs, &dt1, &dt2);
	div2_timestamp(&ofs);

	// 差が大きい場合はサーバーの送信時間を設定
	if (ntohl(ofs.integer) > 2) {
		memcpy(&nc->reference_timestamp, &nc->transmit_timestamp, sizeof(nc->transmit_timestamp));
	}
	else {
		add_timestamp(&nc->reference_timestamp, &nc->recv_resp_timestamp, &ofs);
	}

	set_ntptime_to_systime(&tp, &nc->reference_timestamp);

	ntp_cli_write_time(nc, &tp);

	nc->buf[0] = '\0';
#ifdef __NEED_struct_timespec
	if (ctime_r(&tp.tv_sec, nc->buf) != NULL)
#else
	if (ctime_s(nc->buf, sizeof(nc->buf), &tp.tv_sec) == 0)
#endif
	{
		/* 改行コードの削除 */
		int len = strnlen(nc->buf, sizeof(nc->buf) - 1);
		nc->buf[len - 1] = '\0';
	}
	else {
		nc->buf[0] = '\0';
	}

	syslog(LOG_NOTICE, "[NTP CLI,%d] recv time: %s .%09u",
		nc->cepid, nc->buf, tp.tv_nsec);
}

int ntp_cli_get_timer(T_NTP_CLI_CONTEXT *nc)
{
	return nc->timer;
}

void ntp_cli_progress(T_NTP_CLI_CONTEXT *nc, TMO elapse)
{
	if (nc->timer <= 0)
		return;

	nc->timer -= elapse;
	if (nc->timer < 0)
		nc->timer = 0;
}

void ntp_cli_timeout(T_NTP_CLI_CONTEXT *nc)
{
	struct timespec tp;
	char *line;

	if (nc->timer != 0)
		return;

	switch (nc->state) {
	case NTP_CLI_STATE_ASYNC:
#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
		line = lookup_ipaddr(&nc->ipaddr6, NTP_SRV_URL, API_PROTO_IPV4);
		if (line == NULL || !in6_is_addr_ipv4mapped(&nc->ipaddr6)) {
			syslog(LOG_NOTICE, "[NTP CLI,%d] sleep %d.%03u[s], unknown host.",
				nc->cepid, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			nc->timer = SLP_ITV;
			break;
		}
		nc->snd_rmt.ipaddr = ntohl(nc->ipaddr6.s6_addr32[3]);
#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */
		if ((line = lookup_ipaddr(&nc->snd_rmt.ipaddr, NTP_SRV_URL, DEFAULT_API_PROTO)) == NULL) {
			syslog(LOG_NOTICE, "[NTP CLI,%d] sleep %d.%03u[s], unknown host.",
				nc->cepid, SLP_ITV / SYSTIM_HZ, SLP_ITV % SYSTIM_HZ);
			nc->timer = SLP_ITV;
			break;
		}
#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */
		nc->state = NTP_CLI_STATE_RESOLVE_ADDR;
		nc->timer = 1000;
		/* through */
	case NTP_CLI_STATE_RESOLVE_ADDR:
		// 現在時刻取得
		ntp_cli_read_time(nc, &tp);

		// NTP開始時刻に現在時刻を設定
		set_systime_to_ntptime(&tp, &nc->originate_timestamp);
		memset(&nc->receive_timestamp, 0, sizeof(nc->receive_timestamp));
		memset(&nc->transmit_timestamp, 0, sizeof(nc->transmit_timestamp));

		// NTPパケット送信
		ntp_cli_time_synchronization(nc, NTP_MODE_CLIENT);

		// NTP応答待ちに遷移、500msでリトライ
		nc->retry = 0;
		nc->state = NTP_CLI_STATE_REQUEST;
		nc->timer = 500 * 1000;
		break;
	case NTP_CLI_STATE_REQUEST:
		nc->retry++;
		if (nc->retry > 3) {
			nc->state = NTP_CLI_STATE_ASYNC;
			nc->poll = NTP_POLL_ASYNC;
			nc->timer = (1 << nc->poll) * 1000 * 1000;  // 1024秒後に時刻同期
			break;
		}
		// 現在時刻取得
		ntp_cli_read_time(nc, &tp);

		// NTP開始時刻に現在時刻を設定
		set_systime_to_ntptime(&tp, &nc->originate_timestamp);
		memset(&nc->receive_timestamp, 0, sizeof(nc->receive_timestamp));
		memset(&nc->transmit_timestamp, 0, sizeof(nc->transmit_timestamp));

		// NTPパケット送信
		ntp_cli_time_synchronization(nc, NTP_MODE_CLIENT);

		// NTP応答待ちに遷移、500msでリトライ
		nc->state = NTP_CLI_STATE_REQUEST;
		nc->timer = 500 * 1000;
		break;
	case NTP_CLI_STATE_SYNC:
		// 現在時刻取得
		ntp_cli_read_time(nc, &tp);

		// NTP開始時刻に現在時刻を設定
		set_systime_to_ntptime(&tp, &nc->originate_timestamp);
		memset(&nc->receive_timestamp, 0, sizeof(nc->receive_timestamp));
		memset(&nc->transmit_timestamp, 0, sizeof(nc->transmit_timestamp));

		// NTPパケット送信
		ntp_cli_time_synchronization(nc, NTP_MODE_CLIENT);

		// NTP応答待ちに遷移、500msでリトライ
		nc->state = NTP_CLI_STATE_REQUEST;
		nc->timer = 500 * 1000;
		break;
	}
}

ER ntp_cli_time_synchronization(T_NTP_CLI_CONTEXT *nc, ntp_mode_t mode)
{
	ER_UINT	len = sizeof(nc->ntp_msg);
	T_NTP_MSG *ntp = &nc->ntp_msg;
	ER error;

	memset(ntp, 0, len);

	ntp->leap_indicator = 0;
	ntp->version_number = 4;
	ntp->mode = mode;
	ntp->stratum = (mode == NTP_MODE_CLIENT) ? 2 : 1;
	ntp->poll = nc->poll;
	ntp->precision = 0xfa;   // 精度は10^-6(1μs)
	ntp->root_delay = htonl(0);
	ntp->root_dispersion = htonl(0);
	// 同期のための外部的な手段のないサブネット
	ntp->reference_identifier[0] = 'L';
	ntp->reference_identifier[1] = 'O';
	ntp->reference_identifier[2] = 'C';
	ntp->reference_identifier[3] = 'L';
	// 参照タイムスタンプ
	memcpy(&ntp->reference_timestamp, &nc->reference_timestamp, sizeof(ntp->reference_timestamp));
	// 開始タイムスタンプ
	memcpy(&ntp->originate_timestamp, &nc->originate_timestamp, sizeof(ntp->originate_timestamp));
	// 受信タイムスタンプ
	memcpy(&ntp->receive_timestamp, &nc->receive_timestamp, sizeof(ntp->receive_timestamp));
	// 送信タイムスタンプ
	memcpy(&ntp->transmit_timestamp, &nc->transmit_timestamp, sizeof(ntp->transmit_timestamp));
	// 鍵識別子
	ntp->key_identifier[0] = 0x08;
	ntp->key_identifier[1] = 0x18;
	ntp->key_identifier[2] = 0x00;
	ntp->key_identifier[3] = 0x00;

	if ((error = udp_snd_dat(nc->cepid, &nc->snd_rmt, ntp, len, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[NTP CLI,%d] udp_snd_dat error: %s",
			nc->cepid, itron_strerror(error));
		return error;
	}
	else
		syslog(LOG_DEBUG, "[NTP CLI,%d] udp_snd_dat: to: %s.%d",
			nc->cepid, ip2str(NULL, &nc->snd_rmt.ipaddr), nc->snd_rmt.portno);

	return E_OK;
}

void ntp_cli_read_time(T_NTP_CLI_CONTEXT *nc, struct timespec *tp)
{
	SYSTIM time;
	ER ret;

	ret = get_tim(&time);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "[NTP CLI,%d] get_tim error: %s",
			nc->cepid, itron_strerror(ret));
		tp->tv_sec = 0;
		tp->tv_nsec = 0;
		return;
	}

	tp->tv_sec = time / 1000000ll;
	tp->tv_nsec = (time % 1000000ll) * 1000ll;
}

void ntp_cli_write_time(T_NTP_CLI_CONTEXT *nc, struct timespec *tp)
{
	SYSTIM time;
	ER ret;

	time = (tp->tv_sec * 1000000ll) + (tp->tv_nsec / 1000ll);

	ret = set_tim(time);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "[NTP CLI,%d] set_tim error: %s",
			nc->cepid, itron_strerror(ret));
	}
}

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_ntp_cli (ID cepid, FN fncd, void *p_parblk)
{
	T_NTP_CLI_CONTEXT *nc = &ntp_cli;
	ER_UINT len;

	len = *(ER_UINT*)p_parblk;
	if (len < 0 && len != E_RLWAI) {
		/* E_RLWAI 以外で、0 以下の場合は、エラーを意味している。*/
		syslog(LOG_NOTICE, "[NTP CLI,%d] callback error: %s, fncd: %s", nc->cepid,
			itron_strerror(len), in_strtfn(fncd));
	}
	else {
		if (fncd == TEV_UDP_RCV_DAT) {
			if ((len = udp_rcv_dat(nc->cepid, &nc->rcv_rmt, &nc->ntp_msg, len, TMO_POL)) < 0)
			{
				syslog(LOG_NOTICE, "[NTP CLI,%d] udp_rcv_dat error: %s", nc->cepid,
					itron_strerror(len));
			}
			else
				syslog(LOG_DEBUG, "[NTP CLI,%d] udp_rcv_dat len: %d", nc->cepid, len);

			if (len >= sizeof(nc->ntp_msg))
				nc->rcv_flag = len;
			else
				nc->rcv_flag = 0;
		}
		syscall(wup_tsk(nc->tskid));
	}

	return E_OK;
}

#ifdef NTP_CLI_TASK

/*
 *  NTP クライアント送信タスク
 */

void
ntp_cli_task (intptr_t exinf)
{
	T_NTP_CLI_CONTEXT *nc = &ntp_cli;
	ER		error, ret;
	SYSTIM		prev, time;
	int timer;

	get_tid(&nc->tskid);
	syslog(LOG_NOTICE, "[NTP CLI:%d,%d] started.", nc->tskid, (ID)exinf);

	/* 初期化 */
	ntp_cli_initialize(nc, (ID)exinf);

	ret = get_tim(&time);
	if (ret != E_OK) {
		syslog(LOG_NOTICE, "[NTP CLI,%d] get_tim error: %7lu,%s",
			nc->cepid, time / SYSTIM_HZ, itron_strerror(ret));
		return;
	}

	while (true) {
		prev = time;

		/* タイマー取得 */
		timer = ntp_cli_get_timer(nc);

		/* 待ち */
		error = tslp_tsk(timer);
		if ((error != E_OK) && (error != E_TMOUT)) {
			syslog(LOG_NOTICE, "[NTP CLI,%d] tslp_tsk error: %s %d",
				nc->cepid, itron_strerror(error), timer);
			break;
		}

		ret = get_tim(&time);
		if (ret != E_OK) {
			syslog(LOG_NOTICE, "[NTP CLI,%d] get_tim error: %s",
				nc->cepid, itron_strerror(ret));
			break;
		}

		/* 時間経過 */
		ntp_cli_progress(nc, time - prev);

		/* 起こされた場合 */
		if (error == E_OK) {
			ntp_cli_wakeup(nc);
		}

		/* タイムアウト処理 */
		ntp_cli_timeout(nc);
	}
}

#endif
