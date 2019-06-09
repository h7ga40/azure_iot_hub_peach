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
 *  WWW サーバ
 *
 *    ・送受信タスク同一型
 *    ・ノンブロッキングコール
 *    ・省コピー API
 *    ・IPv4/IPv6
 */

#include <stdlib.h>
#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <sil.h>
#include <t_syslog.h>
#include "kernel/kernel_impl.h"
#include "target_config.h"
#include "target_syssvc.h"
#include "target_stddef.h"
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_defs.h>
#include <tinet_config.h>

#include <net/if.h>
#include <net/if_ppp.h>
#include <net/if_loop.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/net_endian.h>
#include <net/net_buf.h>
#include <net/net_count.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/wwws.h>

#ifdef USE_WWW_SRV

/*
 *  送受信 API の選択
 */

#define EOF		(-1)

typedef struct file {
	ER		(*func)(ID cepid, T_WWW_RWBUF *srbuf);
	char		*uri;
	} T_FILE;

typedef struct status {
	uint16_t	code;
	uint8_t		*reason;
	} T_STATUS;

/*
 *  関数
 */

static ER index_html(ID cepid, T_WWW_RWBUF *srbuf);

#if NET_COUNT_ENABLE

static ER stat_html(ID cepid, T_WWW_RWBUF *srbuf);

#endif	/* of #if NET_COUNT_ENABLE */

#ifdef USE_TCP_EXTENTIONS

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV)

#if NUM_WWW_SRV_TASKS >= 2		/* 2 を超える個数は想定していない。	*/

#define WWW4_SRV_TASK	WWW_SRV_TASK1
#define WWW6_SRV_TASK	WWW_SRV_TASK2

#else	/* of #if NUM_WWW_SRV_TASKS >= 2 */

#define WWW4_SRV_TASK	WWW_SRV_TASK1
#define WWW6_SRV_TASK	WWW_SRV_TASK1

#endif	/* of #if NUM_WWW_SRV_TASKS >= 2 */

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

#define WWW4_SRV_TASK	WWW_SRV_TASK1
#define WWW6_SRV_TASK	WWW_SRV_TASK1

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

#endif	/* of #ifdef USE_TCP_EXTENTIONS */


/*
 *  全域変数
 */

/* TCP 送受信ウィンドバッファ */

#ifndef TCP_CFG_SWBUF_CSAVE
uint8_t www_srv_swbuf[NUM_WWW_SRV_TASKS][WWW_SRV_SWBUF_SIZE];
#endif

#ifndef TCP_CFG_RWBUF_CSAVE
uint8_t www_srv_rwbuf[NUM_WWW_SRV_TASKS][WWW_SRV_RWBUF_SIZE];
#endif

/*
 *  変数
 */

SYSTIM	srv_start;

static const T_FILE file[] = {
	{ index_html,	"/"		},
	{ index_html,	"/index.html"	},

#if NET_COUNT_ENABLE

	{ stat_html,	"/stat.html"	},

#endif	/* of #if NET_COUNT_ENABLE */
	};

#define NUM_FILES	(sizeof(file) / sizeof(T_FILE))

static const char *status[] = {
	"200 OK",
	"404 Not Found",
	};

#define NUM_STATUS	(sizeof(status) / sizeof(T_STATUS))

#define ST_OK		0
#define ST_NOT_FOUND	1

/*
 *  接続相手の情報構造体
 */

#ifdef  NUM_WWW4_SRV_TASKS
#undef  NUM_WWW4_SRV_TASKS
#endif
#define NUM_WWW4_SRV_TASKS		1

typedef struct t_www_srv_info {
	ID	cepid;		/* 通信端点 ID		*/
	ID	repid;		/* 受付口   ID		*/
	T_IPEP	dst;		/* 接続相手アドレス情報	*/
	} T_WWW_SRV_INFO;

#define INDEX_SRV_INFO(id)	((id)-1)

#ifdef USE_TCP_EXTENTIONS

static T_WWW_SRV_INFO www_srv_info[NUM_WWW_SRV_TASKS];
					/* 2 を超える個数は想定していない。	*/

#else	/* of #ifdef USE_TCP_EXTENTIONS */

static T_WWW_SRV_INFO www_srv_info[] = {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV)

	{ WWW4_SRV_CEPID1, WWW4_SRV_REPID },

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	{ WWW_SRV_CEPID1, WWW_SRV_REPID },

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

#if NUM_WWW_SRV_TASKS >= 2		/* 2 を超える個数は想定していない。	*/
	{ WWW_SRV_CEPID2, WWW_SRV_REPID },
#endif

	};

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_COPYSAVE_API

/*
 *  get_char -- 一文字入力する。
 */

static int_t
get_char (ID cepid, T_WWW_RWBUF *srbuf)
{
	int_t	len, ch;
	ER	error;

	if (srbuf->unget) {
		ch = srbuf->unget;
		srbuf->unget = 0;
		}
	else  {
		if (srbuf->rbuf.index >= srbuf->rbuf.len) {
			if ((error = tcp_rel_buf(cepid, srbuf->rbuf.len)) != E_OK)
				syslog(LOG_WARNING, "[WWWn:%02u REL] tcp_rel_buf error: %s",
				       cepid, itron_strerror(error));
			srbuf->rbuf.index = 0;
			if ((len = tcp_rcv_buf(cepid, (void**)&srbuf->rbuf.buf, TMO_FEVR)) <= 0) {
				if (len < 0)
					syslog(LOG_WARNING, "[WWWn:%02u RCV] tcp_rcv_buf error: %s",
					       cepid, itron_strerror(len));
				srbuf->rbuf.len = 0;
				return EOF;
				}
			else
				srbuf->rbuf.len = len;
			}
		ch = srbuf->rbuf.buf[srbuf->rbuf.index ++];
		}
	return ch;
	}

/*
 *  flush_sbuf -- 送信バッファをフラッシュする。
 */

static ER
flush_sbuf (ID cepid, T_WWW_RWBUF *srbuf)
{
	ER_UINT	error;

	error = tcp_snd_buf(cepid, srbuf->sbuf.index);
	srbuf->sbuf.index = srbuf->sbuf.len = 0;

	return error;
	}

/*
 *  put_str -- 文字列を出力する。
 */

static uint_t
put_str (ID cepid, T_WWW_RWBUF *srbuf, const char *str)
{
	ER		error;
	ER_UINT		blen;
	uint16_t	len, off = 0;

	len = strlen(str);
	while (len > 0) {
		if (srbuf->sbuf.index >= srbuf->sbuf.len)
			if ((error = flush_sbuf(cepid, srbuf)) != E_OK) {
				syslog(LOG_WARNING, "[WWWn:%02u SND] error: %s", cepid, itron_strerror(error));
			return 0;
			}

		if (srbuf->sbuf.len == 0) {
			if ((blen = tcp_get_buf(cepid, (void**)&srbuf->sbuf.buf, TMO_FEVR)) < 0) {
				syslog(LOG_WARNING, "[WWWn:%02u GET] error: %s", cepid, itron_strerror(srbuf->sbuf.len));
				return 0;
				}
			srbuf->sbuf.len = (uint16_t)blen;
			}

		if (len > srbuf->sbuf.len - srbuf->sbuf.index)
			blen = srbuf->sbuf.len - srbuf->sbuf.index;
		else
			blen = len;
		memcpy(&srbuf->sbuf.buf[srbuf->sbuf.index], str + off, blen);
		srbuf->sbuf.index += (uint16_t)blen;
		off               += (uint16_t)blen;
		len               -= (uint16_t)blen;
		}

	return off;
	}

#else	/* of #ifdef USE_COPYSAVE_API */

/*
 *  get_char -- 一文字入力する。
 */

static int_t
get_char (ID cepid, T_WWW_RWBUF *srbuf)
{
	int_t	len, ch;

	if (srbuf->unget) {
		ch = srbuf->unget;
		srbuf->unget = 0;
		}
	else  {
		if (srbuf->rbuf.index >= srbuf->rbuf.len) {
			if ((len = tcp_rcv_dat(cepid, srbuf->rbuf.buf, sizeof(srbuf->rbuf.buf), TMO_FEVR)) <= 0) {
				if (len < 0) {
					syslog(LOG_WARNING, "[WWWn:%02u RCV] error: %s",
					       cepid, itron_strerror(len));
					}
				return EOF;
				}
			else {
				srbuf->rbuf.len   = len;
				srbuf->rbuf.index = 0;
				}
			}
		ch = srbuf->rbuf.buf[srbuf->rbuf.index ++];
		}
	return ch;
	}

/*
 *  flush_sbuf -- 送信バッファをフラッシュする。
 */

static ER
flush_sbuf (ID cepid, T_WWW_RWBUF *srbuf)
{
	ER_UINT		len = E_OK;
	uint16_t	off = 0;

	while (off < srbuf->sbuf.index) {
		if ((len = tcp_snd_dat(cepid, srbuf->sbuf.buf + off, srbuf->sbuf.index - off, TMO_FEVR)) < 0)
			break;
		off += len;
		}
	srbuf->sbuf.index = 0;

	return len < 0 ? len : E_OK;
	}

/*
 *  put_str -- 文字列を出力する。
 */

static uint16_t
put_str (ID cepid, T_WWW_RWBUF *srbuf, const char *str)
{
	ER		error;
	uint16_t	blen, len, off = 0;

	len = strlen(str);
	while (len > 0) {
		if (srbuf->sbuf.index >= srbuf->sbuf.len)
			if ((error = flush_sbuf(cepid, srbuf)) != E_OK) {
				syslog(LOG_WARNING, "[WWWn:%02u SND] error: %s", cepid, itron_strerror(error));
			return 0;
			}
		if (len > srbuf->sbuf.len - srbuf->sbuf.index)
			blen = srbuf->sbuf.len - srbuf->sbuf.index;
		else
			blen = len;
		memcpy(&srbuf->sbuf.buf[srbuf->sbuf.index], str + off, blen);
		srbuf->sbuf.index += blen;
		off               += blen;
		len               -= blen;
		}

	return off;
	}

#endif	/* of #ifdef USE_COPYSAVE_API */

/*
 *  get_line -- 一行入力する。
 */

static uint16_t
get_line (ID cepid, T_WWW_LINE *line, T_WWW_RWBUF *srbuf)
{
	int_t		ch = 0;
	uint16_t	len;
	char		*p, *t;

	p = line->buf;
	t = line->buf + WWW_LINE_SIZE;
	while (p < t && (ch = get_char(cepid, srbuf)) != EOF && ch != '\n' && ch != '\r')
		*p ++ = ch;
	*p = '\0';
	len = p - line->buf;
	if (p >= t) {
		while ((ch = get_char(cepid, srbuf)) != EOF && ch != '\n' && ch != '\r')
			len ++;
		}
	if (ch == '\r') {
		len ++;
		if ((ch = get_char(cepid, srbuf)) != EOF && ch != '\n')
			srbuf->unget = ch;
		}
	if (ch == '\n')
		len ++;
	line->len = len;
	return p - line->buf;
	}

#if NET_COUNT_ENABLE

#ifdef _int64_
#define VAL_TYPE	uint64_t
#else
#define VAL_TYPE	uint32_t
#endif

/*
 *  convert -- 数値変換
 */

static int_t
convert (char *buf, VAL_TYPE val, int_t radix, int_t width, bool_t minus, char padchar)
{
	static const char radhex[] = "0123456789abcdef";

	char	digits[23], *start;
	int_t	ix, pad;

	ix = 0;
	start = buf;
	do {
		digits[ix ++] = radhex[val % radix];
		val /= radix;
		} while (val != 0);
	if (minus)
		digits[ix ++] = '-';
	for (pad = ix; pad < width; pad ++)
		*buf ++ = padchar;
	while (ix -- > 0)
		*buf ++ = digits[ix];
	*buf = '\0';
	return buf - start;
	}

#endif	/* of #if NET_COUNT_ENABLE */

/*
 * split_fields -- フィールドに分割する。
 */

static void
split_fields (T_WWW_LINE *line, char *delim)
{
	char	*p, quote;
	int_t	ix = 0;

	line->off[ix ++] = 0;
	for (p = line->buf; ix < WWW_NUM_FIELDS && *p; p ++) {
		if (strchr("\"'`", *p) != NULL) {
			for (quote = *p ++; *p && *p != quote; p ++)
				;
			}
		else if (strchr(delim, *p) != NULL) {
			*p = '\0';
			line->off[ix ++] = (uint8_t)(p - line->buf + 1);
			}
		}
	line->num = ix;
	}

/*
 *  put_status -- status line を出力する。
 */

static uint16_t
put_status (ID cepid, T_WWW_RWBUF *srbuf, int_t index)
{
	uint16_t	len = 0;

	len += put_str(cepid, srbuf, "HTTP/1.0 ");
	len += put_str(cepid, srbuf, status[index]);
	len += put_str(cepid, srbuf, "\r\n");
	return len;
	}

/*
 *  put_content_length -- Content-length を出力する。
 */

static uint16_t
put_content_length (ID cepid, T_WWW_RWBUF *srbuf, int_t content_len)
{
	uint16_t	len = 0;
#if 0
	char		length[TD_DIGITS + 1];

	convert(length, content_len, 10, 0, false, ' ');
	len += put_str(cepid, srbuf, "Content-length: ");
	len += put_str(cepid, srbuf, length);
	len += put_str(cepid, srbuf, "\r\n");
#endif	/* of #if 0 */
	return len;
	}

/*
 *  get_method -- GET メソッドの処理
 */

static ER
get_method (ID cepid, T_WWW_RWBUF *srbuf, T_WWW_LINE *line)
{
	int_t	ix;

	for (ix = NUM_FILES; ix -- > 0; ) {
		if (!strcmp(file[ix].uri, &line->buf[line->off[1]])) {
			//syslog(LOG_NOTICE, "[WWWn:%02u PTH] '%s'", cepid, &line->buf[line->off[1]]);
			return (*file[ix].func)(cepid, srbuf);
			}
		}
	put_status(cepid, srbuf, ST_NOT_FOUND);
	return E_NOEXS;
	}

/*
 *  parse_request -- リクエストを解析する。
 */

static ER
parse_request (ID cepid, T_WWW_RWBUF *srbuf)
{
	T_WWW_LINE	*method, *line;
	ER		error = E_OK;
	int_t		blen = 0;

	if ((error = tget_mpf(MPF_WWW_LINE, (void*)&method, TMO_FEVR)) != E_OK) {
		syslog(LOG_CRIT, "[WWWn:%02u MPF] get line error: %s.",
		       cepid, itron_strerror(error));
		return error;
		}

	if (get_line(cepid, method, srbuf) == 0) {
		if ((error = rel_mpf(MPF_WWW_LINE, method)) != E_OK)
			syslog(LOG_WARNING, "[WWWn:%02u MPF] release line buffer error: %s.",
			       cepid, itron_strerror(error));
		return error;
		}

	if ((error = tget_mpf(MPF_WWW_LINE, (void*)&line, TMO_FEVR)) != E_OK) {
		syslog(LOG_CRIT, "[WWWn:%02u MPF] get line buffer error: %s.",
		       cepid, itron_strerror(error));
		if ((error = rel_mpf(MPF_WWW_LINE, method)) != E_OK)
			syslog(LOG_WARNING, "[WWWn:%02u MPF] release line buffer error: %s.",
			       cepid, itron_strerror(error));
		return error;
		}

	while (get_line(cepid, line, srbuf) > 0) {	/* ヘッダをスキップする。*/
		split_fields(line, ": ");
		if (strcmp("Content-Length", &line->buf[line->off[0]]) == 0)
			blen = atoi(&line->buf[line->off[1]]);
		}
	while (blen > 0 && get_line(cepid, line, srbuf) > 0) {
		/* エンティティ・ボディをスキップする。*/
		blen -= line->len;
		}

	if ((error = rel_mpf(MPF_WWW_LINE, line)) != E_OK)
		syslog(LOG_WARNING, "[WWWn:%02u MPF] release line buffer error: %s.",
		       cepid, itron_strerror(error));
	
	split_fields(method, ": ");
	if (!strcmp(&method->buf[method->off[0]], "GET"))
		error = get_method(cepid, srbuf, method);

	flush_sbuf(cepid, srbuf);

	if ((error = rel_mpf(MPF_WWW_LINE, method)) != E_OK)
		syslog(LOG_WARNING, "[WWWn:%02u MPF] release line buffer error: %s.",
		       cepid, itron_strerror(error));

	return error;
	}

/*
 *  index_html -- /index.html ファイル
 */

static ER
index_html (ID cepid, T_WWW_RWBUF *srbuf)
{
	static const char response[] =
		"\r\n<!DOCTYPE html PUBLIC \"-//W3C//DTD html 3.2//EN\">\r\n"
		"<html><head>\r\n"

#ifdef TOPPERS_S810_CLG3_85
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">\r\n"
#else
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\r\n"
#endif

		"<title>TINET TCP/IP Protocol Stack</title>\r\n"
		"</head><body bgcolor=\"#ffffbb\">\r\n"
		"<hr>この WWW サーバは<br>"

#ifdef TARGET_KERNEL_ASP
		"ASP Kernel for " TARGET_NAME " (" __DATE__ "," __TIME__ ") と<br>\r\n"
#endif

#ifdef TARGET_KERNEL_JSP
		"JSP Kernel for " TARGET_NAME " (" __DATE__ "," __TIME__ ") と<br>\r\n"
#endif

		"TINET TCP/IP プロトコルスタックによりサービスしています。<br><hr>\r\n"

#if NET_COUNT_ENABLE

		"<ul><li type=\"square\"><a href=\"stat.html\">ネットワーク統計情報</a></ul><hr>\r\n"

#else	/* of #if NET_COUNT_ENABLE */

		"<ul><li type=\"square\">ネットワーク統計情報</ul><hr>\r\n"

#endif	/* of #if NET_COUNT_ENABLE */

		"FreeBSD: Copyright (c) 1982, 1986, 1988, 1990, 1993, 1994, 1995<br>\r\n"
		"The Regents of the University of California.  All rights reserved.<br><br>\r\n"

#ifdef SUPPORT_PPP

		"pppd: Copyright (c) 1989 Carnegie Mellon University.<br>\r\n"
		"All rights reserved.<br><br>\r\n"
		"ppp: Written by Toshiharu OHNO (tony-o@iij.ad.jp)<br>\r\n"
		"Copyright (C) 1993, Internet Initiative Japan, Inc. All rights reserverd.<br><br>\r\n"

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

 		"if_ed: Copyright (c) 1995, David Greenman<br>\r\n"
		"All rights reserved.<br><br>\r\n"

#endif	/* of #ifdef SUPPORT_ETHER */

#ifdef TARGET_KERNEL_ASP
		"TOPPERS/ASP Kernel<br>\r\n"
		"Toyohashi Open Platform for Embedded Real-Time Systems/<br>\r\n"
		"Advanced Standard Profile Kernel<br>\r\n"
		"Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory<br>\r\n"
		"Toyohashi Univ. of Technology, JAPAN<br>\r\n"
		"Copyright (C) 2004-2008 by Embedded and Real-Time Systems Laboratory<br>\r\n"
		"Graduate School of Information Science, Nagoya Univ., JAPAN<br><br>\r\n"
#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP
		"TOPPERS/JSP Kernel<br>\r\n"
		"Toyohashi Open Platform for Embedded Real-Time Systems/<br>\r\n"
		"Just Standard Profile Kernel<br>\r\n"
		"Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory<br>\r\n"
		"Toyohashi Univ. of Technology, JAPAN<br>\r\n"
		"Copyright (C) 2005 by Embedded and Real-Time Systems Laboratory<br>\r\n"
		"Graduate School of Information Science, Nagoya Univ., JAPAN<br><br>\r\n"
#endif	/* of #ifdef TARGET_KERNEL_JSP */

		"TINET (TCP/IP Protocol Stack)<br>\r\n"
		"Copyright (C) 2001-2008 by Dep. of Computer Science and Engineering<br>\r\n"
		"Tomakomai National College of Technology, JAPAN\r\n"
		"</body></html>\r\n"
		;

	SYSTIM		start, finish;
	uint16_t	len = 0;

	get_tim(&start);
	len += put_status(cepid, srbuf, ST_OK);
	len += put_content_length(cepid, srbuf, strlen(response) - 2);	/* 2 は最初の \r\n */
	len += put_str(cepid, srbuf, response);
	get_tim(&finish);
	syslog(LOG_NOTICE, "[WWWn:%02u SND] send: index.html, len: %4u, time: %lu [us]",
	                   cepid, len, (finish - start) * 1000 / SYSTIM_HZ);
	return E_OK;
	}

#if NET_COUNT_ENABLE

#define TD_TEMPLATE1	"<tr><td></td><td align=\"right\"></td>\r\n<td align=\"right\"></td>\r\n<td align=\"right\"></td>\r\n<td align=\"right\"></td>\r\n<td align=\"right\"></td>\r\n<td align=\"right\"></td></tr>\r\n"
#define TD_TEMPLATE2	"<tr><td></td><td align=\"right\"></td><td align=\"right\"></td></tr>\r\n"
#define TD_TEMPLATE3	"<tr><td></td><td align=\"right\"></td></tr>\r\n"
#define TD_TEMPLATE4	"<tr><td></td><td align=\"right\"></td><td align=\"right\"></td><td align=\"right\"></td></tr>\r\n"

#define TD_LEN1(i)	(sizeof(i) +  sizeof(TD_TEMPLATE1) + TD_DIGITS  * 6 - 2)
#define TD_LEN2(i)	(sizeof(i) +  sizeof(TD_TEMPLATE2) + TD_DIGITS  * 2 - 2)
#define TD_LEN4(i)	(sizeof(i) +  sizeof(TD_TEMPLATE4) + TD_DIGITS  * 3 - 2)
#define TD_DIGITS	20

static const char time_prefix[] =
	"経過時間: "
	;

static const char time_suffix[] =
	"<hr>\r\n"
	;

static const char table_suffix[] = "</table><br><hr>";

#if defined(SUPPORT_INET4)

static const char table_prefix_ip4[] =
	"<h2>IPv4</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

static const char table_prefix_ip6[] =
	"<h2>IPv6</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

static const char table_prefix_icmp6[] =
	"<h2>ICMPv6</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

static const char table_prefix_nd6[] =
	"<h2>近隣探索</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

#endif	/* of #if defined(SUPPORT_INET6) */

static const char table_prefix_tcp[] =
	"<h2>TCP</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

static const char table_prefix_net_buf[] =
	"<h2>ネットワークバッファ</h2><table border>\r\n"
	"<tr><th>サイズ</th>"
	"<th>用意数</th>"
	"<th>割当要求数</th>"
	"<th>割当数</th>"
	"<th>割当て<br>エラー数</th></tr>\r\n"
	;

#ifdef SUPPORT_PPP

static const char table_prefix_ppp[] =
	"<h2>PPP ネットワークインタフェース</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

static const char *ppp_item[NC_PPP_SIZE] = {
	"受信オクテット数",
	"送信オクテット数",
	"受信フレーム数",
	"送信フレーム数",
	"受信エラーフレーム数",
	"送信エラーフレーム数",
	"バッファ割り当て失敗数",
	};

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

static const char table_prefix_ether_nic[] =
	"<h2>イーサネット・ネットワークインタフェース</h2><table border>\r\n"
	"<tr><th>項目</th><th>カウント</th></tr>\r\n"
	;

static const char *ether_nic_item[NC_ETHER_NIC_SIZE] = {
	"リセット数",
	"送信セマフォ資源返却オーバー数",
	"送信タイムアウト数",
	"衝突数",
	"送信エラーフレーム数",
	"送信フレーム数",
	"送信オクテット数",
	"受信セマフォ資源返却オーバー数",
	"受信バッファ割り当て失敗数",
	"受信エラーフレーム数",
	"受信フレーム数",
	"受信オクテット数",
	};

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(SUPPORT_INET4)

static const char *ip4_item[NC_IP4_SIZE] = {
	"分割送信、フラグメント数",
	"分割送信数",
	"送信エラーデータグラム数",
	"送信データグラム数",
	"送信オクテット数",
	"分割受信タイムアウト数",
	"分割受信バッファ割り当て失敗数",
	"分割受信破棄数",
	"分割受信再構成成功数",
	"分割受信フラグメント数",
	"分割受信数",
	"オプション入力数",
	"プロトコルエラー数",
	"アドレスエラー数",
	"バージョンエラー数",
	"長さエラー数",
	"チェックサムエラー数",
	"受信エラーデータグラム数",
	"受信データグラム数",
	"受信オクテット数",
	};

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

static const char *ip6_item[NC_IP6_SIZE] = {
	"分割送信、フラグメント数",
	"分割送信数",
	"送信エラーデータグラム数",
	"送信データグラム数",
	"送信オクテット数	",
	"分割受信タイムアウト数",
	"分割受信バッファ割り当て失敗数",
	"分割受信破棄数",
	"分割受信再構成成功数",
	"分割受信フラグメント数",
	"分割受信数",
	"プロトコルエラー数	",
	"アドレスエラー数	",
	"バージョンエラー数	",
	"長さエラー数",
	"受信エラーデータグラム数",
	"受信データグラム数",
	"受信オクテット数	",
	};

static const char *icmp6_item[NC_ICMP6_SIZE] = {
	"送信エラー ICMP データ数	",
	"送信 ICMP データ数",
	"送信 ICMP オクテット数",
	"長さエラー数",
	"受信エラー ICMP データ数	",
	"受信 ICMP データ数",
	"受信 ICMP オクテット数",
	};

static const char *nd6_item[NC_ND6_SIZE] = {
	"重複アドレス検出送信数",
	"近隣要請送信数",
	"近隣要請受信数",
	"近隣通知送信数",
	"近隣通知受信数",
	"ルータ要請出力数",
	"ルータ通知受信数",
	};

#endif	/* of #if defined(SUPPORT_INET6) */

static const char *tcp_item[NC_TCP_SIZE] = {
	"能動オープン数",
	"受動オープン数",
	"RTT 更新数",
	"送信 RST 数",
	"送信 ACK 数",
	"送信緊急セグメント数",
	"送信データセグメント数",
	"再送セグメント数",
	"送信セグメント数",
	"送信制御セグメント数",
	"送信データオクテット数",
	"受信キュー解放数",
	"受信多重数",
	"受信破棄数",
	"受信 RST 数",
	"受信多重 ACK 数",
	"受信 ACK 数",
	"受信チェックサム不正数",
	"受信ヘッダ不正数",
	"受信緊急セグメント数",
	"受信データセグメント数",
	"受信セグメント数",
	"受信データオクテット数",
	"受信オクテット数",
	};

/*
 *  put_count_item1 -- カウンタの内容を出力する。グループ 1
 */

static uint16_t
put_count_item1 (ID cepid, T_WWW_RWBUF *srbuf, const char *item, T_NET_COUNT *counter)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, item);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");

	convert(buf, counter->in_octets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td>\r\n<td align=\"right\">");

	convert(buf, counter->out_octets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td>\r\n<td align=\"right\">");

	convert(buf, counter->in_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td>\r\n<td align=\"right\">");

	convert(buf, counter->out_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td>\r\n<td align=\"right\">");

	convert(buf, counter->in_err_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td>\r\n<td align=\"right\">");

	convert(buf, counter->out_err_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	return len;
	}

#ifdef SUPPORT_PPP

/*
 *  put_count_item2 -- カウンタの内容を出力する。グループ 2
 */

static uint16_t
put_count_item2 (ID cepid, T_WWW_RWBUF *srbuf, const char *item, uint32_t octets, uint32_t packets)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, item);
	len += put_str(cepid, srbuf, "</td><td>");

	convert(buf, octets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");

	convert(buf, packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	return len;
	}

/*
 *  td_len_ppp -- put_count_ppp で出力する文字数。
 */

static uint16_t
td_len_ppp (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_PPP_SIZE; ix -- > 0; )
		len += strlen(ppp_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_PPP_SIZE;
	}

/*
 *  put_count_ppp -- カウンタ (PPP) の内容を出力する。
 */

static uint16_t
put_count_ppp (ID cepid, T_WWW_RWBUF *srbuf)
{

	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;

	len += put_str(cepid, srbuf, table_prefix_ppp);

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[0]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.in_octets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[1]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.out_octets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[2]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.in_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[3]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.out_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[4]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.in_err_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[5]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp.out_err_packets, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, "<tr><td>");
	len += put_str(cepid, srbuf, ppp_item[6]);
	len += put_str(cepid, srbuf, "</td><td align=\"right\">");
	convert(buf, net_count_ppp_no_bufs, 10, TD_DIGITS, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, "</td></tr>\r\n");

	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

#endif	/* of #ifdef SUPPORT_PPP */

#if defined(SUPPORT_INET4)

/*
 *  td_len_ip4 -- put_count_ip4 で出力する文字数。
 */

static uint16_t
td_len_ip4 (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_IP4_SIZE; ix -- > 0; )
		len += strlen(ip4_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_IP4_SIZE;
	}

/*
 *  put_count_ip4 -- カウンタ (IP4) の内容を出力する。
 */

static uint16_t
put_count_ip4 (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_ip4);
	for (ix = NC_IP4_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, ip4_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_ip4[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

/*
 *  td_len_nd6 -- put_count_nd6 で出力する文字数。
 */

static uint16_t
td_len_nd6 (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_ND6_SIZE; ix -- > 0; )
		len += strlen(nd6_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_ND6_SIZE;
	}

/*
 *  put_count_nd6 -- カウンタ (ND6) の内容を出力する。
 */

static uint16_t
put_count_nd6 (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_nd6);
	for (ix = NC_ND6_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, nd6_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_nd6[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

/*
 *  td_len_icmp6 -- put_count_icmp6 で出力する文字数。
 */

static uint16_t
td_len_icmp6 (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_ICMP6_SIZE; ix -- > 0; )
		len += strlen(icmp6_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_ICMP6_SIZE;
	}

/*
 *  put_count_icmp6 -- カウンタ (ICMP6) の内容を出力する。
 */

static uint16_t
put_count_icmp6 (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_icmp6);
	for (ix = NC_ICMP6_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, icmp6_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_icmp6[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

/*
 *  td_len_ip6 -- put_count_ip6 で出力する文字数。
 */

static uint16_t
td_len_ip6 (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_IP6_SIZE; ix -- > 0; )
		len += strlen(ip6_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_IP6_SIZE;
	}

/*
 *  put_count_ip6 -- カウンタ (IP6) の内容を出力する。
 */

static uint16_t
put_count_ip6 (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_ip6);
	for (ix = NC_IP6_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, ip6_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_ip6[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  td_len_tcp -- put_count_tcp で出力する文字数。
 */

static uint16_t
td_len_tcp (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_TCP_SIZE; ix -- > 0; )
		len += strlen(tcp_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_TCP_SIZE;
	}

/*
 *  put_count_tcp -- カウンタ (TCP) の内容を出力する。
 */

static uint16_t
put_count_tcp (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_tcp);
	for (ix = NC_TCP_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, tcp_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_tcp[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

/*
 *  put_elapse_time -- 経過時間を出力する。
 */

static uint16_t
put_elapse_time (ID cepid, T_WWW_RWBUF *srbuf)
{
	SYSTIM		now, elapse;
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;

	get_tim(&now);
	elapse = now - srv_start;

	len += put_str(cepid, srbuf, time_prefix);

	convert(buf, elapse / (60 * 60 * SYSTIM_HZ), 10, 4, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, ":");

	convert(buf, (elapse / (60 * SYSTIM_HZ)) % 60, 10, 2, false, ' ');
	len += put_str(cepid, srbuf, buf);
	len += put_str(cepid, srbuf, ":");

	convert(buf, (elapse / SYSTIM_HZ) % 60, 10, 2, false, ' ');
	len += put_str(cepid, srbuf, buf);

	len += put_str(cepid, srbuf, time_suffix);

	return len;
	}

/*
 *  put_count_net_buf -- カウンタ (TCP) の内容を出力する。
 */

static uint16_t
put_count_net_buf (ID cepid, T_WWW_RWBUF *srbuf)
{

	const T_NET_BUF_ENTRY	*tbl;
	char			buf[TD_DIGITS + 1];
	uint16_t		len = 0;
	int_t			ix;

	len += put_str(cepid, srbuf, table_prefix_net_buf);
	tbl = nbuf_get_tbl();
	for (ix = nbuf_get_tbl_size(); ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td align=\"right\">");
		convert(buf, tbl[ix].size, 10, 4, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, tbl[ix].prepares, 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, tbl[ix].requests, 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, tbl[ix].allocs, 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, tbl[ix].errors, 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

#ifdef SUPPORT_ETHER

/*
 *  td_len_ether_nic -- put_count_ether_nic で出力する文字数。
 */

static uint16_t
td_len_ether_nic (void)
{
	int_t		ix;
	uint16_t	len = 0;

	for (ix = NC_ETHER_NIC_SIZE; ix -- > 0; )
		len += strlen(ether_nic_item[ix]);
	return len + (sizeof(TD_TEMPLATE3) + TD_DIGITS - 1) * NC_ETHER_NIC_SIZE;
	}

/*
 *  put_count_ether_nic -- カウンタ (ETHER_NIC) の内容を出力する。
 */

static uint16_t
put_count_ether_nic (ID cepid, T_WWW_RWBUF *srbuf)
{
	char		buf[TD_DIGITS + 1];
	uint16_t	len = 0;
	int_t		ix;

	len += put_str(cepid, srbuf, table_prefix_ether_nic);
	for (ix = NC_ETHER_NIC_SIZE; ix -- > 0; ) {
		len += put_str(cepid, srbuf, "<tr><td>");
		len += put_str(cepid, srbuf, ether_nic_item[ix]);
		len += put_str(cepid, srbuf, "</td><td align=\"right\">");
		convert(buf, net_count_ether_nic[ix], 10, TD_DIGITS, false, ' ');
		len += put_str(cepid, srbuf, buf);
		len += put_str(cepid, srbuf, "</td></tr>\r\n");
		}
	len += put_str(cepid, srbuf, table_suffix);

	return len;
	}

#endif	/* of #ifdef SUPPORT_ETHER */

/*
 *  stat_html -- /stat.html ファイル
 */

static ER
stat_html (ID cepid, T_WWW_RWBUF *srbuf)
{
	static const char res_prefix[] =
		"\r\n<!DOCTYPE html PUBLIC \"-//W3C//DTD html 3.2//EN\">\r\n"
		"<html><head>\r\n"

#ifdef TOPPERS_S810_CLG3_85
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\">\r\n"
#else
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=EUC-JP\">\r\n"
#endif

		"<title>ネットワーク統計情報</title>\r\n"
		"</head><body bgcolor=\"#ffffbb\">\r\n"
		"<hr><h1 align=\"center\">ネットワーク統計情報</h1><hr>\r\n"
		;

	static const char res_suffix[] =
		"</body></html>\r\n"
		;

	static const char table_prefix1[] =
		"<h2>グループ 1</h2><table border>\r\n"
		"<tr><th>項目</th>\r\n"
		"<th>受信オクテット数</th>"
		"<th>送信オクテット数</th>\r\n"
		"<th>受信パケット数</th>"
		"<th>送信パケット数</th>\r\n"
		"<th>受信エラー<br>パケット数</th>"
		"<th>送信エラー<br>パケット数</th></tr>\r\n"
		;

#ifdef SUPPORT_PPP

	static const char table_prefix2[] =
		"<h2>グループ 2</h2><table border>\r\n"
		"<tr><th>項目</th>"
		"<th>受信オクテット数</th>"
		"<th>受信フレーム数</th></tr>\r\n"
		;

#endif	/* of #ifdef SUPPORT_PPP */

	uint16_t	content_len, len = 0;
	SYSTIM		start, finish;

	get_tim(&start);
	content_len = 0

#ifdef SUPPORT_PPP

	    + TD_LEN1("HDLC")

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_PPP

	    + td_len_ppp()
	    + TD_LEN2("LCP")
	    + TD_LEN2("IPCP")

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	    + TD_LEN1("ARP")
	    + td_len_ether_nic()

#endif	/* of #ifdef SUPPORT_ETHER */

	    + TD_LEN1("ICMP")
	    + TD_LEN1("UDP")

#if defined(SUPPORT_INET4)

	    + td_len_ip4()

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	    + td_len_ip6()
	    + td_len_nd6()
	    + td_len_icmp6()

#endif	/* of #if defined(SUPPORT_INET6) */

	    + td_len_tcp()
	    + TD_LEN4("0123") * nbuf_get_tbl_size();
	    ;

	len += put_status(cepid, srbuf, ST_OK);
	len += put_content_length(cepid, srbuf, strlen(res_prefix) 
	                        + strlen(res_suffix)
	                        + strlen(time_prefix)
	                        + strlen("1234:12:12")
	                        + strlen(time_suffix)
	                        + strlen(table_prefix1)
	                        + strlen(table_prefix_net_buf)

#if defined(SUPPORT_INET4)

	                        + strlen(table_prefix_ip4)

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	                        + strlen(table_prefix_ip6)
	                        + strlen(table_prefix_nd6)
	                        + strlen(table_prefix_icmp6)

#endif	/* of #if defined(SUPPORT_INET6) */
	                        + strlen(table_prefix_tcp)
	                        + strlen(table_suffix) * 3

#ifdef SUPPORT_PPP
	                        + strlen(table_prefix2)
	                        + strlen(table_prefix_ppp)
	                        + strlen(table_suffix) * 2

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	                        + strlen(table_prefix_ether_nic)
	                        + strlen(table_suffix) * 1

#endif	/* of #ifdef SUPPORT_ETHER */

	                        + content_len
	                        - 2);	/* 2 は最初の \r\n */

	len += put_str(cepid, srbuf, res_prefix);
	len += put_elapse_time(cepid, srbuf);
	len += put_str(cepid, srbuf, table_prefix1);

#ifdef SUPPORT_PPP

	len += put_count_item1(cepid, srbuf, "HDLC", &net_count_hdlc);

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	len += put_count_item1(cepid, srbuf, "Ethernet", &net_count_ether);

#if defined(SUPPORT_INET4)

	len += put_count_item1(cepid, srbuf, "ARP", &net_count_arp);

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(SUPPORT_INET4)

	len += put_count_item1(cepid, srbuf, "ICMP", &net_count_icmp4);

#endif	/* of #if defined(SUPPORT_INET4) */

#ifdef SUPPORT_UDP

	len += put_count_item1(cepid, srbuf, "UDP", &net_count_udp);

#endif	/* of #ifdef SUPPORT_UDP */

	len += put_str(cepid, srbuf, table_suffix);

#ifdef SUPPORT_PPP

	len += put_str(cepid, srbuf, table_prefix2);
	len += put_count_item2(cepid, srbuf, "LCP", 
	                       net_count_ppp_lcp_in_octets,
	                       net_count_ppp_lcp_in_packets);
	len += put_count_item2(cepid, srbuf, "IPCP", 
	                       net_count_ppp_ipcp_in_octets,
	                       net_count_ppp_ipcp_in_packets);
	len += put_str(cepid, srbuf, table_suffix);

	len += put_count_ppp(cepid, srbuf);

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_ETHER

	len += put_count_ether_nic(cepid, srbuf);

#endif	/* of #ifdef SUPPORT_ETHER */

#if defined(SUPPORT_INET4)

	len += put_count_ip4(cepid, srbuf);

#endif	/* of #if defined(SUPPORT_INET4) */

#if defined(SUPPORT_INET6)

	len += put_count_ip6(cepid, srbuf);
	len += put_count_nd6(cepid, srbuf);
	len += put_count_icmp6(cepid, srbuf);

#endif	/* of #if defined(SUPPORT_INET6) */

	len += put_count_tcp(cepid, srbuf);

	len += put_count_net_buf(cepid, srbuf);

	len += put_str(cepid, srbuf, res_suffix);

	get_tim(&finish);
	syslog(LOG_NOTICE, "[WWWn:%02u SND]send: stat.html,  len: %4u, time: %lu [us]",
	                   cepid, len, (finish - start) * 1000 / SYSTIM_HZ);
	return E_OK;
	}

#endif	/* of #if NET_COUNT_ENABLE */

#ifdef USE_TCP_EXTENTIONS

/*
 *  get_tcp_rep -- TCP 受付口を獲得する。
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

static ER
get_tcp_rep (ID *repid)
{
	ID		tskid;
	T_TCP6_CREP	crep6;
	T_TCP_CREP	crep4;

	get_tid(&tskid);
	if (tskid == WWW4_SRV_TASK) {
		crep4.repatr = UINT_C(0);
		crep4.myaddr.portno = UINT_C(80);
		crep4.myaddr.ipaddr = IPV4_ADDRANY;
		return alloc_tcp4_rep(repid, tskid, &crep4);
		}
	else {
		crep6.repatr = UINT_C(0);
		crep6.myaddr.portno = UINT_C(80);
		memcpy(&crep6.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));
		return alloc_tcp6_rep(repid, tskid, &crep6);
		}
	}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

static ER
get_tcp_rep (ID *repid)
{
	ID		tskid;
	T_TCPN_CREP	crep;

	get_tid(&tskid);

	crep.repatr = UINT_C(0);
	crep.myaddr.portno = UINT_C(80);

#if defined(SUPPORT_INET6)

	memcpy(&crep.myaddr.ipaddr, &ipv6_addrany, sizeof(T_IN6_ADDR));

#else	/* #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
	crep.myaddr.ipaddr = IPV4_ADDRANY;
#endif

#endif	/* #if defined(SUPPORT_INET6) */

	return ALLOC_TCP_REP(repid, tskid, &crep);
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  get_tcp_cep -- TCP 通信端点を獲得する。
 */

static ER
get_tcp_cep (ID *cepid)
{
	ID		tskid;
	T_TCP_CCEP	ccep;

	get_tid(&tskid);

	ccep.cepatr = UINT_C(0);
	ccep.sbufsz = WWW_SRV_SWBUF_SIZE;
	ccep.rbufsz = WWW_SRV_RWBUF_SIZE;
	ccep.callback = NULL;

#ifdef TCP_CFG_SWBUF_CSAVE
	ccep.sbuf = NADR;
#else
	ccep.sbuf = www_srv_swbuf[0];
#endif
#ifdef TCP_CFG_RWBUF_CSAVE
	ccep.rbuf = NADR;
#else
	ccep.rbuf = www_srv_rwbuf[0];
#endif

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if (tskid == WWW4_SRV_TASK)
		return alloc_tcp4_cep(cepid, tskid, &ccep);
	else
		return alloc_tcp6_cep(cepid, tskid, &ccep);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	return ALLOC_TCP_CEP(cepid, tskid, &ccep);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

/*
 *  tcp_passive_open -- 受動オープンを実行する。
 *
 *    USE_TCP_NON_BLOCKING	OFF
 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

static ER
tcp_passive_open (T_WWW_SRV_INFO *info, char apip)
{
	ER		error = E_OK;
	T_IPV4EP	dst4;

	if (apip == API_PROTO_IPV6) {
		/* 受付口は IPv6 */
		if ((error = tcp6_acp_cep(info->cepid, info->repid, &info->dst, TMO_FEVR)) != E_OK)
			return error;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp6_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

		}
	else {
		/* 受付口は IPv4 */
		if ((error = tcp_acp_cep(info->cepid, info->repid, &dst4, TMO_FEVR)) != E_OK)
			return error;
		in6_make_ipv4mapped (&info->dst.ipaddr, dst4.ipaddr);
		info->dst.portno = dst4.portno;

#ifdef USE_TCP_EXTENTIONS
		if ((error = free_tcp4_rep(info->repid, true)) != E_OK)
			return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */
		}

	return error;
	}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

static ER
tcp_passive_open (T_WWW_SRV_INFO *info, char apip)
{
	ER		error = E_OK;

	if ((error = TCP_ACP_CEP(info->cepid, info->repid, &info->dst, TMO_FEVR)) != E_OK)
		return error;

#ifdef USE_TCP_EXTENTIONS
	if ((error = FREE_TCP_REP(info->repid, true)) != E_OK)
		return error;
#endif	/* of #ifdef USE_TCP_EXTENTIONS */

	return error;
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  WWW サーバ
 */

static ER
www_srv (uint_t six, char apip)
{
	T_WWW_RWBUF	*srbuf;
	ER		error;
	SYSTIM		now;

	if ((error = tcp_passive_open(&www_srv_info[six], apip)) != E_OK)
		return error;

	syscall(get_tim(&now));

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV)

	if (apip == API_PROTO_IPV6)
		syslog(LOG_NOTICE, "[WWW6:%02u ACP] conct:  %6lu,from: %s.%u",
		                   www_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ipv62str(NULL, &www_srv_info[six].dst.ipaddr),
		                   www_srv_info[six].dst.portno);
	else
		syslog(LOG_NOTICE, "[WWW4:%02u ACP] conct:  %6lu,from: %s.%u",
		                   www_srv_info[six].cepid, now / SYSTIM_HZ,
		                   ip2str(NULL, &www_srv_info[six].dst.ipaddr.s6_addr32[3]),
		                   www_srv_info[six].dst.portno);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	syslog(LOG_NOTICE, "[WWWn:%02u ACP] conct:  %6lu,from: %s.%u",
	                   www_srv_info[six].cepid, now / SYSTIM_HZ,
	                   IP2STR(NULL, &www_srv_info[six].dst.ipaddr),
	                   www_srv_info[six].dst.portno);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	if ((error = tget_mpf(MPF_WWW_RWBUF, (void*)&srbuf, TMO_FEVR)) != E_OK) {
		syslog(LOG_CRIT, "[WWWn:%02u MPF] get buffer error: %s.",
		                 www_srv_info[six].cepid, itron_strerror(error));
		srbuf = NULL;
		}
	else {
		srbuf->rbuf.len = srbuf->rbuf.index = srbuf->sbuf.index = srbuf->unget = 0;

#ifdef USE_COPYSAVE_API

		srbuf->sbuf.len = 0;

#else	/* of #ifdef USE_COPYSAVE_API */

		srbuf->sbuf.len = sizeof(srbuf->sbuf.buf);

#endif	/* of #ifdef USE_COPYSAVE_API */

		if ((error = parse_request(www_srv_info[six].cepid, srbuf)) != E_OK)
			syslog(LOG_WARNING, "[WWWn:%02u RCV] parse request error: %s",
			                    www_srv_info[six].cepid, itron_strerror(error));
		}

	if ((error = tcp_sht_cep(www_srv_info[six].cepid)) != E_OK)
		syslog(LOG_WARNING, "[WWWn:%02u SHD] shutdown error: %s", 
		                    www_srv_info[six].cepid, itron_strerror(error));

	if (srbuf != NULL) {
		while (get_char(www_srv_info[six].cepid, srbuf) != EOF)
			;
		if (srbuf != NULL && (error = rel_mpf(MPF_WWW_RWBUF, srbuf)) != E_OK)
			syslog(LOG_WARNING, "[WWWn:%02u REL] release buffer error: %s.",
			                    www_srv_info[six].cepid, itron_strerror(error));
		}

	if ((error = tcp_cls_cep(www_srv_info[six].cepid, TMO_FEVR)) != E_OK)
		syslog(LOG_WARNING, "[WWWn:%02u CLS] close error: %s",
 		                    www_srv_info[six].cepid, itron_strerror(error));

	get_tim(&now);
	syslog(LOG_NOTICE, "[WWWn:%02u FIN] finsh:  %6lu",
	                   www_srv_info[six].cepid, now / SYSTIM_HZ);

	return error;
	}

#ifdef USE_TCP_EXTENTIONS

/*
 *  WWW サーバタスク
 */

void
www_srv_task(intptr_t exinf)
{
	ID	tskid;
	ER	error = E_OK;
	uint_t	six;
	char	apip;

	syscall(get_tid(&tskid));
	six = INDEX_SRV_INFO((ID)exinf);

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV)

	if (six >= NUM_WWW4_SRV_TASKS)
		apip = API_PROTO_IPV6;
	else
		apip = API_PROTO_IPV4;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	apip = API_PROTO_IPVn;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	syslog(LOG_NOTICE, "[WWW%c:%d] started.", apip, tskid);
	while (true) {

		syscall(slp_tsk());
		if ((error = get_tcp_cep(&www_srv_info[six].cepid)) != E_OK) {
			syslog(LOG_NOTICE, "[WWW%c:%02u EXT] create CEP error: %s",
				           apip, www_srv_info[six].cepid, itron_strerror(error));
			continue;
			}

		while (true) {

			if ((error = get_tcp_rep(&www_srv_info[six].repid)) != E_OK) {
				syslog(LOG_NOTICE, "[WWW%c:%02u EXT] create REP error: %02u, %s",
				                   apip, www_srv_info[six].cepid, www_srv_info[six].repid, itron_strerror(error));
				break;
				}
			else if ((error = www_srv(six, apip)) != E_OK) {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

				if (tskid == WWW4_SRV_TASK)
					error = free_tcp4_rep(www_srv_info[six].repid, error != E_DLT);
				else
					error = free_tcp6_rep(www_srv_info[six].repid, error != E_DLT);

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

				error = FREE_TCP_REP(www_srv_info[six].repid, error != E_DLT);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

				if (error != E_OK) {
					syslog(LOG_NOTICE, "[WWW%c:%02u EXT] delete REP error: %02u, %s",
				                           apip, www_srv_info[six].cepid,
				                                              www_srv_info[six].repid, itron_strerror(error));

					}
				break;
				}
			}

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

		if (tskid == WWW4_SRV_TASK) {
			if ((error = free_tcp4_cep(www_srv_info[six].cepid)) != E_OK)
				syslog(LOG_NOTICE, "[WWW%c:%02u EXT] delete CEP error: %s",
					           apip, www_srv_info[six].cepid, itron_strerror(error));
			}
		else {
			if ((error = free_tcp6_cep(www_srv_info[six].cepid)) != E_OK)
				syslog(LOG_NOTICE, "[WWW%c:%02u EXT] delete CEP error: %s",
					           apip, www_srv_info[six].cepid, itron_strerror(error));
			}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		if ((error = FREE_TCP_CEP(www_srv_info[six].cepid)) != E_OK)
			syslog(LOG_NOTICE, "[WWW%c:%02u EXT] delete CEP error: %s",
				           apip, www_srv_info[six].cepid, itron_strerror(error));

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		}
	}

/*
 *  wakeup_www_srv -- WWW サーバタスクの休止解除
 */

ER
wakeup_www_srv (char apip)
{
	ID	taskid;

	if (apip == API_PROTO_IPV6)
		taskid = WWW6_SRV_TASK;
	else
		taskid = WWW4_SRV_TASK;
	syslog(LOG_NOTICE, "[WWW%c(EXT):%d] wake up.", apip, taskid);
	return wup_tsk(taskid);
	}

#else	/* of #ifdef USE_TCP_EXTENTIONS */

/*
 *  WWW サーバタスク
 */

void
www_srv_task(intptr_t exinf)
{
	ID	tskid, cepid;
	uint_t	six;
	char	apip;

	get_tim(&srv_start);
	get_tid(&tskid);
	six = INDEX_SRV_INFO((ID)exinf);
	cepid = www_srv_info[INDEX_SRV_INFO((ID)exinf)].cepid;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV)

	if (six >= NUM_WWW4_SRV_TASKS)
		apip = API_PROTO_IPV6;
	else
		apip = API_PROTO_IPV4;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	apip = API_PROTO_IPVn;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && defined(USE_WWW4_SRV) */

	syslog(LOG_NOTICE, "[WWW%c SRV:%d,%d] started.", apip, tskid, cepid);

	while (true) {
		while (www_srv(six, apip) == E_OK)
			;
		}
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#endif	/* of #ifdef USE_WWW_SRV */
