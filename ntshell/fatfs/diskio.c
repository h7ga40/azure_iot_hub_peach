/*-----------------------------------------------------------------------*/
/* PFF - Low level disk control module for Win32        (C)ChaN, 2014    */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include "ffconf.h"
#include "diskio.h"

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

#define MAX_DRIVES	1	/* Max number of physical drives to be used */

extern unsigned char RamDisk[];

typedef struct {
	DSTATUS	status;
	WORD sz_sector;
	DWORD wofs;
	DWORD wip;
	DWORD n_sectors;
} RD_STAT;

static volatile
RD_STAT Stat[MAX_DRIVES];

static
BYTE Buffer[SS_RAMDISK];

static
int get_status(volatile RD_STAT *stat) {
	stat->sz_sector = SS_RAMDISK;
	if(stat->sz_sector < FF_MIN_SS || stat->sz_sector > FF_MAX_SS) return 0;
	stat->n_sectors = SZ_RAMDISK * 1024 / SS_RAMDISK;
	stat->status = 0;
	return 1;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS ramdisk_initialize (void)
{
	DSTATUS sta;

	get_status(&Stat[0]);
	sta = Stat[0].status;
	Stat[0].wip = 0;

	return sta;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS ramdisk_get_status (void)
{
	return Stat[0].status;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
DRESULT ramdisk_read (
	BYTE* buff,		/* Data read buffer */
	DWORD sector,	/* Sector number (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	int dofs;


	if(Stat[0].status & STA_NOINIT) return RES_NOTRDY;

	if(sector >= Stat[0].n_sectors || !count || (sector + count) > Stat[0].n_sectors/* || Stat[0].wip*/) {
		res = RES_PARERR;
	}
	else {
		if(buff) {
			dofs = sector * Stat[0].sz_sector;
			memcpy(buff, &RamDisk[dofs], count * Stat[0].sz_sector);
			res = RES_OK;
		}
		else {
			res = RES_OK;
		}
	}

	return res;
}

/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/
DRESULT ramdisk_write (
	const BYTE* buff,	/* Pointer to the write data */
	DWORD sc,			/* Sector number (LBA), Number of bytes to send */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;
	int dofs;

	if(Stat[0].status & STA_NOINIT) return RES_NOTRDY;

	if(!buff) {
		if(sc) {
			Stat[0].wip = sc;
			Stat[0].wofs = 0;
			memset(Buffer, 0, sizeof Buffer);
			res = RES_OK;
		}
		else {
			if(Stat[0].wip) {
				dofs = Stat[0].wip * Stat[0].sz_sector;
				memcpy(RamDisk + dofs, Buffer, Stat[0].sz_sector);
				Stat[0].wip = 0;
				res = RES_OK;
			}
			else {
				res = RES_OK;
			}
		}
	}
	else {
		if(Stat[0].wofs + sc > Stat[0].sz_sector) sc = Stat[0].sz_sector - Stat[0].wofs;
		memcpy(&Buffer[Stat[0].wofs], buff, sc);
		Stat[0].wofs += sc;
		res = RES_OK;
	}

	return res;
}
