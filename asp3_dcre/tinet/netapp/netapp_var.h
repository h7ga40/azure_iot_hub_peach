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

#ifndef _NETAPP_VAR_H_
#define _NETAPP_VAR_H_

/*
 *  TOPPERS/JSP カーネル、システム時刻 (SYSTIM) の周波数
 */

#define SYSTIM_HZ		ULONG_C(1000000)	/* [us]					*/

/*
 *  IPv6/IPv4 に関する定義
 */

/*
 *  指定されたマクロに対するサンプルアプリケーション内部のマクロの定義
 *
 *    SUPPORT_INET6		 .   .  ON  ON  ON  ON
 *    SUPPORT_INET4		ON  ON   .   .  ON  ON
 *    API_CFG_IP4MAPPED_ADDR	 .  ON   .  ON   .  ON
 *    ----------------------    ----------------------
 *    NETAPP_IP6_CFG             .   .  ON  ON  ON  ON
 *    NETAPP_IP4_CFG            ON  ON   .  ON  ON  ON
 *    
 */

#if defined(SUPPORT_INET6)

#define	NETAPP_IP6_CFG

#endif	/* #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4) || (defined(SUPPORT_INET6) && defined(API_CFG_IP4MAPPED_ADDR))
#define NETAPP_IP4_CFG
#endif

/*
 *  IPv4 と IPv6 をコンパイル時に選択するためのマクロ
 */

#define DBG_LINE_SIZE		63

#if defined(SUPPORT_INET6)

#define T_IN_ADDR		T_IN6_ADDR
#define T_IPEP			T_IPV6EP
#define IP_ADDRANY		IPV6_ADDRANY
#define T_TCPN_CREP		T_TCP6_CREP

#else	/* of #if defined(SUPPORT_INET6) */

#define T_IN_ADDR		T_IN4_ADDR
#define T_IPEP			T_IPV4EP
#define IP_ADDRANY		IPV4_ADDRANY
#define T_TCPN_CREP		T_TCP_CREP

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  API IPプロトコルの指定
 */

#if defined(SUPPORT_INET6)
#define DEFAULT_API_PROTO	API_PROTO_IPV6
#else
#define DEFAULT_API_PROTO	API_PROTO_IPV4
#endif

/*
 *  resolv_hoststr が返す flags の値の定義
 *
 *    注意:
 *
 *      dns.h で定義されている DNS_OPCODE、
 *      resolver.h で定義されている DNS_LUP_FLAGS と重ならない事。
 */

#define HOSTSTR_OTHER		0x0000U
#define HOSTSTR_IPV4		DNS_LUP_FLAGS_NAME_IPV4
#define HOSTSTR_IPV6		DNS_LUP_FLAGS_NAME_IPV6
#define HOSTSTR_HOSTNAME	DNS_LUP_FLAGS_NAME_HOST
#define HOSTSTR_FQDN		DNS_LUP_FLAGS_NAME_FQDN

/*
 *  定数の定義
 */

#define DBG_LINE_SIZE		63

#if defined(SUPPORT_INET6)

#define NUM_VRID_TCP_CEPS	NUM_VRID_TCP6_CEPS

#else	/* of #if defined(SUPPORT_INET6) */

#define NUM_VRID_TCP_CEPS	NUM_VRID_TCP4_CEPS

#endif	/* of #if defined(SUPPORT_INET6) */

/*
 *  関数シミュレーションマクロ
 */

/* メッセージからの読出し */

#define MSG2HOST16(p)		(msg2host16((void*)&(p)))
#define MSG2HOST32(p)		(msg2host32((void*)&(p)))

/* メッセージへの書込み */

#define HOST2MSG16(p,v)		(host2msg16((void*)&(p),(v)))
#define HOST2MSG32(p,v)		(host2msg32((void*)&(p),(v)))

/*
 *  ネットワーク経由コンソール入出力
 */

#ifdef USE_NET_CONS

/* リネーム */

#define syslog		net_syslog
#define serial_ctl_por	net_serial_ctl_por

/* 関数シミュレーションマクロ */

#define FLUSH_SND_BUFF()	flush_snd_buff()
#define WAI_NET_CONS_PRINTF()	syscall(wai_sem(SEM_NET_CONS_PRINTF))
#define SIG_NET_CONS_PRINTF()	sig_sem(SEM_NET_CONS_PRINTF)

#else	/* of #ifdef USE_NET_CONS */

/* 関数シミュレーションマクロ */

#define FLUSH_SND_BUFF()
#define WAI_NET_CONS_PRINTF()
#define SIG_NET_CONS_PRINTF()

#endif	/* of #ifdef USE_NET_CONS */

/*
 *  関数
 */

extern const char *in_strtfn (FN fncd);
extern const char *itron_strerror (ER ercd);
extern const char *dns_strtype (uint_t type);
extern const char *dns_strclass (uint_t class);
extern ER	net_syslog(uint_t prio, const char *format, ...) throw();
extern ER	net_serial_ctl_por(ID portid, uint_t ioctl) throw();
extern void	flush_snd_buff (void);
extern ER	discon_net_cons (void);
extern void	wup_net_cons (void);
extern bool_t	tcp6_is_cepid (ID cepid);
extern ER	alloc_tcp6_rep (ID *repid, ID tskid, T_TCP6_CREP *crep);
extern ER	alloc_tcp6_cep (ID *cepid, ID tskid, T_TCP_CCEP *ccep);
extern ER	alloc_udp6_cep (ID *cepid, ID tskid, T_UDP6_CCEP *ccep);
extern ER	free_tcp6_rep (ID repid, bool_t call_tcp_del_rep);
extern ER	free_tcp6_cep (ID cepid);
extern ER	free_udp6_cep (ID cepid, bool_t call_udp_del_cep);
extern bool_t	tcp4_is_cepid (ID cepid);
extern ER	alloc_tcp4_rep (ID *repid, ID tskid, T_TCP_CREP *crep);
extern ER	alloc_tcp4_cep (ID *cepid, ID tskid, T_TCP_CCEP *ccep);
extern ER	alloc_udp4_cep (ID *cepid, ID tskid, T_UDP_CCEP *ccep);
extern ER	free_tcp4_rep (ID repid, bool_t call_tcp_del_rep);
extern ER	free_tcp4_cep (ID cepid);
extern ER	free_udp4_cep (ID cepid, bool_t call_udp_del_cep);
extern void	ping6 (T_IN6_ADDR *addr, uint_t tmo, uint_t len);
extern void	ping4 (T_IN4_ADDR *addr, uint_t tmo, uint_t len);
extern char	*get_ipv6addr (T_IN6_ADDR *addr, char *line);
extern char	*get_ipv4addr (T_IN4_ADDR *addr, char *line);
extern char	*resolv_options (uint_t *flags, char *line, char apip);
extern char	*resolv_hoststr (uint_t *flags, char *hostname, uint_t name_size, char *line);
extern char	*lookup_ipaddr (T_IN_ADDR *addr, char *line, char apip);
extern uint_t	show_dns_domain_name (ID portid, uint8_t *hdr, uint_t offset);
extern void	host2msg16 (void *msg, uint16_t host_data);
extern void	host2msg32 (void *msg, uint32_t host_data);
extern uint16_t	msg2host16 (void *msg);
extern uint32_t	msg2host32 (void *msg);
extern uint32_t	netapp_rand (void);
extern void	netapp_srand (uint32_t seed);

#if defined(SUPPORT_INET6)

#define TCP_CRE_REP(i,r)	tcp6_cre_rep(i,r)
#define TCP_CRE_CEP(i,c)	tcp6_cre_cep(i,c)
#define UDP_CRE_CEP(i,c)	udp6_cre_cep(i,c)
#define TCP_ACP_CEP(c,r,d,t)	tcp6_acp_cep(c,r,d,t)
#define TCP_CON_CEP(c,m,d,t)	tcp6_con_cep(c,m,d,t)

#define UDP_SND_DAT(c,d,b,l,t)	udp6_snd_dat(c,d,b,l,t)
#define UDP_RCV_DAT(c,d,b,l,t)	udp6_rcv_dat(c,d,b,l,t)

#define TCP_IS_CEPID(i)		tcp6_is_cepid(i)
#define ALLOC_TCP_REP(i,t,r)	alloc_tcp6_rep(i,t,r)
#define ALLOC_TCP_CEP(i,t,c)	alloc_tcp6_cep(i,t,c)
#define ALLOC_UDP_CEP(i,t,c)	alloc_udp6_cep(i,t,c)
#define FREE_TCP_REP(i,r)	free_tcp6_rep(i,r)
#define FREE_TCP_CEP(i)		free_tcp6_cep(i)
#define FREE_UDP_CEP(i,c)	free_udp6_cep(i,c)

#define IP2STR(s,a)		ipv62str(s,a)
#define GET_IPADDR(a,l)		get_ipv6addr(a,l)
#define PING(a,t,l)		ping6(a,t,l)

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

#define TCP_CRE_REP(i,r)	tcp_cre_rep(i,r)
#define TCP_CRE_CEP(i,c)	tcp_cre_cep(i,c)
#define UDP_CRE_CEP(i,c)	udp_cre_cep(i,c)
#define TCP_ACP_CEP(c,r,d,t)	tcp_acp_cep(c,r,d,t)
#define TCP_CON_CEP(c,m,d,t)	tcp_con_cep(c,m,d,t)

#define UDP_SND_DAT(c,d,b,l,t)	udp_snd_dat(c,d,b,l,t)
#define UDP_RCV_DAT(c,d,b,l,t)	udp_rcv_dat(c,d,b,l,t)

#define TCP_IS_CEPID(i)		tcp4_is_cepid(i)
#define ALLOC_TCP_REP(i,t,r)	alloc_tcp4_rep(i,t,r)
#define ALLOC_TCP_CEP(i,t,c)	alloc_tcp4_cep(i,t,c)
#define ALLOC_UDP_CEP(i,t,c)	alloc_udp4_cep(i,t,c)
#define FREE_TCP_REP(i,r)	free_tcp4_rep(i,r)
#define FREE_TCP_CEP(i)		free_tcp4_cep(i)
#define FREE_UDP_CEP(i,c)	free_udp4_cep(i,c)

#define IP2STR(s,a)		ip2str(s,a)
#define GET_IPADDR(a,l)		get_ipv4addr(a,l)
#define PING(a,t,l)		ping4(a,t,l)

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

#endif	/* of #ifndef _NETAPP_VAR_H_ */
