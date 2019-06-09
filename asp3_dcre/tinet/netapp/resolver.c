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

#include <string.h>

#ifdef TARGET_KERNEL_ASP

#include <kernel.h>
#include <t_syslog.h>
#include <target_syssvc.h>
#define LOGTASK_PORTID SIO_PORTID
#include "kernel_cfg.h"
#include "tinet_cfg.h"

#endif	/* of #ifdef TARGET_KERNEL_ASP */

#ifdef TARGET_KERNEL_JSP

#include <s_services.h>
#include <t_services.h>
#include "kernel_id.h"
#include "tinet_id.h"

#endif	/* of #ifdef TARGET_KERNEL_JSP */

#include <tinet_config.h>

#include <netinet/in.h>
#include <netinet/in_itron.h>

#include <netapp/netapp.h>
#include <netapp/netapp_var.h>
#include <netapp/resolver.h>

#if defined(USE_RESOLVER)

/*
 *  局所変数
 */

static uint8_t	dns_domain_name[DNS_NAME_LENGTH + 1] = RSLV_CFG_DNS_DOMAIN_NAME_STR;

#if defined(SUPPORT_INET6)
static T_IN6_ADDR in6_addr_dns      = IPV6_ADDR_DNS_INIT;
static T_IN6_ADDR in6_addr_dns_init = IPV6_ADDR_DNS_INIT;
#endif

#if defined(SUPPORT_INET4)
static T_IN4_ADDR in4_addr_dns = IPV4_ADDR_DNS;
#endif

static uint16_t	dns_msg_id = 0;

/*
 *  get_dns_header -- DNS メッセージから DNS ヘッダを取り出す。
 */

static ER_UINT
get_dns_header (uint8_t *msg, uint_t length, T_RSLV_DNS_MSG *rslv)
{
	T_DNS_HDR	*dns_hdr;

	/* DNS ヘッダを取り出す。*/
	if (length < sizeof(rslv->dns_hdr))
		return EV_RSLV_ESIZE;
	dns_hdr = (T_DNS_HDR*)msg;
	rslv->dns_hdr.id	= MSG2HOST16(dns_hdr->id);
	rslv->dns_hdr.code	= MSG2HOST16(dns_hdr->code);
	rslv->dns_hdr.qdcount	= MSG2HOST16(dns_hdr->qdcount);
	rslv->dns_hdr.ancount	= MSG2HOST16(dns_hdr->ancount);
	rslv->dns_hdr.nscount	= MSG2HOST16(dns_hdr->nscount);
	rslv->dns_hdr.arcount	= MSG2HOST16(dns_hdr->arcount);
	return sizeof(rslv->dns_hdr);
	}

/*
 *  dns_strtype -- DNS の type の文字列を返す。
 */

static const char *
dns_flag_qtype (uint16_t flags)
{
	if      (flags & DNS_LUP_FLAGS_QTYPE_AAAA)
		return "AAAA";
	else if (flags & DNS_LUP_FLAGS_QTYPE_A)
		return "   A";
	else if (flags & DNS_LUP_FLAGS_QTYPE_PTR)
		return " PTR";
	else
		return "unknown type";
	}

#if defined(SUPPORT_INET6)

/*
 *  udp6_dns -- UDP/IPv6 により DNS サーバからメッセージを受信する。
 */

static ER
udp6_dns (uint16_t flags, T_IN6_ADDR *addr, uint8_t *msg, uint_t msg_size, uint_t length)
{
	T_IPV6EP	ep_dns;
	ER_UINT		len = E_OK;
	int		retry;

	/* DNS サーバの IPv6 アドレスが未定義の時はパラメータエラー */
	if (IN6_IS_ADDR_UNSPECIFIED(addr) || (in6_is_addr_ipv4mapped(addr) && addr->s6_addr32[3] == IPV4_ADDRANY)) {
		syslog(LOG_NOTICE, "[RSLV] undefined IPv6 DNS server address.");
		return E_PAR;
		}

	memcpy(&ep_dns.ipaddr, addr, sizeof(*addr));
	ep_dns.portno = RSLV_CFG_DNS_PORTNO;
	for (retry = NUM_DNS_UDP_RETRY + 1; retry -- > 0; ) {
		if ((len = udp6_snd_dat(UDP6_RESOLVER_CEPID, &ep_dns, msg, length, TMO_DNS_UDP_SND)) >= 0) {
			if ((len = udp6_rcv_dat(UDP6_RESOLVER_CEPID, &ep_dns, msg, msg_size, TMO_DNS_UDP_RCV)) >= 0)
				return len;
			}
		if (flags & DNS_LUP_FLAGS_MSG) {
			if (len == E_TMOUT)
				syslog(LOG_NOTICE, "[RSLV] TMOUT: %d, type: %s, server: %s.%d.",
				       NUM_DNS_UDP_RETRY + 1 - retry,
				       dns_flag_qtype(flags),
				       ipv62str(NULL, &ep_dns.ipaddr), ep_dns.portno);
			else {
				syslog(LOG_NOTICE, "[RSLV] error:  %s, type: %s,\n       server: %s.%d.",
				       itron_strerror(len),
				       dns_flag_qtype(flags),
				       ipv62str(NULL, &ep_dns.ipaddr), ep_dns.portno);
				return len;
				}
			}
		}
	return E_TMOUT;
	}

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

/*
 *  udp4_dns -- UDP/IPv4 により DNS サーバからメッセージを受信する。
 */

static ER
udp4_dns (uint16_t flags, T_IN4_ADDR *addr, uint8_t *msg, uint_t msg_size, uint_t length)
{
	T_IPV4EP	ep_dns;
	ER_UINT		len = E_OK;
	int		retry;

	/* DNS サーバの IPv4 アドレスが未定義の時はパラメータエラー */
	if (*addr == IPV4_ADDRANY) {
		syslog(LOG_NOTICE, "[RSLV] undefined IPv4 DNS server address.");
		return E_PAR;
		}

	memcpy(&ep_dns.ipaddr, addr, sizeof(*addr));
	ep_dns.portno = RSLV_CFG_DNS_PORTNO;
	for (retry = NUM_DNS_UDP_RETRY + 1; retry -- > 0; ) {
		if ((len = udp_snd_dat(UDP4_RESOLVER_CEPID, &ep_dns, msg, length, TMO_DNS_UDP_SND)) >= 0) {
			if ((len = udp_rcv_dat(UDP4_RESOLVER_CEPID, &ep_dns, msg, msg_size, TMO_DNS_UDP_RCV)) >= 0)
				return len;
			}
		if (flags & DNS_LUP_FLAGS_MSG) {
			if (len == E_TMOUT)
				syslog(LOG_NOTICE, "[RSLV] TMOUT: %d, type: %s, server: %s.%d.",
				       NUM_DNS_UDP_RETRY + 1 - retry,
				       dns_flag_qtype(flags),
				       ip2str(NULL, &ep_dns.ipaddr), ep_dns.portno);
			else {
				syslog(LOG_NOTICE, "[RSLV] error: %s, type: %s, server: %s.%d.",
				       itron_strerror(len),
				       dns_flag_qtype(flags),
				       ip2str(NULL, &ep_dns.ipaddr), ep_dns.portno);
				return len;
				}
			}
		}
	return E_TMOUT;
	}

#endif	/* of #if defined(SUPPORT_INET4) */

/*
 *  skip_label -- ラベルを読み飛ばす。
 */

static char *
skip_label (char *name)
{
	while (!(*name == '\0' || *name == '.'))
		name ++;
	return name;
	}

/*
 *  add_name -- メッセージに名前を追加する。
 */

static ER_UINT
add_name (uint8_t *msg, uint_t size, uint_t offset, char *name)
{
	uint_t		label_len;
	uint8_t		*tail;

	while (*name != '\0') {

		/* メッセージサイズに達していたらエラー */
		if (offset + 1 > size)
			return E_BOVR;

		/* ラベルの終わりを見つける。*/
		tail = skip_label(name);

		/* ラベル長を設定する。*/
		label_len = tail - (uint8_t*)name;
		if (label_len > DNS_LABEL_LENGTH) {	/* 63 オクテットを超えるとエラー */
			syslog(LOG_NOTICE, "[RSLV] label length(%d) too long > %d.",
			                   label_len, DNS_LABEL_LENGTH);
			return E_PAR;
			}

		*(msg + offset) = (uint8_t)label_len;
		offset ++;

		/* ラベル名を設定する。*/
		if (offset + label_len > size) {	/* メッセージサイズを超えるとエラー */
			syslog(LOG_NOTICE, "[RSLV] message length(%d) too long > %d.",
			                   offset + label_len, size);
			return E_BOVR;
			}

		memcpy((void*)(msg + offset), (void*)name, label_len);
		offset += label_len;

		/* 次のラベルに移動する。*/
		if (*tail == '\0')
			name = tail;
		else
			name = tail + 1;	/* '.' をスキップする。*/
		}

	return offset;
	}

/*
 *  add_ipv6addr -- メッセージに IPv6 アドレスを追加する。
 */

static ER_UINT
add_ipv6addr (uint8_t *msg, uint_t size, uint_t offset, char *name)
{
	T_IN6_ADDR	addr;
	ER_UINT		error;
	uint_t		len;
	int			col;
	char		digit[2];

	len = strlen(name);
	if (len == 0)
		return E_PAR;

	digit[1] = '\0';
	get_ipv6addr(&addr, name);
	for (col = sizeof(T_IN6_ADDR) * 2; -- col >= 0; ) {

		/* 16進数 1桁の値を、名前として追加する。*/
		digit[0] = (addr.s6_addr8[col / 2] >> ((1 - (col % 2)) << 2)) & 0x0f;
		if (digit[0] >= 10)
			digit[0] += 'a' - 10;
		else
			digit[0] += '0';
		if ((error = add_name(msg, size, offset, digit)) < 0)
			return error;
		offset = error;
		}

	/* "ip6" を追加する。*/
	if ((error = add_name(msg, size, offset, "ip6")) < 0)
		return error;

	/* "arpa" を追加する。*/
	return add_name(msg, size, error, "arpa");
	}

/*
 *  add_ipv4addr -- メッセージに IPv4 アドレスを追加する。
 */

static ER_UINT
add_ipv4addr (uint8_t *msg, uint_t size, uint_t offset, char *name)
{
	T_IN4_ADDR	addr;
	ER_UINT		error;
	uint_t		len, octet;
	int			col;
	char		*op, ostr[4];

	len = strlen(name);
	if (len == 0)
		return E_PAR;

	ostr[3] = '\0';
	get_ipv4addr(&addr, name);
	for (col = 0; col < sizeof(T_IN4_ADDR); col ++) {

		/* 1オクテットの値を、整数値からリテラルに変換する。*/
		octet = (addr >> (col << 3)) & 0xff;
		op = &ostr[3];
		while (octet > 0) {
			*(-- op) = octet % 10 + '0';
			octet   /= 10;
			}

		/* 1オクテットの値を、名前として追加する。*/
		if ((error = add_name(msg, size, offset, op)) < 0)
			return error;
		offset = error;
		}

	/* "in-addr" を追加する。*/
	if ((error = add_name(msg, size, offset, "in-addr")) < 0)
		return error;

	/* "arpa" を追加する。*/
	return add_name(msg, size, error, "arpa");
	}

/*
 *  setup_dns_msg -- DNS メッセージを作成する。
 */

static ER
setup_dns_msg (uint16_t flags, char *name, uint8_t *msg, uint_t msg_size)
{
	T_DNS_HDR	dns_hdr;
	ER_UINT		offset;

	memset((void*)&dns_hdr, 0, sizeof(dns_hdr));

	dns_msg_id++;
	HOST2MSG16(dns_hdr.id,      dns_msg_id);
	HOST2MSG16(dns_hdr.code,    (flags & DNS_OPCODE_MASK) | DNS_RECURSION_DESIRED);
	HOST2MSG16(dns_hdr.qdcount, 1);

	memcpy((void*)msg, (void*)&dns_hdr, sizeof(dns_hdr));
	offset = sizeof(dns_hdr);

	switch (flags & DNS_LUP_FLAGS_NAME_MASK) {

	case DNS_LUP_FLAGS_NAME_HOST:
	case DNS_LUP_FLAGS_NAME_FQDN:

		/* ホスト名、または FQDN の時の処理。*/
		/* ホスト名を追加する。*/
		if ((offset = add_name(msg, msg_size, offset, name)) < 0)
			return offset;

		if (offset - sizeof(dns_hdr) > DNS_NAME_LENGTH) {		/* 名前が 255 オクテットを超えるとエラー */
			syslog(LOG_NOTICE, "[RSLV] name length(%d) too long > %d.",
			                   offset - sizeof(dns_hdr), DNS_NAME_LENGTH);
			return E_PAR;
			}

		/* ホスト名だけの時は、ドメイン名を追加する。*/
		if ((flags & DNS_LUP_FLAGS_NAME_MASK) == DNS_LUP_FLAGS_NAME_HOST) {

			/* ドメイン名を追加する。*/
			if ((offset = add_name(msg, msg_size, offset, dns_domain_name)) < 0)
				return offset;

			if (offset - sizeof(dns_hdr) > DNS_NAME_LENGTH) {		/* 名前が 255 オクテットを超えるとエラー */
				syslog(LOG_NOTICE, "[RSLV] name length(%d) too long > %d.",
				                   offset - sizeof(dns_hdr), DNS_NAME_LENGTH);
				return E_PAR;
				}
			}
		break;

	case DNS_LUP_FLAGS_NAME_IPV4:
		if ((offset = add_ipv4addr(msg, msg_size, offset, name)) < 0)
			return offset;
		break;

	case DNS_LUP_FLAGS_NAME_IPV6:
		if ((offset = add_ipv6addr(msg, msg_size, offset, name)) < 0)
			return offset;
		break;

	default:
		syslog(LOG_NOTICE, "[RSLV] unknown name type: %04x.", flags & DNS_LUP_FLAGS_NAME_MASK);
		return E_PAR;
		break;

		}

	/* 名前の終了コード、TYPE、CLASS を追加してもメッセージ長を超えないことを確認する。*/
	if (offset + sizeof(uint16_t) * 2 + 1 > msg_size) {
		syslog(LOG_NOTICE, "[RSLV] message length(%d) too long > %d.",
		                   offset + sizeof(uint16_t) * 2 + 1, msg_size);
		return E_BOVR;
		}

	/* 名前の終了コードを設定する。*/
	*(msg + offset) = '\0';
	offset ++;

	/* DNS の TYPE を設定する。*/
	if (flags & DNS_LUP_FLAGS_QTYPE_PTR)
		host2msg16(msg + offset, DNS_TYPE_PTR);
	else if (flags & DNS_LUP_FLAGS_QTYPE_AAAA)
		host2msg16(msg + offset, DNS_TYPE_AAAA);
	else if (flags & DNS_LUP_FLAGS_QTYPE_A)
		host2msg16(msg + offset, DNS_TYPE_A);
	else {
		syslog(LOG_NOTICE, "[RSLV] unknown query type: %04x.", flags & DNS_LUP_FLAGS_QTYPE_MASK);
		return E_PAR;
		}
	offset += sizeof(uint16_t);

	/* DNS の CLASS を設定する。*/
	host2msg16(msg + offset, DNS_CLASS_IN);

	return offset + sizeof(uint16_t);
	}

/*
 *  skip_dns_name -- DNS メッセージの名前を読み飛ばす。
 *
 *    注意: 圧縮形式を考慮する。上位 2ビットが 11なら圧縮形式。
 */

static ER_UINT
skip_dns_name (uint8_t *name, uint_t offset, uint_t length)
{
	while (*(name + offset)) {
		if ((*(name + offset) & 0xc0) == 0xc0) {
			if (offset + 2 > length)
				return EV_RSLV_ESIZE;
			else
				return offset + 2;
			}
		else if (offset + *name + 1 > length)
			return EV_RSLV_ESIZE;
		else
			offset += *name + 1;
		}
	return offset + 1;
	}

/*
 *  skip_dns_qd -- DNS Question section を読み飛ばす。
 */

static ER_UINT
skip_dns_qd (uint8_t *msg, uint_t offset, uint_t length, uint_t count)
{
	ER_UINT	next;

	while (count != 0) {
		/* name を読み飛ばす。*/
		if ((next = skip_dns_name(msg, offset, length)) < 0)
			return next;
		offset = next;

		/* type、class を読み飛ばす。*/
		if ((offset + sizeof(uint16_t) + sizeof(uint16_t)) > length)
			return EV_RSLV_ESIZE;
		offset += sizeof(uint16_t) + sizeof(uint16_t);

		count --;
		}

	return offset;
	}

/*
 *  skip_dns_rr -- DNS RR を読み飛ばす。
 */

static ER_UINT
skip_dns_rr (uint8_t *msg, uint_t offset, uint_t length, uint_t count)
{
	ER_UINT		next;
	uint16_t	len = 0;

	while (count != 0) {
		/* name を読み飛ばす。*/
		if ((next = skip_dns_name(msg, offset, length)) < 0)
			return next;
		offset = next;

		/* type、class、TTL を読み飛ばす。*/
		if (offset + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) > length)
			return EV_RSLV_ESIZE;
		offset += sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t);

		/* RD length と RDATA を読み飛ばす。*/
		if (offset + sizeof(uint16_t) > length)
			return EV_RSLV_ESIZE;
		len = msg2host16(msg + offset);

		if (offset + sizeof(uint16_t) + len > length)
			return EV_RSLV_ESIZE;
		offset += sizeof(uint16_t) + len;

		count --;
		}

	return offset;
	}

/*
 *  dns_analyze_qd -- DNS メッセージの question section を解析する。
 */

ER_UINT
dns_analyze_qd (T_RSLV_QD *qd, uint_t offset, uint8_t *msg, uint_t length)
{
	ER_UINT		error;
	T_DNS_QD	*dns_qd;

	if ((error = skip_dns_name(msg, offset, length)) < 0)
		return error;
	offset = error;

	/* DNS メッセージ内の形式のサイズでチェックする。*/
	if (offset + T_DNS_QD_SIZE > length)
		return EV_RSLV_ESIZE;

	/*
	 *  DNS メッセージ内の SOA RDATA をバイトオーダーを調整して
	 *  内部構造体にコピーする。
	 */
	dns_qd = (T_DNS_QD*)(msg + offset);
	qd->type  = MSG2HOST16(dns_qd->type);
	qd->class = MSG2HOST16(dns_qd->class);

	return offset + T_DNS_QD_SIZE;
	}

/*
 *  dns_analyze_soa -- DNS メッセージの SOA RDATA を解析する。
 */

ER_UINT
dns_analyze_soa (T_RSLV_SOA *soa, uint_t offset, uint8_t *msg, uint_t length)
{
	ER_UINT		error;
	T_DNS_SOA	*dns_soa;

	/* MNAME をスキップする。*/
	if ((error = skip_dns_name(msg, offset, length)) < 0)
		return error;
	offset = error;

	/* RNAME をスキップする。*/
	if ((error = skip_dns_name(msg, offset, length)) < 0)
		return error;
	offset = error;

	/* DNS メッセージ内の形式のサイズでチェックする。*/
	if (offset + T_DNS_SOA_SIZE > length)
		return EV_RSLV_ESIZE;

	/*
	 *  DNS メッセージ内の SOA RDATA をバイトオーダーを調整して
	 *  内部構造体にコピーする。
	 */
	dns_soa = (T_DNS_SOA*)(msg + offset);
	soa->serial  = MSG2HOST32(dns_soa->serial);
	soa->refresh = MSG2HOST32(dns_soa->refresh);
	soa->retry   = MSG2HOST32(dns_soa->retry);
	soa->expire  = MSG2HOST32(dns_soa->expire);
	soa->minimum = MSG2HOST32(dns_soa->minimum);

	return offset + T_DNS_SOA_SIZE;
	}

/*
 *  dns_analyze_rr -- DNS メッセージの AN/NS/AR section を解析する。
 */

ER_UINT
dns_analyze_rr (T_RSLV_RR *rr, uint_t offset, uint8_t *msg, uint_t length)
{
	ER_UINT		error;
	T_DNS_RR	*dns_rr;

	if ((error = skip_dns_name(msg, offset, length)) < 0)
		return error;
	offset = error;

	/* DNS メッセージ内の形式のサイズでチェックする。*/
	if (offset + T_DNS_RR_SIZE > length)
		return EV_RSLV_ESIZE;

	/*
	 *  DNS メッセージ内の SOA RDATA をバイトオーダーを調整して
	 *  内部構造体にコピーする。
	 */
	dns_rr = (T_DNS_RR*)(msg + offset);
	rr->type	= MSG2HOST16(dns_rr->type);
	rr->class	= MSG2HOST16(dns_rr->class);
	rr->ttl		= MSG2HOST32(dns_rr->ttl);
	rr->rdlength	= MSG2HOST16(dns_rr->rdlength);
	rr->rdata_offset= offset + sizeof(rr->type) + sizeof(rr->class)
	                         + sizeof(rr->ttl)  + sizeof(rr->rdlength);

	return offset + T_DNS_RR_SIZE + rr->rdlength;
	}

/*
 *  analyze_dns_msg -- DNS メッセージを解析する。
 */

static ER_UINT
analyze_dns_msg (uint8_t *msg, uint_t length, T_RSLV_DNS_MSG *rslv)
{
	ER_UINT	offset;

	/* DNS ヘッダを取り出す。*/
	if ((offset = get_dns_header(msg, length, rslv)) < 0)
		return offset;

	/* question section を読み飛ばす。*/
	rslv->qd_offset = offset;
	if ((offset = skip_dns_qd(msg, offset, length, rslv->dns_hdr.qdcount)) < 0)
		return offset;

	/* answer section を読み飛ばす。*/
	rslv->an_offset = offset;
	if ((offset = skip_dns_rr(msg, offset, length, rslv->dns_hdr.ancount)) < 0)
		return offset;

	/* authority records section を読み飛ばす。*/
	rslv->ns_offset = offset;
	if ((offset = skip_dns_rr(msg, offset, length, rslv->dns_hdr.nscount)) < 0)
		return offset;

	/* additional records  section を読み飛ばす。*/
	rslv->ar_offset = offset;
	if ((offset = skip_dns_rr(msg, offset, length, rslv->dns_hdr.arcount)) < 0)
		return offset;

	return E_OK;
	}

/*
 *  dns_lookup_host_sub -- DNS によりホストの情報を取得する（補助関数）。
 */

static ER_UINT
dns_lookup_host_sub (uint16_t flags, char *hostname, uint8_t *msg, 
                     uint_t msg_size, T_RSLV_DNS_MSG *rslv)
{
#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	uint16_t	proto;
#endif
	ER_UINT		length;
	ER		error;

	if ((length = setup_dns_msg(flags, hostname, msg, msg_size)) < 0)
		return length;

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

	if (flags & DNS_LUP_FLAGS_PROTO_IPV6) {
		proto = DNS_LUP_FLAGS_PROTO_IPV6;
		if ((length = udp6_dns(flags, &in6_addr_dns, msg, msg_size, length)) < 0)
			return length;
		}
	else if (flags & DNS_LUP_FLAGS_PROTO_IPV4) {
		proto = DNS_LUP_FLAGS_PROTO_IPV4;
		if ((length = udp4_dns(flags, &in4_addr_dns, msg, msg_size, length)) < 0)
			return length;
		}
	else {
		syslog(LOG_NOTICE, "[RSLV] unknown lookup protocol: %04x.", flags & DNS_LUP_FLAGS_PROTO_MASK);
		return E_PAR;
		}

#else	/* of #if defined(SUPPORT_INET4) */

	if (flags & DNS_LUP_FLAGS_PROTO_IPV6) {
		if ((length = udp6_dns(flags, &in6_addr_dns, msg, msg_size, length)) < 0)
			return length;
		}
	else {
		syslog(LOG_NOTICE, "[RSLV] unknown lookup protocol: %04x.", flags & DNS_LUP_FLAGS_PROTO_MASK);
		return E_PAR;
		}

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

	if (flags & DNS_LUP_FLAGS_PROTO_IPV4) {
		if ((length = udp4_dns(flags, &in4_addr_dns, msg, msg_size, length)) < 0)
			return length;
		}
	else {
		syslog(LOG_NOTICE, "[RSLV] unknown lookup protocol: %04x.", flags & DNS_LUP_FLAGS_PROTO_MASK);
		return E_PAR;
		}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

	if ((error = analyze_dns_msg(msg, length, rslv)) != E_OK)
		return error;
	else {
		if ((rslv->dns_hdr.ancount > 0) && (flags & DNS_LUP_FLAGS_MSG)) {

#if defined(SUPPORT_INET6)

#if defined(SUPPORT_INET4)

			if (proto & DNS_LUP_FLAGS_PROTO_IPV6) {
				syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
				       dns_flag_qtype(flags),
				       ipv62str(NULL, &in6_addr_dns),
				       RSLV_CFG_DNS_PORTNO);
				}
			else {
				syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
				       dns_flag_qtype(flags),
				       ip2str(NULL, &in4_addr_dns),
				       RSLV_CFG_DNS_PORTNO);
				}

#else	/* of #if defined(SUPPORT_INET4) */

			syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
			       dns_flag_qtype(flags),
			       ipv62str(NULL, &in6_addr_dns),
			       RSLV_CFG_DNS_PORTNO);

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

			syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
			       dns_flag_qtype(flags),
			       ip2str(NULL, &in4_addr_dns),
			       RSLV_CFG_DNS_PORTNO);

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

			}
		return length;
		}
	}

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

/*
 *  udp64_dns -- DNS/IPv6、DNS/IPv4 により DNS サーバからメッセージを受信する。
 */

static ER_UINT
udp64_dns (uint16_t flags, char *hostname, uint8_t *msg, 
           uint_t msg_size, T_RSLV_DNS_MSG *rslv)
{
	ER_UINT		length;
	ER		error;
	uint16_t	proto;

	/* DNS メッセージを作成する。*/
	if ((length = setup_dns_msg(flags, hostname, msg, msg_size)) < 0)
		return length;

	/* UDP/IPv6 の DNS サーバに照会する。*/
	proto = DNS_LUP_FLAGS_PROTO_IPV6;
	if ((length = udp6_dns(flags, &in6_addr_dns, msg, msg_size, length)) < 0) {
		/*  UDP/IPv6 の DNS サーバへの照会がエラーになった時は、UDP/IPv4 の DNS サーバに照会する。*/
		if ((length = setup_dns_msg(flags, hostname, msg, msg_size)) < 0)
			return length;

		/* UDP/IPv4 の DNS サーバに照会する。*/
		proto = DNS_LUP_FLAGS_PROTO_IPV4;
		if ((length = udp4_dns(flags, &in4_addr_dns, msg, msg_size, length)) < 0)
			return length;
		}

	/* DNS メッセージを解析する。*/
	if ((error = analyze_dns_msg(msg, length, rslv)) != E_OK)
		return error;
	else {
		if ((rslv->dns_hdr.ancount > 0) && (flags & DNS_LUP_FLAGS_MSG)) {
			if (proto & DNS_LUP_FLAGS_PROTO_IPV6) {
				syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
				       dns_flag_qtype(flags),
				       ipv62str(NULL, &in6_addr_dns),
				       RSLV_CFG_DNS_PORTNO);
				}
			else {
				syslog(LOG_NOTICE, "[RSLV] reslv:    type: %s, server: %s.%d.",
				       dns_flag_qtype(flags),
				       ip2str(NULL, &in4_addr_dns),
				       RSLV_CFG_DNS_PORTNO);
				}
			}
		return length;
		}
	}

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

/*
 *  dns_lookup_host -- DNS によりホストの情報を取得する。
 */

ER_UINT
dns_lookup_host (uint16_t flags, char *hostname, uint8_t *msg, 
              uint_t msg_size, T_RSLV_DNS_MSG *rslv)
{
	ER	error;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	if ((flags & DNS_LUP_FLAGS_PROTO_MASK) == (DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_PROTO_IPV4)) {

		/*
		 *  IPv6/IPv4 の両方が指定された時の処理
		 *  まず type == AAAA で DNS サーバに照会する。
		 */
		if (flags & DNS_LUP_FLAGS_QTYPE_PTR) {
			if ((error = udp64_dns(flags, hostname, msg, msg_size, rslv)) < 0)
				return error;
			}
		else if (flags & DNS_LUP_FLAGS_QTYPE_AAAA) {
			if ((error = udp64_dns(flags & ~DNS_LUP_FLAGS_QTYPE_A, hostname, msg, msg_size, rslv)) < 0)
				return error;
			}
		else {
			HOST2MSG16(rslv->dns_hdr.ancount, 0);
			error = E_OK;
			}

		if (rslv->dns_hdr.ancount == 0) {

			/*
			 *  type == AAAA でアドレス解決ができなかった時の処理
			 *  type == A で DNS サーバに照会する。
			 */
			if (flags & DNS_LUP_FLAGS_QTYPE_A)
				return udp64_dns(flags & ~DNS_LUP_FLAGS_QTYPE_AAAA, hostname, msg, msg_size, rslv);
			else {
				syslog(LOG_NOTICE, "[RSLV] unknown query type: %04x.", flags & DNS_LUP_FLAGS_QTYPE_MASK);
				return E_PAR;
				}
			}
		else
			return error;
		}

	if ((flags & DNS_LUP_FLAGS_QTYPE_MASK) == (DNS_LUP_FLAGS_QTYPE_AAAA | DNS_LUP_FLAGS_QTYPE_A)) {

		/*
		 *  type == AAAA と A の両方が指定された時の処理
		 *  まず type == AAAA で DNS サーバに照会する。
		 */
		if ((error = dns_lookup_host_sub(flags & ~DNS_LUP_FLAGS_QTYPE_A, hostname, msg, msg_size, rslv)) < 0)
			return error;

		if (rslv->dns_hdr.ancount == 0) {

			/*
			 *  type == AAAA でアドレス解決ができなかった時の処理
			 *  type == A で DNS サーバに照会する。
			 */
			return dns_lookup_host_sub(flags & ~DNS_LUP_FLAGS_QTYPE_AAAA, hostname, msg, msg_size, rslv);
			}
#if 0
		else {
			syslog(LOG_NOTICE, "[RSLV] unknown query type: %04x.", flags & DNS_LUP_FLAGS_QTYPE_MASK);
			return E_PAR;
			}
#endif
		}

	else {
		/* IPv6/IPv4 のどちらか一方が指定された時の処理 */
		error = dns_lookup_host_sub(flags, hostname, msg, msg_size, rslv);
		}

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	error = dns_lookup_host_sub(flags, hostname, msg, msg_size, rslv);

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	return error;
	}

#if defined(SUPPORT_INET6)

/*
 *  dns_in6_set_dname -- DNS の ドメイン名を設定する（DHCPv6用）。
 *
 *    注意: new に NULL を指定すると、初期値に戻す。
 */

const uint8_t *
dns_in6_set_dname (const uint8_t *new, uint_t len)
{
	uint8_t	*dst;
	int	label_len, name_len = DNS_NAME_LENGTH;

	if (new == NULL)
		strcpy(dns_domain_name, RSLV_CFG_DNS_DOMAIN_NAME_STR);
	else {
		dst = dns_domain_name;
		while ((*new != 0) && (name_len > 0) && (len > 0)) {
			label_len = *new ++;
			len --;
			while ((name_len > 0) && (len > 0) && (label_len > 0)) {
				*dst ++ = *new ++;
				name_len  --;
				len       --;
				label_len --;
				}
			if ((*new != 0) && (name_len > 0)) {
				*dst ++ = '.';
				name_len --;
				}
			}
		*dst = '\0';
		}
	return dns_domain_name;
	}

/*
 *  dns_in6_get_dname -- DNS の ドメイン名を得る。
 */

const uint8_t *
dns_in6_get_dname (void)
{
	return dns_domain_name;
	}

/*
 *  dns_in6_set_addr -- DNS サーバの IPv6 アドレスを設定する（DHCPv6用）。
 *
 *    注意: new に NULL を指定すると、初期値に戻す。
 */

T_IN6_ADDR *
dns_in6_set_addr (T_IN6_ADDR *new)
{
	if (new == NULL)
		in6_addr_dns = in6_addr_dns_init;
	else
		in6_addr_dns = *new;
	return &in6_addr_dns;
	}

/*
 *  dns_in6_get_addr -- DNS サーバの IPv6 アドレスを得る。
 */

T_IN6_ADDR *
dns_in6_get_addr (T_IN6_ADDR *dst)
{
	if (dst == NULL)
		return NULL;
	else {
		*dst = in6_addr_dns;
		return dst;
		}
	}

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

/*
 *  dns_in4_set_dname -- DNS の ドメイン名を設定する（DHCPv4用）。
 *
 *    注意: new に NULL を指定すると、初期値に戻す。
 */

const uint8_t *
dns_in4_set_dname (const uint8_t *new, uint_t len)
{
	uint8_t	*dst;
	int	name_len = DNS_NAME_LENGTH;

	if (new == NULL)
		strcpy(dns_domain_name, RSLV_CFG_DNS_DOMAIN_NAME_STR);
	else {
		dst = dns_domain_name;
		while ((name_len -- > 0) && (len -- > 0) && *new)
			*dst ++ = *new ++;
		*dst = '\0';
		}
	return dns_domain_name;
	}

/*
 *  dns_in4_get_dname -- DNS の ドメイン名を得る。
 */

const uint8_t *
dns_in4_get_dname (void)
{
	return dns_domain_name;
	}

/*
 *  dns_in4_set_addr -- DNS サーバの IPv4 アドレスを設定する（DHCPv4用）。
 *
 *    注意: new に NULL を指定すると、初期値に戻す。
 */

T_IN4_ADDR *
dns_in4_set_addr (T_IN4_ADDR *new)
{
	if (new == NULL)
		in4_addr_dns = IPV4_ADDR_DNS;
	else
		in4_addr_dns = *new;
	return &in4_addr_dns;
	}

/*
 *  dns_in4_get_addr -- DNS サーバの IPv4 アドレスを得る。
 */

T_IN4_ADDR *
dns_in4_get_addr (T_IN4_ADDR *dst)
{
	if (dst == NULL)
		return NULL;
	else {
		*dst = in4_addr_dns;
		return dst;
		}
	}

#endif	/* of #if defined(SUPPORT_INET4) */

/*
 *  dns_host_addr -- ホスト名を IP アドレスに変換する。
 */

ER
dns_host_addr (uint16_t flags, char *hostname, T_IN_ADDR *addr)
{
	T_RSLV_DNS_MSG	rslv;
	T_RSLV_RR	rr;
	ER_UINT		length, offset;
	ER		error = E_OK;
	int		scount;
	uint8_t 	*msg;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	T_IN4_ADDR	addr4;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	if ((error = tget_mpf(MPF_RSLV_SRBUF, (void*)&msg, TMO_FEVR)) != E_OK)
		return error;

	if ((length = dns_lookup_host(flags, hostname, msg, DNS_UDP_MSG_LENGTH, &rslv)) < 0) {
		error = length;
		goto err_ret;
		}

	offset = rslv.an_offset;
	for (scount = 1; scount <= rslv.dns_hdr.ancount; scount ++) {
		if ((offset = dns_analyze_rr(&rr, offset, msg, length)) < 0) {
			error = offset;
			goto err_ret;
			}

#if defined(SUPPORT_INET6)

		if (rr.type == DNS_TYPE_AAAA && rr.class == DNS_CLASS_IN &&  rr.rdlength == sizeof(*addr)) {
			memcpy((void*)addr, (void*)(msg + rr.rdata_offset), sizeof(*addr));
			break;
			}

#if defined(SUPPORT_INET4)

		if (rr.type == DNS_TYPE_A && rr.class == DNS_CLASS_IN && rr.rdlength == sizeof(addr4)) {
			addr4 = msg2host32(msg + rr.rdata_offset);
			in6_make_ipv4mapped(addr, addr4);
			break;
			}

#endif	/* of #if defined(SUPPORT_INET4) */

#else	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)

		if (rr.type == DNS_TYPE_A && rr.class == DNS_CLASS_IN && rr.rdlength == sizeof(*addr)) {
			*addr = msg2host32((msg + rr.rdata_offset));
			break;
			}

#endif	/* of #if defined(SUPPORT_INET4) */

#endif	/* of #if defined(SUPPORT_INET6) */

		}
	if (scount > rslv.dns_hdr.ancount)
		error = EV_RSLV_UNEXPCT;

err_ret:
	rel_mpf(MPF_RSLV_SRBUF, (void*)msg);
	return error;
	}

#endif	/* of #if defined(USE_RESOLVER) */
