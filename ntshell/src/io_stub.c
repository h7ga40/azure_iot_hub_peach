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
#include "ff.h"
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "fdtable.h"
#include "util/ntstdio.h"
#include "usrcmd.h"
#include "core/ntlibc.h"
#include "kernel_cfg.h"
#include "target_syssvc.h"

struct SHELL_DIR {
	FATFS_DIR dir;
	struct dirent dirent;
};

int fresult2errno(FRESULT res)
{
	switch (res) {
	case FR_INVALID_OBJECT:
		return -EINVAL;
	case FR_TOO_MANY_OPEN_FILES:
		return -ENOMEM;
	case FR_NO_FILE:
	case FR_NO_PATH:
	case FR_INVALID_DRIVE:
	case FR_INVALID_NAME:
		return -ENOENT;
	case FR_DISK_ERR:
	case FR_NO_FILESYSTEM:
	case FR_NOT_ENABLED:
		return -ENODEV;
	case FR_WRITE_PROTECTED:
	case FR_DENIED:
		return -EACCES;
	case FR_EXIST:
		return -EEXIST;
	case FR_INT_ERR:
	default:
		return -EIO;
	}
}

static int file_close(struct SHELL_FILE *fp);
static size_t file_read(struct SHELL_FILE *fp, unsigned char *data, size_t len);
static size_t file_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len);
static off_t file_seek(struct SHELL_FILE *fp, off_t ofs, int org);
static int file_ioctl(struct SHELL_FILE *fp, int req, void *arg);
static bool_t file_readable(struct SHELL_FILE *fp);
static bool_t file_writable(struct SHELL_FILE *fp);
static void file_delete(struct SHELL_FILE *fp);

static int dir_close(struct SHELL_FILE *fp);
static size_t dir_read(struct SHELL_FILE *fp, unsigned char *data, size_t len);
static size_t dir_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len);
static off_t dir_seek(struct SHELL_FILE *fp, off_t ofs, int org);
static int dir_ioctl(struct SHELL_FILE *fp, int req, void *arg);
static bool_t dir_readable(struct SHELL_FILE *fp);
static bool_t dir_writable(struct SHELL_FILE *fp);
static void dir_delete(struct SHELL_FILE *fp);

IO_TYPE IO_TYPE_FILE = { file_close, file_read, file_write, file_seek, file_ioctl, file_readable, file_writable, file_delete };
IO_TYPE IO_TYPE_DIR = { dir_close, dir_read, dir_write, dir_seek, dir_ioctl, dir_readable, dir_writable, dir_delete };

int shell_open(const char *path, int flags, void *arg)
{
	FRESULT res;
	struct SHELL_FILE *fp;

	if (flags & O_DIRECTORY) {
		fp = new_fp(&IO_TYPE_DIR, 0, 0);
		if (fp == NULL)
			return -ENOMEM;

		fp->exinf = malloc(sizeof(struct SHELL_DIR));
		memset(fp->exinf, 0, sizeof(struct SHELL_DIR));

		FATFS_DIR *dir = &((struct SHELL_DIR *)fp->exinf)->dir;
		FRESULT res;
		if ((res = f_opendir(dir, path)) != FR_OK) {
			delete_fp(fp);
			return fresult2errno(res);
		}
		return 0;
	}

	fp = new_fp(&IO_TYPE_FILE, 0, 1);
	if (fp == NULL)
		return -ENOMEM;

	fp->exinf = malloc(sizeof(FIL));
	memset(fp->exinf, 0, sizeof(FIL));

	BYTE fmd = 0;
	switch (flags & O_ACCMODE) {
	case O_RDONLY:
		fmd = FA_READ;
		break;
	case O_WRONLY:
		fmd = FA_WRITE;
		break;
	default:
		fmd = FA_READ | FA_WRITE;
		break;
	}
	/* ファイルを作成 */
	if (flags & O_CREAT) {
		/* 既存の内容は消す */
		if (flags & O_TRUNC) {
			fmd |= FA_CREATE_ALWAYS;
		}
		/* 新規作成の保障 */
		else if (flags & O_EXCL) {
			fmd |= FA_CREATE_NEW;
		}
		else {
			fmd |= FA_OPEN_ALWAYS;
		}
	}
	/* ある場合は開く */
	else {
		/* 既存の内容は消す */
		if (flags & O_TRUNC) {
			fmd |= FA_CREATE_ALWAYS;
		}
	}

	if ((res = f_open((FIL *)fp->exinf, path, fmd)) == FR_OK) {
		fp->handle = fp->fd;
		return fp->fd;
	}

	delete_fp(fp);
	return fresult2errno(res);
}

int file_close(struct SHELL_FILE *fp)
{
	FRESULT res;

	if ((res = f_close((FIL *)fp->exinf)) == FR_OK) {
		return 0;
	}

	return -EINVAL;
}

size_t file_read(struct SHELL_FILE *fp, unsigned char *data, size_t len)
{
	unsigned int ret = 0;
	FRESULT res;

	if ((res = f_read((FIL *)fp->exinf, data, len, &ret)) != FR_OK)
		return -EIO;

	return ret;
}

size_t file_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len)
{
	unsigned int ret = 0;
	FRESULT res;

	if ((res = f_write((FIL *)fp->exinf, data, len, &ret)) != FR_OK)
		return -EIO;

	return ret;
}

off_t file_seek(struct SHELL_FILE *fp, off_t ptr, int dir)
{
	switch (dir) {
	case SEEK_SET:
		dir = F_SEEK_SET;
		break;
	case SEEK_CUR:
		dir = F_SEEK_CUR;
		break;
	case SEEK_END:
		dir = F_SEEK_END;
		break;
	default:
		return -EINVAL;
	}

	FRESULT res;
	if ((res = f_seek((FIL *)fp->exinf, ptr, dir)) != FR_OK)
		return -EIO;

	return ((FIL *)fp->exinf)->fptr;
}

int file_ioctl(struct SHELL_FILE *fp, int req, void *arg)
{
	DRESULT res;

	if ((res = disk_ioctl(((FIL *)fp->exinf)->fs->drv, req, arg) != RES_OK))
		return -EINVAL;

	return 0;
}

bool_t file_readable(struct SHELL_FILE *fp)
{
	return fp->readevt_w != fp->readevt_r;
}

bool_t file_writable(struct SHELL_FILE *fp)
{
	return fp->writable && (fp->writeevt_w == fp->writeevt_r);
}

void file_delete(struct SHELL_FILE *fp)
{
	free((FIL *)fp->exinf);
	fp->exinf = NULL;
}

int shell_close(int fd)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	int ret = fp->type->close(fp);

	delete_fp(fp);

	return ret;
}

ssize_t shell_read(int fd, void *data, size_t len)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	return fp->type->read(fp, (unsigned char *)data, len);
}

int shell_readv(int fd, const struct iovec *iov, int iovcnt)
{
	int result = 0;
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	const struct iovec *end = &iov[iovcnt];
	for (; iov < end; iov++) {
		result += fp->type->read(fp, (unsigned char *)iov->iov_base, iov->iov_len);
	}

	return result;
}

ssize_t shell_write(int fd, const void *data, size_t len)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	return fp->type->write(fp, (unsigned char *)data, len);
}

int shell_writev(int fd, const struct iovec *iov, int iovcnt)
{
	int result = 0;
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	const struct iovec *end = &iov[iovcnt];
	for (; iov < end; iov++) {
		result += fp->type->write(fp, (unsigned char *)iov->iov_base, iov->iov_len);
	}

	return result;
}

int shell_llseek(int fd, off_t ptr, off_t *result, int dir)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	off_t ret = fp->type->seek(fp, ptr, dir);
	if (ret < 0)
		return ret;

	*result = ret;
	return 0;
}

int shell_fstat(int fd, struct stat * st)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFCHR;
	//st->st_blksize = 1024;
	return 0;
}

int shell_fsync(int fd)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;
	return -EIO;
}

int shell_ftruncate(int fd, off_t length)
{
	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	FRESULT res;
	if ((res = f_truncate((FIL *)fp->exinf)) != FR_OK)
		return fresult2errno(res);

	return 0;
}

int shell_fcntl(int fd, int cmd, void *arg)
{
	return shell_ioctl(fd, cmd, arg);
}

int shell_stat(const char *__restrict path, struct stat *__restrict st)
{
	FILINFO fi;
	FRESULT ret;
#if FF_USE_LFN
	static char lfn[FF_MAX_LFN + 1];   /* Buffer to store the LFN */
	fi.lfname = lfn;
	fi.lfsize = sizeof lfn;
#endif
	if (strcmp(path, ".") == 0) {
		char cwd[FF_MAX_LFN];
		if ((ret = f_getcwd(cwd, sizeof(cwd))) != FR_OK) {
			return fresult2errno(ret);
		}
		int l = strlen(cwd);
		// ルートディレクトリの場合
		if (cwd[l - 2] == ':' && cwd[l - 1] == '/') {
			st->st_size = 0;
			st->st_mtim.tv_nsec = 0;
			st->st_mtim.tv_sec = 0;
			st->st_mode = S_IFDIR;
			return 0;
		}
		if ((ret = f_stat(cwd, &fi)) != FR_OK) {
			return fresult2errno(ret);
		}
	}
	else if ((ret = f_stat(path, &fi)) != FR_OK) {
		return fresult2errno(ret);
	}

	st->st_size = fi.fsize;
	st->st_mtim.tv_nsec = 0;
	st->st_mtim.tv_sec = fi.fdate + fi.ftime;
	st->st_mode = (S_IRUSR | S_IRGRP | S_IROTH);
	st->st_mode |= (fi.fattrib & AM_RDO) ? 0 : (S_IWUSR | S_IWGRP | S_IWOTH);
	st->st_mode |= (fi.fattrib & (AM_DIR | AM_VOL)) ? S_IFDIR : S_IFREG;

	return 0;
}

int shell_lstat(const char *__restrict path, struct stat *__restrict st)
{
	return shell_stat(path, st);
}

int shell_link(const char *a, const char *b)
{
	return -EPERM;
}

int shell_unlink(const char *path)
{
	FRESULT res;

	if ((res = f_unlink(path)) != FR_OK)
		return -EIO;

	return 0;
}

int shell_rmdir(const char *path)
{
	FRESULT res;

	if ((res = f_unlink(path)) != FR_OK)
		return -EIO;

	return 0;
}

int shell_rename(const char *oldpath, const char *newpath)
{
	FRESULT res;

	if ((res = f_rename(oldpath, newpath)) != FR_OK)
		return fresult2errno(res);
	return 0;
}

#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR

int shell_mkdir(const char *path, mode_t mode)
{
	FRESULT res;

	if ((res = f_mkdir(path)) != FR_OK)
		return fresult2errno(res);

	BYTE attr = 0;
	BYTE mask = AM_RDO | AM_SYS; // AM_ARC, AM_HID

	if (mode & S_IREAD) {
		if ((mode & S_IWRITE) == 0) {
			attr |= AM_RDO;
		}
	}
	else {
		attr |= AM_SYS;
	}

	if ((res = f_chmod(path, attr, mask)) != FR_OK) {
		return fresult2errno(res);
	}

	return 0;
}

int shell_chmod(const char *path, mode_t mode)
{
	FRESULT res;
	BYTE attr = 0;
	BYTE mask = AM_RDO | AM_SYS; // AM_ARC, AM_HID

	if (mode & S_IREAD) {
		if ((mode & S_IWRITE) == 0) {
			attr |= AM_RDO;
		}
	}
	else {
		attr |= AM_SYS;
	}

	if ((res = f_chmod(path, attr, mask)) != FR_OK) {
		return fresult2errno(res);
	}

	return 0;
}

char *shell_getcwd(char *buf, size_t size)
{
	FRESULT ret;
	if ((ret = f_getcwd(buf, size)) != FR_OK) {
		return NULL;
	}

	return buf;
}

int shell_chdir(const char *path)
{
	FRESULT res;
	if ((res = f_chdir(path)) != FR_OK) {
		return fresult2errno(res);
	}

	return 0;
}

int shell_chroot(const char *path)
{
	shell_abort();
	return -EPERM;
}

int dir_close(struct SHELL_FILE *fp)
{
	FRESULT res;
	if ((res = f_closedir(&((struct SHELL_DIR *)fp->exinf)->dir)) != FR_OK) {
		return fresult2errno(res);
	}

	return 0;
}

int shell_getdents(int fd, struct dirent *de, size_t len)
{
	if (len < sizeof(struct dirent))
		return -EINVAL;

	struct SHELL_FILE *fp = fd_to_fp(fd);
	if (fp == NULL)
		return -EBADF;

	FILINFO fno;
#if FF_USE_LFN
	static char lfn[FF_MAX_LFN + 1];   /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
#endif
	FRESULT res;
	if ((res = f_readdir(&((struct SHELL_DIR *)fp->exinf)->dir, &fno)) != FR_OK || fno.fname[0] == '\0') {
		return fresult2errno(res);
	}

	memset(de, 0, sizeof(*de));
#if FF_USE_LFN
	ntlibc_strlcpy(de->d_name, *fno.lfname ? fno.lfname : fno.fname, sizeof(de->d_name));
#else
	ntlibc_strlcpy(de->d_name, fno.fname, sizeof(de->d_name));
#endif

	return 0;
}

size_t dir_read(struct SHELL_FILE *fp, unsigned char *data, size_t len)
{
	return -EPERM;
}

size_t dir_write(struct SHELL_FILE *fp, const unsigned char *data, size_t len)
{
	return -EPERM;
}

off_t dir_seek(struct SHELL_FILE *fp, off_t ptr, int dir)
{
	FRESULT res;

	if (dir != SEEK_SET)
		return -EINVAL;

	if (ptr == 0) {
		if ((res = f_rewinddir(&((struct SHELL_DIR *)fp->exinf)->dir)) != FR_OK) {
			return fresult2errno(res);
		}
	}
	else {
		FILINFO fno;
#if FF_USE_LFN
		static char lfn[FF_MAX_LFN + 1];   /* Buffer to store the LFN */
		fno.lfname = lfn;
		fno.lfsize = sizeof lfn;
#endif
		if ((res = f_rewinddir(&((struct SHELL_DIR *)fp->exinf)->dir)) != FR_OK) {
			return fresult2errno(res);
		}

		for (int i = 0; i < ptr; i++) {
			if ((res = f_readdir(&((struct SHELL_DIR *)fp->exinf)->dir, &fno)) != FR_OK || fno.fname[0] == '\0') {
				return fresult2errno(res);
			}
		}
	}

	return ptr;
}

int dir_ioctl(struct SHELL_FILE *fp, int req, void *arg)
{
	return -EINVAL;
}

bool_t dir_readable(struct SHELL_FILE *fp)
{
	return fp->readevt_w != fp->readevt_r;
}

bool_t dir_writable(struct SHELL_FILE *fp)
{
	return false;
}

void dir_delete(struct SHELL_FILE *fp)
{
	free((struct SHELL_DIR *)fp->exinf);
	fp->exinf = NULL;
}

pid_t shell_getpid(void)
{
	return 1;
}

int shell_access(const char *path, int mode)
{
	struct stat st;
	int ret;

	ret = shell_stat(path, &st);
	if (ret != 0)
		return ret;

	return 0;
}
