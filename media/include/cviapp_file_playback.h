/**
 * File Name: cviapp_file_playback.h
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
   1.Date 			:	2020/12/16
   Author 			:	mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_FILE_PLAYBACK_H_
#define CVIAPP_FILE_PLAYBACK_H_

#include <atomic>

#include "cvi_comm_video.h"
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nbuffer.h"
#include "cvi_nvrlog.h"
#include "cviapp_rtsp_player_manager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#ifdef __cplusplus
}
#endif

#define CVIAPP_AVI_FILENAME_POSTFIX              ".avi"
#define CVIAPP_AVI_FILENAME_POSTFIX_LEN          4
#define CVIAPP_CUT_FILENAME_TIME_SPLIT           "-"
#define CVIAPP_CUT_FILENAME_CUTFILE_SPLIT        "_"

typedef enum
{
    E_CVIAPP_FILEPLAYBACK_IDLE,
    E_CVIAPP_FILEPLAYBACK_STARTING,
    E_CVIAPP_FILEPLAYBACK_START_READY,

    E_CVIAPP_FILEPLAYBACK_PAUSE,

    E_CVIAPP_FILEPLAYBACK_STOPING,
    E_CVIAPP_FILEPLAYBACK_STOP_FINISH,

    E_CVIAPP_FILEPLAYBACK_MAX,
    E_CVIAPP_FILEPLAYBACK_ERROR
} CVIAPP_FILEPLAYBACK_STATUS_E;

/**
  * The Enum of cut status
  * The status ready is means cut have finished and cut file is ready,
  * The status failed is means cut failed, the cut file is invailid.
  *  mantis.0007658, wentao.hu 20220504
  *  */
typedef enum
{
    E_CVIAPP_FILEPLAYBACK_CUT_NOTHING, /* There is no cut task */
    E_CVIAPP_FILEPLAYBACK_CUT_START,
    E_CVIAPP_FILEPLAYBACK_CUT_DOING,
    E_CVIAPP_FILEPLAYBACK_CUT_READ_FINISHED,
    E_CVIAPP_FILEPLAYBACK_CUT_READY, /* Cut file is ready */
    E_CVIAPP_FILEPLAYBACK_CUT_FAILED,
    E_CVIAPP_FILEPLAYBACK_CUT_CUT_MAX
} CVIAPP_FILEPLAYBACK_CUT_STATUS_E;

#define __CUT_INIT           0x00
#define __CUT_FIND_I_FRAME   0x01
#define __CUT_WRITE_I_FRAME  0x02
#define __CUT_RECORDING      0X03
#define __CUT_SPLIT          0x04
#define __CUT_STOP           0x05

#define __CUT_V_FRAME_BUFF_SIZE  (1024 * 1024)
#define __CUT_A_FRAME_BUFF_SIZE  (128 * 1024)

typedef struct
{
    uint8_t m_u8PlaybackChn;

    uint8_t m_u8MonitorWindowChnID;

    string m_strFilePath;

    /* mantis.0007196, wentao.hu 20220309, Seek for file which user ponit to */
    cvi_int64 m_u64SeekTime;
    cvi_int64 m_u64SeekDts;
} CVIAPP_FilePlaybackParam_S;

/**
  * The params of cut av file
  *  mantis.0007658, wentao.hu 20220427
  *  */
typedef struct
{
    string m_strFilePath;
    cvi_int64 m_u64StartTime;
    cvi_int64 m_u64EndTime;
} CVIAPP_FilePlaybackCutFileParam_S;

typedef struct
{
    PAYLOAD_TYPE_E m_enVideoType;

    uint32_t m_u32VideoWidth;
    uint32_t m_u32VideoHeight;

    float m_fps;

} CVIAPP_FilePlayBackMediaInfo_S;

class CVIAPP_FilePlayback_C
{
public:
    CVIAPP_FilePlayback_C(const CVIAPP_FilePlaybackParam_S &param, u_int64_t videoTimems = 0x00);
    CVIAPP_FilePlayback_C(const CVIAPP_FilePlaybackCutFileParam_S &param);
    ~CVIAPP_FilePlayback_C();

    CVI_ERROR_CODE_E Start(string path = "");

    CVI_ERROR_CODE_E Stop();

    CVI_ERROR_CODE_E GetMediaInfo(CVIAPP_FilePlayBackMediaInfo_S &mediaInfo);

    CVI_ERROR_CODE_E Pause();

    CVI_ERROR_CODE_E Resume();

    CVIAPP_FILEPLAYBACK_STATUS_E GetStatus();

    uint64_t GetTimeMs();

    CVI_ERROR_CODE_E UpdateStParam(const CVIAPP_FilePlaybackParam_S& stPara);
    bool UpdateMonitorWindowId(const uint8_t winId);
    bool UpdateChnId(const uint8_t chanId);

    CVI_ERROR_CODE_E GetVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
    CVI_ERROR_CODE_E ReleaseVideoFrame(VIDEO_FRAME_INFO_S* pstVideoFrameInfo);

    CVI_ERROR_CODE_E SetPlaySpeed(CVIAPP_RTSPPLAYER_PLAYSPEED_E ePlaySpeed);
    bool IsDropPktForPlaySpeed(AVPacket pkt);
    CVI_ERROR_CODE_E StartCutAVIFile(string path);
    CVIAPP_FILEPLAYBACK_CUT_STATUS_E getCutStatus();
private:

    CVIAPP_FilePlaybackParam_S m_stParam;
    CVIAPP_FilePlayBackMediaInfo_S m_stMediaInfo;

    CVIAPP_FilePlaybackCutFileParam_S m_stCutFilePram;

    std::atomic<CVIAPP_FILEPLAYBACK_STATUS_E> m_eStatus;

    std::atomic<bool> m_bDemuxThreadEnable;
    CVI_Thread_SC* m_pDemuxThread;

    std::atomic<bool> m_bCutFileThreadEnable;
    CVI_Thread_SC* m_pCutAVReadThread;
    CVI_Thread_SC* m_pCutAVWriteThread;

    float m_fvtb;
    uint64_t m_u64StartVideoPts;
    uint64_t m_u64VideoPts;
    u_int64_t m_u64videoTimems;
    float m_fatb;
    uint64_t m_u64StartAudioPts;
    uint64_t m_u64AudioPts;

    static void *__demuxThread(void *params);
    //static void *cutAVIThread(void *params);
    static void *_cutAVWriteThread(void *params);
    static void *_cutAVReadThread(void *pParam);
    void* _cutStartWriteFile();

    void* m_pPlayer; /* CVIAPP_RtspPlayer_C */

    /*
     * The member to process different play speed
     * mantis.0006999 wentao.hu 20220221
     */
    /* The playspeed just like x2, 1/2 */
    CVIAPP_RTSPPLAYER_PLAYSPEED_E m_ePlaySpeed;
    uint64_t m_u64PktCount; /* the count of pkt to get GOP */
    bool m_bPlayAudioEn; /* The audio thread enable or disable */
    uint32_t m_u32GopPlayDtsNum; /* How many dts we will play for a GOP */
    uint64_t m_u64CurrIpktDts; /* The start dts of curr I frame */
    bool m_bIsStartCut; /* If the cut is started, if the cut have finished, we set it as false */

    CVI_RingBuffer_SC* m_pCutVideoRingBuffer;
    CVI_RingBuffer_SC* m_pCutAudioRingBuffer;
    CVIAPP_FILEPLAYBACK_CUT_STATUS_E m_eCutStatus;
    AVStream *m_oInVStream;
    AVStream *m_oInAStream;
};

#endif
