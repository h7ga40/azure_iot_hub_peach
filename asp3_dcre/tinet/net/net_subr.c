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
 * Copyright (c) 1982, 1986, 1988, 1993
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
 */

/* 
 *  ネットワーク汎用サポートルーチン
 */

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
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

/*
 *  変数
 */

static uint32_t next = ULONG_C(1);

/*
 *  net_rand -- 乱数を返す。
 */

uint32_t
net_rand (void)
{
	next = (next * 99991 + 12345) & ULONG_C(0x7fffffff);
	return next;
	}

/*
 *  srand -- 乱数を初期化する。
 */

void
net_srand (uint32_t seed)
{
	SYSTIM now;

	syscall(get_tim(&now));
	next += now + seed + IF_SRAND();
	}

/*
 *  convert_hexdigit -- 16進数→文字列変換
 */

int_t
convert_hexdigit (char *buf, uint_t val, int_t radix, int_t width, char padchar)
{
	static const char radhex[] = "0123456789abcdef";

	char	digits[11], *start;
	int_t	ix, pad;

	ix = 0;
	start = buf;
	do {
		digits[ix ++] = radhex[(val % radix) & 0x0f];
		val /= radix;
		} while (val != 0);
	for (pad = ix; pad < width; pad ++)
		*buf ++ = padchar;
	while (ix -- > 0)
		*buf ++ = digits[ix];
	*buf = '\0';
	return (int_t)(buf - start);
	}

#ifdef SUPPORT_ETHER

/*
 *  mac2str -- MAC アドレスを文字列に変換する。
 */

char *
mac2str (char *buf, uint8_t *macaddr)
{
	static char	addr_sbuf[NUM_MACADDR_STR_BUFF][sizeof("00:00:00:00:00:00")];
	static int_t	bix = NUM_MACADDR_STR_BUFF;

	char *start;

	if (buf == NULL) {
		syscall(wai_sem(SEM_MAC2STR_BUFF_LOCK));
		buf = addr_sbuf[-- bix];
		if (bix <= 0)
			bix = NUM_MACADDR_STR_BUFF;
		syscall(sig_sem(SEM_MAC2STR_BUFF_LOCK));
		}

	start = buf;
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*(buf ++) = ':';
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*(buf ++) = ':';
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*(buf ++) = ':';
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*(buf ++) = ':';
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*(buf ++) = ':';
	buf += convert_hexdigit(buf, (uint_t)(*macaddr ++), 16, 2, '0');
	*buf = '\0';
	return start;
	}

#endif	/* of #ifdef SUPPORT_ETHER */

#if _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN

/*
 *  rev_memcpy_hword -- 反転メモリコピー（2 バイト）
 *
 *    バイト単位にアクセスする事により、
 *    境界へのアラインの問題を解決する。
 */

void
rev_memcpy_hword (void *dst, void *src)
{
	*((uint8_t*)dst    ) = *((uint8_t*)src + 1);
	*((uint8_t*)dst + 1) = *((uint8_t*)src    );
	}

/*
 *  rev_memcpy_word -- 反転メモリコピー（4 バイト）
 *
 *    バイト単位にアクセスする事により、
 *    境界へのアラインの問題を解決する。
 */

void
rev_memcpy_word (void *dst, void *src)
{
	*((uint8_t*)dst    ) = *((uint8_t*)src + 3);
	*((uint8_t*)dst + 1) = *((uint8_t*)src + 2);
	*((uint8_t*)dst + 2) = *((uint8_t*)src + 1);
	*((uint8_t*)dst + 3) = *((uint8_t*)src    );
	}

/*
 *  rev_memcmp_word -- 反転メモリ比較（4 バイト）
 *
 *    バイト単位にアクセスする事により、
 *    境界へのアラインの問題を解決する。
 */

int_t
rev_memcmp_word (void *data1, void *data2)
{
	int_t	ix, diff;

	for (ix = sizeof(uint32_t); ix -- > 0; ) {
		diff = *((uint8_t*)data1 + ix) -  *((uint8_t*)data2 + (3 - ix));
		if (diff != 0)
			return diff;
		}
	return 0;
	}

#endif	/* of #if _NET_CFG_BYTE_ORDER == _NET_CFG_LITTLE_ENDIAN */
