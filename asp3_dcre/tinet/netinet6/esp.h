/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2017 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *  Copyright (C) 2008-2009 by Hokkaido Industrial Research Institute, JAPAN
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

/*	$FreeBSD: src/sys/netinet6/esp.h,v 1.6 2002/04/19 04:46:22 suz Exp $	*/
/*	$KAME: esp.h,v 1.19 2001/09/04 08:43:19 itojun Exp $	*/

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
 
#ifndef _ESP_H_
#define _ESP_H_

/*
 *  RFC2406 暗号化ヘッダ
 */

typedef struct t_esp_hdr {
	int32_t	spi;	/* Security Parameter Index (SPI)	*/
	int32_t	seq;	/* シーケンス番号			*/
	} __attribute__((packed, aligned(2)))T_ESP_HDR;

typedef struct t_esp_tail {
	int8_t	padlen;	/* パディング長				*/	
	int8_t	next;	/* Next Header				*/
	} __attribute__((packed, aligned(2)))T_ESP_TAIL;

struct t_secas_var;

struct t_esp_algorithm {
	size_t padbound;	/* pad boundary, in byte */
	int_t ivlenval;		/* iv length, in byte */
	int_t (*mature) (struct t_secas_var *);
	int_t keymin;	/* in bits */
	int_t keymax;	/* in bits */
	//int_t (*schedlen) (const struct t_esp_algorithm *);
	int32_t (*schedlen) (const struct t_esp_algorithm *);
	const char *name;
	int_t (*ivlen) (const struct t_esp_algorithm *, struct t_secas_var *);
	int_t (*decrypt) (T_NET_BUF *, size_t,
		struct t_secas_var *, const struct t_esp_algorithm *, int_t,
			T_NET_BUF *, size_t);
	int_t (*encrypt) (T_NET_BUF *, size_t, size_t, int8_t*,
		struct t_secas_var *, const struct t_esp_algorithm *, int_t,
			T_NET_BUF *, size_t);
	/* not supposed to be called directly */
	int_t (*schedule) (const struct t_esp_algorithm *, struct t_secas_var *);
	int_t (*blockdecrypt) (const struct t_esp_algorithm *,
		struct t_secas_var *, int8_t *, int8_t *);
	int_t (*blockencrypt) (const struct t_esp_algorithm *,
		struct t_secas_var *, int8_t *, int8_t *);
};

typedef struct t_esp_algorithm T_ESP_ALGORITHM;

#define MAXIVLEN	16

/*
 * 関数
 */

extern const struct t_esp_algorithm *esp_algorithm_lookup (int_t);
extern int_t esp_max_ivlen (void);
extern int_t esp_schedule (const struct t_esp_algorithm *, struct t_secas_var *);
extern int_t esp4_input(T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);

#ifdef T_IPSEC_REQUEST_DEFINED

extern int_t esp4_output(T_NET_BUF *m, T_IPSEC_REQUEST *isr, T_NET_BUF** result);

#endif	/* of #ifdef T_IPSEC_REQUEST_DEFINED */

#endif	/* of #ifndef _ESP_H_ */
