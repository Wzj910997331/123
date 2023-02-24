/**
 * File Name: cvi_source.h
 *
 * Version: V1.0
 *
 * Brief: message/event/log's source.
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
#ifndef _CVI_SOURCE_H_
#define _CVI_SOURCE_H_
#include "cvi_common_type.h"
#include "cvi_base_type.h"
#include "cvi_platform.h"
#include "cvi_utility.h"
#include "cvi_const.h"
#include "cvi_nvrlog.h"
//#include "cvi_message.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum
{
	E_CVIBSV_SOURCE_MODULE_FIRST = 0,
	E_CVIBSV_SOURCE_MODULE_LOAPP = E_CVIBSV_SOURCE_MODULE_FIRST,
	E_CVIBSV_SOURCE_MODULE_REAPP,
	E_CVIBSV_SOURCE_MODULE_BASESVC,
	E_CVIBSV_SOURCE_MODULE_DEVSIGNAL,
	E_CVIBSV_SOURCE_MODULE_DEVMEDIA,
	E_CVIBSV_SOURCE_MODULE_TIMER,
	E_CVIBSV_SOURCE_MODULE_REMOTEPROB,
	E_CVIBSV_SOURCE_MODULE_NUM
}CVIBSV_SOURCE_MODULE_E;

class CVIBSV_MsgHeader_C
{
public:
	cvi_uint32 m_u32Id; // generate automatically(by system tick)
	CVI_MSGCLASS_E m_eClass;
	CVI_MSGTYPE_E m_eType;
	CVIBSV_MsgHeader_C()
	{
		m_u32Id = 0;
		m_eClass = E_CVI_MSGCLASS_COMMAND;
		m_eType = E_CVI_MSGTYPE_DEFAULT;
	}
	void DUMP()
	{
		CVI_NVRLOGD("m_u32Id:%d",m_u32Id);
		CVI_NVRLOGD("m_eClass:%d",m_eClass);
		CVI_NVRLOGD("m_eType:%d",m_eType);
	}
};
class CVIBSV_Source_C
{
public:
	cvi_bool m_bIsValid;
	//cvi_uint16 m_u16ChnNo; // what is this!!!
	string m_strIPV4;// empty is local
	//cvi_bool m_bIsLocal;
	cvi_uint64 m_u64UserId;
	
	CVIBSV_SOURCE_MODULE_E m_eSrcModule;
	CVIBSV_MsgHeader_C m_oOrgCmdHeader; // original command message header

	CVIBSV_Source_C()
	{
		m_bIsValid = CVI_FALSE;
		//m_u16ChnNo = 0;
		
		//m_bIsLocal = CVI_TRUE;
		m_u64UserId = 0;		
	}
	void DUMP()
	{
		CVI_NVRLOGD("=========================================================");
		CVI_NVRLOGD("m_bIsValid:%d",m_bIsValid);
		CVI_NVRLOGD("m_strIPV4:%s",m_strIPV4.c_str());
		CVI_NVRLOGD("m_u64UserId:%lu",m_u64UserId);
		CVI_NVRLOGD("m_eSrcModule:%d",m_eSrcModule);
		CVI_NVRLOGD("m_oOrgCmdHeader DUMP");
		m_oOrgCmdHeader.DUMP();
		CVI_NVRLOGD("=========================================================");		
	}
};

#ifdef __cplusplus
	}
#endif
#endif//_CVI_MESSAGE_H_
