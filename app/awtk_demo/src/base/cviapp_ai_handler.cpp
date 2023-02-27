/**
 * File Name: cviapp_ai_handler.cpp
 *
 * Version: V1.0
 *
 * Brief: Some handler funcitons for Ai feature.
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

#ifdef ENABLE_CVIAI

#include "cviapp_ai_handler.h"
#include "cviapp_ai.h"

static idle_func_t g_chnNotifyList[CVIAPP_PREVIEW_MAX_IPC_VIEW];
static void *g_ipcViewPointer[CVIAPP_PREVIEW_MAX_IPC_VIEW];

#define AI_ONE_THREAD 1

static CVI_Mutex_SC __oMutex;

static inline void __scaleCviAiBbox2Rect(cvai_bbox_t *piBox, uint32_t i_w,
                                         uint32_t i_h, uint32_t o_w,
                                         uint32_t o_h, rect_t *por) {
  if (i_w != o_w && i_h != o_h) {
    xy_t x1 = ((float)o_w / (float)i_w) * piBox->x1;
    xy_t x2 = ((float)o_w / (float)i_w) * piBox->x2;

    xy_t y1 = ((float)o_h / (float)i_h) * piBox->y1;
    xy_t y2 = ((float)o_h / (float)i_h) * piBox->y2;

    por->x = x1;
    por->y = y1;

    por->w = x2 - x1;
    por->h = y2 - y1;
  } else {
    por->x = (xy_t)piBox->x1;
    por->y = (xy_t)piBox->y1;

    por->w = (wh_t)((xy_t)piBox->x2 - (xy_t)piBox->x1);
    por->h = (wh_t)((xy_t)piBox->y2 - (xy_t)piBox->y1);
  }
}

static uint8_t __aiGetFrameCallback(uint8_t u8Chn,
                                    VIDEO_FRAME_INFO_S *pstVideoFrameInfo) {
  CVIAPP_RtspPlayer_C *pPlayer = NULL;
  int ret = 0;

  pPlayer = CVIAPP_GetRtspPlayerByCh(u8Chn);
  if (NULL == pPlayer) {
    return -1;
  }

  ret = pPlayer->AIGetVideoFrame(pstVideoFrameInfo);
  if (E_CVI_ERROR_CODE_SUCC == ret) {
    return 0;
  } else {
    return -1;
  }
}

static void __aiReleaseFrameCallback(uint8_t u8Chn,
                                     VIDEO_FRAME_INFO_S *pstVideoFrameInfo) {
  CVIAPP_RtspPlayer_C *pPlayer = NULL;

  if (NULL == pstVideoFrameInfo) {
    return;
  }

  pPlayer = CVIAPP_GetRtspPlayerByCh(u8Chn);
  if (NULL == pPlayer) {
    return;
  }

  pPlayer->ReleaseVideoFrame(pstVideoFrameInfo);
}

static void __aiNotifyCallback(uint8_t u8Chn, CVIAPP_AiResult_S *pResult) {
  CVIAPP_AiDrawRectMsg_S *pmsg = NULL;
  uint32_t i = 0;

  if (NULL == pResult) {
    CVI_NVRLOGE("ERROR: NULL ai result");
    return;
  }

  __oMutex.Lock();

  switch (pResult->eType) {
  case CVIAPP_AI_FACE_CAPTURE:

    break;
  case CVIAPP_AI_OBJECT_DETECTION:
    if ((NULL != g_chnNotifyList[u8Chn]) && (NULL != g_ipcViewPointer[u8Chn])) {
      cvai_object_t *resultInfo = NULL;
      resultInfo = (cvai_object_t *)pResult->pResult;
      pmsg = CVIAPP_AiAllocDrawRectMsg(resultInfo->size);
      if ((NULL == resultInfo) || (NULL == pmsg)) {
        break;
      }
      printf("resultInfo->size = %d\n", resultInfo->size);
      for (i = 0; i < resultInfo->size; i++) {
        pmsg->pr[i].x = (uint32_t)resultInfo->info[i].bbox.x1;
        pmsg->pr[i].y = (uint32_t)resultInfo->info[i].bbox.y1;

        pmsg->pr[i].w = (uint32_t)(resultInfo->info[i].bbox.x2 -
                                   resultInfo->info[i].bbox.x1);
        pmsg->pr[i].h = (uint32_t)(resultInfo->info[i].bbox.y2 -
                                   resultInfo->info[i].bbox.y1);
      }

      pmsg->u8Chn = u8Chn;
      pmsg->pIpcView = g_ipcViewPointer[u8Chn];

      idle_queue(g_chnNotifyList[u8Chn], (void *)pmsg);
    }
    break;
  case CVIAPP_AI_FACE_DETECION: {
    if ((NULL != g_chnNotifyList[u8Chn]) && (NULL != g_ipcViewPointer[u8Chn])) {
      CVIAPP_AiResultInfo_S *resultInfo = NULL;
      resultInfo = (CVIAPP_AiResultInfo_S *)pResult->pResult;
      pmsg = CVIAPP_AiAllocDrawRectMsg(resultInfo->face.size);
      if ((NULL == resultInfo) || (NULL == pmsg)) {
        break;
      }
      for (i = 0; i < resultInfo->face.size; i++) {
        /* Scale face box to adapter the preview screen */
        __scaleCviAiBbox2Rect(&resultInfo->face.info[i].bbox,
                              resultInfo->face.width, resultInfo->face.height,
                              CVIAPP_DEFAULT_GUI_WIDTH,
                              CVIAPP_DEFAULT_GUI_HEIGHT, &pmsg->pr[i]);
      }

      pmsg->sMatchResult = (CVIAPP_MatchFace_S)resultInfo->sMatchResult;
      pmsg->u8Chn = u8Chn;
      pmsg->pIpcView = g_ipcViewPointer[u8Chn];

      idle_queue(g_chnNotifyList[u8Chn], (void *)pmsg);
    }
  } break;

  default:
    break;
  }

  __oMutex.Unlock();
}

/**
 * To init Ai handler And start Ai
 *
 * mantis.0010798 wentao.hu 20230210
 *
 * \return: the enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_AiHandlerInit(void) {
  CVIAPP_AiInit();

  for (uint8_t i = 0; i < CVIAPP_PREVIEW_MAX_IPC_VIEW; i++) {
    g_ipcViewPointer[i] = NULL;
    g_chnNotifyList[i] = NULL;
  }

  CVIAPP_AiParam_S param;

  /*face cature*/
  memset(&param, 0, sizeof(CVIAPP_AiParam_S));
  param.eType = CVIAPP_AI_FACE_CAPTURE;
  param.pModelPath = FACE_RETINA_PATH;
  // param.pNotifyFun = __aiNotifyCallback;
  param.pGetFrameFun = __aiGetFrameCallback;
  param.pReleaseFrameFun = __aiReleaseFrameCallback;
  CVIAPP_AiStart(param);

  /*face_dection*/
  memset(&param, 0, sizeof(CVIAPP_AiParam_S));
  param.eType = CVIAPP_AI_FACE_DETECION;
  param.pModelPath = FACE_RETINA_PATH;
  param.pNotifyFun = __aiNotifyCallback;
  param.pGetFrameFun = __aiGetFrameCallback;
  param.pReleaseFrameFun = __aiReleaseFrameCallback;
  CVIAPP_AiStart(param);

  // /*obj decetion*/
  // memset(&param, 0, sizeof(CVIAPP_AiParam_S));
  // param.eType = CVIAPP_AI_OBJECT_DETECTION;
  // param.pModelPath = PD_MODEL_PATH;
  // param.pNotifyFun = __aiNotifyCallback;
  // param.pGetFrameFun = __aiGetFrameCallback;
  // param.pReleaseFrameFun = __aiReleaseFrameCallback;
  // CVIAPP_AiStart(param);

  return E_CVI_ERROR_CODE_SUCC;
}

/**
 * To release Ai handler And stop Ai
 *
 * mantis.0010798 wentao.hu 20230210
 *
 * \return: the enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_AiHandlerRelease(void) {

  for (uint8_t i = 0; i < CVIAPP_PREVIEW_MAX_IPC_VIEW; i++) {
    g_ipcViewPointer[i] = NULL;
    g_chnNotifyList[i] = NULL;
  }

  CVIAPP_AiStop();

  return E_CVI_ERROR_CODE_SUCC;
}

/**
 * To register notify function for drawing ai rect
 *
 * mantis.0010798 wentao.hu 20230210
 *
 * \input pIpcView: The preview win;
 * \input u8Chn: The channel id of preview;
 * \input on_idle: The notify function;
 * \return: the enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_AiRegDrawRectNotify(void *pIpcView, uint8_t u8Chn,
                                            idle_func_t on_idle) {
  if (u8Chn > CVIAPP_PREVIEW_MAX_IPC_VIEW) {
    CVI_NVRLOGE("ERROR: out of max channel id");
    return E_CVI_ERROR_CODE_FAULT;
  }

  __oMutex.Lock();

  g_ipcViewPointer[u8Chn] = pIpcView;
  g_chnNotifyList[u8Chn] = on_idle;

  __oMutex.Unlock();

  return E_CVI_ERROR_CODE_SUCC;
}

/**
 * To unregister notify function for drawing ai rect
 *
 * mantis.0010798 wentao.hu 20230210
 *
 * \input u8Chn: The channel id of preview;
 * \return: the enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_AiUnregDrawRectNotify(uint8_t u8Chn) {
  if (u8Chn > CVIAPP_PREVIEW_MAX_IPC_VIEW) {
    CVI_NVRLOGE("ERROR: out of max channel id");
    return E_CVI_ERROR_CODE_FAULT;
  }

  __oMutex.Lock();

  g_chnNotifyList[u8Chn] = NULL;
  g_ipcViewPointer[u8Chn] = NULL;

  __oMutex.Unlock();

  return E_CVI_ERROR_CODE_SUCC;
}

/**
 * To Alloc rect msg
 * mantis.0010798 wentao.hu 20230210
 * \input size: The request values;
 * \input eType: The request values;
 * \return: The point of draw rect msg struct.
 */
CVIAPP_AiDrawRectMsg_S *CVIAPP_AiAllocDrawRectMsg(uint32_t size) {
  CVIAPP_AiDrawRectMsg_S *p = NULL;
  p = (CVIAPP_AiDrawRectMsg_S *)malloc(sizeof(CVIAPP_AiDrawRectMsg_S));

  if (NULL == p) {
    return NULL;
  }
  p->size = size;

  p->pr = (rect_t *)malloc(sizeof(rect_t) * size);

  return p;
}

/**
 * To Free rect msg
 *
 * mantis.0010798 wentao.hu 20230210
 *
 * \input pmsg: The msg we will frees;
 * \return: The enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_AiFreeDrawRectMsg(CVIAPP_AiDrawRectMsg_S *pmsg) {
  if (NULL == pmsg) {
    CVI_NVRLOGE("ERROR: NULL pmsg...");
    return NULL;
  }

  if (NULL == pmsg->pr) {
    CVI_NVRLOGE("ERROR: NULL pmsg...");
    return NULL;
  }

  free(pmsg->pr);
  free(pmsg);

  return E_CVI_ERROR_CODE_SUCC;
}

#endif /* End of ENABLE_CVIAI */
