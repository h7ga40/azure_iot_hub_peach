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
#include <net/if_loop.h>
#include <net/if_ppp.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_var.h>

#ifndef NOUSE_MPF_NET_BUF

/*
 *  関数
 */

extern const char *itron_strerror (ER ercd);

/*
 *  変数
 */

static T_NET_BUF_ENTRY net_buf_table[] = {

#if defined(NUM_MPF_NET_BUF6_65536) && NUM_MPF_NET_BUF6_65536 > 0
	{
		MPF_NET_BUF6_65536,
		IF_IP6_HDR_SIZE + 65536,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF6_65536,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF6_65536) && NUM_MPF_NET_BUF6_65536 > 0 */

#if defined(NUM_MPF_NET_BUF6_REASSM) && NUM_MPF_NET_BUF6_REASSM > 0

	{
		MPF_NET_BUF_REASSM,
		IP6_CFG_FRAG_REASSM_SIZE,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF6_REASSM,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},

#else	/* of #if defined(NUM_MPF_NET_BUF6_REASSM) && NUM_MPF_NET_BUF6_REASSM > 0 */

#if defined(NUM_MPF_NET_BUF4_REASSM) && NUM_MPF_NET_BUF4_REASSM > 0

	{
		MPF_NET_BUF_REASSM,
		IP4_CFG_FRAG_REASSM_SIZE,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF4_REASSM,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},

#endif	/* of #if defined(NUM_MPF_NET_BUF4_REASSM) && NUM_MPF_NET_BUF4_REASSM > 0 */

#endif	/* of #if defined(NUM_MPF_NET_BUF6_REASSM) && NUM_MPF_NET_BUF6_REASSM > 0 */

#if defined(NUM_MPF_NET_BUF_IF_PDU) && NUM_MPF_NET_BUF_IF_PDU > 0
	{
		MPF_NET_BUF_IF_PDU,
		IF_PDU_SIZE,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_IF_PDU,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_IF_PDU) && NUM_MPF_NET_BUF_IF_PDU > 0 */

#if defined(NUM_MPF_NET_BUF_IPV6_MMTU) && NUM_MPF_NET_BUF_IPV6_MMTU > 0
	{
		MPF_NET_BUF_IPV6_MMTU,
		IF_HDR_SIZE + IPV6_MMTU,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_IPV6_MMTU,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_IPV6_MMTU) && NUM_MPF_NET_BUF_IPV6_MMTU > 0 */

#if defined(NUM_MPF_NET_BUF_1024) && NUM_MPF_NET_BUF_1024 > 0
	{
		MPF_NET_BUF_1024,
		UINT_C(1024),

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_1024,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_1024) && NUM_MPF_NET_BUF_1024 > 0 */

#if defined(NUM_MPF_NET_BUF_IP_MSS) && NUM_MPF_NET_BUF_IP_MSS > 0
	{
		MPF_NET_BUF_IP_MSS,
		IF_HDR_SIZE + IP4_MSS,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_IP_MSS,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_IP_MSS) && NUM_MPF_NET_BUF_IP_MSS > 0 */

#if defined(NUM_MPF_NET_BUF_512) && NUM_MPF_NET_BUF_512 > 0
	{
		MPF_NET_BUF_512,
		UINT_C(512),

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_512,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_512) && NUM_MPF_NET_BUF_512 > 0 */

#if defined(NUM_MPF_NET_BUF_256) && NUM_MPF_NET_BUF_256 > 0
	{
		MPF_NET_BUF_256,
		UINT_C(256),

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_256,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_256) && NUM_MPF_NET_BUF_256 > 0 */

#if defined(NUM_MPF_NET_BUF_128) && NUM_MPF_NET_BUF_128 > 0
	{
		MPF_NET_BUF_128,
		UINT_C(128),

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_128,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_128) && NUM_MPF_NET_BUF_128 > 0 */

#if defined(_IP6_CFG)

#if defined(NUM_MPF_NET_BUF_CSEG) && NUM_MPF_NET_BUF_CSEG > 0
	{
		MPF_NET_BUF_CSEG,
		IF_HDR_SIZE + IP_HDR_SIZE + TCP_HDR_SIZE,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_CSEG,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_CSEG) && NUM_MPF_NET_BUF_CSEG > 0 */

#endif	/* of #if defined(_IP6_CFG) */

#if defined(NUM_MPF_NET_BUF_64) && NUM_MPF_NET_BUF_64 > 0
	{
		MPF_NET_BUF_64,
		UINT_C(64),

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_64,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_64) && NUM_MPF_NET_BUF_64 > 0 */

#if defined(_IP4_CFG) && !defined(_IP6_CFG)

#if defined(NUM_MPF_NET_BUF_CSEG) && NUM_MPF_NET_BUF_CSEG > 0
	{
		MPF_NET_BUF_CSEG,
		IF_HDR_SIZE + IP_HDR_SIZE + TCP_HDR_SIZE,

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

		NUM_MPF_NET_BUF_CSEG,

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

		},
#endif	/* of #if defined(NUM_MPF_NET_BUF_CSEG) && NUM_MPF_NET_BUF_CSEG > 0 */

#endif	/* of #if defined(_IP4_CFG) && !defined(_IP6_CFG) */

	};

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

/*
 *  nbuf_get_tbl -- ネットワークバッファ管理表を獲得する。
 *
 *    注意: ネットワーク統計情報の計測用
 */

const T_NET_BUF_ENTRY *
nbuf_get_tbl (void)
{
	return net_buf_table;
	}

/*
 *  nbuf_get_tbl_size -- ネットワークバッファ管理エントリ数を獲得する。
 *
 *    注意: ネットワーク統計情報の計測用
 */

uint_t
nbuf_get_tbl_size (void)
{
	return sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY);
	}

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

/*
 *  tget_net_buf_up -- 大きなサイズの方向に探索して、ネットワークバッファを獲得する。
 */

static ER
tget_net_buf_up (T_NET_BUF **buf, uint_t minlen, uint_t maxlen, TMO tmout)
{
	int_t	ix, req_ix;
	ER	error = E_OK;

	/* 最適なサイズの固定長メモリプールを探す。*/
	ix = sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY);
	while (ix -- > 0 && minlen > net_buf_table[ix].size)
		;
	req_ix = ix;

	NET_COUNT_NET_BUF(net_buf_table[req_ix].requests, 1);

	while (1) {
		if ((error = tget_mpf((ID)net_buf_table[ix].index, (void **)buf, ix == 0 ? tmout : TMO_POL)) == E_OK) {
			(*buf)->idix  = (uint8_t)ix;
			(*buf)->len   = (uint16_t)minlen;
			(*buf)->flags = 0;
#ifdef IF_ETHER_MULTI_NIC
			(*buf)->conn_pos = 0;
#endif
#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF
			NET_COUNT_NET_BUF(net_buf_table[ix].allocs, 1);
			net_buf_table[ix].busies ++;
#endif
			return error;
			}
		ix --;
		if (ix < 0 || net_buf_table[ix].size > maxlen)
			break;
		}

	syslog(LOG_WARNING, "[BUF] busy, up   index:%d,%d[%4d], len:%4d.",
	                    (uint16_t)req_ix, ix, net_buf_table[req_ix].size, minlen);
	*buf = NULL;
	NET_COUNT_NET_BUF(net_buf_table[req_ix].errors, 1);
	return error;
	}

/*
 *  tget_net_buf_down -- 小さなサイズの方向に探索して、ネットワークバッファを獲得する。
 */

static ER
tget_net_buf_down (T_NET_BUF **buf, uint_t minlen, uint_t maxlen, TMO tmout)
{
	int_t	ix, req_ix;
	ER	error = E_OK;

	/* 最適なサイズの固定長メモリプールを探す。*/
	ix = sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY);
	while (ix -- > 0 && maxlen > net_buf_table[ix].size)
		;
	req_ix = ix;

	NET_COUNT_NET_BUF(net_buf_table[req_ix].requests, 1);

	while (1) {
		if ((error = tget_mpf((ID)net_buf_table[ix].index, (void **)buf,
		                      ix == sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY) - 1 ? tmout : TMO_POL)) == E_OK) {
			(*buf)->idix  = (uint8_t)ix;
			(*buf)->len   = net_buf_table[ix].size;
			(*buf)->flags = 0;
#ifdef IF_ETHER_MULTI_NIC
			(*buf)->conn_pos = 0;
#endif

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF
			NET_COUNT_NET_BUF(net_buf_table[ix].allocs, 1);
			net_buf_table[ix].busies ++;
#endif
			return error;
			}
		ix ++;
		if (ix == sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY) || net_buf_table[ix].size < minlen)
			break;
		}

	syslog(LOG_WARNING, "[BUF] busy, down index:%d,%d[%4d], len:%4d.",
	                    (uint16_t)req_ix, ix, net_buf_table[req_ix].size, minlen);
	*buf = NULL;
	NET_COUNT_NET_BUF(net_buf_table[req_ix].errors, 1);
	return error;
	}

/*
 *  tget_net_buf_ex -- ネットワークバッファを獲得する（拡張機能）。
 */

ER
tget_net_buf_ex (T_NET_BUF **buf, uint_t minlen, uint_t maxlen, ATR nbatr, TMO tmout)
{
	/* 最小サイズが最大サイズを超えていたらエラー */
	if ((nbatr & NBA_SEARCH_DESCENT) != 0 && minlen > maxlen) {
		syslog(LOG_WARNING, "[NET BUF] E_PAR, minlen=%4d > maxlen=%4d.", minlen, maxlen);
		return E_PAR;
		}

	/* 割当て可能な最大のサイズを超えていたらエラー */
	if (minlen > net_buf_table[0].size || maxlen > net_buf_table[0].size) {
		syslog(LOG_WARNING, "[NET BUF] E_PAR, minlen=%4d or maxlen=%4d > %4d",
		                    minlen, maxlen, net_buf_table[0].size);
		return E_PAR;
		}

#if defined(SUPPORT_TCP) && defined(TCP_CFG_SWBUF_CSAVE)

	/* TCP で予約したネットワークバッファを取り出す。*/
	if ((nbatr & NBA_RESERVE_TCP) != 0) {
		if ((*buf = TCP_PULL_RES_NBUF(nbatr)) != NULL) {
			return E_OK;
			}
		}

#endif	/* of #if defined(SUPPORT_TCP) && defined(TCP_CFG_SWBUF_CSAVE) */

	if ((nbatr & NBA_SEARCH_DESCENT) != 0)
		return tget_net_buf_down(buf, minlen, maxlen, tmout);
	else if ((nbatr & NBA_SEARCH_ASCENT) != 0)
		return tget_net_buf_up(buf, minlen, (uint_t)net_buf_table[0].size, tmout);
	else {
		syslog(LOG_WARNING, "[NET BUF] E_PAR, nbatr=%08x.", nbatr);
		return E_PAR;
		}
	}

/*
 *  tget_net_buf -- ネットワークバッファを獲得する（互換）。
 */

ER
tget_net_buf (T_NET_BUF **buf, uint_t len, TMO tmout)
{
	return tget_net_buf_ex(buf, len, (uint_t)net_buf_table[0].size, NBA_SEARCH_ASCENT, tmout);
	}

/*
 * rel_net_buf -- ネットワークバッファを返却する。
 */

ER
rel_net_buf (T_NET_BUF *buf)
{
	ER	error = E_OK;

	/* ネットワークバッファの ID の正当性を検証する。*/
	if ((int_t)buf->idix >= (int_t)(sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY))) {
		syslog(LOG_WARNING, "[NET BUF] E_ID, ID=%d.", buf->idix);
		error = E_ID;
		}
	else {

#if defined(SUPPORT_TCP) && defined(TCP_CFG_SWBUF_CSAVE)

		/* TCP で、ネットワークバッファを予約する。*/
		if (TCP_PUSH_RES_NBUF(buf) == NULL)
			return E_OK;

#endif	/* of #if defined(SUPPORT_TCP) && defined(TCP_CFG_SWBUF_CSAVE) */

		/* 固定メモリプールに返す。*/

		int idix = buf->idix;
#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF
		net_buf_table[idix].busies --;
#endif
		if ((error = rel_mpf((ID)net_buf_table[idix].index, buf)) != E_OK) {
			syslog(LOG_WARNING, "[NET BUF] %s, ID=%d.", itron_strerror(error), idix);
			}
		}
	return error;
	}

/*
 * rus_net_buf -- ネットワークバッファを再利用する。
 */

ER
rus_net_buf (T_NET_BUF *buf)
{
	ER	error = E_OK;

	if (buf->idix >= sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY)) {
		syslog(LOG_WARNING, "[NET BUF] E_ID, ID=%d.", buf->idix);
		error = E_ID;
		}
	else {
		buf->len   = net_buf_table[buf->idix].size;
		buf->flags = 0;
		error = E_OK;
		}
	return error;
	}

/*
 * net_buf_siz -- ネットワークバッファのサイズを返す。
 */

ER_UINT
net_buf_siz (T_NET_BUF *buf)
{
	if (buf->idix >= sizeof(net_buf_table) / sizeof(T_NET_BUF_ENTRY)) {
		syslog(LOG_WARNING, "[BUF SIZ] E_ID, ID=%d.", buf->idix);
		return E_ID;
		}
	else
		return (ER_UINT)net_buf_table[buf->idix].size;
	}

/*
 * net_buf_max_siz -- ネットワークバッファの最大サイズを返す。
 */

uint_t
net_buf_max_siz (void)
{
	return (uint_t)net_buf_table[0].size;
	}

#else
#include <stdlib.h>

/*
 *  tget_net_buf_ex -- ネットワークバッファを獲得する（拡張機能）。
 */

ER
tget_net_buf_ex (T_NET_BUF **buf, uint_t minlen, uint_t maxlen, ATR nbatr, TMO tmout)
{
	uint_t len = (minlen > maxlen) ? minlen : maxlen;
	*buf = (T_NET_BUF *)malloc(sizeof(T_NET_BUF) - sizeof(((T_NET_BUF *)0)->buf) + len);
	if (*buf == NULL)
		return E_NOMEM;

	(*buf)->idix = 0;
	(*buf)->len = (uint16_t)len;
	(*buf)->flags = 0;
#ifdef IF_ETHER_MULTI_NIC
	(*buf)->conn_pos = 0;
#endif

	return E_OK;
	}

/*
 *  tget_net_buf -- ネットワークバッファを獲得する（互換）。
 */

ER
tget_net_buf (T_NET_BUF **buf, uint_t len, TMO tmout)
{
	return tget_net_buf_ex(buf, len, len, NBA_SEARCH_ASCENT, tmout);
	}


/*
 * rel_net_buf -- ネットワークバッファを返却する。
 */

ER
rel_net_buf (T_NET_BUF *buf)
{
	free(buf);

	return E_OK;
	}


/*
 * net_buf_max_siz -- ネットワークバッファの最大サイズを返す。
 */

uint_t
net_buf_max_siz (void)
{
	return (uint_t)IF_PDU_SIZE;
	}

#endif
