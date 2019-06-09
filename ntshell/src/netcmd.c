/*
 *  TINET (TCP/IP Protocol Stack)
 * 
 *  Copyright (C) 2001-2017 by Dep. of Computer Science and Engineering
 *                   Tomakomai National College of Technology, JAPAN
 *
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
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
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 *
 *  @(#) $Id$
 */
#include "shellif.h"
#include <kernel.h>
#include <t_stdlib.h>
#include <sil.h>
#include <string.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include <tinet_config.h>
#include <netinet/in.h>
#include <netinet/in_itron.h>
#include <tinet_nic_defs.h>
#include <tinet_cfg.h>
#include <netinet/in_var.h>
#include <net/ethernet.h>
#include <net/if6_var.h>
#include <net/net.h>
#include <net/if_var.h>
#include <netinet/udp_var.h>
#include <netapp/netapp_var.h>
#include <netapp/netapp.h>
#include <netapp/dhcp4_cli.h>
#include <netapp/resolver.h>
#include "core/ntlibc.h"
#include <stdio.h>
#include "ntp_cli.h"
#include "kernel_cfg.h"

#if defined(SUPPORT_INET6)
#define DEFAULT_API_PROTO	API_PROTO_IPV6
#else
#define DEFAULT_API_PROTO	API_PROTO_IPV4
#endif
extern void ping6(T_IN6_ADDR *addr, uint_t tmo, uint_t len);
extern void ping4(T_IN4_ADDR *addr, uint_t tmo, uint_t len);

/*
 *  str_num -- cons_printf の数値変換
 */

static void
str_chr(char *text, int *pos, int size, char c)
{
	if (*pos >= size)
		return;
	text[*pos] = c;
	*pos = *pos + 1;
}

/*
 *  str_num -- cons_printf の数値変換
 */

static int
str_num(char *text, int *pos, int size, ulong_t 	val, int radix,
         const char *radchar, int width, bool_t minus, char padchar)
{
	char	digits[24];
	int	ix, pad, pchars;
	bool_t	left;

	if (width < 0) {
		width = -width;
		left = true;
	}
	else
		left = false;

	ix = 0;
	do {
		digits[ix ++] = radchar[val % radix];
		val /= radix;
	} while (val != 0);

	if (minus)
		digits[ix ++] = '-';

	if (width > ix)
		pchars = width;
	else
		pchars = ix;

	pad = ix;
	if (!left)	/* 右詰め */
		for ( ; pad < width; pad ++)
			str_chr(text, pos, size, padchar);

	while (ix -- > 0)
		str_chr(text, pos, size, digits[ix]);

	if (left)	/* 左詰め */
		for ( ; pad < width; pad ++)
			str_chr(text, pos, size, padchar);

	return pchars;
}

/*
 *  str_ipv4addr -- IPv4 アドレス出力
 */

int
str_ipv4addr (char *text, int size, T_IN4_ADDR *addr, int width)
{
	int len = 3, pos = 0;	/* 3 は '.' の文字数 */

	len += str_num(text, &pos, size, (*addr >> 24) & 0xff, 10, radhex, 0, false, ' '); 
	str_chr(text, &pos, size, '.');
	len += str_num(text, &pos, size, (*addr >> 16) & 0xff, 10, radhex, 0, false, ' '); 
	str_chr(text, &pos, size, '.');
	len += str_num(text, &pos, size, (*addr >>  8) & 0xff, 10, radhex, 0, false, ' '); 
	str_chr(text, &pos, size, '.');
	len += str_num(text, &pos, size,  *addr        & 0xff, 10, radhex, 0, false, ' '); 

	for ( ; len < width; len ++)
		str_chr(text, &pos, size, ' ');

	return len;
}

/*
 *  ipv6addr -- IPv6 アドレス出力
 */

int
str_ipv6addr (char *text, int size, const T_IN6_ADDR *addr, int width)
{
	int	len = 0, ix, len6, pos = 0;
	bool_t	omit = false, zero = false;

	if (addr == NULL || IN6_IS_ADDR_UNSPECIFIED(addr)) {
		str_chr(text, &pos, size, '0');
		str_chr(text, &pos, size, ':');
		str_chr(text, &pos, size, ':');
		str_chr(text, &pos, size, '0');
		len = 4;
	}
	else {
		if (in6_is_addr_ipv4mapped(addr))
			len6 = sizeof(T_IN6_ADDR) / 2 - 2;
		else
			len6 = sizeof(T_IN6_ADDR) / 2;
		for (ix = 0; ix < len6; ix ++) {
			if (omit) {
				len += str_num(text, &pos, size, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					str_chr(text, &pos, size, ':');
					len ++;
				}
			}
			else if (ix > 0 && ix < 7 && addr->s6_addr16[ix] == 0)
				zero = true;
			else {
				if (zero) {
					omit = true;
					str_chr(text, &pos, size, ':');
					len ++;
				}
				len += str_num(text, &pos, size, ntohs(addr->s6_addr16[ix]), 16, radhex, 0, false, ' '); 
				if (ix < 7) {
					str_chr(text, &pos, size, ':');
					len ++;
				}
			}
		}

		if (len6 == sizeof(T_IN6_ADDR) / 2 - 2) {
			T_IN4_ADDR ip4addr;

			ip4addr = ntohl(addr->s6_addr32[3]);
			len += str_ipv4addr(&text[len], size - len, &ip4addr, 0);
		}

		for ( ; len < width; len ++)
			str_chr(text, &pos, size, ' ');
	}
	return len;
}

/*
 *  str_macaddr -- MAC アドレス出力
 */

int
str_macaddr (char *text, int size, uint8_t *mac, int width)
{
	int oct, len, pos = 0;

	for (oct = 5; oct -- > 0; ) {
		str_num(text, &pos, size, *mac ++, 16, radhex, 2, false, '0'); 
		str_chr(text, &pos, size, ':');
	}
	str_num(text, &pos, size, *mac, 16, radhex, 2, false, '0'); 

	for (len = 17; len < width; len ++)
		str_chr(text, &pos, size, ' ');

	return len;
}

int usrcmd_ping(int argc, char **argv)
{
	int tmo = 3, size = 64;
	char apip = DEFAULT_API_PROTO;
	char *line = argv[1];
#if defined(SUPPORT_INET6)
	T_IN6_ADDR	addr;
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	addr4;
#endif
#else
	T_IN4_ADDR	addr;
#endif
	static const char i6rlp_pmtu_str1[] = " FF1E::1:2 1 1452";
	static const char i6rlp_pmtu_str2[] = " FF1E::1:2 1 1352";
	static const char i6rlp_pmtu_str3[] = " fe80::0200:00ff:fe00:0100 1 2";

	if (apip == '1')
		ntlibc_strcpy(line, i6rlp_pmtu_str1);
	else if (apip == '2')
		ntlibc_strcpy(line, i6rlp_pmtu_str2);
	else if (apip == '3')
		ntlibc_strcpy(line, i6rlp_pmtu_str3);

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	if (argc <= 2)
		return 0;

	if ('0' <= *line && *line <= '9') {
		if (*line == '6')
			apip = API_PROTO_IPV6;
		if (*line == '4')
			apip = API_PROTO_IPV4;
		line++;
	}

	line = argv[2];
#else
	if (argc <= 1)
		return 0;
#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	if ((line = lookup_ipaddr(&addr, line, apip)) == NULL) {
		printf("[PING] unknown host.\n");
		return 0;
	}

	if (argc > 2) {
		line = argv[2];
		if ('0' <= *line && *line <= '9')
			line = get_int(&tmo, line);

		if (argc > 3) {
			line = argv[3];
			if ('0' <= *line && *line <= '9')
				line = get_int(&size, line);
		}
	}

#if defined(SUPPORT_INET6)
#if defined(SUPPORT_INET4)
	if (apip == API_PROTO_IPV6) {
		printf("[PING6] size: %d, tmo: %d, host: %s\n", size, tmo, ipv62str(NULL, &addr));
		ping6(&addr, (uint_t)tmo, (uint_t)size);
	}
	else {
		addr4 = ntohl(addr.s6_addr32[3]);
		printf("[PING4] size: %d, tmo: %d, host: %s\n", size, tmo, ip2str(NULL, &addr4));
		ping4(&addr4, (uint_t)tmo, (uint_t)size);
	}
#else /* of #if defined(SUPPORT_INET4) */
	printf("[PING6] size: %d, tmo: %d, host: %s\n", size, tmo, ipv62str(NULL, &addr));
	ping6(&addr, (uint_t)tmo, (uint_t)size);
#endif	/* of #if defined(SUPPORT_INET4) */
#else	/* of #if defined(SUPPORT_INET6) */
	printf("[PING4] size: %d, tmo: %d, host: %s\n", size, tmo, ip2str(NULL, &addr));
	ping4(&addr, (uint_t)tmo, (uint_t)size);
#endif	/* of #if defined(SUPPORT_INET6) */

	return 0;
}

static void dhcp4c_info()
{
	T_IN4_ADDR svaddr;
	SYSTIM bind_start;
	ER ret;
	uint32_t expire, renew, rebind;
	char temp[30];
	int pos;

	if ((ret = dhcp4c_get_info(&svaddr, &expire, &renew, &rebind, &bind_start)) == E_OK) {
		pos = str_ipv4addr(temp, sizeof(temp), &svaddr, 0);
		temp[pos] = '\0';
		printf("DHCPv4 server: %s,\n", temp);
		printf("  Renew:       %u:%02u:%02u,\n",
			renew / 3600, (renew / 60) % 60, renew % 60);
		printf("  Rebind:      %u:%02u:%02u, Expire:   %u:%02u:%02u.\n",
			rebind / 3600, (rebind / 60) % 60, rebind % 60,
			expire / 3600, (expire / 60) % 60, expire % 60);
	}
	else if (ret == E_OBJ)
		printf("DHCPv4 server: not available.\n");
}

int usrcmd_dhcp4c(int argc, char **argv)
{
	ER ret;

	if (argc < 2)
		return 0;

	if (ntlibc_strcmp(argv[1], "rel") == 0) {
		ret = dhcp4c_rel_info();
		printf("dhcp4c_rel_info %d\n", ret);
	}
	else if (ntlibc_strcmp(argv[1], "renew") == 0) {
		ret = dhcp4c_renew_info();
		printf("dhcp4c_renew_info %d\n", ret);
	}
	else {
		dhcp4c_info();
	}

	return 0;
}

/*
*  s_show_dns_domain_name -- DNS のドメイン名を表示する。
*/

static uint_t
s_show_dns_domain_name(uint8_t *hdr, uint_t offset)
{
	uint8_t	*ptr;
	uint_t	c;

	ptr = hdr + offset;
	while (*ptr) {
		if ((*ptr & DNS_MSG_COMP_MARK) == DNS_MSG_COMP_MARK) {
			s_show_dns_domain_name(hdr, (*ptr & ~DNS_MSG_COMP_MARK) << 8 | *(ptr + 1));
			ptr += 2;
			break;
		}
		else {
			for (c = 1; c <= *ptr; c++)
				printf("%c", *(ptr + c));
			ptr += *ptr + 1;
			if (*ptr)
				printf(".");
		}
	}
	return ptr - hdr;
}

/*
 *  show_dns_soa -- DNS の SOA RDATA を表示する。
 */

static ER_UINT
show_dns_soa(uint8_t *msg, ER_UINT length, uint_t offset)
{
	T_RSLV_SOA	soa;
	ER_UINT		error;
	uint_t		rn_offset;

	if ((error = dns_analyze_soa(&soa, offset, msg, length)) < 0)
		return error;

	printf("    mname:   ");
	rn_offset = s_show_dns_domain_name(msg, offset);
	putchar('\n');
	printf("    rname:   ");
	s_show_dns_domain_name(msg, rn_offset);
	putchar('\n');

	printf("    serial:  %d\n", soa.serial);
	printf("    refresh: %d\n", soa.refresh);
	printf("    retry:   %d\n", soa.retry);
	printf("    expirel: %d\n", soa.expire);
	printf("    minimum: %d\n", soa.minimum);

	return E_OK;
}

/*
 *  show_dns_qdsection -- DNS の Question セクションを表示する。
 */

static ER_UINT
show_dns_qdsection(uint8_t *msg, ER_UINT length, T_RSLV_DNS_MSG *rslv)
{
	T_RSLV_QD	qd;
	ER_UINT		offset, error;
	int		scount;

	printf("question   section: %d\n", rslv->dns_hdr.qdcount);
	offset = rslv->qd_offset;
	for (scount = 1; scount <= rslv->dns_hdr.qdcount; scount++) {
		if ((error = dns_analyze_qd(&qd, offset, msg, length)) < 0)
			return error;

		printf("%2d: ", scount);
		s_show_dns_domain_name(msg, offset);
		printf("\n    type: %-4s, class: %2s\n", dns_strtype(qd.type), dns_strclass(qd.class));
		offset = error;
	}

	return E_OK;
}

/*
 *  show_dns_section -- DNS の各セクションを表示する。
 */

static ER_UINT
show_dns_section(uint8_t *msg, ER_UINT length, uint_t scount, uint_t offset, char *title)
{
	T_RSLV_RR	rr;
	T_IN4_ADDR	in4_addr;
	ER_UINT		error;
	int		count, dcount, col;
	T_IN6_ADDR	in6_addr;
	char temp[30];
	int pos;

	printf("%10s section: %d\n", title, scount);
	for (count = 1; count <= scount; count++) {
		if ((error = dns_analyze_rr(&rr, offset, msg, length)) < 0)
			return error;

		printf("%2d: ", count);
		s_show_dns_domain_name(msg, offset);
		printf("\n    type: %-4s, class: %2s, TTL: %2d, len: %3d, offset: 0x%02x\n",
			dns_strtype(rr.type), dns_strclass(rr.class), rr.ttl, rr.rdlength, rr.rdata_offset);

		switch (rr.type) {
		case DNS_TYPE_A:
			memcpy((void*)&in4_addr, (void*)(msg + rr.rdata_offset), sizeof(in4_addr));
			in4_addr = ntohl(in4_addr);
			pos = str_ipv4addr(temp, sizeof(temp), &in4_addr, 0);
			temp[pos] = '\0';
			printf("    IPv4 addr: %s\n", temp);
			break;
		case DNS_TYPE_NS:
			printf("    host: ");
			s_show_dns_domain_name(msg, rr.rdata_offset);
			putchar('\n');
			break;
		case DNS_TYPE_CNAME:
			printf("    host: ");
			s_show_dns_domain_name(msg, rr.rdata_offset);
			putchar('\n');
			break;
		case DNS_TYPE_SOA:
			show_dns_soa(msg, length, rr.rdata_offset);
			break;
		case DNS_TYPE_PTR:
			printf("     PTR: ");
			s_show_dns_domain_name(msg, rr.rdata_offset);
			putchar('\n');
			break;
		case DNS_TYPE_AAAA:
			memcpy((void*)&in6_addr, (void*)(msg + rr.rdata_offset), sizeof(in6_addr));
			pos = str_ipv6addr(temp, sizeof(temp), &in6_addr, 0);
			temp[pos] = '\0';
			printf("    IPv6 addr: %s\n", temp);
			break;
		default:
			printf("    data: ");
			col = 32;
			for (dcount = 0; dcount < rr.rdlength; dcount++) {
				printf("%02x", *(msg + rr.rdata_offset + dcount));
				if (--col == 0) {
					printf("\n          ");
					col = 32;
				}
			}
			putchar('\n');
			break;
		}
	}

	return E_OK;
}
/*
 *  dns_info -- DNS 情報の表示
 */

static void dns_info()
{
#if defined(SUPPORT_INET6)
	T_IN6_ADDR	in6_addr;
#endif
#if defined(SUPPORT_INET4)
	T_IN4_ADDR	in4_addr;
#endif
	char temp[30];
	int pos;

#if defined(SUPPORT_INET6)

	printf("domain name:     %s\n", dns_in6_get_dname());

#else	/* of #if defined(SUPPORT_INET6) */

	printf("domain name:     %s\n", dns_in4_get_dname());

#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET6)
	dns_in6_get_addr(&in6_addr);
	printf("IPv6 DNS server: ");
	if (IN6_IS_ADDR_UNSPECIFIED(&in6_addr))
		printf("not available.\n");
	else {
		pos = str_ipv6addr(temp, sizeof(temp), &in6_addr, 0);
		temp[pos] = '\0';
		printf("%s.\n", temp);
	}
#endif	/* of #if defined(SUPPORT_INET6) */

#if defined(SUPPORT_INET4)
	dns_in4_get_addr(&in4_addr);
	printf("IPv4 DNS server: ");
	if (in4_addr == IPV4_ADDRANY)
		printf("not available.\n");
	else {
		pos = str_ipv4addr(temp, sizeof(temp), &in4_addr, 0);
		temp[pos] = '\0';
		printf("%s.\n", temp);
	}
#endif	/* of #if defined(SUPPORT_INET4) */
}

/*
 *  name_lookup -- ホスト名－IP アドレス変換
 */

const char *rcode_str[] = {
	"no error",
	"format error",
	"server error",
	"name error",
	"not implement",
	"refused",
};

int usrcmd_dnsc(int argc, char **argv)
{
	char *line = argv[1];
	static char hostname[DBG_LINE_SIZE + 1];

	T_RSLV_DNS_MSG	rslv;
	ER_UINT		length, offset;
	ER		error;
	uint_t		flags = 0;
	uint8_t 	*msg;

	if (ntlibc_strcmp(line, "info") == 0) {
		dns_info();
		return 0;
	}

	/* コマンドのオプションを設定する。*/
	line = skip_blanks(resolv_options(&flags, line, DEFAULT_API_PROTO));
	if ((flags & (DNS_LUP_FLAGS_PROTO_IPV6 | DNS_LUP_FLAGS_PROTO_IPV4)) == 0) {
		printf("DNS server not available.\n");
		return 0;
	}

	/* 照会するホスト名・IP アドレスを解析する。*/
	resolv_hoststr(&flags, hostname, sizeof(hostname), line);

	/* 正引きでも逆引きでもプロトコル上は正引きを指定する。*/
	flags |= DNS_LUP_OPCODE_FORWARD;

	/* IPv6 アドレス、または IPv4 アドレスが指定された時は、照会タイプは PTR に設定する。*/
	if (((flags & DNS_LUP_FLAGS_NAME_MASK) == DNS_LUP_FLAGS_NAME_IPV6) ||
		((flags & DNS_LUP_FLAGS_NAME_MASK) == DNS_LUP_FLAGS_NAME_IPV4))
		flags = (flags & ~DNS_LUP_FLAGS_QTYPE_MASK) | DNS_LUP_FLAGS_QTYPE_PTR;

	if ((error = tget_mpf(MPF_RSLV_SRBUF, (void*)&msg, TMO_FEVR)) != E_OK) {
		printf("get buffer error: %s.\n", itron_strerror(error));
		return 0;
	}

	if ((length = dns_lookup_host(flags | DNS_LUP_FLAGS_MSG, line, msg, DNS_UDP_MSG_LENGTH, &rslv)) < 0) {
		//printf("error: %s.\n", itron_strerror(length));
		goto err_ret;
	}

	dly_tsk(1 * 1000);
	printf("DNS header: flags: ");
	if (rslv.dns_hdr.code & (DNS_QR_RESPONSE | DNS_AUTHORITATIVE |
		DNS_TRUN_CATION | DNS_RECURSION_DESIRED | DNS_RECURSION_AVAILABLE)) {
		printf((rslv.dns_hdr.code & DNS_QR_RESPONSE) ? "QR," : "");
		printf((rslv.dns_hdr.code & DNS_AUTHORITATIVE) ? "AA," : "");
		printf((rslv.dns_hdr.code & DNS_TRUN_CATION) ? "TC," : "");
		printf((rslv.dns_hdr.code & DNS_RECURSION_DESIRED) ? "RD," : "");
		printf((rslv.dns_hdr.code & DNS_RECURSION_AVAILABLE) ? "RA," : "");
		printf(" ");
	}
	printf("opcode: ");
	printf((rslv.dns_hdr.code & DNS_OPCODE_REVERSE) ? "RV" : "FW");
	printf((rslv.dns_hdr.code & DNS_OPCODE_STATUS) ? ",ST" : "");
	printf(", rcode: %s.\n",
		(rslv.dns_hdr.code & DNS_RCODE_MASK) > DNS_RCODE_REFUSED
		? "6" : rcode_str[rslv.dns_hdr.code & DNS_RCODE_MASK]);

	if ((offset = show_dns_qdsection(msg, length, &rslv)) < 0) {
		printf("msg error: %s.\n", itron_strerror(offset));
	}
	if ((offset = show_dns_section(msg, length, rslv.dns_hdr.ancount, rslv.an_offset, "answer")) < 0) {
		printf("msg error: %s.\n", itron_strerror(offset));
	}
	if ((offset = show_dns_section(msg, length, rslv.dns_hdr.nscount, rslv.ns_offset, "authority")) < 0) {
		printf("msg error: %s.\n", itron_strerror(offset));
	}
	if ((offset = show_dns_section(msg, length, rslv.dns_hdr.arcount, rslv.ar_offset, "additional")) < 0) {
		printf("msg error: %s.\n", itron_strerror(offset));
	}

err_ret:
	if ((error = rel_mpf(MPF_RSLV_SRBUF, msg)) != E_OK)
		printf("release buffer error: %s.\n", itron_strerror(error));
	return 0;
}

int usrcmd_ntpc(int argc, char **argv)
{
	ntp_cli_execute();
	return 0;
}
