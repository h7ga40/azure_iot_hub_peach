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

/*
 *	シリアルインタフェースドライバのファイルディスクリプタ実装部
 */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "target_serial.h"
#include "siofd.h"
#include "serial_api.h"

extern serial_t log_serial;

__attribute__((weak))
int siofd_open(const char *path, int flags, ...)
{
	return -1;
}

__attribute__((weak))
int siofd_close(int fd)
{
	return -1;
}

__attribute__((weak))
ssize_t siofd_read(int fd, void *data, size_t len)
{
	char *pos, *end;

	if (fd != STDIN_FILENO)
		return -1;

	for (pos = (char *)data, *end = &pos[len]; pos < end; pos++) {
		*pos = serial_getc(&log_serial);
	}

	return pos - (char *)data;
}

__attribute__((weak))
ssize_t siofd_write(int fd, const void *data, size_t len)
{
	const char *pos, *end;

	if (fd != STDOUT_FILENO)
		return -1;

	for (pos = (const char *)data, end = &pos[len]; pos < end; pos++) {
		serial_putc(&log_serial, *pos);
	}

	return pos - (char *)data;
}

__attribute__((weak))
int siofd_tcsetattr(int fd, int act, const struct termios *tio)
{
	return 0;
}

__attribute__((weak))
int siofd_tcgetattr(int fd, struct termios *tio)
{
	return 0;
}

__attribute__((weak))
int siofd_fcntl(int fd, int cmd, ...)
{
	return 0;
}
