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
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"

#ifdef TOPPERS_SUPPORT_DYNAMIC_CRE
#define MAX_MUTEX 4
#define TASK_PRIORITY ((TMAX_TPRI + TMIN_TPRI) / 2)

typedef struct _mutex
{
	ID mtxid;
	ID tskid;
	int count;
} toppersMutex;
static toppersMutex mutexs[MAX_MUTEX] = {0};
#endif

LOCK_HANDLE Lock_Init(void)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support lock function.");
	return LOCK_ERROR;
#else
	ER_ID ret;
	T_CMTX cmtx;
	toppersMutex *mutex = NULL;

	for (int i = 0; i < MAX_MUTEX; i++)
	{
		if (mutexs[i].mtxid == 0)
		{
			mutex = &mutexs[i];
			break;
		}
	}

	if (mutex == NULL)
	{
		LogError("Not enough memory.");
	}
	else
	{
		memset(&cmtx, 0, sizeof(cmtx));

		cmtx.mtxatr = TA_TPRI;
		cmtx.ceilpri = TASK_PRIORITY;

		ret = acre_mtx(&cmtx);
		if (ret < 0)
		{
			mutex->mtxid = 0;
			mutex = NULL;
			LogError("Failed acre_mtx(%d).", ret);
		}
		else
		{
			mutex->mtxid = (ID)ret;
			mutex->count = 0;
		}
	}

	return (LOCK_HANDLE *)mutex;
#endif
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support lock function.");
	return LOCK_ERROR;
#else
	T_RMTX rmtx;
	toppersMutex *mutex = (toppersMutex *)handle;
	ID tskid = 0;
	ER ret;

	if (mutex == NULL)
	{
		LogError("Invalid argument; handle is NULL.");
		return LOCK_ERROR;
	}

	ret = get_tid(&tskid);
	if (ret < 0) {
		LogError("Failed task.");
		return LOCK_ERROR;
	}

	ret = loc_mtx(mutex->mtxid);
	if (ret == E_OBJ)
	{
		mutex->count++;
	}
	else if (ret < 0)
	{
		LogError("Failed loc_mtx(%d).", mutex->mtxid);
		return LOCK_ERROR;
	}

	mutex->tskid = tskid;

	return LOCK_OK;
#endif
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support lock function.");
	return LOCK_ERROR;
#else
	toppersMutex *mutex = (toppersMutex *)handle;
	ID tskid = 0;
	ER ret;

	if (mutex == NULL)
	{
		LogError("Invalid argument; handle is NULL.");
		return LOCK_ERROR;
	}

	ret = get_tid(&tskid);
	if (ret < 0) {
		LogError("Failed task.");
		return LOCK_ERROR;
	}

	loc_cpu();

	if ((mutex->tskid == tskid) && (mutex->count > 0))
	{
		mutex->count--;
		unl_cpu();
	}
	else
	{
		unl_cpu();

		ret = unl_mtx(mutex->mtxid);
		if (ret < 0)
		{
			LogError("Failed unl_mtx(%d).", mutex->mtxid);
			return LOCK_ERROR;
		}
	}

	return LOCK_OK;
#endif
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
#ifndef TOPPERS_SUPPORT_DYNAMIC_CRE
	LogError("TOPPERS/ASP3 does not support lock function.");
	return LOCK_ERROR;
#else
	toppersMutex *mutex = (toppersMutex *)handle;
	ER ret;

	if (mutex == NULL)
	{
		LogError("Failed argument; handle is NULL.");
		return LOCK_ERROR;
	}

	ret = del_mtx(mutex->mtxid);
	if (ret < 0)
	{
		LogError("Failed del_mtx(%d).", mutex->mtxid);
		return LOCK_ERROR;
	}

	mutex->mtxid = 0;

	return LOCK_OK;
#endif
}
