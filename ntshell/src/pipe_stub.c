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
 *  @(#) $Id: fdtable.c 1994 2019-07-25 10:56:14Z coas-nagasima $
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

static int pipe_close(struct SHELL_FILE *fp);
static size_t pipe_read(struct SHELL_FILE *fp, unsigned char *data, size_t len);
static size_t pipe_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len);
static off_t pipe_seek(struct SHELL_FILE *fp, off_t ofs, int org);
static int pipe_ioctl(struct SHELL_FILE *fp, int req, void *arg);
static bool_t pipe_readable(struct SHELL_FILE *fp);
static bool_t pipe_writable(struct SHELL_FILE *fp);
static void pipe_delete(struct SHELL_FILE *fp);

IO_TYPE IO_TYPE_PIPE = { pipe_close, pipe_read, pipe_write, pipe_seek, pipe_ioctl, pipe_readable, pipe_writable, pipe_delete };

struct SHELL_PIPE {
	unsigned int flags;
	struct SHELL_PIPE *pair;
	unsigned char buff[64];
};

int shell_pipe(int *fds)
{
	struct SHELL_FILE *fp1 = new_fp(&IO_TYPE_PIPE, 0, 0);
	if (fp1 == NULL)
		return -ENOMEM;
	fp1->exinf = calloc(1, sizeof(struct SHELL_PIPE));

	struct SHELL_FILE *fp2 = new_fp(&IO_TYPE_PIPE, 0, 0);
	if (fp2 == NULL) {
		delete_fp(fp1);
		return -ENOMEM;
	}
	fp2->exinf = calloc(1, sizeof(struct SHELL_PIPE));
	((struct SHELL_PIPE *)fp2->exinf)->pair = fp1;
	((struct SHELL_PIPE *)fp1->exinf)->pair = fp2;

	fds[0] = fp1->fd;
	fds[1] = fp2->fd;

	return 0;
}

int pipe_close(struct SHELL_FILE *fp)
{
	return -ENOSYS;
}

size_t pipe_read(struct SHELL_FILE *fp, unsigned char *data, size_t len)
{
	return -ENOSYS;
}

size_t pipe_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len)
{
	return -ENOSYS;
}

off_t pipe_seek(struct SHELL_FILE *fp, off_t ofs, int org)
{
	return -ENOSYS;
}

int pipe_ioctl(struct SHELL_FILE *fp, int req, void *arg)
{
	switch (req) {
	case F_GETFD:
		*(unsigned int *)arg = ((struct SHELL_PIPE *)fp->exinf)->flags;
		return 0;
	case F_SETFD:
		((struct SHELL_PIPE *)fp->exinf)->flags = (unsigned int)arg;
		return 0;
	}

	return -ENOSYS;
}

bool_t pipe_readable(struct SHELL_FILE *fp)
{
	return false;
}

bool_t pipe_writable(struct SHELL_FILE *fp)
{
	return false;
}

void pipe_delete(struct SHELL_FILE *fp)
{
	free((struct SHELL_PIPE *)fp->exinf);
	fp->exinf = NULL;
}
