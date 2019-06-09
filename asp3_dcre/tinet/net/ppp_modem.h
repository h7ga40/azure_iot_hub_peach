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

#ifndef _PPP_MODEM_H_
#define _PPP_MODEM_H_

#ifdef SUPPORT_PPP

#ifdef PPP_CFG_MODEM

/*
 *  モデムの状態
 */

#define PPP_MODEM_CONNECT	UINT_C(0)
#define PPP_MODEM_DISCONN	UINT_C(1)
#define PPP_MODEM_RING		UINT_C(2)
#define PPP_MODEM_DIAL		UINT_C(3)
#define PPP_MODEM_UP		UINT_C(4)
#define PPP_MODEM_DOWN		UINT_C(5)

/*
 *  全域変数
 */

extern uint8_t modem_state;	/* モデムの状態		*/

/*
 *  関数
 */

extern T_NET_BUF *modem_getline (T_NET_BUF *nbuf, uint16_t size);
extern int modem_cntl (T_NET_BUF *input);
extern void init_modem (void);
extern void dial (void);
extern ER wait_modem (void);

#else	/* of #ifdef PPP_CFG_MODEM */

/*
 *  関数
 */

#define init_modem

#endif	/* of #ifdef PPP_CFG_MODEM */

#endif	/* of #ifdef SUPPORT_PPP */

#endif	/* of #ifndef _PPP_MODEM_H_ */
