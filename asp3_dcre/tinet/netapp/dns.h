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
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
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

#ifndef DNS_H
#define DNS_H

/*
 *  DNS メッセージに関する定義
 *
 *    RFC1034、RFC1035、RFC1886、RFC2136、
 *    RFC2671、RFC2845、RFC2930、RFC3596
 *
 *  DNS メッセージ
 *
 *  +---------------------+
 *  |       Header        |
 *  +---------------------+
 *  |      Question       | Question section 形式
 *  +---------------------+
 *  |       Answer        | RR (Resouce Record) 形式
 *  +---------------------+
 *  |     Authority       | RR (Resouce Record) 形式
 *  +---------------------+
 *  |     Additional      | RR (Resouce Record) 形式
 *  +---------------------+
 *
 *  DNS ヘッダ
 *
 *    0                    7  8                   15
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                      ID                       |
 *  +--+-----------+--+--+--+--+--------+-----------+
 *  |QR|   Opcode  |AA|TC|RD|RA|    Z   |   RCODE   |
 *  +--+--------+--+--+--+--+--+--------+-----------+
 *  |                    QDCOUNT                    |
 *  +-----------------------------------------------+
 *  |                    ANCOUNT                    |
 *  +-----------------------------------------------+
 *  |                    NSCOUNT                    |
 *  +-----------------------------------------------+
 *  |                    ARCOUNT                    |
 *  +-----------------------------------------------+
 */

typedef struct dns_hdr {
	uint16_t	id;		/* ID */
	uint16_t	code;	/* QR:		1
				   Opcode:	4
				   AA:		1
				   TC:		1
				   RD:		1
				   RA:		1
				   Z:		3
				   RCODE:	4
				   */
	uint16_t	qdcount;	/* question   section */
	uint16_t	ancount;	/* answer     section */
	uint16_t	nscount;	/* authority  section */
	uint16_t	arcount;	/* additional section */
	} T_DNS_HDR;

/*
 *  code の定義
 */

/* QR の定義 */

#define DNS_QR_QUERY		0x0000U
#define DNS_QR_RESPONSE		0x8000U

/* Opcode の定義 */

#define DNS_OPCODE_MASK		0x7800U
#define DNS_OPCODE_FORWARD	0x0000U
#define DNS_OPCODE_REVERSE	0x0800U
#define DNS_OPCODE_STATUS	0x1000U

/* AA の定義 */

#define DNS_AUTHORITATIVE	0x0400U

/* TC の定義 */

#define DNS_TRUN_CATION		0x0200U

/* RD の定義 */

#define DNS_RECURSION_DESIRED	0x0100U

/* RA の定義 */

#define DNS_RECURSION_AVAILABLE	0x0080U

/* RCODE の定義 */

#define DNS_RCODE_MASK		0x000fU
#define DNS_RCODE_NO_ERROR	0x0000U
#define DNS_RCODE_FORMAT_ERROR	0x0001U
#define DNS_RCODE_SERVER_ERROR	0x0002U
#define DNS_RCODE_NAME_ERROR	0x0003U
#define DNS_RCODE_NOT_IMPLEMENT	0x0004U
#define DNS_RCODE_REFUSED	0x0005U

/*
 * DNS TYPE の定義
 */

#define DNS_TYPE_A		(uint16_t)1U
#define DNS_TYPE_NS		(uint16_t)2U
#define DNS_TYPE_CNAME		(uint16_t)5U
#define DNS_TYPE_SOA		(uint16_t)6U
#define DNS_TYPE_PTR		(uint16_t)12U
#define DNS_TYPE_AAAA		(uint16_t)28U

/*
 * DNS CLASS の定義
 */

#define DNS_CLASS_IN		(uint16_t)1U

/*
 *  RR (Resouce Record) 形式
 *
 *    0                    7  8                   15
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                       .                       |
 *  +-----------------      .      -----------------+
 *  |                      NAME                     |
 *  +-----------------      .      -----------------+
 *  |                       .                       |
 *  +-----------------------------------------------+
 *  |                      TYPE                     |
 *  +-----------------------------------------------+
 *  |                      CLASS                    |
 *  +-----------------------------------------------+
 *  |                       TTL                     |
 *  |                                               |
 *  +-----------------------------------------------+
 *  |                    RDLENGTH                   |
 *  +-----------------------------------------------+
 *  |                       .                       |
 *  +-----------------      .      -----------------+
 *  |                     RDATA                     |
 *  +-----------------      .      -----------------+
 *  |                       .                       |
 *  +-----------------------------------------------+
 *
 *    注意: offset の原点は DNS message の先頭
 */

typedef struct dns_rr {
					/* NAME			*/
	uint16_t	type;		/* TYPE			*/
	uint16_t	class;		/* CLASS		*/
	uint32_t	ttl;		/* TTL			*/
	uint16_t	rdlength;	/* RDATA length		*/
					/* RDATA		*/
	} T_DNS_RR;

#define T_DNS_RR_SIZE	(sizeof(uint16_t)*3+sizeof(uint32_t))

/*
 *  Question section 形式
 *
 *    0                    7  8                   15
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                       .                       |
 *  +-----------------      .      -----------------+
 *  |                     QNAME                     |
 *  +-----------------      .      -----------------+
 *  |                       .                       |
 *  +-----------------------------------------------+
 *  |                     QTYPE                     |
 *  +-----------------------------------------------+
 *  |                     QCLASS                    |
 *  +-----------------------------------------------+
 */

typedef struct dns_qd {
					/* QNAME		*/
	uint16_t	type;		/* TYPE			*/
	uint16_t	class;		/* CLASS		*/
	} T_DNS_QD;

#define T_DNS_QD_SIZE	(sizeof(uint16_t)*2)

/*
 *  SOA RDATA 形式
 *
 *    0                    7  8                   15
 *  +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *  |                       .                       |
 *  +-----------------      .      -----------------+
 *  |                     MNAME                     |
 *  +-----------------      .      -----------------+
 *  |                       .                       |
 *  +-----------------------------------------------+
 *  |                       .                       |
 *  +-----------------      .      -----------------+
 *  |                     RNAME                     |
 *  +-----------------      .      -----------------+
 *  |                       .                       |
 *  +-----------------------------------------------+
 *  |                    SERIAL                     |
 *  |                                               |
 *  +-----------------------------------------------+
 *  |                    REFRESH                    |
 *  |                                               |
 *  +-----------------------------------------------+
 *  |                     RETRY                     |
 *  |                                               |
 *  +-----------------------------------------------+
 *  |                    EXPIRE                     |
 *  |                                               |
 *  +-----------------------------------------------+
 *  |                    MINIMUM                    |
 *  |                                               |
 *  +-----------------------------------------------+
 *
 *    注意: offset の原点は DNS message の先頭
 */

typedef struct dns_soa {
					/* MNAME		*/
					/* RNAME		*/
	uint32_t	serial;		/* SERIAL		*/
	uint32_t	refresh;	/* REFRESH		*/
	uint32_t	retry;		/* RETRY		*/
	uint32_t	expire;		/* EXPIRE		*/
	uint32_t	minimum;	/* MINIMUM		*/
	} T_DNS_SOA;

#define T_DNS_SOA_SIZE	(sizeof(uint32_t)*5)

/*
 *  DNS の定数の定義
 */

#define DNS_LABEL_LENGTH	63
#define DNS_NAME_LENGTH		255
#define DNS_UDP_MSG_LENGTH	512

#define DNS_MSG_COMP_MARK	0xc0

#endif	/* of #ifndef DNS_H */
