/**
 * File Name: cvi_thread.h
 *
 * Version: V1.0
 *
 * Brief: thread.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 *
 =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 *
 ====================================================================================*/
#ifndef _CVI_THREAD_H_
#define _CVI_THREAD_H_
#include <iostream>
#include <vector>
using namespace std;

#include "cvi_common_type.h"
#include "cvi_const.h"
#include "cvi_message.h"
#include "cvi_platform.h"
#include "cvi_queue_sc.h"

#ifdef __cplusplus
extern "C" {
#endif

class CVI_CmdThread_SC : public CVI_Thread_SC {
public:
  CVI_CmdThread_SC(CVI_THREAD_ROUTINE_FN_T pFn, void *pParam,
                   const char *pThreadName = "cmd_thread");
  virtual ~CVI_CmdThread_SC();

public:
  // sync/async or direct call
  virtual CVI_ERROR_CODE_E
  SendCmd(shared_ptr<CVI_CmdMsg_C> poMsg); // directly call
  virtual CVI_ERROR_CODE_E PostCmd(shared_ptr<CVI_CmdMsg_C> poMsg);

protected:
  CVI_CmdMsgQueue_C m_oCmdQueue;
  CVI_RespMsgQueue_C m_oRespQueue;

private:
  CVI_THREAD_ROUTINE_FN_T m_pFn;
};

#ifdef __cplusplus
}
#endif
#endif //_CVI_THREAD_H_
