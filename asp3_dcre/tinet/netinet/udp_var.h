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
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	@(#)udp_var.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: src/sys/netinet/udp_var.h,v 1.16.2.2 1999/08/29 16:29:59 peter Exp $
 */
 
#ifndef _UDP_VAR_H_
#define _UDP_VAR_H_

/*
 *  UDP ヘッダ・ペイロードアクセスマクロ
 */

#define GET_UDP_HDR(nbuf,uhoff)		((T_UDP_HDR*)((uint8_t*)((nbuf)->buf) + uhoff))
#define GET_UDP_HDR_OFFSET(nbuf)	(GET_IF_IP_HDR_SIZE(nbuf))
#define GET_UDP_SDU(nbuf,uhoff)		((uint8_t*)((nbuf)->buf) + uhoff+ UDP_HDR_SIZE)

#define IF_IP_UDP_HDR_SIZE(nbuf)	(IF_IP_HDR_SIZE(nbuf) + UDP_HDR_SIZE)
#define IF_IP_UDP_HDR_OFFSET(nbuf)	(IF_IP_HDR_SIZE(nbuf))

/*
 *  TINET 1.1 との互換性のための定義
 */

#if defined(_IP4_CFG)

#define IP4_UDP_HDR_SIZE		IP_UDP_HDR_SIZE
#define IF_IP4_UDP_HDR_SIZE		IF_IP_UDP_HDR_SIZE
#define GET_IP4_UDP_HDR_SIZE(nbuf)	GET_IP_UDP_HDR_SIZE(nbuf)	
#define GET_IF_IP4_UDP_HDR_SIZE(nbuf)	GET_IF_IP_UDP_HDR_SIZE(nbuf)

#endif	/* of #if defined(_IP4_CFG) */

/*
 *  コールバック関数の定義
 */

typedef ER	(*t_udp_callback)(ID cepid, FN fncd, void *p_parblk);

typedef struct t_udp_rcv_dat_para {
	uint_t		len;
	T_NET_BUF	*input;
	uint_t		off;
	union {
		T_IPV4EP rep4;
		T_IPV6EP rep6;
	};
	} T_UDP_RCV_DAT_PARA;

/*
 *  UDP 通信端点
 */

/* IPv6 用 UDP 通信端点 */

typedef struct t_udp6_cep {

	/*
	 * ITRON TCP/IP API、TCP 通信端点と同じメンバ
	 */
	ATR		cepatr;		/* UDP 通信端点属性		*/
	T_IPV6EP	myaddr;		/* 自分のアドレス		*/
	t_udp_callback	callback;	/* コールバック関数		*/

	/*
	 * TINET 独自のメンバ
	 */
	uint16_t	flags;		/* 通信端点フラグ		*/
	ID		semid_lock;	/* 通信端点ロック		*/
	ID		snd_tskid;	/* 送信タスク識別子		*/
	ID		rcv_tskid;	/* 受信タスク識別子		*/
	ID		rcvqid;		/* 非コールバック用受信キュー識別子*/
	T_NET_BUF	*cb_netbuf;	/* コールバック用受信ネットワークバッファ*/

#ifdef UDP_CFG_NON_BLOCKING

	T_IPV6EP 	*snd_p_dstaddr;	/* 送信相手のアドレスへのポインタ*/
	T_IPV6EP 	*rcv_p_dstaddr;	/* 受信相手のアドレスへのポインタ*/
	void		*snd_data;	/* 送信データ領域の先頭アドレス	*/
	int_t		snd_len;	/* 送信データ領域の長さ		*/
	void		*rcv_data;	/* 受信データ領域の先頭アドレス	*/
	int_t		rcv_len;	/* 受信データ領域の長さ		*/

#endif	/* of #ifdef UDP_CFG_NON_BLOCKING */

#ifdef UDP_CFG_EXTENTIONS
	ER		error;		/* 待ち中に発生したエラー	*/
#endif

#ifdef SUPPORT_IGMP
	bool_t igmp_loopback;
	uint8_t igmp_ttl;
	T_IN4_ADDR 	igmp_mcaddr;	/*  */
#endif

	} T_UDP6_CEP;

/* IPv4 用 UDP 通信端点 */

typedef struct t_udp4_cep {

	/*
	 * ITRON TCP/IP API、TCP 通信端点と同じメンバ
	 */
	ATR		cepatr;		/* UDP 通信端点属性		*/
	T_IPV4EP	myaddr;		/* 自分のアドレス		*/
	t_udp_callback	callback;	/* コールバック関数		*/

	/*
	 * TINET 独自のメンバ
	 */
	uint16_t	flags;		/* 通信端点フラグ		*/
	ID		semid_lock;	/* 通信端点ロック		*/
	ID		snd_tskid;	/* 送信タスク識別子		*/
	ID		rcv_tskid;	/* 受信タスク識別子		*/
	ID		rcvqid;		/* 非コールバック用受信キュー識別子*/
	T_NET_BUF	*cb_netbuf;	/* コールバック用受信ネットワークバッファ*/

#ifdef UDP_CFG_NON_BLOCKING

	T_IPV4EP 	*snd_p_dstaddr;	/* 送信相手のアドレスへのポインタ*/
	T_IPV4EP	*rcv_p_dstaddr;	/* 受信相手のアドレスへのポインタ*/
	void		*snd_data;	/* 送信データ領域の先頭アドレス	*/
	int_t		snd_len;	/* 送信データ領域の長さ		*/
	void		*rcv_data;	/* 受信データ領域の先頭アドレス	*/
	int_t		rcv_len;	/* 受信データ領域の長さ		*/

#endif	/* of #ifdef UDP_CFG_NON_BLOCKING */

#ifdef UDP_CFG_EXTENTIONS
	ER		error;		/* 待ち中に発生したエラー	*/
#endif

#ifdef SUPPORT_IGMP
	bool_t igmp_loopback;
	uint8_t igmp_ttl;
	T_IN4_ADDR 	igmp_mcaddr;	/*  */
#endif

	} T_UDP4_CEP;

#if defined(_IP6_CFG)

#define T_UDP_CEP	T_UDP6_CEP

#else	/* of #if defined(_IP6_CFG) */

#if defined(_IP4_CFG)

#define T_UDP_CEP	T_UDP4_CEP

#endif	/* of #if defined(_IP4_CFG) */

#endif	/* of #if defined(_IP6_CFG) */

/*
 *  UDP 通信端点フラグ
 */

#define UDP_CEP_FLG_POST_OUTPUT	UINT_C(0x0200)	/* 送信を開始する。		*/
#define UDP_CEP_FLG_DYNAMIC	UINT_C(0x0400)	/* 動的生成・削除可能通信端点。	*/
#define UDP_CEP_FLG_VALID	UINT_C(0x0800)	/* 生成済みで有効な通信端点。	*/

/*
 *  オブジェクト ID の最小値の定義
 */

#define	TMIN_UDP_CEPID		1			/* UDP      通信端点 ID の最小値 */
#define	TMIN_UDP6_CEPID		1			/* UDP/IPv6 通信端点 ID の最小値 */
#define	TMIN_UDP4_CEPID		(TNUM_UDP6_CEPID+1)	/* UDP/IPv4 通信端点 ID の最小値 */

/*
 *  SNMP の 管理情報ベース (MIB)
 */

typedef struct t_udp_stats {
	uint32_t	udpInDatagrams;
	uint32_t	udpNoPorts;
	uint32_t	udpInErrors;
	uint32_t	udpOutDatagrams;
} T_UDP_STATS;

/*
 *  関数シミュレーションマクロ
 */

#define VALID_UDP6_CEPID(id)	(TMIN_UDP6_CEPID<=(id)&&(id)<=tmax_udp6_cepid)
#define VALID_UDP4_CEPID(id)	(TMIN_UDP4_CEPID<=(id)&&(id)<=tmax_udp4_cepid)

#define INDEX_UDP6_CEP(id)	((id)-TMIN_UDP6_CEPID)
#define INDEX_UDP4_CEP(id)	((id)-TMIN_UDP4_CEPID)

#define GET_UDP6_CEP(id)	(&(udp6_cep[INDEX_UDP6_CEP(id)]))
#define GET_UDP4_CEP(id)	(&(udp4_cep[INDEX_UDP4_CEP(id)]))

#define GET_UDP6_CEPID(cep)	((ID)(((cep)-udp6_cep)+TMIN_UDP6_CEPID))
#define GET_UDP4_CEPID(cep)	((ID)(((cep)-udp4_cep)+TMIN_UDP4_CEPID))

#define VALID_TFN_UDP_CAN(t)	((t)==TFN_UDP_SND_DAT||(t)==TFN_UDP_RCV_DAT||\
				 (t)==TFN_UDP_ALL)

#define VALID_UDP_CEP(cep)	(((cep)->flags&UDP_CEP_FLG_VALID)!=0)
#define DYNAMIC_UDP_CEP(cep)	(((cep)->flags&UDP_CEP_FLG_DYNAMIC)!=0)

/*
 *  全域変数
 */

extern T_UDP_STATS udp_stats;

/* UDP 通信端点 */

extern T_UDP6_CEP udp6_cep[];
extern T_UDP4_CEP udp4_cep[];

#if defined(SUPPORT_INET6)
#define udp_cep	udp6_cep
#elif defined(SUPPORT_INET4)
#define udp_cep	udp4_cep
#endif

extern const ID tmax_udp_cepid;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
extern const ID tmax_udp6_cepid;
extern const ID tmax_udp4_cepid;
#else
#define tmax_udp6_cepid	tmax_udp_cepid
#define tmax_udp4_cepid	tmax_udp_cepid
#endif

/*
 *  TINET 1.3 との互換性のための定義
 */

#define	TMIN_UDP_CCEPID		TMIN_UDP_CEPID
#define	TNUM_UDP_CCEPID		TNUM_UDP_CEPID

/*
 *  関数
 */

/* IPv6 */

extern uint_t udp6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern ER_UINT udp6_send_data (T_UDP6_CEP *cep, T_IPV6EP *p_dstaddr,
                               void *data, int_t len, TMO tmout);
extern ER udp6_can_snd (T_UDP6_CEP *cep, ER error);
extern ER udp6_can_rcv (T_UDP6_CEP *cep, ER error);
extern ER udp6_alloc_auto_port (T_UDP6_CEP *cep);
extern ER udp6_alloc_port (T_UDP6_CEP *cep, uint16_t portno);
extern T_UDP6_CEP*udp6_find_cep (T_NET_BUF *input, uint_t off);
extern void udp6_notify (T_NET_BUF *input, ER error);
extern void udp6_input_select (T_UDP6_CEP *cep, T_NET_BUF *input, uint_t off);

/* IPv4 */

extern uint_t udp4_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern ER_UINT udp4_send_data (T_UDP4_CEP *cep, T_IPV4EP *p_dstaddr,
                               void *data, int_t len, TMO tmout);
extern ER udp4_can_snd (T_UDP4_CEP *cep, ER error);
extern ER udp4_can_rcv (T_UDP4_CEP *cep, ER error);
extern ER udp4_alloc_auto_port (T_UDP4_CEP *cep);
extern ER udp4_alloc_port (T_UDP4_CEP *cep, uint16_t portno);
extern T_UDP4_CEP*udp4_find_cep (T_NET_BUF *input, uint_t off);
extern void udp4_notify (T_NET_BUF *input, ER error);
extern void udp4_input_select (T_UDP4_CEP *cep, T_NET_BUF *input, uint_t off);

/* ノンブロッキングコールを行う場合に組み込むタスク */

extern void	udp_output_task(intptr_t exinf);

#endif	/* of #ifndef _UDP_VAR_H_ */
