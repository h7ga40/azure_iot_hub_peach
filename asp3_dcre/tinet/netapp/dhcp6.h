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

#ifndef DHCP6_H
#define DHCP6_H

/*
 *  DHCPv6 に関する定義
 *
 *    RFC3315
 */

/*
 *  DHCPv6 メッセージの定数の定義
 */

#define DHCP6_XID_LEN		3
#define DHCP6_OPTION_LEN	(512-(1+DHCP6_XID_LEN))

/*
 *  DHCPv6 メッセージ
 */

typedef struct dhcp6_msg_hdr {
	uint8_t	type;				/* message type		*/
	uint8_t	xid[DHCP6_XID_LEN];		/* transaction ID	*/
	} T_DHCP6_MSG_HDR;

typedef struct dhcp6_msg {
	T_DHCP6_MSG_HDR	hdr;
	uint8_t		options[DHCP6_OPTION_LEN];	/* options	*/
	} T_DHCP6_MSG;

/* message type の定義 */

#define DHCP6_SOLICIT		UINT_C(1)
#define DHCP6_ADVERTISE		UINT_C(2)
#define DHCP6_REQUEST		UINT_C(3)
#define DHCP6_CONFIRM		UINT_C(4)
#define DHCP6_RENEW		UINT_C(5)
#define DHCP6_REBIND		UINT_C(6)
#define DHCP6_REPLY		UINT_C(7)
#define DHCP6_RELEASE		UINT_C(8)
#define DHCP6_DECLINE		UINT_C(9)
#define DHCP6_RECONFIGURE	UINT_C(10)
#define DHCP6_INFO_REQ		UINT_C(11)
#define DHCP6_RELAY_FORW	UINT_C(12)
#define DHCP6_RELAY_REPL	UINT_C(13)

/*
 *  DHCPv6 オプション
 */

#define DHCP6_OPT_CLIENTID	UINT_C(1)
#define DHCP6_OPT_SERVERID	UINT_C(2)
#define DHCP6_OPT_IA_NA		UINT_C(3)
#define DHCP6_OPT_IA_TA		UINT_C(4)
#define DHCP6_OPT_IAADDR	UINT_C(5)
#define DHCP6_OPT_ORO		UINT_C(6)
#define DHCP6_OPT_PREFERENCE	UINT_C(7)
#define DHCP6_OPT_UNICAST	UINT_C(12)
#define DHCP6_OPT_STATUS_CODE	UINT_C(13)
#define DHCP6_OPT_RAPID_COMMIT	UINT_C(14)
#define DHCP6_OPT_NAME_SERVERS	UINT_C(23)
#define DHCP6_OPT_DOMAIN_SEARCH	UINT_C(24)
#define DHCP6_OPT_IA_PD		UINT_C(25)

/* hardware address type の定義 */

#define DHCP6_HWA_TYPE_ETHER	UINT_C(1)
#define DHCP6_HWA_TYPE_IEEE802	UINT_C(6)

/* STATUS CODE の定義 */

#define DHCP6_STAT_SUCCESS	UINT_C(0)
#define DHCP6_STAT_UNSPECIFID	UINT_C(1)
#define DHCP6_STAT_NO_AVAIL	UINT_C(2)
#define DHCP6_STAT_NO_BINDING	UINT_C(3)
#define DHCP6_STAT_NO_ONLINK	UINT_C(4)
#define DHCP6_STAT_USE_MCAST	UINT_C(5)

/* 時間定数[s]の定義 */

#define DHCP6_MAX_TIME		UINT_C(0xffffffff)

/*
 *  オプションのヘッダ部の定義
 */

/* オプションの共通部 */

typedef struct dhcp6_option {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	} T_DHCP6_OPTION;

/* オプションの共通部 */

/* CLIENTID オプション */

typedef struct dhcp6_opt_clientid {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	} T_DHCP6_OPT_CLIENTID;

/* SERVERID オプション */

typedef struct dhcp6_opt_serverid {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	} T_DHCP6_OPT_SERVERID;

/* IA_NA/IA_PD オプション */

typedef struct dhcp6_opt_ia_na_pd {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	uint32_t	iaid;		/* IA ID		*/
	uint32_t	renew;		/* renew  timeout (T1)	*/
	uint32_t	rebind;		/* rebind timeout (T2)	*/
	} T_DHCP6_OPT_IA_NA_PD;

/* IA_TA オプション */

typedef struct dhcp6_opt_ia_ta {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	uint32_t	iaid;		/* IA ID		*/
	} T_DHCP6_OPT_IA_TA;

/* IAADDR オプション */

#ifdef T_IN6_ADDR_DEFINED

typedef struct dhcp6_opt_iaaddr {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	T_IN6_ADDR	addr;		/* IPv6 address		*/
	uint32_t	prefer;		/* preferred lifetime	*/
	uint32_t	valid;		/* valid     lifetime	*/
	} T_DHCP6_OPT_IAADDR;

#endif	/* of #ifdef T_IN6_ADDR_DEFINED */

/* ORO オプション */

typedef struct dhcp6_opt_oro {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	} T_DHCP6_OPT_ORO;

/* PREFERENCE オプション */

typedef struct dhcp6_opt_preference {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	uint8_t		prefer;		/* preference value	*/
	} T_DHCP6_OPT_PREFERENCE;

/* UNICAST オプション */

#ifdef T_IN6_ADDR_DEFINED

typedef struct dhcp6_opt_unicast {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	T_IN6_ADDR	addr;		/* IPv6 address for server*/
	} T_DHCP6_OPT_UNICAST;

#endif	/* of #ifdef T_IN6_ADDR_DEFINED */

/* STATUS CODE オプション */

typedef struct dhcp6_opt_status_code {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	uint16_t	status;		/* status code		*/
	} T_DHCP6_OPT_STATUS_CODE;

/* NAME SERVERS オプション */

typedef struct dhcp6_opt_name_servers {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	T_IN6_ADDR	addrs[1];	/* IPv6 address		*/
	} T_DHCP6_OPT_NAME_SERVERS;

/* DOMAIN SEARCH オプション */

typedef struct dhcp6_opt_domain_search {
	uint16_t	code;		/* option code		*/
	uint16_t	len;		/* option length	*/
	uint8_t		name[1];	/* domain search	*/
	} T_DHCP6_OPT_DOMAIN_SEARCH;

/*
 *  DHCPv6 タイミング定数【RFC3315 5.5 参照】
 *  その他のタイミング定数は dhcp6_cli.h 参照
 */

#ifdef SYSTIM_HZ

#define TMO_SOL_MAX_DELAY	(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_SOL_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_SOL_MAX_RT		(120*SYSTIM_HZ)	/* 120[s] 	*/
#define TMO_REQ_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_REQ_MAX_RT		(30*SYSTIM_HZ)	/*  30[s] 	*/
#define TMO_REQ_MAX_RC		10		/* 再送信回数	*/
#define TMO_CNF_MAX_DELAY	(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_CNF_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_CNF_MAX_RT		(4*SYSTIM_HZ)	/*   4[s] 	*/
#define TMO_CNF_MAX_RD		(10*SYSTIM_HZ)	/*  10[s] 	*/
#define TMO_REN_TIMEOUT		(10*SYSTIM_HZ)	/*  10[s] 	*/
#define TMO_REN_MAX_RT		(600*SYSTIM_HZ)	/* 600[s] 	*/
#define TMO_REB_TIMEOUT		(10*SYSTIM_HZ)	/*  10[s] 	*/
#define TMO_REB_MAX_RT		(600*SYSTIM_HZ)	/* 600[s] 	*/
#define TMO_INF_MAX_DELAY	(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_INF_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_INF_MAX_RT		(120*SYSTIM_HZ)	/* 120[s] 	*/
#define TMO_REL_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_REL_MAX_RC		5		/* 再送信回数	*/
#define TMO_DEC_TIMEOUT		(1*SYSTIM_HZ)	/*   1[s] 	*/
#define TMO_DEC_MAX_RC		5		/* 再送信回数	*/
#define TMO_REC_TIMEOUT		(2*SYSTIM_HZ)	/*   2[s] 	*/
#define TMO_REC_MAX_RC		8		/* 再送信回数	*/

#endif	/* of #ifdef SYSTIM_HZ */

/*
 *  DUID -- DHCP Unique ID
 */

/* DUID_LLT 構造体 */

typedef struct dhcp6_duid_llt_hdr {
	uint16_t	code;		/* DUID code		*/
	uint16_t	type;		/* hardware type	*/
	uint32_t	time;		/* time			*/
	} T_DHCP6_DUID_LLT_HDR;

/* DUID_LL 構造体 */

typedef struct dhcp6_duid_ll_hdr {
	uint16_t	code;		/* DUID code		*/
	uint16_t	type;		/* hardware type	*/
	} T_DHCP6_DUID_LL_HDR;

/* DUID タイプ【RFC3315 9参照】*/

#define DUID_LLT		1
#define DUID_EN			2
#define DUID_LL			3

/*
 *  DUID 時間の調整
 *  DUID 時間の開始は 2001/1/1
 */

#define DUID_TIME_EPOCH		0

/* リレー転送の最大ホップ数 */

#define HOP_COUNT_LIMIT		32

/*
 *  IPv6 アドレス
 */

#define IPV6_ADDR_ALL_DHCP_RELAY_SERVERS_INIT	\
	{{{ UINT_C(0xff), UINT_C(0x02), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x01), UINT_C(0x00), UINT_C(0x02) }}}

#define IPV6_ADDR_ALL_DHCP_SERVERS_INIT	\
	{{{ UINT_C(0xff), UINT_C(0x05), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), UINT_C(0x00), \
	    UINT_C(0x00), UINT_C(0x01), UINT_C(0x00), UINT_C(0x03) }}}

/*
 *  UDP ポート番号
 */

#ifndef DHCP6_SRV_CFG_PORTNO
#define DHCP6_SRV_CFG_PORTNO	547	/* server	*/
#endif

#ifndef DHCP6_CLI_CFG_PORTNO
#define DHCP6_CLI_CFG_PORTNO	546	/* client	*/
#endif

#endif	/* of #ifndef DHCP6_H */
