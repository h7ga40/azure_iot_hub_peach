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

/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
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
 *
 *	From: @(#)tcp_usrreq.c	8.2 (Berkeley) 1/3/94
 * $FreeBSD: src/sys/netinet/tcp_usrreq.c,v 1.40.2.3 1999/08/29 16:29:57 peter Exp $
 */

#ifdef TCP_CFG_EXTENTIONS

#if (API_PROTO == API_PROTO_IPV6 && TNUM_TCP6_REPID > 0) || (API_PROTO == API_PROTO_IPV4 && TNUM_TCP4_REPID > 0)

/*
 *  tcp_cre_rep -- TCP 受付口の生成【拡張機能】
 */

#ifdef __tcp_cre_rep

ER
TCP_CRE_REP (ID repid, T_TCPN_CREP *pk_crep)
{
	T_TCP_REP	*rep;
	ER		error;

	/* TCP 受付口 ID をチェックする。*/
	if (!VALID_TCP_REPID(repid))
		return E_ID;

	/* pk_crep が NULL ならエラー */
	if (pk_crep == NULL)
		return E_PAR;

#if (API_PROTO == API_PROTO_IPV6) && !defined(API_CFG_IP4MAPPED_ADDR)

	/*
	 * API が IPv6 で IPv4 射影アドレスが認められていないのにもかかわらず、
	 * IPv4 射影アドレスが指定されたらエラー
	 */
	if (in6_is_addr_ipv4mapped(&pk_crep->myaddr.ipaddr))
		return E_PAR;

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) && !defined(DAPI_CFG_IP4MAPPED_ADDR) */

	/* TCP 受付口を得る。*/
	rep = GET_TCP_REP(repid);

	/* TCP 受付口が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_REP(rep))
		return E_ID;

	/* 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/*
	 * TCP 受付口をチェックする。生成済みであればエラー
	 */
	if (VALID_TCP_REP(rep))
		error = E_OBJ;
	else {

		/* TCP 受付口生成情報をコピーする。*/
		rep->repatr = pk_crep->repatr;			/* 受付口属性		*/
		rep->myaddr.ipaddr = pk_crep->myaddr.ipaddr;	/* 自分のアドレス	*/
		rep->myaddr.portno = pk_crep->myaddr.portno;	/* 自分のアドレス	*/

		/* TCP 受付口を生成済みにする。*/
		rep->flags |= TCP_REP_FLG_VALID;
		error = E_OK;
		}

	/* 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

	return error;
	}

#endif	/* of #ifdef __tcp_cre_rep */

#ifdef __tcp_del_rep

/*
 *  tcp_find_cep_rep -- TCP 受付口をリンクしている TCP 通信端点を得る。
 */

static T_TCP_CEP*
TCP_FIND_CEP_REP (T_TCP_REP* rep)
{
	T_TCP_CEP*	cep;

	for (cep = &tcp_cep[tmax_tcp_cepid]; cep -- != tcp_cep; ) {

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4)
		if (cep->rep4 == rep)
#else
		if (cep->rep == rep)
#endif
			return cep;
		}

	return NULL;
	}

/*
 *  tcp_del_rep_body -- TCP 受付口の削除【拡張機能】の本体
 */

static ER
TCP_DEL_REP_BODY (ID repid)
{
	T_TCP_CEP	*cep;
	T_TCP_REP	*rep;
	ER		error = E_OK;

	/* TCP 受付口 ID のチェックは終了している。*/

	/* TCP 受付口を得る。*/
	rep = GET_TCP_REP(repid);

	/* TCP 受付口が、動的生成用でなければエラー */
	if (!DYNAMIC_TCP_REP(rep))
		return E_ID;

	/* 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/* TCP 受付口をチェックする。未生成の場合はエラー */
	if (!VALID_TCP_REP(rep))
		error = E_NOEXS;
	else {
		if ((cep = TCP_FIND_CEP_REP(rep)) != NULL) {
			/*
			 * すでに受動オープンしているとき
			 *（tcp_acp_cep が呼ばれているとき）は、
			 * tcp_acp_cep を終了させる。
			 */

			/* 通信端点をロックする。*/
			syscall(wai_sem(cep->semid_lock));

			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
			cep->rep4 = NULL;
#endif

			cep->fsm_state = TCP_FSM_CLOSED;
			cep->error = E_DLT;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));

#ifdef TCP_CFG_NON_BLOCKING

			if (cep->rcv_nblk_tfn != TFN_TCP_UNDEF) {
				if (IS_PTR_DEFINED(cep->callback)) {

#ifdef TCP_CFG_NON_BLOCKING_COMPAT14

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)E_DLT);

#else	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					ER	error = E_DLT;

					(*cep->callback)(GET_TCP_CEPID(cep), cep->rcv_nblk_tfn, (void*)&error);

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING_COMPAT14 */

					}
				else
					error = E_OBJ;
				cep->rcv_nblk_tfn = TFN_TCP_UNDEF;
				}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

			cep->rcv_tskid = TA_NULL;
			cep->rcv_tfn   = TFN_TCP_UNDEF;

			/* 通信端点のロックを解除する。*/
			syscall(sig_sem(cep->semid_lock));

			}
		else
			error = E_OK;

		/* TCP 受付口を未生成にする。*/
		rep->flags &= ~TCP_REP_FLG_VALID;
		}

	/* 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

	return error;
	}

#endif	/* of #ifdef __tcp_del_rep */

#endif	/* of #if (API_PROTO == API_PROTO_IPV6 && TNUM_TCP6_REPID > 0) || (API_PROTO == API_PROTO_IPV4 && TNUM_TCP4_REPID > 0) */

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

#if (API_PROTO == API_PROTO_IPV6 && TNUM_TCP6_REPID > 0) || (API_PROTO == API_PROTO_IPV4 && TNUM_TCP4_REPID > 0)

/*
 *  tcp_acp_cep -- 接続要求待ち (受動オープン)【標準機能】
 */

#ifdef __tcp_acp_cep

ER
TCP_ACP_CEP (ID cepid, ID repid, T_IPEP *p_dstaddr, TMO tmout)
{
	T_TCP_REP	*rep;
	T_TCP_CEP	*cep;
	ER		error;
	FLGPTN		flag;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	/*
	 * API (tcp6_acp_cep と tcp_acp_cep) と、
	 * TCP通信端点のプロトコルが矛盾していればエラー
	 */

#if API_PROTO == API_PROTO_IPV6

	if (GET_TCP_CEP(cepid)->flags & TCP_CEP_FLG_IPV4)
		return E_ID;

#endif	/* of #if API_PROTO == API_PROTO_IPV6 */

#if API_PROTO == API_PROTO_IPV4

	if ((GET_TCP_CEP(cepid)->flags & TCP_CEP_FLG_IPV4) == 0)
		return E_ID;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	/* TCP 受付口をチェックする。*/
	if (!VALID_TCP_REPID(repid))
		return E_ID;

#ifdef TCP_CFG_NON_BLOCKING

	/* p_dstaddr が NULL ならエラー */
	if (p_dstaddr == NULL)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/* p_dstaddr が NULL か、tmout が TMO_NBLK ならエラー */
	if (p_dstaddr == NULL || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

#if (API_PROTO == API_PROTO_IPV6) && !defined(API_CFG_IP4MAPPED_ADDR)

	/*
	 * API が IPv6 で IPv4 射影アドレスが認められていないのにもかかわらず、
	 * IPv4 射影アドレスが指定されたらエラー
	 */
	if (in6_is_addr_ipv4mapped(&p_dstaddr->ipaddr))
		return E_PAR;

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) && !defined(DAPI_CFG_IP4MAPPED_ADDR) */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_ACP_CEP)) != E_OK)
		return error;

	/* CEP の FSM がクローズ状態でなければエラー。*/
	if (cep->fsm_state != TCP_FSM_CLOSED) {
		error = E_OBJ;
		goto err_ret;
		}
	syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_CLOSED)));

	/* TCP 通信端点を初期化する。*/
	tcp_init_cep(cep);

	/* TCP 受付口を得る。*/

	rep = GET_TCP_REP(repid);

#ifdef TCP_CFG_EXTENTIONS

	/* TCP 受付口をロックする。*/
	syscall(wai_sem(rep->semid_lock));

	/* TCP 受付口をチェックする。*/
	if (!VALID_TCP_REP(rep)) {
		syscall(sig_sem(rep->semid_lock));
		error = E_NOEXS;
		goto err_ret;
		}

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	/* TCP 通信端点にTCP受付口を記録する。*/

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

#if API_PROTO == API_PROTO_IPV6

	cep->rep = rep;

#else	/* of #if API_PROTO == API_PROTO_IPV6 */

	cep->rep4 = rep;

#endif	/*of #if API_PROTO == API_PROTO_IPV6 */

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	cep->rep = rep;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

#ifdef TCP_CFG_EXTENTIONS

	/* TCP 受付口のロックを解除する。*/
	syscall(sig_sem(rep->semid_lock));

#endif	/* of #ifdef TCP_CFG_EXTENTIONS */

	/* TCP 受付口のアドレスをコピーする。*/

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4)

	if (rep->myaddr.ipaddr == IPV4_ADDRANY)
		memcpy(&cep->myaddr.ipaddr, &in6_addr_unspecified,
		                      sizeof(in6_addr_unspecified));
	else
		in6_make_ipv4mapped(&cep->myaddr.ipaddr, rep->myaddr.ipaddr);
	cep->myaddr.portno = rep->myaddr.portno;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

	cep->myaddr = rep->myaddr;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

#if API_PROTO == API_PROTO_IPV4

	/* TCP 通信端点のネットワーク層プロトコルを設定する。*/
	cep->flags |= TCP_CEP_FLG_IPV4;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

	/* 通信端点を設定する。*/
	cep->fsm_state = TCP_FSM_LISTEN;

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {
		/* ノンブロッキングコール */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

#if API_PROTO == API_PROTO_IPV4

		cep->p_dstaddr4   = p_dstaddr;
		cep->p_dstaddr    = NULL;

#else	/* of #if API_PROTO == API_PROTO_IPV4 */

		cep->p_dstaddr4   = NULL;
		cep->p_dstaddr    = p_dstaddr;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		cep->p_dstaddr    = p_dstaddr;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		cep->rcv_nblk_tfn = TFN_TCP_ACP_CEP;
		return E_WBLK;
		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/*
		 *  FSM が ESTABLISHED になるまで待つ。
		 *  FSM が CLOSED になった場合は、エラーが発生したことを意味している。
		 */
		error = twai_flg(cep->est_flgid, (TCP_CEP_EVT_CLOSED |
		                                  TCP_CEP_EVT_ESTABLISHED), TWF_ORW, &flag, tmout);
		if (error == E_OK) {
			if (cep->error != E_OK)
				error = cep->error;
			else if (cep->fsm_state == TCP_FSM_CLOSED)
				error = E_TMOUT;
			}

		syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_ESTABLISHED)));

		if (error == E_OK) {
			/* 相手のアドレスをコピーする。*/
#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4)
			p_dstaddr->ipaddr = ntohl(cep->dstaddr.ipaddr.s6_addr32[3]);
			p_dstaddr->portno = cep->dstaddr.portno;
#else
			*p_dstaddr = cep->dstaddr;
#endif
			}
		else {
			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
			cep->rep4 = NULL;
#endif

			cep->fsm_state = TCP_FSM_CLOSED;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->rcv_tskid = TA_NULL;
	cep->rcv_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_acp_cep */

#endif	/* of #if (API_PROTO == API_PROTO_IPV6 && TNUM_TCP6_REPID > 0) || (API_PROTO == API_PROTO_IPV4 && TNUM_TCP4_REPID > 0) */

/*
 *  tcp_con_cep -- 接続要求 (能動オープン)【標準機能】
 */

#ifdef __tcp_con_cep

ER
TCP_CON_CEP (ID cepid, T_IPEP *p_myaddr, T_IPEP *p_dstaddr, TMO tmout)
{
	T_TCP_CEP	*cep;
	ER		error;
	FLGPTN		flag;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

	/*
	 * API (tcp6_acp_cep と tcp_acp_cep) と、
	 * TCP通信端点のプロトコルが矛盾していればエラー
	 */

#if API_PROTO == API_PROTO_IPV6

	if (GET_TCP_CEP(cepid)->flags & TCP_CEP_FLG_IPV4)
		return E_ID;

#endif	/* of #if API_PROTO == API_PROTO_IPV6 */

#if API_PROTO == API_PROTO_IPV4

	if ((GET_TCP_CEP(cepid)->flags & TCP_CEP_FLG_IPV4) == 0)
		return E_ID;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

	/*
	 *  あて先がマルチキャストアドレスならエラー
	 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4)

	if (IN4_IS_ADDR_MULTICAST(p_dstaddr->ipaddr))
		return E_PAR;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

	if (IN_IS_ADDR_MULTICAST(&p_dstaddr->ipaddr))
		return E_PAR;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

#if (API_PROTO == API_PROTO_IPV6) && !defined(API_CFG_IP4MAPPED_ADDR)

	/*
	 * API が IPv6 で IPv4 射影アドレスが認められていないのにもかかわらず、
	 * IPv4 射影アドレスが指定されたらエラー
	 */
	if (in6_is_addr_ipv4mapped(&p_dstaddr->ipaddr))
		return E_PAR;

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) && !defined(DAPI_CFG_IP4MAPPED_ADDR) */

#ifdef TCP_CFG_NON_BLOCKING

	/*
	 *  p_dstaddr または p_myaddr が NULL ならエラー
	 */
	if (p_myaddr == NULL || p_dstaddr == NULL)
		return E_PAR;

#else	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  p_dstaddr または p_myaddr が NULL 、
	 *  tmout が TMO_NBLK ならエラー
	 */
	if (p_myaddr == NULL || p_dstaddr == NULL || tmout == TMO_NBLK)
		return E_PAR;

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

	/*
	 *  CEP をロックし、API 機能コードとタスク識別子を記録する。
	 *  すでに記録されていれば、ペンディング中なのでエラー
	 */
	if ((error = tcp_lock_cep(cepid, &cep, TFN_TCP_CON_CEP)) != E_OK)
		return error;

	/* CEP の FSM がクローズ状態でなければエラー。*/
	if (cep->fsm_state != TCP_FSM_CLOSED) {
		error = E_OBJ;
		goto err_ret;
		}
	syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_CLOSED)));

	/* シーケンス番号を初期化する。*/
	if (tcp_iss == 0)
		tcp_init_iss();

	/* 通信端点を初期化する。*/
	tcp_init_cep(cep);

	/*
	 *  IP アドレスを設定する。
	 *  p_myaddr が NADR (-1) ではなく、自 IP アドレスが ANY でなければ、
	 *  指定された IP アドレスを割り当てる。
	 */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4)

	if (p_myaddr != NADR && !IN4_IS_ADDR_ANY(&p_myaddr->ipaddr))
		in6_make_ipv4mapped(&cep->myaddr.ipaddr, p_myaddr->ipaddr);
	else
		in6_make_ipv4mapped(&cep->myaddr.ipaddr, IF_GET_IFNET()->in4_ifaddr.addr);

	in6_make_ipv4mapped(&cep->dstaddr.ipaddr, p_dstaddr->ipaddr);
	cep->dstaddr.portno = p_dstaddr->portno;

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

	if (p_myaddr != NADR && !IN_IS_ADDR_ANY(&p_myaddr->ipaddr))
		cep->myaddr.ipaddr = p_myaddr->ipaddr;
	else {
		if (IN_ADDRWITHIFP(IF_GET_IFNET(), &cep->myaddr.ipaddr, &p_dstaddr->ipaddr) == NULL) {
			error = E_PAR;
			goto err_ret;
			}
		}

	cep->dstaddr   = *p_dstaddr;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) && (API_PROTO == API_PROTO_IPV4) */

#if API_PROTO == API_PROTO_IPV4

	/* TCP 通信端点のネットワーク層プロトコルを設定する。*/
	cep->flags |= TCP_CEP_FLG_IPV4;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

	/* 通信端点を設定する。*/
	//NET_DEBUG_TCP5("tcp_con_cep3[c=%d,d=%lI.%d,s=%lI.%d]\n",
	cep->fsm_state = TCP_FSM_SYN_SENT;
	cep->iss       = tcp_iss;
	cep->timer[TCP_TIM_KEEP] = TCP_TVAL_KEEP_INIT;
	tcp_iss += TCP_ISS_INCR() / 2;
	init_send_seq(cep);

#ifdef TCP_CFG_NON_BLOCKING

	/* タイムアウトをチェックする。*/
	if (tmout == TMO_NBLK) {
		/* ノンブロッキングコール */

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)

#if API_PROTO == API_PROTO_IPV4

		cep->p_dstaddr4   = p_dstaddr;
		cep->p_myaddr4    = p_myaddr;
		cep->p_dstaddr    = NULL;
		cep->p_myaddr     = NADR;

#else	/* of #if API_PROTO == API_PROTO_IPV4 */

		cep->p_dstaddr4   = NULL;
		cep->p_myaddr4    = NADR;
		cep->p_dstaddr    = p_dstaddr;
		cep->p_myaddr     = p_myaddr;

#endif	/* of #if API_PROTO == API_PROTO_IPV4 */

#else	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		cep->p_dstaddr    = p_dstaddr;
		cep->p_myaddr     = p_myaddr;

#endif	/* of #if defined(SUPPORT_INET6) && defined(SUPPORT_INET4) */

		cep->snd_nblk_tfn = TFN_TCP_CON_CEP;

		/* コネクションの開設をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);
		return E_WBLK;
		}
	else {

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

		/*
		 *  p_myaddr が NADR (-1) か、
		 *  自ポート番号が TCP_PORTANY なら、自動で割り当てる。
		 */
		if (p_myaddr == NADR || p_myaddr->portno == TCP_PORTANY)
			tcp_alloc_auto_port(cep);
		else if ((error = tcp_alloc_port(cep, p_myaddr->portno)) != E_OK)
			goto err_ret;

		/* コネクションの開設をポストする。*/
		cep->flags |= TCP_CEP_FLG_POST_OUTPUT;
		sig_sem(SEM_TCP_POST_OUTPUT);

		/*
		 *  イベントが ESTABLISHED になるまで待つ。
		 *  イベントが CLOSED になった場合は、何らかのエラーが発生したか、
		 *  接続要求が拒否されたことを意味している。
		 */
		error = twai_flg(cep->est_flgid, (TCP_CEP_EVT_CLOSED |
		                                  TCP_CEP_EVT_ESTABLISHED), TWF_ORW, &flag, tmout);
		if (error == E_OK) {
			if (cep->error != E_OK)
				error = cep->error;
			else if (cep->fsm_state == TCP_FSM_CLOSED)
				error = E_CLS;
			}

		syscall(clr_flg(cep->est_flgid, (FLGPTN)(~TCP_CEP_EVT_ESTABLISHED)));

		if (error != E_OK) {
			/*
			 *  通信端点から受付口を解放し、
			 *  イベントフラグをクローズに設定する。
			 */
			cep->rep = NULL;

#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
			cep->rep4 = NULL;
#endif

			cep->fsm_state = TCP_FSM_CLOSED;
			syscall(set_flg(cep->est_flgid, TCP_CEP_EVT_CLOSED));
			}

#ifdef TCP_CFG_NON_BLOCKING

		}

#endif	/* of #ifdef TCP_CFG_NON_BLOCKING */

err_ret:
	cep->snd_tskid = TA_NULL;
	cep->snd_tfn   = TFN_TCP_UNDEF;
	return error;
	}

#endif	/* of #ifdef __tcp_con_cep */
