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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 * $FreeBSD: src/sys/netinet/ip_var.h,v 1.45.2.1 1999/08/29 16:29:51 peter Exp $
 */
 
#ifndef _IP4_VAR_H_
#define _IP4_VAR_H_

/*
 *  IPv4 ヘッダ・ペイロードアクセスマクロ
 */

#define IF_IP4_HDR_SIZE			(IF_HDR_SIZE + IP4_HDR_SIZE)

#define GET_IP4_HDR(nbuf)		((T_IP4_HDR*)((nbuf)->buf + IF_HDR_SIZE))
#define GET_IP4_SDU(nbuf)		((uint8_t*)((nbuf)->buf) + GET_IF_IP4_HDR_SIZE(nbuf))

#define GET_IP4_HDR_SIZE(nbuf)		((uint8_t)IP4_VHL_HL(GET_IP4_HDR(nbuf)->vhl) << 2)
#define GET_IP4_SDU_SIZE(nbuf)		(ntohs(GET_IP4_HDR(nbuf)->len) - GET_IP4_HDR_SIZE(nbuf))
#define GET_IF_IP4_HDR_SIZE(nbuf)	(IF_HDR_SIZE + (GET_IP4_HDR_SIZE(nbuf)))

#define SET_IP4_SDU_SIZE(nbuf,slen)	(GET_IP4_HDR(nbuf)->len=(uint16_t)htons((slen) + IP4_HDR_SIZE))

/*
 *  IP の MSS サイズのネットワークバッファ
 */

typedef struct t_net_buf_ip_mss {
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
	uint8_t		buf[IF_HDR_SIZE + IP4_MSS];
				/* バッファ本体		*/
	} T_NET_BUF_IP_MSS;

/*
 *  65536 オクテットサイズのネットワークバッファ
 */

typedef struct t_net_buf4_65536 {
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
	uint8_t		buf[IF_IP4_HDR_SIZE + 65536];
				/* バッファ本体		*/
#if defined(IF_PDU_HDR_PADDING)
	uint8_t		padding[IF_PDU_HDR_PADDING];
#endif				/* ヘッダの余分		*/
	} T_NET_BUF4_65536;

#ifdef IP4_CFG_FRAGMENT

/*
 *  再構成キュー用 IPv4 ヘッダの定義
 */

typedef struct t_qip4_hdr {
	uint8_t		vhl;		/* version << 4 | header length >> 2	*/
					/* length は 4 octets 単位		*/
	uint8_t		type;		/* type of services			*/
	uint16_t	len;		/* total length				*/

	uint16_t	id;		/* identification			*/
	uint16_t	flg_off;	/* fragment flag & offset		*/

	uint8_t		ttl;		/* time to live				*/
	uint8_t		proto;		/* protocol				*/
	uint16_t	sum;		/* checksum				*/

	uint32_t	src;		/* source address			*/

	T_NET_BUF	*next;		/* 次の IPv4 フラグメント			*/
	} __attribute__((packed, aligned(2))) T_QIP4_HDR;

#define QIP4_HDR_SIZE			(sizeof(T_QIP4_HDR))
#define IF_QIP4_HDR_SIZE		(IF_HDR_SIZE + QIP4_HDR_SIZE)

#define GET_QIP4_HDR(nbuf)		((T_QIP4_HDR*)((nbuf)->buf + IF_HDR_SIZE))
#define GET_QIP4_SDU(nbuf)		((uint8_t*)(nbuf)->buf + GET_IF_QIP4_HDR_SIZE(nbuf))

#define GET_QIP4_HDR_SIZE(iph)		((uint8_t)IP4_VHL_HL((iph)->vhl) << 2)
#define GET_IF_QIP4_HDR_SIZE(nbuf)	(IF_HDR_SIZE + (GET_QIP4_HDR_SIZE(GET_QIP4_HDR(nbuf))))

/*
 *  データグラム再構成用ネットワークバッファ
 */

typedef struct t_net_buf4_reassm {
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
	uint8_t		align[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[IP4_CFG_FRAG_REASSM_SIZE];
				/* バッファ本体		*/
	} __attribute__((packed, aligned(2))) T_NET_BUF4_REASSM;

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

/*
 *  TCP の再構成キューにおける IPv4 ヘッダの定義
 */

typedef struct t_ip4_tcp_q_hdr {
	uint8_t		vhl;		/* version << 4 | header length >> 2	*/
					/* length は 4 octets 単位		*/
	uint8_t		type;		/* type of services			*/
	uint16_t	len;		/* total length				*/

	uint16_t	id;		/* identification			*/
	uint16_t	flg_off;	/* fragment flag & offset		*/

	uint8_t		ttl;		/* time to live				*/
	uint8_t		proto;		/* protocol				*/
	uint16_t	thoff;		/* TCP ヘッダのオフセット		*/

	T_IN4_ADDR	src;		/* source address			*/
	T_IN4_ADDR	dst;		/* destination address			*/
	} __attribute__((packed, aligned(2))) T_IP4_TCP_Q_HDR;

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

typedef struct t_ip_stats {
	uint32_t	ipInReceives;
	uint32_t	ipInHdrErrors;
	uint32_t	ipInAddrErrors;
/*	uint32_t	ipForwDatagrams;*/
	uint32_t	ipInUnknownProtos;
/*	uint32_t	ipInDiscards;*/
	uint32_t	ipInDelivers;
	uint32_t	ipOutRequests;
	uint32_t	ipOutDiscards;
/*	uint32_t	ipOutNoRoutes;*/

#ifdef IP4_CFG_FRAGMENT

	uint32_t	ipReasmReqds;
	uint32_t	ipReasmOKs;
	uint32_t	ipReasmFails;
	uint32_t	ipFragOKs;
	uint32_t	ipFragFails;
	uint32_t	ipFragCreates;

#else	/* of #ifdef IP4_CFG_FRAGMENT */

	uint32_t	ipReasmReqds;
/*	uint32_t	ipReasmOKs;*/
	uint32_t	ipReasmFails;
/*	uint32_t	ipFragOKs;*/
/*	uint32_t	ipFragFails;*/
/*	uint32_t	ipFragCreates;*/

#endif	/* of #ifdef IP4_CFG_FRAGMENT */

} T_IP_STATS;

extern T_IP_STATS ip_stats;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  関数
 */

extern void ip_frag_timer (void);
extern void ip_init (void);
extern ER ip_remove_options (T_NET_BUF *nbuf);
extern ER ip_output (T_NET_BUF *nbuf, TMO tmout);
extern void ip_exchg_addr (T_NET_BUF *nbuf);

#endif	/* of #ifndef _IP4_VAR_H_ */
