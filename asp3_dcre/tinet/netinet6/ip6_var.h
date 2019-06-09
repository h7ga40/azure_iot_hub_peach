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

/*	$FreeBSD: src/sys/netinet6/ip6_var.h,v 1.10 2002/10/16 01:54:45 sam Exp $	*/
/*	$KAME: ip6_var.h,v 1.62 2001/05/03 14:51:48 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
 *	@(#)ip_var.h	8.1 (Berkeley) 6/10/93
 */
 
#ifndef _IP6_VAR_H_
#define _IP6_VAR_H_

/*
 *  IPv6 ヘッダ・ペイロードアクセスマクロ
 */

#define IF_IP6_HDR_SIZE			(IF_HDR_SIZE + IP6_HDR_SIZE)

#define GET_IP6_HDR(nbuf)		((T_IP6_HDR*)((nbuf)->buf + IF_HDR_SIZE))
#define GET_IP6_SDU(nbuf)		((uint8_t*)((nbuf)->buf) + GET_IF_IP6_HDR_SIZE(nbuf))
#define GET_IP6_NEXT_HDR(nbuf)		((uint8_t*)((nbuf)->buf) + IF_IP6_HDR_SIZE)

#define GET_IP6_HDR_SIZE(nbuf)		(get_ip6_hdr_size(nbuf))
#define GET_IP6_SDU_SIZE(nbuf)		(ntohs(GET_IP6_HDR(nbuf)->plen))
#define GET_IF_IP6_HDR_SIZE(nbuf)	(IF_HDR_SIZE + (GET_IP6_HDR_SIZE(nbuf)))

#define SET_IP6_CF(nbuf,cf)		(GET_IP6_HDR(nbuf)->vcf=htonl(IP6_MAKE_VCF(IP6_VCF_V(ntohl(GET_IP6_HDR(nbuf)->vcf)),cf)))
#define SET_IP6_SDU_SIZE(nbuf,slen)	(GET_IP6_HDR(nbuf)->plen=htons(slen))

/* ip6_output のフラグ */

#define IPV6_OUT_FLG_DAD		0x0100	/* 重複アドレス検出	*/
#define IPV6_OUT_FLG_HOP_LIMIT		0x0200	/* ホップリミット指定	*/

/* ホップリミットフラグが付く時、ホップリミット値を操作するマクロ */

#define IPV6_OUT_GET_HOP_LIMIT(f)	((f)&0x00ff)
#define IPV6_OUT_SET_HOP_LIMIT(f,l)	((f)|(l))

/*
 *  ip6_unknown_opt -- 未知のオプションの処理
 *
 *    戻り値:
 *
 *      0 以上 			正常、オプション長。
 *      IP6_OPT_RET_ERR (-1)	エラー、呼出側でネットワークバッファを開放する。
 *      IP6_OPT_RET_REL (-2)	エラー、ネットワークバッファは開放されている。
 */

#define IP6_OPT_RET_ERR		(-1)
#define IP6_OPT_RET_REL		(-2)

/*
 *  IPv6 の MMTU サイズのネットワークバッファ
 */

#if defined(IF_HDR_SIZE)

typedef struct t_net_buf_ipv6_mmtu {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		unit;	/* インタフェース番号	*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[IF_HDR_SIZE + IPV6_MMTU];
				/* バッファ本体		*/
	} T_NET_BUF_IPV6_MMTU;

#endif	/* of #if defined(IF_HDR_SIZE) */

/*
 *  65536 オクテットサイズのネットワークバッファ
 */

typedef struct t_net_buf6_65536 {
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
	uint8_t		buf[IF_IP6_HDR_SIZE + 65536];
				/* バッファ本体		*/
#if defined(IF_PDU_HDR_PADDING)
	uint8_t		padding[IF_PDU_HDR_PADDING];
#endif				/* ヘッダの余分		*/
	} T_NET_BUF6_65536;

#ifdef IP6_CFG_FRAGMENT

/*
 *  再構成キュー用 IPv6 ヘッダの定義
 */

typedef struct t_qip6_hdr {
	uint32_t		vcf;		/* Version:        4 bit		*/
					/* Traffic Class:  8 bit		*/
					/* Flow Label:    20 bit		*/
	uint16_t	plen;		/* Payload Length			*/
	uint8_t		next;		/* Next Header				*/
	uint8_t		hlim;		/* Hop Limit				*/
	T_IN6_ADDR	src;		/* Source Address			*/
	T_NET_BUF	*next_frag;	/* 次の IPv6 フラグメント	*/
	uint16_t	foff;		/* 断片ヘッダのオフセット		*/
	uint16_t	flen;		/* 断片長				*/
	uint16_t	ftim;		/* 再構成タイムアウト			*/
	uint16_t	dummy1;
	uint8_t		dummy2[sizeof(T_IN6_ADDR) - (sizeof(T_NET_BUF*) + sizeof(uint16_t) * 4)];
	} __attribute__((packed, aligned(2)))T_QIP6_HDR;

#define QIP6_HDR_SIZE			(sizeof(T_QIP6_HDR))
#define IF_QIP6_HDR_SIZE		(IF_HDR_SIZE + QIP6_HDR_SIZE)

#define GET_QIP6_HDR(nbuf)		((T_QIP6_HDR*)((nbuf)->buf + IF_HDR_SIZE))
#define GET_QIP6_SDU(nbuf)		((uint8_t*)(nbuf)->buf + GET_IF_QIP6_HDR_SIZE(nbuf))

#define GET_QIP6_HDR_SIZE(iph)		((uint8_t)IP6_VHL_HL((iph)->vhl) << 2)
#define GET_IF_QIP6_HDR_SIZE(nbuf)	(IF_HDR_SIZE + (GET_QIP6_HDR_SIZE(GET_QIP6_HDR(nbuf))))

/*
 *  データグラム再構成用ネットワークバッファ
 */

typedef struct t_net_buf6_reassm {
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
	uint8_t		buf[IP6_CFG_FRAG_REASSM_SIZE];
				/* バッファ本体		*/
	} T_NET_BUF6_REASSM;

#endif	/* of #ifdef IP6_CFG_FRAGMENT */

/*
 *  TCP の再構成キューにおける IPv6 ヘッダの定義
 */

typedef struct t_ip6_tcp_q_hdr {
	uint8_t vc;		/* Version:        4 bit		*/
	uint8_t cf;		/* Traffic Class:  8 bit		*/
			/* Flow Label:     4 bit (20 bit)	*/
	uint16_t thoff;	/* TCP ヘッダのオフセット		*/
	uint16_t plen;	/* Payload Length			*/
	uint8_t next;	/* Next Header				*/
	uint8_t hlim;	/* Hop Limit				*/
	T_IN6_ADDR src;	/* Source Address			*/
	T_IN6_ADDR dst;	/* Destination Address			*/
	} __attribute__((packed, aligned(2))) T_IP6_TCP_Q_HDR;

#ifdef SUPPORT_MIB

/*
 *  SNMP の 管理情報ベース (MIB)
 */

typedef struct t_in6_ifstat {
	uint32_t	ipv6IfStatsInReceives;
	uint32_t	ipv6IfStatsInHdrErrors;
/*	uint32_t	ipv6IfStatsInTooBigErrors;*/
/*	uint32_t	ipv6IfStatsInNoRoutes;*/
	uint32_t	ipv6IfStatsInAddrErrors;
	uint32_t	ipv6IfStatsInUnknownProtos;
	uint32_t	ipv6IfStatsInTruncatedPkts;
/*	uint32_t	ipv6IfStatsInDiscards;*/
/*	uint32_t	ipv6IfStatsInDelivers;*/
/*	uint32_t	ipv6IfStatsOutForwDatagrams;*/
	uint32_t	ipv6IfStatsOutRequests;
	uint32_t	ipv6IfStatsOutDiscards;
	uint32_t	ipv6IfStatsOutFragOKs;
	uint32_t	ipv6IfStatsOutFragFails;
	uint32_t	ipv6IfStatsOutFragCreates;
	uint32_t	ipv6IfStatsReasmReqds;
	uint32_t	ipv6IfStatsReasmOKs;
	uint32_t	ipv6IfStatsReasmFails;
	uint32_t	ipv6IfStatsInMcastPkts;
	uint32_t	ipv6IfStatsOutMcastPkts;
} T_IN6_IFSTAT;

extern T_IN6_IFSTAT in6_ifstat;

#endif	/* of #ifdef SUPPORT_MIB */

/*
 *  変数
 */

extern uint8_t	ip6_defhlim;

/*
 *  関数
 */

extern ER ip6_output (T_NET_BUF *output, uint16_t flags, TMO tmout);
extern void ip6_init (void);
extern uint8_t *ip6_get_prev_hdr (T_NET_BUF *nbuf, uint_t off);
extern uint_t get_ip6_hdr_size (T_NET_BUF *nbuf);
extern ER_UINT ip6_nexthdr (T_NET_BUF *nbuf, uint_t off, uint_t proto, uint_t *nextp);
extern ER_UINT ip6_lasthdr (T_NET_BUF *nbuf, uint_t off, uint_t proto, uint_t *nextp);
extern ER_UINT ip6_unknown_opt (T_NET_BUF *input, uint8_t *opt);
extern uint_t dest6_input  (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern uint_t route6_input (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern uint_t frag6_input  (T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);
extern void frag6_timer (void);
extern ER ip6_remove_exthdrs (T_NET_BUF *nbuf);

#endif	/* of #ifndef _IP6_VAR_H_ */
