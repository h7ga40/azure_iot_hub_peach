/**
 * @file usrcmd.h
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

#ifndef USRCMD_H
#define USRCMD_H

typedef int(*USRCMDFUNC)(int argc, char **argv);

typedef struct
{
	const char *cmd;
	const char *desc;
	USRCMDFUNC func;
} cmd_table_t;

int usrcmd_cd(int argc, char **argv);
int usrcmd_ls(int argc, char **argv);
int usrcmd_cp(int argc, char **argv);
int usrcmd_rm(int argc, char **argv);
int usrcmd_mv(int argc, char **argv);
int usrcmd_mkdir(int argc, char **argv);
int usrcmd_hexdump(int argc, char **argv);
int usrcmd_date(int argc, char **argv);

int usrcmd_help(int argc, char **argv);
int usrcmd_info(int argc, char **argv);
int usrcmd_exit(int argc, char **argv);

#endif

