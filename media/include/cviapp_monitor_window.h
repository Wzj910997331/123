/**
 * File Name: cviapp_monitor_window.h
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
   1.Date 			:	2020/12/11
   Author 			:	mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_MONITOR_WINDOW_H_
#define CVIAPP_MONITOR_WINDOW_H_

#include <atomic>

#include "cvi_vdec.h"

#include "cvi_comm_video.h"
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nvrlog.h"

#define CVIAPP_MONITOR_MAX_CHN  9

typedef enum
{
    E_CVIAPP_WM_TYPE_FULL_SCREEN = 1,
    E_CVIAPP_WM_TYPE_2_SPLIT = 2,
    E_CVIAPP_WM_TYPE_4_SPLIT = 4,
    E_CVIAPP_WM_TYPE_6_SPLIT = 6,
    E_CVIAPP_WM_TYPE_8_SPLIT = 8,
    E_CVIAPP_WM_TYPE_9_SPLIT = 9,
    E_CVIAPP_WM_TYPE_CUSTOM,
    E_CVIAPP_WM_TYPE_MAX
} CVIAPP_MonitorWindowType_E;

typedef struct
{
    uint8_t m_u8Priority;  /* not use yet */
    RECT_S  m_stRect;
} CVIAPP_MonitorWindowChnParam_S;

typedef struct
{
    VO_DEV m_voDev;
    SIZE_S m_voSize;

    vector<CVIAPP_MonitorWindowChnParam_S> m_stlChnParam;
} CVIAPP_MonitorWindowParam_S;

class CVIAPP_MonitorWindow_C
{
public:
    CVIAPP_MonitorWindow_C();
    ~CVIAPP_MonitorWindow_C();

    /* only call once in system init */
    static CVI_ERROR_CODE_E InitMonitorWindowParam(const CVIAPP_MonitorWindowParam_S& param);

    static CVI_ERROR_CODE_E ReInitMonitorWindowParam(const CVIAPP_MonitorWindowParam_S& param);

    /* use custom param */
    static CVI_ERROR_CODE_E SetMonitorWindowParam(const CVIAPP_MonitorWindowParam_S& param);

    /* use preset param */
    static CVI_ERROR_CODE_E SetMonitorWindowType(CVIAPP_MonitorWindowType_E eType);
    static CVI_ERROR_CODE_E SortMonitorWindowByList(vector<uint8_t> sortLists);

    static CVI_ERROR_CODE_E UpdateMonitorWindowRect(const RECT_S& stRect);

    static CVIAPP_MonitorWindowType_E GetMonitorWindowType();

    static bool IsMonitorWindowReady();

    static void __SetVpssBgColor();
    static void SetVpssBgColor();

    static void Enable();
    static void Disable();

    static CVI_ERROR_CODE_E SendChnFrame(cvi_uint8 chnID, VIDEO_FRAME_INFO_S* pstVideoFrame);
    static CVI_ERROR_CODE_E __SendChnFrame(cvi_uint8 chnID, VIDEO_FRAME_INFO_S* pstVideoFrame,bool  isGetFrame);

private:
    static CVIAPP_MonitorWindow_C m_Instance;
    static CVIAPP_MonitorWindow_C *m_pInstance;

    std::atomic<bool> m_bMonitorThreadEnable;
    CVI_Thread_SC* m_pMonitorThread;
    static void *__MonitorThread(void *params);
    bool m_isVdecRefreshEnable[CVIAPP_MONITOR_MAX_CHN];
    bool m_isGetFrame[CVIAPP_MONITOR_MAX_CHN];

    vector<VIDEO_FRAME_INFO_S* > m_polChnParam;
};

#endif

