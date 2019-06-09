/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2017-2019 Cores Co., Ltd. Japan
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id$
 */
#include "shellif.h"
#include <stdint.h>
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#ifdef TOPPERS_OMIT_TECS
#include "target_serial.h"
#endif // TOPPERS_OMIT_TECS
#include "fdtable.h"
#include "kernel_cfg.h"
#include <string.h>
#include <errno.h>
#include "util/ntstdio.h"
#include "hal/serial_api.h"
#include "target_rename.h"
#include "target_kernel_impl.h"
#include "syssvc/siofd.h"

#ifdef _DEBUG
static const char THIS_FILE[] = __FILE__;
#endif

static unsigned char stdio_xi(struct ntstdio_t *handle);
static void stdio_xo(struct ntstdio_t *handle, unsigned char c);
static void stdio_serial_irq_handler(uint32_t id, SerialIrq event);

static int stdio_close(struct SHELL_FILE *fp);
static size_t stdio_read(struct SHELL_FILE *fp, unsigned char *data, size_t len);
static size_t stdio_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len);
static void stdio_delete(struct SHELL_FILE *fp);

static unsigned char sio_xi(struct ntstdio_t *handle);
static void sio_xo(struct ntstdio_t *handle, unsigned char c);
static void sio_serial_irq_handler(uint32_t id, SerialIrq event);

static int sio_close(struct SHELL_FILE *fp);
static size_t sio_read(struct SHELL_FILE *fp, unsigned char *data, size_t len);
static size_t sio_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len);
static off_t sio_seek(struct SHELL_FILE *fp, off_t ofs, int org);
static int sio_ioctl(struct SHELL_FILE *fp, int req, void *arg);
static bool_t sio_readable(struct SHELL_FILE *fp);
static bool_t sio_writable(struct SHELL_FILE *fp);
static void sio_delete(struct SHELL_FILE *fp);

IO_TYPE IO_TYPE_STDIN = { stdio_close, sio_read, stdio_write, sio_seek, sio_ioctl, sio_readable, sio_writable, stdio_delete };
IO_TYPE IO_TYPE_STDOUT = { stdio_close, stdio_read, sio_write, sio_seek, sio_ioctl, sio_readable, sio_writable, stdio_delete };
IO_TYPE IO_TYPE_STDERR = { stdio_close, stdio_read, sio_write, sio_seek, sio_ioctl, sio_readable, sio_writable, stdio_delete };
IO_TYPE IO_TYPE_SIO = { sio_close, sio_read, sio_write, sio_seek, sio_ioctl, sio_readable, sio_writable, sio_delete };

#define STDIO_UART_BUF_SZ 256

typedef struct stdio_sio_t {
	serial_t serial;
	ntstdio_t ntstdio;
	struct SHELL_FILE *fp;
	unsigned char tx_buf[STDIO_UART_BUF_SZ];
	int tx_pos_w;
	int tx_pos_r;
	unsigned char rx_buf[STDIO_UART_BUF_SZ];
	int rx_pos_w;
	int rx_pos_r;
} stdio_sio_t;

stdio_sio_t stdio_sio;

void sys_init(intptr_t exinf)
{
	serial_t *serial = &stdio_sio.serial;
	ntstdio_t *ntstdio = &stdio_sio.ntstdio;

	serial_init(serial, STDIO_UART_TX, STDIO_UART_RX);
	serial_baud(serial, UART_BAUDRATE);
	serial_format(serial, 8, ParityNone, 1);

	ntstdio_init(ntstdio, NTSTDIO_OPTION_LINE_ECHO | NTSTDIO_OPTION_CANON | NTSTDIO_OPTION_LF_CRLF | NTSTDIO_OPTION_LF_CR, stdio_xi, stdio_xo);
	ntstdio->exinf = (void *)&stdio_sio;

	struct SHELL_FILE *fp;

	fp = fd_to_fp(STDIN_FILENO);
	fp->handle = SIO_PORTID;
	fp->exinf = (void *)ntstdio;

	fp = fd_to_fp(STDOUT_FILENO);
	fp->handle = SIO_PORTID;
	fp->exinf = (void *)ntstdio;

	fp = fd_to_fp(STDERR_FILENO);
	fp->handle = SIO_PORTID;
	fp->exinf = (void *)ntstdio;

	serial_irq_handler(&stdio_sio.serial, stdio_serial_irq_handler, 0);

	serial_irq_set(serial, RxIrq, true);
}

ID serial_portid;

void stdio_open(ID portid)
{
	struct SHELL_FILE *fp;

	serial_portid = portid;
	serial_ctl_por(portid, IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV);

	fp = fd_to_fp(STDIN_FILENO);
	fp->handle = portid;

	fp = fd_to_fp(STDOUT_FILENO);
	fp->handle = portid;

	fp = fd_to_fp(STDERR_FILENO);
	fp->handle = portid;
}

int stdio_close(struct SHELL_FILE *fp)
{
	return -EPERM;
}

size_t stdio_read(struct SHELL_FILE *fp, unsigned char *data, size_t len)
{
	return -EPERM;
}

size_t stdio_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len)
{
	return -EPERM;
}

void stdio_delete(struct SHELL_FILE *fp)
{
}

int sio_close(struct SHELL_FILE *fp)
{
	return -EPERM;
}

size_t sio_read(struct SHELL_FILE *fp, unsigned char *data, size_t len)
{
	int i = 0;
	while (i < len) {
		int c = ntstdio_getc((struct ntstdio_t *)fp->exinf);
		data[i++] = c;
		if ((c == EOF) || (c == '\n'))
			break;
	}
	return i;
}

size_t sio_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len)
{
	for (int i = 0; i < len; i++) {
		ntstdio_putc((struct ntstdio_t *)fp->exinf, data[i]);
	}
	return len;
}

off_t sio_seek(struct SHELL_FILE *fp, off_t ofs, int org)
{
	return -EPERM;
}

int sio_tcgetattr(struct SHELL_FILE *fp, struct termios *termios)
{
	ntstdio_t *ntstdio = (ntstdio_t *)fp->exinf;

	memset(termios, 0, sizeof(*termios));

	if (ntstdio->option & NTSTDIO_OPTION_LINE_ECHO) {
		termios->c_lflag |= ECHO;
	}
	else {
		termios->c_lflag &= ~ECHO;
	}
	if (ntstdio->option & NTSTDIO_OPTION_CANON) {
		termios->c_lflag |= ICANON;
	}
	else {
		termios->c_lflag &= ~ICANON;
	}
	if (ntstdio->option & NTSTDIO_OPTION_LF_CR) {
		termios->c_iflag |= INLCR;
	}
	else {
		termios->c_iflag &= ~INLCR;
	}
	if (ntstdio->option & NTSTDIO_OPTION_LF_CRLF) {
		termios->c_oflag |= ONLCR;
	}
	else {
		termios->c_oflag &= ~ONLCR;
	}

	return 0;
}

int sio_tcsetattr(struct SHELL_FILE *fp, int optional_actions, const struct termios *termios)
{
	ntstdio_t *ntstdio = (ntstdio_t *)fp->exinf;

	if (optional_actions == TCSANOW) {
		if (termios->c_lflag & ECHO) {
			ntstdio->option |= NTSTDIO_OPTION_LINE_ECHO;
		}
		else {
			ntstdio->option &= ~NTSTDIO_OPTION_LINE_ECHO;
		}
		if (termios->c_lflag & ICANON) {
			ntstdio->option |= NTSTDIO_OPTION_CANON;
		}
		else {
			ntstdio->option &= ~NTSTDIO_OPTION_CANON;
		}
		if (termios->c_iflag & INLCR) {
			ntstdio->option |= NTSTDIO_OPTION_LF_CR;
		}
		else {
			ntstdio->option &= ~NTSTDIO_OPTION_LF_CR;
		}
		if (termios->c_oflag & ONLCR) {
			ntstdio->option |= NTSTDIO_OPTION_LF_CRLF;
		}
		else {
			ntstdio->option &= ~NTSTDIO_OPTION_LF_CRLF;
		}
		return 0;
	}

	//shell_abort();
	return 0;
}

int sio_ioctl(struct SHELL_FILE *fp, int request, void *arg)
{
	if ((fp == NULL) || (fp->type != &IO_TYPE_SIO))
		return -EBADF;

	switch (request) {
	case TIOCGWINSZ:
		return 0;
	case TCGETS:
		return sio_tcgetattr(fp, (struct termios *)arg);
	case TCSETS + TCSANOW:
	case TCSETS + TCSADRAIN:
	case TCSETS + TCSAFLUSH:
		return sio_tcsetattr(fp, request - TCSETS, (const struct termios *)arg);
	}

	return -EINVAL;
}

bool_t sio_readable(struct SHELL_FILE *fp)
{
	return fp->readevt_w != fp->readevt_r;
}

bool_t sio_writable(struct SHELL_FILE *fp)
{
	return fp->writable && (fp->writeevt_w == fp->writeevt_r);
}

void sio_delete(struct SHELL_FILE *fp)
{
	free((stdio_sio_t *)((struct ntstdio_t *)fp->exinf)->exinf);
	((struct ntstdio_t *)fp->exinf)->exinf = NULL;
	free((struct ntstdio_t *)fp->exinf);
	fp->exinf = NULL;
}

static unsigned char ntstdio_xi(struct ntstdio_t *handle, struct SHELL_FILE *fp)
{
	stdio_sio_t *uart = (stdio_sio_t *)handle->exinf;
	ER ret;
	unsigned char c;

	bool_t lock = sense_lock();
	if (lock)
		unl_cpu();

	while (uart->rx_pos_w == uart->rx_pos_r) {
		FLGPTN flgptn = 0, waitptn = 0;
		FD_SET(fp->fd, (fd_set *)&waitptn);

		ret = wai_flg(FLG_SELECT_WAIT, waitptn, TWF_ORW, &flgptn);
		if (ret != E_OK) {
			if (lock)
				loc_cpu();
			syslog(LOG_ERROR, "wai_flg => %d", ret);
			return 0;
		}

		ret = clr_flg(FLG_SELECT_WAIT, ~(flgptn & waitptn));
		if (ret != E_OK) {
			syslog(LOG_ERROR, "clr_flg => %d", ret);
		}
	}

	if (lock)
		unl_cpu();

	if (fp->readevt_w != fp->readevt_r) fp->readevt_r++;

	c = uart->rx_buf[uart->rx_pos_r++];
	if (uart->rx_pos_r >= sizeof(uart->rx_buf))
		uart->rx_pos_r = 0;

	return c;
}

extern volatile int ntshell_state;
extern ID serial_portid;

static void serial_rx_irq_handler(int fd)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	stdio_sio_t *uart = (stdio_sio_t *)((struct ntstdio_t *)fp->exinf)->exinf;
	serial_t *serial = (serial_t *)&uart->serial;

	if (serial_readable(serial)) {
		unsigned char c = (unsigned char)serial_getc(serial);

		if (ntshell_state == 1) {
			ER ret;
			FLGPTN flgptn = 0;

			uart->rx_buf[uart->rx_pos_w++] = c;
			if (uart->rx_pos_w >= sizeof(uart->rx_buf))
				uart->rx_pos_w = 0;

			if (fp->readevt_w == fp->readevt_r) fp->readevt_w++;

			FD_SET(fd, (fd_set *)&flgptn);

			ret = set_flg(FLG_SELECT_WAIT, flgptn);
			if (ret != E_OK) {
				syslog(LOG_ERROR, "set_flg => %d", ret);
			}
		}
		else
			sio_isr_rcv(fp->handle, c);
	}
}

static void ntstdio_xo(struct ntstdio_t *handle, struct SHELL_FILE *fp, unsigned char c)
{
	stdio_sio_t *uart = (stdio_sio_t *)handle->exinf;
	serial_t *serial = (serial_t *)&uart->serial;
	ER ret;

	// タスクコンテキストでない場合
	if (sense_context()) {
		// 送信可能になるまで待ち
		while (!serial_writable(serial));
		// 送信する
		serial_putc(serial, c);
		return;
	}

	// 送信バッファが空で
	if (uart->tx_pos_w == uart->tx_pos_r) {
		// 送信可能なら
		if (serial_writable(serial)) {
			// 送信する
			serial_putc(serial, c);
			return;
		}
	}

	// 送信バッファに追加する
	uart->tx_buf[uart->tx_pos_w++] = c;
	if (uart->tx_pos_w >= sizeof(uart->tx_buf))
		uart->tx_pos_w = 0;

	// 送信可能になったら割り込みをもらうよう設定
	serial_irq_set(serial, TxIrq, true);

	bool_t lock = sense_lock();
	if (lock)
		unl_cpu();

	do {
		FLGPTN flgptn = 0, waitptn = 0;
		FD_SET(fp->fd, (fd_set *)&waitptn);

		ret = wai_flg(FLG_SELECT_WAIT, waitptn, TWF_ORW, &flgptn);
		if (ret != E_OK) {
			syslog(LOG_ERROR, "wai_flg => %d", ret);
			break;
		}

		ret = clr_flg(FLG_SELECT_WAIT, ~(flgptn & waitptn));
		if (ret != E_OK) {
			syslog(LOG_ERROR, "clr_flg => %d", ret);
		}
	} while (fp->writeevt_w == fp->writeevt_r);

	fp->writeevt_r++;

	if (lock)
		loc_cpu();
}

static void serial_tx_irq_handler(int fd)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	stdio_sio_t *uart = (stdio_sio_t *)((struct ntstdio_t *)fp->exinf)->exinf;
	serial_t *serial = (serial_t *)&uart->serial;
	ER ret;
	unsigned char c;

	if (fp->writeevt_w == fp->writeevt_r) fp->writeevt_w++;

	FLGPTN flgptn = 0;
	FD_SET(fd, (fd_set *)&flgptn);

	ret = set_flg(FLG_SELECT_WAIT, flgptn);
	if (ret != E_OK) {
		syslog(LOG_ERROR, "set_flg => %d", ret);
	}

	// 送信バッファが空なら
	if (uart->tx_pos_w == uart->tx_pos_r) {
		// SIOドライバの送信
		sio_isr_snd(fp->handle);
		// やっぱり送信バッファが空なら
		if (uart->tx_pos_w == uart->tx_pos_r) {
			// 送信割り込み停止
			serial_irq_set(serial, TxIrq, false);
			return;
		}
	}

	c = uart->tx_buf[uart->tx_pos_r++];
	if (uart->tx_pos_r >= sizeof(uart->tx_buf))
		uart->tx_pos_r = 0;

	// 送信する
	serial_putc(serial, c);
}

void stdio_serial_irq_handler(uint32_t id, SerialIrq event)
{
	switch (event) {
	case RxIrq:
		serial_rx_irq_handler(STDIN_FILENO);
		break;
	case TxIrq:
		serial_tx_irq_handler(STDOUT_FILENO);
		break;
	}
}

void sio_serial_irq_handler(uint32_t id, SerialIrq event)
{
	struct SHELL_FILE *fp = fd_to_fp(id);
	if ((fp == NULL) || (fp->type != &IO_TYPE_SIO))
		return;

	switch (event) {
	case RxIrq:
		serial_rx_irq_handler(id);
		break;
	case TxIrq:
		serial_tx_irq_handler(id);
		break;
	}
}

unsigned char stdio_xi(struct ntstdio_t *handle)
{
	return ntstdio_xi(handle, fd_to_fp(STDIN_FILENO));
}

void stdio_xo(struct ntstdio_t *handle, unsigned char c)
{
	ntstdio_xo(handle, fd_to_fp(STDOUT_FILENO), c);
}

unsigned char sio_xi(struct ntstdio_t *handle)
{
	return ntstdio_xi(handle, ((stdio_sio_t *)handle->exinf)->fp);
}

void sio_xo(struct ntstdio_t *handle, unsigned char c)
{
	ntstdio_xo(handle, ((stdio_sio_t *)handle->exinf)->fp, c);
}

int siofd_open(const char *path, int flags, ...)
{
	return -1;
}

int siofd_close(int fd)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || (fp->type != &IO_TYPE_SIO)) {
		errno = EBADF;
		return -1;
	}

	errno = sio_close(fp);
	if (errno != 0)
		return 1;
	return 0;
}

ssize_t siofd_read(int fd, void *data, size_t len)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || ((fp->type != &IO_TYPE_STDIN) && (fp->type != &IO_TYPE_SIO))) {
		errno = EBADF;
		return -1;
	}
	char *pos, *end;

	for (pos = (char *)data, end = &pos[len]; pos < end; pos++) {
		*pos = ntstdio_xi((struct ntstdio_t *)fp->exinf, fp);
	}

	return pos - (char *)data;
}

ssize_t siofd_write(int fd, const void *data, size_t len)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || ((fp->type != &IO_TYPE_STDOUT) && (fp->type != &IO_TYPE_STDERR)
		&& (fp->type != &IO_TYPE_SIO))) {
		errno = EBADF;
		return -1;
	}
	const char *pos, *end;

	for (pos = (const char *)data, end = &pos[len]; pos < end; pos++) {
		ntstdio_xo((struct ntstdio_t *)fp->exinf, fp, *pos);
	}

	return pos - (char *)data;
}

int siofd_tcsetattr(int fd, int act, const struct termios *tio)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || ((fp->type != &IO_TYPE_STDIN) && (fp->type != &IO_TYPE_STDOUT)
		&& (fp->type != &IO_TYPE_STDERR) && (fp->type != &IO_TYPE_SIO))) {
		errno = EBADF;
		return -1;
	}

	errno = sio_tcsetattr(fp, act, tio);
	if (errno != 0)
		return 1;
	return 0;
}

int siofd_tcgetattr(int fd, struct termios *tio)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || ((fp->type != &IO_TYPE_STDIN) && (fp->type != &IO_TYPE_STDOUT)
		&& (fp->type != &IO_TYPE_STDERR) && (fp->type != &IO_TYPE_SIO))) {
		errno = EBADF;
		return -1;
	}

	errno = sio_tcgetattr(fp, tio);
	if (errno != 0)
		return 1;
	return 0;
}

int siofd_fcntl(int fd, int cmd, ...)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if ((fp == NULL) || ((fp->type != &IO_TYPE_STDIN) && (fp->type != &IO_TYPE_STDOUT)
		&& (fp->type != &IO_TYPE_STDERR) && (fp->type != &IO_TYPE_SIO))) {
		errno = EBADF;
		return -1;
	}
	void *arg;
	va_list ap;
	va_start(ap, cmd);
	arg = va_arg(ap, void *);
	va_end(ap);

	errno = sio_ioctl(fp, cmd, arg);
	if (errno != 0)
		return 1;
	return 0;
}
