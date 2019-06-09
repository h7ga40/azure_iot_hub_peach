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
 * Copyright (c) 1982, 1986, 1993, 1994, 1995
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
 *	@(#)tcp_var.h	8.4 (Berkeley) 5/24/95
 * $FreeBSD: src/sys/netinet/tcp_var.h,v 1.49.2.1 1999/08/29 16:29:57 peter Exp $
 */
 
#ifndef _TCP_VAR_H_
#define _TCP_VAR_H_

/*
 *  TCP ヘッダ・ペイロードアクセスマクロ
 */

#define GET_TCP_HDR(nbuf,thoff)		((T_TCP_HDR*)((uint8_t*)((nbuf)->buf) + thoff))
#define GET_TCP_OPT(nbuf,thoff)		((uint8_t*)((nbuf)->buf) + thoff + TCP_HDR_SIZE)
#define GET_TCP_SDU(nbuf,thoff)		((uint8_t*)((nbuf)->buf) + thoff + GET_TCP_HDR_SIZE(nbuf,thoff))

#define GET_TCP_HDR_OFFSET(nbuf)	(GET_IF_IP_HDR_SIZE(nbuf))

#define GET_TCP_HDR_SIZE(nbuf,thoff)	(TCP_HDR_LEN(GET_TCP_HDR(nbuf,thoff)->doff))

#define GET_IP_TCP_HDR_SIZE(nbuf,thoff) (GET_IP_HDR_SIZE(nbuf) + GET_TCP_HDR_SIZE(nbuf,thoff))
#define GET_IF_IP_TCP_HDR_SIZE(nbuf,thoff) \
					(IF_HDR_SIZE + GET_IP_TCP_HDR_SIZE(nbuf,thoff))
#define IF_IP_TCP_HDR_SIZE(nbuf)	(IF_IP_HDR_SIZE(nbuf) + TCP_HDR_SIZE)
#define IF_IP_TCP_HDR_OFFSET(nbuf)	(IF_IP_HDR_SIZE(nbuf))
#define IF_IP_TCP_NET_HDR_SIZE(addr)	(IF_IP_NET_HDR_SIZE(addr) + TCP_HDR_SIZE)
#define IF_IP_TCP_NET_HDR_OFFSET(addr)	(IF_IP_NET_HDR_SIZE(addr))

/*
 *  TINET 1.1 との互換性のための定義
 */

#if !defined(_IP6_CFG) && defined(_IP4_CFG)

#define IP4_TCP_HDR_SIZE		IP_TCP_HDR_SIZE
#define IF_IP4_TCP_HDR_SIZE		IF_IP_TCP_HDR_SIZE
#define GET_IP4_TCP_HDR_SIZE(nbuf)	GET_IP_TCP_HDR_SIZE1(nbuf)	
#define GET_IF_IP4_TCP_HDR_SIZE(nbuf)	GET_IF_IP_TCP_HDR_SIZE1(nbuf)	

#endif	/* of #if !defined(_IP6_CFG) && defined(_IP4_CFG) */

/*
 *  TINET 1.3 との互換性のための定義
 */

#define	TMIN_TCP_CREPID			TMIN_TCP_REPID
#define	TNUM_TCP_CREPID			TNUM_TCP_REPID

#define	TMIN_TCP_CCEPID			TMIN_TCP_CEPID
#define	TNUM_TCP_CCEPID			TNUM_TCP_CEPID

/*
 *  コールバック関数の定義
 */

typedef ER	(*t_tcp_callback)(ID cepid, FN fncd, void *p_parblk);

/*
 * TCP で使用する変数の型の定義
 */

typedef uint32_t T_TCP_SEQ;
typedef uint16_t T_TCP_SEG;
typedef uint16_t T_TCP_WND;
typedef int16_t T_TCP_TIME;

/*
 *  TCP タイマ変数
 */

#define NUM_TCP_TIMERS		4	/* TCP のタイマ数	*/

/*
 *  TCP 受付口
 */

/* IPv6 用 TCP 受付口 */

typedef struct t_tcp6_rep {

	/*
	 * ITRON TCP/IP API、TCP 受付口と同じメンバ
	 */
	ATR		repatr;		/* 受付口属性		*/
	T_IPV6EP	myaddr;		/* 自分のアドレス	*/

#ifdef TCP_CFG_EXTENTIONS
	uint16_t	flags;		/* 受付口フラグ		*/
	ID		semid_lock;	/* 受付口ロック		*/
#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	} T_TCP6_REP;

/* IPv4 用 TCP 受付口 */

typedef struct t_tcp4_rep {

	/*
	 * ITRON TCP/IP API、TCP 受付口と同じメンバ
	 */
	ATR		repatr;		/* 受付口属性		*/
	T_IPV4EP	myaddr;		/* 自分のアドレス	*/

#ifdef TCP_CFG_EXTENTIONS
	uint16_t	flags;		/* 受付口フラグ		*/
	ID		semid_lock;	/* 受付口ロック		*/
#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	} T_TCP4_REP;

#if defined(_IP6_CFG)

#define T_TCP_REP	T_TCP6_REP

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

#define T_TCP_REP	T_TCP4_REP

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */

#define T_TCP_REP_DEFINED

/*
 *  受付口フラグ
 */

#define TCP_REP_FLG_DYNAMIC	UINT_C(0x0400)	/* 動的生成・削除可能受付口。	*/
#define TCP_REP_FLG_VALID	UINT_C(0x0800)	/* 生成済みで有効な受付口。	*/

/*
 *  TCP 通信端点
 */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

typedef struct t_tcp_cep {

	/*
	 * ITRON TCP/IP API、TCP 通信端点と同じメンバ
	 */
	ATR		cepatr;		/* 通信端点属性			*/
	void		*sbuf;		/* 送信用ウィンドバッファ	*/
	int_t		sbufsz;		/* 送信用ウィンドバッファサイズ	*/
	void		*rbuf;		/* 受信用ウィンドバッファ	*/
	int_t		rbufsz;		/* 受信用ウィンドバッファサイズ	*/
	t_tcp_callback	callback;	/* コールバック関数		*/

	/*
	 * TINET 独自のメンバ
	 */
	uint32_t	flags;		/* 通信端点フラグ		*/
	ID		semid_lock;	/* 通信端点ロック		*/
	ID		est_flgid;	/* コネクション状態イベントフラグ	*/
	ID		snd_flgid;	/* 送信イベントフラグ		*/
	ID		rcv_flgid;	/* 受信イベントフラグ		*/
	ID		snd_tskid;	/* 送信タスク識別子		*/
	ID		rcv_tskid;	/* 受信タスク識別子		*/
	uint8_t		*sbuf_rptr;	/* 送信用読み出しポインタ	*/
	uint8_t		*sbuf_wptr;	/* 送信用書き込みポインタ	*/
	uint8_t		*rbuf_rptr;	/* 受信用読み出しポインタ	*/
	uint8_t		*rbuf_wptr;	/* 受信用書き込みポインタ	*/
	T_TCP_SEQ	snd_ssthresh;	/* 輻輳ウィンドサイズ(snd_cwnd)	*/
					/* のしきい値			*/
	T_TCP_TIME	rxtcur;		/* 現在の再送タイムアウト	*/
	T_TCP_TIME	srtt;		/* 平滑化された RTT		*/
	T_TCP_TIME	rttvar;		/* 平滑化された分散		*/
	T_TCP_WND	snd_cwnd;	/* 輻輳ウィンドサイズ		*/

	/*
	 *  相手の最大受信セグメントサイズ	
	 *
	 *    オリジナルでは、一般のセグメントの最大サイズ (t_maxseg) と、
	 *    オプション付きのセグメントの最大サイズ (t_maxopd) が別に
	 *    なっている。これは、現在のインターネットでは、オプション付きの
	 *    セグメントのほうが多いからと説明されているためである。
	 *    本実装では、最大セグメントサイズオプション以外のオプションは
	 *    実装しないので、t_maxseg の使用する。
	 */
	T_TCP_SEG	maxseg;

	int16_t		snd_tfn;	/* 送信動作中の API 機能コード	*/
	int16_t		rcv_tfn;	/* 受信動作中の API 機能コード	*/

#ifdef TCP_CFG_NON_BLOCKING

	T_IPEP 		*p_dstaddr;	/* 相手のアドレスへのポインタ	*/
	T_IPEP	 	*p_myaddr;	/* 自分のアドレスへのポインタ	*/

#if defined(_IP6_CFG) && defined(_IP4_CFG)
	T_IPV4EP	*p_dstaddr4;	/* 相手のアドレスへのポインタ	*/
	T_IPV4EP 	*p_myaddr4;	/* 自分のアドレスへのポインタ	*/
#endif
	
	int16_t		snd_nblk_tfn;	/* 送信動作中の API 機能コード	*/
	int16_t		rcv_nblk_tfn;	/* 受信動作中の API 機能コード	*/
	void		*snd_data;	/* 送信データ領域の先頭アドレス	*/
	int_t		snd_len;	/* 送信データ領域の長さ		*/
	void		*rcv_data;	/* 受信データ領域の先頭アドレス	*/
	int_t		rcv_len;	/* 受信データ領域の長さ		*/
	void		**snd_p_buf;	/* 送信バッファへのポインタ	*/
	void		**rcv_p_buf;	/* 受信バッファへのポインタ	*/

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

#ifdef TCP_CFG_SWBUF_CSAVE
	T_NET_BUF	*swbufq;	/* 送信ウィンドバッファキュー	*/
#endif

	/*
	 *  これ以降のメンバーは tcp_init_cep() 関数でゼロクリアーされる。
	 */

	T_TCP_TIME	timer[NUM_TCP_TIMERS];	/* TCP のタイマ		*/
	T_IPEP		dstaddr;	/* 相手のアドレス		*/
	T_IPEP		myaddr;		/* 自分のアドレス		*/
	T_NET_BUF	*reassq;	/* 受信再構成キュー		*/

#if defined(_IP6_CFG)

#if defined(_IP4_CFG)

	T_TCP4_REP	*rep4;		/* IPv4 用受付口		*/

#endif	/* of #if defined(_IP4_CFG) */

	T_TCP6_REP	*rep;		/* IPv6 用受付口		*/

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

	T_TCP4_REP	*rep;		/* IPv4 用受付口		*/

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */


#ifdef TCP_CFG_RWBUF_CSAVE
	T_NET_BUF	*rwbufq;	/* 受信ウィンドバッファキュー	*/
#endif

#ifdef TCP_CFG_EXTENTIONS
	T_TCP_HDR	*urg_tcph;	/* 緊急データ入りのセグメントの TCP ヘッダ	*/
	T_TCP_SEQ	snd_up;		/* 送信した緊急ポインタ		*/
	T_TCP_SEQ	rcv_up;		/* 受信した緊急ポインタ		*/
#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	T_TCP_SEQ	snd_una;	/* 未確認の最小送信 SEQ つまり	*/
					/* 確認された最大送信 SEQ	*/
	T_TCP_SEQ	snd_max;	/* 送信した最大 SEQ		*/
	T_TCP_SEQ	snd_nxt;	/* 次に送信する SEQ		*/
	T_TCP_SEQ	snd_old_nxt;	/* 元の snd_nxt			*/
	T_TCP_SEQ	snd_wl1;	/* 前回ウィンドを更新した SEQ	*/
	T_TCP_SEQ	snd_wl2;	/* 前回ウィンドを更新した ACK	*/
	T_TCP_SEQ	iss;		/* 自分の SEQ の初期値		*/
	T_TCP_SEQ	irs;		/* 相手の SEQ の初期値		*/
	T_TCP_SEQ	rcv_nxt;	/* 受信を期待している最小の SEQ	*/
	T_TCP_SEQ	rcv_adv;	/* 受信を期待している最大の SEQ	*/
	T_TCP_SEQ	rcv_wnd;	/* 受信可能なウィンドサイズ	*/
	T_TCP_SEQ	rtseq;		/* 時間計測を始めた SEQ		*/
	T_TCP_SEQ	last_ack_sent;	/* 最後に送信した ACK		*/
	T_TCP_TIME	idle;		/* アイドル時間			*/
	ER		error;		/* 非同期に発生したエラー	*/
	ER		net_error;	/* ネットワークのエラー状態	*/
	T_TCP_WND	snd_wnd;	/* 相手の受信可能ウィンドサイズ	*/
	T_TCP_WND	max_sndwnd;	/* 今までの最大送信ウィンドサイズ	*/
	T_TCP_TIME	rtt;		/* 往復時間				*/
	uint16_t	swbuf_count;	/* 送信ウィンドバッファの使用中サイズ	*/
	uint16_t	rwbuf_count;	/* 受信ウィンドバッファの使用中サイズ	*/
	uint16_t	rcv_buf_len;	/* tcp_rcv_buf の割当て長	*/
	uint16_t	get_buf_len;	/* tcp_rcv_buf の割当て長	*/
	uint8_t		rxtshift;	/* 再送信回数の log(2)		*/
	uint8_t		fsm_state;	/* FSM 状態			*/
	uint8_t		dupacks;	/* 再送 ACK 数			*/

#if defined(TCP_CFG_RWBUF_CSAVE) && defined(TCP_CFG_RWBUF_CSAVE_MAX_QUEUES)
	uint16_t	rwbufq_entries;	/* 受信ウィンドバッファキューのエントリ数	*/
#endif

	} T_TCP_CEP;

#define T_TCP6_CEP	T_TCP_CEP
#define T_TCP4_CEP	T_TCP_CEP

/*
 * Time Wait 用 TCP 通信端点
 */

typedef struct t_tcp_twcep {
	int_t		rbufsz;		/* 受信用ウィンドバッファサイズ	*/
	T_TCP_TIME	timer_2msl;	/* 2MSL タイマ			*/
	T_IPEP		dstaddr;	/* 相手のアドレス		*/
	T_IPEP		myaddr;		/* 自分のアドレス		*/
	T_TCP_SEQ	snd_una;	/* 未確認の最小送信 SEQ つまり	*/
					/* 確認された最大送信 SEQ	*/
	T_TCP_SEQ	rcv_nxt;	/* 受信を期待している最小の SEQ	*/
	uint16_t	rwbuf_count;	/* 受信ウィンドバッファの使用中サイズ	*/
	uint8_t		fsm_state;	/* FSM 状態			*/
	uint8_t	flags;			/* 通信端点フラグ		*/

	} T_TCP_TWCEP;

#define T_TCP_CEP_DEFINED

/*
 *  TCP 通信端点の状態フラグ
 */

#define TCP_CEP_FLG_IPV4		ULONG_C(0x00000001)	/* IPv4 用。			*/
#define TCP_CEP_FLG_ACK_NOW		ULONG_C(0x00000100)	/* 直ちに相手に ACK を送る。	*/
#define TCP_CEP_FLG_DEL_ACK		ULONG_C(0x00000200)	/* ACK を遅延する。		*/
#define TCP_CEP_FLG_SENT_FIN		ULONG_C(0x00000400)	/* FIN を送ってある。		*/
#define TCP_CEP_FLG_NEED_SYN		ULONG_C(0x00000800)	/* SYN を送信する。		*/
#define TCP_CEP_FLG_NEED_FIN		ULONG_C(0x00001000)	/* FIN を送信する。		*/
#define TCP_CEP_FLG_FORCE		ULONG_C(0x00002000)	/* 強制送信。			*/
#define TCP_CEP_FLG_NO_PUSH		ULONG_C(0x00004000)	/* push しない。		*/
#define TCP_CEP_FLG_NO_DELAY		ULONG_C(0x00008000)	/* 遅延しない。			*/
#define TCP_CEP_FLG_FORCE_CLEAR		ULONG_C(0x00010000)	/* 強制送信した後フラグをクリアーする。*/
#define TCP_CEP_FLG_POST_OUTPUT		ULONG_C(0x00020000)	/* 送信を開始する。		*/
#define TCP_CEP_FLG_RESERVE_OUTPUT	ULONG_C(0x00040000)	/* 送信を予約する。		*/
#define TCP_CEP_FLG_DYNAMIC		ULONG_C(0x00100000)	/* 動的生成・削除可能通信端点。	*/
#define TCP_CEP_FLG_VALID		ULONG_C(0x00200000)	/* 生成済みで有効な通信端点。	*/
#define TCP_CEP_FLG_CLOSE_AFTER_OUTPUT	ULONG_C(0x00400000)	/* 送信した後コネクションを閉じる。*/
#define TCP_CEP_FLG_RESTORE_NEXT_OUTPUT	ULONG_C(0x00800000)	/* 送信した後、snd_nxt を元に戻す。*/

/* 送受信ウィンドバッファの省コピー機能 */
#define TCP_CEP_FLG_WBCS_NBUF_REQ	ULONG_C(0x80000000)	/* ネットワークバッファ割当て要求*/
#define TCP_CEP_FLG_WBCS_MASK		ULONG_C(0x70000000)	/* マスク			*/
#define TCP_CEP_FLG_WBCS_FREE		ULONG_C(0x00000000)	/* 送信ウィンドバッファ未使用	*/
#define TCP_CEP_FLG_WBCS_WOPEN_PEND	ULONG_C(0x10000000)	/* 相手の受信ウィンドの開き待ち	*/
#define TCP_CEP_FLG_WBCS_NBUF_PEND	ULONG_C(0x20000000)	/* ネットワークバッファ予約待ち	*/
#define TCP_CEP_FLG_WBCS_NBUF_RSVD	ULONG_C(0x30000000)	/* ネットワークバッファ予約済み	*/
#define TCP_CEP_FLG_WBCS_NBUF_READY	ULONG_C(0x40000000)	/* ネットワークバッファ割当て済み*/
#define TCP_CEP_FLG_WBCS_SEND_READY	ULONG_C(0x50000000)	/* 送信可能			*/
#define TCP_CEP_FLG_WBCS_SENT		ULONG_C(0x60000000)	/* 送信済みで、ACK待ち		*/
#define TCP_CEP_FLG_WBCS_ACKED		ULONG_C(0x70000000)	/* 送信済みで、ACK完了		*/

/* 初期化から除外するフラグの定義 */
#define TCP_CEP_FLG_NOT_CLEAR		(TCP_CEP_FLG_WBCS_NBUF_REQ|TCP_CEP_FLG_WBCS_MASK|\
					 TCP_CEP_FLG_DYNAMIC|TCP_CEP_FLG_VALID|TCP_CEP_FLG_IPV4)

/*
 *  通信端点イベントフラグ
 */

#define TCP_CEP_EVT_ESTABLISHED		ULONG_C(0x00000001)	/* コネクション確立		*/
#define TCP_CEP_EVT_CLOSED		ULONG_C(0x00000002)	/* 未使用状態			*/
#define TCP_CEP_EVT_SWBUF_READY		ULONG_C(0x00000004)	/* 送信ウィンドバッファ空き	*/
#define TCP_CEP_EVT_RWBUF_READY		ULONG_C(0x00000008)	/* 送信ウィンドバッファデータあり*/
#define TCP_CEP_EVT_ALL			ULONG_C(0xffffffff)	/* 全イベント			*/

/*
 *  追加 API 機能・事象コード
 */

/* TCP 関係 */

#define TFN_TCP_UNDEF		(0)

/*
 *  srtt:   平滑化された RTT
 *  rttvar: 平滑化された分散
 *  rtt:    測定された往復時間
 *
 *  平滑化された RTT (srtt) と平滑化された分散 (rttvar) を
 *  整数型で計算するために、左シフトしてスケールする。
 *  srtt は 2 進小数点以下 3 ビット、
 *  rttvar は 2 進小数点以下 2 ビットを保持する。
 */

#define TCP_DELTA_SHIFT		2
			/* 差分 = rtt/8 + srtt*7/8 の計算のためのシフト値	*/
#define TCP_SRTT_SHIFT		(TCP_DELTA_SHIFT + 3)
					/* srtt のシフト値、3(+2)ビット	*/
#define TCP_RTTVAR_SHIFT	(TCP_DELTA_SHIFT + 2)
					/* rttvar のシフト値、2(+2)ビット	*/
#define TCP_SRTT_SCALE		(1<<TCP_SRTT_SHIFT)
#define TCP_RTTVAR_SCALE	(1<<TCP_RTTVAR_SHIFT)

/*
 *  高速再転送を開始する再送 ACK 数のしきい値
 */

#define MAX_TCP_REXMT_THRESH	3

/*
 *  受信再構成キューでの TCP ヘッダ
 */

typedef struct t_tcp_q_hdr {
	uint16_t	sport;		/* 送信元ポート番号			*/
	uint16_t	dport;		/* 宛先ポート番号			*/

	uint32_t	seq;		/* SEQ 番号				*/

	T_NET_BUF 	*next;		/* 次のセグメント、元は ACK 番号(ack)	*/

	uint8_t		doff;		/* データオフセット(上位4ビット)	*/
	uint8_t		flags;		/* フラグ(下位6ビット)			*/
	uint16_t	soff;		/* SDU オフセット、元はウィンドサイズ(win)	*/

	uint16_t	slen;		/* SDU 長、元はチェックサム(sum)	*/
	uint16_t	urp;		/* 緊急ポインタ（SDU 長補正）		*/
	} __attribute__((packed, aligned(2)))T_TCP_Q_HDR;

#define GET_TCP_Q_HDR(nbuf,thoff)	((T_TCP_Q_HDR*)((uint8_t*)((nbuf)->buf) + thoff))

/*
 *  オブジェクト ID の最小値の定義
 */

#define	TMIN_TCP_REPID		1			/* TCP      受付口 ID の最小値			*/
#define	TMIN_TCP6_REPID		1			/* TCP/IPv6 受付口 ID の最小値			*/
#define	TMIN_TCP4_REPID		(TNUM_TCP6_REPID+1)	/* TCP/IPv4 受付口 ID の最小値			*/

#define	TMIN_TCP_CEPID		1			/* TCP      通信端点 ID の最小値			*/
#define	TMIN_TCP6_CEPID		1			/* TCP/IPv6 通信端点 ID の最小値			*/
#define	TMIN_TCP4_CEPID		(TNUM_TCP6_CEPID+1)	/* TCP/IPv4 通信端点 ID の最小値			*/

#define	TMIN_TCP_TWCEPID	1			/* Time Wait 用 TCP 通信端点 ID の最小値	*/

/*
 *  制御セグメント（データなし）サイズのネットワークバッファ
 */

#if defined(IF_HDR_SIZE) && defined(IP_HDR_SIZE)

typedef struct t_net_buf_cseg {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[IF_HDR_SIZE + IP_HDR_SIZE + TCP_HDR_SIZE];
				/* バッファ本体		*/
	} T_NET_BUF_CSEG;

#endif	/* of #if defined(IF_HDR_SIZE) && defined(IP_HDR_SIZE) */

/*
 *  SNMP の 管理情報ベース (MIB)
 */

typedef struct t_tcp_stats {
	uint32_t	tcpActiveOpens;
	uint32_t	tcpPassiveOpens;
	uint32_t	tcpAttemptFails;
	uint32_t	tcpEstabResets;
/*	uint32_t	tcpCurrEstab;*/
	uint32_t	tcpInSegs;
	uint32_t	tcpOutSegs;
	uint32_t	tcpRetransSegs;
	uint32_t	tcpInErrs;
	uint32_t	tcpOutRsts;
} T_TCP_STATS;

/*
 *  関数シミュレーションマクロ
 */

/* TCP 受付口 */

#define VALID_TCP6_REPID(id)	(TMIN_TCP6_REPID<=(id)&&(id)<=tmax_tcp6_repid)
#define VALID_TCP4_REPID(id)	(TMIN_TCP4_REPID<=(id)&&(id)<=tmax_tcp4_repid)

#define INDEX_TCP6_REP(id)	((id)-TMIN_TCP6_REPID)
#define INDEX_TCP4_REP(id)	((id)-TMIN_TCP4_REPID)

#define GET_TCP6_REP(id)	(&(tcp6_rep[INDEX_TCP6_REP(id)]))
#define GET_TCP4_REP(id)	(&(tcp4_rep[INDEX_TCP4_REP(id)]))

#define GET_TCP6_REPID(rep)	((ID)(((rep)-tcp6_rep)+TMIN_TCP6_REPID))
#define GET_TCP4_REPID(rep)	((ID)(((rep)-tcp4_rep)+TMIN_TCP4_REPID))

#ifdef TCP_CFG_EXTENTIONS

#define VALID_TCP_REP(rep)	(((rep)->flags&TCP_REP_FLG_VALID)!=0)
#define DYNAMIC_TCP_REP(rep)	(((rep)->flags&TCP_REP_FLG_DYNAMIC)!=0)

#else	/* of #ifdef TCP_CFG_EXTENTIONS */

#define VALID_TCP_REP(rep)	(true)
#define DYNAMIC_TCP_REP(rep)	(false)

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

/* TCP 通信端点 */

#define VALID_TCP_CEPID(id)	(TMIN_TCP_CEPID<=(id)&&(id)<=tmax_tcp_cepid)
#define INDEX_TCP_CEP(id)	((id)-TMIN_TCP_CEPID)

#define GET_TCP_CEP(id)		(&(tcp_cep[INDEX_TCP_CEP(id)]))

#define GET_TCP_CEPID(cep)	((ID)(((cep)-tcp_cep)+TMIN_TCP_CEPID))
#define GET_TCP_TWCEPID(cep)	((ID)(((cep)-tcp_twcep)+TMIN_TCP_TWCEPID))

#define VALID_TCP_CEP(cep)	(((cep)->flags&TCP_CEP_FLG_VALID)!=0)
#define DYNAMIC_TCP_CEP(cep)	(((cep)->flags&TCP_CEP_FLG_DYNAMIC)!=0)
#define TCP_IS_CEP_IPV6(cep)	(((cep)->flags&TCP_CEP_FLG_IPV4)==0)
#define TCP_IS_CEP_IPV4(cep)	(((cep)->flags&TCP_CEP_FLG_IPV4)!=0)

#define IS_TFN_TCP_RCV(t)	((t)==TFN_TCP_ACP_CEP||(t)==TFN_TCP_RCV_DAT||\
				 (t)==TFN_TCP_CLS_CEP||\
				 (t)==TFN_TCP_RCV_BUF||(t)==TFN_TCP_REL_BUF||\
				 (t)==TFN_TCP_RCV_OOB)

#define VALID_TFN_TCP_CAN(t)	((t)==TFN_TCP_ACP_CEP||(t)==TFN_TCP_CON_CEP||\
				 (t)==TFN_TCP_CLS_CEP||\
				 (t)==TFN_TCP_SND_DAT||(t)==TFN_TCP_RCV_DAT||\
				 (t)==TFN_TCP_GET_BUF||(t)==TFN_TCP_RCV_BUF||\
				 (t)==TFN_TCP_SND_OOB||\
				 (t)==TFN_TCP_ALL)

#define VALID_URG_POINTER(urg)	(TCP_CFG_URG_OFFSET==0||urg!=0)

/*
 *  全域変数
 */

extern uint8_t tcp_initialized;

extern T_TCP_STATS tcp_stats;

/* TCP 受付口 */

extern T_TCP6_REP tcp6_rep[];
extern T_TCP4_REP tcp4_rep[];

#if defined(SUPPORT_INET6)
#define tcp_rep	tcp6_rep
#elif defined(SUPPORT_INET4)
#define tcp_rep	tcp4_rep
#endif

extern const ID tmax_tcp_repid;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
extern const ID tmax_tcp6_repid;
extern const ID tmax_tcp4_repid;
#else
#define tmax_tcp6_repid	tmax_tcp_repid
#define tmax_tcp4_repid	tmax_tcp_repid
#endif

/* TCP 通信端点 */

extern T_TCP_CEP tcp_cep[];
extern T_TCP_TWCEP tcp_twcep[];

extern const ID tmax_tcp_cepid;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
extern const ID tmax_tcp6_cepid;
extern const ID tmax_tcp4_cepid;
#else
#define tmax_tcp6_cepid	tmax_tcp_cepid
#define tmax_tcp4_cepid	tmax_tcp_cepid
#endif

/*
 *  Inline 関数
 */

#ifndef TCP_CFG_RWBUF_CSAVE

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能が
 *  無効の場合は何もしないためのダミーの関数
 */

/*
 *  tcp_free_rwbufq -- 受信ウィンドバッファキューを解放する。
 */

Inline void
tcp_free_rwbufq_ncs (T_TCP_CEP *cep)
{
	}

#endif	/* of #ifndef TCP_CFG_RWBUF_CSAVE */

#ifdef TCP_CFG_SWBUF_CSAVE_ONLY

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能のみが
 *  有効の場合は何もしないためのダミーの関数
 */

/*
 *  tcp_read_swbuf_cs -- 送信ウィンドバッファからデータを読み出す。
 */

Inline void
tcp_read_swbuf_cs (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff)
{
	}

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE_ONLY */

#ifndef TCP_CFG_SWBUF_CSAVE

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能が
 *  無効の場合は何もしないためのダミーの関数
 */

/*
 *  tcp_free_swbufq -- 送信ウィンドバッファキューを解放する。
 */

Inline void
tcp_free_swbufq_ncs (T_TCP_CEP *cep)
{
	}

/*
 *  tcp_alloc_swbuf -- 送信ウィンドバッファ用のネットワークバッファの割り当てを指示する。
 */

Inline void
tcp_alloc_swbuf_ncs (T_TCP_CEP *cep)
{
	}

/*
 *  tcp_push_res_nbuf -- ネットワークバッファを予約する。
 */

Inline T_NET_BUF *
tcp_push_res_nbuf_ncs (T_NET_BUF *nbuf)
{
	return nbuf;
	}

/*
 *  tcp_pull_res_nbuf -- 予約してあるネットワークバッファ返す。
 */

Inline T_NET_BUF *
tcp_pull_res_nbuf_ncs (ATR nbatr)
{
	return NULL;
	}

#endif	/* of #ifndef TCP_CFG_SWBUF_CSAVE */

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能の切り替えマクロ
 */

#ifdef TCP_CFG_RWBUF_CSAVE

#ifdef TCP_CFG_RWBUF_CSAVE_ONLY

#define TCP_FREE_RWBUFQ(cep)			tcp_free_rwbufq_cs(cep)
#define TCP_DROP_RWBUF(cep,len)			tcp_drop_rwbuf_cs(cep,len)
#define TCP_READ_RWBUF(cep,data,len)		tcp_read_rwbuf_cs(cep,data,len)
#define TCP_GET_RWBUF_ADDR(cep,buf)		tcp_get_rwbuf_addr_cs(cep,buf)
#define TCP_WRITE_RWBUF(cep,input,thoff)	tcp_write_rwbuf_cs(cep,input,thoff)

#else	/* of #ifdef TCP_CFG_RWBUF_CSAVE_ONLY */

#define TCP_FREE_RWBUFQ(cep)			tcp_free_rwbufq(cep)
#define TCP_DROP_RWBUF(cep,len)			tcp_drop_rwbuf(cep,len)
#define TCP_READ_RWBUF(cep,data,len)		tcp_read_rwbuf(cep,data,len)
#define TCP_GET_RWBUF_ADDR(cep,buf)		tcp_get_rwbuf_addr(cep,buf)
#define TCP_WRITE_RWBUF(cep,input,thoff)	tcp_write_rwbuf(cep,input,thoff)

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE_ONLY */

#else	/* of #ifdef TCP_CFG_RWBUF_CSAVE */

#define TCP_FREE_RWBUFQ(cep)			tcp_free_rwbufq_ncs(cep)
#define TCP_DROP_RWBUF(cep,len)			tcp_drop_rwbuf_ncs(cep,len)
#define TCP_READ_RWBUF(cep,data,len)		tcp_read_rwbuf_ncs(cep,data,len)
#define TCP_GET_RWBUF_ADDR(cep,buf)		tcp_get_rwbuf_addr_ncs(cep,buf)
#define TCP_WRITE_RWBUF(cep,input,thoff)	tcp_write_rwbuf_ncs(cep,input,thoff)

#endif	/* of #ifdef TCP_CFG_RWBUF_CSAVE */

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能の切り替えマクロ
 */

#ifdef TCP_CFG_SWBUF_CSAVE

#define TCP_PUSH_RES_NBUF(nbuf)			tcp_push_res_nbuf(nbuf)
#define TCP_PULL_RES_NBUF(atr)			tcp_pull_res_nbuf(atr)

#ifdef TCP_CFG_SWBUF_CSAVE_ONLY

#define TCP_FREE_SWBUFQ(cep)			tcp_free_swbufq_cs(cep)
#define TCP_ALLOC_SWBUF(cep)			tcp_alloc_swbuf_cs(cep)
#define TCP_DROP_SWBUF(cep,len)			tcp_drop_swbuf_cs(cep,len)
#define TCP_WRITE_SWBUF(cep,data,len)		tcp_write_swbuf_cs(cep,data,len)
#define TCP_READ_SWBUF(cep,output,len,doff)	tcp_read_swbuf_cs(cep,output,len,doff)
#define TCP_WAIT_SWBUF(cep,tmout)		tcp_wait_swbuf_cs(cep,tmout)
#define TCP_GET_SWBUF_ADDR(cep,buf)		tcp_get_swbuf_addr_cs(cep,buf)
#define TCP_SEND_SWBUF(cep,len)			tcp_send_swbuf_cs(cep,len)
#define TCP_IS_SWBUF_FULL(cep)			tcp_is_swbuf_full_cs(cep)

#else	/* of #ifdef TCP_CFG_SWBUF_CSAVE_ONLY */

#define TCP_FREE_SWBUFQ(cep)			tcp_free_swbufq(cep)
#define TCP_ALLOC_SWBUF(cep)			tcp_alloc_swbuf(cep)
#define TCP_DROP_SWBUF(cep,len)			tcp_drop_swbuf(cep,len)
#define TCP_WRITE_SWBUF(cep,data,len)		tcp_write_swbuf(cep,data,len)
#define TCP_READ_SWBUF(cep,output,len,doff)	tcp_read_swbuf(cep,output,len,doff)
#define TCP_WAIT_SWBUF(cep,tmout)		tcp_wait_swbuf(cep,tmout)
#define TCP_GET_SWBUF_ADDR(cep,buf)		tcp_get_swbuf_addr(cep,buf)
#define TCP_SEND_SWBUF(cep,len)			tcp_send_swbuf(cep,len)
#define TCP_IS_SWBUF_FULL(cep)			tcp_is_swbuf_full(cep)

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE_ONLY */

#else	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

#define TCP_READ_SWBUF(cep,output,len,doff)	tcp_read_swbuf_ncs(cep,output,len,doff)
#define TCP_FREE_SWBUFQ(cep)			tcp_free_swbufq_ncs(cep)
#define TCP_ALLOC_SWBUF(cep)			tcp_alloc_swbuf_ncs(cep)
#define TCP_PUSH_RES_NBUF(nbuf)			tcp_push_res_nbuf_ncs(nbuf)
#define TCP_PULL_RES_NBUF(atr)			tcp_pull_res_nbuf_ncs(atr)
#define TCP_DROP_SWBUF(cep,len)			tcp_drop_swbuf_ncs(cep,len)
#define TCP_WRITE_SWBUF(cep,data,len)		tcp_write_swbuf_ncs(cep,data,len)
#define TCP_WAIT_SWBUF(cep,tmout)		tcp_wait_swbuf_ncs(cep,tmout)
#define TCP_GET_SWBUF_ADDR(cep,buf)		tcp_get_swbuf_addr_ncs(cep,buf)
#define TCP_SEND_SWBUF(cep,len)			tcp_send_swbuf_ncs(cep,len)
#define TCP_IS_SWBUF_FULL(cep)			tcp_is_swbuf_full_ncs(cep)

#endif	/* of #ifdef TCP_CFG_SWBUF_CSAVE */

/*
 *  関数
 */

extern uint_t tcp_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern void tcp_init (void);
extern void tcp_init_iss (void);
extern T_TCP_CEP *tcp_close (T_TCP_CEP *cep);
extern T_TCP_CEP *tcp_drop  (T_TCP_CEP *cep, ER err);
extern void tcp_respond (T_NET_BUF *nbuf, T_TCP_CEP *cep,
                         T_TCP_SEQ ack, T_TCP_SEQ seq, uint_t rbfree, uint8_t flags);
extern ER tcp_get_segment (T_NET_BUF **nbuf, T_TCP_CEP *cep,
                           uint_t optlen, uint_t len, uint_t maxlen, ATR nbatr, TMO tmout);
extern ER tcpn_get_segment (T_NET_BUF **nbuf, T_TCP_CEP *cep,
                            uint_t optlen, uint_t len, uint_t maxlen, ATR nbatr, TMO tmout);
extern void tcp_init_cep (T_TCP_CEP *cep);
extern void tcp_notify (T_NET_BUF *input, int code);
extern T_TCP_TIME tcp_rexmt_val (T_TCP_CEP *cep);
extern void tcp_set_persist_timer (T_TCP_CEP *cep);
extern void tcp_cancel_timers (T_TCP_CEP *cep);
extern void tcp_alloc_auto_port (T_TCP_CEP *cep);
extern ER tcp_alloc_port (T_TCP_CEP *cep, uint16_t portno);
extern ER tcp_lock_cep (ID cepid, T_TCP_CEP **p_cep, FN tfn);
extern void tcp_free_reassq (T_TCP_CEP *cep);
extern ER tcp_can_send_more (T_TCP_CEP *cep, FN fncd, TMO tmout);
extern ER tcp_can_recv_more (ER *error, T_TCP_CEP *cep, FN fncd, TMO tmout);
extern uint8_t tcp_move_ra2rw (T_TCP_CEP *cep, uint8_t flags);
extern uint8_t tcp_write_raque (T_NET_BUF *input, T_TCP_CEP *cep, uint_t thoff, uint8_t flags);
extern ER tcp_wait_rwbuf (T_TCP_CEP *cep, TMO tmout);
extern T_TCP_CEP *tcp_user_closed (T_TCP_CEP *cep);
extern T_TCP_CEP *tcp_find_cep (T_NET_BUF *input, uint_t off);
extern T_TCP_TWCEP*tcp_find_twcep (T_NET_BUF *input, uint_t off);
extern bool_t tcp_is_addr_accept (T_NET_BUF *input, uint_t off);
extern void tcp_move_twcep (T_TCP_CEP *cep);

/*
 *  TCP 通信端点の受信ウィンドバッファ用関数
 */

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能が有効の場合、
 *  関数の選択に使用する。
 */

extern void tcp_free_rwbufq (T_TCP_CEP *cep);
extern void tcp_drop_rwbuf (T_TCP_CEP *cep, uint_t len);
extern void tcp_write_rwbuf (T_TCP_CEP *cep, T_NET_BUF *input, uint_t tcpoff);
extern uint_t tcp_read_rwbuf (T_TCP_CEP *cep, void *data, uint_t len);
extern uint_t tcp_get_rwbuf_addr (T_TCP_CEP *cep, void **p_buf);

/*
 *  TCP 通信端点の受信ウィンドバッファの省コピー機能用関数
 */

extern void tcp_free_rwbufq_cs (T_TCP_CEP *cep);
extern void tcp_drop_rwbuf_cs (T_TCP_CEP *cep, uint_t len);
extern void tcp_write_rwbuf_cs (T_TCP_CEP *cep, T_NET_BUF *input, uint_t tcpoff);
extern uint_t tcp_read_rwbuf_cs (T_TCP_CEP *cep, void *data, uint_t len);
extern uint_t tcp_get_rwbuf_addr_cs (T_TCP_CEP *cep, void **p_buf);

/*
 *  TCP 通信端点の受信ウィンドバッファの【非】省コピー機能用関数
 */

extern void tcp_free_rwbufq_ncs (T_TCP_CEP *cep);
extern void tcp_drop_rwbuf_ncs (T_TCP_CEP *cep, uint_t len);
extern void tcp_write_rwbuf_ncs (T_TCP_CEP *cep, T_NET_BUF *input, uint_t tcpoff);
extern uint_t tcp_read_rwbuf_ncs (T_TCP_CEP *cep, void *data, uint_t len);
extern uint_t tcp_get_rwbuf_addr_ncs (T_TCP_CEP *cep, void **p_buf);

/*
 *  TCP 通信端点の送信ウィンドバッファ用関数
 */

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能が有効の場合、
 *  関数の選択に使用する。
 */

extern void tcp_free_swbufq (T_TCP_CEP *cep);
extern void tcp_drop_swbuf (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_write_swbuf (T_TCP_CEP *cep, void *data, uint_t len);
extern void tcp_read_swbuf (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff);
extern void tcp_send_swbuf (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_get_swbuf_addr (T_TCP_CEP *cep, void **p_buf);
extern ER tcp_wait_swbuf (T_TCP_CEP *cep, TMO tmout);
extern T_NET_BUF *tcp_push_res_nbuf (T_NET_BUF *buf);
extern T_NET_BUF *tcp_pull_res_nbuf (ATR nbatr);
extern void tcp_alloc_swbuf (T_TCP_CEP *cep);
extern bool_t tcp_is_swbuf_full (T_TCP_CEP *cep);
extern void tcp_init_swbuf (T_TCP_CEP *cep);

/*
 *  TCP 通信端点の送信ウィンドバッファの省コピー機能用関数
 */

extern void tcp_free_swbufq_cs (T_TCP_CEP *cep);
extern void tcp_drop_swbuf_cs (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_write_swbuf_cs (T_TCP_CEP *cep, void *data, uint_t len);
extern void tcp_read_swbuf_cs (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff);
extern void tcp_send_swbuf_cs (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_get_swbuf_addr_cs (T_TCP_CEP *cep, void **p_buf);
extern ER tcp_wait_swbuf_cs (T_TCP_CEP *cep, TMO tmout);
extern void tcp_alloc_swbuf_cs (T_TCP_CEP *cep);
extern bool_t tcp_is_swbuf_full_cs (T_TCP_CEP *cep);

/*
 *  TCP 通信端点の送信ウィンドバッファの【非】省コピー機能用関数
 */

extern void tcp_free_swbufq_ncs (T_TCP_CEP *cep);
extern void tcp_drop_swbuf_ncs (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_write_swbuf_ncs (T_TCP_CEP *cep, void *data, uint_t len);
extern void tcp_read_swbuf_ncs (T_TCP_CEP *cep, T_NET_BUF *output, uint_t len, uint_t doff);
extern void tcp_send_swbuf_ncs (T_TCP_CEP *cep, uint_t len);
extern ER_UINT tcp_get_swbuf_addr_ncs (T_TCP_CEP *cep, void **p_buf);
extern ER tcp_wait_swbuf_ncs (T_TCP_CEP *cep, TMO tmout);
extern T_NET_BUF *tcp_push_res_nbuf_ncs (T_NET_BUF *buf);
extern T_NET_BUF *tcp_pull_res_nbuf_ncs (ATR nbatr);
extern void tcp_alloc_swbuf_ncs (T_TCP_CEP *cep);
extern bool_t tcp_is_swbuf_full_ncs (T_TCP_CEP *cep);

/*
 *  TCP ヘッダのトレース出力機能用関数
 */
extern void tcp_output_trace (T_NET_BUF *output, T_TCP_CEP *cep);
extern void tcp_input_trace  (T_NET_BUF *input,  T_TCP_CEP *cep);

/* TCP 出力タスク */

extern void	tcp_output_task(intptr_t exinf);

#endif	/* of #ifndef _TCP_VAR_H_ */
