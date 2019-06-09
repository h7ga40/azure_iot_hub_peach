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
 */

#ifndef _IF_LOOP_H_
#define _IF_LOOP_H_

#ifdef SUPPORT_LOOP

/*
 *  ローカルループバックインタフェース・ヘッダとアドレス
 */

typedef struct t_loop_hdr  { } T_LOOP_HDR;
typedef struct t_loop_addr { } T_LOOP_ADDR;

/*
 *  インタフェースの選択マクロ
 */

#define T_IF_HDR		T_LOOP_HDR		/* インタフェースのヘッダ	*/
#define T_IF_ADDR		T_LOOP_ADDR		/* インタフェースのアドレス	*/
#define IF_HDR_ALIGN		4			/* ヘッダのアライン単位		*/
#define IF_MTU			LOMTU			/* インタフェースの MTU		*/

#define IF_OUTPUT(o,d,g,t)	looutput(o,t)		/* インタフェースの出力関数	*/
#define IF_RAW_OUTPUT(o,t)	looutput(o,t)		/* インタフェースの出力関数、アドレス解決無し*/
#define IF_SET_PROTO(b,p)				/* インタフェースのプロトコル設定関数	*/
#define IF_GET_IFNET()		loop_get_ifnet()	/* ネットワークインタフェース構造体を返す。		*/
#define IF_SRAND()		(0)			/* インターフェースの乱数初期値			*/

/* IPv4 関係 */

#define IF_PROTO_IP		0			/* インタフェースの IP プロトコル指定		*/

/* IPv6 関係 */

#define MAX_IF_MADDR_CNT	0			/* インタフェースのマルチキャストアドレス配列の最大サイズ	*/
#define IF_IN6_NEED_CACHE(i)	(false)			/* 近隣探索キャッシュを使用する。				*/

/*
 *  前方参照
 */

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

#ifndef T_IF_SOFTC_DEFINED

typedef struct t_if_softc T_IF_SOFTC;

#define T_IF_SOFTC_DEFINED

#endif	/* of #ifndef T_IF_SOFTC_DEFINED */

#ifndef T_IFNET_DEFINED

typedef struct t_ifnet T_IFNET;

#define T_IFNET_DEFINED

#endif	/* of #ifndef T_IFNET_DEFINED */

/*
 *  ネットワークインタフェースに依存しないソフトウェア情報
 */

struct t_if_softc { };

/*
 *  関数
 */

extern T_IFNET *loop_get_ifnet (void);
ER looutput (T_NET_BUF *output, TMO tmout);

/*
 *  ローカルループバックインタフェース・処理タスク
 */

extern void	if_loop_output_task(intptr_t exinf);
extern void	if_loop_input_task(intptr_t exinf);

#endif	/* of #ifdef SUPPORT_LOOP */

#endif	/* of #ifndef _IF_LOOP_H_ */
