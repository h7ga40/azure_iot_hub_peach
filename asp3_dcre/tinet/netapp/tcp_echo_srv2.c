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
 *  TCP ECHO サーバ #2
 *
 *    ・送受信タスク分離型
 *    ・ノンブロッキングコール
 *    ・IPv4/IPv6
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net_timer.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/tcp_echo_srv2.h>

#ifdef USE_TCP_ECHO_SRV2

/*
 *  表示
 */

/*#define SHOW_RCV_RANGE*/

/*
 *  コネクション切断方法の指定
 */

/*#define USE_TCP_SHT_CEP*/

/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t tcp_echo_srv_swbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t tcp_echo_srv_rwbuf[NUM_TCP_ECHO_SRV_TASKS][TCP_ECHO_SRV_RWBUF_SIZE];
#endif

/*
 *  変数
 */

/*
 *  注意:
 *
 *    BUF_SIZE は TCP の
 *    送信ウインドウバッファサイズ + 受信ウインドウバッファサイズの 
 *    3/2 倍以上の大きさがなければ、デッドロックする可能性がある。
 */

#define BUF_SIZE	((TCP_ECHO_SRV_SWBUF_SIZE + \
                          TCP_ECHO_SRV_RWBUF_SIZE) * 3 / 2)

static char	buffer[BUF_SIZE];	/* バッファ		*/

static T_IPEP	dst;			/* 接続相手		*/
static bool_t	ena_rcv;		/* 受信可能フラグ	*/
static char	*rptr = buffer;		/* 受信ポインタ		*/
static char	*sptr = buffer;		/* 送信ポインタ		*/
static uint32_t	rcv_total;		/* 受信文字数		*/
static uint32_t	snd_total;		/* 送信文字数		*/
static uint16_t	chars;			/* バッファ内の文字数	*/
static uint16_t	rcv_count;		/* 受信回数		*/
static uint16_t	snd_count;		/* 送信回数		*/

#ifdef USE_TCP_NON_BLOCKING

static ER	rcv_error = E_OK;	/* 受信、ノンブロッキングコールエラー */
static ER	snd_error = 1;		/* 送信、ノンブロッキングコールエラー */

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_tcp_echo_srv (ID cepid, FN fncd, void *p_parblk)
{
	ER		error = E_OK;
	ER_UINT		slen, rlen;
	SYSTIM		now;
	uint16_t	last;
	char		head, tail;

	get_tim(&now);

	switch (fncd) {

	case TFN_TCP_ACP_CEP:
		rcv_error = *(ER*)p_parblk;
		if (rcv_error == E_OK) {
			sptr = rptr = buffer;
			snd_count = snd_total = rcv_count = rcv_total = chars = 0;
			snd_error = 1;
			ena_rcv   = true;
			syslog(LOG_NOTICE, "[TES:%02u CBN] connect: %6lu, from: %s.%d",
			                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
			}
		else
			syslog(LOG_NOTICE, "[TES:%02u CBN] accept error: %s\n", cepid, itron_strerror(rcv_error));

		syscall(sig_sem(SEM_TCP_ECHO_SRV_RCV_READY));
		break;

	case TFN_TCP_SND_DAT:
		slen = snd_error = *(ER*)p_parblk;
		if (slen > 0) {
			head = *sptr;
			tail = *(sptr + slen - 1);
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02u CBN] send len: %4u, data %02x -> %02x",
			                   cepid, (uint16_t)slen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
			snd_count ++;
			snd_total += slen;

			syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));

			/* 送信ポインタを進める。*/
			if (sptr - buffer + slen >= sizeof(buffer))
				sptr -= sizeof(buffer) - slen;
			else
				sptr += slen;

			/* バッファ内の文字数を減らす。*/
			last   = chars;
			chars -= slen;

 			/* バッファ内に文字があれば、送信タスクを起床する。*/
			if (chars > 0 || (!ena_rcv && chars == 0))
				sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

 			/* 受信可能で、バッファ内に空きができれば、受信タスクを起床する。*/
			if (ena_rcv && last == sizeof(buffer) && sizeof(buffer) - chars > 0)
				syscall(sig_sem(SEM_TCP_ECHO_SRV_RCV_READY));

			syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));
			}
		else {
			syslog(LOG_NOTICE, "[TES:%02u CBN] send error = %s", cepid, itron_strerror(snd_error));
			sig_sem(SEM_TCP_ECHO_SRV_SND_READY);
			}
		break;

	case TFN_TCP_RCV_DAT:
		rlen = rcv_error = *(ER*)p_parblk;
		if (rlen > 0) {
			head = *rptr;
			tail = *(rptr + rlen - 1);
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02u CBN] recv len: %4u, data %02x -> %02x",
			                   cepid, (uint16_t)rlen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
			rcv_count ++;
			rcv_total += rlen;

			syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));

			/* 受信ポインタを進める。*/
			if (rptr - buffer + rlen >= sizeof(buffer))
				rptr -= sizeof(buffer) - rlen;
			else
				rptr += rlen;

			/* バッファ内の文字数を増やす。*/
			last   = chars;
			chars += rlen;

 			/* バッファ内に文字が入れば、送信タスクを起床する。*/
			if (last == 0 && chars > 0)
				sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

 			/* バッファ内に空きがあれば、受信タスクを起床する。*/
			if (sizeof(buffer) - chars > 0)
				sig_sem(SEM_TCP_ECHO_SRV_RCV_READY);

			syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));
			}
		else {
			if (rlen < 0)
				syslog(LOG_NOTICE, "[TES:%02u CBN] recv error = %s", cepid, itron_strerror(rcv_error));
			sig_sem(SEM_TCP_ECHO_SRV_RCV_READY);
			}
		break;

	case TFN_TCP_CLS_CEP:
		snd_error = *(ER*)p_parblk;
		if (snd_error != E_OK)
			syslog(LOG_NOTICE, "[TES:%02u CBN] close error = %s", cepid, itron_strerror(snd_error));

		syslog(LOG_NOTICE, "[TES:%02u CBN] finish:  %6lu, snd: %4u, rcv: %4u, len: %ld",
		                   cepid, now / SYSTIM_HZ, snd_count, rcv_count, snd_total);

		sig_sem(SEM_TCP_ECHO_SRV_CLS_READY);
		break;

	case TFN_TCP_CON_CEP:
	case TFN_TCP_GET_BUF:
	case TFN_TCP_RCV_BUF:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		}
	return error;
	}

/*
 *  TCP ECHO サーバ受信ルーチン (ノンブロッキングコール使用)
 */

ER
tcp_echo_srv_rcv (ID cepid, ID repid)
{
	ER		error;
	uint16_t	blen;

	if ((error = psnd_dtq(DTQ_TCP_ECHO_SRV_SND, (intptr_t)cepid)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02u RCV] sync error: %s", cepid, itron_strerror(error));
		return error;
		}

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_NBLK)) != E_WBLK) {
		syslog(LOG_NOTICE, "[TES:%02u ACP] accept error = %s", cepid, itron_strerror(error));
		return error;
		}

	/* コネクション開設完了まで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_RCV_READY));

	/* 待っている間にエラーになったら、コネクションの開設待ちに戻る。*/
	if (rcv_error != E_OK)
		return rcv_error;

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02u DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	while (true) {

		/* 受信サイズを計算する。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));
		if (sptr > rptr)
			blen = sptr - rptr;
		else
			blen = sizeof(buffer) - (rptr - buffer);
		syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));

		/* 受信する。*/
		if ((error = tcp_rcv_dat(cepid, rptr, blen, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES:%02u RCV] recv error = %s", cepid, itron_strerror(error));
			break;
			}

		/* 受信バッファに空ができるまで待つ。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_RCV_READY));

		/* 待っている間に、受信文字数が 0 になるか、エラーになったら終了する。*/
		if (rcv_error <= 0)
			break;

		}

	/* 受信完了 */
	ena_rcv = false;

	/* 受信バッファに文字が入るのを待っている送信タスクを起床する。*/
	if (chars == 0)
		sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

	/* コネクション切断完了まで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_CLS_READY));

	return error == E_WBLK ? E_OK : error;
	}

/*
 *  TCP ECHO サーバ送信タスク (ノンブロッキングコール使用)
 */

void
tcp_echo_srv_snd_task(intptr_t exinf)
{
	ER		error;
	ID		tskid, cepid;
	uint16_t	blen;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO SRV (NBLK) SND:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_SRV_SND, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TES:%02u SND] sync error: %s",
			                   cepid, itron_strerror(error));
			continue;
			}

		while (true) {

			/* バッファに文字が入るまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_SND_READY));

			/* 待っている間にエラーになったら終了する。*/
			if (snd_error <= 0)
				break;

			/* 受信完了で、バッファに文字がなければ終了する。*/
			if (!ena_rcv && chars == 0)
				break;

			/* 送信サイズを計算する。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));
			if (rptr > sptr)
				blen = rptr - sptr;
			else
				blen = sizeof(buffer) - (sptr - buffer);
			syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));

			/* 送信する。*/
			if ((error = tcp_snd_dat(cepid, sptr, blen, TMO_NBLK)) != E_WBLK) {
				syslog(LOG_NOTICE, "[TES:%02u SND] send error = %s", cepid, itron_strerror(error));
				break;
				}
			}

		/* 受信バッファに空ができるを待っている受信タスクを起床する。*/
		if (chars == sizeof(buffer))
			sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

#ifdef USE_TCP_SHT_CEP
		if ((error = tcp_sht_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TES:%02u SND] shutdown error = %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

		if ((error = tcp_cls_cep(cepid, TMO_NBLK)) != E_WBLK) {
			syslog(LOG_NOTICE, "[TES:%02u SND] close error = %s", cepid, itron_strerror(error));
			}

		pol_sem(SEM_TCP_ECHO_SRV_SND_READY);
		pol_sem(SEM_TCP_ECHO_SRV_RCV_READY);
		}
	}

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

/*
 *  TCP ECHO サーバ受信ルーチン
 */

ER
tcp_echo_srv_rcv (ID cepid, ID repid)
{
	ER_UINT		rlen;
	ER		error;
	SYSTIM		now;
	uint16_t	blen, last;
	char		head, tail;

	if ((error = psnd_dtq(DTQ_TCP_ECHO_SRV_SND, (intptr_t)cepid)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02u RCV] sync error: %s", cepid, itron_strerror(error));
		return error;
		}

	if ((error = TCP_ACP_CEP(cepid, repid, &dst, TMO_FEVR)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02u ACP] accept error = %s", cepid, itron_strerror(error));
		return error;
		}

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(repid, true)) != E_OK) {
		syslog(LOG_NOTICE, "[TES:%02u DEL] REP delete error: %s", cepid, itron_strerror(error));
		return error;
		}
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	syscall(get_tim(&now));
	syslog(LOG_NOTICE, "[TES:%02u ACP] connect: %6lu, from: %s.%d",
	                   cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
	sptr = rptr = buffer;
	snd_count = snd_total = rcv_count = rcv_total = chars = 0;
	ena_rcv   = true;
	while (true) {

		/* 受信サイズを計算する。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));
		if (sptr > rptr)
			blen = sptr - rptr;
		else
			blen = sizeof(buffer) - (rptr - buffer);
		syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));

		/* 受信する。*/
		if ((rlen = tcp_rcv_dat(cepid, rptr, blen, TMO_FEVR)) <= 0) {
			if (rlen < 0)
				syslog(LOG_NOTICE, "[TES:%02u RCV] recv error = %s",
				                   cepid, itron_strerror(rlen));
			break;
			}
		head = *rptr;
		tail = *(rptr + rlen - 1);
#ifdef SHOW_RCV_RANGE
		syslog(LOG_NOTICE, "[TES:%02u RCV] len: %4u, data %02x -> %02x",
		                   cepid, (uint16_t)rlen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
		rcv_count ++;
		rcv_total += rlen;

		syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));

		/* 受信ポインタを進める。*/
		if (rptr - buffer + rlen >= sizeof(buffer))
			rptr -= sizeof(buffer) - rlen;
		else
			rptr += rlen;

		/* バッファ内の文字数を増やす。*/
		last   =     chars;
		chars += (uint16_t)rlen;

 				/* バッファ内に文字が入れば、送信タスクを起床する。*/
		if (last == 0 && chars > 0)
			sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

 				/* バッファ内に空きがあれば、受信タスクを起床する。*/
		if (sizeof(buffer) - chars > 0)
			sig_sem(SEM_TCP_ECHO_SRV_RCV_READY);

		syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));

		/* バッファに空きができるまで待つ。*/
		syscall(wai_sem(SEM_TCP_ECHO_SRV_RCV_READY));
		}

	/* 受信完了 */
	ena_rcv = false;

	/* 受信バッファに文字が入るのを待っている送信タスクを起床する。*/
	if (chars == 0)
		sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

	/* コネクションの切断が完了するまで待つ。*/
	syscall(wai_sem(SEM_TCP_ECHO_SRV_CLS_READY));

	return error;
	}

/*
 *  TCP ECHO サーバ送信タスク
 */

void
tcp_echo_srv_snd_task(intptr_t exinf)
{
	ID		tskid, cepid;
	ER		error;
	ER_UINT		slen;
	SYSTIM		now;
	uint16_t	blen, last;
	char		head, tail;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO SRV SND:%d] started.", tskid);
	while (true) {
		if ((error = rcv_dtq(DTQ_TCP_ECHO_SRV_SND, (intptr_t*)&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TES:%02u SND] sync error: %s",
			                   cepid, itron_strerror(error));
			continue;
			}

		while (true) {

			/* バッファに文字が入るまで待つ。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_SND_READY));

			/* 受信完了で、バッファに文字がなければ終了する。*/
			if (!ena_rcv && chars == 0)
				break;

			/* 送信サイズを計算する。*/
			syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));
			if (rptr > sptr)
				blen = rptr - sptr;
			else
				blen = sizeof(buffer) - (sptr - buffer);
			syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));

			if ((slen = tcp_snd_dat(cepid, sptr, blen, TMO_FEVR)) <= 0) {
				syslog(LOG_NOTICE, "[TES:%02u SND] send error = %s", cepid, itron_strerror(slen));
				break;
				}

			head = *sptr;
			tail = *(sptr + slen - 1);
#ifdef SHOW_RCV_RANGE
			syslog(LOG_NOTICE, "[TES:%02u SND] len: %4u, data %02x -> %02x",
			                   cepid, (uint16_t)slen, head, tail);
#endif	/* of #ifdef SHOW_RCV_RANGE */
			snd_count ++;
			snd_total += slen;

			syscall(wai_sem(SEM_TCP_ECHO_SRV_LOCK));

			/* 送信ポインタを進める。*/
			if (sptr - buffer + slen >= sizeof(buffer))
				sptr -= sizeof(buffer) - slen;
			else
				sptr += slen;

			/* バッファ内の文字数を減らす。*/
			last   =     chars;
			chars -= (uint16_t)slen;

			/* 受信完了で、バッファに文字がなければ終了する。*/
			if (!ena_rcv && chars == 0) {
				syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));
				break;
				}

 			/* バッファ内に文字があれば、送信タスクを起床する。*/
			if (chars > 0)
				sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

 			/* 受信可能で、バッファ内に空きができれば、受信タスクを起床する。*/
			if (ena_rcv && last == sizeof(buffer) && sizeof(buffer) - chars > 0)
				sig_sem(SEM_TCP_ECHO_SRV_RCV_READY);

			syscall(sig_sem(SEM_TCP_ECHO_SRV_LOCK));
			}

		/* 受信バッファに空ができるを待っている受信タスクを起床する。*/
		if (chars == sizeof(buffer))
			sig_sem(SEM_TCP_ECHO_SRV_SND_READY);

#ifdef USE_TCP_SHT_CEP
		if ((error = tcp_sht_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TES:%02u SND] shtudown error = %s", cepid, itron_strerror(error));
#endif	/* of #ifdef USE_TCP_SHT_CEP */

		if ((error = tcp_cls_cep(cepid, TMO_FEVR)) != E_OK)
			syslog(LOG_NOTICE, "[TES:%02u SND] close error = %s", cepid, itron_strerror(error));

		get_tim(&now);
		syslog(LOG_NOTICE, "[TES:%02u SND] finish:  %6lu, snd: %4u, rcv: %4u, len: %ld",
		                   cepid, now / SYSTIM_HZ, snd_count, rcv_count, snd_total);

		sig_sem(SEM_TCP_ECHO_SRV_CLS_READY);

		pol_sem(SEM_TCP_ECHO_SRV_SND_READY);
		pol_sem(SEM_TCP_ECHO_SRV_RCV_READY);
		}
	}

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifdef USE_TCP_EXTENTIONS

/*
 *  get_tcp_rep -- TCP 受付口を獲得する。
 */

static ER
get_tcp_rep (ID *repid)
{
	ID		tskid;
	T_TCP_CREP	crep;

	get_tid(&tskid);

	crep.repatr = UINT_C(0);
	crep.myaddr.portno = UINT_C(7);

#if defined(SUPPORT_INET4)
	crep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#if defined(SUPPORT_INET6)
	memcpy(&crep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
#endif

	return ALLOC_TCP_REP(repid, tskid, &crep);
	}

/*
 *  get_tcp_cep -- TCP 通信端点を獲得する。
 */

static ER
get_tcp_cep (ID *cepid)
{
	ID		tskid;
	T_TCP_CCEP	ccep;

	get_tid(&tskid);

	ccep.cepatr = UINT_C(0);
	ccep.sbufsz = TCP_ECHO_SRV_SWBUF_SIZE;
	ccep.rbufsz = TCP_ECHO_SRV_RWBUF_SIZE;

#ifdef TCP_CFG_SWBUF_CSAVE
	ccep.sbuf = NADR;
#else
	ccep.sbuf = tcp_echo_srv_swbuf;
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = tcp_echo_srv_rwbuf;
#endif
#ifdef USE_TCP_NON_BLOCKING
	ccep.callback = (FP)callback_nblk_tcp_echo_srv;
#else
	ccep.callback = NULL;
#endif

	return ALLOC_TCP_CEP(cepid, tskid, &ccep);
	}

/*
 *  TCP ECHO サーバ受信タスク
 */

void
tcp_echo_srv_rcv_task (intptr_t exinf)
{
	ID	tskid, cepid, repid;
	ER	error = E_OK;

	syscall(get_tid(&tskid));
	syslog(LOG_NOTICE, "[TCP ECHO SRV:%d] started.", tskid);
	while (true) {

		syscall(slp_tsk());
		if ((error = get_tcp_cep (&cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[TES:00 EXT] CEP create error: %s", itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep (&repid)) != E_OK) {
				syslog(LOG_NOTICE, "[TES:00 EXT] REP create error: %s", itron_strerror(error));
				break;
				}
			else if ((error = tcp_echo_srv_rcv(cepid, repid)) != E_OK) {
				error = FREE_TCP_REP(repid, error != E_DLT);
				break;
				}
			}

		if ((error = free_tcp_cep(cepid)) != E_OK)
			syslog(LOG_NOTICE, "[TES:%02u EXT] CEP delete error: %s", cepid, itron_strerror(error));

		}
	}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

void
tcp_echo_srv_rcv_task(intptr_t exinf)
{
	ID	tskid;

	get_tid(&tskid);
	syslog(LOG_NOTICE, "[TCP ECHO SRV RCV:%d,%d] started.", tskid, (ID)exinf);
	while (true) {
		while (tcp_echo_srv_rcv((ID)exinf, TCP_ECHO_SRV_REPID) == E_OK)
			;
		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_TCP_ECHO_SRV2 */
