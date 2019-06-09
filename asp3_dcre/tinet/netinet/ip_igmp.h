/*
 *  TINET (TCP/IP Protocol Stack)
 *
 *  Copyright (C) 2001-2012 by Dep. of Computer Science and Engineering
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

#ifndef _IP_IGMP_H_
#define _IP_IGMP_H_

#define NUM_IGMP_ENTRY				5	/* マルチキャストグループのエントリ数 */

 /*
  *  IGMPv1 ヘッダの定義、RFC 1112
  */

typedef struct t_igmpv1_hdr
{
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		unused;		/* 未使用			*/
	uint16_t	sum;		/* チェックサム		*/
	T_IN4_ADDR	addr;		/* グループアドレス */
}  __attribute__((packed)) T_IGMPV1_HDR;

/*
*  IGMPv2 ヘッダの定義、RFC 2236
*/

typedef struct t_igmpv2_hdr
{
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		time;		/* 最大応答時間		*/
	uint16_t	sum;		/* チェックサム		*/
	T_IN4_ADDR	addr;		/* グループアドレス */
}  __attribute__((packed)) T_IGMPV2_HDR;

typedef struct t_igmpv3_query_hdr
{
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		time;		/* 最大応答時間		*/
	uint16_t	sum;		/* チェックサム		*/
	T_IN4_ADDR	grpaddr;	/* グループアドレス */
	uint8_t		sqrv;		/* Sフラグ、Querier Robustness Value */
	uint8_t		qqic;		/* Qurier's Query Interval Code */
	uint16_t	num;		/* 送信元の数 */
	T_IN4_ADDR	sdraddr;	/* 送信元アドレス */
}  __attribute__((packed)) T_IGMPV3Q_HDR;

typedef struct t_igmp_group_record
{
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		len;		/* 補助データ長		*/
	uint16_t	num;		/* 送信元の数 */
	T_IN4_ADDR	grpaddr;	/* グループアドレス */
	T_IN4_ADDR	sdraddr;	/* 送信元アドレス	*/
							/* 補助データ		*/
}  __attribute__((packed)) T_IGMP_GROUP_RECORD;

typedef struct t_igmpv3_report_hdr
{
	uint8_t		type;		/* メッセージの型	*/
	uint8_t		rsv1;		/* 予約				*/
	uint16_t	sum;		/* チェックサム		*/
	uint16_t	rsv2;		/* 予約				*/
	uint16_t	num;		/* グループレコードの数 */
	T_IGMP_GROUP_RECORD record;	/* グループレコード */
}  __attribute__((packed)) T_IGMPV3R_HDR;

#define IGMP_HDR_SIZE			(sizeof(T_IGMPV1_HDR))
#define IP4_IGMP_HDR_SIZE		(IP4_HDR_SIZE + IGMP_HDR_SIZE)
#define IF_IP4_IGMP_HDR_SIZE		(IF_IP4_HDR_SIZE + IGMP_HDR_SIZE)
#define IF_IP4_IGMP_HDR_OFFSET		(IF_IP4_HDR_SIZE)

#define GET_IGMPV1_HDR(nbuf,ihoff)	((T_IGMPV1_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_IGMPV2_HDR(nbuf,ihoff)	((T_IGMPV2_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_IGMPV3Q_HDR(nbuf,ihoff)	((T_IGMPV3Q_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_IGMPV3R_HDR(nbuf,ihoff)	((T_IGMPV3R_HDR*)((uint8_t*)((nbuf)->buf) + ihoff))
#define GET_IGMP_SDU(nbuf,ihoff)	((uint8_t*)((nbuf)->buf) + ihoff + IGMP_HDR_SIZE)

/*
*  メッセージの型
*/

#define IGMP_MEMBERSHIP_QUERY		0x11	/* メンバーシップクエリー		*/
#define IGMP_MEMBERSHIP_REPORT		0x12	/* メンバーシップレポート		*/
#define IGMP_MEMBERSHIP_REPORT_V2	0x16	/* メンバーシップレポート		*/
#define IGMP_LEAVE_GROUP			0x17	/* リーブグループ				*/
#define IGMP_MEMBERSHIP_REPORT_V3	0x22	/* メンバーシップレポート		*/

/*
*  UDP通信端点オプション
*/

typedef struct ip_mreq
{
	T_IN4_ADDR imr_multiaddr; /* IP multicast address of group */
	T_IN4_ADDR imr_interface; /* local IP address of interface */
} T_IP_MREQ;

typedef struct ip_msfilter
{
	T_IN4_ADDR imsf_multiaddr;     /* IP multicast group address */
	T_IN4_ADDR imsf_interface;     /* IP address of local interface */
	uint32_t   imsf_fmode;         /* Filter-mode */
	uint32_t   imsf_numsrc;        /* Number of sources in the following array */
	T_IN4_ADDR imsf_slist[1];      /* Array of source addresses */
} T_IP_MSFILTER;

#define MCAST_INCLUDE     1
#define MCAST_EXCLUDE     2

extern ER igmp_set_loop(T_UDP_CEP *cep, uint8_t optval);
extern ER igmp_get_loop(T_UDP_CEP *cep, uint8_t *optval);
extern ER igmp_set_ttl(T_UDP_CEP *cep, uint8_t optval);
extern ER igmp_get_ttl(T_UDP_CEP *cep, uint8_t *optval);
extern ER igmp_set_if(T_UDP_CEP *cep, T_IN4_ADDR *optval);
extern ER igmp_get_if(T_UDP_CEP *cep, T_IN4_ADDR *optval);
extern ER igmp_add_membership(T_UDP_CEP *cep, T_IP_MREQ *optval);
extern ER igmp_drop_membership(T_UDP_CEP *cep, T_IP_MREQ *optval);
extern ER igmp_set_ip_msfilter(T_UDP_CEP *cep, const T_IP_MSFILTER *optval);
extern ER igmp_get_ip_msfilter(T_UDP_CEP *cep, T_IP_MSFILTER *optval);

/*
 *  IGMP グループエントリ
 */

typedef struct igmp_entry
{
	int16_t		timer;		/* 送信までの時間[秒]	*/
	int16_t		resptime;	/* 最大応答時間[秒]	*/
	T_IN4_ADDR	ip_addr;	/* IP アドレス			*/
} T_IGMP_ENTRY;

extern void igmp_init(void);
extern bool_t igmp_is_joined_group(T_IN4_ADDR addr);
extern uint_t igmp_input(T_NET_BUF **inputp, uint_t *offp, uint_t *nextp);

#endif	/* of #ifndef _IP_IGMP_H_ */
