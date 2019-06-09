/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2019 Cores Co., Ltd. Japan
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
#include <stdint.h>
#include <stdlib.h>
#include <kernel.h>
#include <sil.h>
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct tick_counter_instance_tag
{
	SYSTIM last_clock_value;
	tickcounter_ms_t current_ms;
} TICK_COUNTER_INSTANCE_TAG;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
	ER ret;
	SYSTIM now;
	TICK_COUNTER_INSTANCE_TAG* result;

	result = (TICK_COUNTER_INSTANCE_TAG *)malloc(sizeof(TICK_COUNTER_INSTANCE_TAG));

	if (get_tim(&now) != E_OK) {
		LogError("tickcounter failed: time return INVALID_TIME.");
		free(result);
		result = NULL;
	}
	else {
		result->last_clock_value = now;
		result->current_ms = 0;
	}

	return (TICK_COUNTER_HANDLE)result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
	if (tick_counter != NULL)
	{
		free((TICK_COUNTER_INSTANCE_TAG *)tick_counter);
	}
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
	ER ret;
	SYSTIM now;
	int result;

	if (tick_counter == NULL || current_ms == NULL)
	{
		result = MU_FAILURE;
	}
	else
	{
		if (get_tim(&now) != E_OK) {
			result = MU_FAILURE;
		}
		else {
			TICK_COUNTER_INSTANCE_TAG* tick_counter_instance = (TICK_COUNTER_INSTANCE_TAG*)tick_counter;
			tick_counter_instance->current_ms += (now - tick_counter_instance->last_clock_value) / 1000;
			tick_counter_instance->last_clock_value = now;
			*current_ms = tick_counter_instance->current_ms;
			result = 0;
		}
	}

	return result;
}
