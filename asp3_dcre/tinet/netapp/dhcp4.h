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

#ifndef DHCP4_H
#define DHCP4_H

/*
 *  DHCP に関する定義
 *
 *    RFC2131
 */

/*
 *  DHCP メッセージの定数の定義
 */

#define DHCP4_CHADDR_LEN	16
#define DHCP4_SNAME_LEN		64
#define DHCP4_FILE_LEN		128
#define DHCP4_DFLT_OPT_LEN	312
#define DHCP4_UDP_MSG_LENGTH	(sizeof(uint32_t)*7+DHCP4_CHADDR_LEN	\
				 +DHCP4_SNAME_LEN+DHCP4_FILE_LEN+DHCP4_DFLT_OPT_LEN)
#define DHCP4_SNAME_OFFSET	(sizeof(uint32_t)*7+DHCP4_CHADDR_LEN)
#define DHCP4_FILE_OFFSET	(DHCP4_SNAME_OFFSET+DHCP4_SNAME_LEN)
#define DHCP4_OPTION_OFFSET	(DHCP4_FILE_OFFSET+DHCP4_FILE_LEN)
#define DHCP4_TIME_INFINITY	UINT_C(0xffffffff)

/*
 *  DHCP メッセージ
 */

typedef struct dhcp4_msg {
	uint8_t		op;			/* packet type			*/
	uint8_t		htype;			/* hardware address type	*/
	uint8_t		hlen;			/* hardware address length	*/
	uint8_t		hops;			/* gateway hops			*/
	uint32_t	xid;			/* transaction ID		*/
	uint16_t	secs;			/* seconds since boot began	*/
	uint16_t	flags;
	uint32_t	ciaddr;			/* client IP address		*/
	uint32_t	yiaddr;			/* your   IP address		*/
	uint32_t	siaddr;			/* server IP address		*/
	uint32_t	giaddr;			/* relay agent IP address	*/
	uint8_t		chaddr [DHCP4_CHADDR_LEN];	/* client hardware address	*/
	uint8_t		sname  [DHCP4_SNAME_LEN];	/* server host name		*/
	uint8_t		file   [DHCP4_FILE_LEN];		/* boot file name		*/
	uint8_t		options[DHCP4_DFLT_OPT_LEN];	/* optional parameters field	*/
	} __attribute__((packed)) T_DHCP4_MSG;

/* packet type の定義 */

#define DHCP4_REQUEST		UINT_C(1)
#define DHCP4_REPLY		UINT_C(2)

/* hardware address type の定義 */

#define DHCP4_HWA_TYPE_ETHER	UINT_C(1)
#define DHCP4_HWA_TYPE_IEEE802	UINT_C(6)

/*
 *  DHCP オプション
 *
 *    RFC1533
 */

/*
 *  オプションの穴埋め
 */
#define DHCP4_OPT_PAD		UINT_C(0)

/*
 *  サブネットマスク
 *
 *     code   len   subnet mask
 *   +------+-----+-----+-----+-----+-----+
 *   |   1  |  4  |  m1 |  m2 |  m3 |  m4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_SUBNET_MASK	UINT_C(1)

/*
 *  ルータ
 *
 *     code   len   addres #1               address #2
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 *   |   3  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 | ...
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_ROUTER		UINT_C(3)

/*
 *  DNS サーバ
 *
 *     code   len   addres #1               address #2
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 *   |   6  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 | ...
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_DNS_SERVER	UINT_C(6)

/*
 *  ホスト名
 *
 *     code   len   host name
 *   +------+-----+-----+-----+-----+-----+---
 *   |  12  |  n  |  h1 |  h2 |  h3 |  h4 | ...
 *   +------+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_HOST_NAME	UINT_C(12)

/*
 *  ドメイン名
 *
 *     code   len   domain name
 *   +------+-----+-----+-----+-----+-----+---
 *   |  15  |  n  |  d1 |  d2 |  d3 |  d4 | ...
 *   +------+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_DOMAIN_NAME	UINT_C(15)

/*
 *  ルータ探索
 *
 *     code   len  value
 *   +------+-----+-----+
 *   |  31  |  1  | 0/1 |
 *   +------+-----+-----+
 */
#define DHCP4_OPT_RTR_DISOVERY	UINT_C(31)

/*
 *  静的経路
 *
 *     code   len   destination #1          router #1
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 *   |  33  |  n  |  d1 |  d2 |  d3 |  d4 |  r1 |  r2 | ...
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_STATIC_ROUTE	UINT_C(33)

/*
 *  ベンダー固有情報
 *
 *     code   len   vendor-specific informations
 *   +------+-----+-----+-----+-----+---
 *   |  43  |  n  |  i1 |  i2 |  i3 | ...  
 *   +------+-----+-----+-----+-----+---
 *
 */
#define DHCP4_OPT_VENDOR_INFO	UINT_C(43)

/*
 *  NetBIOS over TCP/IP ネームサーバ
 *
 *     code   len   addres #1               address #2
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 *   |  44  |  n  |  a1 |  a2 |  a3 |  a4 |  a1 |  a2 | ...
 *   +------+-----+-----+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_NETBIOS_NS	UINT_C(44)

/*
 *  NetBIOS over TCP/IP ノードタイプ
 *
 *     code   len   type
 *   +------+-----+-----+
 *   |  46  |  1  | type|
 *   +------+-----+-----+
 */
#define DHCP4_OPT_NETBIOS_NODE	UINT_C(46)

/*
 *  NetBIOS over TCP/IP スコープ
 *
 *     code   len   NetBIOS scope
 *   +------+-----+-----+-----+-----+-----+---
 *   |  44  |  n  |  s1 |  s2 |  s3 |  s4 |  ...
 *   +------+-----+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_NETBIOS_SCOPE	UINT_C(47)

/*
 *  特定の IPv4 アドレス要求
 *
 *     code   len   IPv4 address
 *   +------+-----+-----+-----+-----+-----+
 *   |  50  |  4  |  a1 |  a2 |  a3 |  a4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_REQ_IPV4	UINT_C(50)

/*
 *  リース時間
 *
 *     code   len   lease time
 *   +------+-----+-----+-----+-----+-----+
 *   |  51  |  4  |  t1 |  t2 |  t3 |  t4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_LEASE_TIME	UINT_C(51)

/*
 *  オプションオーバーロード
 *
 *     code   len  value
 *   +------+-----+-----+
 *   |  52  |  1  | 1-3 |
 *   +------+-----+-----+
 */
#define DHCP4_OPT_OPT_OVERLOAD	UINT_C(52)

/* value の定義 */

#define DHCP4_OPTOL_FILE	UINT_C(0x01)
#define DHCP4_OPTOL_SNAME	UINT_C(0x02)
#define DHCP4_OPTOL_BOTH	UINT_C(0x03)

/*
 *  DHCP メッセージタイプ
 *
 *     code   len   type
 *   +------+-----+-----+
 *   |  53  |  1  | 1-7 |
 *   +------+-----+-----+
 */
#define DHCP4_OPT_MSG_TYPE	UINT_C(53)

/* type の定義 */

#define DHCP4_MTYPE_BOOTP	UINT_C(0)
#define DHCP4_MTYPE_DISCOVER	UINT_C(1)
#define DHCP4_MTYPE_OFFER	UINT_C(2)
#define DHCP4_MTYPE_REQUEST	UINT_C(3)
#define DHCP4_MTYPE_DECLINE	UINT_C(4)
#define DHCP4_MTYPE_ACK		UINT_C(5)
#define DHCP4_MTYPE_NAK		UINT_C(6)
#define DHCP4_MTYPE_RELEASE	UINT_C(7)
#define DHCP4_MTYPE_INFOMATION	UINT_C(8)

/*
 *  サーバ ID
 *
 *     code   len   IPv4 address
 *   +------+-----+-----+-----+-----+-----+
 *   |  54  |  4  |  a1 |  a2 |  a3 |  a4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_SERVER_ID	UINT_C(54)

/*
 *  要求パラメータリスト
 *
 *     code   len   option code
 *   +------+-----+-----+-----+-----+---
 *   |  55  |  n  |  c1 |  c2 |  c3 | ...  
 *   +------+-----+-----+-----+-----+---
 *
 */
#define DHCP4_OPT_REQ_PARAM	UINT_C(55)

/*
 *  Renewal (T1) 時間
 *
 *     code   len   T1 interval
 *   +------+-----+-----+-----+-----+-----+
 *   |  58  |  4  |  t1 |  t2 |  t3 |  t4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_RENEWAL_T1	UINT_C(58)

/*
 *  Rebinding (T2) 時間
 *
 *     code   len   T2 interval
 *   +------+-----+-----+-----+-----+-----+
 *   |  59  |  4  |  t1 |  t2 |  t3 |  t4 |
 *   +------+-----+-----+-----+-----+-----+
 */
#define DHCP4_OPT_REBINDING_T2	UINT_C(59)

/*
 *  ベンダークラス ID（RFC2132）
 *
 *     code   len   vendor class ID
 *   +------+-----+-----+-----+-----+---
 *   |  60  |  n  |  i1 |  i2 |  i3 | ...  
 *   +------+-----+-----+-----+-----+---
 *
 */
#define DHCP4_OPT_VCLASS_ID	UINT_C(60)

/*
 *  クライアント ID
 *
 *     code   len   type  client ID
 *   +------+-----+-----+-----+-----+---
 *   |  61  |  n  | type|  i1 |  i2 | ...  
 *   +------+-----+-----+-----+-----+---
 *
 *   type       : ハードウェアアドレスタイプ
 *   i1, i2, ...: 通常 MAC アドレス
 */
#define DHCP4_OPT_CLIENT_ID	UINT_C(61)

/*
 *  クラスなし静的経路（RFC3442）
 *
 *     code   len   destination #1     router #1
 *   +------+-----+-----+------+-----+-----+-----+-----+---
 *   |  121 |  n  |  d1 |  ... |  dN |  r1 |  r2 |  r3 | ...
 *   +------+-----+-----+------+-----+-----+-----+-----+---
 */
#define DHCP4_OPT_CLASSLESS_RTE	UINT_C(121)

/*
 *  オプションの終了
 */
#define DHCP4_OPT_END		UINT_C(255)

/*
 *  magic 番号（RFC1048）
 */

#define DHCP4_RFC1048_MAGIC	{ UINT_C(99), UINT_C(130), UINT_C(83), UINT_C(99), }
#define DHCP4_MAGIC_LEN		4

/*
 *  UDP ポート番号
 */

#ifndef DHCP4_SRV_CFG_PORTNO
#define DHCP4_SRV_CFG_PORTNO	67	/* server	*/
#endif

#ifndef DHCP4_CLI_CFG_PORTNO
#define DHCP4_CLI_CFG_PORTNO	68	/* client	*/
#endif

#endif	/* of #ifndef DHCP4_H */
