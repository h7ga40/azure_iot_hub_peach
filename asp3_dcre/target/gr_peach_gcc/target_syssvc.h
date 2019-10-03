/*
 *		システムサービスのターゲット依存部
 *
 *  システムサービスのターゲット依存部のヘッダファイル．システムサービ
 *  スのターゲット依存の設定は，できる限りコンポーネント記述ファイルで
 *  記述し，このファイルに記述するものは最小限とする．
 * 
 *  $Id$
 */

#ifndef TOPPERS_TARGET_SYSSVC_H
#define TOPPERS_TARGET_SYSSVC_H

/*
 *  コアで共通な定義（チップ依存部は飛ばす）
 */
#include "core_syssvc.h"

/*
 *  起動メッセージのターゲットシステム名
 */
#define	TARGET_NAME	"GR-PEACH"

/*
 *  使用するシリアルポートID
 */
#define	SIO_PORTID			UINT_C( 1 )

#endif /* TOPPERS_TARGET_SYSSVC_H */
