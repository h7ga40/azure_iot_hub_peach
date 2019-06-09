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
 *  ネットワーク経由コンソール入出力
 */

#include <stdarg.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <syssvc/serial.h>
#include <syssvc/logtask.h>
#include <t_syslog.h>
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/net_cons.h>

#ifdef USE_NET_CONS

/*
 *  マクロ定義
 */

/*  リネームの解除  */

#undef syslog
#undef serial_rea_dat
#undef serial_ctl_por

#define EOF		(-1)

/*  TELNET オプション */

#define TELNET_OPT_SE		(240)
#define TELNET_OPT_NOP		(241)
#define TELNET_OPT_DM		(242)
#define TELNET_OPT_BRK		(243)
#define TELNET_OPT_IP		(244)
#define TELNET_OPT_AO		(245)
#define TELNET_OPT_AYT		(246)
#define TELNET_OPT_EC		(247)
#define TELNET_OPT_EL		(248)
#define TELNET_OPT_GA		(249)
#define TELNET_OPT_SB		(250)
#define TELNET_OPT_WILL		(251)
#define TELNET_OPT_WONT		(252)
#define TELNET_OPT_DO		(253)
#define TELNET_OPT_DONT		(254)
#define TELNET_OPT_IAC		(255)

#if defined(SUPPORT_INET6)
#define API_PROTO		'6'
#else
#define API_PROTO		'4'
#endif

/*
 *  変数
 */

/*  TCP 送受信ウィンドバッファ  */

uint8_t net_cons_swbuf[NET_CONS_SWBUF_SIZE];
uint8_t net_cons_rwbuf[NET_CONS_RWBUF_SIZE];

bool_t	connected	= false;	/* コネクションの状態	*/
bool_t	wait_accept	= false;	/* 接続要求待ち中	*/

#if defined(SUPPORT_INET6)

T_IPV6EP	dst;

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

T_IPV4EP	dst;

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

uint8_t		*snd_buff	= NULL;
uint8_t		*rcv_buff	= NULL;
uint_t		snd_off		= 0;
uint_t		rcv_off		= 0;
uint_t		net_ioctl	= IOCTL_NULL;
ER_UINT		snd_len		= 0;
ER_UINT		rcv_len		= 0;

/*
 *  ノンブロッキングコールのコールバック関数
 */

ER
callback_nblk_net_cons (ID cepid, FN fncd, void *p_parblk)
{
	ER	error = E_OK;
	SYSTIM	now;

	switch (fncd) {

	case TFN_TCP_ACP_CEP:
		get_tim(&now);
		if (*(ER*)p_parblk == E_OK) {
			syslog(LOG_NOTICE, "[NCS%c:%02u CBN] conct: %7lu, from: %s.%u",
			                   API_PROTO, cepid, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
			connected = true;
			}
		else
			syslog(LOG_NOTICE, "[NCS%c:%02d CBN] error: %s",
			                   API_PROTO, itron_strerror(*(ER*)p_parblk));
		snd_len = snd_off = rcv_len = rcv_off = 0;
		wait_accept = false;
		break;

	case TFN_TCP_CLS_CEP:
	case TFN_TCP_RCV_BUF:
	case TFN_TCP_GET_BUF:
	case TFN_TCP_RCV_DAT:
	case TFN_TCP_SND_DAT:
	case TFN_TCP_CON_CEP:
	case TFN_TCP_SND_OOB:
	default:
		error = E_PAR;
		break;
		}
	return error;
	}

/*
 *  送信バッファのフラッシュ
 */

void
flush_snd_buff (void)
{
	ER_UINT	error;

	if (connected) {
		syscall(wai_sem(SEM_NET_CONS_SEND));
		if (snd_off > 0) {
			if ((error = tcp_snd_buf(NET_CONS_CEPID, snd_off)) != E_OK && error != E_CLS)
				syslog(LOG_NOTICE, "[NCS%c:%02d SND] flush send error: %s",
				                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
			snd_len = snd_off = 0;
			}
		syscall(sig_sem(SEM_NET_CONS_SEND));
		}
	return;
	}

/*
 *  文字送信
 */

void
cons_putchar (ID portid, char ch)
{
	ER_UINT	error;

	if (connected) {
		if (ch == '\n' && (net_ioctl & IOCTL_CRLF) != 0)
			cons_putchar(portid, '\r');
		syscall(wai_sem(SEM_NET_CONS_SEND));
		if (snd_off >= snd_len) {
			if ((error = tcp_snd_buf(NET_CONS_CEPID, snd_off)) != E_OK) {
				if (error != E_CLS)
					syslog(LOG_NOTICE, "[NCS%c:%02d SND] send buff error: %s",
					                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
				syscall(sig_sem(SEM_NET_CONS_SEND));
				return;
				}
			snd_off = 0;
			if ((snd_len = tcp_get_buf(NET_CONS_CEPID, (void*)&snd_buff, TMO_FEVR)) <= 0) {
				if (snd_len != E_CLS)
					syslog(LOG_NOTICE, "[NCS%c:%02d SND] get buff error: %s",
					                   API_PROTO, NET_CONS_CEPID, itron_strerror(snd_len));
				syscall(sig_sem(SEM_NET_CONS_SEND));
				return;
				}
			}
		snd_buff[snd_off ++] = ch;
		syscall(sig_sem(SEM_NET_CONS_SEND));
		}
	else
		serial_wri_dat(portid, &ch, sizeof(ch));
	}

/*
 *  文字受信（変換なし）
 */

static int_t
cons_getchar_raw (void)
{
	ER	error = E_OK;

	if (connected) {
		if (rcv_off >= rcv_len) {
			rcv_off = 0;
			if ((error = tcp_rel_buf(NET_CONS_CEPID, rcv_len)) != E_OK) {
				if (error != E_CLS)
					syslog(LOG_NOTICE, "[NCS%c:%02d RCV] release buff error: %s",
					                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
				discon_net_cons();
				return EOF;
				}
			if ((rcv_len = tcp_rcv_buf(NET_CONS_CEPID, (void*)&rcv_buff, TMO_FEVR)) == 0) {
				discon_net_cons();
				return EOF;
				}
			else if (rcv_len < 0) {
				if (rcv_len != E_CLS)
					syslog(LOG_NOTICE, "[NCS%c:%02d RCV] recieve buff error: %s",
					                   API_PROTO, NET_CONS_CEPID, itron_strerror(rcv_len));
				discon_net_cons();
				return EOF;
				}
			}
		return rcv_buff[rcv_off ++];
		}
	else
		return EOF;
	}

/*
 *  文字受信（TELNET オプションをスキップ）
 */

int_t
cons_getchar (ID portid)
{
	T_SERIAL_RPOR	rpor;
	int_t		ch;
	char		uch;
	ER		error;

	if (!wait_accept && !connected) {
		wait_accept = true;
		error = TCP_ACP_CEP(NET_CONS_CEPID, NET_CONS_REPID, &dst, TMO_NBLK);
#if 0
		syslog(LOG_NOTICE, "[NCS%c:%02d ACP] status: %s",
		                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
#endif
		if (error != E_WBLK)
			return EOF;
		}

	while (true) {
		if (connected) {
			while ((ch = cons_getchar_raw()) == TELNET_OPT_IAC || ch == '\0') {
				if (ch != '\0') {
					switch (ch = cons_getchar_raw()) {
					case TELNET_OPT_WILL:
					case TELNET_OPT_WONT:
					case TELNET_OPT_DO:
					case TELNET_OPT_DONT:
						cons_getchar_raw();
						break;
					case TELNET_OPT_SB:
						while ((ch = cons_getchar_raw()) != EOF && ch != TELNET_OPT_IAC) {
							if ((ch = cons_getchar_raw()) == EOF || ch == TELNET_OPT_SE)
								break;
							}
						break;
					default:
						break;
						}
					}
				}
			if (ch != '\n') {
				if ((net_ioctl & IOCTL_ECHO) != 0 && ch != EOF) {
					cons_putchar(portid, ch);
					flush_snd_buff();
					}
				return ch;
				}
			}

		else if (serial_ref_por(portid, &rpor) == E_OK && rpor.reacnt > 0) {
			if (serial_rea_dat(portid, &uch, sizeof(uch)) > 0)
				return uch;
			else
				return EOF;
			}
		dly_tsk(100);
		}
	return EOF;
	}

/*
 *  コネクションを切断
 */

ER
discon_net_cons (void)
{
	ER	error = E_OK;
	SYSTIM	now;

	if (connected) {
		syscall(wai_sem(SEM_NET_CONS_SEND));
		if (snd_off > 0) {
			if ((error = tcp_snd_buf(NET_CONS_CEPID, snd_off)) != E_OK && error != E_CLS)
				syslog(LOG_NOTICE, "[NCS%c:%02d SND] send buff error: %s",
				                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
			}
		if ((error = tcp_sht_cep(NET_CONS_CEPID)) != E_OK)
			syslog(LOG_NOTICE, "[NCS%c:%02d SHT] shutdown error: %s",
			                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
		if ((error = tcp_cls_cep(NET_CONS_CEPID, TMO_FEVR)) != E_OK)
			syslog(LOG_NOTICE, "[NCS%c:%02d CLS] close error: %s",
			                   API_PROTO, NET_CONS_CEPID, itron_strerror(error));
		connected = false;
		syscall(sig_sem(SEM_NET_CONS_SEND));
		get_tim(&now);
		syslog(LOG_NOTICE, "[NCS%c:%02u SND] discn: %7lu, from: %s.%u",
		                   API_PROTO, NET_CONS_CEPID, now / SYSTIM_HZ, IP2STR(NULL, &dst.ipaddr), dst.portno);
		}
	return error;
	}

/*
 *  シリアルポートの制御
 */

ER
net_serial_ctl_por (ID portid, uint_t ioctl)
{
	net_ioctl = ioctl;
	return serial_ctl_por(portid, ioctl);
	}

/*
 *  ログ出力
 */

ER
net_syslog (uint_t prio, const char *format, ...)
{
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	*addr;
#endif	/* of #if defined(SUPPORT_INET4) */

	ulong_t		val;
	SYSLOG		log;
	va_list		ap;
	char		padchar, *str;
	int_t		ch, width, left, i;
	bool_t		longflag;

	if (connected) {
		syscall(wai_sem(SEM_NET_CONS_PRINTF));
		va_start(ap, format);
		while ((ch = *format ++) != '\0') {
			if (ch != '%') {		/* 書式指定以外 */
				cons_putchar(CONSOLE_PORTID, (char)ch);
				continue;
				}

			width = 0;
			longflag = false;
			padchar = ' ';

			if (ch == '-') {		/* 左詰め */
				format ++;
				left = -1;
				}
			else
				left = 1;

			if ((ch = *format ++) == '0') {	/* 上位桁の 0 */
				padchar = '0';
				ch = *format ++;
				}

			while ('0' <= ch && ch <= '9') {	/* 出力幅 */
				width = width * 10 + ch - '0';
				ch = *format ++;
				}

			while (ch == 'l') {		/* long (long) の指定 */
				longflag = true;
				ch = *format ++;
				}

			switch (ch) {
			case 'd':
				val = longflag ? (ulong_t)va_arg(ap, long_t)
				               : (ulong_t)va_arg(ap, int_t);
				if ((long_t)val >= 0)
					cons_putnumber(CONSOLE_PORTID,  val, 10, radhex, width * left, false, padchar);
				else
					cons_putnumber(CONSOLE_PORTID, -val, 10, radhex, width * left, true, padchar);
				break;

			case 'u':
				val = longflag ? (ulong_t)va_arg(ap, ulong_t)
				               : (ulong_t)va_arg(ap, uint_t);
				cons_putnumber(CONSOLE_PORTID, val, 10, radhex, width * left, false, padchar);
				break;

			case 'x':
				val = longflag ? (ulong_t)va_arg(ap, ulong_t)
				               : (ulong_t)va_arg(ap, uint_t);
				cons_putnumber(CONSOLE_PORTID, val, 16, radhex, width * left, false, padchar);
				break;

			case 'X':
				val = longflag ? (ulong_t)va_arg(ap, ulong_t)
				               : (ulong_t)va_arg(ap, uint_t);
				cons_putnumber(CONSOLE_PORTID, val, 16, radHEX, width * left, false, padchar);
				break;

			case 'c':
				ch = va_arg(ap, int_t);
				cons_putchar(CONSOLE_PORTID, (char)ch);
				break;

			case 's':
				str = va_arg(ap, char*);
				while ((ch = *str ++) != '\0') {
					cons_putchar(CONSOLE_PORTID, (char)ch);
					width --;
					}
				while (width -- > 0)
					cons_putchar(CONSOLE_PORTID, ' ');
				break;

			case 'I':

#if defined(SUPPORT_INET4)

				addr = va_arg(ap, T_IN4_ADDR*);
				put_ipv4addr(CONSOLE_PORTID, addr, width);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

				str = va_arg(ap, char*);
				put_ipv6addr(CONSOLE_PORTID, (T_IN6_ADDR *)str, width);

#endif	/* of #if defined(SUPPORT_INET6) */

				break;

			case 'M':
				str = va_arg(ap, char*);
				put_macaddr(CONSOLE_PORTID, (uint8_t *)str, width);
				break;

			case '%':
				cons_putchar(CONSOLE_PORTID, '%');
				break;

			case '0':
				format --;
				break;

			default:
				break;
				}

			}
		va_end(ap);
		cons_putchar(CONSOLE_PORTID, '\n');
		flush_snd_buff();
		syscall(sig_sem(SEM_NET_CONS_PRINTF));
		return E_OK;
		}
	else {
		log.logtype = LOG_TYPE_COMMENT;
		log.loginfo[0] = (intptr_t)format;
		i = 1;
		va_start(ap, format);

		while ((ch = *format++) != '\0' && i < TMAX_LOGINFO) {
			if (ch != '%') {
				continue;
				}

			longflag = false;

			ch = *format++;
			while ('0' <= ch && ch <= '9') {
				ch = *format++;
				}

			while (ch == 'l') {		/* long (long) の指定 */
				longflag = true;
				ch = *format ++;
				}

			switch (ch) {
			case 'd':
				log.loginfo[i++] = longflag
					? (intptr_t)va_arg(ap, long_t) 
					: (intptr_t)va_arg(ap, int_t);
				break;
			case 'u':
			case 'x':
			case 'X':
				log.loginfo[i++] = longflag
					? (intptr_t)va_arg(ap, ulong_t)
					: (intptr_t)va_arg(ap, uint_t);
				break;
			case 'p':
				log.loginfo[i++] = (intptr_t)va_arg(ap, void *);
				break;
			case 'c':
				log.loginfo[i++] = (intptr_t)va_arg(ap, int_t);
				break;
			case 's':
				log.loginfo[i++] = (intptr_t)va_arg(ap, const char *);
				break;
			case '\0':
				format--;
				break;
			default:
				break;
				}
			}
		va_end(ap);

#ifdef TARGET_KERNEL_ASP

		return(syslog_wri_log(prio, &log));

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

		return(vwri_log(prio, &log));

#endif	/* of #ifdef TARGET_KERNEL_JSP */

		}
	}

#endif	/* of #ifdef USE_NET_CONS */
