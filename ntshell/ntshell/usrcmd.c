/**
 * @file usrcmd.c
 * @author CuBeatSystems
 * @author Shinichiro Nakamura
 * @copyright
 * ===============================================================
 * Natural Tiny Shell (NT-Shell) Version 0.3.1
 * ===============================================================
 * Copyright (c) 2010-2016 Shinichiro Nakamura
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "shellif.h"
#include <kernel.h>
#include <string.h>
#include <stdlib.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <sil.h>
#include "syssvc/syslog.h"
#include "target_syssvc.h"
#include "kernel_cfg.h"
#include "main.h"
#include "ffarch.h"
#include "ff.h"
#include "usrcmd.h"
#include "core/ntshell.h"
#include "core/ntlibc.h"
#include <stdio.h>
#include <getopt.h>

extern int ntshell_exit;

void put_rc(const char *func, FRESULT rc)
{
	const char *p =
		"OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0"
		"DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0"
		"NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0"
		"NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = 0; i != rc && *p; i++) {
		while (*p++);
	}
	printf("%s() =>%u FR_%s\n", func, (UINT)rc, p);
}

void put_drc(const char *func, DRESULT rc)
{
	const char *p =
		"Successful\0R/W Error\0Write Protected\0Not Ready\0Invalid Parameter\0";
	DRESULT i;

	for (i = 0; i != rc && *p; i++) {
		while (*p++);
	}
	printf("%s() =>%u %s\n", func, (UINT)rc, p);
}

char *basename(char *s)
{
	size_t i, j;
	if (!s || !*s) return ".";
	i = strlen(s) - 1;
	for (j = 0; j <= i; j++) if (s[j] == ':') { s = &s[j + 1]; i -= j; break; }
	for (; i&&s[i] == '/'; i--) s[i] = 0;
	for (; i&&s[i - 1] != '/'; i--);
	return s + i;
}

char *dirname(char *_s)
{
	char *s = _s;
	size_t i, j;
	if (!s || !*s) return ".";
	i = strlen(s) - 1;
	for (j = 0; j <= i; j++) if (s[j] == ':') { s = &s[j + 1]; i -= j; break; }
	for (; s[i] == '/'; i--) if (!i) return (s == _s) ? "/" : _s;
	for (; s[i] != '/'; i--) if (!i) return (s == _s) ? "." : _s;
	for (; s[i] == '/'; i--) if (!i) { if (s == _s) return "/"; else break; }
	s[i + 1] = 0;
	return _s;
}

int usrcmd_cd(int argc, char **argv)
{
	FRESULT res;

	if (argc == 2) {
		if ((res = f_chdir(argv[1])) != FR_OK) {
			put_rc("f_chdir", res);
			return 0;
		}
		if ((res = f_chdrive(argv[1]) != FR_OK)) {
			put_rc("f_chdrive", res);
			return 0;
		}
	}

	char path[256];
	if ((res = f_getcwd(path, sizeof(path))) != FR_OK) {
		put_rc("f_getcwd", res);
		return 0;
	}

	ntlibc_strlcat(path, "\n", sizeof(path));
	printf(path);

	return 0;
}

#define LS_ALL	0x01
#define LS_LONG	0x02
/* lsコマンド 1行表示 */
void print_one_list(FILINFO *fno, BYTE list_option)
{
	char *fn;

#if FF_USE_LFN
	fn = *fno->lfname ? fno->lfname : fno->fname;
#else
	fn = fno->fname;
#endif
	if (!(list_option & LS_ALL)) {
		if ((fno->fattrib & AM_HID) || (fn[0] == '.'))
			return;
	}

	if (list_option & LS_LONG) {
		printf("%c%c%c%c%c %04d/%02d/%02d %02d:%02d:%02d ",
			(fno->fattrib & AM_DIR) ? 'd' : '-',
			(fno->fattrib & AM_RDO) ? 'r' : '-',
			(fno->fattrib & AM_HID) ? 'h' : '-',
			(fno->fattrib & AM_SYS) ? 's' : '-',
			(fno->fattrib & AM_ARC) ? 'a' : '-',
			((fno->fdate & 0xFE00) >> 9) + 1980,
			((fno->fdate & 0x01E0) >> 5),
			( fno->fdate & 0x001F),
			((fno->ftime & 0xF800) >> 11),
			((fno->ftime & 0x07E0) >> 5),
			( fno->ftime & 0x001F));

		if (fno->fattrib & AM_DIR) {                    /* It is a directory */
			printf("%10s ", " ");
		}
		else {
			printf("%10lu ", fno->fsize);
		}
	}

	if (fno->fattrib & AM_DIR) {                    /* It is a directory */
		printf("\x1B[32m%s\x1B[0m\n", fn);
	}
	else {
		printf("%s\n", fn);
	}
}

#define LFN_BUF_SIZE (FF_MAX_LFN + 1)
/* lsコマンド dir内 表示 */
void print_ls(char *path_p, char *pattern_p, BYTE list_option)
{
	FRESULT res;
	FILINFO fno;
	FATFS_DIR dir;
	char *fn;   /* This function assumes non-Unicode configuration */

	if ((pattern_p != NULL) && (pattern_p[0] == '\0'))
		pattern_p = NULL;

	char *path_backup = NULL;
	path_backup = ff_memalloc(LFN_BUF_SIZE);
	if (path_backup == NULL) {
		printf("ff_memalloc err.\n");
		return;
	}

#if FF_USE_LFN
	char *lfn = NULL;
	lfn = ff_memalloc(LFN_BUF_SIZE);
	if (lfn == NULL) {
		printf("ff_memalloc err.\n");
		ff_memfree(path_backup);
		return;
	}
	fno.lfname = lfn;
	fno.lfsize = LFN_BUF_SIZE;
#endif

	if ((path_p != NULL) && (pattern_p == NULL)) {
		if ((res = f_opendir(&dir, path_p)) != FR_OK)
			put_rc("f_opendir", res);
		res = f_readdir(&dir, &fno);
	}
	else {
		res = f_findfirst(&dir, &fno, path_p, pattern_p);
	}

	while ((res == FR_OK) && (fno.fname[0] != 0)) {
		if (pattern_p != NULL && (fno.fattrib & AM_DIR) && ((fno.fname[0] == '.') ? (pattern_p[0] == '.') : 1)) {/* FATFS_DIR とパターンマッチしている場合は FATFS_DIR 内部を ls する */
#if FF_USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno->fname;
#endif
			if ((res = f_getcwd(path_backup, LFN_BUF_SIZE)) != FR_OK) {
				put_rc("f_getcwd", res);
			}

			if ((res = f_chdrive(path_p)) != FR_OK) {
				put_rc("f_chdrive", res);
			}

			if ((res = f_chdir(path_p)) != FR_OK) {
				put_rc("f_chdir", res);
			}

			printf("\n%s/%s:\n", path_p, fn);

			print_ls(fn, NULL, list_option);

			printf("\n");

			if ((res = f_chdrive(path_backup)) != FR_OK) {
				put_rc("f_chdrive", res);
			}

			if ((res = f_chdir(path_backup)) != FR_OK) {
				put_rc("f_chdir", res);
			}
		}
		else {
			print_one_list(&fno, list_option);
		}

		if (pattern_p == NULL)
			res = f_readdir(&dir, &fno);	/* all */
		else
			res = f_findnext(&dir, &fno);	/* pattern matching */
	}

	f_closedir(&dir);

	if (lfn			!= NULL) ff_memfree(lfn);
	if (path_backup != NULL) ff_memfree(path_backup);
}

int usrcmd_ls(int argc, char **argv)
{
	char *pattern_p = NULL, *basename_p = NULL, *dirname_p = NULL;
	char default_pattern[FF_MAX_LFN] = "";
	int c;
	BYTE list_option = 0;

	while ((c = getopt(argc, argv, "al")) != -1) {
		switch (c) {
		case 'a':
			list_option |= LS_ALL;
			break;
		case 'l':
			list_option |= LS_LONG;
			break;
		default:
			break;
		}
	}
	pattern_p = *(argv + optind);

	if (pattern_p != NULL)
		ntlibc_strlcpy(default_pattern, pattern_p, sizeof(default_pattern));
	basename_p = basename(pattern_p);
	dirname_p = dirname(default_pattern);
	if (((dirname_p[0] == '/') && (basename_p[0] == '/')) ||
		(!ntlibc_strncmp(dirname_p, ".", strlen(dirname_p)) && !ntlibc_strncmp(basename_p, ".", strlen(basename_p))))
	{
		basename_p = NULL;
	}
	print_ls(dirname_p, basename_p, list_option);

	return 0;
}

int usrcmd_cp(int argc, char **argv)
{
	char *src_str_p = NULL, *dst_str_p = NULL;
	unsigned char i;
	FRESULT res;
	FILINFO fno;
	FIL src_fp, dst_fp;
	size_t read_size, write_size;
	char *lfn = NULL, *local_buff = NULL, *dst_mod_str_p = NULL;
	char *src_basename_p;

	if (argc < 2)
		return 0;

	/* 引数チェック */
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			continue;
		if (argv[i][0] != '\0') {
			if (src_str_p == NULL)
				src_str_p = &argv[i][0];
			else {
				dst_str_p = &argv[i][0];
				break;
			}
		}
	}
	if ((src_str_p == NULL) || (dst_str_p == NULL))
		return 0;

#if FF_USE_LFN
	/* LFN buffer alloc */
	lfn = ff_memalloc(LFN_BUF_SIZE);
	if (lfn == NULL) {
		printf("alloc err.\n");
		goto cp_end;
	}
	fno.lfname = lfn;
	fno.lfsize = LFN_BUF_SIZE;
#endif

	/* copy buffer alloc */
	local_buff = ff_memalloc(64);
	if (local_buff == NULL) {
		printf("alloc err.\n");
		goto cp_end;
	}

	/*************/
	/* src check */
	/*************/
	res = f_stat(src_str_p, &fno);
	if (res != FR_OK) {
		if (res == FR_NO_FILE)
			printf("src no file.\n");
		else
			printf("src stat err(%d).\n", res);
		goto cp_end;
	}
	if (fno.fattrib & AM_DIR) {	/* src is dir */
		/*******************************************************/	/* from dir */ /* 未実装 */
	}
	else {						/* src is file */
		res = f_open(&src_fp, src_str_p, (FA_OPEN_EXISTING | FA_READ));
		if (res != FR_OK) {
			printf("src open err(%d).\n", res);
			goto cp_end;
		}
	}

	/*************/
	/* dst check */
	/*************/
	res = f_stat(dst_str_p, &fno);
	if (res != FR_NO_FILE) {
		if (res == FR_OK) {
			if (fno.fattrib & AM_DIR) {	/* dst is dir */
				src_basename_p = basename(src_str_p);
				dst_mod_str_p = ff_memalloc(LFN_BUF_SIZE);
				if (dst_mod_str_p == NULL) {
					printf("alloc err.\n");
					goto cp_end;
				}
				snprintf(dst_mod_str_p, LFN_BUF_SIZE, "%s/%s\0", dst_str_p, src_basename_p);
				dst_str_p = dst_mod_str_p;
			}
			else {
				printf("dst file exists.\n");
				goto cp_end_1;
			}
		}
		else {
			printf("src stat err(%d).\n", res);
			goto cp_end_1;
		}
	}
	res = f_open(&dst_fp, dst_str_p, (FA_CREATE_NEW | FA_WRITE));
	if (res != FR_OK) {
		printf("dst open err(%d).\n", res);
		goto cp_end_1;
	}

	/********/
	/* copy */
	/********/
	do {
		/* read from src */
		res = f_read(&src_fp, local_buff, sizeof(local_buff), &read_size);
		if (res != FR_OK) {
			printf("src read err(%d).\n", res);
			goto cp_end_2;
		}

		/* write to dst */
		res = f_write(&dst_fp, local_buff, read_size, &write_size);
		if (res != FR_OK) {
			printf("dst write err(%d).\n", res);
			goto cp_end_2;
		}
		if (read_size != write_size) {
			printf("dst write err(disk full).\n");
			goto cp_end_2;
		}
	} while (read_size == sizeof(local_buff));

cp_end_2:
	f_close(&dst_fp);
cp_end_1:
	f_close(&src_fp);
cp_end:
	if(dst_mod_str_p != NULL) ff_memfree(dst_mod_str_p);
	if(local_buff	 != NULL) ff_memfree(local_buff);
	if(lfn			 != NULL) ff_memfree(lfn);

	return 0;
}

int usrcmd_rm(int argc, char **argv)
{
	FRESULT res;

	if (argc == 2) {
		if ((res = f_unlink(argv[1])) != FR_OK) {
			put_rc("f_unlink", res);
			return 0;
		}
	}

	return 0;
}

int usrcmd_mv(int argc, char **argv)
{
	FRESULT res;

	if (argc == 3) {
		if ((res = f_rename(argv[1], argv[2])) != FR_OK) {
			put_rc("f_rename", res);
			return 0;
		}
	}

	return 0;
}

int usrcmd_mkdir(int argc, char **argv)
{
	FRESULT res;

	if (argc == 2) {
		if ((res = f_mkdir(argv[1])) != FR_OK) {
			put_rc("f_mkdir", res);
			return 0;
		}
	}

	return 0;
}

#define HEXDUMP_EXTRA_FOR_UTF8 3 /* need extra buffer size */
#define CCOLOR_RESET	0
#define CCOLOR_BLACK	30
#define CCOLOR_RED		31
#define CCOLOR_GREEN	32
#define CCOLOR_YELLOW	33
#define CCOLOR_BLUE		34
#define CCOLOR_MAGENTA	35
#define CCOLOR_CYAN		36
#define CCOLOR_WHITE	37
enum {
	HEXDUMP_OPT_DEFAULT = 1 << 0,
	HEXDUMP_OPT_UTF8 = 1 << 1,
};

int usrcmd_hexdump(int argc, char **argv)
{
	FRESULT res;
	FIL fsrc;
	char data[16 + 1 + HEXDUMP_EXTRA_FOR_UTF8];
	char ascii[sizeof("\x1B[31m0\x1B[0m\x1B[31m1\x1B[0m\x1B[31m2\x1B[0m\x1B[31m3\x1B[0m\x1B[31m4\x1B[0m\x1B[31m5\x1B[0m\x1B[31m6\x1B[0m\x1B[31m7\x1B[0m\x1B[31m8\x1B[0m\x1B[31m9\x1B[0m\x1B[31ma\x1B[0m\x1B[31mb\x1B[0m\x1B[31mc\x1B[0m\x1B[31md\x1B[0m\x1B[31me\x1B[0m\x1B[31mf\x1B[0m") + HEXDUMP_EXTRA_FOR_UTF8];
	char line[sizeof("00000000: 00 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00 00 : \n") + sizeof(ascii)];
	TCHAR *filename_p = NULL;
	int op, option_flag = HEXDUMP_OPT_DEFAULT;
	char *option_ptr, *option_endptr, ccolor, utf8_done_flg, utf8_odd_bytes = 0;
	unsigned int op_offset = 0, op_size = 0, op_end = 0;

	while ((op = getopt(argc, argv, "hduos0123456789xX")) != -1) {
		switch (op) {
		case 'h': /* help */
			printf(" hexdump [OPTION] file\n");
			printf("  -h : help\n");
			printf("  -d : print all byte with convert and color [in character area] (default)\n");
			printf("  -u : try print UTF-8 code [in character area]\n");
			printf("  -oOFFSET : print start offset address from top\n");
			printf("  -sSIZE   : print size\n");
			break;
		case 'd': /* print one byte character [in character area] (default) */
			option_flag |= HEXDUMP_OPT_DEFAULT;
			break;
		case 'u': /* try print UTF-8 code [in character area] */
			option_flag |= HEXDUMP_OPT_UTF8;
			break;
		case 'o': /* print start offset address from top */
			option_ptr = *(argv + optind);
			op_offset = ntlibc_strtoul(&option_ptr[2], &option_endptr, 0);
			break;
		case 's': /* print size */
			option_ptr = *(argv + optind);
			op_size = ntlibc_strtoul(&option_ptr[2], &option_endptr, 0);
			break;
		default:
			break;
		}
	}
	filename_p = *(argv + optind);
	if (filename_p == NULL)
		return 0;

	if ((res = f_open(&fsrc, filename_p, FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
		put_rc("f_open", res);
		return 0;
	}

	/* position adjusting */
	if (op_offset >= fsrc.fsize) {
		printf("error : input offset is bigger than file size(0x%lX).\n", fsrc.fsize);
		return 0;
	}
	op_end = op_offset + op_size;
	if ((op_size == 0) || (op_end >= fsrc.fsize))
		op_end = fsrc.fsize;
	f_lseek(&fsrc, op_offset);

	for (int i = op_offset; i < op_end; i += 16) {
		ascii[0] = '\0';
		line[0] = '\0';
		char *pos = line;
		int rst = sizeof(line);
		int len = snprintf(pos, rst, "%08X: ", i);
		pos += len;
		rst -= len;

		UINT br = 0;
		if ((res = f_read(&fsrc, data, 16 + HEXDUMP_EXTRA_FOR_UTF8, &br)) != FR_OK) {
			put_rc("f_read", res);
			f_close(&fsrc);
			return 0;
		}
		data[br] = '\0';
		f_lseek(&fsrc, i + 16);
		if (br > 16) br = 16;

		char *apos = ascii;
		int arst = sizeof(ascii);
		for (int j = 0; j < br; j++) {
			char c = data[j];
			if (j != 7)
				len = snprintf(pos, rst, "%02X ", c);
			else
				len = snprintf(pos, rst, "%02X-", c);
			pos += len;
			rst -= len;

			len = 0;
			utf8_done_flg = 0;
			ccolor = CCOLOR_RESET;

			if (c < 0x20) {
				ccolor = CCOLOR_RED;
				c = c + 0x40;
			}
			else {
				if (option_flag & HEXDUMP_OPT_UTF8) { /* try UTF-8 */
					/* check character code */
					int bf_utf8_bytes, af_utf8_bytes;
					unsigned char utf8_code[4];
					WCHAR utf16_code;

					utf16_code = Utf8_to_Utf16(&data[j], &bf_utf8_bytes);       /* try UTF-8 -> UTF-16 */
					Utf16_to_Utf8(utf8_code, &af_utf8_bytes, (UINT)utf16_code); /* try UTF-16 -> UTF-8 */
					if ((af_utf8_bytes <= 3 && bf_utf8_bytes == af_utf8_bytes) &&
						!memcmp(&data[j], utf8_code, af_utf8_bytes)) { /* size & code match */
						utf8_done_flg = 1;
						for (int k = 1; k < af_utf8_bytes; k++) { /* pos */
							if (j + k >= 16)
								break;
							if (j + k != 7)
								len = snprintf(pos, rst, "%02X ", data[j + k]);
							else
								len = snprintf(pos, rst, "%02X-", data[j + k]);
							pos += len;
							rst -= len;
						}
						memcpy(apos, &data[j], af_utf8_bytes); /* apos */
						apos[af_utf8_bytes] = 0;
						len = af_utf8_bytes;
						j += af_utf8_bytes - 1;
						if (af_utf8_bytes > 1) {
							utf8_odd_bytes = j < 15 ? af_utf8_bytes - 2 : af_utf8_bytes - 1;
						}
					}
				}
			}

			if (utf8_odd_bytes > 0 && j < 15) {
				apos += len;
				arst -= len;
				len = snprintf(apos, arst, "\x1B[%dm%c\x1B[0m", CCOLOR_RESET, ' ');
				utf8_odd_bytes--;
			}
			else if (utf8_done_flg == 0) {
				if (c >= 0x80 && c < 0x9F) {
					ccolor = CCOLOR_RED;
					c = c - 0x60;
				}
				else if (c >= 0xA0 && c < 0xDF) {
					ccolor = CCOLOR_GREEN;
					c = c - 0x60;
				}
				else if (c >= 0xE0 && c < 0xFF) {
					ccolor = CCOLOR_GREEN;
					c = c - 0xC0;
				}
				else if (c == 0x7F || c == 0x9F || c == 0xDF || c == 0xFF) {
					ccolor = CCOLOR_CYAN;
					c = '?';
				}
				len = snprintf(apos, arst, "\x1B[%dm%c\x1B[0m", ccolor, c);
			}

			apos += len;
			arst -= len;
		}

		for (int j = br; j < 16; j++) {
			if (j != 7)
				len = snprintf(pos, rst, "   ");
			else
				len = snprintf(pos, rst, "  -");
			pos += len;
			rst -= len;
		}

		len = snprintf(pos, rst, ": %s\n", ascii);
		pos += len;
		rst -= len;

		puts(line);
	}

	f_close(&fsrc);
	return 0;
}

int usrcmd_date(int argc, char **argv)
{
	int ret;
	struct timespec tp;
	char buf[30];

	ret = clock_gettime(CLOCK_REALTIME, &tp);
	if (ret != 0) {
		printf("clock_gettime error %d", ret);
		return 0;
	}

	memset(buf, 0, sizeof(buf));
	if (ctime_r(&tp.tv_sec, buf) == NULL) {
		printf("ctime_r error");
		return 0;
	}

	/* 改行コードの削除 */
	ret = ntlibc_strlen(buf);
	buf[ret - 1] = '\0';

	printf("%s .%09ld\n", buf, tp.tv_nsec);
	return 0;
}

int usrcmd_info(int argc, char **argv)
{
	if (argc != 2) {
		printf("info sys\n");
		printf("info ver\n");
		return 0;
	}
	if (strcmp(argv[1], "sys") == 0) {
		printf(TARGET_NAME" Monitor\n");
		return 0;
	}
	if (strcmp(argv[1], "ver") == 0) {
		int mj, mn, bd;
		ntshell_version(&mj, &mn, &bd);
		printf("Version %d.%d.%d\n", mj, mn, bd);
		return 0;
	}
	printf("Unknown sub command found\n");
	return -1;
}

int usrcmd_exit(int argc, char **argv)
{
	ntshell_exit = 1;

	return 0;
}
