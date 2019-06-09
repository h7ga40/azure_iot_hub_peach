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

/*	$FreeBSD: src/sys/netinet6/in6_var.h,v 1.3.2.2 2001/07/03 11:01:52 ume Exp $	*/
/*	$KAME: in6_var.h,v 1.56 2001/03/29 05:34:31 itojun Exp $	*/

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
 * Copyright (c) 1985, 1986, 1993
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
 *	@(#)in_var.h	8.1 (Berkeley) 6/10/93
 */

#ifndef _IN6_VAR_H_
#define _IN6_VAR_H_

/*
 *  アドレスの定義
 */

#ifdef _NET_CFG_BYTE_ORDER

#if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN

#define IPV6_ADDR_INT32_ONE		ULONG_C(0x00000001)
#define IPV6_ADDR_INT32_TWO		ULONG_C(0x00000002)
#define IPV6_ADDR_INT32_0000FFFF	ULONG_C(0x0000ffff)
#define IPV6_ADDR_INT32_MNL		ULONG_C(0xff010000)
#define IPV6_ADDR_INT32_MLL		ULONG_C(0xff020000)
#define IPV6_ADDR_INT32_ULL		ULONG_C(0xfe800000)
#define IPV6_ADDR_INT16_ULL		UINT_C(0xfe80)
#define IPV6_ADDR_INT16_USL		UINT_C(0xfec0)
#define IPV6_ADDR_INT16_MLL		UINT_C(0xff02)

#elif _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN

#define IPV6_ADDR_INT32_ONE		ULONG_C(0x01000000)
#define IPV6_ADDR_INT32_TWO		ULONG_C(0x02000000)
#define IPV6_ADDR_INT32_0000FFFF	ULONG_C(0xffff0000)
#define IPV6_ADDR_INT32_MNL		ULONG_C(0x000001ff)
#define IPV6_ADDR_INT32_MLL		ULONG_C(0x000002ff)
#define IPV6_ADDR_INT32_ULL		ULONG_C(0x000080fe)
#define IPV6_ADDR_INT16_ULL		UINT_C(0x80fe)
#define IPV6_ADDR_INT16_USL		UINT_C(0xc0fe)
#define IPV6_ADDR_INT16_MLL		UINT_C(0x02ff)

#endif	/* #if _NET_CFG_BYTE_ORDER == _NET_CFG_BIG_ENDIAN */

#endif	/* of #ifdef _NET_CFG_BYTE_ORDER */

/*
 *  特別なアドレスのチェック
 */

/* IPv4 互換 */

#ifdef _NET_CFG_BYTE_ORDER
#define IN6_IS_ADDR_V4COMPAT(a)		\
	(memcmp((void *)(a), (void *)&in6_addr_unspecified, sizeof(T_IN6_ADDR) - 4) == 0 && \
	 (a)->s6_addr32[3] > IPV6_ADDR_INT32_ONE)
#endif

/* IPv4 射影 */

#ifdef _NET_CFG_BYTE_ORDER
#define IN6_IS_ADDR_V4MAPPED(a)		\
	(((a)->s6_addr32[0] == ULONG_C(0x00000000)) && \
	 ((a)->s6_addr32[1] == ULONG_C(0x00000000)) && \
	 ((a)->s6_addr32[2] == IPV6_ADDR_INT32_0000FFFF))
#endif

/* 要請マルチキャスト */

#ifdef _NET_CFG_BYTE_ORDER
#define IN6_IS_ADDR_NS_MULTICAST(a)	\
	(((a)->s6_addr32[0] == IPV6_ADDR_INT32_MLL) && \
	 ((a)->s6_addr32[1] == ULONG_C(0x00000000)) && \
	 ((a)->s6_addr32[2] == IPV6_ADDR_INT32_ONE) && \
	 ((a)->s6_addr8[12] == UINT_C(0xff)))
#endif

#ifndef _MACRO_ONLY

/*
 *  アドレス有効時間構造体
 */

typedef struct in6_addr_lifetime {

	/*
	 * expire と perferred は、システムが起動してからの経過時間
	 */
	uint32_t	expire;		/* 有効時間が切れる時刻 [s]	*/
	uint32_t	preferred;	/* 推奨有効時間が切れる時刻 [s]	*/

	/*
	 * vltime と pltime は、追加/変更してからの相対時間
	 */
	uint32_t	vltime;		/* 有効時間 [us]			*/
	uint32_t	pltime;		/* 推奨有効時間 [us]		*/
	} __attribute__((packed, aligned(2)))T_IN6_ADDR_LIFETIME;

/*
 *  インタフェースアドレス構造体
 */

typedef struct t_in6_ifaddr {
	T_IN6_ADDR		addr;		/* IPv6 アドレス		*/
	T_IN6_ADDR_LIFETIME	lifetime;	/* 有効時間		*/
	uint8_t			flags;		/* フラグ		*/
	uint8_t			prefix_len;	/* プレフィックス長	*/
	uint8_t			router_index;	/* ルータインデックス	*/
	uint8_t			prefix_index;	/* プレフィックスインデックス	*/
	uint8_t			na_icount;	/* 重複アドレス検出 (DAD) 通知の入力回数	*/
	uint8_t			ns_icount;	/* 重複アドレス検出 (DAD) 要請の入力回数	*/
	uint8_t			ns_ocount;	/* 重複アドレス検出 (DAD) 要請の出力回数	*/
	} __attribute__((packed, aligned(2)))T_IN6_IFADDR;

/*
 *  アドレス状態フラグ
 */

#define IN6_IFF_ANYCAST		UINT_C(0x01)	/* エニーキャスト	*/
#define IN6_IFF_DEFINED		UINT_C(0x02)	/* 定義済み		*/
#define IN6_IFF_TENTATIVE	UINT_C(0x04)	/* 重複検出中		*/
#define IN6_IFF_DUPLICATED	UINT_C(0x08)	/* 重複を検出した	*/
#define IN6_IFF_DEPRECATED	UINT_C(0x10)	/* 非推奨		*/
#define IN6_IFF_NODAD		UINT_C(0x20)	/* 重複検出不要		*/
#define IN6_IFF_AUTOCONF	UINT_C(0x40)	/* 自動設定された	*/
#define IN6_IFF_DETACHED	UINT_C(0x80)	/* 非使用		*/

/*
 *  アドレスの有効性チェック
 */

#define IFA6_IS_READY(a)	(((a)->flags & IN6_IFF_DEFINED) != 0 && \
				 ((a)->flags & (IN6_IFF_TENTATIVE|IN6_IFF_DUPLICATED|IN6_IFF_DETACHED)) == 0)
#define IFA6_IS_NOTREADY(a)	(((a)->flags & IN6_IFF_DEFINED) == 0 ||	\
				 ((a)->flags & (IN6_IFF_TENTATIVE|IN6_IFF_DUPLICATED|IN6_IFF_DETACHED)) != 0)

#define IFA6_IS_DEPRECATED(a,n)	\
	((a)->lifetime.preferred != 0 && (int32_t)((a)->lifetime.preferred - ((n / SYSTIM_HZ))) < 0)

#define IFA6_IS_INVALID(a,n)	\
	((a)->lifetime.expire != 0 && (int32_t)((a)->lifetime.expire - ((n / SYSTIM_HZ))) < 0)

/*
 *  ルータのインデックス
 */

#define IN6_RTR_IX_UNREACH	0xff

/*
 *  制限値
 */

#define IP6_HDR_NEST_LIMIT	50	/* IPv6 ヘッダのネスト制限値	*/

/*
 *  アドレス情報配列のインデックス
 */

#define IPV6_IFADDR_IX_LINKLOCAL	0
#define IPV6_IFADDR_IX_INVALID		0xff

/*
 *  IPv6 マルチキャストアドレス配列のインデックス
 */

#define IPV6_MADDR_IX_SOL_NODE		0	/* 要請マルチキャスト・アドレス			*/
#define IPV6_MADDR_IX_LL_ALL_NODE	1	/* 全ノード・リンクローカル・マルチキャストアドレス	*/
#define IPV6_MADDR_IX_EXTRA		2

#define MAX_IN6_MADDR_CNT		MAX_IF_MADDR_CNT
						/* IPv6 マルチキャストアドレス配列の最大サイズ	*/

/* ルーティング表エントリ構造体 */

typedef struct t_in6_rtentry {
	T_IN6_ADDR	target;		/* 目標ネットワークアドレス	*/
	T_IN6_ADDR	gateway;	/* ゲートウェイの IP アドレス	*/
	uint32_t	expire;		/* 有効時間が切れる時刻 [s]	*/
	uint8_t		flags;		/* フラグ			*/
	uint8_t		prefix_len;	/* プレフィックス長		*/
	} __attribute__((packed, aligned(2)))T_IN6_RTENTRY;

/*
 *  ホストキャッシュ
 */

/* ホストキャッシュ構造体 */

typedef struct t_in6_hostcache_entry {
	T_IN6_ADDR	dst;		/* 目標ネットワークアドレス	*/
	uint32_t	expire;		/* 有効時間が切れる時刻 [s]	*/
	uint32_t	mtu;		/* Path MTU			*/
	} __attribute__((packed, aligned(2)))T_IN6_HOSTCACHE_ENTRY;

#define IN6_HOSTCACHE_EXPIRE	(60*60*SYSTIM_HZ)	/* 有効時間	*/
#define IN6_HOSTCACHE_PRUNE	(5*60*SYSTIM_HZ)	/* 更新間隔	*/
#define IN6_HOSTCACHE_INCREASE	(10*60*SYSTIM_HZ)	/* 増加する場合の保護時間	*/

#define IN6_IS_HOSTCACHE_FREE(e)	IN6_IS_ADDR_UNSPECIFIED(&((e)->dst))
#define IN6_IS_HOSTCACHE_BUSY(e)	(!IN6_IS_HOSTCACHE_FREE(e))

/*
 *  ルーティング表
 */

extern T_IN6_RTENTRY routing6_tbl[];

/*
 *  前方参照
 */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

/*
 *  変数
 */

extern uint32_t linkmtu;

/*
 *  関数シミュレーションマクロ
 */

#define IN6_COPY_TO_HOST(dst,nbuf)	memcpy(dst,&GET_IP6_HDR(nbuf)->src,sizeof(T_IN6_ADDR))
#define INN6_IS_DSTADDR_ACCEPT(addr,nbuf)	\
					in6_is_dstaddr_accept(addr,&GET_IP6_HDR(nbuf)->dst)

/*
 *  関数
 */

extern void in6_ifainit (void);
extern int_t in6_addr2maix (const T_IN6_ADDR *addr);
extern ER in6_update_ifa (T_IFNET *ifp, T_IN6_IFADDR *ia, const T_IN6_ADDR *addr,
                          uint_t prefix_len, uint32_t vltime, uint32_t pltime,
                          int_t router_index, int_t prefix_index, uint_t flags);
extern ER in6_set_header (T_NET_BUF *nbuf, uint_t len,
                          const T_IN6_ADDR *dstaddr, const T_IN6_ADDR *srcaddr,
                          uint8_t next, uint8_t hlim);
extern ER in6_get_datagram (T_NET_BUF **nbuf, uint_t len, uint_t maxlen,
                            const T_IN6_ADDR *dstaddr, const T_IN6_ADDR *srcaddr,
                            uint8_t next, uint8_t hlim, ATR nbatr, TMO tmout);
extern uint16_t in6_cksum (T_NET_BUF *nbuf, uint8_t proto, uint_t off, uint_t len);
extern T_IN6_IFADDR *in6ifa_ifpwithix (T_IFNET *ifp, int_t ix);
extern T_IN6_IFADDR *in6ifa_ifpwithrtrix (T_IFNET *ifp, int_t rtr_ix);
extern T_IN6_IFADDR *in6_ifawithifp (T_IFNET *ifp, const T_IN6_ADDR *dst);
extern const T_IN6_ADDR *in6_addrwithifp (T_IFNET *ifp, T_IN6_ADDR *src, const T_IN6_ADDR *dst);
extern void ip6_input (T_NET_BUF *input);
extern T_IN6_IFADDR *in6_lookup_ifaddr (T_IFNET *ifp, const T_IN6_ADDR *addr);
extern bool_t in6_lookup_multi (T_IFNET *ifp, const T_IN6_ADDR *maddr);
extern bool_t in6_is_dstaddr_accept (const T_IN6_ADDR *myaddr, const T_IN6_ADDR *dstaddr);
extern bool_t inn6_is_dstaddr_accept (const T_IN6_ADDR *myaddr, T_NET_BUF *input);
extern const T_IN6_ADDR *in6_rtalloc (T_IFNET *ifp, const T_IN6_ADDR *dst);
extern int_t in6_if2idlen (T_IFNET *ifp);
extern int_t in6_addr2ifaix (const T_IN6_ADDR *addr);
extern void in6_plen2pmask (T_IN6_ADDR *mask, uint_t prefix_len);
extern bool_t in6_are_prefix_equal (const T_IN6_ADDR *addr, const T_IN6_ADDR *prefix,
                                  uint_t prefix_len);
extern void in6_ifaddr_timer (T_IFNET *ifp);
extern T_IN6_RTENTRY *in6_gateway_lookup (const T_IN6_ADDR *src);
extern void in6_rtredirect (const T_IN6_ADDR *gateway, const T_IN6_ADDR *target,
                            uint_t prefix_len, uint8_t flags, uint32_t tmo);
extern void in6_init (void);
extern void in6_if_up (T_IFNET *ifp);
extern void in6_hostcache_update (T_IN6_ADDR *dst, uint32_t mtu);
extern uint32_t in6_hostcache_getmtu (const T_IN6_ADDR *dst);
extern uint_t in6_addrscope (const T_IN6_ADDR *addr);
extern const T_NET_BUF**ip6_get_frag_queue (void);

#endif	/* of #ifndef _MACRO_ONLY */

#endif	/* of #ifndef _IN6_VAR_H_ */
