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
 *  PPP モデム制御
 */

#include <string.h>

#include <t_services.h>
#include "kernel_id.h"

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>
#include <net/ppp.h>
#include <net/ppp_var.h>
#include <net/ppp_fsm.h>
#include <net/ppp_hdlc.h>
#include <net/ppp_modem.h>
#include <net/ppp_lcp.h>

#ifdef SUPPORT_PPP

/*
 *  全域変数
 */

uint8_t	modem_state = PPP_MODEM_DOWN;	/* モデムの状態		*/

/*
 *  モデム入力文字列
 */

#define MODEM_RCODE_OK		0
#define MODEM_RCODE_CONNECT	1
#define MODEM_RCODE_RING	2
#define MODEM_RCODE_NO_CARRIER	3
#define MODEM_RCODE_ERROR	4
#define MODEM_RCODE_NO_DIALTONE	5
#define MODEM_RCODE_BUSY	6
#define MODEM_RCODE_DELAYED	7

/*
 *  変数
 */

static uint8_t *modem_rcode[] = {
	"OK",
	"CONNECT",
	"RING",
	"NO CARRIER",
	"ERROR",
	"NO DIALTONE",
	"BUSY",
	"DELAYED"
	};

static int_t dial_retry;

/*
 *  関数
 */

static void do_dial (void);
static void modem_putstr (uint8_t *str);
static void modem_wait (uint8_t *line);
static void modem_putline (uint8_t *line);

/*
 *  modem_getline -- モデム行入力
 */

T_NET_BUF *
modem_getline (T_NET_BUF *nbuf, uint16_t size)
{
	uint8_t *buf;

	buf = nbuf->buf;
	size --;			/* '\0' の分 */
	while (size > 0 && (*buf = HDLC_raw_getoctet()) != '\r') {
		buf  ++;
		size --;
		}
	*buf = '\0';
	HDLC_raw_getoctet();		/* \n を読み飛ばす。*/
	nbuf->len = buf - nbuf->buf;
	return nbuf;
	}

/*
 *  モデム制御
 *
 *	戻り値: モデムからのリザルトコード
 */

int_t
modem_cntl (T_NET_BUF *input)
{
	int_t	code;

	code = sizeof(modem_rcode) / sizeof(uint8_t*) - 1;
	while (strcmp(modem_rcode[code], input->buf) != 0) {
		if (code == 0)
			return modem_state;
		code --;
		}

	switch (modem_state) {

	case PPP_MODEM_UP:
		if (code == MODEM_RCODE_RING)
			modem_state = PPP_MODEM_RING;
		break;

	case PPP_MODEM_DIAL:
		if (code == MODEM_RCODE_CONNECT) {
			syslog(LOG_NOTICE, "[MODEM] up.");
			modem_state = PPP_MODEM_CONNECT;
			lcp_open(PPP_OPEN_ACTIVE);
			sig_sem(SEM_MODEM_READY);
			}
		else {
			syslog(LOG_NOTICE, "[MODEM] dial failed '%s'.", modem_rcode[code]);
			if (-- dial_retry == 0) {
				syslog(LOG_WARNING, "[MODEM] dial failed %d times.", MODEM_CFG_RETRY_CNT);
				modem_state = PPP_MODEM_UP;
				}
			else {
				syscall(dly_tsk(MODEM_CFG_RETRY_WAIT));
				do_dial();
				}
			}
		break;

	case PPP_MODEM_RING:
		if (code == MODEM_RCODE_CONNECT) {
			syslog(LOG_NOTICE, "[MODEM] up.");
			modem_state = PPP_MODEM_CONNECT;
			lcp_open(PPP_OPEN_PASSIVE);
			}
		else
			modem_state = PPP_MODEM_UP;
		break;

	default:
		if (code == MODEM_RCODE_NO_CARRIER) {
			if (lcp_fsm.state == PPP_FSM_OPENED)
				lcp_close();
			syslog(LOG_NOTICE, "[MODEM] down.");
			modem_state = PPP_MODEM_UP;
			}
		break;

		}

	return code;
	}

/*
 *  init_modem -- モデムを初期化する。
 */

void
init_modem (void)
{
	modem_putline("AT");

#ifdef MODEM_CFG_INIT

	modem_putline(MODEM_CFG_INIT);

#endif	/* of #ifdef MODEM_CFG_INIT */

	modem_state = PPP_MODEM_UP;
	}

/*
 *  wait_modem -- 接続完了まで待つ。
 *
 *	戻り値: 接続に失敗したら E_OBJ を返す。
 */

ER
wait_modem (void)
{
	ER error = E_OK;

	if (modem_state == PPP_MODEM_UP) {
		dial();
		}

	if (modem_state != PPP_MODEM_CONNECT) {
		/* 接続完了まで待つ。*/
		wai_sem(SEM_MODEM_READY);
		if (modem_state != PPP_MODEM_CONNECT)
			error = E_OBJ;
		}

	return error;
	}

/*
 *  dial -- ダイヤルする。リトライ回数をセットする。
 */

void
dial (void)
{
	do_dial();
	dial_retry = MODEM_CFG_RETRY_CNT;
	modem_state = PPP_MODEM_DIAL;
	}

/*
 *  do_dial -- ダイヤルする。dial() の下請け関数
 */

static void
do_dial (void)
{
	syslog(LOG_NOTICE, "[MODEM] dial: %s.", MODEM_CFG_PHONE_NUMBER);
	modem_putstr(MODEM_CFG_DIAL);
	modem_putstr(MODEM_CFG_PHONE_NUMBER);
	modem_putstr("\r");
	}

/*
 *  modem_putstr -- モデム文字列出力
 */

static void
modem_putstr (uint8_t *str)
{
	while (*str)
		HDLC_raw_putoctet(*str ++);
	}

/*
 *  modem_wait -- モデムから文字列を待つ
 */

static void
modem_wait (uint8_t *line)
{
	int_t ch, state, len = strlen(line);

	state = len;
	while (state != 0) {
		ch = HDLC_raw_getoctet();
		if (ch != line[len - state])
			state = len;
		if (ch == line[len - state])
			state --;
		}
	}

/*
 *  modem_putline -- モデム行出力
 */

static void
modem_putline (uint8_t *line)
{
	modem_putstr(line);
	modem_putstr("\r");
	modem_wait("OK\r\n");
	}

#endif	/* fo #ifdef SUPPORT_PPP */
