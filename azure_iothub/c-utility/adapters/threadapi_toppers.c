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
#include <t_stdlib.h>
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/xlogging.h"

MU_DEFINE_ENUM_STRINGS(THREADAPI_RESULT, THREADAPI_RESULT_VALUES);

#ifdef TOPPERS_SUPPORT_DYNAMIC_CRE
#define MAX_THREADS 4
#define TASK_PRIORITY ((TMAX_TPRI + TMIN_TPRI) / 2)

typedef struct _thread
{
	ID tskid;
	int result;
} toppersThread;
static toppersThread threads[MAX_THREADS] = { 0 };
#endif

THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support multi-thread function.");
	return THREADAPI_ERROR;
#else
	THREADAPI_RESULT result;
	if ((threadHandle == NULL) ||
		(func == NULL))
	{
		result = THREADAPI_INVALID_ARG;
		LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
	}
	else {
		ER_ID ret;
		T_CTSK ctsk;
		toppersThread *thread = NULL;

		for (int i = 0; i < MAX_THREADS; i++) {
			if (threads[i].tskid == 0) {
				thread = &threads[i];
				break;
			}
		}

		memset(&ctsk, 0, sizeof(ctsk));

		ctsk.exinf = (intptr_t)arg;
		ctsk.task = (TASK)func;
		ctsk.itskpri = TASK_PRIORITY;

		ret = acre_tsk(&ctsk);
		if (ret > 0) {
			if (thread == NULL) {
				for (int i = 0; i < MAX_THREADS; i++) {
					if (threads[i].tskid == ret) {
						thread = &threads[i];
						break;
					}
				}
			}
			if (thread != NULL) {
				thread->result = 0;
			}
			if (act_tsk(ret) == E_OK) {
				*threadHandle = (THREAD_HANDLE)(ret);
				result = THREADAPI_OK;
			}
			else {
				del_tsk(ret);
				result = THREADAPI_ERROR;
				LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
			}
		}
		else {
			result = THREADAPI_NO_MEMORY;
			LogError("(result = %s)", MU_ENUM_TO_STRING(THREADAPI_RESULT, result));
		}
	}

	return result;
#endif
}

THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int *res)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support multi-thread function.");
	return THREADAPI_ERROR;
#else
	ID tskid = (ID)threadHandle;
	T_RTSK rtsk;
	toppersThread *thread = NULL;
	ER ret;

	for (;;) {
		ret = ref_tsk(tskid, &rtsk);
		if ((ret != E_OK) || (rtsk.tskstat == TTS_DMT))
			break;

		dly_tsk(10000);
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		if (threads[i].tskid == ret) {
			thread = &threads[i];
			break;
		}
	}

	if (thread != NULL) {
		*res = thread->result;
	}

	return THREADAPI_OK;
#endif
}

void ThreadAPI_Exit(int res)
{
	ER ret;
#ifdef TOPPERS_SUPPORT_DYNAMIC_CRE
	ID tskid;

	ret = get_tid(&tskid);
	if (ret != E_OK) {
		LogError("get_tid error. %s", itron_strerror(ret));
		return;
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		if (threads[i].tskid == tskid) {
			threads[i].result = res;
			break;
		}
	}
#endif
	ret = ext_tsk();
	if (ret != E_OK) {
		LogError("ext_tsk error. %s", itron_strerror(ret));
	}
}

void ThreadAPI_Sleep(unsigned int milliseconds)
{
	ER ret;

	ret = dly_tsk((RELTIM)milliseconds * 1000);
	if (ret != E_OK) {
		LogError("dly_tsk error. %s", itron_strerror(ret));
	}
}
