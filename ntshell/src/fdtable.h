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
#ifndef SOCKET_STUB_H
#define SOCKET_STUB_H

typedef const struct io_type_s IO_TYPE;

struct SHELL_FILE {
	int fd;
	IO_TYPE *type;
	int handle;
	int readevt_r;
	int readevt_w;
	int writeevt_r;
	int writeevt_w;
	int writable;
	int errorevt_r;
	int errorevt_w;
	void *exinf;
};

struct io_type_s {
	int (*close)(struct SHELL_FILE *);
	size_t (*read)(struct SHELL_FILE *, unsigned char *, size_t);
	size_t (*write)(struct SHELL_FILE *, const unsigned char *, size_t);
	off_t (*seek)(struct SHELL_FILE *, off_t, int);
	int (*ioctl)(struct SHELL_FILE *, int, void *);
	bool_t (*readable)(struct SHELL_FILE *);
	bool_t (*writable)(struct SHELL_FILE *);
	void (*delete)(struct SHELL_FILE *);
};

#ifndef bool
#define bool int
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

struct SHELL_FILE *new_fp(IO_TYPE *type, int id, int writable);
int delete_fd_by_id(IO_TYPE *type, int id);
struct SHELL_FILE *fd_to_fp(int fd);
struct SHELL_FILE *id_to_fd(IO_TYPE *type, int id);

int delete_fp(struct SHELL_FILE *fp);
void clean_fd();

#endif // !SOCKET_STUB_H
