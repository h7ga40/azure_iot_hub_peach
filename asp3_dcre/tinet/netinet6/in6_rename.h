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

#ifndef _IN6_RENAME_H_
#define _IN6_RENAME_H_

/*
 *  TCP/IPv6 プロトコルスタック内部名のリネーム
 */

/* netinet6/in6.c */

#define in6_are_prefix_equal	_tinet_in6_are_prefix_equal
#define in6ifa_ifpwithrtrix	_tinet_in6ifa_ifpwithrtrix
#define in6_if_up		_tinet_in6_if_up
#define in6_if2idlen		_tinet_in6_if2idlen
#define in6_ifaddr_timer	_tinet_in6_ifaddr_timer
#define in6ifa_ifpwithix	_tinet_in6ifa_ifpwithix
#define in6_addr2maix		_tinet_in6_addr2maix
#define in6_ifawithifp		_tinet_in6_ifawithifp
#define in6_addr2ifaix		_tinet_in6_addr2ifaix
#define in6_update_ifa		_tinet_in6_update_ifa
#define in6_ifainit		_tinet_in6_ifainit
#define in6_addrwithifp		_tinet_in6_addrwithifp
#define in6_addr_loopback	_tinet_in6_addr_loopback

/* netinet6/in6_ifattach.c */

#define in6_ifattach		_tinet_in6_ifattach

/* netinet6/in6_subr.c */

#define inn6_is_dstaddr_accept	_tinet_inn6_is_dstaddr_accept
#define in6_lookup_ifaddr	_tinet_in6_lookup_ifaddr
#define in6_rtalloc		_tinet_in6_rtalloc
#define in6_get_ifaddr		_tinet_in6_get_ifaddr
#define in6_cksum		_tinet_in6_cksum
#define in6_gateway_lookup	_tinet_in6_gateway_lookup
#define in6_is_dstaddr_accept	_tinet_in6_is_dstaddr_accept
#define in6_hostcache_getmtu	_tinet_in6_hostcache_getmtu
#define in6_set_header		_tinet_in6_set_header
#define in6_get_datagram	_tinet_in6_get_datagram
#define in6_rtredirect		_tinet_in6_rtredirect
#define get_ip6_hdr_size	_tinet_get_ip6_hdr_size
#define in6_plen2pmask		_tinet_in6_plen2pmask
#define in6_rtnewentry		_tinet_in6_rtnewentry
#define in6_rtinit		_tinet_in6_rtinit
#define in6_rttimer		_tinet_in6_rttimer
#define in6_lookup_multi	_tinet_in6_lookup_multi
#define in6_hostcache_update	_tinet_in6_hostcache_update
#define in6_init		_tinet_in6_init

/* netinet6/if6_ether.c */

#define ether_map_ipv6_multicast	\
				_tinet_ether_map_ipv6_multicast

/* netinet6/ip6_input.c */

#define ip6_input		_tinet_ip6_input
#define ip6_lasthdr		_tinet_ip6_lasthdr
#define ip6_unknown_opt		_tinet_ip6_unknown_opt
#define ip6_init		_tinet_ip6_init
#define ip6_nexthdr		_tinet_ip6_nexthdr
#define ip6_remove_exthdrs	_tinet_ip6_remove_exthdrs
#define ip6_get_prev_hdr	_tinet_ip6_get_prev_hdr

/* netinet6/ip6_output.c */

#define ip6_output		_tinet_ip6_output
#define linkmtu			_tinet_linkmtu

/* netinet6/dest6.c */

#define dest6_input		_tinet_dest6_input

/* netinet6/rout6.c */

#define route6_input		_tinet_route6_input

/* netinet6/frag6.c */

#define frag6_input		_tinet_frag6_input
#define frag6_timer		_tinet_frag6_timer
#define ip6_get_frag_queue	_tinet_ip6_get_frag_queue

/* netinet6/ah_input.c */

#define ah6_input		_tinet_ah6_input

/* netinet6/esp_input.c */

#define esp6_input		_tinet_esp6_input

/* netinet6/icmp6.c */

#define icmp6_input		_tinet_icmp6_input
#define icmp6_error		_tinet_icmp6_error
#define icmp6_echo_reply	_tinet_icmp6_echo_reply

/* netinet6/nd6.c */

#define nd6_ifattach		_tinet_nd6_ifattach
#define nd6_is_addr_neighbor	_tinet_nd6_is_addr_neighbor
#define nd6_cache_lladdr	_tinet_nd6_cache_lladdr
#define nd6_output_hold		_tinet_nd6_output_hold
#define nd6_init		_tinet_nd6_init
#define nd6_output		_tinet_nd6_output
#define nd6_options		_tinet_nd6_options
#define nd6_storelladdr		_tinet_nd6_storelladdr
#define nd6_lookup		_tinet_nd6_lookup
#define nd6_get_drl		_tinet_nd6_get_drl
#define nd6_get_cache		_tinet_nd6_get_cache
#define nd6_timer		_tinet_nd6_timer

/* netinet6/nd6_nbr.c */

#define nd6_dad_start		_tinet_nd6_dad_start
#define nd6_ns_input		_tinet_nd6_ns_input
#define nd6_na_input		_tinet_nd6_na_input
#define nd6_ns_output		_tinet_nd6_ns_output
#define nd6_na_output		_tinet_nd6_na_output

/* netinet6/nd6_rtr.c */

#define nd6_get_prl		_tinet_nd6_get_prl
#define nd6_router_lookup	_tinet_nd6_router_lookup
#define nd6_onlink_prefix_lookup	\
				_tinet_nd6_onlink_prefix_lookup
#define nd6_defrtrlist_del	_tinet_nd6_defrtrlist_del
#define nd6_prefix_onlink	_tinet_nd6_prefix_onlink
#define nd6_get_drl		_tinet_nd6_get_drl
#define nd6_rtrsol_ctl		_tinet_nd6_rtrsol_ctl
#define nd6_defrtrlist_timer	_tinet_nd6_defrtrlist_timer
#define nd6_ra_input		_tinet_nd6_ra_input
#define nd6_prelist_timer	_tinet_nd6_prelist_timer
#define nd6_prefix_offlink	_tinet_nd6_prefix_offlink
#define nd6_defrtrlist_lookup	_tinet_nd6_defrtrlist_lookup
#define ip6_defhlim		_tinet_ip6_defhlim
#define nd6_reachable_time	_tinet_nd6_reachable_time
#define nd6_base_reachable_time	_tinet_nd6_base_reachable_time
#define nd6_recalc_reachtm_interval	\
				_tinet_nd6_recalc_reachtm_interval
#define nd6_retrans_time	_tinet_nd6_retrans_time

/* netinet6/scope6.c */

#define in6_addrscope		_tinet_in6_addrscope

#endif	/* of #ifndef _IN6_RENAME_H_ */
