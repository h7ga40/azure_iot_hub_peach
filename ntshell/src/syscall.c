/*
 *  TOPPERS PROJECT Home Network Working Group Software
 *
 *  Copyright (C) 2018-2019 Cores Co., Ltd. Japan
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
#include "bits/syscall.h"
#include "shellif.h"
#include <t_syslog.h>
#include "MBRZA1H.h"

void *thread_area;
void *tid_address;

// https://os.mbed.com/users/dkato/code/FlashAccess/#652a093cf264
void flash_access_cache_control(void) {
	unsigned int assoc;

	/* ==== Cleaning and invalidation of the L1 data cache ==== */
	L1C_CleanInvalidateDCacheAll();
	__DSB();

	/* ==== Cleaning and invalidation of the L2 cache ==== */
	if (L2C_310->AUX_CNT & (1 << 16)) {
		assoc = 16;
	}
	else {
		assoc = 8;
	}
	L2C_310->INV_WAY = (1 << assoc) - 1;
	while (L2C_310->INV_WAY & ((1 << assoc) - 1)); // poll invalidate
	L2C_310->CACHE_SYNC = 0x0;

	/* ==== Invalidate all TLB entries ==== */
	MMU_InvalidateTLB();

	/* ==== Invalidate the L1 instruction cache ==== */
	L1C_InvalidateICacheAll();
	__DSB();
	__ISB();
}

long no_implement(const char *text)
{
	syslog(LOG_ERROR, text);
	DebugBreak();
	return -ENOSYS;
}

long __syscall_nr(long n, ...)
{
	long ret = -ENOSYS;
	va_list ap;

	va_start(ap, n);

	switch (n) {
	case __NR_setresgid:
		no_implement("setresgid");
		break;
	case __NR_setresuid:
		no_implement("setresuid");
		break;
	case __NR_setgid:
		no_implement("setgid");
		break;
	case __NR_setregid:
		no_implement("setregid");
		break;
	case __NR_setreuid:
		no_implement("setreuid");
		break;
	case __NR_setuid:
		no_implement("setuid");
		break;
	default:
		no_implement("syscall");
		break;
	}

	va_end(ap);

	return ret;
}

long SYS__llseek(long a, long b, long c, long d, long e) {
	return shell_llseek((int)a, ((off_t)b << 32) | (off_t)c, (off_t *)d, (int)e);
}

long SYS__newselect(long a, long b, long c, long d, long e) {
	return shell_select((int)a, (fd_set *)b, (fd_set *)c, (fd_set *)d, (struct timeval *)e);
}

long SYS_accept(long a, long b, long c) {
	return shell_accept((int)a, (struct sockaddr *)b, (socklen_t *)c);
}

long SYS_access(long a, long b) {
	return shell_access((const char *)a, (int)b);
}

long SYS_bind(long a, long b, long c) {
	return shell_bind((int)a, (const struct sockaddr *)b, (socklen_t)c);
}

long SYS_brk(long a) {
	return (long)shell_brk((void *)a);
}

long SYS_chdir(long a) {
	return shell_chdir((const char *)a);
}

long SYS_chmod(long a, long b) {
	return shell_chmod((const char *)a, (mode_t)b);
}

long SYS_chroot(long a) {
	return shell_chroot((const char *)a);
}

long SYS_clock_getres(long a, long b) {
	return shell_clock_getres((clockid_t)a, (struct timespec *)b);
}

long SYS_clock_gettime(long a, long b) {
	return shell_clock_gettime((clockid_t)a, (struct timespec *)b);
}

long SYS_clock_settime(long a, long b) {
	return shell_clock_settime((clockid_t)a, (const struct timespec *)b);
}

long SYS_close(long a) {
	return shell_close((int)a);
}

long SYS_connect(long a, long b, long c) {
	return shell_connect((int)a, (const struct sockaddr *)b, (socklen_t)c);
}

long SYS_exit(long a) {
	shell_exit((int)a);
	return 0;
}

long SYS_exit_group(long a) {
	shell_exit_group((int)a);
	return 0;
}

long SYS_fcntl64(long a, long b, long c) {
	return shell_fcntl((int)a, (int)b, (void *)c);
}

long SYS_fdatasync(long a) {
	//int fdatasync(int fd)
	return no_implement("fdatasync\n");
}

long SYS_fstat64(long a, long b) {
	return shell_fstat((int)a, (struct stat *)b);
}

long SYS_fsync(long a) {
	return shell_fsync((int)a);
}

long SYS_ftruncate64(long a, long b, long c) {
	return shell_ftruncate((int)a, ((off_t)b << 32) | (off_t)c);
}

long SYS_futex(long a, long b, long c, long d, long e, long f) {
	//int futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3);
	return no_implement("futex\n");
}

long SYS_futimesat(long a, long b, long c) {
	//int futimesat(int dirfd, const char *pathname, const struct timeval times[2])
	return no_implement("futimesat\n");
}

long SYS_getcwd(long a, long b) {
	return (long)shell_getcwd((char *)a, (size_t)b);
}

long SYS_getdents64(long a, long b, long c) {
	return shell_getdents((int)a, (struct dirent *)b, (size_t)c);
}

long SYS_getpeername(long a, long b, long c) {
	return shell_getpeername((int)a, (struct sockaddr *)b, (socklen_t *)c);
}

long SYS_getsockname(long a, long b, long c) {
	return shell_getsockname((int)a, (struct sockaddr *)b, (socklen_t *)c);
}

long SYS_getpid() {
	return shell_getpid();
}

long SYS_getsockopt(long a, long b, long c, long d, long e) {
	return shell_getsockopt((int)a, (int)b, (int)c, (void *)d, (socklen_t *)e);
}

long SYS_gettid() {
	return shell_gettid();
}

long SYS_gettimeofday(long a, long b) {
	return shell_gettimeofday((struct timeval *)a, (void *)b);
}

long SYS_getuid32() {
	return no_implement("getuid\n");
}

long SYS_ioctl(long a, long b, long c) {
	return shell_ioctl((int)a, (int)b, (void *)c);
}

long SYS_kill(long a, long b) {
	return shell_kill((int)a, (int)b);
}

long SYS_link(long a, long b) {
	return shell_link((const char *)a, (const char *)b);
}

long SYS_listen(long a, long b) {
	return shell_listen((int)a, (int)b);
}

long SYS_lstat64(long a, long b) {
	return shell_lstat((const char *__restrict)a, (struct stat *__restrict)b);
}

long SYS_madvise(long a, long b, long c) {
	return shell_madvise((void *)a, (size_t)b, (int)c);
}

long SYS_mkdir(long a, long b) {
	return shell_mkdir((const char *)a, (mode_t)b);
}

long SYS_mmap2(long a, long b, long c, long d, long e, long f, long g) {
	return (int)shell_mmap2((void *)a, (size_t)b, (int)c, (int)d, (int)e, ((off_t)f << 32) | (off_t)g);
}

long SYS_mprotect(long a, long b, long c) {
	return shell_mprotect((void *)a, (size_t)b, (int)c);
}

long SYS_munmap(long a, long b) {
	//int munmap(void *start, size_t len)
	return no_implement("munmap\n");
}

long SYS_open(long a, long b, long c) {
	return shell_open((const char *)a, (int)b, (void *)c);
}

long SYS_poll(long a, long b, long c) {
	return shell_poll((struct pollfd *)a, (nfds_t)b, (int)c);
}

long SYS_pread64(long a, long b, long c, long d) {
	//#define pread64 pread
	//ssize_t pread(int fd, void *buf, size_t size, off_t ofs)
	return no_implement("pread64\n");
}

long SYS_pwrite64(long a, long b, long c, long d) {
	//#define pwrite64 pwrite
	//ssize_t pwrite(int fd, const void *buf, size_t size, off_t ofs)
	return no_implement("pwrite64\n");
}

long SYS_read(long a, long b, long c) {
	return shell_read((int)a, (void *)b, (size_t)c);
}

long SYS_readv(long a, long b, long c) {
	return shell_readv((int)a, (const struct iovec *)b, (int)c);
}

long SYS_recv(long a, long b, long c, long d) {
	return shell_recvfrom((int)a, (void *)b, (size_t)c, (int)d, NULL, NULL);
}

long SYS_recvfrom(long a, long b, long c, long d, long e, long f) {
	return shell_recvfrom(a, (void *)b, (size_t)c, (int)d, (struct sockaddr *)e, (socklen_t *)f);
}

long SYS_recvmsg(long a, long b, long c) {
	return shell_recvmsg((int)a, (struct msghdr *)b, (size_t)c);
}

long SYS_rename(long a, long b) {
	return shell_rename((const char *)a, (const char *)b);
}

long SYS_rmdir(long a) {
	return shell_rmdir((const char *)a);
}

long SYS_rt_sigaction(long a, long b, long c, long d) {
	return shell_sigaction((int)a, (const struct k_sigaction *)b, (struct k_sigaction *)c, (size_t)d);
}

long SYS_rt_sigprocmask(long a, long b, long c) {
	return shell_sigprocmask((int)a, (const sigset_t *)b, (sigset_t *)c);
}

long SYS_rt_sigqueueinfo(long a, long b, long c) {
	//int rt_sigqueueinfo(pid_t tgid, int sig, siginfo_t *uinfo)
	return no_implement("rt_sigqueueinfo\n");
}

long SYS_send(long a, long b, long c, long d) {
	return shell_sendto((int)a, (const void *)b, (size_t)c, (int)d, NULL, 0);
}

long SYS_sendmsg(long a, long b, long c) {
	return shell_sendmsg((int)a, (const struct msghdr *)b, (size_t)c);
}

long SYS_sendto(long a, long b, long c, long d, long e, long f) {
	return shell_sendto(a, (const void *)b, (size_t)c, (int)d, (const struct sockaddr *)e, (socklen_t)f);
}

long SYS_setsockopt(long a, long b, long c, long d, long e) {
	return shell_setsockopt((int)a, (int)b, (int)c, (const void *)d, (socklen_t)e);
}

long SYS_shutdown(long a, long b) {
	return shell_shutdown((int)a, (int)b);
}

long SYS_sched_setscheduler(long a, long b, long c) {
	//int sched_setscheduler(pid_t pid, int sched, const struct sched_param *param)
	return no_implement("sched_setscheduler\n");
}

long SYS_set_robust_list(long a, long b) {
	//long set_robust_list(struct robust_list_head *head, size_t len)
	return no_implement("set_robust_list\n");
}

#ifdef SYS_set_thread_area
long SYS_set_thread_area(long a) {
	thread_area = (void *)a;
	return 0;
}
#endif

long SYS_set_tid_address(long a) {
	tid_address = (void *)a;
	return 0;
}

long SYS_socket(long a, long b, long c) {
	return shell_socket((int)a, (int)b, (int)c);
}

long SYS_stat64(long a, long b) {
	return shell_stat((const char *)a, (struct stat *)b);
}

long SYS_tkill(long a, long b) {
	return shell_tkill((int)a, (int)b);
}

long SYS_uname(long a) {
	return shell_uname((struct utsname *)a);
}

long SYS_unlink(long a) {
	return shell_unlink((const char *)a);
}

long SYS_utimensat(long a, long b, long c, long d) {
	//int utimensat(int fd, const char *path, const struct timespec times[2], int flags)
	return no_implement("utimensat\n");
}

long SYS_utimes(long a, long b) {
	//int utimes(const char *path, const struct timeval times[2])
	return no_implement("utimes\n");
}

long SYS_write(long a, long b, long c) {
	return shell_write((int)a, (const void *)b, (size_t)c);
}

long SYS_writev(long a, long b, long c) {
	return shell_writev((int)a, (const struct iovec *)b, (size_t)c);
}

long SYS_nanosleep(long a, long b)
{
	return shell_nanosleep((const struct timespec *)a, (struct timespec *)b);
}

long SYS_dup(long a) {
	//int dup(int fd)
	return no_implement("dup\n");
}

long SYS_dup2(long a, long b) {
	//int dup2(int old, int new)
	return no_implement("dup2\n");
}

long SYS_pipe(long a) {
	//int pipe(int fd[2])
	return no_implement("pipe\n");
}

long SYS_readlink(long a, long b, long c) {
	//ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
	return no_implement("readlink\n");
}

long SYS_symlink(long a, long b) {
	//int symlink(const char *existing, const char *new)
	return no_implement("symlink\n");
}

long SYS_umask(long a) {
	//mode_t umask(mode_t mode)
	return no_implement("umask\n");
}

long SYS_execve(long a, long b, long c) {
	//int execve(const char *path, char *const argv[], char *const envp[])
	return no_implement("execve\n");
}

long SYS_fork() {
	//pid_t fork(void)
	return no_implement("fork\n");
}

long SYS_wait4(long a, long b, long c, long d) {
	//pid_t wait4(pid_t pid, int *status, int options, struct rusage *usage)
	return no_implement("wait4\n");
}

long SYS_socketpair(long a, long b, long c, long d) {
	//int socketpair(int domain, int type, int protocol, int fd[2])
	return no_implement("socketpair\n");
}

long SYS_flock(long a, long b) {
	//int flock(int fd, int op)
	return no_implement("flock\n");
}

long SYS_fchdir()
{
	return no_implement("fchdir\n");
}

long SYS_getegid32()
{
	return no_implement("getegid32\n");
}

long SYS_geteuid32()
{
	return no_implement("geteuid32\n");
}

long SYS_getgid32()
{
	return no_implement("getgid32\n");
}

long SYS_pipe2()
{
	return no_implement("pipe2\n");
}

long SYS_setgid32()
{
	return no_implement("setgid32\n");
}

long SYS_setpgid()
{
	return no_implement("setpgid\n");
}

long SYS_setsid()
{
	return no_implement("setsid\n");
}

long SYS_setuid32()
{
	return no_implement("setuid32\n");
}

long SYS_tgkill()
{
	return no_implement("tgkill\n");
}

long SYS_prlimit64(long a, long b, long c, long d) {
	//#define prlimit64 prlimit
	//int prlimit(pid_t pid, int resource, const struct rlimit *new_limit, struct rlimit *old_limit)
	return no_implement("prlimit64\n");
}

long SYS_sched_getaffinity(long a, long b, long c) {
	//int sched_getaffinity(pid_t tid, size_t size, cpu_set_t *set)
	return no_implement("sched_getaffinity\n");
}

long SYS_sysinfo(long a) {
	//int sysinfo(struct sysinfo *info)
	return no_implement("sysinfo\n");
}

long SYS_ugetrlimit(long a, long b) {
	//#define SYS_getrlimit SYS_ugetrlimit
	//int getrlimit(int resource, struct rlimit *rlim)
	return no_implement("ugetrlimit\n");
}

long ARM_SYS_cacheflush() {
	flash_access_cache_control();
	return 0;
}

long ARM_SYS_set_tls(long a) {
	thread_area = (void *)a;
	__asm__ __volatile__("mcr p15,0,%0,c13,c0,3" :: "r"(thread_area));
	return 0;
}
