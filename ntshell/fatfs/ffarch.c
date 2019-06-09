/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2014-2019 Cores Co., Ltd. Japan
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
 */

/* 
 *  サンプルプログラム(1)の本体
 */

#include "shellif.h"
#include <kernel.h>
#include <stdlib.h>
#include <string.h>
#include "t_stdlib.h"
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#include "kernel_cfg.h"
#include "diskio.h"
#include "sdfs.h"
#include "ff.h"
#include <stdio.h>
#include "hal/rtc_api.h"
#include "ffarch.h"

enum ffarch_state_t {
	FFS_IDLE,
	FFS_RETRY_WAIT,
};

enum ffarch_state_t ffarch_state;
volatile int ffarch_timer;
int ffarch_retry_count;
uint8_t sdfs_new_status, sdfs_prev_status;

static void ffarch_initialize();

/*
 * FatFSタスク
 */
void ffarch_task(intptr_t exinf)
{
	ER ret, ret2;
	SYSTIM prev, now;
	int timer;

	ffarch_initialize();

	ret2 = get_tim(&now);
	if (ret2 != E_OK){
		syslog(LOG_NOTICE, "[ffarch] get_tim error: %s",
			itron_strerror(ret2));
		return;
	}

	for(;;){
		prev = now;

		/* タイマー取得 */
		timer = ffarch_get_timer();

		/* 待ち */
		ret = tslp_tsk(timer);
		if ((ret != E_OK) && (ret != E_TMOUT)) {
			syslog(LOG_NOTICE, "[ffarch] tslp_tsk error: %s %d",
				itron_strerror(ret), timer);
			break;
		}

		ret2 = get_tim(&now);
		if (ret2 != E_OK) {
			syslog(LOG_NOTICE, "[ffarch] get_tim error: %s",
				itron_strerror(ret2));
			break;
		}

		/* 時間経過 */
		int elapse = now - prev;
		ffarch_progress(elapse);

		/* タイムアウト処理 */
		ffarch_timeout();
	}
}

FATFS RomDisk;

gpio_t ins;
sdfs_t sdfs;
#define WP() false

typedef struct SD {
	DSTATUS dst;
	BYTE type;
	FATFS FatFs;
} SD;
SD Sd;

bool_t romdisk_init();
bool_t SD_begin();

void ffarch_init(void)
{
	/* SD_CD */
	gpio_init_in(&ins, P7_8);

	/* SDカードを初期化 */
	sdfs_init(&sdfs, P8_5, P8_6, P8_3, P8_4, "sd");

	ffarch_state = FFS_IDLE;
	ffarch_timer = 0;
	ffarch_retry_count = 3;
	sdfs_new_status = 0;
	sdfs_prev_status = STA_NODISK;

	ffarch_timeout();

	act_tsk(FFARCH_TASK);

	if (romdisk_init()) {
		syslog(LOG_NOTICE, "ROM disk (0:) OK!");
	}
	else {
		syslog(LOG_NOTICE, "ROM disk (0:) NG!");
	}
}

void ffarch_initialize()
{
	sta_cyc(SDFS_CYC);
}

bool_t romdisk_init()
{
	DSTATUS dst;
	FRESULT res;

	if (RomDisk.fs_type != 0)
		return true;

	if ((dst = ramdisk_initialize()) != RES_OK) {
		return false;
	}

	if ((res = f_mount(&RomDisk, "0:", 1)) != FR_OK) {
		return false;
	}

	return true;
}

bool_t SD_begin()
{
	DSTATUS dst;
	FRESULT res;
	BYTE pdrv = 1, type;

	if (Sd.FatFs.fs_type != 0)
		return true;

	if ((dst = disk_initialize(pdrv)) != RES_OK) {
		return false;
	}

	if ((dst = disk_ioctl(pdrv, MMC_GET_TYPE, &type)) != RES_OK) {
		Sd.dst = dst;
		Sd.type = 0;
	}
	else {
		Sd.dst = RES_OK;
		Sd.type = type;
	}

	if ((res = f_mount(&Sd.FatFs, "1:", 1)) != FR_OK) {
		return false;
	}

	return true;
}

DSTATUS disk_initialize(BYTE pdrv)
{
	switch (pdrv) {
	case 0:
		return ramdisk_initialize();
	case 1:
		return sdfs_initialize(&sdfs);
	}
	return STA_NOINIT;
}

DSTATUS disk_status(BYTE pdrv)
{
	switch (pdrv) {
	case 0:
		return ramdisk_get_status();
	case 1:
		return sdfs._is_initialized;
	}
	return STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
	switch (pdrv) {
	case 0:
		return ramdisk_read(buff, sector, count);
	case 1:
		return sdfs_read(&sdfs, buff, sector, count);
	}
	return RES_PARERR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
	switch (pdrv) {
	case 0:
		return ramdisk_write(buff, sector, count);
	case 1:
		return sdfs_write(&sdfs, buff, sector, count);
	}
	return RES_PARERR;
}

DRESULT sdfs_ioctl(BYTE ctrl, void *buff);

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	switch (pdrv) {
	case 0:
		return RES_PARERR; /* ramdisk_ioctl(cmd, buff); */
	case 1:
		return sdfs_ioctl(cmd, buff);
	}
	return RES_PARERR;
}

DRESULT sdfs_ioctl(BYTE ctrl, void *buff)
{
	DRESULT res;
	uint32_t *dp = (uint32_t *)buff;
	uint8_t *ptr = (uint8_t *)buff;

	if (sdfs._is_initialized & STA_NOINIT)
		return RES_NOTRDY;

	res = RES_ERROR;

	switch (ctrl) {
	case CTRL_SYNC:
		if (sdfs_sync(&sdfs) == 0) {
			res = RES_OK;
		}
		break;
	case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
		res = RES_OK;
		break;
	case GET_SECTOR_COUNT:
		*(DWORD*)buff = sdfs._capacity / 512;
		res = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD*)buff = sdfs._block_len;
		res = RES_OK;
		break;
	case CTRL_TRIM:
		res = sdfs_trim(&sdfs, dp[0], dp[1]);
		break;
	case MMC_GET_TYPE:
		*ptr = sdfs._card_type;
		res = RES_OK;
		break;
	case MMC_GET_CSD:
		memcpy(ptr, sdfs._csd, sizeof(sdfs._csd));
		res = RES_OK;
		break;
	case MMC_GET_CID:
		memcpy(ptr, sdfs._cid, sizeof(sdfs._cid));
		res = RES_OK;
		break;
	case MMC_GET_OCR:
		if (sdfs_get_ocr(&sdfs, buff) == 0) {
			res = RES_OK;
		}
		break;
	case MMC_GET_SDSTAT:
		if (sdfs_get_sdstat(&sdfs, buff) == 0) {
			res = RES_OK;
		}
		break;
	default:
		res = RES_PARERR;
	}

	return res;
}

void sdfs_cychdr(intptr_t exinf)
{
	BYTE s = 0;
             
	/* Write protected */
	if (WP())
		s |= STA_PROTECT;
	else
		s &= ~STA_PROTECT;

	/* Card is in socket */
	if (gpio_read(&ins))
		s |= (STA_NODISK | STA_NOINIT);
	else
		s &= ~STA_NODISK;

	sdfs_new_status = s;

	if ((sdfs_prev_status != sdfs_new_status) && (ffarch_state == FFS_IDLE)) {
		ffarch_timer = 0;
		wup_tsk(FFARCH_TASK);
	}
}

int ffarch_get_timer(void)
{
	return ffarch_timer;
}

void ffarch_progress(int elapse)
{
	if (ffarch_timer != TMO_FEVR) {
		ffarch_timer -= elapse;
		if (ffarch_timer < 0) {
			ffarch_timer = 0;
		}
	}
}

void ffarch_timeout(void)
{
	if (ffarch_timer != 0)
		return;

	switch (ffarch_state) {
	case FFS_RETRY_WAIT:
		if (ffarch_retry_count == 0) {
			syslog(LOG_NOTICE, "SD card (1:) initialize tired...");

			ffarch_state = FFS_IDLE;
			ffarch_timer = TMO_FEVR;
			ffarch_retry_count = 3;
			break;
		}
		ffarch_retry_count--;
		//goto case FFS_IDLE;
	case FFS_IDLE:
 		/* SDカードが入れられた場合 */
 		if ((sdfs._is_initialized & STA_NOINIT)
 			|| (((sdfs_prev_status & STA_NODISK) != 0) && ((sdfs_new_status & STA_NODISK) == 0))) {
			syslog(LOG_NOTICE, "SD card initializing ...");

			if (SD_begin()) {
				syslog(LOG_NOTICE, "SD card (1:) OK!");

				/* uploadディレクトリを作成しておく */
				f_mkdir("1:/upload");

				ffarch_state = FFS_IDLE;
				ffarch_timer = TMO_FEVR;
				ffarch_retry_count = 3;
			}
			else {
				syslog(LOG_NOTICE, "SD card (1:) NG!");
				ffarch_state = FFS_RETRY_WAIT;
				ffarch_timer = 1000 * 1000;
			}
		}
		/* SDカードが抜かれた場合 */
		else if (((sdfs_prev_status & STA_NODISK) == 0) && ((sdfs_new_status & STA_NODISK) != 0)) {
			syslog(LOG_NOTICE, "SD card unmount");

			f_mount(&Sd.FatFs, "1:", 0);
			ffarch_state = FFS_IDLE;
			ffarch_timer = TMO_FEVR;
			ffarch_retry_count = 3;
		}
		if (((sdfs_prev_status ^ sdfs_new_status) & STA_PROTECT) != 0) {
			if ((sdfs_new_status & STA_PROTECT) != 0)
				sdfs._is_initialized |= STA_PROTECT;
			else
				sdfs._is_initialized &= ~STA_PROTECT;
		}
		sdfs_prev_status = sdfs_new_status;
		break;
	}
}

int ff_cre_syncobj(BYTE vol, FF_SYNC_t* sobj)
{
	return 1;
}

int ff_req_grant(FF_SYNC_t sobj)
{
	ER ret;
	ret = wai_sem(SEM_FILESYSTEM);
	return ret == E_OK;
}

void ff_rel_grant(FF_SYNC_t sobj)
{
	sig_sem(SEM_FILESYSTEM);
}

int ff_del_syncobj(FF_SYNC_t sobj)
{
	return 1;
}

void* ff_memalloc (UINT msize)
{
	return malloc(msize);
}

void ff_memfree (void* mblock)
{
	free(mblock);
}

DWORD get_fattime(void)
{
	time_t temp;
	struct tm _tm;

	temp = rtc_read();
	gmtime_r(&temp, &_tm);

	return	  ((DWORD)(_tm.tm_year - 1980) << 25)
		| ((DWORD)_tm.tm_mon << 21)
		| ((DWORD)_tm.tm_mday << 16)
		| ((DWORD)_tm.tm_hour << 11)
		| ((DWORD)_tm.tm_min << 5)
		| ((DWORD)_tm.tm_sec >> 1);
}
