/* #[<PREAMBLE>]#
 * #[<...>]# から #[</...>]# で囲まれたコメントは編集しないでください
 * tecsmerge によるマージに使用されます
 *
 * 属性アクセスマクロ #_CAAM_#
 * NAME_LEN         int16_t          ATTR_NAME_LEN   
 * name             char_t*          VAR_name        
 * name2            char_t*          VAR_name2       
 * selectedCbp      int8_t*          VAR_selectedCbp 
 * selectedInibp    int8_t*          VAR_selectedInibp
 *
 * 呼び口関数 #_TCPF_#
 * call port: cTECSInfo signature: nTECSInfo_sTECSInfo context:task
 *   ER             cTECSInfo_findNamespace( const char_t* namespace_path, Descriptor( nTECSInfo_sNamespaceInfo )* NSdesc );
 *   ER             cTECSInfo_findRegion( const char_t* namespace_path, Descriptor( nTECSInfo_sRegionInfo )* RGNdesc );
 *   ER             cTECSInfo_findSignature( const char_t* namespace_path, Descriptor( nTECSInfo_sSignatureInfo )* SIGdesc );
 *   ER             cTECSInfo_findCelltype( const char_t* namespace_path, Descriptor( nTECSInfo_sCelltypeInfo )* CTdesc );
 *   ER             cTECSInfo_findCell( const char_t* namespace_path, Descriptor( nTECSInfo_sCellInfo )* CELLdesc );
 * call port: cNSInfo signature: nTECSInfo_sNamespaceInfo context:task optional:true
 *   bool_t     is_cNSInfo_joined()                     check if joined
 *   ER             cNSInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cNSInfo_getNameLength( );
 *   uint32_t       cNSInfo_getNNamespace( );
 *   ER             cNSInfo_getNamespaceInfo( uint32_t ith, Descriptor( nTECSInfo_sNamespaceInfo )* des );
 *   uint32_t       cNSInfo_getNSignature( );
 *   ER             cNSInfo_getSignatureInfo( uint32_t ith, Descriptor( nTECSInfo_sSignatureInfo )* des );
 *   uint32_t       cNSInfo_getNCelltype( );
 *   ER             cNSInfo_getCelltypeInfo( uint32_t ith, Descriptor( nTECSInfo_sCelltypeInfo )* des );
 *   [dynamic, optional]
 *      void           cNSInfo_set_descriptor( Descriptor( nTECSInfo_sNamespaceInfo ) desc );
 *      void           cNSInfo_unjoin(  );
 * call port: cCelltypeInfo signature: nTECSInfo_sCelltypeInfo context:task optional:true
 *   bool_t     is_cCelltypeInfo_joined()                     check if joined
 *   ER             cCelltypeInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cCelltypeInfo_getNameLength( );
 *   uint32_t       cCelltypeInfo_getNAttr( );
 *   ER             cCelltypeInfo_getAttrInfo( uint32_t ith, Descriptor( nTECSInfo_sVarDeclInfo )* desc );
 *   uint32_t       cCelltypeInfo_getNVar( );
 *   ER             cCelltypeInfo_getVarInfo( uint32_t ith, Descriptor( nTECSInfo_sVarDeclInfo )* desc );
 *   uint32_t       cCelltypeInfo_getNCall( );
 *   ER             cCelltypeInfo_getCallInfo( uint32_t ith, Descriptor( nTECSInfo_sCallInfo )* desc );
 *   uint32_t       cCelltypeInfo_getNEntry( );
 *   ER             cCelltypeInfo_getEntryInfo( uint32_t ith, Descriptor( nTECSInfo_sEntryInfo )* desc );
 *   bool_t         cCelltypeInfo_isSingleton( );
 *   bool_t         cCelltypeInfo_isIDX_is_ID( );
 *   uint32_t       cCelltypeInfo_sizeOfCB( );
 *   uint32_t       cCelltypeInfo_sizeOfINIB( );
 *   [dynamic, optional]
 *      void           cCelltypeInfo_set_descriptor( Descriptor( nTECSInfo_sCelltypeInfo ) desc );
 *      void           cCelltypeInfo_unjoin(  );
 * call port: cAttrInfo signature: nTECSInfo_sVarDeclInfo context:task optional:true
 *   bool_t     is_cAttrInfo_joined()                     check if joined
 *   ER             cAttrInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cAttrInfo_getNameLength( );
 *   uint32_t       cAttrInfo_getOffset( );
 *   void           cAttrInfo_getTypeInfo( Descriptor( nTECSInfo_sTypeInfo )* desc );
 *   void           cAttrInfo_getSizeIsExpr( char_t* expr_str, int32_t max_len );
 *   ER             cAttrInfo_getSizeIs( uint32_t* size, const void* p_cb );
 *   [dynamic, optional]
 *      void           cAttrInfo_set_descriptor( Descriptor( nTECSInfo_sVarDeclInfo ) desc );
 *      void           cAttrInfo_unjoin(  );
 * call port: cVarInfo signature: nTECSInfo_sVarDeclInfo context:task optional:true
 *   bool_t     is_cVarInfo_joined()                     check if joined
 *   ER             cVarInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cVarInfo_getNameLength( );
 *   uint32_t       cVarInfo_getOffset( );
 *   void           cVarInfo_getTypeInfo( Descriptor( nTECSInfo_sTypeInfo )* desc );
 *   void           cVarInfo_getSizeIsExpr( char_t* expr_str, int32_t max_len );
 *   ER             cVarInfo_getSizeIs( uint32_t* size, const void* p_cb );
 *   [dynamic, optional]
 *      void           cVarInfo_set_descriptor( Descriptor( nTECSInfo_sVarDeclInfo ) desc );
 *      void           cVarInfo_unjoin(  );
 * call port: cTypeInfo signature: nTECSInfo_sTypeInfo context:task optional:true
 *   bool_t     is_cTypeInfo_joined()                     check if joined
 *   ER             cTypeInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cTypeInfo_getNameLength( );
 *   uint32_t       cTypeInfo_getSize( );
 *   int8_t         cTypeInfo_getKind( );
 *   uint32_t       cTypeInfo_getNType( );
 *   ER             cTypeInfo_getTypeInfo( Descriptor( nTECSInfo_sTypeInfo )* desc );
 *   uint32_t       cTypeInfo_getNMember( );
 *   ER             cTypeInfo_getMemberInfo( uint32_t ith, Descriptor( nTECSInfo_sVarDeclInfo )* desc );
 *   [dynamic, optional]
 *      void           cTypeInfo_set_descriptor( Descriptor( nTECSInfo_sTypeInfo ) desc );
 *      void           cTypeInfo_unjoin(  );
 * call port: cRegionInfo signature: nTECSInfo_sRegionInfo context:task optional:true
 *   bool_t     is_cRegionInfo_joined()                     check if joined
 *   ER             cRegionInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cRegionInfo_getNameLength( );
 *   uint32_t       cRegionInfo_getNCell( );
 *   ER             cRegionInfo_getCellInfo( uint32_t ith, Descriptor( nTECSInfo_sCellInfo )* des );
 *   uint32_t       cRegionInfo_getNRegion( );
 *   ER             cRegionInfo_getRegionInfo( uint32_t ith, Descriptor( nTECSInfo_sRegionInfo )* des );
 *   [dynamic, optional]
 *      void           cRegionInfo_set_descriptor( Descriptor( nTECSInfo_sRegionInfo ) desc );
 *      void           cRegionInfo_unjoin(  );
 * call port: cCellInfo signature: nTECSInfo_sCellInfo context:task optional:true
 *   bool_t     is_cCellInfo_joined()                     check if joined
 *   ER             cCellInfo_getName( char_t* name, int_t max_len );
 *   uint16_t       cCellInfo_getNameLength( );
 *   void           cCellInfo_getCelltypeInfo( Descriptor( nTECSInfo_sCelltypeInfo )* desc );
 *   void           cCellInfo_getCBP( void** cbp );
 *   void           cCellInfo_getINIBP( void** inibp );
 *   [dynamic, optional]
 *      void           cCellInfo_set_descriptor( Descriptor( nTECSInfo_sCellInfo ) desc );
 *      void           cCellInfo_unjoin(  );
 *
 * #[</PREAMBLE>]# */

/* プロトタイプ宣言や変数の定義をここに書きます #_PAC_# */
#include <string.h>
#include "nTECSInfo_tTECSInfoAccessor_tecsgen.h"

#ifndef E_OK
#define	E_OK	0		/* success */
#define	E_ID	(-18)	/* illegal ID */
#endif

/* 受け口関数 #_TEPF_# */
/* #[<ENTRY_PORT>]# eSelector
 * entry port: eSelector
 * signature:  nTECSInfo_sAccessor
 * context:    task
 * #[</ENTRY_PORT>]# */

/* #[<ENTRY_FUNC>]# eSelector_selectNamespaceInfoByName
 * name:         eSelector_selectNamespaceInfoByName
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectNamespaceInfoByName
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectNamespaceInfoByName(CELLIDX idx, const char_t* namespacePath)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
  Descriptor( nTECSInfo_sNamespaceInfo ) NSdesc;
    if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

  /* ここに処理本体を記述します #_TEFB_# */
  ercd = cTECSInfo_findNamespace( namespacePath, &NSdesc );
  if( ercd == E_OK ){
      cNSInfo_set_descriptor( NSdesc );
  }
  else{
      cNSInfo_unjoin();
  }

  return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCelltypeInfoByName
 * name:         eSelector_selectCelltypeInfoByName
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCelltypeInfoByName
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCelltypeInfoByName(CELLIDX idx, const char_t* namespacePath)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
  Descriptor( nTECSInfo_sCelltypeInfo ) desc;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */
  ercd = cTECSInfo_findCelltype( namespacePath, &desc );
  if( ercd == E_OK ){
      cCelltypeInfo_set_descriptor( desc );
  }
  else{
      cCelltypeInfo_unjoin();
  }

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectSignatureInfoByName
 * name:         eSelector_selectSignatureInfoByName
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectSignatureInfoByName
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectSignatureInfoByName(CELLIDX idx, const char_t* namespacePath)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectRegionInfoByName
 * name:         eSelector_selectRegionInfoByName
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectRegionInfoByName
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectRegionInfoByName(CELLIDX idx, const char_t* regionPath)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCellInfoByName
 * name:         eSelector_selectCellInfoByName
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCellInfoByName
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCellInfoByName(CELLIDX idx, const char_t* cellPath)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedNamespaceInfo
 * name:         eSelector_getSelectedNamespaceInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedNamespaceInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedNamespaceInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* num_celltype, int_t* num_signature)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCelltypeInfo
 * name:         eSelector_selectCelltypeInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCelltypeInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCelltypeInfo(CELLIDX idx, int_t i)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectSignatureInfo
 * name:         eSelector_selectSignatureInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectSignatureInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectSignatureInfo(CELLIDX idx, int_t i)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCelltypeInfo
 * name:         eSelector_getSelectedCelltypeInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCelltypeInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCelltypeInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* num_attr, int_t* num_var, int_t* num_call, int_t* num_entry)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCallInfo
 * name:         eSelector_selectCallInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCallInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCallInfo(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectEntryInfo
 * name:         eSelector_selectEntryInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectEntryInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectEntryInfo(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectAttrInfo
 * name:         eSelector_selectAttrInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectAttrInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectAttrInfo(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectVarInfo
 * name:         eSelector_selectVarInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectVarInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectVarInfo(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedAttrInfo
 * name:         eSelector_getSelectedAttrInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedAttrInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedAttrInfo(CELLIDX idx, char_t* name, int_t max_len)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSizeIsExprOfAttr
 * name:         eSelector_getSizeIsExprOfAttr
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSizeIsExprOfAttr
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
void
eSelector_getSizeIsExprOfAttr(CELLIDX idx, char_t* expr_str, int32_t max_len)
{
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		/* エラー処理コードをここに記述します */
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

}

/* #[<ENTRY_FUNC>]# eSelector_selectTypeInfoOfAttr
 * name:         eSelector_selectTypeInfoOfAttr
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectTypeInfoOfAttr
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectTypeInfoOfAttr(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedVarInfo
 * name:         eSelector_getSelectedVarInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedVarInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedVarInfo(CELLIDX idx, char_t* name, int_t max_len)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSizeIsExprOfVar
 * name:         eSelector_getSizeIsExprOfVar
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSizeIsExprOfVar
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
void
eSelector_getSizeIsExprOfVar(CELLIDX idx, char_t* expr_str, int32_t max_len)
{
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		/* エラー処理コードをここに記述します */
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

}

/* #[<ENTRY_FUNC>]# eSelector_selectTypeInfoOfVar
 * name:         eSelector_selectTypeInfoOfVar
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectTypeInfoOfVar
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectTypeInfoOfVar(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCallInfo
 * name:         eSelector_getSelectedCallInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCallInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCallInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* array_size)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectSignatureOfCall
 * name:         eSelector_selectSignatureOfCall
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectSignatureOfCall
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectSignatureOfCall(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCallSpecifierInfo
 * name:         eSelector_getSelectedCallSpecifierInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCallSpecifierInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCallSpecifierInfo(CELLIDX idx, bool_t* b_optional, bool_t* b_dynamic, bool_t* b_ref_desc, bool_t* b_omit)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCallInternalInfo
 * name:         eSelector_getSelectedCallInternalInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCallInternalInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCallInternalInfo(CELLIDX idx, bool_t* b_allocator_port, bool_t* b_require_port)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCallLocationInfo
 * name:         eSelector_getSelectedCallLocationInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCallLocationInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCallLocationInfo(CELLIDX idx, uint32_t* offset, int8_t* place)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedCallOptimizeInfo
 * name:         eSelector_getSelectedCallOptimizeInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedCallOptimizeInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedCallOptimizeInfo(CELLIDX idx, bool_t* b_VMT_useless, bool_t* b_skelton_useless, bool_t* b_cell_unique)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedEntryInfo
 * name:         eSelector_getSelectedEntryInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedEntryInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedEntryInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* array_size)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectSignatureOfEntry
 * name:         eSelector_selectSignatureOfEntry
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectSignatureOfEntry
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectSignatureOfEntry(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedEntryInlineInfo
 * name:         eSelector_getSelectedEntryInlineInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedEntryInlineInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedEntryInlineInfo(CELLIDX idx, bool_t* b_inline)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedSignatureInfo
 * name:         eSelector_getSelectedSignatureInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedSignatureInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedSignatureInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* num_function)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selecFunctionInfoByIndex
 * name:         eSelector_selecFunctionInfoByIndex
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selecFunctionInfoByIndex
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selecFunctionInfoByIndex(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedFunctionInfo
 * name:         eSelector_getSelectedFunctionInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedFunctionInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedFunctionInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* num_args)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectTypeInfoOfReturn
 * name:         eSelector_selectTypeInfoOfReturn
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectTypeInfoOfReturn
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectTypeInfoOfReturn(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedParamInfo
 * name:         eSelector_getSelectedParamInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedParamInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedParamInfo(CELLIDX idx, char_t* name, int_t max_len, int8_t* dir)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectParamInfo
 * name:         eSelector_selectParamInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectParamInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectParamInfo(CELLIDX idx, int_t ith)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectTypeInfoOfParam
 * name:         eSelector_selectTypeInfoOfParam
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectTypeInfoOfParam
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectTypeInfoOfParam(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedTypeInfo
 * name:         eSelector_getSelectedTypeInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedTypeInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedTypeInfo(CELLIDX idx, char_t* name, int_t max_len, int8_t* kind)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectTypeInfoOfType
 * name:         eSelector_selectTypeInfoOfType
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectTypeInfoOfType
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectTypeInfoOfType(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelectedRegionInfo
 * name:         eSelector_getSelectedRegionInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelectedRegionInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelectedRegionInfo(CELLIDX idx, char_t* name, int_t max_len, int_t* num_cell)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCellInfo
 * name:         eSelector_selectCellInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCellInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCellInfo(CELLIDX idx, int_t i)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getSelecteCellInfo
 * name:         eSelector_getSelecteCellInfo
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getSelecteCellInfo
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getSelecteCellInfo(CELLIDX idx, char_t* name, int_t max_len)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_selectCelltypeInfoOfCell
 * name:         eSelector_selectCelltypeInfoOfCell
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_selectCelltypeInfoOfCell
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_selectCelltypeInfoOfCell(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getAttrValue
 * name:         eSelector_getAttrValue
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getAttrValue
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getAttrValue(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getAttrSizeIsValue
 * name:         eSelector_getAttrSizeIsValue
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getAttrSizeIsValue
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getAttrSizeIsValue(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getVarValue
 * name:         eSelector_getVarValue
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getVarValue
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getVarValue(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<ENTRY_FUNC>]# eSelector_getVarSizeIsValue
 * name:         eSelector_getVarSizeIsValue
 * global_name:  nTECSInfo_tTECSInfoAccessor_eSelector_getVarSizeIsValue
 * oneway:       false
 * #[</ENTRY_FUNC>]# */
ER
eSelector_getVarSizeIsValue(CELLIDX idx)
{
	ER		ercd = E_OK;
	CELLCB	*p_cellcb;
	if (VALID_IDX(idx)) {
		p_cellcb = GET_CELLCB(idx);
	}
	else {
		return(E_ID);
	} /* end if VALID_IDX(idx) */

	/* ここに処理本体を記述します #_TEFB_# */

	return(ercd);
}

/* #[<POSTAMBLE>]#
 *   これより下に非受け口関数を書きます
 * #[</POSTAMBLE>]#*/
