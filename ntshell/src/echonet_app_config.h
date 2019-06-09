/*
 *  TOPPERS PROJECT Home Network Working Group Software
 * 
 *  Copyright (C) 2016-2019 Cores Co., Ltd. Japan
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

#ifndef _ECHONET_APP_CONFIG_H_
#define _ECHONET_APP_CONFIG_H_

#define ECHONET_TASK_STACK_SIZE	1024	/* ECNL UDPタスク用スタック領域のサイズ */

#define ECHONET_API_DATAQUEUE_COUNT 5		/* ECHONET Lite API用データキュー */
#define ECHONET_SVC_DATAQUEUE_COUNT 5		/* ECHONET Lite サービス処理タスク用データキュー */
#define ECHONET_UDP_DATAQUEUE_COUNT 5		/* ECHONET Lite UDP通信処理用データキュー */

/*
 * サービス処理タスクに追加の処理を挿入したい場合に定義する
 *
 * タイムアウト値を返す
 * TMO echonet_task_get_timer();
 * #define ECHONET_TASK_GET_TIMER echonet_task_get_timer
 *
 * タイマーの時間を進める
 * void echonet_task_progress(TMO interval);
 * #define ECHONET_TASK_PROGRESS(interval) echonet_task_progress(interval)
 *
 * タイムアウト処理（のための呼び出し）
 * void echonet_task_timeout();
 * #define ECHONET_TASK_TIMEOUT echonet_task_timeout()
 */

/*
 * ＵＤＰ通信タスクに追加の処理を挿入したい場合に定義する
 *
 * タイムアウト値を返す
 * TMO echonet_udp_task_get_timer();
 * #define ECHONET_UDP_TASK_GET_TIMER echonet_udp_task_get_timer
 *
 * タイマーの時間を進める
 * void echonet_udp_task_progress(TMO interval);
 * #define ECHONET_UDP_TASK_PROGRESS(interval) echonet_udp_task_progress(interval)
 *
 * タイムアウト処理（のための呼び出し）
 * void echonet_udp_task_timeout();
 * #define ECHONET_UDP_TASK_TIMEOUT echonet_udp_task_timeout()
 */

/* デバッグ出力有効化設定するときに定義 */
/* #define ECN_DBG_PUT_ENA */

/* デバッグ出力(CARP)有効化設定するときに定義 */
/* #define ECN_CAP_PUT_ENA */

/* syslog空き待ち有効化設定するときに定義 */
/* #define ECN_DBG_WAI_ENA */

/* 標準のecn_data_prop_setを使わないときに定義 */
/* #define ECN_USER_DATA_PROP_SET */

/* 標準のecn_data_prop_getを使わないときに定義 */
/* #define ECN_USER_DATA_PROP_GET */

#endif /* _ECHONET_APP_CONFIG_H_ */
