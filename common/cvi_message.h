/**
 * File Name: cvi_message.h
 *
 * Version: V1.0
 *
 * Brief: command message and response message.
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
#ifndef _CVI_MESSAGE_H_
#define _CVI_MESSAGE_H_
#include "cvi_const.h"
#include "cvi_common_type.h"
#include "cvi_base_type.h"
#include "cvi_platform.h"
#include "cvi_utility.h"
#include "cvi_source.h"
#include "cvi_nvrlog.h"

#include <memory>
using namespace std;

//#ifdef __cplusplus
//	extern "C" {
//#endif





///////////////////////////////////////////////////////////////////////////
///  SIGNAL COMMAND
///////////////////////////////////////////////////////////////////////////
#define CVI_MSGTYPE_MODULE_SIGNAL_BASE			(20000)
#define CVI_MSGTYPE_MODULE_MEDIA_BASE			(22000)
#define CVI_MSGTYPE_MODULE_RECORD_BASE			(24000)
#define CVI_MSGTYPE_MODULE_DEVPROBE_BASE		(26000)
#define CVI_MSGTYPE_MODULE_SVCCMD_BASE			(28000)
#define CVI_MSGTYPE_MODULE_SVCEVENT_BASE		(30000)
#define CVI_MSGTYPE_MODULE_APNOTIFY_BASE		(32000)


class CVI_Msg_C
{
public:
	cvi_uint32 m_u32Id; // generate automatically(by system tick)
	CVI_MSGCLASS_E m_eClass;
	CVI_MSGTYPE_E m_eType;
	CVIBSV_Source_C m_oSrc;

    shared_ptr<CVI_Object_C> pObj;

	CVI_Msg_C()
	{
		// genrate the u32Id by system tick
		m_u32Id = CVI_Utility_C::GenerateUniqueId();
		m_eClass = E_CVI_MSGCLASS_DEFAULT;
		m_eType = E_CVI_MSGTYPE_DEFAULT;
    }	
    virtual ~CVI_Msg_C(){}
    
public:
    cvi_uint32 GetMagic() {return m_u32Magic;}

    template <typename T>
    static shared_ptr<T> MakeMsg() 
    {
        shared_ptr<T> p = make_shared<T>();
        p->SetMagic();
        return p;
    }
    virtual void DUMP()
    {
		CVI_NVRLOGD("=========================================================");
		CVI_NVRLOGD("m_u32Id:%d",m_u32Id);
		CVI_NVRLOGD("m_eClass:%d",m_eClass);
		CVI_NVRLOGD("m_eType:%d",m_eType);
		CVI_NVRLOGD("pObj:%p",pObj);
		CVI_NVRLOGD("m_u32Magic:%d",GetMagic());
		CVI_NVRLOGD("m_oSrc DUMP");
        m_oSrc.DUMP();
		CVI_NVRLOGD("=========================================================");
    }
private:
    cvi_uint32 m_u32Magic;
    void SetMagic() {m_u32Magic = 0x55AA;}
};

typedef CVI_ERROR_CODE_E (*CVI_EVENT_HANDLER_FN_T)(CVI_Msg_C*);

class CVI_RespMsg_C : public CVI_Msg_C
{
public:
	//CVI_MSGTYPE_E m_eSrcCmdType;
	CVI_ERROR_CODE_E m_eReturnCode;

	CVI_RespMsg_C()
	{
		//m_eSrcCmdType = E_CVI_MSGTYPE_DEFAULT;
		m_eReturnCode = E_CVI_ERROR_CODE_SUCC;
	}

    ~CVI_RespMsg_C()
    {

    }
    virtual void DUMP()
    {
		CVI_NVRLOGD("=========================================================");
		CVI_NVRLOGD("base DUMP=>");
        CVI_Msg_C::DUMP();
		CVI_NVRLOGD("m_eReturnCode:%d",m_eReturnCode);        
		CVI_NVRLOGD("=========================================================");
    }
};

class CVI_RespMsgQueue_C : public CVI_Queue_SC<shared_ptr<CVI_RespMsg_C>>
{
public:
    CVI_ERROR_CODE_E Push(shared_ptr<CVI_RespMsg_C> &pMsg)
    {
        if(0x55AA != pMsg->GetMagic())
        {
            LOG(ERROR) << "plase use CVI_Msg_C::MakeMsg<T>(); to make msg.";
            CHECK_EQ(0,1);
        }

        return CVI_Queue_SC<shared_ptr<CVI_RespMsg_C>>::Push(pMsg);
    }

    CVI_ERROR_CODE_E TryPush(shared_ptr<CVI_RespMsg_C> &pMsg, cvi_uint32 u32Timeout)
    {
        if(0x55AA != pMsg->GetMagic())
        {
            LOG(ERROR) << "plase use CVI_Msg_C::MakeMsg<T>(); to make msg.";
            CHECK_EQ(0,1);
        }

        return CVI_Queue_SC<shared_ptr<CVI_RespMsg_C>>::TryPush(pMsg, u32Timeout);
    }
};

class CVI_CmdMsg_C : public CVI_Msg_C
{
public:
	//CVI_Queue_SC<CVI_Msg_C> *m_poRespQueue;// SYNC:RespQ; ASYNC(return EVENT):EventQ; ASYNC(no return):NULL
    CVI_RespMsgQueue_C *m_poRespQueue;

	CVI_CmdMsg_C()
	{
		m_poRespQueue = NULL;
	}
    virtual void DUMP()
    {
		CVI_NVRLOGD("=========================================================");
		CVI_NVRLOGD("base DUMP=>");
        CVI_Msg_C::DUMP();
		CVI_NVRLOGD("m_poRespQueue:%p",m_poRespQueue);   
		CVI_NVRLOGD("=========================================================");
    }    
};

class CVI_CmdMsgQueue_C : public CVI_Queue_SC<shared_ptr<CVI_CmdMsg_C>>
{
public:
    CVI_ERROR_CODE_E Push(shared_ptr<CVI_CmdMsg_C> &pMsg)
    {
        if(0x55AA != pMsg->GetMagic())
        {
            LOG(ERROR) << "plase use CVI_Msg_C::MakeMsg<T>(); to make msg.";
            CHECK_EQ(0,1);
        }

        return CVI_Queue_SC<shared_ptr<CVI_CmdMsg_C>>::Push(pMsg);
    }

    CVI_ERROR_CODE_E TryPush(shared_ptr<CVI_CmdMsg_C> &pMsg, cvi_uint32 u32Timeout)
    {
        if(0x55AA != pMsg->GetMagic())
        {
            LOG(ERROR) << "plase use CVI_Msg_C::MakeMsg<T>(); to make msg.";
            CHECK_EQ(0,1);
        }

        return CVI_Queue_SC<shared_ptr<CVI_CmdMsg_C>>::TryPush(pMsg, u32Timeout);
    }
};




//#ifdef __cplusplus
//	}
//#endif
#endif//_CVI_MESSAGE_H_
