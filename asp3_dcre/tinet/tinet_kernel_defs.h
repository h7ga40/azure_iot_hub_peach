/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2017 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *
 *  上記著作権者は，以下の (1)～(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次の条件を満たすこと．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id$
 */

/*
 *  カーネルに関する互換性定義ファイル
 */

#ifndef _TINET_KERNEL_DEFS_H_
#define _TINET_KERNEL_DEFS_H_

#if defined(TARGET_KERNEL_JSP)

#ifndef _MACRO_ONLY

/*
 *  データ型の定義（inttype.h）
 */

#ifdef _int8_
typedef _int8_			int8_t;		/* 符号付き 8 ビット整数 */
typedef unsigned _int8_		uint8_t;	/* 符号無し 8 ビット整数 */
#endif	/* of #ifdef _int8_ */

#ifdef _int16_
typedef _int16_			int16_t;	/* 符号付き 16 ビット整数 */
typedef unsigned _int16_	uint16_t;	/* 符号無し 16 ビット整数 */
#endif	/* of #ifdef _int16_ */

#ifdef _int32_
typedef _int32_			int32_t;	/* 符号付き 32 ビット整数 */
typedef unsigned _int32_	uint32_t;	/* 符号無し 32 ビット整数 */
#endif	/* of #ifdef _int32_ */

#ifdef _int64_
typedef unsigned _int64_	uint64_t;	/* 符号無し 64 ビット整数 */
#endif	/* of #ifdef _int64_ */

typedef signed int		int_t;		/* 自然なサイズの符号付き整数 */
typedef unsigned int		uint_t;		/* 自然なサイズの符号無し整数 */

typedef signed long		long_t;		/* 長めのサイズの符号付き整数 */
typedef unsigned long		ulong_t;	/* 長めのサイズの符号無し整数 */

#ifdef _vp_int_
typedef _vp_int_		intptr_t;	/* ポインタまたは符号付き整数 */
#else	/* of #ifdef _vp_int_ */
typedef VP			intptr_t;	/* ポインタまたは符号付き整数 */
#endif	/* of #ifdef _vp_int_ */

typedef int			bool_t;		/* 真偽値 */

#ifndef true
#define true			1		/* 真 */
#endif /* true */

#ifndef false
#define false			0		/* 偽 */
#endif /* false */

#endif	/* of #ifndef _MACRO_ONLY */

/* 構造体 structure 中のフィールド field のバイト位置を返すマクロ */
#ifndef offsetof
#define offsetof(structure, field) ((size_t) &(((structure *) 0)->field))
#endif	/* of #ifndef offsetof */

/*
 *  整数定数を作るマクロ
 */
#ifndef UINT_C
#define UINT_C(val)		(val ## U)	/* uint_t型の定数を作るマクロ */
#endif /* UINT_C */

#ifndef ULONG_C
#define ULONG_C(val)		(val ## UL)	/* ulong_t型の定数を作るマクロ */
#endif /* ULONG_C */

#elif defined(TARGET_KERNEL_ASP)

#define syscall(api)		(api)

#endif	/* of #if defined(TARGET_KERNEL_JSP) */

#endif	/* of #ifndef _TINET_KERNEL_DEFS_H_ */
