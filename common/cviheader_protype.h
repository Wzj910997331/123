/**
 * File Name: cviap_xxxx.h(application level)/cvibsv_xxxx.h(base service level)
 *            cvifrk_xxxx.h(framework level)/cvicmp_xxxx.h(component level)
 *
 * Version: V1.0
 *
 * Brief: brief describe the file's functionality.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description: more detail descriptions.
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
   2.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	add file header
 * ====================================================================================*/

#ifndef _CVIAP_WIN_PREVIEW_H_
#define _CVIAP_WIN_PREVIEW_H_
#include "cvi_common_type.h"
// define function prototype
typedef void CVIXX_TypeName_F(cvi_int16 a,cvi_int16 b);

// define enumerations
// CVIXX could be CVIAP/CVIBSV/CVIMW
typedef enum _CVIXX_TYPENAME_E {
	E_CVIXX_TYPENAME_FIRST = 0,

	/**
	* @const E_CVIXX_TYPENAME_1
	* descriptions...
	*/
	E_CVIXX_TYPENAME_1 = E_CVIXX_TYPENAME_FIRST,

	/**
	* @const E_CVIXX_TYPENAME_2
	* descriptions...
	*/
	E_CVIXX_TYPENAME_2,
	
	/**
	* @const E_CVIXX_TYPENAME_N
	* descriptions...
	*/
	E_CVIXX_TYPENAME_N,

	E_CVIXX_TYPENAME_NUM
} CVIXX_TYPENAME_E;

// define class
// CVIXX could be CVIAP/CVIBSV/CVIMW
class CVIXX_TypeName_C{
	cvi_int8 i8Name;// desc:or i16Name,i32Name,i64Name
};

// define structures
// CVIXX could be CVIAP/CVIBSV/CVIMW
typedef struct _CVIXX_TypeName1_S {
	cvi_int8 i8Name;// desc:or i16Name,i32Name,i64Name
} CVIXX_TypeName1_S;

typedef struct _CVIXX_TypeName2_S {
	cvi_int8 i8Name;// desc:or i16Name,i32Name,i64Name
	cvi_uint8 u8Name; // desc:or u16Name,u32Nmae,u64Name
	cvi_int8 *pi8Name;// desc:or pi16Name,pi32Name,pi64Name
	cvi_uint8 *pu8Name;// desc:or pu16Name,pu32Name,pu64Name
	cvi_int8 ai8Name[32];// desc:a mean array
	
	CVIXX_TypeName1_S stName; // desc:st mean structure
	CVIXX_TypeName1_S *pstName; // desc:pst mean pointer to structure
	
	CVIXX_TypeName_C oName; // desc: o mean object
	CVIXX_TypeName_C *poName; // desc: po mean pointer to object

	CVIXX_TypeName_F fnName; // desc: fn mean function
	CVIXX_TypeName_F *pfnName; // desc: pfn mean pointer to function
} CVIXX_TypeName2_S;

// define APIs
// CVIXX could be CVIAP/CVIBSV/CVIMW
cvi_int16 CVIXX_FunctionName(cvi_int16,cvi_int8);

#endif//_CVIAP_WIN_PREVIEW_H_
