/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2007-2016 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
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
 *  $Id$
 */

/*
 *		カーネルのターゲット依存部（GR-PEACH用）
 */

#include <kernel.h>
#include <sil.h>
#include "us_ticker_api.h"
#include <sys/types.h>
#include <errno.h>
#include "kernel_cfg.h"
#include "t_syslog.h"

extern void musl_start(const char *const args);

int main()
{
	static const char *const args[] = {
		(char *)1,
		"asp3",
		0,
		// envp
		"TZ=JST-9",
		0,
		// auxv
		0
	};
	musl_start(args);
	return 0;
}

void wait_us(int us);

void wait(float s)
{
	wait_us(s * 1000000.0f);
}

void wait_ms(int ms)
{
	wait_us(ms * 1000);
}

void wait_us(int us)
{
	ER ercd;

	ercd = dly_tsk(us);
	if (ercd == E_CTX){
		uint32_t start = us_ticker_read();
		while ((us_ticker_read() - start) < (uint32_t)us);
	}
}

int32_t IRQ_Initialize (void)
{
  GIC_Enable();
  return (0);
}

__attribute__((weak))
long SYS_poll()
{
	return -ENOSYS;
}

__attribute__((weak))
long SYS_open()
{
	return -ENOSYS;
}

void *tid_address;

__attribute__((weak))
long SYS_set_tid_address(long a) {
	tid_address = (void *)a;
	return 0;
}

__attribute__((weak))
long ARM_SYS_set_tls(long a)
{
	__asm__ __volatile__("mcr p15,0,%0,c13,c0,3" :: "r"(a));
	return 0;
}

__attribute__((weak))
long SYS_exit_group()
{
	ext_ker();
	return 0;
}

__attribute__((weak))
long SYS_exit()
{
	ext_ker();
	return 0;
}

__attribute__((weak))
long SYS_futex(long a, long b, long c, long d, long e, long f) {
	//int futex(int *uaddr, int op, int val, const struct timespec *timeout, int *uaddr2, int val3);
	return -ENOSYS;
}

#ifndef _MSC_VER
extern uint32_t __HeapBase;
extern uint32_t __HeapLimit;
extern uint32_t __end__;
#else
uint8_t __HeapBase[14 * 4096];
#define __HeapLimit __HeapBase[sizeof(__HeapBase)]
#endif

void *_sbrk(int incr)
{
    static unsigned char *heap = (unsigned char *)&__end__;
    unsigned char        *prev_heap = heap;
    unsigned char        *new_heap = heap + incr;

    if (new_heap >= (unsigned char *)&__HeapLimit) {
        errno = ENOMEM;
        return (void *) -1;
    }

    heap = new_heap;
    return (void *) prev_heap;
}

void *shell_brk(void *addr)
{
	if (addr == 0) {
		return (void *)(&__HeapBase);
	}
	if ((addr >= (void *)&__HeapBase) && (addr < (void *)&__HeapLimit)) {
		return addr;
	}
	return (void *)-1;
}

void *shell_mmap2(void *start, size_t length, int prot, int flags, int fd, off_t pgoffset)
{
	if (fd != -1)
		return (void *)-EINVAL;

	if ((length >= 0) && (length <= sizeof(&__HeapBase))) {
		return &__HeapBase;
	}
	return (void *)-1;
}

int shell_mprotect(void *addr, size_t len, int prot)
{
	//if ((addr >= (void *)&__HeapBase) && (addr + len < (void *)&__HeapLimit)) {
		return 0;
	//}
	//return -1;
}

__attribute__((weak))
long SYS_brk(long a) {
	return (long)shell_brk((void *)a);
}

__attribute__((weak))
long SYS_mmap2(long a, long b, long c, long d, long e, long f, long g) {
	return (int)shell_mmap2((void *)a, (size_t)b, (int)c, (int)d, (int)e, ((off_t)f << 32) | (off_t)g);
}

__attribute__((weak))
long SYS_mprotect(long a, long b, long c) {
	return shell_mprotect((void *)a, (size_t)b, (int)c);
}

__attribute__((weak))
long SYS_madvise()
{
	return -ENOSYS;
}

__attribute__((weak))
long SYS_mremap()
{
	return -ENOSYS;
}

__attribute__((weak))
long SYS_munmap()
{
	return -ENOSYS;
}

int malloc_lock_sem_count[TNUM_TSKID];

__attribute__((weak))
void __malloc_lock(struct _reent *_r)
{
	ER ercd;
	ID tskid = 0;

	ercd = get_tid(&tskid);
	if (ercd != E_OK) {
		goto error;
	}

	if (malloc_lock_sem_count[tskid - 1] == 0) {
		ercd = wai_sem(SEM_MALLOC);
		if (ercd != E_OK) {
			goto error;
		}
	}

	malloc_lock_sem_count[tskid - 1]++;
	return;
error:
	syslog(LOG_ERROR, "%s (%d) __malloc_lock error.",
		itron_strerror(ercd), SERCD(ercd));
	asm("bkpt #0");
	while(0);
}

__attribute__((weak))
void __malloc_unlock(struct _reent *_r)
{
	ER ercd;
	ID tskid = 0;
	int count;

	ercd = get_tid(&tskid);
	if (ercd != E_OK) {
		goto error;
	}

	malloc_lock_sem_count[tskid - 1]--;
	if (malloc_lock_sem_count[tskid - 1] > 0)
		return;

	if (malloc_lock_sem_count[tskid - 1] < 0) {
		goto error;
	}

	ercd = sig_sem(SEM_MALLOC);
	if (ercd != E_OK) {
		goto error;
	}
	return;
error:
	syslog(LOG_ERROR, "%s (%d) __malloc_unlock error.",
		itron_strerror(ercd), SERCD(ercd));
	asm("bkpt #0");
	while(0);
}
