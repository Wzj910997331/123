/**
 * File Name: cviapp_jpeg_player.h
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

#ifndef CVIAPP_JPEG_PLAYER_H_
#define CVIAPP_JPEG_PLAYER_H_

#include "cvi_comm_video.h"
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const_error.h"

#include "cviapp_monitor_window.h"

typedef struct
{
    VDEC_CHN m_vdecChn;

    uint8_t m_u8MonitorWindowChnID;
} CVIAPP_JpegPlayerParam_S;

class CVIAPP_JpegPlayer_C
{
public:
    CVIAPP_JpegPlayer_C(const CVIAPP_JpegPlayerParam_S &param);
    ~CVIAPP_JpegPlayer_C();

    CVI_ERROR_CODE_E DecodeJpeg(string path);

private:
    CVI_ERROR_CODE_E JpegDecInit();
    CVI_ERROR_CODE_E JpegDecDeInit();

private:
    CVIAPP_JpegPlayerParam_S m_stParam;
};

#endif

