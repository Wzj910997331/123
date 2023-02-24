/**
 * File Name: cvi_app_rtsp_player_manager.h
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
   1.Date 			:	2021/01/15
   Author 			:   mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_RTSP_PLAYER_MANAGER_H_
#define CVIAPP_RTSP_PLAYER_MANAGER_H_

#include "cvi_comm_video.h"
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nbuffer.h"
#include "cvi_nvrlog.h"

#define CVIAPP_RTSPPLAY_MAX_CHN  9

/* FPS val, mantis.0006999 wentao.hu 20220218 */
#define CVIAPP_RTSP_PLAYER_FPS_25    25
#define CVIAPP_RTSP_PLAYER_FPS_30    30

typedef enum
{
    E_CVIAPP_RTSPPLAY_MODE_REAL_TIME,
    E_CVIAPP_RTSPPLAY_MODE_AVERAGE,
    E_CVIAPP_RTSPPLAY_MODE_SMOOTH,
    E_CVIAPP_RTSPPLAY_MODE_NO_ACTION,
    E_CVIAPP_RTSPPLAY_MODE_MAX
}CVIAPP_RTSPPLAY_MODE_E;

typedef enum
{
    E_CVIAPP_RTSPPLAY_IDLE,
    E_CVIAPP_RTSPPLAY_STARTING,
    E_CVIAPP_RTSPPLAY_START_READY,

    E_CVIAPP_RTSPPLAY_PAUSE,

    E_CVIAPP_RTSPPLAY_STOPING,
    E_CVIAPP_RTSPPLAY_STOP_FINISH,

    E_CVIAPP_RTSPPLAY_MAX
} CVIAPP_RTSPPLAY_STATUS_E;

/**
 * The enum for play speed of rtspPlayer
 * mantis.0006999 wentao.hu 20220218
 */
typedef enum
{
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x1d16 = 0, /* x(1/16) */
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x1d8, /* x(1/8) */
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x1d4, /* x(1/4) */
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x1d2, /* x(1/2) */
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x1, /* x1 */
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x2,
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x3,
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x4,
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x8,
    E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x16,
    E_CVIAPP_RTSPPLAYER_PLAYSPEED_MAX = E_CVIAPP_RTSP_PLAYER_PLAYSPEED_x16
} CVIAPP_RTSPPLAYER_PLAYSPEED_E;

typedef struct
{
    uint8_t m_u8Chn;
    uint8_t m_u8MonitorWindowChnID;

    PAYLOAD_TYPE_E m_enType;

    uint32_t m_u32InputVideoWidth;
    uint32_t m_u32InputVideoHeight;

    float m_fps;

    CVI_RingBuffer_SC* m_pVideoRingBuffer;
    CVI_RingBuffer_SC* m_pAudioRingBuffer;

    void* m_pVideoStream;  /* ffmpeg AVStream* */
    void* m_pAudioStream;  /* ffmpeg AVStream* */

    string m_strPhotoPath;
} CVIAPP_RtspPlayerParam_S;

class CVIAPP_RtspPlayerManager_C
{
public:
    CVIAPP_RtspPlayerManager_C();
    ~CVIAPP_RtspPlayerManager_C();

    static CVIAPP_RtspPlayerManager_C *GetInstance();

    CVI_ERROR_CODE_E StartRtspPlayer(CVIAPP_RtspPlayerParam_S &param);
    CVI_ERROR_CODE_E StopRtspPlayer(uint8_t u8Chn);

    bool IsVideoReady(uint8_t u8Chn);

    CVI_ERROR_CODE_E SetVideoMute(uint8_t u8Chn, bool bStatus);
    bool GetVideoMute(uint8_t u8Chn);

    CVI_ERROR_CODE_E SetVideoMuteAll(bool bStatus);

    CVI_ERROR_CODE_E SetAudioMute(uint8_t u8Chn, bool bStatus);
    bool GetAudioMute(uint8_t u8Chn);

    CVI_ERROR_CODE_E SetAudioMuteAll(bool bStatus);

    CVI_ERROR_CODE_E TakePhoto(uint8_t u8Chn);

    CVI_ERROR_CODE_E Pause(uint8_t u8Chn);
    CVI_ERROR_CODE_E Resume(uint8_t u8Chn);

    CVIAPP_RTSPPLAY_STATUS_E GetStatus(uint8_t u8Chn);

    uint64_t GetTimeMs(uint8_t u8Chn);

    CVI_ERROR_CODE_E AIGetVideoFrame(uint8_t u8Chn, VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
    CVI_ERROR_CODE_E GetVideoFrame(uint8_t u8Chn, VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
    CVI_ERROR_CODE_E ReleaseVideoFrame(uint8_t u8Chn, VIDEO_FRAME_INFO_S* pstVideoFrameInfo);

    CVI_ERROR_CODE_E SetPlayMode(uint8_t u8Chn, CVIAPP_RTSPPLAY_MODE_E eMode);

public:
    CVI_ERROR_CODE_E UpdateMonitorWindowChnID(uint8_t u8Chn, uint8_t u8MWID);
};

#endif

