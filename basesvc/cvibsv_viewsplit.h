/**
 * File Name: cvibsv_view_split.h
 *
 * Version: V1.0
 *
 * Brief: cvi base services:view split's data structure&api 
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
 * ====================================================================================*/


#ifndef _CVIBSV_VIEWSPLIT_SETTING_H_
#define _CVIBSV_VIEWSPLIT_SETTING_H_
#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include "cvi_common_type.h"
#include "cvi_base_type.h"
#include "cvi_platform.h"
#include "cvi_const.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
///   VIEW SPLIT GLOBAL CONFIG
///////////////////////////////////////////////////////////////////////////
#define E_CVI_ERROR_CODE_MODULE_VIEWSPLIT_NOT_INIT						(CVI_ERROR_CODE_MODULE_VIEWSPLIT_BASE)
#define E_CVI_ERROR_CODE_MODULE_VIEWPORT_CHNBINDING_NOT_INIT			(CVI_ERROR_CODE_MODULE_VIEWSPLIT_BASE+1)

typedef enum
{
	E_CVI_VIEWSPLT_MODE_FIRST = 0,
	E_CVI_VIEWSPLT_MODE_PIC1 = E_CVI_VIEWSPLT_MODE_FIRST,
	E_CVI_VIEWSPLT_MODE_PIC2,
	E_CVI_VIEWSPLT_MODE_PIC4,
	E_CVI_VIEWSPLT_MODE_PIC6,
	E_CVI_VIEWSPLT_MODE_PIC8,
	E_CVI_VIEWSPLT_MODE_PIC9,
	E_CVI_VIEWSPLT_MODE_PIC16,
	E_CVI_VIEWSPLT_MODE_PIC25,
	E_CVI_VIEWSPLT_MODE_PIC32,
	E_CVI_VIEWSPLT_MODE_PIC36,
	E_CVI_VIEWSPLT_MODE_PIC64
}CVI_VIEWSPLT_MODE_E;

#define CVI_HDMI_VIEWSPLIT_MAXVP_NUM 36
#define CVI_HDMI_VIEWSPLIT_PATN_MODE_MAXNO E_CVI_VIEWSPLT_MODE_PIC9

#define CVI_VGA_VIEWSPLIT_MAXVP_NUM 36
#define CVI_VGA_VIEWSPLIT_PATN_MODE_MAXNO E_CVI_VIEWSPLT_MODE_PIC9

#define CVI_VIEWSPLIT_STD_PATN_MODE_MAXNUM \
	(CVI_HDMI_VIEWSPLIT_PATN_MODE_MAXNO>CVI_VGA_VIEWSPLIT_PATN_MODE_MAXNO?CVI_HDMI_VIEWSPLIT_PATN_MODE_MAXNO:CVI_VGA_VIEWSPLIT_PATN_MODE_MAXNO)
#define CVI_VIEWSPLIT_USRDEF_PATN_MODE_MAXNUM 8

#define CVI_VIEWSPLIT_MAXVP_NUM \
	(CVI_HDMI_VIEWSPLIT_MAXVP_NUM>CVI_VGA_VIEWSPLIT_MAXVP_NUM?CVI_HDMI_VIEWSPLIT_MAXVP_NUM:CVI_VGA_VIEWSPLIT_MAXVP_NUM)

class CVIBSV_ViewSplitPattern_C
{
public:
	cvi_uint8 GetVPortNum();
	string GetName();
	CVI_ERROR_CODE_E SetName(string strName);

	// oPRect:parent's View Rect
	void GetVPortParams(CVI_Rect_C oPRect,vector<CVI_Rect_C> &oRectList); 
	CVI_ERROR_CODE_E MergeVPort(vector<cvi_uint8> u8lVPIdxList);
	void Reset(CVIBSV_ViewSplitPattern_C oPatn);

	CVIBSV_ViewSplitPattern_C(){};
	~CVIBSV_ViewSplitPattern_C(){};

private:
	string m_strName;
	vector<CVI_Rect_C> m_olVPList;
};


class CVIBSV_ViewSplitPatternSet_C
{
public:
	cvi_uint8 GetCount();
	CVI_ERROR_CODE_E Del(cvi_uint8 u8PatnNo);
	CVI_ERROR_CODE_E Add(CVIBSV_ViewSplitPattern_C oPatn);
	CVI_ERROR_CODE_E Revise(cvi_uint8 u8PatnNo,CVIBSV_ViewSplitPattern_C oPatn);
	CVI_ERROR_CODE_E Get(cvi_uint8 u8PatnNo,CVIBSV_ViewSplitPattern_C &oPatn);

private:
	vector<CVIBSV_ViewSplitPattern_C> m_olList;
};

class CVIBSV_ViewSplitPatternSetSettingCfg_GSC : public CVI_Config_C
{
public:
	static CVI_ERROR_CODE_E Init();
	static CVI_ERROR_CODE_E Deinit();
	static CVI_ERROR_CODE_E GetStdPatnSet(CVIBSV_ViewSplitPatternSet_C &oPatnSet);
	static CVI_ERROR_CODE_E GetUsrDefPatnSet(CVIBSV_ViewSplitPatternSet_C &oPatnSet);
	static CVI_ERROR_CODE_E SetStdPatnSet(CVIBSV_ViewSplitPatternSet_C oPatnSet);
	static CVI_ERROR_CODE_E SetUsrDefPatnSet(CVIBSV_ViewSplitPatternSet_C oPatnSet);

private:
	static cvi_bool m_bIsInit;
	static CVI_Mutex_SC m_oMutex;
	static CVIBSV_ViewSplitPatternSet_C m_oStdPatnSet;
	static CVIBSV_ViewSplitPatternSet_C m_oUserDefPatnSet;
};

class CVIBSV_ViewPortChnBindingSet_C
{
public:
	cvi_uint8 GetVPNum();
	cvi_uint16 GetChannelNo(cvi_uint8 u8ViewNo);

	void SetChannelNo(cvi_uint8 u8ViewNo,cvi_uint16 u16ChnNo);
	void ResetAllChannelNo();
	void SetAllChannelNo();
	//TODO
	
	CVIBSV_ViewPortChnBindingSet_C()
	{
	    //cvi_int32 i;
		m_u16lViewPortChnNoList.empty();
		/*
		for(i=0;i<CVI_VIEWPORT_NUM;i++)
		{
			m_u16lViewPortChnNoList.at(i) = 0;
		}
		*/
	}
	
private:
	vector<cvi_uint16> m_u16lViewPortChnNoList;
};

// define the view idx's response channel no
class CVIBSV_ViewPortChnBindingSettingCfg_GSC : public CVI_Config_C
{
public:
	static CVI_ERROR_CODE_E Init();
	static CVI_ERROR_CODE_E Deinit();
	static CVI_ERROR_CODE_E GetHdmiViewPortChnBindingSet(CVIBSV_ViewPortChnBindingSet_C &oParams);
	static CVI_ERROR_CODE_E SetHdmiViewPortChnBindingSet(CVIBSV_ViewPortChnBindingSet_C oParams);
	static CVI_ERROR_CODE_E GetVGAViewPortChnBindingSet(CVIBSV_ViewPortChnBindingSet_C &oParams);
	static CVI_ERROR_CODE_E SetVGAViewPortChnBindingSet(CVIBSV_ViewPortChnBindingSet_C oParams);
private:
	static cvi_bool m_bIsInit;
	static CVI_Mutex_SC m_oMutex;
	static CVIBSV_ViewPortChnBindingSet_C m_oHdmiViewPortChnNoSet;
	static CVIBSV_ViewPortChnBindingSet_C m_oVGAViewPortChnNoSet;
};

#ifdef __cplusplus
	}
#endif

#endif//_CVIBSV_VIEWSPLIT_SETTING_H_
