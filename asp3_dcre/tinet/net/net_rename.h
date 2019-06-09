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

#ifndef _NET_RENAME_H_
#define _NET_RENAME_H_

/*
 *  汎用ネットワークインタフェース内部名のリネーム
 */

/* net/ethernet.c */

#define ether_input_task	_tinet_ether_input_task
#define ether_get_ifnet		_tinet_ether_get_ifnet
#define ether_in6_resolve_multicast	\
				_tinet_ether_in6_resolve_multicast
#define ether_srand		_tinet_ether_srand
#define ether_output		_tinet_ether_output
#define	ether_raw_output	_tinet_ether_raw_output
#define ether_output_task	_tinet_ether_output_task

#define ether_broad_cast_addr	_tinet_ether_broad_cast_addr
#define ether_ipv4_addr_local	_tinet_ether_ipv4_addr_local
#define ether_ipv4_addr_local_mask	\
				_tinet_ether_ipv4_addr_local_mask

#define ether_in6_ifaddr	_tinet_ether_in6_ifaddr
#define ether_in6_ifaddr_list	_tinet_ether_in6_ifaddr_list

/* net/if.c */

#ifdef SUPPORT_ETHER

#define ifinit			_tinet_ifinit
#define if_addmulti		_tinet_if_addmulti

#endif	/* of #ifdef SUPPORT_ETHER */

/* net/net_buf.c */

#define tget_net_buf_ex		_tinet_tget_net_buf_ex
#define tget_net_buf		_tinet_tget_net_buf
#define rel_net_buf			_tinet_rel_net_buf
#define rus_net_buf			_tinet_rus_net_buf
#define net_buf_siz			_tinet_net_buf_siz
#define net_buf_max_siz		_tinet_net_buf_max_siz
#define nbuf_get_tbl		_tinet_nbuf_get_tbl
#define nbuf_get_tbl_size	_tinet_nbuf_get_tbl_size

/* net/net_subr.c */

#define convert_hexdigit	_tinet_convert_hexdigit
#define net_rand		_tinet_net_rand
#define net_srand		_tinet_net_srand
#define rev_memcpy_word		_tinet_rev_memcpy_word
#define rev_memcmp_word		_tinet_rev_memcmp_word

/* net/net_timer.c */

#define timeout			_tinet_timeout
#define untimeout		_tinet_untimeout
#define busy_callout		_tinet_busy_callout
#define max_busy		_tinet_max_busy

/* route_cfg.c */

#define routing6_tbl		_tinet_routing6_tbl
#define routing4_tbl		_tinet_routing4_tbl

#ifdef SUPPORT_PPP

/* net/ppp.c */

#define ppp_output		_tinet_ppp_output
#define ppp_cp_output		_tinet_ppp_cp_output
#define ppp_phase		_tinet_ppp_phase
#define protocols		_tinet_protocols

/* net/ppp_hdlc.c */

#define HDLC_dummy_read		_tinet_HDLC_dummy_read
#define HDLC_read		_tinet_HDLC_read
#define HDLC_write		_tinet_HDLC_write
#define remote_ACCM		_tinet_remote_ACCM

/* net/ppp_lcp.c */

#define lcp_close		_tinet_lcp_close
#define lcp_open		_tinet_lcp_open
#define lcp_lowerdown		_tinet_lcp_lowerdown
#define lcp_sprotrej		_tinet_lcp_sprotrej
#define lcp_lowerup		_tinet_lcp_lowerup
#define lcp_finished		_tinet_lcp_finished
#define lcp_down		_tinet_lcp_down
#define lcp_callbacks		_tinet_lcp_callbacks
#define lcp_fsm			_tinet_lcp_fsm
#define lcp_protent		_tinet_lcp_protent
#define lcp_local_ack_cfg	_tinet_lcp_local_ack_cfg
#define lcp_remote_ack_cfg	_tinet_lcp_remote_ack_cfg

/* net/ppp_fsm.c */

#define fsm_init		_tinet_fsm_init
#define fsm_open		_tinet_fsm_open
#define fsm_input		_tinet_fsm_input
#define fsm_protreject		_tinet_fsm_protreject
#define fsm_lowerdown		_tinet_fsm_lowerdown
#define fsm_close		_tinet_fsm_close
#define fsm_lowerup		_tinet_fsm_lowerup
#define ppp_open_mode		_tinet_ppp_open_mode

/* net/ppp_auth.c */

#define auth_reset		_tinet_auth_reset
#define link_terminated		_tinet_link_terminated
#define link_established	_tinet_link_established
#define link_required		_tinet_link_required
#define network_phase		_tinet_network_phase
#define link_down		_tinet_link_down

/* net/ppp_ipcp.c */

#define wait_ipcp		_tinet_wait_ipcp
#define ppp_get_ifnet		_tinet_ppp_get_ifnet
#define ipcp_protent		_tinet_ipcp_protent
#define ipcp_callbacks		_tinet_ipcp_callbacks
#define ipcp_fsm		_tinet_ipcp_fsm
#define ipcp_remote_ack_cfg	_tinet_ipcp_remote_ack_cfg
#define ipcp_local_ack_cfg	_tinet_ipcp_local_ack_cfg

/* net/ppp_upap.c */

#define upap_auth_client	_tinet_upap_auth_client
#define upap_auth_server	_tinet_upap_auth_server
#define pap_protent		_tinet_pap_protent

#endif	/* of #ifdef SUPPORT_PPP */

#ifdef SUPPORT_LOOP

#endif	/* of #ifdef SUPPORT_LOOP */

#endif	/* of #ifndef _NET_RENAME_H_ */
