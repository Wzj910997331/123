/**
 * File Name: cviapp_rtsp_player.h
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
   1.Date 			:	2020/12/02
   Author 			:	mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_RTSP_PLAYER_H_
#define CVIAPP_RTSP_PLAYER_H_

#include <atomic>

#include "cvi_comm_video.h"
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nbuffer.h"
#include "cvi_nvrlog.h"

#include "cviapp_monitor_window.h"

#include "cviapp_rtsp_player_manager.h"

/* see ao_init (u32PtNumPerFrm * AUDIO_SOUND_MODE_STEREO * AUDIO_BIT_WIDTH_16) */
//#define CVI_AO_SENDFRAME_SIZE  1024
//#define CVI_AO_SENDFRAME_SIZE  4096
#define CVI_AO_SENDFRAME_SIZE  3840

typedef enum
{
    E_CVIAPP_RTSPPLAY_FRAME_STATE_IDLE,
    E_CVIAPP_RTSPPLAY_FRAME_STATE_VDECING,
    E_CVIAPP_RTSPPLAY_FRAME_STATE_VDECED,
    E_CVIAPP_RTSPPLAY_FRAME_STATE_VPSSVOING,
    E_CVIAPP_RTSPPLAY_FRAME_STATE_VPSSVOED,
    E_CVIAPP_RTSPPLAY_FRAME_STATE_MAX
}CVIAPP_RTSPPLAY_FRAME_STATE_E;

class CVIAPP_RtspPlayer_C
{
public:
    CVIAPP_RtspPlayer_C(const CVIAPP_RtspPlayerParam_S &param, CVIAPP_RtspPlayer_C* lastRtspPlayer = NULL,\
                                    CVIAPP_RTSPPLAY_MODE_E eMode = E_CVIAPP_RTSPPLAY_MODE_REAL_TIME);
    ~CVIAPP_RtspPlayer_C();

    bool IsVideoReady();

    void SetVideoMute(bool status);
    bool GetVideoMute();

    void SetAudioMute(bool status);
    bool GetAudioMute();

    void TakePhoto();

    bool UpdateMonitorWindowId(const uint8_t winId);
    bool UpdateChnId(const uint8_t chnId);


    CVI_ERROR_CODE_E Pause();
    CVI_ERROR_CODE_E Resume();

    CVI_ERROR_CODE_E StopPlay();

    CVIAPP_RTSPPLAY_STATUS_E GetStatus();

    uint64_t GetTimeMs();

    CVI_ERROR_CODE_E AIGetVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
    CVI_ERROR_CODE_E GetVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
    CVI_ERROR_CODE_E ReleaseVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrameInfo);

    CVI_ERROR_CODE_E SetPlayMode(CVIAPP_RTSPPLAY_MODE_E eMode);

    CVI_ERROR_CODE_E SetStartVideoPts(uint64_t startVideoPts);

    /* Set play speed like x2, 1/2, mantis.0006999 wentao.hu 20220218 */
    CVI_ERROR_CODE_E SetPlaySpeed(CVIAPP_RTSPPLAYER_PLAYSPEED_E ePlaySpeed);
    /*
     * Drop frame or delay for speed x2,1/2,
     * mantis.0006999 wentao.hu 20220218
     */
    bool isDoneDropOrDelay(VIDEO_FRAME_INFO_S *pstVdecFrame);

    CVIAPP_RTSPPLAYER_PLAYSPEED_E GetPlaySpeed();

private:
    void Init();

    CVI_ERROR_CODE_E VdecInit();
    CVI_ERROR_CODE_E VdecDeInit();

    CVI_ERROR_CODE_E JpegEncInit();
    CVI_ERROR_CODE_E JpegEncDeInit();

    CVI_ERROR_CODE_E VdecPrintStatus();

private:
    CVIAPP_RtspPlayer_C* m_lastRtspPlayer;
    CVIAPP_RtspPlayerParam_S m_stParam;

    std::atomic<bool> m_bIsSendChnFrameReady;
    std::atomic<bool> m_bEnableSendChnFrame;
    std::atomic<bool> m_bTakePhoto;

    std::atomic<CVIAPP_RTSPPLAY_STATUS_E> m_eStatus;

    bool m_bStartVideoPtsCustom;
    uint64_t m_u64StartVideoPtsCustom;
    uint64_t m_u64StartVideoPts;
    std::atomic<uint64_t> m_u64VideoPts;
    std::atomic<bool> m_bVdecThradEnable;
    CVI_Thread_SC* m_pVdecThread;
    static void *__vdecThread(void *params);
    static void *___vdecThread(void *params);

    std::atomic<bool> m_bEnableAudio;

    uint64_t m_u64StartAudioPts;
    std::atomic<uint64_t> m_u64AudioPts;
    std::atomic<bool> m_bAdecThradEnable;
    CVI_Thread_SC* m_pAdecThread;
    static void *__adecThread(void *params);

    float m_fvtb;
    float m_fatb;

    CVI_Mutex_SC m_oGetVideoFrameMutex;
    bool m_bGetVideoFrame;
    VIDEO_FRAME_INFO_S* m_pstVideoFrameInfo;
    CVI_Semaphore_SC* m_pGetVideoFrameSem;

    uint8_t m_u8ReleaseStatus;
    bool m_bAiGetFrame;

    std::atomic<CVIAPP_RTSPPLAY_MODE_E> m_ePlayMode;

    /* The play speed for this player */
    CVIAPP_RTSPPLAYER_PLAYSPEED_E m_ePlaySpeed;
    /* The count num of current video Frame */
    uint64_t m_u64CurrVideoFrameNum;
};

#endif
