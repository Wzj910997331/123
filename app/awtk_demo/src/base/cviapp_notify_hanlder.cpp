/**
 * File Name: cviapp_notify_handler.cpp
 *
 * Version: V1.0
 *
 * Brief:
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/12/08
   Author 			:   mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#include "cviapp_notify_handler.h"
#include "cviapp_notify_handler2.h"

#define GET_EXT_CMDMSG(pMsg, extMsgType)                                        \
    shared_ptr<extMsgType> pExtMsg = dynamic_pointer_cast<extMsgType>(pMsg);    \
    CVI_NVRLOG_ASSERT(pExtMsg != NULL)


