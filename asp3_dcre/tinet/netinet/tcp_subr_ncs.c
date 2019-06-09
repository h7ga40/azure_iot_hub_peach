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
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
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
 *	@(#)tcp_subr.c	8.2 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_subr.c,v 1.49.2.4 1999/08/29 16:29:55 peter Exp $
 */

#include <string.h>

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
#include <net/if_arp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_var.h>
#include <net/net_buf.h>
#include <net/net_timer.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_itron.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>

#ifdef SUPPORT_TCP

#ifndef TCP_CFG_SWBUF_CSAVE_ONLY

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能が無効
 */

/*
 *  tcp_read_swbuf -- 送信ウィンドバッファからデータを読み出す。
 */

void
tcp_read_swbuf_ncs (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff)
{
	uint8_t	*wptr, *rptr;
	uint_t	sdu_size   = GET_IP_SDU_SIZE(output);
	uint_t	hdr_offset = IF_IP_TCP_HDR_OFFSET(output);

	/* SDU の大きさをチェックする。*/
	if (sdu_size < GET_TCP_HDR_SIZE(output, hdr_offset) + len) {
		syslog(LOG_INFO, "[TCP] shrink SUD len: %d -> %d",
		       (uint16_t)len, (uint16_t)(sdu_size - GET_TCP_HDR_SIZE(output, hdr_offset)));
		len = sdu_size - GET_TCP_HDR_SIZE(output, hdr_offset);
		}

	wptr = GET_TCP_SDU(output, hdr_offset);

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	rptr = cep->sbuf_rptr + doff;
	if (rptr - (uint8_t*)cep->sbuf > cep->sbufsz) {

		/* 転送開始が送信ウィンドバッファの終わりを超えるときの処理　*/
		rptr = (uint8_t*)cep->sbuf_rptr - (cep->sbufsz - doff);
		}
	else if (len + (rptr - (uint8_t*)cep->sbuf) > cep->sbufsz) {
		uint_t sub;

		/* 転送範囲がラウンドするときの処理 */
		sub = (uint_t)(cep->sbufsz - (rptr - (uint8_t*)cep->sbuf));
		memcpy((void*)wptr, rptr, (size_t)sub);
		len  -= sub;
		wptr += sub;
		rptr  = (uint8_t*)cep->sbuf;
		}
	memcpy((void*)wptr, rptr, (size_t)len);

	NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_DATA_SEGS],   1);
	NET_COUNT_TCP(net_count_tcp[NC_TCP_SEND_DATA_OCTETS], len);

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));
	}

/*
 *  tcp_drop_swbuf -- 送信ウィンドバッファから指定されたオクテット分削除する。
 */

void
tcp_drop_swbuf_ncs (T_TCP_CEP *cep, uint_t len)
{
	uint_t	last;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	last = cep->swbuf_count;

	if (cep->sbuf_rptr + len > (uint8_t*)cep->sbuf + cep->sbufsz)
		/* ラウンドするときの計算 */
		cep->sbuf_rptr -= cep->sbufsz - len;
	else
		cep->sbuf_rptr += len;
	cep->swbuf_count -= (uint16_t)len;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

#ifdef TCP_CFG_NON_BLOCKING

	if (cep->snd_nblk_tfn == TFN_TCP_GET_BUF) {	/* ノンブロッキングコール */

		/* 通信端点をロックする。*/
		syscall(wai_sem(cep->semid_lock));

		/* 送信データ長を計算する。*/
		if (cep->sbufsz - cep->swbuf_count > cep->sbufsz - (cep->sbuf_wptr - (uint8_t*)cep->sbuf))
			/* 書込みポインタがラウンドするときは、書込みポインタから送信ウィンドバッファの終わりまで */
			len = (uint_t)(cep->sbufsz - (cep->sbuf_wptr - (uint8_t*)cep->sbuf));
		else
			len = cep->sbufsz - cep->swbuf_count;

		/* 通信端点のロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		if (len > 0) {

			/* tcp_get_buf の割当て長を設定する。*/
			cep->get_buf_len = len;

			/* 送信ウィンドバッファのアドレスを返す。*/
			*cep->snd_p_buf = cep->sbuf_wptr;

			if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)(uint32_t)len);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&len);
#endif
			else
				syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));

			/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
			cep->snd_tskid = TA_NULL;
			cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
			}
		}

#ifdef TCP_CFG_EXTENTIONS

	else if (cep->snd_nblk_tfn == TFN_TCP_SND_DAT ||
	         cep->snd_nblk_tfn == TFN_TCP_SND_OOB) {	/* ノンブロッキングコール */

#else	/* of #ifdef TCP_CFG_EXTENTIONS */

	else if (cep->snd_nblk_tfn == TFN_TCP_SND_DAT) {	/* ノンブロッキングコール */

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

		ER_UINT	error;

	 	/* 送信ウィンドバッファにデータを書き込む。*/
		error = TCP_WRITE_SWBUF(cep, cep->snd_data, (uint_t)cep->snd_len);

#ifdef TCP_CFG_EXTENTIONS

		/* 送信緊急ポインタを設定する。*/
	        if (cep->snd_nblk_tfn == TFN_TCP_SND_OOB)
			cep->snd_up = cep->snd_una + error;

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

		if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
			(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)error);
#else
			(*cep->callback)(GET_TCP_CEPID(cep), cep->snd_nblk_tfn, (void*)&error);
#endif
		else
			syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));

		/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
		cep->snd_tskid = TA_NULL;
		cep->snd_tfn   = cep->snd_nblk_tfn = TFN_TCP_UNDEF;
		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/* 送信ウィンドバッファに空きが出来たら出力タスクを起床する。*/
		if (last == cep->sbufsz && cep->swbuf_count < cep->sbufsz)
			syscall(set_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY));

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	if (cep->swbuf_count > 0) {
		/* 送信ウィンドバッファにデータがあれば出力をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	}

/*
 *  tcp_write_swbuf -- 送信ウィンドバッファにデータを書き込む。
 */

ER_UINT
tcp_write_swbuf_ncs (T_TCP_CEP *cep, void *data, uint_t len)
{
	uint_t	offset;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* len と送信ウィンドバッファの空きの小さい方を移すデータ数にする。*/
	if (len > (cep->sbufsz - cep->swbuf_count))
		len = cep->sbufsz - cep->swbuf_count;

	/* 書き込みポインタがラウンドするときの処理 */
	if (len > cep->sbufsz - (cep->sbuf_wptr - (uint8_t*)cep->sbuf)) {
		offset = (uint_t)(cep->sbufsz - (cep->sbuf_wptr -(uint8_t*)cep->sbuf));
		memcpy(cep->sbuf_wptr, data, (size_t)offset);
		cep->sbuf_wptr  = (uint8_t*)cep->sbuf;
		cep->swbuf_count += offset;
		}
	else
		offset = 0;

	memcpy(cep->sbuf_wptr, (void*)((uint8_t*)data + offset), (size_t)(len - offset));
	cep->sbuf_wptr  += len - offset;
	cep->swbuf_count += len - offset;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return (ER_UINT)len;
	}

/*
 *  tcp_wait_swbuf -- 送信ウィンドバッファが空くのを待つ。
 */

ER
tcp_wait_swbuf_ncs (T_TCP_CEP *cep, TMO tmout)
{
	ER	error;
	FLGPTN	flag;

	while (cep->swbuf_count >= cep->sbufsz) {
		/*
		 *  送信ウィンドバッファの空きがなければ、出力をポストして、
		 *  送信ウィンドバッファが空くまで待つ。
		 */
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);

		if ((error = twai_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY, TWF_ORW, &flag, tmout)) != E_OK) {
			return error;
			}
		syscall(clr_flg(cep->snd_flgid, (FLGPTN)(~TCP_CEP_EVT_SWBUF_READY)));

		/*
		 *  送信できるか、CEP の FSM 状態を見る。
		 *  送信ウィンドバッファが空くまで待つ間に、送信不能になった場合は、
		 *  コネクションが切断されたことを意味している。
		 */
		if (!TCP_FSM_CAN_SEND_MORE(cep->fsm_state)) {
			return E_CLS;
			}
		}

	return E_OK;
	}
/*
 *  tcp_get_swbuf_addr -- 送信ウィンドバッファの空きアドレスを獲得する。
 */

ER_UINT
tcp_get_swbuf_addr_ncs (T_TCP_CEP *cep, void **p_buf)
{
	uint_t	len;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* 送信ウィンドバッファの空きアドレスの先頭を設定する。*/
	*p_buf = cep->sbuf_wptr;

	/* 送信ウィンドバッファの空きサイズを計算する。*/
	if (cep->sbufsz - cep->swbuf_count > cep->sbufsz - (cep->sbuf_wptr - (uint8_t*)cep->sbuf))
		/* 書込みポインタがラウンドするときは、書込みポインタから送信ウィンドバッファの終わりまで */
		len = (uint_t)(cep->sbufsz - (cep->sbuf_wptr - (uint8_t*)cep->sbuf));
	else
		len = cep->sbufsz - cep->swbuf_count;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* 送信ウィンドバッファの空きデータ長を設定し、その値を返す。*/
	cep->get_buf_len = len;
	return (ER_UINT)len;
	}

/*
 *  tcp_send_swbuf -- 送信ウィンドバッファのデータを送信可能にする。
 */

void
tcp_send_swbuf_ncs (T_TCP_CEP *cep, uint_t len)
{
	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	cep->sbuf_wptr  += len;
	cep->swbuf_count += len;

	/* 書込みポインタがラウンドするときの処理 */
	if (cep->sbuf_wptr >= (uint8_t*)cep->sbuf + cep->sbufsz)
		cep->sbuf_wptr  = (uint8_t*)cep->sbuf;

	/* tcp_get_buf の割当て長をリセットする。*/
	cep->get_buf_len = 0;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* 送信ウィンドバッファがフルのときは強制的に送信する。*/
	if (cep->swbuf_count >= cep->sbufsz)
		cep->flags |= TCP_CEP_FLG_FORCE | TCP_CEP_FLG_FORCE_CLEAR;
	}

/*
 *  tcp_is_swbuf_full -- 送信ウィンドバッファが満杯かチェックする。
 */

bool_t
tcp_is_swbuf_full_ncs (T_TCP_CEP *cep)
{
	return cep->swbuf_count >= cep->sbufsz;
	}

#endif	/* of #ifndef TCP_CFG_SWBUF_CSAVE_ONLY */

#ifndef TCP_CFG_RWBUF_CSAVE_ONLY

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能が無効
 */

/*
 *  tcp_drop_rwbuf -- 受信ウィンドバッファから指定されたオクテット分削除する。
 */

void
tcp_drop_rwbuf_ncs (T_TCP_CEP *cep, uint_t len)
{
	cep->rwbuf_count -= len;
	cep->rbuf_rptr  += len;

	/* 読み出しポインタがラウンドするときの処理 */
	if (cep->rbuf_rptr >= (uint8_t*)cep->rbuf + cep->rbufsz)
		cep->rbuf_rptr  = (uint8_t*)cep->rbuf;
	}

/*
 *  tcp_read_rwbuf -- 受信ウィンドバッファから指定されたオクテット分読み出す。
 */

uint_t
tcp_read_rwbuf_ncs (T_TCP_CEP *cep, void *data, uint_t len)
{
	uint_t	offset;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* len と受信ウィンドバッファのデータ数の小さい方を取り出すデータ数にする。*/
	if (len > cep->rwbuf_count)
		len = cep->rwbuf_count;

	/* 読み出しポインタがラウンドするときの処理 */
	if (len > cep->rbufsz - (cep->rbuf_rptr - (uint8_t*)cep->rbuf)) {
		offset = (uint_t)(cep->rbufsz - (cep->rbuf_rptr -(uint8_t*)cep->rbuf));
		memcpy(data, cep->rbuf_rptr, (size_t)offset);
		cep->rwbuf_count -= offset;
		cep->rbuf_rptr   = (uint8_t*)cep->rbuf;
		}
	else
		offset = 0;

	memcpy((void*)((uint8_t*)data + offset), cep->rbuf_rptr, (size_t)(len - offset));
	cep->rwbuf_count -= len - offset;
	cep->rbuf_rptr  += len - offset;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return len;
	}

/*
 *  tcp_get_rwbuf_addr -- 受信ウィンドバッファの空きアドレスを獲得する。
 */

uint_t
tcp_get_rwbuf_addr_ncs (T_TCP_CEP *cep, void **p_buf)
{
	uint_t	len;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/* 受信ウィンドバッファのデータ長を計算する。*/
	if (cep->rwbuf_count > cep->rbufsz - (cep->rbuf_rptr - (uint8_t*)cep->rbuf))
		/* ポインタがラウンドするときは受信ウィンドバッファの終わりまで */
		len = (uint_t)(cep->rbufsz - (cep->rbuf_rptr - (uint8_t*)cep->rbuf));
	else
		len = cep->rwbuf_count;

	/* 受信ウィンドバッファのデータのアドレスの先頭を設定する。*/
	*p_buf = cep->rbuf_rptr;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* 受信ウィンドバッファのデータ長を設定し、その値を返す。*/
	cep->rcv_buf_len = len;
	return len;
	}

/*
 *  tcp_write_rwbuf -- 受信ウィンドバッファにデータを書き込む。
 *
 *    注意:
 *	入力 input は、T_TCP_Q_HDR によりリンクされ、
 *	並べ替えが終了していなければならない。また、
 *	追加するデータは、受信ウィンドバッファに収まること。
 */

void
tcp_write_rwbuf_ncs (T_TCP_CEP *cep, T_NET_BUF *input, uint_t thoff)
{
	T_TCP_Q_HDR	*qhdr;
	uint_t		offset, inlen, last;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	qhdr  = (T_TCP_Q_HDR*)GET_TCP_HDR(input, thoff);

	/* 受信済みシーケンス番号を更新する。*/
	cep->rcv_nxt += qhdr->slen;

	last  = cep->rwbuf_count;
	inlen = qhdr->slen;

	/* 緊急データの SDU 補正を行う。*/
	if (qhdr->urp > 0 && inlen > 0) {
		inlen      -= qhdr->urp;
		qhdr->slen -= qhdr->urp;
		qhdr->urp   = 0;
		}

	/*
	 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
	 *  これは、アプリケーションに、相手からこれ以上データが送られて
	 *  こないことを知らせるためである。
	 */
	if (inlen > 0) {

		/* 書き込みポインタがラウンドするときの処理 */
		if (inlen > cep->rbufsz - (cep->rbuf_wptr - (uint8_t*)cep->rbuf)) {
			offset = (uint_t)(cep->rbufsz - (cep->rbuf_wptr - (uint8_t*)cep->rbuf));
			memcpy(cep->rbuf_wptr, (void*)(GET_TCP_SDU(input, thoff) + qhdr->soff), (size_t)offset);
			cep->rbuf_wptr   = (uint8_t*)cep->rbuf;
			cep->rwbuf_count += offset;
			inlen           -= offset;
			}
		else
			offset	= 0;

		memcpy(cep->rbuf_wptr, (void*)(GET_TCP_SDU(input, thoff) + qhdr->soff + offset), (size_t)inlen);
		cep->rbuf_wptr  += inlen;
		cep->rwbuf_count += inlen;

		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DATA_SEGS],   1);
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DATA_OCTETS], inlen);
		}

	syscall(rel_net_buf(input));

#ifdef TCP_CFG_NON_BLOCKING

	if (cep->rcv_nblk_tfn == TFN_TCP_RCV_BUF) {	/* ノンブロッキングコール */

		int_t	len;

		/* 受信ウィンドバッファの空きデータ長を計算する。*/
		if (cep->rwbuf_count > cep->rbufsz - (cep->rbuf_rptr - (uint8_t*)cep->rbuf)) {
			/* ポインタがラウンドするときは受信ウィンドバッファの終わりまで */
			len = (uint_t)(cep->rbufsz - (cep->rbuf_rptr - (uint8_t*)cep->rbuf));
			}
		else
			len = cep->rwbuf_count;

		/* 通信端点のロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		/*
		 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
		 *  これは、アプリケーションに、相手からこれ以上データが送られて
		 *  こないことを知らせるためである。
		 */
		if (len > 0 || inlen == 0) {

			/* tcp_rcv_buf の割当て長を設定する。*/
			cep->rcv_buf_len = len;

			/* 受信ウィンドバッファのアドレスを返す。*/
			*cep->rcv_p_buf = cep->rbuf_rptr;


			if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)(uint32_t)len);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
#endif
			else
				syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));

			if (len == 0) {

				/*
				 *  通信端点をロックして、
				 *  受信ウィンドバッファキューのネットワークバッファを解放する。
				 */
				syscall(wai_sem(cep->semid_lock));
				TCP_FREE_RWBUFQ(cep);
				syscall(sig_sem(cep->semid_lock));
				}
			}

		/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
		cep->rcv_tskid = TA_NULL;
		cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
		}

	else if (cep->rcv_nblk_tfn == TFN_TCP_RCV_DAT) {	/* ノンブロッキングコール */
		ER_UINT	len;

		/* 通信端点のロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		/* 受信ウィンドバッファからデータを取り出す。*/
		if ((len = TCP_READ_RWBUF(cep, cep->rcv_data, (uint_t)cep->rcv_len)) > 0) {
			/* 相手にウィンドウサイズが変わったことを知らせるため出力をポストする。*/
			cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
			sig_sem(SEM_TCP_POST_OUTPUT);
			}

		/*
		 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
		 *  これは、アプリケーションに、相手からこれ以上データが送られて
		 *  こないことを知らせるためである。
		 */
		if (len > 0 || inlen == 0) {

			if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)(uint32_t)len);
#else
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&len);
#endif
			else
				syslog(LOG_WARNING, "[TCP] no call back, CEP: %d.", GET_TCP_CEPID(cep));
			}

		/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
		cep->rcv_tskid = TA_NULL;
		cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;

		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/* 通信端点のロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		if (inlen == 0 && cep->rwbuf_count == 0) {
			/*
			 *  受信ウィンドバッファ内のデータ数が 0 で、
			 *  相手から FIN フラグが付いたセグメントを受信したときは、
			 *  通信端点をロックして、
			 *  受信ウィンドバッファキューのネットワークバッファを解放する。
			 */
			syscall(wai_sem(cep->semid_lock));
			TCP_FREE_RWBUFQ(cep);
			syscall(sig_sem(cep->semid_lock));
			}

		/*
		 *  受信ウィンドバッファにデータが入るか、 inlen == 0 の時、入力タスクを起床する。
		 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
		 *  これは、アプリケーションに、相手からこれ以上データが送られて
		 *  こないことを知らせるためである。
		 */
		if ((last == 0 && cep->rwbuf_count > 0) || inlen == 0)
			syscall(set_flg(cep->rcv_flgid, TCP_CEP_EVT_RWBUF_READY));

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	}

#endif	/* of #ifndef TCP_CFG_RWBUF_CSAVE_ONLY */

#endif	/* of #ifdef SUPPORT_TCP */
