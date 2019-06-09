/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2018-2019 Cores Co., Ltd. Japan
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

#ifndef _NTP_CLI_H_
#define _NTP_CLI_H_

/* スタックサイズ */

#define NTP_CLI_STACK_SIZE	1024		/* NTP クライアントタスク（IPv4）のスタックサイズ	*/

/* 優先度 */

#define NTP_CLI_MAIN_PRIORITY	5		/* NTP クライアントタスク（IPv4）の優先度		*/

/* タスク */

extern void	ntp_cli_task(intptr_t exinf);

/* NTPタイムスタンプ */
typedef struct ntp_timestamp {
	uint32_t integer;
	uint32_t fraction;
} __attribute__((packed)) T_NTP_TIMESTAMP;

/* NTPパケット */
typedef struct ntp_msg {
	uint8_t mode : 3;
	uint8_t version_number : 3;
	uint8_t leap_indicator : 2;
	uint8_t stratum;
	uint8_t poll;
	uint8_t precision;
	uint32_t root_delay;
	uint32_t root_dispersion;
	uint8_t reference_identifier[4];
	T_NTP_TIMESTAMP reference_timestamp;
	T_NTP_TIMESTAMP originate_timestamp;
	T_NTP_TIMESTAMP receive_timestamp;
	T_NTP_TIMESTAMP transmit_timestamp;
	uint8_t key_identifier[4];
	uint8_t message_digest[16];
} __attribute__((packed)) T_NTP_MSG;

typedef enum ntp_mode {
	NTP_MODE_RESERVED,
	NTP_MODE_SYMMETRIC_ACTIVE,
	NTP_MODE_SYMMETRIC_PASSIVE,
	NTP_MODE_CLIENT,
	NTP_MODE_SERVER,
	NTP_MODE_BROADCAST,
	NTP_MODE_NTP_CONTROL_MESSAGE,
	NTP_MODE_RESERVED_FOR_PRIVATE_USER
} ntp_mode_t;

/* NTPクライアント状態 */
typedef enum ntp_cli_state {
	NTP_CLI_STATE_ASYNC,
	NTP_CLI_STATE_RESOLVE_ADDR,
	NTP_CLI_STATE_REQUEST,
	NTP_CLI_STATE_SYNC,
} ntp_cli_state_t;

typedef struct ntp_cli {
	ntp_cli_state_t state;
	int timer;
	int retry;
#if defined(SUPPORT_INET6) && defined(SUPPORT_INET4)
	T_IN6_ADDR	ipaddr6;
#endif
	uint16_t portno;
	ID tskid;
	ID cepid;
	T_NTP_MSG ntp_msg;
	T_IPV4EP snd_rmt;
	T_IPV4EP rcv_rmt;
	uint8_t poll;
	T_NTP_TIMESTAMP reference_timestamp;
	T_NTP_TIMESTAMP originate_timestamp;
	T_NTP_TIMESTAMP receive_timestamp;
	T_NTP_TIMESTAMP transmit_timestamp;
	T_NTP_TIMESTAMP recv_resp_timestamp;
	char buf[30];
	int rcv_flag;
	int exe_flag;
} T_NTP_CLI_CONTEXT;

/* 関数 */

void ntp_cli_initialize(T_NTP_CLI_CONTEXT *nc, ID cepid);
int ntp_cli_get_timer(T_NTP_CLI_CONTEXT *nc);
void ntp_cli_progress(T_NTP_CLI_CONTEXT *nc, TMO elapse);
void ntp_cli_wakeup(T_NTP_CLI_CONTEXT *nc);
void ntp_cli_timeout(T_NTP_CLI_CONTEXT *nc);
ER callback_nblk_ntp_cli(ID cepid, FN fncd, void *p_parblk);

void ntp_cli_execute();

#endif	/* of #ifndef _NTP_CLI_H_ */
