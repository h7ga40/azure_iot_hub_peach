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

#ifndef _NET_BUF_H_
#define _NET_BUF_H_

/*
 *  ネットワークバッファ
 *
 *    4 オクテットでアラインする必要のあるプロセッサを考慮して、
 *    IF ヘッダサイズが 4 オクテット境界でないの場合、
 *    IP ヘッダ以降を 4 オクテット境界に調整する。
 */

#ifndef CPU_NET_ALIGN
#error "CPU_NET_ALIGN expected."
#endif

/*
 *  テンプレート
 */

struct t_net_buf {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[IF_MIN_LEN];	/* バッファ本体 */
	};

#ifndef T_NET_BUF_DEFINED

typedef struct t_net_buf T_NET_BUF;

#define T_NET_BUF_DEFINED

#endif	/* of #ifndef T_NET_BUF_DEFINED */

/*
 *  64 オクテット
 */

typedef struct t_net_buf_64 {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[64]; /* バッファ本体	*/
	} T_NET_BUF_64;

/*
 *  128 オクテット
 */

typedef struct t_net_buf_128 {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[128]; /* バッファ本体	*/
	} T_NET_BUF_128;

/*
 *  256 オクテット
 */

typedef struct t_net_buf_256 {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[256]; /* バッファ本体	*/
	} T_NET_BUF_256;

/*
 *  512 オクテット
 */

typedef struct t_net_buf_512 {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[512];/* バッファ本体	*/
	} T_NET_BUF_512;

/*
 *  1024 オクテット
 */

typedef struct t_net_buf_1024 {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[1024]; /* バッファ本体	*/
	} T_NET_BUF_1024;

/*
 *  ネットワークインタフェースの最大 PDU サイズ
 */

#if defined(IF_PDU_SIZE)

typedef struct t_net_buf_if_pdu {
	uint16_t	len;	/* データの長さ		*/
	uint8_t		idix;	/* mpfid のインデックス	*/
	uint8_t		flags;	/* フラグ		*/
#ifdef IF_ETHER_MULTI_NIC
	uint8_t		conn_pos;
#endif
#ifdef IF_ETHER_NIC_NET_BUF_ALIGN
	uint8_t		nalign[IF_ETHER_NIC_NET_BUF_ALIGN];
#endif
#if CPU_NET_ALIGN == 4 && IF_HDR_ALIGN != 4
	uint8_t		halign[4 - IF_HDR_ALIGN];
#endif
	uint8_t		buf[IF_PDU_SIZE];
				/* バッファ本体		*/
#if defined(IF_PDU_HDR_PADDING)
	uint8_t		padding[IF_PDU_HDR_PADDING];
#endif				/* ヘッダの余分		*/
	} T_NET_BUF_IF_PDU;

#endif	/* of #if defined(IF_PDU_SIZE) */

/*
 *  ネットワークバッファ表
 */

typedef struct t_net_buf_entry {

	uint16_t	index;
	uint_t		size;

#if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF

	ulong_t		prepares;
	ulong_t		busies;
	ulong_t		requests;
	ulong_t		allocs;
	ulong_t		errors;

#endif	/* of #if NET_COUNT_ENABLE & PROTO_FLG_NET_BUF */

	} T_NET_BUF_ENTRY;

/*
 *  ネットワークバッファ・フラグ	
 */

/*
 *  ネットワークインタフェース出力後にネットワークバッファを開放しない。
 *  注意: 出力後にこのフラグはクリアされる。
 */
#define NB_FLG_NOREL_IFOUT	UINT_C(0x80)

/*
 *  ESP 用フラグ
 */
#define NB_FLG_DECRYPED		UINT_C(0x40)

/*
 *  ネットワークバッファ獲得属性
 */

/* 探索属性 */

#define NBA_SEARCH_ASCENT	UINT_C(0x1000)	/* 大きなサイズの方向に探索する。	*/
#define NBA_SEARCH_DESCENT	UINT_C(0x2000)	/* 小さなサイズの方向に探索する。	*/
#define NBA_RESERVE_TCP		UINT_C(0x4000)	/* TCP 用にネットワークバッファを予約する。*/
#define NBA_ID_MASK		UINT_C(0x0fff)	/* 通信端点 ID を渡す場合のマスク値	*/

/*
 *  関数シミュレーションマクロ
 */

#define get_net_buf(b,l)	tget_net_buf((b),(l),TMO_FEVR)

/*
 *  関数
 */

extern ER tget_net_buf_ex (T_NET_BUF **blk, uint_t minlen, uint_t maxlen, ATR nbatr, TMO tmout);
extern ER tget_net_buf (T_NET_BUF **blk, uint_t len, TMO tmout);
extern ER rel_net_buf (T_NET_BUF *blk);
extern ER rus_net_buf (T_NET_BUF *blk);
extern ER_UINT net_buf_siz (T_NET_BUF *blk);
extern uint_t net_buf_max_siz (void);
extern const T_NET_BUF_ENTRY *nbuf_get_tbl (void);
extern uint_t nbuf_get_tbl_size (void);

#endif	/* of #ifndef _NET_BUF_H_ */
