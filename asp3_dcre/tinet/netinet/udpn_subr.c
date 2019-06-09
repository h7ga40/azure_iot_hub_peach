/*
 *  TINET (UDP/IP Protocol Stack)
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

/*
 * Copyright (c) 1982, 1986, 1988, 1990, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must ceproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 *  udp_alloc_auto_port -- 自動割り当てポート番号を設定する。
 */

ER
UDP_ALLOC_AUTO_PORT (T_UDP_CEP *cep)
{
	int_t	ix;
	uint16_t	portno, portno_start;

	portno_start = udp_port_auto;
	do {
		portno = udp_port_auto ++;
		if (udp_port_auto > UDP_PORT_LAST_AUTO)
			udp_port_auto = UDP_PORT_FIRST_AUTO;

		syscall(wai_sem(SEM_UDP_CEP));
		for (ix = TMAX_UDP_CEPID; ix -- > 0; ) {
			if (VALID_UDP_CEP(&UDP_CEP[ix]) && UDP_CEP[ix].myaddr.portno == portno) {
				portno = UDP_PORTANY;
				break;
				}
			}

		if (portno != UDP_PORTANY) {
			cep->myaddr.portno = portno;
			syscall(sig_sem(SEM_UDP_CEP));
			return E_OK;
			}
		syscall(sig_sem(SEM_UDP_CEP));

		} while (portno_start != udp_port_auto);

	return E_NOID;
	}

/*
 *  udp_alloc_port -- 指定されたポート番号を設定する。
 */

ER
UDP_ALLOC_PORT (T_UDP_CEP *cep, uint16_t portno)
{
	int_t	ix;

	syscall(wai_sem(SEM_UDP_CEP));
	for (ix = TMAX_UDP_CEPID; ix -- > 0; )
		if (VALID_UDP_CEP(&UDP_CEP[ix]) && UDP_CEP[ix].myaddr.portno == portno) {
			syscall(sig_sem(SEM_UDP_CEP));
			return E_PAR;
			}
	cep->myaddr.portno = portno;
	syscall(sig_sem(SEM_UDP_CEP));
	return E_OK;
	}

/*
 *  udp_find_cep -- ポート番号から UDP 通信端点を得る。
 */

T_UDP_CEP*
UDP_FIND_CEP (T_NET_BUF *input, uint_t off)
{
	T_UDP_CEP*	cep;
	T_UDP_HDR	*udph;

	udph = GET_UDP_HDR(input, off);
	
	/*
	 *  IP アドレスとポート番号が一致する通信端点を探索する。
	 */
	for (cep = &UDP_CEP[TMAX_UDP_CEPID]; cep -- != UDP_CEP; ) {
		if (VALID_UDP_CEP(cep)                                &&
		    UDP_IS_DSTADDR_ACCEPT(&cep->myaddr.ipaddr, input) &&
		    ntohs(udph->dport) == cep->myaddr.portno)
			return cep;
		}

	return NULL;
	}

/*
 *  udp_notify -- ICMP エラーの処理
 */

void
UDP_NOTIFY (T_NET_BUF *input, ER error)
{
	if (UDP_FIND_CEP(input, GET_UDP_HDR_OFFSET(input)) != NULL) {
		syslog(LOG_NOTICE, "[UDP] error, code: %d.", error);
		}
	}
