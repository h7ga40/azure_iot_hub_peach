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
 *
 *	@(#)udp_usrreq.c	8.6 (Berkeley) 5/23/95
 * $FreeBSD: src/sys/netinet/udp_usrreq.c,v 1.49.2.2 1999/08/29 16:29:58 peter Exp $
 */

#ifdef __udp_send_data

/*
 *  udp_send_data -- パケット送信の主要部
 */

ER_UINT
UDP_SEND_DATA (T_UDP_CEP *cep, T_IPEP *p_dstaddr, void *data, int_t len, TMO tmout)
{
	T_NET_BUF	*output;
	T_UDP_HDR	*udph;
	SYSTIM		before, after;
	ER		error = E_OK;

#ifdef UDP_CFG_OUT_CHECKSUM
	uint16_t	sum;
#endif	/* of #ifdef UDP_CFG_OUT_CHECKSUM */

	NET_COUNT_UDP(net_count_udp.out_octets,  len);
	NET_COUNT_UDP(net_count_udp.out_packets, 1);

	/* IP データグラム割り当ての時間を tmout から減ずる。*/
	if (!(tmout == TMO_POL || tmout == TMO_FEVR))
		syscall(get_tim(&before));

	/* IP データグラムを割り当てる。*/
	if ((error = IN_GET_DATAGRAM(&output, (uint_t)(UDP_HDR_SIZE + len), 0,
	                             &p_dstaddr->ipaddr,
	                             &cep->myaddr.ipaddr,
	                             IPPROTO_UDP, IP_DEFTTL, 
	                             NBA_SEARCH_ASCENT, tmout)) != E_OK)
		goto err_ret;

	/* IP データグラム割り当ての時間を tmout から減ずる。*/
	if (!(tmout == TMO_POL || tmout == TMO_FEVR)) {
		syscall(get_tim(&after));
		if (after - before > tmout) {
			syscall(rel_net_buf(output));
			error = E_TMOUT;
			goto err_ret;
			}
		tmout -= (TMO)(after - before);
		}

	/* UDP ヘッダに情報を設定する。*/
	udph		= GET_UDP_HDR(output, IF_IP_UDP_HDR_OFFSET(output));
	udph->sport	= htons(cep->myaddr.portno);
	udph->dport	= htons(p_dstaddr->portno);
	udph->ulen	= htons(UDP_HDR_SIZE + len);
	udph->sum	= 0;

	/* データをコピーする。*/
	memcpy((void*)GET_UDP_SDU(output, IF_IP_UDP_HDR_OFFSET(output)), data, (size_t)len);

#ifdef UDP_CFG_OUT_CHECKSUM

	sum = IN_CKSUM(output, IPPROTO_UDP, IF_IP_UDP_HDR_OFFSET(output), (uint_t)(UDP_HDR_SIZE + len));

	/* 計算したチェックサムの値が 0 なら 0xffff を入れる。*/
	if (sum == 0)
		sum = 0xffff;
	udph->sum = sum;

#endif	/* of #ifdef UDP_CFG_OUT_CHECKSUM */

	/* ネットワークバッファ長を調整する。*/
	output->len = (uint16_t)(IF_IP_UDP_HDR_SIZE(output) + len);

	/* ネットワーク層 (IP) の出力関数を呼び出す。*/
	if ((error = IP_OUTPUT(output, tmout)) == E_OK) {
		NET_COUNT_MIB(udp_stats.udpOutDatagrams, 1);
		cep->snd_tskid = TA_NULL;
		return len;
		}

err_ret:
	NET_COUNT_UDP(net_count_udp.out_err_packets, 1);
	cep->snd_tskid = TA_NULL;
	return error;
	}

#endif	/* of #ifdef __udp_send_data */

/*
 *  udp_cancel_cep -- ペンディングしている処理をキャンセルする。
 *
 *    udp_can_cep から呼出されるメイン関数
 */

#ifdef __udp_can_cep

static ER
UDP_CANCEL_CEP (T_UDP_CEP *cep, FN fncd)
{
	ER	error = E_OK, snd_err, rcv_err;

	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;

	/* API 機能コードをチェックする。*/
	if (!VALID_TFN_UDP_CAN(fncd))
		return E_PAR;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if (fncd == TFN_UDP_ALL) {	/* TFN_UDP_ALL の処理 */

		snd_err = UDP_CAN_SND(cep, E_RLWAI);
		rcv_err = UDP_CAN_RCV(cep, E_RLWAI);

		/*
		 *  snd_err と rcv_err のどちらも EV_NOPND
		 *  なら、ペンディングしていないのでエラー
		 */
		if (snd_err == EV_NOPND && rcv_err == EV_NOPND)
			error = E_OBJ;
		else {
			if (snd_err == EV_NOPND)
				snd_err = E_OK;
			if (rcv_err == EV_NOPND)
				rcv_err = E_OK;

			if (snd_err != E_OK)
				error = snd_err;
			else if (rcv_err != E_OK)
				error = rcv_err;
			}
		}
	else if (fncd == TFN_UDP_SND_DAT) {	/* 送信処理のキャンセル */
		if ((error = UDP_CAN_SND(cep, E_RLWAI)) == EV_NOPND)
			error = E_OBJ;
		}
	else if (fncd == TFN_UDP_RCV_DAT) {	/* 受信処理のキャンセル */
		if ((error = UDP_CAN_RCV(cep, E_RLWAI)) == EV_NOPND)
			error = E_OBJ;
		}
	else
		error = E_PAR;

	/* 通信端点をロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

#endif	/* of #ifdef __udp_can_cep */

#if defined(SUPPORT_INET6) && (defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0)

#else	/* of #if defined(SUPPORT_INET6) && (defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0) */

/*
 *  udp_can_cep -- ペンディングしている処理のキャンセル【標準機能】
 */

#ifdef __udp_can_cep

ER
udp_can_cep (ID cepid, FN fncd)
{
	T_UDP_CEP	*cep;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* メイン関数を呼び出す。*/
	return UDP_CANCEL_CEP(cep, fncd);
	}

#endif	/* of #ifdef __udp_can_cep */

#endif	/* of #if defined(SUPPORT_INET6) && (defined(SUPPORT_INET4) && TNUM_UDP4_CEPID > 0) */

#ifdef UDP_CFG_EXTENTIONS

/*
 *  udp_cre_cep -- UDP 通信端点の生成【拡張機能】
 */

#ifdef __udp_cre_cep

ER
UDP_CRE_CEP (ID cepid, T_UDPN_CCEP *pk_ccep)
{
	T_UDP_CEP	*cep;
	ER		error;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* pk_ccep が NULL ならエラー */
	if (pk_ccep == NULL)
		return E_PAR;

#if (API_PROTO == API_PROTO_IPV6) && !defined(API_CFG_IP4MAPPED_ADDR)

	/*
	 * API が IPv6 で IPv4 射影アドレスが認められていないのにもかかわらず、
	 * IPv4 射影アドレスが指定されたらエラー
	 */
	if (in6_is_addr_ipv4mapped(&pk_ccep->myaddr.ipaddr))
		return E_PAR;

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) && !defined(DAPI_CFG_IP4MAPPED_ADDR) */

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* UDP 通信端点をチェックする。*/
	if (VALID_UDP_CEP(cep))
		return E_OBJ;

	/* UDP 通信端点が、動的生成用でなければエラー */
	if (!DYNAMIC_UDP_CEP(cep))
		return E_ID;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 * UDP 通信端点をチェックする。生成済みであればエラー
	 */
	if (VALID_UDP_CEP(cep))
		error = E_OBJ;
	else {

		/*
		 *  自ポート番号が UDP_PORTANY なら、自動で割り当てる。
		 */
		if (pk_ccep->myaddr.portno == UDP_PORTANY)
			error = UDP_ALLOC_AUTO_PORT(cep);
		else 
			error = UDP_ALLOC_PORT(cep, pk_ccep->myaddr.portno);

			if (error == E_OK) {
				/* UDP 通信端点生成情報をコピーする。*/
				cep->cepatr        = pk_ccep->cepatr;			/* 通信端点属性		*/
				cep->myaddr.ipaddr = pk_ccep->myaddr.ipaddr;		/* 自分のアドレス	*/
				cep->callback      = (void*)pk_ccep->callback;		/* コールバック		*/

				/* UDP 通信端点を生成済みにする。*/
				cep->flags |= UDP_CEP_FLG_VALID;
				}
			}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

#endif	/* of #ifdef __udp_cre_cep */

#ifdef __udp_del_cep

/*
 *  udp_delete_cep -- UDP 通信端点を削除する。
 *
 *    udp_del_cep から呼出されるメイン関数
 */

static ER
UDP_DELETE_CEP (T_UDP_CEP *cep)
{
	ER	error;

	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;

	/* UDP 通信端点が、動的生成用でなければエラー */
	if (!DYNAMIC_UDP_CEP(cep))
		return E_ID;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	/*
	 * UDP 通信端点をチェックする。未生成の場合はエラー
	 * ・未生成。
	 */
	if (!VALID_UDP_CEP(cep))
		error = E_NOEXS;
	else {
		UDP_CAN_SND(cep, E_DLT);
		UDP_CAN_RCV(cep, E_DLT);

		/* UDP 通信端点を未生成にする。*/
		cep->flags &= ~UDP_CEP_FLG_VALID;
		error = E_OK;
		}

	/* 通信端点のロックを解除する。*/
	syscall(sig_sem(cep->semid_lock));

	return error;
	}

/*
 *  udp_del_cep -- UDP 通信端点の削除【拡張機能】
 */

ER
UDP_DEL_CEP (ID cepid)
{
	T_UDP_CEP	*cep;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* メイン関数を呼び出す。*/
	return UDP_DELETE_CEP(cep);
	}

#endif	/* of #ifdef __udp_del_cep */

#ifdef __udp_set_opt

/*
 *  udp_set_option -- UDP 通信端点オプションを設定する。
 *
 *    udp_set_opt から呼出されるメイン関数
 */

static ER
UDP_SET_OPTION (T_UDP_CEP *cep, int_t optname, void *optval, int_t optlen)
{
	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;
#if defined(SUPPORT_INET6)
#elif defined(SUPPORT_INET4) && defined(SUPPORT_IGMP) && TNUM_UDP4_CEPID > 0
	switch (optname) {
	case IP_MULTICAST_LOOP:
		if (optlen >= sizeof(uint8_t)) {
			return igmp_set_loop(cep, *(uint8_t *)optval);
			}
		break;
	case IP_MULTICAST_TTL:
		if (optlen >= sizeof(uint8_t)) {
			return igmp_set_ttl(cep, *(uint8_t *)optval);
			}
		break;
	case IP_MULTICAST_IF:
		if (optlen >= sizeof(T_IN4_ADDR)) {
			return igmp_set_if(cep, (T_IN4_ADDR *)optval);
			}
		break;
	case IP_ADD_MEMBERSHIP:
		if (optlen >= sizeof(T_IP_MREQ)) {
			return igmp_add_membership(cep, (T_IP_MREQ *)optval);
			}
		break;
	case IP_DROP_MEMBERSHIP:
		if (optlen >= sizeof(T_IP_MREQ)) {
			return igmp_drop_membership(cep, (T_IP_MREQ *)optval);
			}
		break;
	case IP_MSFILTER:
		if (optlen >= sizeof(T_IP_MSFILTER)) {
			return igmp_set_ip_msfilter(cep, (T_IP_MSFILTER *)optval);
			}
		break;
		}
#endif
	return E_PAR;
	}

/*
 *  udp_set_opt -- UDP 通信端点オプションの設定【拡張機能】
 *
 *  注意: 設定可能な UDP 通信端点オプションは無いため、E_PAR が返される。
 */

ER
UDP_SET_OPT (ID cepid, int_t optname, void *optval, int_t optlen)
{
	T_UDP_CEP	*cep;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* メイン関数を呼び出す。*/
	return UDP_SET_OPTION(cep, optname, optval, optlen);
	}

#endif	/* of #ifdef __udp_set_opt */

#ifdef __udp_get_opt

/*
 *  udp_get_option -- UDP 通信端点オプションを獲得する。
 *
 *    udp_get_opt から呼出されるメイン関数
 */

static ER
UDP_GET_OPTION (T_UDP_CEP *cep, int_t optname, void *optval, int_t optlen)
{
	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;
#if defined(SUPPORT_INET6)
#elif defined(SUPPORT_INET4) && defined(SUPPORT_IGMP) && TNUM_UDP4_CEPID > 0
	switch (optname) {
	case IP_MULTICAST_LOOP:
		if (optlen >= sizeof(uint8_t)) {
			return igmp_get_loop(cep, (uint8_t *)optval);
			}
		break;
	case IP_MULTICAST_TTL:
		if (optlen >= sizeof(uint8_t)) {
			return igmp_get_ttl(cep, (uint8_t *)optval);
			}
		break;
	case IP_MULTICAST_IF:
		if (optlen >= sizeof(uint32_t)) {
			return igmp_get_if(cep, (uint32_t *)optval);
			}
		break;
	case IP_MSFILTER:
		if (optlen >= sizeof(T_IP_MSFILTER)) {
			return igmp_get_ip_msfilter(cep, (T_IP_MSFILTER *)optval);
			}
		break;
		}
#endif
	return E_PAR;
	}

/*
 *  udp_get_opt -- UDP 通信端点オプションの獲得【拡張機能】
 *
 *  注意: 設定可能な UDP 通信端点オプションは無いため、E_PAR が返される。
 */

ER
UDP_GET_OPT (ID cepid, int_t optname, void *optval, int_t optlen)
{
	T_UDP_CEP	*cep;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* メイン関数を呼び出す。*/
	return UDP_GET_OPTION(cep, optname, optval, optlen);
	}

#endif	/* of #ifdef __udp_get_opt */

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

#ifdef UDP_CFG_NON_BLOCKING

#else	/* of #ifdef UDP_CFG_NON_BLOCKING */

#ifdef __udp_can_snd

/*
 *  udp_can_snd -- ペンディングしている送信のキャンセル
 */

ER
UDP_CAN_SND (T_UDP_CEP *cep, ER error)
{
	if (cep->snd_tskid != TA_NULL) {	/* 非ノンブロッキングコールでペンディング中 */

#ifdef UDP_CFG_EXTENTIONS

		/* 待ち中に発生したエラー情報を設定する。*/
		cep->error = error;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

		error = rel_wai(cep->snd_tskid);
		cep->snd_tskid = TA_NULL;
		}
	else					/* どちらでもないならペンディングしていない */
		error = EV_NOPND;

	return error;
	}

#endif	/* of #ifdef __udp_can_snd */

#ifdef __udp_can_rcv

/*
 *  udp_can_rcv -- ペンディングしている受信のキャンセル
 */

ER
UDP_CAN_RCV (T_UDP_CEP *cep, ER error)
{
	if (cep->rcv_tskid != TA_NULL) {	/* 非ノンブロッキングコールでペンディング中 */

#ifdef UDP_CFG_EXTENTIONS

		/* 待ち中に発生したエラー情報を設定する。*/
		cep->error = error;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

		error = rel_wai(cep->rcv_tskid);
		cep->rcv_tskid = TA_NULL;
		}
	else					/* どちらでもないならペンディングしていない */
		error = EV_NOPND;

	return error;
	}

#endif	/* of #ifdef __udp_can_rcv */

#ifdef __udp_snd_dat

/*
 *  udp_snd_dat -- パケットの送信【標準機能】
 */

ER_UINT
UDP_SND_DAT (ID cepid, T_IPEP *p_dstaddr, void *data, int_t len, TMO tmout)
{
	T_UDP_CEP	*cep;
	ER		error;

	/* p_dstaddr または data が NULL か、tmout が TMO_NBLK ならエラー */
	if (p_dstaddr == NULL || data == NULL || tmout == TMO_NBLK)
		return E_PAR;

	/* データ長をチェックする。*/
	if (len < 0 || len + IP_HDR_SIZE + UDP_HDR_SIZE > IF_MTU)
		return E_PAR;

#if (API_PROTO == API_PROTO_IPV6) && !defined(API_CFG_IP4MAPPED_ADDR)

	/*
	 * API が IPv6 で IPv4 射影アドレスが認められていないのにもかかわらず、
	 * IPv4 射影アドレスが指定されたらエラー
	 */
	if (in6_is_addr_ipv4mapped(&p_dstaddr->ipaddr))
		return E_PAR;

#endif	/* of #if (API_PROTO == API_PROTO_IPV6) && !defined(DAPI_CFG_IP4MAPPED_ADDR) */

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;

	/*
	 *  自ポート番号が UDP_PORTANY なら、自動で割り当てる。
	 */
	if (cep->myaddr.portno == UDP_PORTANY) {
		if ((error = UDP_ALLOC_AUTO_PORT(cep)) != E_OK)
			return error;
		}
	
	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if (cep->snd_tskid != TA_NULL) {

		/* 非ノンブロッキングコールでペンディング中 */
		error = E_QOVR;

		/* 通信端点をロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));
		}
	else {
		/* 現在のタスク識別子を記録する。*/
		get_tid(&(cep->snd_tskid));

#ifdef UDP_CFG_EXTENTIONS

		/* 待ち中に発生したエラー情報をリセットする。*/
		cep->error = E_OK;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

		/* 通信端点をロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		/* パケットを送信する。*/
		error = UDP_SEND_DATA(cep, p_dstaddr, data, len, tmout);

#ifdef UDP_CFG_EXTENTIONS

		/* 待ち中に発生したエラー情報を返す。*/
		if (error == E_RLWAI)
			error = cep->error;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */
		}

	return error;
	}

#endif	/* of #ifdef __udp_snd_dat */

#ifdef __udp_rcv_dat

/*
 *  udp_rcv_dat -- パケットの受信【標準機能】
 */

ER_UINT
UDP_RCV_DAT (ID cepid, T_IPEP *p_dstaddr, void *data, int_t len, TMO tmout)
{
	T_NET_BUF	*input;
	T_UDP_CEP	*cep;
	T_UDP_HDR	*udph;
	ER_UINT		error;
	uint_t		ulen, uhoff;

	/* p_dstaddr または data が NULL 、len < 0 か、tmout が TMO_NBLK ならエラー */
	if (p_dstaddr == NULL || data == NULL || len < 0 || tmout == TMO_NBLK)
		return E_PAR;

	/* UDP 通信端点 ID をチェックする。*/
	if (!VALID_UDP_CEPID(cepid))
		return E_ID;

	/* UDP 通信端点を得る。*/
	cep = GET_UDP_CEP(cepid);

	/* UDP 通信端点をチェックする。*/
	if (!VALID_UDP_CEP(cep))
		return E_NOEXS;

	/* 通信端点をロックする。*/
	syscall(wai_sem(cep->semid_lock));

	if (cep->rcv_tskid != TA_NULL) {

		/* 非ノンブロッキングコールでペンディング中 */
		error = E_QOVR;

		/* 通信端点をロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));
		}
	else {

		/* 現在のタスク識別子を記録する。*/
		get_tid(&(cep->rcv_tskid));

#ifdef UDP_CFG_EXTENTIONS

		/* 待ち中に発生したエラー情報をリセットする。*/
		cep->error = E_OK;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

		/* 通信端点をロックを解除する。*/
		syscall(sig_sem(cep->semid_lock));

		/* 入力があるまで待つ。*/
		if (cep->cb_netbuf != NULL) {

			/*
			 *  ここにくる場合は、コールバック関数の中から
			 *  udp_rcv_dat を呼び出していることになり、
			 *  すでに入力済みである。
			 */
			input = cep->cb_netbuf;
			cep->cb_netbuf = NULL;
			}
		else if ((error = trcv_dtq(cep->rcvqid, (intptr_t*)&input, tmout)) != E_OK) {

#ifdef UDP_CFG_EXTENTIONS

			/* 待ち中に発生したエラー情報を返す。*/
			if (error == E_RLWAI)
				error = cep->error;

#endif	/* of #ifdef UDP_CFG_EXTENTIONS */

			cep->rcv_tskid = TA_NULL;
			return error;
			}

		/* p_dstaddr を設定する。*/
		uhoff = (uint_t)GET_UDP_HDR_OFFSET(input);
		udph = GET_UDP_HDR(input, uhoff);
		p_dstaddr->portno = ntohs(udph->sport);
		IN_COPY_TO_HOST(&p_dstaddr->ipaddr, input);

		/* データをバッファに移す。*/
		ulen = ntohs(udph->ulen);
		if (ulen - UDP_HDR_SIZE > len)
			error = E_BOVR;
		else {
			len   =    (uint_t)(ulen - UDP_HDR_SIZE);
			error = (ER_UINT)(ulen - UDP_HDR_SIZE);
			}

		memcpy(data, GET_UDP_SDU(input, uhoff), (size_t)len);

		syscall(rel_net_buf(input));

		cep->rcv_tskid = TA_NULL;
		}

	return error;
	}

#endif	/* of #ifdef __udp_rcv_dat */

#endif	/* of #ifdef UDP_CFG_NON_BLOCKING */
