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

#ifndef _TCP_ECHO_CLI_H_
#define _TCP_ECHO_CLI_H_

/* スタックサイズ */

#ifdef TOPPERS_S810_CLG3_85

#define TCP_ECHO_CLI_RCV_STACK_SIZE	512		/* TCP ECHO クライアント受信タスクのスタックサイズ	*/
#define TCP_ECHO_CLI_SND_STACK_SIZE	512		/* TCP ECHO クライアント送信タスクのスタックサイズ	*/

#else	/* of #ifdef TOPPERS_S810_CLG3_85 */

#define TCP_ECHO_CLI_RCV_STACK_SIZE	1024		/* TCP ECHO クライアント受信タスクのスタックサイズ	*/
#define TCP_ECHO_CLI_SND_STACK_SIZE	1024		/* TCP ECHO クライアント送信タスクのスタックサイズ	*/

#endif	/* of #ifdef TOPPERS_S810_CLG3_85 */

/* 優先度 */

#define TCP_ECHO_CLI_RCV_MAIN_PRIORITY	5		/* TCP ECHO クライアント受信タスクの優先度		*/
#define TCP_ECHO_CLI_SND_MAIN_PRIORITY	5		/* TCP ECHO クライアント送信タスクの優先度		*/

/*
 *  TCP 送受信ウインドバッファサイズ
 */

#if defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU)

#if defined(SUPPORT_INET6)

#define TCP_ECHO_CLI_SWBUF_SIZE		(1024)
#define TCP_ECHO_CLI_RWBUF_SIZE		(1024)

#elif defined(SUPPORT_INET4)

#define TCP_ECHO_CLI_SWBUF_SIZE		(512)
#define TCP_ECHO_CLI_RWBUF_SIZE		(512)

#endif	/* of #if defined(SUPPORT_INET6) */

#else	/* of #if defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU) */

#define TCP_ECHO_CLI_SWBUF_SIZE		(2048)
#define TCP_ECHO_CLI_RWBUF_SIZE		(2048)

#endif	/* of #if defined(USE_TCP_MSS_SEG) || defined(USE_IPV6_MMTU) */

/*
 *  バッファサイズの定義
 */

#define NUM_ECHO		20
#define NUM_REP_PAT		20
#define PAT_BEGIN		' '
#define PAT_END			'~'
#define SND_BUF_SIZE		((PAT_END - PAT_BEGIN + 1) * NUM_REP_PAT)
#define RCV_BUF_SIZE		(SND_BUF_SIZE*15/10)

/*
 *  変数
 */

extern bool_t	tcp_echo_cli_valid;
extern uint_t	tcp_echo_cli_scount;
extern ER_UINT	tcp_echo_cli_nblk_error;

/* TCP 送受信バッファ */

extern uint8_t tcp_echo_cli_swbuf[];
extern uint8_t tcp_echo_cli_rwbuf[];

/* タスク*/

extern void	tcp_echo_cli_rcv_task(intptr_t exinf);
extern void	tcp_echo_cli_snd_task(intptr_t exinf);

/* 関数 */

extern ER callback_nblk_tcp_echo_cli (ID cepid, FN fncd, void *p_parblk);

#ifdef T_IN_ADDR
extern ER tcp_echo_cli_snd (ID cepid, T_IN_ADDR *ipaddr, uint16_t portno, char apip);
#endif

#endif	/* of #ifndef _TCP_ECHO_CLI_H_ */
