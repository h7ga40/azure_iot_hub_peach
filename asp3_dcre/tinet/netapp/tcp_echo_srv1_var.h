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

#ifdef USE_TCP_ECHO_SRV1

#ifndef _TCP_ECHO_SRV1_VAR_H_
#define _TCP_ECHO_SRV1_VAR_H_

/*
 *  注意:
 *
 *    BUF_SIZE は TCP の
 *    送信ウインドウバッファサイズ + 受信ウインドウバッファサイズの 
 *    3/2 倍以上の大きさがなければ、デッドロックする可能性がある。
 */

#define BUF_SIZE	((TCP_ECHO_SRV_SWBUF_SIZE + \
                          TCP_ECHO_SRV_RWBUF_SIZE) * 3 / 2)

/*
 *  サーバ情報構造体
 */

typedef struct t_tcp_echo_srv_info {
	ID	cepid;		/* 通信端点 ID		*/
	ID	repid;		/* 受付口   ID		*/

#ifdef USE_TCP_NON_BLOCKING
	ID	semid;		/* NBLK 用 セマフォID	*/
	ER	error;		/* NBLK 用 error	*/
	ER_UINT	slen;		/* NBLK 用 送信データ数	*/
	ER_UINT	rlen;		/* NBLK 用 受信データ数	*/
#endif

	T_IPEP	dst;		/* 接続相手アドレス情報	*/

#ifdef USE_TCP_NON_BLOCKING

	char	buffer[BUF_SIZE];

#else	/* of #ifdef USE_TCP_NON_BLOCKING */

#ifndef USE_COPYSAVE_API
	char	buffer[BUF_SIZE];
#endif

#endif	/* of #ifdef USE_TCP_NON_BLOCKING */

	} T_TCP_ECHO_SRV_INFO;

#define INDEX_SRV_INFO(id)	((id)-1)

/*
 *  変数
 */

extern T_TCP_ECHO_SRV_INFO tcp_echo_srv_info[];

/*
 *  関数
 */

extern ER tcp_echo_srv (uint_t six, char apip);

#endif	/* of #ifndef _TCP_ECHO_SRV1_VAR_H_ */

#endif	/* of #ifdef USE_TCP_ECHO_SRV1 */
