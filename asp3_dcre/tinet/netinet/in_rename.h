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

#ifndef _IN_RENAME_H_
#define _IN_RENAME_H_

/*
 *  TCP/IP プロトコルスタック内部名のリネーム
 */

/* netinet/tcp_input.c */

#define tcp_input		_tinet_tcp_input
#define tcp_move_twcep		_tinet_tcp_move_twcep

/* netinet/tcp_output.c */

/* netinet/tcp_timer.c */

#define	tcp_cancel_timers	_tinet_tcp_cancel_timers
#define tcp_slow_timo		_tinet_tcp_slow_timo
#define tcp_fast_timo		_tinet_tcp_fast_timo
#define tcp_range_set		_tinet_tcp_range_set
#define tcp_set_persist_timer	_tinet_tcp_set_persist_timer

/* netinet/tcp_usrrq.c */

#define tcp_user_closed		_tinet_tcp_user_closed

/* netinet/tcp_subr.c */

#define tcp_iss			_tinet_tcp_iss
#define tcp_init		_tinet_tcp_init
#define tcp_init_iss		_tinet_tcp_init_iss
#define tcp_close		_tinet_tcp_close
#define tcp_drop		_tinet_tcp_drop
#define tcp_respond		_tinet_tcp_respond
#define	tcp_get_segment		_tinet_tcp_get_segment
#define tcp_rexmt_val		_tinet_tcp_rexmt_val
#define tcp_init_cep		_tinet_tcp_init_cep
#define tcp_notify		_tinet_tcp_notify
#define tcp_alloc_auto_port	_tinet_tcp_alloc_auto_port
#define tcp_alloc_port		_tinet_tcp_alloc_port
#define tcp_free_reassq		_tinet_tcp_free_reassq
#define tcp_move_ra2rw		_tinet_tcp_move_ra2rw
#define tcp_wait_rwbuf		_tinet_tcp_wait_rwbuf
#define tcp_write_raque		_tinet_tcp_write_raque
#define tcp_lock_cep		_tinet_tcp_lock_cep
#define tcp_can_send_more	_tinet_tcp_can_send_more
#define tcp_can_recv_more	_tinet_tcp_can_recv_more
#define tcp_output_trace	_tinet_tcp_output_trace
#define tcp_input_trace		_tinet_tcp_input_trace
#define tcp_find_cep		_tinet_tcp_find_cep
#define tcp_find_twcep		_tinet_tcp_find_twcep
#define tcp_is_addr_accept	_tinet_tcp_is_addr_accept
#define tcp_move_twcep		_tinet_tcp_move_twcep
#define tcpn_get_segment	_tinet_tcpn_get_segment

/* netinet/tcp_subr_cs.c */

#define tcp_read_rwbuf		_tinet_tcp_read_rwbuf
#define tcp_write_rwbuf		_tinet_tcp_write_rwbuf
#define tcp_drop_rwbuf		_tinet_tcp_drop_rwbuf
#define tcp_free_rwbufq		_tinet_tcp_free_rwbufq
#define tcp_get_rwbuf_addr	_tinet_tcp_get_rwbuf_addr

#define tcp_free_rwbufq_cs	_tinet_tcp_free_rwbufq_cs
#define tcp_get_rwbuf_addr_cs	_tinet_tcp_get_rwbuf_addr_cs
#define tcp_read_rwbuf_cs	_tinet_tcp_read_rwbuf_cs
#define tcp_write_rwbuf_cs	_tinet_tcp_write_rwbuf_cs
#define tcp_drop_rwbuf_cs	_tinet_tcp_drop_rwbuf_cs

#define tcp_wait_swbuf		_tinet_tcp_wait_swbuf
#define tcp_init_swbuf		_tinet_tcp_init_swbuf
#define tcp_alloc_swbuf		_tinet_tcp_alloc_swbuf
#define tcp_is_swbuf_full	_tinet_tcp_is_swbuf_full
#define tcp_drop_swbuf		_tinet_tcp_drop_swbuf
#define tcp_free_swbufq		_tinet_tcp_free_swbufq
#define tcp_read_swbuf		_tinet_tcp_read_swbuf
#define tcp_write_swbuf		_tinet_tcp_write_swbuf
#define tcp_pull_res_nbuf	_tinet_tcp_pull_res_nbuf
#define tcp_push_res_nbuf	_tinet_tcp_push_res_nbuf
#define tcp_get_swbuf_addr	_tinet_tcp_get_swbuf_addr
#define tcp_send_swbuf		_tinet_tcp_send_swbuf

#define tcp_get_swbuf_addr_cs	_tinet_tcp_get_swbuf_addr_cs
#define tcp_wait_swbuf_cs	_tinet_tcp_wait_swbuf_cs
#define tcp_is_swbuf_full_cs	_tinet_tcp_is_swbuf_full_cs
#define tcp_alloc_swbuf_cs	_tinet_tcp_alloc_swbuf_cs
#define tcp_send_swbuf_cs	_tinet_tcp_send_swbuf_cs
#define tcp_free_swbufq_cs	_tinet_tcp_free_swbufq_cs
#define tcp_drop_swbuf_cs	_tinet_tcp_drop_swbuf_cs
#define tcp_write_swbuf_cs	_tinet_tcp_write_swbuf_cs

/* netinet/tcp_subr_ncs.c */

#define tcp_drop_rwbuf_ncs	_tinet_tcp_drop_rwbuf_ncs
#define tcp_read_rwbuf_ncs	_tinet_tcp_read_rwbuf_ncs
#define tcp_is_swbuf_full_ncs	_tinet_tcp_is_swbuf_full_ncs
#define tcp_drop_swbuf_ncs	_tinet_tcp_drop_swbuf_ncs
#define tcp_read_swbuf_ncs	_tinet_tcp_read_swbuf_ncs
#define tcp_wait_swbuf_ncs	_tinet_tcp_wait_swbuf_ncs
#define tcp_write_swbuf_ncs	_tinet_tcp_write_swbuf_ncs
#define tcp_write_rwbuf_ncs	_tinet_tcp_write_rwbuf_ncs
#define tcp_get_swbuf_addr_ncs	_tinet_tcp_get_swbuf_addr_ncs
#define tcp_send_swbuf_ncs	_tinet_tcp_send_swbuf_ncs
#define tcp_get_rwbuf_addr_ncs	_tinet_tcp_get_rwbuf_addr_ncs

/* tcp_cfg.c */

#define tcp6_rep		_tinet_tcp6_rep
#define tcp4_rep		_tinet_tcp4_rep
#define tcp_cep			_tinet_tcp_cep
#define tcp_twcep		_tinet_tcp_twcep
#define tmax_tcp_repid		_tinet_tmax_tcp_repid
#define tmax_tcp_cepid		_tinet_tmax_tcp_cepid
#define tmax_tcp_crepid		_tinet_tmax_tcp_repid	/* TINET 1.3 との互換性のための定義 */
#define tmax_tcp_ccepid		_tinet_tmax_tcp_cepid	/* TINET 1.3 との互換性のための定義 */

/* netinet/udp_input.c */

#define udp6_reply_no_port	_tinet_udp6_reply_no_port
#define udp6_input_select	_tinet_udp6_input_select
#define udp6_input		_tinet_udp6_input
#define udp4_reply_no_port	_tinet_udp4_reply_no_port
#define udp4_input_select	_tinet_udp4_input_select
#define udp4_input		_tinet_udp4_input

/* netinet/udp_subr.c */

#define udp6_alloc_port		_tinet_udp6_alloc_port
#define udp6_alloc_auto_port	_tinet_udp6_alloc_auto_port
#define udp6_notify		_tinet_udp6_notify
#define udp6_find_cep		_tinet_udp6_find_cep
#define udp4_alloc_port		_tinet_udp4_alloc_port
#define udp4_alloc_auto_port	_tinet_udp4_alloc_auto_port
#define udp4_notify		_tinet_udp4_notify
#define udp4_find_cep		_tinet_udp4_find_cep

/* netinet/udp_usrreq.c */

#define udp_send_data		_tinet_udp_send_data

/* netinet/udp_usrreq_nblk.c */

#define udp_can_rcv		_tinet_udp_can_rcv
#define udp_can_snd		_tinet_udp_can_snd

/* udp_cfg.c */

#define udp6_cep		_tinet_udp6_cep
#define udp4_cep		_tinet_udp4_cep
#define tmax_udp_cepid		_tinet_tmax_udp_cepid
#define tmax_udp_ccepid		_tinet_tmax_udp_cepid	/* TINET 1.3 との互換性のための定義 */

/* netinet/ip_icmp.c */

#define icmp_input		_tinet_icmp_input
#define icmp_error		_tinet_icmp_error
#define icmp_echo_reply		_tinet_icmp_echo_reply

/* netinet/ip_input.c */

#define ip_input		_tinet_ip_input
#define ip_init			_tinet_ip_init
#define ip_frag_timer		_tinet_ip_frag_timer
#define ip_get_frag_queue	_tinet_ip_get_frag_queue
#define ip_remove_options	_tinet_ip_remove_options

/* netinet/ip_output.c */

#define ip_output		_tinet_ip_output

/* netinet/in_subr.c */

#define inn_addrwithifp		_tinet_inn_addrwithifp
#define in_cksum_carry		_tinet_in_cksum_carry
#define inn_is_addr_multicast	_tinet_inn_is_addr_multicast
#define inn_get_datagram	_tinet_inn_get_datagram
#define inn_is_dstaddr_accept	_tinet_inn_is_dstaddr_accept
#define ip_exchg_addr		_tinet_ip_exchg_addr
#define inn_copy_to_host	_tinet_inn_copy_to_host
#define in_cksum_sum		_tinet_in_cksum_sum
#define inn_are_net_srcaddr_equal	\
				_tinet_inn_are_net_srcaddr_equal

/* netinet/in4_subr.c */

#define in4_add_ifaddr		_tinet_in4_add_ifaddr
#define in4_set_header		_tinet_in4_set_header
#define inn4_is_dstaddr_accept	_tinet_inn4_is_dstaddr_accept
#define in4_rtredirect		_tinet_in4_rtredirect
#define in_rtnewentry		_tinet_in_rtnewentry
#define in_cksum		_tinet_in_cksum
#define	in4_addrwithifp		_tinet_in4_addrwithifp
#define in4_rtalloc		_tinet_in4_rtalloc
#define in4_init		_tinet_in4_init
#define in_rttimer		_tinet_in_rttimer
#define in4_get_ifaddr		_tinet_in4_get_ifaddr
#define in4_get_datagram	_tinet_in4_get_datagram
#define in4_is_dstaddr_accept	_tinet_in4_is_dstaddr_accept
#define in4_cksum		_tinet_in4_cksum
#define in_rtinit		_tinet_in_rtinit
#define in4_ifawithifp		_tinet_in4_ifawithifp

/* netinet/if_ether.c */

#define arp_request		_tinet_arp_request
#define arp_input		_tinet_arp_input
#define arp_resolve		_tinet_arp_resolve
#define arp_init		_tinet_arp_init
#define arp_get_cache		_tinet_arp_get_cache

#define ether_map_ipv4_multicast	_tinet_ether_map_ipv4_multicast

/* netinet6/icmp6.c */

#define icmp6_input		_tinet_icmp6_input

#endif	/* of #ifndef _IN_RENAME_H_ */
