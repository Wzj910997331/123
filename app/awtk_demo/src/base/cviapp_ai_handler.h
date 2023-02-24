/**
 * File Name: cviapp_ai_handler.h
 *
 * Version: V1.0
 *
 * Brief: The header file for Ai handler funcitons.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2023. All rights reserved.
 *
 * Description:
 *
 * History          :
 * =========================================================================
   1.Date           :    2023/02/10
   Author           :    wentao.hu
   Modification     :    Created file
 * =========================================================================*/

#ifndef CVI_APP_AI_HANDLER_H_
#define CVI_APP_AI_HANDLER_H_

#ifdef ENABLE_CVIAI
#include "awtk.h"
#include "cviapp_interface.h"

#include "cviapp_ai.h"
//#include "cviapp_notify_handler.h"
#include "cviapp_typedef.h"

//ai_face_lib
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "core/core/cvai_core_types.h"
#include "core/utils/vpss_helper.h"

#define NVR_AI_DB "/mnt/data/nvr.db"

typedef struct
{
    uint8_t u8Chn;
    uint32_t size;
    CVIAPP_AI_MODEL_E eType;
    rect_t *pr;
    CVIAPP_MatchFace_S sMatchResult;  // face
    int inRegion[512];                // region
    void* pIpcView;
} CVIAPP_AiDrawRectMsg_S;

CVI_ERROR_CODE_E CVIAPP_AiHandlerInit(void);
CVI_ERROR_CODE_E CVIAPP_AiHandlerRelease(void);

CVI_ERROR_CODE_E CVIAPP_AiRegDrawRectNotify(void* pIpcView, uint8_t u8Chn,
                                            idle_func_t on_idle);
CVI_ERROR_CODE_E CVIAPP_AiUnregDrawRectNotify(uint8_t u8Chn);

CVIAPP_AiDrawRectMsg_S* CVIAPP_AiAllocDrawRectMsg(uint32_t size,
                                                  CVIAPP_AI_MODEL_E eType);

CVI_ERROR_CODE_E CVIAPP_AiFreeDrawRectMsg(CVIAPP_AiDrawRectMsg_S *pmsg);


#endif /* End of ENABLE_CVIAI */

#endif

