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
 *  ネットワーク応用プログラムサポートルーチン
 */

#ifdef USE_TCP_EXTENTIONS

/*
 *  alloc_tcp_rep -- TCP 受付口を獲得する。
 */

ER
ALLOC_TCP_REP (ID *repid, ID tskid, T_TCPN_CREP *crep)
{
	int_t	ix;

	*repid = TCP_REP_NONE;
	syscall(wai_sem(SEM_ALLOC_TCP_REP_LOCK));
	for (ix = NUM_VRID_TCP_REPS; ix -- > 0; ) {
		if (TSKID_TCP_REP[ix] == TSK_NONE) {
			*repid = VRID_TCP_REP[ix];
			TSKID_TCP_REP[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_REP_LOCK));

	if (*repid == TCP_REP_NONE)
		return E_NOEXS;
	else
		return TCP_CRE_REP(*repid, crep);
	}

/*
 *  free_tcp_rep -- TCP 受付口を解放する。
 */

ER
FREE_TCP_REP (ID repid, bool_t call_tcp_del_rep)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_TCP_REP_LOCK));
	for (ix = NUM_VRID_TCP_REPS; ix -- > 0; ) {
		if (repid == VRID_TCP_REP[ix]) {
			TSKID_TCP_REP[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_REP_LOCK));

	if (call_tcp_del_rep)
		return tcp_del_rep(repid);
	else
		return E_OK;
	}

/*
 *  tcpn_is_cepid -- TCP 通信端点が指定されたネットワーク層なら true を返す。
 */

bool_t
TCP_IS_CEPID (ID cepid)
{
	int_t	ix;

	for (ix = NUM_VRID_TCP_CEPS; ix -- > 0; ) {
		if (cepid == VRID_TCP_CEP[ix])
			return true;
		}
	return false;
	}

/*
 *  alloc_tcp_cep -- TCP 通信端点を獲得する。
 */

ER
ALLOC_TCP_CEP (ID *cepid, ID tskid, T_TCP_CCEP *ccep)
{
	int_t	ix;

	*cepid = TCP_CEP_NONE;
	syscall(wai_sem(SEM_ALLOC_TCP_CEP_LOCK));
	for (ix = NUM_VRID_TCP_CEPS; ix -- > 0; ) {
		if (TSKID_TCP_CEP[ix] == TSK_NONE) {
			*cepid = VRID_TCP_CEP[ix];
			TSKID_TCP_CEP[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_CEP_LOCK));

	if (*cepid == TCP_CEP_NONE)
		return E_NOEXS;
	else
		return tcp_cre_cep(*cepid, ccep);
	}

/*
 *  free_tcp_cep -- TCP 通信端点を解放する。
 */

ER
FREE_TCP_CEP (ID cepid)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_TCP_CEP_LOCK));
	for (ix = NUM_VRID_TCP_CEPS; ix -- > 0; ) {
		if (cepid == VRID_TCP_CEP[ix]) {
			TSKID_TCP_CEP[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_TCP_CEP_LOCK));

	return tcp_del_cep(cepid);
	}

#endif	/* of #ifdef USE_TCP_EXTENTIONS */

#ifdef USE_UDP_EXTENTIONS

/*
 *  alloc_udp_cep -- UDP 通信端点を獲得する。
 */

ER
ALLOC_UDP_CEP (ID *cepid, ID tskid, T_UDPN_CCEP *ccep)
{
	int_t	ix;

	*cepid = UDP_CEP_NONE;
	syscall(wai_sem(SEM_ALLOC_UDP_CEP_LOCK));
	for (ix = NUM_VRID_UDP_CEPS; ix -- > 0; ) {
		if (TSKID_UDP_CEP[ix] == TSK_NONE) {
			*cepid = VRID_UDP_CEP[ix];
			TSKID_UDP_CEP[ix] = tskid;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_UDP_CEP_LOCK));

	if (*cepid == UDP_CEP_NONE)
		return E_NOEXS;
	else
		return UDP_CRE_CEP(*cepid, ccep);
	}

/*
 *  free_udp_cep -- UDP 通信端点を解放する。
 */

ER
FREE_UDP_CEP (ID cepid, bool_t call_udp_del_cep)
{
	int_t	ix;

	syscall(wai_sem(SEM_ALLOC_UDP_CEP_LOCK));
	for (ix = NUM_VRID_UDP_CEPS; ix -- > 0; ) {
		if (cepid == VRID_UDP_CEP[ix]) {
			TSKID_UDP_CEP[ix] = TSK_NONE;
			break;
			}
		}
	syscall(sig_sem(SEM_ALLOC_UDP_CEP_LOCK));

	if (call_udp_del_cep)
		return UDP_DEL_CEP(cepid);
	else
		return E_OK;
	}

#endif	/* of #ifdef USE_UDP_EXTENTIONS */
