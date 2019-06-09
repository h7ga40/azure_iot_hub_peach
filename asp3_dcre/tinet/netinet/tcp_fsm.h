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
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)tcp_fsm.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/tcp_fsm.h,v 1.10.2.3 1999/11/15 02:52:54 green Exp $
 */
 
#ifndef _TCP_FSM_H_
#define _TCP_FSM_H_

/*
 *  TCP FSM 状態の定義、RFC 793 参照
 */

#define TCP_FSM_CLOSED		0	/* クローズ				*/
#define TCP_FSM_LISTEN		1	/* 受動オープン				*/
#define TCP_FSM_SYN_SENT	2	/* 能動オープン、SYN 送信済み		*/
#define TCP_FSM_SYN_RECVD	3	/* 受動オープン、SYN 受信済み		*/
#define TCP_FSM_ESTABLISHED	4	/* コネクション開設完了			*/
#define TCP_FSM_CLOSE_WAIT	5	/* 相手から FIN 受信、APP の終了待ち	*/
#define TCP_FSM_FIN_WAIT_1	6	/* APP が終了、FIN 送信済み、ACK 待ち	*/
#define TCP_FSM_CLOSING		7	/* 同時クローズ、FIN 交換済み、ACK 待ち	*/
#define TCP_FSM_LAST_ACK	8	/* APP が終了、ACK 待ち			*/
#define TCP_FSM_FIN_WAIT_2	9	/* 相手からの FIN 待ち			*/
#define TCP_FSM_TIME_WAIT	10	/* 相手からの FIN 受信済み、時間待ち	*/

/*
 *  関数シミュレーションマクロ
 */

#define TCP_FSM_HAVE_RCVD_SYN(s)	((s) >= TCP_FSM_SYN_RECVD)
#define TCP_FSM_HAVE_ESTABLISHED(s)	((s) >= TCP_FSM_ESTABLISHED)
#define TCP_FSM_HAVE_RCVD_FIN(s)	((s) >= TCP_FSM_TIME_WAIT)

#define TCP_FSM_CAN_SEND_MORE(s)	((s) == TCP_FSM_ESTABLISHED || \
					 (s) == TCP_FSM_CLOSE_WAIT)
#define TCP_FSM_CAN_RECV_MORE(s)	((s) == TCP_FSM_ESTABLISHED || \
					 (s) == TCP_FSM_FIN_WAIT_1  || \
					 (s) == TCP_FSM_FIN_WAIT_2)

#endif	/* of #ifndef _TCP_FSM_H_ */
