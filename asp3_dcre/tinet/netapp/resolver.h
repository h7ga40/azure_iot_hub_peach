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

/*
 *  ホスト名←→IP アドレス変換
 */

#ifndef RESOLVER_H
#define RESOLVER_H

#include <netapp/dns.h>

/*
 *  コンパイル時コンフィギュレーションの指定
 */

/* ポート番号 */

#define RSLV_CFG_DNS_PORTNO	53

/*
 *  タイムアウト値
 */

#define TMO_DNS_UDP_SND		ULONG_C(1*1000*1000)	/* [us]、DNS サーバへの   UDP 送信タイムアウト	*/
#define TMO_DNS_UDP_RCV		ULONG_C(5*1000*1000)	/* [us]、DNS サーバからの UDP 受信タイムアウト	*/
#define NUM_DNS_UDP_RETRY	1		/* リトライ回数					*/

/*
 *  ドメイン名
 */


#if !defined(RSLV_CFG_DNS_DOMAIN_NAME_STR)
#define RSLV_CFG_DNS_DOMAIN_NAME_STR	""
#endif

/*
 *  DNS サーバーの IPアドレス
 */

#if !defined(IPV6_ADDR_DNS_INIT)
#define IPV6_ADDR_DNS_INIT	IPV6_ADDR_UNSPECIFIED_INIT
#endif

#if !defined(IPV4_ADDR_DNS)
#define	IPV4_ADDR_DNS		IPV4_ADDRANY
#endif

/*
 *  送受信バッファ数
 */

#define NUM_MPF_RSLV_SRBUF	1

/*
 *  プログラムで使用する情報の定義
 */

/*
 *  プログラムで使用する DNS ヘッダ
 *
 *    注意: メンバはホストバイトオーダー
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

typedef struct rslv_dns_hdr {
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
	} __attribute__((packed))T_RSLV_DNS_HDR;

/*
 *  DNS message data
 *
 *    注意: メンバはホストバイトオーダー
 */

typedef struct rslv_dns_msg {
	T_RSLV_DNS_HDR	dns_hdr;	/* DNS heder                            */
	uint_t		qd_offset;	/* offset of question           section */
	uint_t		an_offset;	/* offset of answer             section */
	uint_t		ns_offset;	/* offset of authority  records section */
	uint_t		ar_offset;	/* offset of additional records section */
	} __attribute__((packed))T_RSLV_DNS_MSG;

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
 *          メンバはホストバイトオーダー
 */

typedef struct rslv_rr {
	uint16_t	type;		/* TYPE			*/
	uint16_t	class;		/* CLASS		*/
	uint32_t	ttl;		/* TTL			*/
	uint16_t	rdlength;	/* RDATA length		*/
	uint_t		rdata_offset;	/* offset of RDATA	*/
	} __attribute__((packed))T_RSLV_RR;

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
 *
 *    注意: メンバはホストバイトオーダー
 */

typedef struct rslv_qd {
	uint16_t	type;		/* TYPE			*/
	uint16_t	class;		/* CLASS		*/
	} __attribute__((packed))T_RSLV_QD;

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
 *          メンバはホストバイトオーダー
 */

typedef struct rslv_soa {
	uint32_t	serial;		/* SERIAL		*/
	uint32_t	refresh;	/* REFRESH		*/
	uint32_t	retry;		/* RETRY		*/
	uint32_t	expire;		/* EXPIRE		*/
	uint32_t	minimum;	/* MINIMUM		*/
	} __attribute__((packed))T_RSLV_SOA;

/*
 *  dns_lookup_host の flags の定義
 *
 *    注意:
 *      dns.h で定義されている DNS_OPCODE と重ならないこと。
 *      #define DNS_OPCODE_MASK		0x7800U
 */

#define DNS_LUP_FLAGS_NAME_IPV4		0x0004U
#define DNS_LUP_FLAGS_NAME_IPV6		0x0006U
#define DNS_LUP_FLAGS_NAME_HOST		0x0010U
#define DNS_LUP_FLAGS_NAME_FQDN		0x0011U
#define DNS_LUP_FLAGS_NAME_MASK		0x001fU

#define DNS_LUP_OPCODE_FORWARD		DNS_OPCODE_FORWARD
#define DNS_LUP_OPCODE_REVERSE		DNS_OPCODE_REVERSE
#define DNS_LUP_OPCODE_STATUS		DNS_OPCODE_STATUS
#define DNS_LUP_OPCODE_MASK		DNS_OPCODE_MASK

#define DNS_LUP_FLAGS_PROTO_IPV4	0x0040U
#define DNS_LUP_FLAGS_PROTO_IPV6	0x0080U
#define DNS_LUP_FLAGS_PROTO_MASK	0x00c0U

#define DNS_LUP_FLAGS_QTYPE_A		0x0100U
#define DNS_LUP_FLAGS_QTYPE_AAAA	0x0200U
#define DNS_LUP_FLAGS_QTYPE_PTR		0x0400U
#define DNS_LUP_FLAGS_QTYPE_MASK	0x0700U

#define DNS_LUP_FLAGS_MSG		0x0020U

/*
 *  ネットワークエラーコード
 */

#define EV_RSLV_ESIZE			(-100)		/* DNSメッセージの名前長異常	*/
#define EV_RSLV_UNEXPCT			(-101)		/* 予期しないDNSメッセージ	*/

/*
 *  関数
 */

extern ER_UINT	dns_lookup_host (uint16_t flags, char *hostname, uint8_t *msg, uint_t msg_len, T_RSLV_DNS_MSG *rslv);
extern ER	dns_host_addr (uint16_t flags, char *hostname, T_IN_ADDR *addr);
extern ER_UINT	dns_analyze_qd (T_RSLV_QD *qd, uint_t offset, uint8_t *msg, uint_t length);
extern ER_UINT	dns_analyze_rr (T_RSLV_RR *rr, uint_t offset, uint8_t *msg, uint_t length);
extern ER_UINT	dns_analyze_soa (T_RSLV_SOA *soa, uint_t offset, uint8_t *msg, uint_t length);

/*
 *  注意: dns_in6_set_dname、dns_in4_set_dname、
 *        dns_in6_set_addr、 dns_in4_set_addr で、
 *        new に NULL を指定すると、初期値に戻す。
 */

extern const uint8_t *dns_in6_set_dname (const uint8_t *new, uint_t len);
extern const uint8_t *dns_in6_get_dname (void);
extern const uint8_t *dns_in4_set_dname (const uint8_t *new, uint_t len);
extern const uint8_t *dns_in4_get_dname (void);
extern T_IN6_ADDR *dns_in6_set_addr (T_IN6_ADDR *new);
extern T_IN6_ADDR *dns_in6_get_addr (T_IN6_ADDR *dst);
extern T_IN4_ADDR *dns_in4_set_addr (T_IN4_ADDR *new);
extern T_IN4_ADDR *dns_in4_get_addr (T_IN4_ADDR *dst);

#endif	/* of #ifndef RESOLVER_H */
