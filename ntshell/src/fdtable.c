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
#include "fdtable.h"
#include "kernel_cfg.h"
#include <string.h>
#include "util/ntstdio.h"
#include "hal/serial_api.h"

#ifdef _DEBUG
static const char THIS_FILE[] = __FILE__;
#endif

extern IO_TYPE IO_TYPE_STDIN;
extern IO_TYPE IO_TYPE_STDOUT;
extern IO_TYPE IO_TYPE_STDERR;

static struct SHELL_FILE fd_table[8 * sizeof(FLGPTN)] = {
	{ STDIN_FILENO, &IO_TYPE_STDIN, 0 },
	{ STDOUT_FILENO, &IO_TYPE_STDOUT, 0 },
	{ STDERR_FILENO, &IO_TYPE_STDERR, 0 },
};
#define fd_table_count (sizeof(fd_table) / sizeof(fd_table[0]))

struct SHELL_FILE *new_fp(IO_TYPE *type, int id, int writable)
{
	struct SHELL_FILE *fp = NULL;
	ER ret;

	ret = wai_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "wai_sem => %d", ret);
	}

	for (int fd = 3; fd < fd_table_count; fd++) {
		fp = &fd_table[fd];
		if (fp->type != NULL)
			continue;

		memset(fp, 0, sizeof(struct SHELL_FILE));
		fp->fd = fd;
		fp->type = type;
		fp->handle = id;
		fp->writable = writable;
		break;
	}

	ret = sig_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "sig_sem => %d", ret);
	}

	if (fp != NULL) {
		FLGPTN flgptn = 0;

		FD_SET(fp->fd, (fd_set *)&flgptn);

		ret = clr_flg(FLG_SELECT_WAIT, ~flgptn);
		if (ret != E_OK) {
			syslog(LOG_ERROR, "clr_flg => %d", ret);
		}
	}

	return fp;
}

struct SHELL_FILE *id_to_fd(IO_TYPE *type, int id)
{
	struct SHELL_FILE *fp = NULL;
	ER ret;

	ret = wai_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "wai_sem => %d", ret);
	}

	for (int fd = 3; fd < fd_table_count; fd++) {
		fp = &fd_table[fd];
		if ((fp->type == type) && (fp->handle == id))
			break;
	}

	ret = sig_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "sig_sem => %d", ret);
	}

	return fp;
}

int delete_fd_by_id(IO_TYPE *type, int id)
{
	struct SHELL_FILE *fp = id_to_fd(type, id);
	if (fp == NULL)
		return -EBADF;

	return delete_fp(fp);
}

int delete_fp(struct SHELL_FILE *fp)
{
	ER ret;

	fp->type->delete(fp);

	ret = wai_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "wai_sem => %d", ret);
	}

	memset(fp, 0, sizeof(struct SHELL_FILE));

	ret = sig_sem(SEM_FILEDESC);
	if (ret < 0) {
		syslog(LOG_ERROR, "sig_sem => %d", ret);
	}

	return 0;
}

struct SHELL_FILE *fd_to_fp(int fd)
{
	if ((fd < 0) || (fd >= fd_table_count))
		return NULL;
	return &fd_table[fd];
}

void memand(void *dst, void *src, size_t len)
{
	uint8_t *d = (uint8_t *)dst;
	uint8_t *s = (uint8_t *)src;
	uint8_t *e = &s[len];

	while (s < e) {
		*d++ &= *s++;
	}
}

struct fd_events {
	int count;
	fd_set readfds;
	fd_set writefds;
	fd_set errorfds;
};

ER shell_get_evts(struct fd_events *evts, TMO tmout);

#define TMO_MAX INT_MAX

int shell_select(int n, fd_set *__restrict rfds, fd_set *__restrict wfds, fd_set *__restrict efds, struct timeval *__restrict tv)
{
	ER ret;
	TMO tmout = TMO_FEVR;
	struct fd_events evts;

	if (tv != NULL) {
		if (tv->tv_sec < (TMO_MAX / 1000000))
			tmout = tv->tv_sec * 1000000 + tv->tv_usec;
		else
			tmout = TMO_MAX;
	}

	if (rfds != NULL)
		memcpy(&evts.readfds, rfds, sizeof(fd_set));
	else
		memset(&evts.readfds, 0, sizeof(fd_set));
	if (wfds != NULL)
		memcpy(&evts.writefds, wfds, sizeof(fd_set));
	else
		memset(&evts.writefds, 0, sizeof(fd_set));
	if (efds != NULL)
		memcpy(&evts.errorfds, efds, sizeof(fd_set));
	else
		memset(&evts.errorfds, 0, sizeof(fd_set));
	evts.count = 0;

	ret = shell_get_evts(&evts, tmout);
	if (ret == E_OK) {
		if (rfds != NULL)
			memand(rfds, &evts.readfds, sizeof(fd_set));
		if (wfds != NULL)
			memand(wfds, &evts.writefds, sizeof(fd_set));
		if (efds != NULL)
			memand(efds, &evts.errorfds, sizeof(fd_set));
		return evts.count;
	}
	if (ret == E_TMOUT) {
		if (rfds != NULL)
			memset(rfds, 0, sizeof(fd_set));
		if (wfds != NULL)
			memset(wfds, 0, sizeof(fd_set));
		if (efds != NULL)
			memset(efds, 0, sizeof(fd_set));
		return 0;
	}

	return -EBADF;
}

int shell_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	ER ret;
	TMO tmout;
	struct fd_events evts;

	if(timeout < 0)
		tmout = TMO_FEVR;
	else if (timeout < (TMO_MAX / 1000))
		tmout = timeout * 1000;
	else
		tmout = TMO_MAX;

	memset(&evts, 0, sizeof(evts));

	for (int i = 0; i < nfds; i++) {
		struct pollfd *pfd = &fds[i];
		int fd = pfd->fd;
		if ((fd < 0) || (fd >= fd_table_count))
			continue;

		if (pfd->events & POLLIN)
			FD_SET(fd, &evts.readfds);
		if (pfd->events & POLLOUT)
			FD_SET(fd, &evts.writefds);
		if (pfd->events & POLLERR)
			FD_SET(fd, &evts.errorfds);
		pfd->revents = 0;
	}

	ret = shell_get_evts(&evts, tmout);
	if (ret == E_OK) {
		int result = 0;
		for (int i = 0; i < nfds; i++) {
			struct pollfd *pfd = &fds[i];
			int fd = pfd->fd;
			if ((fd < 0) || (fd >= fd_table_count))
				continue;

			if (FD_ISSET(fd, &evts.readfds))
				pfd->revents |= POLLIN;
			if (FD_ISSET(fd, &evts.writefds))
				pfd->revents |= POLLOUT;
			if (FD_ISSET(fd, &evts.errorfds))
				pfd->revents |= POLLERR;
			if (pfd->revents != 0)
				result++;
		}
		return result;
	}
	if (ret == E_TMOUT) {
		return 0;
	}

	return -EBADF;
}

ER shell_get_evts(struct fd_events *evts, TMO tmout)
{
	int count;
	SYSTIM prev, now;
	FLGPTN flgptn;

	get_tim(&prev);

	for (;;) {
		ER ret;
		FLGPTN waitptn, readfds = 0, writefds = 0;
		struct SHELL_FILE *fp = NULL;

#ifndef NTSHELL_NO_SOCKET
		waitptn = *((FLGPTN *)&evts->errorfds);
#else
		waitptn = *((FLGPTN *)&evts->readfds) | *((FLGPTN *)&evts->errorfds);
#endif
		count = 0;
		for (int fd = 0; fd < fd_table_count; fd++) {
			fp = &fd_table[fd];

			if (FD_ISSET(fd, &evts->readfds)) {
				if (fp->type->readable(fp)) {
					FD_SET(fd, (fd_set *)&readfds);
					count++;
					if (fp->readevt_w == fp->readevt_r) fp->readevt_r--;
				}
				else {
					FD_SET(fd, (fd_set *)&waitptn);
				}
			}

			if (FD_ISSET(fd, &evts->writefds)) {
				if (fp->type->writable(fp)) {
					FD_SET(fd, (fd_set *)&writefds);
					count++;
					if (fp->writeevt_w == fp->writeevt_r) fp->writeevt_r--;
				}
				else {
					FD_SET(fd, (fd_set *)&waitptn);
				}
			}
		}
		memset(evts, 0, sizeof(*evts));

		if (waitptn == 0) {
			memcpy(&evts->readfds, &readfds, sizeof(evts->readfds));
			memcpy(&evts->writefds, &writefds, sizeof(evts->writefds));
			evts->count = count;
			return E_OK;
		}
		else if ((readfds | writefds) != 0) {
			set_flg(FLG_SELECT_WAIT, (readfds | writefds));
		}

		/* イベント待ち */
		flgptn = 0;
		ret = twai_flg(FLG_SELECT_WAIT, waitptn, TWF_ORW, &flgptn, tmout);
		if (ret != E_OK) {
			if (ret != E_TMOUT) {
				syslog(LOG_ERROR, "twai_flg => %d", ret);
				return ret;
			}
		}

		if (flgptn != 0) {
			flgptn &= waitptn;

			/* 受け取ったフラグのみクリア */
			ret = clr_flg(FLG_SELECT_WAIT, ~flgptn);
			if (ret != E_OK) {
				syslog(LOG_ERROR, "clr_flg => %d", ret);
			}
		}

		count = 0;
		for (int fd = 0; fd < fd_table_count; fd++) {
			fp = &fd_table[fd];

			if (fp->readevt_w != fp->readevt_r) {
				fp->readevt_r++;
				if (FD_ISSET(fd, (fd_set *)&waitptn))
					FD_SET(fd, &evts->readfds);
				count++;
			}
			if (fp->writeevt_w != fp->writeevt_r) {
				fp->writeevt_r++;
				if (FD_ISSET(fd, (fd_set *)&waitptn))
					FD_SET(fd, &evts->writefds);
				count++;
			}
			if (fp->errorevt_w != fp->errorevt_r) {
				fp->errorevt_r++;
				if (FD_ISSET(fd, (fd_set *)&waitptn))
					FD_SET(fd, &evts->errorfds);
				count++;
			}
		}

		if ((flgptn == 0) || (count > 0))
			break;

		get_tim(&now);

		SYSTIM elapse = now - prev;
		if (elapse > tmout) {
			flgptn = 0;
			break;
		}

		prev = now;
		tmout -= elapse;
	}

	evts->count = count;

	return (flgptn == 0) ? E_TMOUT : E_OK;
}

void clean_fd()
{
	struct SHELL_FILE *fp = NULL;
	for (int fd = 3; fd < fd_table_count; fd++) {
		fp = &fd_table[fd];
		if ((fp->type == 0) || (fp->fd == 0))
			continue;

		fp->type->close(fp);

		delete_fp(fp);
	}
}

int shell_ioctl(int fd, int request, void *arg)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	return fp->type->ioctl(fp, request, arg);
}

#ifdef NTSHELL_NO_SOCKET

int shell_socket(int family, int type, int protocol)
{
	return -ENOMEM;
}

int shell_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
	return -ENOMEM;
}

int shell_listen(int fd, int backlog)
{
	return -ENOMEM;
}

int shell_connect(int fd, const struct sockaddr *addr, socklen_t len)
{
	return -ENOMEM;
}

int shell_accept(int fd, struct sockaddr *__restrict addr, socklen_t *__restrict len)
{
	return -ENOMEM;
}

ssize_t shell_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t alen)
{
	return -ENOMEM;
}

ssize_t shell_sendmsg(int fd, const struct msghdr *msg, int flags)
{
	return -ENOMEM;
}

ssize_t shell_recvfrom(int fd, void *__restrict buf, size_t len, int flags, struct sockaddr *__restrict addr, socklen_t *__restrict alen)
{
	return -ENOMEM;
}

ssize_t shell_recvmsg(int fd, struct msghdr *msg, int flags)
{
	return -ENOMEM;
}

int shell_shutdown(int fd, int how)
{
	return -ENOMEM;
}

int shell_getsockopt(int fd, int level, int optname, void *optval, socklen_t *__restrict optlen)
{
	return -ENOMEM;
}

int shell_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
	return -ENOMEM;
}

int shell_getpeername(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
	return -ENOMEM;
}

int shell_getsockname(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
	return -ENOMEM;
}
#endif
