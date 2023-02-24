/**
 * File Name: cvi_base_type.h
 *
 * Version: V1.0
 *
 * Brief: base types.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description: 
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 * ====================================================================================*/
#ifndef _CVI_BASE_TYPE_H_
#define _CVI_BASE_TYPE_H_
#include <iostream>
#include <vector>
#include <list>
using namespace std;

#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nvrlog.h"

typedef struct
{
    const cvi_uint8 *data;
    cvi_uint32 size;
    cvi_uint32 index;
} sps_bit_stream;

typedef struct
{
    cvi_uint32 profile_idc;
    cvi_uint32 level_idc;
    
    cvi_uint32 width;
    cvi_uint32 height;
    cvi_uint32 fps;  //SPS maybe not define fps
} sps_info_struct;

class CVI_Object_C 
{
public:
	CVI_Object_C(){}
	virtual ~CVI_Object_C(){}
};

class CVI_Config_C
{
public:
    static CVI_ERROR_CODE_E CVI_Config_IniFileSave(string iniFilePath);
	static CVI_ERROR_CODE_E CVI_Config_IniGet(string iniFilePath, string section, string key, string* value);
    static CVI_ERROR_CODE_E CVI_Config_IniSet(string iniFilePath, string section, string key, string value);
	static CVI_ERROR_CODE_E CVI_Config_IniSetInTmp(string section, string key, string value);
	static CVI_ERROR_CODE_E CVI_Config_SizeConvert(cvi_uint32& actualSize, cvi_uint32& convertSize);
	static CVI_ERROR_CODE_E CVI_Config_TimeStampConvert(cvi_uint64& actualTimeStamp, cvi_uint64& convertTimeStamp);
	static cvi_int32 CVI_Config_H264ParseSps(const cvi_uint8* data, cvi_uint32 dataSize, sps_info_struct *info);
	CVI_Config_C(){};
	virtual ~CVI_Config_C(){};
};

class CVI_DataBase_C
{
public:
	CVI_DataBase_C(){};
	virtual ~CVI_DataBase_C(){};
};


class CVI_Rect_C
{
public:
	cvi_uint16 m_u16X,m_u16Y;
	cvi_uint16 m_u16Width,m_u16Height;
	CVI_Rect_C(cvi_uint16 X = 0, cvi_uint16 Y = 0, cvi_uint16 W = 0, cvi_uint16 H = 0) :\
        m_u16X(X),m_u16Y(Y),m_u16Width(W),m_u16Height(H)
	{

	}
};

class CVI_Point_C
{
public:
	cvi_uint16 m_u16X,m_u16Y;
	CVI_Point_C(cvi_uint16 x = 0, cvi_uint16 y = 0):m_u16X(x),m_u16Y(y)
	{

	}
};
class CVI_Polygon_C
{
public:
	vector<CVI_Point_C> m_olPtList;
	
};
class CVI_IntRange_C
{
public:
	cvi_int32 m_i32Min;
	cvi_int32 m_i32Max;
    
	CVI_IntRange_C(cvi_int32 min = 0, cvi_int32 max = 0):m_i32Min(min),m_i32Max(max)
    {

	}
};
class CVI_FloatRange_C
{
public:
	cvi_float m_fMin;
	cvi_float m_fMax;

	CVI_FloatRange_C(cvi_float min = 0, cvi_float max = 0):m_fMin(min),m_fMax(max)
	{

	}
};
class CVI_IntRectangleRange_C
{
public:
	CVI_IntRange_C m_oXRange;
	CVI_IntRange_C m_oYRange;
	CVI_IntRange_C m_oWidthRange;
	CVI_IntRange_C m_oHeightRange;

    CVI_IntRectangleRange_C(){}
};

#endif//_CVI_BASE_TYPE_H_
