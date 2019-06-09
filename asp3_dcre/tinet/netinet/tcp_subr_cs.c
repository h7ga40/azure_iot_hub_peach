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

#ifdef TCP_CFG_SWBUF_CSAVE

#ifdef TCP_CFG_SWBUF_CSAVE_ONLY

#else	/* of #ifdef TCP_CFG_SWBUF_CSAVE_ONLY */

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能が有効な場合
 *  ただし、通信端点の送信ウィンドバッファ（cep->sbuf）の値により、
 *  TCP 通信端点毎に送信ウィンドバッファの省コピー機能を使用するか、
 *  使用しないかを切り替える。
 */

/*
 *  tcp_drop_swbuf -- 送信ウィンドバッファから指定されたオクテット分削除する（選択）。
 */

void
tcp_drop_swbuf (T_TCP_CEP *cep, uint_t len)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		tcp_drop_swbuf_ncs(cep, len);
	else
		tcp_drop_swbuf_cs(cep, len);
	}

/*
 *  tcp_write_swbuf -- 送信ウィンドバッファにデータを書き込む（選択）。
 */

ER_UINT
tcp_write_swbuf (T_TCP_CEP *cep, void *data, uint_t len)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		return tcp_write_swbuf_ncs(cep, data, len);
	else
		return tcp_write_swbuf_cs(cep, data, len);
	}

/*
 *  tcp_read_swbuf -- 送信ウィンドバッファからデータを読み出す（選択）。
 */

void
tcp_read_swbuf (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		tcp_read_swbuf_ncs(cep, output, len, doff);
	}

/*
 *  tcp_wait_swbuf -- 送信ウィンドバッファと相手の受信ウィンドが開くのを待つ（選択）。
 */

ER
tcp_wait_swbuf (T_TCP_CEP *cep, TMO tmout)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		return tcp_wait_swbuf_ncs(cep, tmout);
	else
		return tcp_wait_swbuf_cs(cep, tmout);
	}

/*
 *  tcp_get_swbuf_addr -- 送信ウィンドバッファの空きアドレスを獲得する（選択）。
 */

ER_UINT
tcp_get_swbuf_addr (T_TCP_CEP *cep, void **p_buf)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		return tcp_get_swbuf_addr_ncs(cep, p_buf);
	else
		return tcp_get_swbuf_addr_cs(cep, p_buf);
	}

/*
 *  tcp_send_swbuf -- 送信ウィンドバッファのデータを送信可能にする（選択）。
 */

void
tcp_send_swbuf (T_TCP_CEP *cep, uint_t len)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		tcp_send_swbuf_ncs(cep, len);
	else
		tcp_send_swbuf_cs(cep, len);
	}

/*
 *  tcp_free_swbufq -- 送信ウィンドバッファキューの解放を指示する（選択）。
 */

void
tcp_free_swbufq (T_TCP_CEP *cep)
{
	if (!IS_PTR_DEFINED(cep->sbuf))
		tcp_free_swbufq_cs(cep);
	}

/*
 *  tcp_alloc_swbuf -- 送信ウィンドバッファ用のネットワークバッファの割り当てを指示する（選択）。
 */

void
tcp_alloc_swbuf (T_TCP_CEP *cep)
{
	if (!IS_PTR_DEFINED(cep->sbuf))
		tcp_alloc_swbuf_cs(cep);
	}

/*
 *  tcp_is_swbuf_full -- 送信ウィンドバッファが満杯かチェックする（選択）。
 */

bool_t
tcp_is_swbuf_full (T_TCP_CEP *cep)
{
	if (IS_PTR_DEFINED(cep->sbuf))
		return tcp_is_swbuf_full_ncs(cep);
	else
		return tcp_is_swbuf_full_cs(cep);
	}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE_ONLY */

/*
 *  tcp_drop_swbuf_cs -- 送信ウィンドバッファから指定されたオクテット分削除する（専用）。
 */

void
tcp_drop_swbuf_cs (T_TCP_CEP *cep, uint_t len)
{

	/*送信ウィンドバッファから指定されたオクテット分削除する。*/
	cep->swbuf_count -= (uint16_t)len;

	/* 送信済みで、ACK待ちの時は、ACK完了に変更する。*/
	if (len > 0 && (cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_SENT) {
		cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_ACKED;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	}

/*
 *  tcp_write_swbuf_cs -- 送信ウィンドバッファにデータを書き込む（専用）。
 */

ER_UINT
tcp_write_swbuf_cs (T_TCP_CEP *cep, void *data, uint_t len)
{
	ER_UINT	error;
	uint_t	hdr_size = IF_IP_TCP_NET_HDR_SIZE(&cep->dstaddr.ipaddr);

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if ((error = net_buf_siz(cep->swbufq)) > 0 && error >= hdr_size) {

		/*
		 *  +-----------+--------+---------+---------+
		 *  | Ehter HDR | IP HDR | TCP HDR | TCP SDU |
		 *  +-----------+--------+---------+---------+
		 *        14        20        20        n
		 *   <---------------- error --------------->
		 *              ^
		 *              net_buf で 4 オクテット境界にアラインされている。
		 *
		 *  tcp_output と ip_output で、チェックサムを計算するとき、
		 *  n が 4 オクテット境界になるように SDU の後ろに 0 を
		 *  パッディングする。その分を考慮して送信ウィンドバッファの
		 *  空きサイズを 4 オクテット境界に調整する。
		 */
		error = (uint_t)(((error - hdr_size) >> 2 << 2) + hdr_size);

		if (len > cep->sbufsz - cep->swbuf_count)
			len = (uint_t)(cep->sbufsz - cep->swbuf_count);
		if (len > (error - hdr_size))
			len = (uint_t)(error - hdr_size);

		/* 送信ウインドサイズによりサイズを調整する。*/
		if (len > cep->snd_wnd) 
			len = cep->snd_wnd;
		if (len > cep->snd_cwnd)
			len = cep->snd_cwnd;

		/* 相手の最大受信セグメントサイズ (maxseg) を超えないようにする。*/
		if (len > cep->maxseg) 
			len = cep->maxseg;

		/* データを移す。*/
		memcpy(cep->sbuf_wptr, (void*)((uint8_t*)data), (size_t)len);
		cep->sbuf_wptr   += len;
		cep->swbuf_count += len;
		error             = len;

		/* ネットワークバッファ長と IP データグラム長を設定する。*/
		cep->swbufq->len = (uint16_t)(cep->swbuf_count + hdr_size);
		SET_IP_SDU_SIZE(cep->swbufq, cep->swbuf_count + TCP_HDR_SIZE);

		/* フラグを、送信可能に設定する。*/
		cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_SEND_READY;
		}

	else {	/* 送信ウインドバッファが不正 */
		syslog(LOG_WARNING, "[TCP] illegal window buff for send, CEP: %d, %4d < %4d.",
		                    GET_TCP_CEPID(cep), error, hdr_size);

		/* 送信ウィンドバッファキューのネットワークバッファを解放する。*/
		tcp_free_swbufq_cs(cep);
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

/*
 *  tcp_wait_swbuf_cs -- 送信ウィンドバッファと相手の受信ウィンドが開くのを待つ（専用）。
 */

ER
tcp_wait_swbuf_cs (T_TCP_CEP *cep, TMO tmout)
{
	FLGPTN	flag;
	SYSTIM	before, after;
	ER	error;
	int_t	win;

	/* 送信ウィンドバッファが割当て済みで、空きがあれば終了する。*/
	if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_NBUF_READY &&
	    (cep->swbuf_count + IF_IP_TCP_NET_HDR_SIZE(&cep->dstaddr.ipaddr)) < net_buf_siz(cep->swbufq))
		return E_OK;

	/* 送信中であれば、終了するまで待機する。*/
	while ((cep->flags & TCP_CEP_FLG_WBCS_MASK) != TCP_CEP_FLG_WBCS_FREE) {

		/* 送信待ちの時間を tmout から減ずる。*/
		if (!(tmout == TMO_POL || tmout == TMO_FEVR))
			syscall(get_tim(&before));

		if ((error = twai_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY,
		                      TWF_ORW, &flag, tmout)) != E_OK)
			return error;
		syscall(clr_flg(cep->snd_flgid, (FLGPTN)(~TCP_CEP_EVT_SWBUF_READY)));

		/* 送信待ちの時間を tmout から減ずる。*/
		if (!(tmout == TMO_POL || tmout == TMO_FEVR)) {
			syscall(get_tim(&after));
			if (after - before > tmout)
				return E_TMOUT;
			tmout -= (TMO)(after - before);
			}

		/*
		 *  送信できるか、CEP の FSM 状態を見る。
		 *  送信ウインドバッファが空くまで待つ間に、送信不能になった場合は、
		 *  コネクションが切断されたことを意味している。
		 */
		if (!TCP_FSM_CAN_SEND_MORE(cep->fsm_state))
			return E_CLS;
		}

	/* 相手の受信ウィンドが閉じている場合は、開くまで待機する。*/
	while (true) {

		/*
		 *  snd_wnd:  相手の受信可能ウィンドサイズ
		 *  snd_cwnd: 輻輳ウィンドサイズ
		 *
		 *  win: どちらか小さいウィンドサイズに設定する。
		 */
		win = cep->snd_wnd < cep->snd_cwnd ? cep->snd_wnd : cep->snd_cwnd;
		if (win > 0)
			break;

		/* 開き待ちの時間を tmout から減ずる。*/
		if (!(tmout == TMO_POL || tmout == TMO_FEVR))
			syscall(get_tim(&before));

		if ((error = twai_flg(cep->snd_flgid, TCP_CEP_EVT_SWBUF_READY,
		                      TWF_ORW, &flag, tmout)) != E_OK)
			return error;
		syscall(clr_flg(cep->snd_flgid, (FLGPTN)(~TCP_CEP_EVT_SWBUF_READY)));

		/* 開き待ちの時間を tmout から減ずる。*/
		if (!(tmout == TMO_POL || tmout == TMO_FEVR)) {
			syscall(get_tim(&after));
			if (after - before > tmout)
				return E_TMOUT;
			tmout -= (TMO)(after - before);
			}

		/*
		 *  送信できるか、CEP の FSM 状態を見る。
		 *  送信ウインドバッファが空くまで待つ間に、送信不能になった場合は、
		 *  コネクションが切断されたことを意味している。
		 */
		if (!TCP_FSM_CAN_SEND_MORE(cep->fsm_state))
			return E_CLS;
		}

	/* ネットワークバッファを獲得する。*/
	if ((error = tcpn_get_segment(&cep->swbufq, cep, 0,
	                             TCP_CFG_SWBUF_CSAVE_MIN_SIZE,
	                             TCP_CFG_SWBUF_CSAVE_MAX_SIZE,
	                             NBA_SEARCH_DESCENT, tmout)) != E_OK)
		return error;

	/* 送信ウィンドバッファを初期化する。*/
	tcp_init_swbuf(cep);

	return E_OK;
	}

/*
 *  tcp_get_swbuf_addr_cs -- 送信ウィンドバッファの空きアドレスを獲得する（専用）。
 */

ER_UINT
tcp_get_swbuf_addr_cs (T_TCP_CEP *cep, void **p_buf)
{
	ER_UINT	error;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if ((error = net_buf_siz(cep->swbufq)) > 0) {

		/*
		 *  +-----------+--------+---------+---------+
		 *  | Ehter HDR | IP HDR | TCP HDR | TCP SDU |
		 *  +-----------+--------+---------+---------+
		 *        14        20        20        n
		 *   <---------------- error --------------->
		 *              ^
		 *              net_buf で 4 オクテット境界にアラインされている。
		 *
		 *  tcp_output と ip_output で、チェックサムを計算するとき、
		 *  n が 4 オクテット境界になるように SDU の後ろに 0 を
		 *  パッディングする。その分を考慮して送信ウィンドバッファの
		 *  空きサイズを 4 オクテット境界に調整する。
		 */
		error = (uint_t)(((error - IF_IP_TCP_NET_HDR_SIZE(&cep->dstaddr.ipaddr)) >> 2 << 2) - cep->swbuf_count);

		/* 送信ウインドサイズによりサイズを調整する。*/
		if (error > cep->snd_wnd) 
			error = cep->snd_wnd;
		if (error > cep->snd_cwnd)
			error = cep->snd_cwnd;

		/* 相手の最大受信セグメントサイズ (maxseg) を超えないようにする。*/
		if (error > cep->maxseg) 
			error = cep->maxseg;

		/* 送信ウィンドバッファの空きデータ長を設定し、その値を返す。*/
		cep->get_buf_len = error;

		/* 送信ウィンドバッファの空きアドレスの先頭を設定する。*/
		*p_buf = cep->sbuf_wptr;
		}

	else {	/* ネットワークバッファが不正 */

		/* 送信ウィンドバッファキューのネットワークバッファを解放する。*/
		tcp_free_swbufq_cs(cep);
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

/*
 *  tcp_send_swbuf_cs -- 送信ウィンドバッファのデータを送信可能にする（専用）。
 */

void
tcp_send_swbuf_cs (T_TCP_CEP *cep, uint_t len)
{
	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	cep->sbuf_wptr  += len;
	cep->swbuf_count += len;

	/* ネットワークバッファ長と IP データグラム長を設定する。*/
	cep->swbufq->len = (uint16_t)(cep->swbuf_count + IF_IP_TCP_NET_HDR_SIZE(&cep->dstaddr.ipaddr));
	SET_IP_SDU_SIZE(cep->swbufq, len + TCP_HDR_SIZE);

	/* tcp_get_buf の割当て長をリセットする。*/
	cep->get_buf_len = 0;

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* フラグを、送信可能に設定し、強制的に送信する。*/
	cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_SEND_READY
	                                               | TCP_CEP_FLG_FORCE
	                                               | TCP_CEP_FLG_FORCE_CLEAR
	                                               | TCP_CEP_FLG_POST_OUTPUT;
	}

/*
 *  tcp_free_swbufq_cs -- 送信ウィンドバッファキューの解放を指示する（専用）。
 *
 *    注意:
 *      必要であれば、この関数を呼び出す前に、通信端点をロックし、
 *      戻った後、解除する必要がある。
 */

void
tcp_free_swbufq_cs (T_TCP_CEP *cep)
{
	/* 送信ウィンドバッファの空き待ちを取り消す。*/
	cep->flags &= ~TCP_CEP_FLG_WBCS_NBUF_REQ;

	if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) != TCP_CEP_FLG_WBCS_FREE) {
		/*
		 *  送信ウィンドバッファを削除するために、フラグを ACK 完了に設定し、
		 *  TCP 出力タスクを起動する。
		 */
		cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_ACKED;
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	}

/*
 *  tcp_alloc_swbuf_cs -- 送信ウィンドバッファ用のネットワークバッファの割り当てを指示する（専用）。
 */

void
tcp_alloc_swbuf_cs (T_TCP_CEP *cep)
{
	cep->flags |= TCP_CEP_FLG_WBCS_NBUF_REQ;

	/*
	 *  送信ウィンドバッファが未使用の場合のみ、
	 *  送信ウィンドバッファ用のネットワークバッファの割り当てを指示する。
	 */
	if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_FREE)
		sig_sem(SEM_TCP_POST_OUTPUT);
	}

/*
 *  tcp_is_swbuf_full_cs -- 送信ウィンドバッファが満杯かチェックする（専用）。
 */

bool_t
tcp_is_swbuf_full_cs (T_TCP_CEP *cep)
{
	return (cep->flags & TCP_CEP_FLG_WBCS_MASK) != TCP_CEP_FLG_WBCS_NBUF_READY ||
	        cep->swbuf_count >= cep->swbufq->len - IF_IP_TCP_NET_HDR_SIZE(&cep->dstaddr.ipaddr);
	}

/*
 *  tcp_init_swbuf -- 送信ウィンドバッファを初期化する（専用）。
 *
 *    注意: tcp_get_segment でネットワークバッファを獲得した
 *          直後に呼び出すこと。
 */

void
tcp_init_swbuf (T_TCP_CEP *cep)
{
	/* 送信ウインドバッファの使用中サイズをリセットする。*/
	cep->swbuf_count = 0;

	/* 送信ウインドバッファのアドレスを設定する。*/
	cep->sbuf_wptr = cep->sbuf_rptr = GET_TCP_SDU(cep->swbufq, IF_IP_TCP_NET_HDR_OFFSET(&cep->dstaddr.ipaddr));

	/* フラグを、ネットワークバッファ割当て済みに設定する。*/
	cep->flags = (cep->flags & ~(TCP_CEP_FLG_WBCS_NBUF_REQ | TCP_CEP_FLG_WBCS_MASK))
	                         |   TCP_CEP_FLG_WBCS_NBUF_READY;
	}

/*
 *  tcp_push_res_nbuf -- ネットワークバッファを予約する（専用）。
 */

T_NET_BUF *
tcp_push_res_nbuf (T_NET_BUF *nbuf)
{
	static int_t last_ix = 0;

	T_TCP_CEP	*cep;
 	int_t		ix, sel_ix;

	if (++ last_ix == tmax_tcp_cepid)
		last_ix = 0;
	sel_ix = ix = last_ix;
	do {
		cep = &tcp_cep[ix];
		if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_NBUF_PEND) {

			/* ネットワークバッファを予約する。*/
			cep->swbufq = nbuf;
			nbuf = NULL;

			/* フラグを予約に設定する。*/
			cep->flags = (cep->flags & ~TCP_CEP_FLG_WBCS_MASK) | TCP_CEP_FLG_WBCS_NBUF_RSVD;
			sel_ix = ix;
			break;
			}
		if (++ ix == tmax_tcp_cepid)
			ix = 0;
		} while (ix != last_ix);

	last_ix = sel_ix;

	return nbuf;
	}

/*
 *  tcp_pull_res_nbuf -- 予約してあるネットワークバッファ返す（専用）。
 */

T_NET_BUF *
tcp_pull_res_nbuf (ATR nbatr)
{
	T_NET_BUF	*nbuf;
	T_TCP_CEP	*cep;

	cep = GET_TCP_CEP(nbatr & NBA_ID_MASK);
	if ((cep->flags & TCP_CEP_FLG_WBCS_MASK) == TCP_CEP_FLG_WBCS_NBUF_RSVD) {

		/* 予約してあるネットワークバッファを返す。*/
		nbuf = cep->swbufq;
		cep->swbufq = NULL;

		/* TCP 出力タスクを起動する。*/
		sig_sem(SEM_TCP_POST_OUTPUT);
		}
	else
		nbuf = NULL;

	return nbuf;
	}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

#ifdef TCP_CFG_RWBUF_CSAVE 

#ifdef TCP_CFG_RWBUF_CSAVE_ONLY

#else	/* of #ifdef TCP_CFG_RWBUF_CSAVE_ONLY */

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能が有効な場合
 *  ただし、通信端点の受信ウィンドバッファ（cep->rbuf）の値により、
 *  TCP 通信端点毎に受信ウィンドバッファの省コピー機能を使用するか、
 *  使用しないかを切り替える。
 */

/*
 *  tcp_drop_rwbuf -- 受信ウィンドバッファキューの最初のネットワークバッファを解放する。
 */

void
tcp_drop_rwbuf (T_TCP_CEP *cep, uint_t len)
{
	if (IS_PTR_DEFINED(cep->rbuf))
		tcp_drop_rwbuf_ncs(cep, len);
	else
		tcp_drop_rwbuf_cs(cep, len);
	}

/*
 *  tcp_read_rwbuf -- 受信ウィンドバッファから指定されたオクテット分読み出す。
 */

uint_t
tcp_read_rwbuf (T_TCP_CEP *cep, void *data, uint_t len)
{
	if (IS_PTR_DEFINED(cep->rbuf))
		return tcp_read_rwbuf_ncs(cep, data, len);
	else
		return tcp_read_rwbuf_cs(cep, data, len);
	}

/*
 *  tcp_get_rwbuf_addr -- 受信ウィンドバッファの空きアドレスを獲得する。
 */

uint_t
tcp_get_rwbuf_addr (T_TCP_CEP *cep, void **p_buf)
{
	if (IS_PTR_DEFINED(cep->rbuf))
		return tcp_get_rwbuf_addr_ncs(cep, p_buf);
	else
		return tcp_get_rwbuf_addr_cs(cep, p_buf);
	}

/*
 *  tcp_free_rwbufq -- 受信ウィンドバッファキューを解放する。
 */

void
tcp_free_rwbufq (T_TCP_CEP *cep)
{
	if (!IS_PTR_DEFINED(cep->rbuf))
		tcp_free_rwbufq_cs(cep);
	}

/*
 *  tcp_write_rwbuf -- 受信ウィンドバッファにデータを書き込む。
 */

void
tcp_write_rwbuf (T_TCP_CEP *cep, T_NET_BUF *input, uint_t thoff)
{
	if (IS_PTR_DEFINED(cep->rbuf))
		tcp_write_rwbuf_ncs(cep, input, thoff);
	else
		tcp_write_rwbuf_cs(cep, input, thoff);
	}

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE_ONLY */

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能が有効
 */

/*
 *  tcp_drop_rwbuf -- 受信ウィンドバッファキューの最初のネットワークバッファを解放する。
 */

void
tcp_drop_rwbuf_cs (T_TCP_CEP *cep, uint_t len)
{
	T_TCP_Q_HDR	*qhdr;

	if (cep->rwbufq != NULL) {

		qhdr = GET_TCP_Q_HDR(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq));
		cep->rwbuf_count -= len;

		/* ネットワークバッファにデータが無くなったら解放する。*/
		if (len == qhdr->slen) {
			T_NET_BUF *next;

#ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES

			cep->rwbufq_entries --;

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES */

			next = qhdr->next;
			syscall(rel_net_buf(cep->rwbufq));
			cep->rwbufq = next;
			}
		else {
			qhdr->slen -= len;
			qhdr->soff += len;
			}
		}
	}

/*
 *  tcp_read_rwbuf -- 受信ウィンドバッファから指定されたオクテット分読み出す。
 */

uint_t
tcp_read_rwbuf_cs (T_TCP_CEP *cep, void *data, uint_t len)
{
	T_TCP_Q_HDR	*qhdr;
	uint_t		blen, rlen = 0;
	uint8_t		*buf = (uint8_t*)data;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 *  受信ウィンドバッファキューにネットワークバッファが無くなるか
	 *  len が 0 になるまで続ける。
	 */
	while (cep->rwbufq != NULL && len > 0) {
		qhdr = GET_TCP_Q_HDR(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq));

		/*
		 *  len と受信ウィンドバッファキューの先頭ネットワークバッファの
		 *  データ数の小さい方を移すデータ数にする。
		 */
		if (len > qhdr->slen)
			blen = qhdr->slen;
		else
			blen = len;

		memcpy(buf,
		       GET_TCP_SDU(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq)) + qhdr->soff,
		       (size_t)blen);

		/*
		 *  受信ウィンドバッファキューのネットワークバッファに
		 *  データが無くなったら解放する。
		 */
		tcp_drop_rwbuf_cs(cep, blen);

		buf  += blen;
		rlen += blen;
		len  -= blen;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return rlen;
	}

/*
 *  tcp_get_rwbuf_addr -- 受信ウィンドバッファの空きアドレスを獲得する。
 */

uint_t
tcp_get_rwbuf_addr_cs (T_TCP_CEP *cep, void **p_buf)
{
	T_TCP_Q_HDR	*qhdr;
	uint_t		len;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if (cep->rwbufq == NULL) {

		/* 受信ウィンドバッファが全て開放されているとき。*/
		*p_buf = NULL;
		len = 0;
		}
	else {

		/* 受信ウィンドバッファのデータのアドレスの先頭を設定する。*/
		qhdr = GET_TCP_Q_HDR(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq));
		*p_buf = GET_TCP_SDU(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq)) + qhdr->soff;

		/* 受信ウィンドバッファのデータ長を計算する。*/
		len = qhdr->slen;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	/* 受信ウィンドバッファのデータ長を設定し、その値を返す。*/
	cep->rcv_buf_len = len;
	return len;
	}

/*
 *  tcp_free_rwbufq -- 受信ウィンドバッファキューを解放する。
 *
 *    注意:
 *      必要であれば、この関数を呼び出す前に、通信端点をロックし、
 *      戻った後、解除する必要がある。
 */

void
tcp_free_rwbufq_cs (T_TCP_CEP *cep)
{
	T_NET_BUF *next;

	if (cep->rwbuf_count == 0 && cep->reassq == NULL) {
		while (cep->rwbufq != NULL) {
			next = GET_TCP_Q_HDR(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq))->next;

#ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES

			cep->rwbufq_entries --;

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES */

			syscall(rel_net_buf(cep->rwbufq));
			cep->rwbufq = next;
			}

		/* 受信ウィンドバッファの使用中サイズをリセットする。*/
		cep->rwbuf_count = 0;
		}
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
tcp_write_rwbuf_cs (T_TCP_CEP *cep, T_NET_BUF *input, uint_t thoff)
{
	T_TCP_Q_HDR	*qhdr;
	T_NET_BUF	**nextp;
	uint_t		inlen, last;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	qhdr  = (T_TCP_Q_HDR*)GET_TCP_HDR(input, thoff);

	inlen = qhdr->slen;

#ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES

	if (cep->rwbufq_entries >= TCP_CFG_RWBUF_CSAVE_MAX_QUEUES &&
	    inlen > 0 && (qhdr->flags & TCP_FLG_FIN) == 0) {

		/*
		 *  TCP 通信端点の受信ウィンドバッファの省コピー機能の、
		 *  受信ウィンドバッファキューの最大エントリ数を超えるときは、
		 *  受信したセグメントを破棄する。
		 *  ただし、SDU 長が 0 のセグメントと FIN セグメントは破棄しない。
		 *  なお、正常に受信したセグメントも破棄するため、再送回数が増加する。
		 */
		syscall(rel_net_buf(input));

		/* 通信端点のロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));
		return;
		}

	cep->rwbufq_entries ++;

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE_MAX_QUEUES */

	qhdr  = (T_TCP_Q_HDR*)GET_TCP_HDR(input, thoff);

	/* 受信済みシーケンス番号を更新する。*/
	cep->rcv_nxt += inlen;

	/* 緊急データの SDU 補正を行う。*/
	if (qhdr->urp > 0 && inlen > 0) {
		inlen      -= qhdr->urp;
		qhdr->slen -= qhdr->urp;
		qhdr->urp   = 0;
		}

	last  = cep->rwbuf_count;

	/* ネットワークバッファを受信ウィンドバッファキューの最後に連結する。*/
	qhdr->next = NULL;
	nextp = &cep->rwbufq;
	while (*nextp)
		nextp = &GET_TCP_Q_HDR(*nextp, GET_IP_TCP_Q_HDR_OFFSET(*nextp))->next;
	*nextp = input;

	/*
	 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
	 *  これは、アプリケーションに、相手からこれ以上データが送られて
	 *  こないことを知らせるためである。
	 */
	if (inlen > 0) {
		cep->rwbuf_count += inlen;
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DATA_SEGS],   1);
		NET_COUNT_TCP(net_count_tcp[NC_TCP_RECV_DATA_OCTETS], inlen);
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

#ifdef TCP_CFG_NON_BLOCKING

	if (cep->rcv_nblk_tfn == TFN_TCP_RCV_BUF) {	/* ノンブロッキングコール */

		int_t	len;

		qhdr = GET_TCP_Q_HDR(cep->rwbufq, GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq));
		len = qhdr->slen;

		/*
		 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
		 *  これは、アプリケーションに、相手からこれ以上データが送られて
		 *  こないことを知らせるためである。
		 */
		if (len > 0 || inlen == 0) {

			/* tcp_rcv_buf の割当て長を設定する。*/
			cep->rcv_buf_len = len;

			/* 受信ウィンドバッファのアドレスを返す。*/
			*cep->rcv_p_buf = GET_TCP_SDU(cep->rwbufq,
			                  GET_IP_TCP_Q_HDR_OFFSET(cep->rwbufq)) + qhdr->soff;

			if (IS_PTR_DEFINED(cep->callback))

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)len);
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
				tcp_free_rwbufq_cs(cep);
				syscall(sig_sem(cep->semid_lock));
				}
			}

		/* 記憶されているタスク ID と API 機能コードをクリアーする。*/
		cep->rcv_tskid = TA_NULL;
		cep->rcv_tfn   = cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
		}

	else if (cep->rcv_nblk_tfn == TFN_TCP_RCV_DAT) {	/* ノンブロッキングコール */
		int_t	len;

		/* 受信ウィンドバッファからデータを取り出す。*/
		if ((len = tcp_read_rwbuf_cs(cep, cep->rcv_data, (uint_t)cep->rcv_len)) > 0) {
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
				(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)len);
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

		if (inlen == 0 && cep->rwbuf_count == 0) {
			/*
			 *  受信ウィンドバッファ内のデータ数が 0 で、
			 *  相手から FIN フラグが付いたセグメントを受信したときは、
			 *  通信端点をロックして、
			 *  受信ウィンドバッファキューのネットワークバッファを解放する。
			 */
			syscall(wai_sem(cep->semid_lock));
			tcp_free_rwbufq_cs(cep);
			syscall(sig_sem(cep->semid_lock));
			}

		/*
		 *  受信ウィンドバッファにデータが入るか、 inlen == 0 の時、入力タスクを起床する。
		 *  FIN フラグが付いたセグメントは inlen == 0 になることもある。
		 *  これは、アプリケーションに、相手からこれ以上データが送られて
		 *  こないことを知らせるためである。
		 */
		if ((last == 0 && cep->rwbuf_count > 0) || inlen == 0) {
			syscall(set_flg(cep->rcv_flgid, TCP_CEP_EVT_RWBUF_READY));
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	}

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE */

#endif	/* of #ifdef SUPPORT_TCP */
