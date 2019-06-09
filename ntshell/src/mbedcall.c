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
#include <stdarg.h>
#include "analogin_api.h"
#include "gpio_api.h"
#include "i2c_api.h"
#include "pwmout_api.h"
//#include "rtc_api.h"
#include "serial_api.h"
#include "us_ticker_api.h"
#include "mbed_wait_api.h"
#include "../../mbed_api/src/mbedcall.h"

static inline long __exp_analogin_init(struct regs_t *r)
{
	analogin_init((analogin_t *)r->a, (PinName)r->b);
	return 0;
}

static inline long __exp_analogin_read_u16(struct regs_t *r)
{
	return (long)analogin_read_u16((analogin_t *)r->a);
}

static inline long __exp_gpio_init(struct regs_t *r)
{
	gpio_init((gpio_t *)r->a, (PinName)r->b);
	return 0;
}

static inline long __exp_gpio_mode(struct regs_t *r)
{
	gpio_mode((gpio_t *)r->a, (PinMode)r->b);
	return 0;
}

static inline long __exp_gpio_dir(struct regs_t *r)
{
	gpio_dir((gpio_t *)r->a, (PinDirection)r->b);
	return 0;
}

static inline long __exp_gpio_write(struct regs_t *r)
{
	gpio_write((gpio_t *)r->a, (int)r->b);
	return 0;
}

static inline long __exp_gpio_read(struct regs_t *r)
{
	return (long)gpio_read((gpio_t *)r->a);
}

static inline long __exp_i2c_start(struct regs_t *r)
{
	return (long)i2c_start((i2c_t *)r->a);
}

static inline long __exp_i2c_stop(struct regs_t *r)
{
	return (long)i2c_stop((i2c_t *)r->a);
}

static inline long __exp_i2c_init(struct regs_t *r)
{
	i2c_init((i2c_t *)r->a, (PinName)r->b, (PinName)r->c);
	return 0;
}

static inline long __exp_i2c_read(struct regs_t *r)
{
	return (long)i2c_read((i2c_t *)r->a, (int)r->b, (char *)r->c, (int)r->d, (int)r->e);
}

static inline long __exp_i2c_write(struct regs_t *r)
{
	return (long)i2c_write((i2c_t *)r->a, (int)r->b, (const char *)r->c, (int)r->d, (int)r->e);
}

static inline long __exp_i2c_byte_read(struct regs_t *r)
{
	return (long)i2c_byte_read((i2c_t *)r->a, (int)r->b);
}

static inline long __exp_i2c_byte_write(struct regs_t *r)
{
	return (long)i2c_byte_write((i2c_t *)r->a, (int)r->b);
}

static inline long __exp_pin_function(struct regs_t *r)
{
	return (long)pin_function((PinName)r->a, (int)r->b);
}

static inline long __exp_pin_mode(struct regs_t *r)
{
	return (long)pin_mode((PinName)r->a, (PinMode)r->b);
}

static inline long __exp_pwmout_free(struct regs_t *r)
{
	pwmout_free((pwmout_t *)r->a);
	return 0;
}

static inline long __exp_pwmout_init(struct regs_t *r)
{
	pwmout_init((pwmout_t *)r->a, (PinName)r->b);
	return 0;
}

static inline long __exp_pwmout_pulsewidth_us(struct regs_t *r)
{
	pwmout_pulsewidth_us((pwmout_t *)r->a, (int)r->b);
	return 0;
}

static inline long __exp_rtc_init(struct regs_t *r)
{
	rtc_init();
	return 0;
}

static inline long __exp_rtc_free(struct regs_t *r)
{
	rtc_free();
	return 0;
}

static inline long __exp_rtc_isenabled(struct regs_t *r)
{
	return (long)rtc_isenabled();
}

static inline long __exp_rtc_read(struct regs_t *r)
{
	return (long)rtc_read();
}

static inline long __exp_rtc_write(struct regs_t *r)
{
	rtc_write((time_t)r->a);
	return 0;
}

static inline long __exp_serial_baud(struct regs_t *r)
{
	serial_baud((serial_t *)r->a, (int)r->b);
	return 0;
}

static inline long __exp_serial_init(struct regs_t *r)
{
	serial_init((serial_t *)r->a, (PinName)r->b, (PinName)r->c);
	return 0;
}

static inline long __exp_serial_readable(struct regs_t *r)
{
	return (long)serial_readable((serial_t *)r->a);
}

static inline long __exp_serial_getc(struct regs_t *r)
{
	return (long)serial_getc((serial_t *)r->a);
}

static inline long __exp_serial_putc(struct regs_t *r)
{
	serial_putc((serial_t *)r->a, (int)r->b);
	return 0;
}

static inline long __exp_us_ticker_init(struct regs_t *r)
{
	us_ticker_init();
	return 0;
}

static inline long __exp_us_ticker_read(struct regs_t *r)
{
	return (long)us_ticker_read();
}

static inline long __exp_us_ticker_set_interrupt(struct regs_t *r)
{
	us_ticker_set_interrupt((timestamp_t)r->a);
	return 0;
}

static inline long __exp_us_ticker_disable_interrupt(struct regs_t *r)
{
	us_ticker_disable_interrupt();
	return 0;
}

static inline long __exp_us_ticker_clear_interrupt(struct regs_t *r)
{
	us_ticker_clear_interrupt();
	return 0;
}

static inline long __exp_SD_begin(struct regs_t *r)
{
	return (long)SD_begin();
}

static long __mbedcall(struct regs_t *r)
{
	long ret = -ENOSYS;

	shellif_into();

	switch (r->n) {
	case MBED_analogin_init:
		ret = __exp_analogin_init(r);
		break;
	case MBED_analogin_read_u16:
		ret = __exp_analogin_read_u16(r);
		break;
	case MBED_gpio_init:
		ret = __exp_gpio_init(r);
		break;
	case MBED_gpio_mode:
		ret = __exp_gpio_mode(r);
		break;
	case MBED_gpio_dir:
		ret = __exp_gpio_dir(r);
		break;
	case MBED_gpio_write:
		ret = __exp_gpio_write(r);
		break;
	case MBED_gpio_read:
		ret = __exp_gpio_read(r);
		break;
	case MBED_i2c_start:
		ret = __exp_i2c_start(r);
		break;
	case MBED_i2c_stop:
		ret = __exp_i2c_stop(r);
		break;
	case MBED_i2c_init:
		ret = __exp_i2c_init(r);
		break;
	case MBED_i2c_read:
		ret = __exp_i2c_read(r);
		break;
	case MBED_i2c_write:
		ret = __exp_i2c_write(r);
		break;
	case MBED_i2c_byte_read:
		ret = __exp_i2c_byte_read(r);
		break;
	case MBED_i2c_byte_write:
		ret = __exp_i2c_byte_write(r);
		break;
	case MBED_pin_function:
		ret = __exp_pin_function(r);
		break;
	case MBED_pin_mode:
		ret = __exp_pin_mode(r);
		break;
	case MBED_pwmout_free:
		ret = __exp_pwmout_free(r);
		break;
	case MBED_pwmout_init:
		ret = __exp_pwmout_init(r);
		break;
	case MBED_pwmout_pulsewidth_us:
		ret = __exp_pwmout_pulsewidth_us(r);
		break;
	case MBED_rtc_init:
		ret = __exp_rtc_init(r);
		break;
	case MBED_rtc_free:
		ret = __exp_rtc_free(r);
		break;
	case MBED_rtc_isenabled:
		ret = __exp_rtc_isenabled(r);
		break;
	case MBED_rtc_read:
		ret = __exp_rtc_read(r);
		break;
	case MBED_rtc_write:
		ret = __exp_rtc_write(r);
		break;
	case MBED_serial_baud:
		ret = __exp_serial_baud(r);
		break;
	case MBED_serial_init:
		ret = __exp_serial_init(r);
		break;
	case MBED_serial_readable:
		ret = __exp_serial_readable(r);
		break;
	case MBED_serial_getc:
		ret = __exp_serial_getc(r);
		break;
	case MBED_serial_putc:
		ret = __exp_serial_putc(r);
		break;
	case MBED_us_ticker_init:
		ret = __exp_us_ticker_init(r);
		break;
	case MBED_us_ticker_read:
		ret = __exp_us_ticker_read(r);
		break;
	case MBED_us_ticker_set_interrupt:
		ret = __exp_us_ticker_set_interrupt(r);
		break;
	case MBED_us_ticker_disable_interrupt:
		ret = __exp_us_ticker_disable_interrupt(r);
		break;
	case MBED_us_ticker_clear_interrupt:
		ret = __exp_us_ticker_clear_interrupt(r);
		break;
	case MBED_SD_begin:
		ret = __exp_SD_begin(r);
		break;
	case MBED___enable_irq:
		ret = 0;
		break;
	case MBED___disable_irq:
		ret = 0;
		break;
	default:
		asm("bkpt #0");
		break;
	}

	shellif_outof();

	return ret;
}

void __attribute__((naked)) __mbedcall_cp_asm()
{
	register long sp asm("sp");
	asm("push	{r0, r1, r2, r3, r4, r5, r7, r12, lr}");
	__mbedcall((struct regs_t *)sp);
	asm("add	sp, sp, #4");
	asm("pop	{r1, r2, r3, r4, r5, r7, r12, pc}");
}

