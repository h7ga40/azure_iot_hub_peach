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
#ifndef SHELLIF_H
#define SHELLIF_H

#ifdef _MSC_VER
#undef NULL
#endif

#define __NEED_off_t
#define __NEED_ssize_t
#define __NEED_size_t
#define __NEED_mode_t
#define __NEED_dev_t
#define __NEED_pid_t
#define __NEED_gid_t
#define __NEED_uid_t
#define __NEED_socklen_t
#define __NEED_nlink_t
#define __NEED_blksize_t
#define __NEED_blkcnt_t
#define __NEED_ino_t
#define __NEED_time_t
#define __NEED_suseconds_t
#define __NEED_struct_timeval
#define __NEED_struct_timespec
#define __NEED_struct_iovec
#define __NEED_clockid_t
#define __NEED_socklen_t
#define __NEED_sa_family_t
#define __NEED_sigset_t

#ifndef _MSC_VER
#include <bits/alltypes.h>

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

#include <fcntl.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <time.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/socket.h>
#else
#include "../musl-1.1.18/include/bits/alltypes.h"

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

#include "../musl-1.1.18/include/bits/fcntl.h"
#include "../musl-1.1.18/include/bits/termios.h"
#include "../musl-1.1.18/include/bits/stat.h"
#include "../musl-1.1.18/include/sys/select.h"
#include "../musl-1.1.18/include/time.h"
#include "../musl-1.1.18/include/signal.h"
#include "../musl-1.1.18/include/stdio.h"
#include "../musl-1.1.18/include/sys/utsname.h"
#include "../musl-1.1.18/include/dirent.h"
#include "../musl-1.1.18/include/bits/ioctl.h"
#include "../musl-1.1.18/include/fcntl.h"
#include "../musl-1.1.18/include/bits/errno.h"
#include "../musl-1.1.18/include/sys/socket.h"
#endif

#include <stddef.h>
#include <stdarg.h>

#if !defined(_MSC_VER) && !defined(__c2__)
#define DebugBreak()	asm("bkpt #0")
#else
#ifdef _MSC_VER
void _stdcall DebugBreak();
#else
void __attribute__((stdcall)) DebugBreak();
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// time.h
#define CLOCK_REALTIME 0
int shell_nanosleep(const struct timespec *req, struct timespec *rem);

// dirent.h
typedef struct __dirstream FATFS_DIR;

int shell_getdents(int, struct dirent *, size_t);
// pool.h
typedef unsigned long nfds_t;

struct pollfd {
	int fd;
	short events;
	short revents;
};

int shell_poll(struct pollfd *, nfds_t, int);

#define POLLIN     0x001
#define POLLOUT    0x004
#define POLLERR    0x008

// stropts.h
int shell_ioctl(int, int, void *);
// unistd.h
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int shell_rmdir(const char *);
int shell_ftruncate(int, off_t);
int shell_chdir(const char *);
char *shell_getcwd(char *, size_t);
int shell_chroot(const char *);
int shell_close(int);
int shell_llseek(int, off_t, off_t *, int);
int shell_fsync(int);
ssize_t shell_read(int, void *, size_t);
int shell_readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t shell_write(int, const void *, size_t);
int shell_writev(int fd, const struct iovec *iov, int iovcnt);
int shell_link(const char *, const char *);
int shell_unlink(const char *);
pid_t shell_getpid(void);
int shell_access(const char *path, int mode);
// signal.h
int shell_sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict old);
struct k_sigaction;
int shell_sigaction(int sig, const struct k_sigaction *__restrict sa,
	struct k_sigaction *__restrict old, size_t size);
// socket.h
int shell_socket(int, int, int);
int shell_connect(int, const struct sockaddr *, socklen_t);
ssize_t shell_sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
ssize_t shell_sendmsg(int, const struct msghdr *, int);
int shell_bind(int, const struct sockaddr *, socklen_t);
int shell_listen(int, int);
int shell_accept(int, struct sockaddr *__restrict, socklen_t *__restrict);
ssize_t shell_recvfrom(int, void *__restrict, size_t, int, struct sockaddr *__restrict, socklen_t *__restrict);
ssize_t shell_recvmsg(int, struct msghdr *, int);
int shell_getsockopt(int, int, int, void *__restrict, socklen_t *__restrict);
int shell_setsockopt(int, int, int, const void *, socklen_t);
int shell_shutdown(int, int);
int shell_getpeername(int fd, struct sockaddr *__restrict addr, socklen_t *__restrict len);
int shell_getsockname(int fd, struct sockaddr *__restrict addr, socklen_t *__restrict len);
// select.h
int shell_select(int, fd_set *__restrict, fd_set *__restrict, fd_set *__restrict, struct timeval *__restrict);
// syslog.h
void vsyslog (int, const char *, va_list);
// fcntl.h
int shell_open(const char *, int, void *arg);
int shell_fcntl(int fd, int cmd, void *arg);
// stdio.h
int shell_rename(const char *, const char *);
// stat.h
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFREG 0100000

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IROTH 0004
#define S_IWOTH 0002

int shell_chmod(const char *, mode_t);
int shell_mkdir(const char *, mode_t);
int shell_stat(const char *__restrict, struct stat *__restrict);
int shell_fstat(int, struct stat *);
int shell_lstat(const char *__restrict, struct stat *__restrict);

int shell_clock_getres(clockid_t clk_id, struct timespec *res);
int shell_clock_gettime(clockid_t clk_id, struct timespec *tp);
int shell_clock_settime(clockid_t clk_id, const struct timespec *tp);
int shell_gettimeofday(struct timeval *tp, void * tzvp);

void shell_abort();
void shell_exit(int exitcd);
void shell_exit_group(int exitcd);
int shell_kill(int pid, int sig);

void *shell_brk(void *addr);
void *shell_mmap2(void *start, size_t length, int prot, int flags, int fd, off_t pgoffset);
// sys/mman.h
int shell_mprotect(void *addr, size_t len, int prot);
// TODO
int shell_clone(int, int);
int shell_dup2(int, int);
int shell_execve(const char *, char *const [], char *const []);
int shell_faccessat(int, const char *, int, int);
int shell_fchmodat(int, const char *, mode_t, int);
int shell_flock(int, int);
int shell_fork();
int shell_futex(volatile int *, int, int, void *);
int shell_futimesat(int, const char *, struct timeval *);
int shell_getegid();
int shell_geteuid();
int shell_getgid();
int shell_gettid();
int shell_getuid();
int shell_madvise(void *, size_t, int);
int shell_mremap(void *, size_t, size_t, int, void *);
int shell_munmap(void *, size_t);
int shell_pipe(int [2]);
int shell_pipe2(int [2], int);
int shell_ppoll(struct pollfd *, nfds_t, const struct timespec *,const sigset_t *);
int shell_pselect6(int, fd_set *__restrict, fd_set *__restrict,
	fd_set *__restrict, const struct timespec *__restrict, const sigset_t *__restrict);
int shell_setpgid(pid_t, pid_t);
int shell_setsid();
int shell_symlink(const char *, const char *);
int shell_tkill(int, int);
int shell_umask(mode_t);
int shell_uname(struct utsname *);
int shell_utimensat(int, const char *, const struct timespec [2], int);
int shell_utimes(const char *, struct timeval *);
// sys/resource.h
struct rusage;
int shell_wait4(pid_t, int *, int, struct rusage *);
// ↑↑↑musl

// arduino.h
int SD_begin();

char *basename(char *);

struct regs_t {
	long a;
	long b;
	long c;
	long d;
	long e;
	long f;
	long n;
};

long no_implement(const char *text);
void __mbedcall_cp_asm();

#include "ff.h"
WCHAR Utf8_to_Utf16(const char *, int *); /* in ff.c */
void Utf16_to_Utf8(unsigned char *, int *, UINT); /* in ff.c */

#ifdef __cplusplus
extern "C" {
#endif
int Kernel_StartExternal(int argc, char **argv);
#ifdef __cplusplus
}
#endif

void *malloc(size_t size);
void *calloc(size_t size, size_t count);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* SHELLIF_H */
