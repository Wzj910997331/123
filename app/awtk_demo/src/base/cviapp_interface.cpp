#include <algorithm>

#include "ifaddrs.h"
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>

#include "cviapp_interface.h"
#include "cviapp_init.h"

#include "cviapp_lt9611.h"

#include "cvi_rtspcliif_ffmpeg_test.h"

#include "cviapp_monitor_window.h"


#include "cvi_sys.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vpss.h"
#include "cvi_vpss.h"
#include "cvi_math.h"
#include "cviapp_ai.h"
#include "cviapp_ai_handler.h"

CVIAPP_RtspPlayer_C *lastPlayer = NULL;

/**
 *  Global params manager for rtsp player,
 *  mantis.0010798  wentao.hu 20230209
 */
static CVIAPP_RtspPlayerManager_S g_rtspPlayerMng_list[CVIAPP_AIBOX_DEVICE_NUM_MAX];


static void __monitorWindowInit()
{
    uint32_t w,h;
    CVIAPP_GetVoSize(&w, &h);

    CVIAPP_MonitorWindowParam_S param;

    param.m_voDev = 0;

    param.m_voSize.u32Width  = w;
    param.m_voSize.u32Height = h;

    CVIAPP_MonitorWindowChnParam_S chnParam;

    chnParam.m_u8Priority = 0;

    chnParam.m_stRect.u32Width = w;
    chnParam.m_stRect.u32Height = h;

    chnParam.m_stRect.s32X = 0;
    chnParam.m_stRect.s32Y = 0;

    param.m_stlChnParam.push_back(chnParam);

    CVIAPP_MonitorWindow_C::InitMonitorWindowParam(param);
}

CVI_ERROR_CODE_E CVIAPP_SYS_Init()
{

    FLAGS_logtostderr = true;
    google::InitGoogleLogging("app");

    __monitorWindowInit();

    return E_CVI_ERROR_CODE_SUCC;
}

CVI_ERROR_CODE_E CVIAPP_SYS_DeInit()
{
    return E_CVI_ERROR_CODE_SUCC;
}

/**
 * To get rtspplayer by channel id
 *
 * mantis.0010798  wentao.hu 20230209
 *
 * \input id: the id of IPC channel;
 * \return: the CVIAPP_RtspPlayer_C entry.
 */
CVIAPP_RtspPlayer_C* CVIAPP_GetRtspPlayerByCh(uint8_t id)
{
    if (id >= CVIAPP_AIBOX_DEVICE_NUM_MAX)
    {
        return NULL;
    }

    if(NULL == g_rtspPlayerMng_list[id].pPlayer)
    {
        //CVI_NVRLOGD("CVIAPP_GetRtspPlayerByCh NULL == pPlayer for chn %d", id);
        return NULL;
    }

    return g_rtspPlayerMng_list[id].pPlayer;
}

/**
 * To play the rtsp stream by the rtsp url
 *
 * mantis.0010798 wentao.hu 20230209
 *
 * \input chNo, the num of IPC channel;
 * \input rtspUrl, the URL of Rtsp stream in IPC;
 * \input rtspUser, the User name to request the rtsp;
 * \input rtspPasswd, the password to request the rtsp;
 * \input winSplitType, the type of split screen, it can be 1, 2, 4...;
 * \input viewWinID, the id of preview windows in the split screen;
 * \return, the Enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_StartRtspPlayChn(
                     uint8_t chNo, string rtspUrl, string rtspUser,
                     string rtspPasswd,
                     CVIAPP_MonitorWindowType_E winSplitType,
                     uint8_t viewWinID)
{
    CVI_FFMPEGRtspClient_T_SC *pFFMPEGRtspClient0
        = new CVI_FFMPEGRtspClient_T_SC();
    CVIBSV_RtspConnParams_C *pConnParam = NULL;
    CVIAPP_RtspPlayer_C *pPlayer = NULL;

    pConnParam = &g_rtspPlayerMng_list[chNo].stRtspConnParam;

    /**
     * Prepare the request param of rtsp stream
     */
    memset(pConnParam, 0x0, sizeof(CVIBSV_RtspConnParams_C));
    pConnParam->m_eDevProfile = E_CVI_DEV_PROFILE_MAIN;
    pConnParam->m_strUser = rtspUser;
    pConnParam->m_strPasswd = rtspPasswd;
    pConnParam->m_strUriStr = rtspUrl;

    /**
     * Start to request rtsp stream
     */
    pFFMPEGRtspClient0->Start(*pConnParam);

    sleep(2);


    /**
     * Prepare the rtsp play param of rtsp stream
     */
    CVIAPP_RtspPlayerParam_S *pPlayerParam = NULL;
    pPlayerParam = &g_rtspPlayerMng_list[chNo].stRtspPlayerParam;

    pPlayerParam->m_u8Chn = chNo;
    pPlayerParam->m_enType = PT_H264;

    pPlayerParam->m_fps = CVIAPP_AIBOX_FPS_DEFAULT;
    pPlayerParam->m_u8MonitorWindowChnID = viewWinID;


    switch(winSplitType)
    {
    case E_CVIAPP_WM_TYPE_FULL_SCREEN:
        pPlayerParam->m_u32InputVideoWidth = CVIAPP_DEFAULT_GUI_WIDTH;
        pPlayerParam->m_u32InputVideoHeight = CVIAPP_DEFAULT_GUI_HEIGHT;
        break;

    case E_CVIAPP_WM_TYPE_2_SPLIT:
        pPlayerParam->m_u32InputVideoWidth = CVIAPP_DEFAULT_GUI_WIDTH;
        pPlayerParam->m_u32InputVideoHeight
            = CVIAPP_DEFAULT_GUI_HEIGHT >> 1;
        break;


    case E_CVIAPP_WM_TYPE_4_SPLIT:
        pPlayerParam->m_u32InputVideoWidth
            = CVIAPP_DEFAULT_GUI_WIDTH >> 1;
        pPlayerParam->m_u32InputVideoHeight
            = CVIAPP_DEFAULT_GUI_HEIGHT >> 1;
        break;

    default:
        pPlayerParam->m_u32InputVideoWidth = CVIAPP_DEFAULT_GUI_WIDTH;
        pPlayerParam->m_u32InputVideoHeight = CVIAPP_DEFAULT_GUI_HEIGHT;
        break;
    }


    CVI_NVRLOGD("GetVideoBuffer");
    pFFMPEGRtspClient0->GetVideoBuffer(pPlayerParam->m_pVideoRingBuffer);

    vector<CVIBSV_RtspCliTrackInfo_C> olInfoList;
    pFFMPEGRtspClient0->GetTrackSetInfo(olInfoList);


    for(uint8_t i = 0; i < olInfoList.size(); i++)
    {
        if(CVI_RTSP_MEDIA_TYPE_VIDEO == olInfoList[i].m_eMediaType)
        {
            switch(olInfoList[i].m_eEncodeType)
            {
            case E_CVI_RTSP_ENCODER_TYPE_H264:
                pPlayerParam->m_enType = PT_H264;
                break;
            case E_CVI_RTSP_ENCODER_TYPE_H265:
                pPlayerParam->m_enType = PT_H265;
                break;
            default:
                pPlayerParam->m_enType = PT_H264;
                CVI_NVRLOGE("not support encode type!!!");
                break;
            }

            pPlayerParam->m_u32InputVideoWidth = olInfoList[i].m_u16VidWidth;
            pPlayerParam->m_u32InputVideoHeight = olInfoList[i].m_u16VidHeight;
            pPlayerParam->m_fps = olInfoList[i].m_u8VidFrameRate;
            pPlayerParam->m_pVideoStream = (void*) olInfoList[i].priv;
            printf("\n%d  %d %f \n\n",
                 pPlayerParam->m_u32InputVideoWidth,
                 pPlayerParam->m_u32InputVideoHeight, pPlayerParam->m_fps);
        }
    }


    /**
     * Start play the stream
     */
    lastPlayer = NULL;
    pPlayer = lastPlayer;
    pPlayer = new CVIAPP_RtspPlayer_C(*pPlayerParam, lastPlayer);
    pPlayer->SetPlayMode(E_CVIAPP_RTSPPLAY_MODE_REAL_TIME);

    g_rtspPlayerMng_list[chNo].pPlayer = pPlayer; /* Set player to gobal */

    return E_CVI_ERROR_CODE_SUCC;
}

/**
 * Start the Preview
 *
 * mantis.0010798 wentao.hu 20230209
 *
 * \return, the Enum of CVI_ERROR_CODE_E.
 */
CVI_ERROR_CODE_E CVIAPP_StartPreview()
{
    CVIAPP_MonitorWindowType_E winSplitType = E_CVIAPP_WM_TYPE_4_SPLIT;
    uint8_t chNo = 0;
    string rtspUrl = "";
    string rtspUser = "";
    string rtspPasswd = "";
    uint8_t viewWinID = 0;

    CVIAPP_MonitorWindow_C::Disable();

    CVIAPP_MonitorWindow_C::SetMonitorWindowType(winSplitType);

    CVIAPP_MonitorWindow_C::Enable();


    rtspUrl = "rtsp://192.168.1.107:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";
    CVIAPP_StartRtspPlayChn(0, rtspUrl, "admin", "L2A34AC7",
                              E_CVIAPP_WM_TYPE_4_SPLIT, 0);

    rtspUrl = "rtsp://192.168.1.108:554/cam/realmonitor?channel=1s&subtype=0&unicast=true&proto=Onvif";
    CVIAPP_StartRtspPlayChn(1, rtspUrl, "admin", "cvitek123",
                              E_CVIAPP_WM_TYPE_4_SPLIT, 1);

    rtspUrl = "rtsp://192.168.1.106:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif";

    CVIAPP_StartRtspPlayChn(2, rtspUrl, "admin", "L2E4E1D9",
                              E_CVIAPP_WM_TYPE_4_SPLIT, 2);

    rtspUrl = "rtsp://admin:cvitek1234@192.168.1.80:554/cam/realmonitor?channel=1&subtype=1&unicast=true&proto=Onvif";

    CVIAPP_StartRtspPlayChn(3, rtspUrl, "admin", "cvitek1234",
                              E_CVIAPP_WM_TYPE_4_SPLIT, 3);


    sleep(5);

    CVIAPP_AiHandlerInit();

    return E_CVI_ERROR_CODE_SUCC;
}

