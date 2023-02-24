
#ifndef _CVIAPP_INTERFACE_H_
#define _CVIAPP_INTERFACE_H_

#include "cvi_base_type.h"
#include "cvi_common_type.h"
#include "cvi_const_error.h"
#include "cvi_platform.h"
#include "cviapp_monitor_window.h"
#include "cviapp_rtsp_player.h"
#include "cviapp_rtsp_player_manager.h"
#include "cvibsv_rtspcli_if.h"
#include <sstream>
#include <string>
#include <vector>

#include "awtk.h"

/* The default num for aibox to connect IPC */
#define CVIAPP_AIBOX_DEVICE_NUM_MAX 6

/* The default num for the fps of IPC wich the aibox connect to */
#define CVIAPP_AIBOX_FPS_DEFAULT 25

/* The UI screen default width and height */
#define CVIAPP_DEFAULT_GUI_WIDTH 1920
#define CVIAPP_DEFAULT_GUI_HEIGHT 1080

/**
 * The Struct to manager the rtspplayer
 * mantis.0010798  wentao.hu 20230209
 */
typedef struct {
  uint8_t u8ChNo;                             /* The channel id */
  CVIBSV_RtspConnParams_C stRtspConnParam;    /* The params to connect IPC */
  CVIAPP_RtspPlayerParam_S stRtspPlayerParam; /* The params for player */
  CVIAPP_RtspPlayer_C *pPlayer;               /* The rtsp player */
} CVIAPP_RtspPlayerManager_S;

CVI_ERROR_CODE_E CVIAPP_SYS_Init();
CVI_ERROR_CODE_E CVIAPP_SYS_DeInit();

CVI_ERROR_CODE_E CVIAPP_StartPreview();
CVI_ERROR_CODE_E CVIAPP_StartRtspPlayChn(
    uint8_t chNo, string rtspUrl, string rtspUser, string rtspPasswd,
    CVIAPP_MonitorWindowType_E winSplitType, uint8_t viewWinID);

CVIAPP_RtspPlayer_C *CVIAPP_GetRtspPlayerByCh(uint8_t id);

#endif
