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
 *	@(#)if_ether.h	8.3 (Berkeley) 5/2/95
 * $FreeBSD: src/sys/netinet/if_ether.h,v 1.21.2.1 1999/08/29 16:29:31 peter Exp $
 */

#ifndef _IF_ETHER_H_
#define _IF_ETHER_H_

#ifdef SUPPORT_ETHER

/*
 *  Ethernet Address Resolution Protocol.
 *
 *  RFC 826 参照
 */

typedef struct t_ether_arp_hdr {
	uint8_t		shost[ETHER_ADDR_LEN];	/* 送信ホストの物理アドレス	*/
	uint8_t		sproto[IPV4_ADDR_LEN];	/* 送信ホストの IP アドレス	*/
	uint8_t		thost[ETHER_ADDR_LEN];	/* 解決対象物理アドレス		*/
	uint8_t		tproto[IPV4_ADDR_LEN];	/* 解決対象 IP アドレス		*/
	} __attribute__((packed, aligned(2)))T_ETHER_ARP_HDR;

#define ETHER_ARP_HDR_SIZE		(sizeof(T_ETHER_ARP_HDR))
#define IF_ARP_ETHER_HDR_SIZE		(IF_HDR_SIZE + ARP_HDR_SIZE + ETHER_ARP_HDR_SIZE)

#define GET_ETHER_ARP_HDR(nbuf)		((T_ETHER_ARP_HDR*)((nbuf)->buf + GET_IF_ARP_HDR_SIZE(nbuf)))

/*
 *  ARP キャッシュエントリ
 */

typedef struct arp_entry {
	T_NET_BUF	*hold;		/* ARP の解決待ち出力フレーム	*/
	T_IN4_ADDR	ip_addr;	/* IP アドレス			*/
	uint16_t	expire;		/* 破棄までの時間、0 なら未使用	*/
	uint8_t		mac_addr[ETHER_ADDR_LEN];	/* MAC アドレス	*/
	} __attribute__((packed, aligned(2)))T_ARP_ENTRY;

/*
 *  関数
 */

extern T_ARP_ENTRY *arp_lookup (T_IN4_ADDR addr, bool_t create);
extern ER arp_request (T_IF_ADDR *ifaddr, T_IN4_ADDR dst);
extern void arp_input (T_IF_ADDR *ifaddr, T_NET_BUF *input);
extern bool_t arp_resolve (T_IF_ADDR *ifaddr, T_NET_BUF *output, T_IN4_ADDR gw);
extern void arp_init (void);
extern const T_ARP_ENTRY *arp_get_cache (void);

#ifdef ETHER_CFG_MULTICAST
extern void ether_map_ipv4_multicast (T_ETHER_ADDR *eaddr, T_IN4_ADDR maddr);
#endif /* ETHER_CFG_MULTICAST */

/*
 *  TINET 独自 API
 */

#ifdef ARP_CFG_CALLBACK_DUPLICATED

extern bool_t arp_callback_duplicated(uint8_t *shost);

#endif	/* of #ifdef ARP_CFG_CALLBACK_DUPLICATED */

#endif	/* of #ifdef SUPPORT_ETHER */

#endif	/* of #ifndef _IF_ETHER_H_ */
