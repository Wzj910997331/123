/**
 * File Name: cvibsv_device.h
 *
 * Version: V1.0
 *
 * Brief: device.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 * ====================================================================================*/
#ifndef _CVIBSV_DEVICE_H_
#define _CVIBSV_DEVICE_H_
#include <iostream>
#include <vector>
using namespace std;

#include "cvi_common_type.h"
#include "cvi_base_type.h"
#include "cvi_platform.h"
#include "cvi_const.h"

#ifdef __cplusplus
	extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
///   AUDIO&VIDEO ENCODE TYPE
///////////////////////////////////////////////////////////////////////////
typedef enum
{
	E_CVI_VIDEO_ENCODE_FIRST = 0,
	E_CVI_VIDEO_ENCODE_UNKNOWN = E_CVI_VIDEO_ENCODE_FIRST,
	E_CVI_VIDEO_ENCODE_JPEG,
	E_CVI_VIDEO_ENCODE_MPEG4,
	E_CVI_VIDEO_ENCODE_H264,
	E_CVI_VIDEO_ENCODE_H265,
	E_CVI_VIDEO_ENCODE_NUM
}CVI_VIDEO_ENCODE_TYPE_E;

inline CVI_VIDEO_ENCODE_TYPE_E videoEncodeTypeTransform(string str)
{
    if(0 == str.compare("H265"))
    {
        return E_CVI_VIDEO_ENCODE_H265;
    }
    else if(0 == str.compare("H264"))
    {
        return E_CVI_VIDEO_ENCODE_H264;
    }
    else if(0 == str.compare("MPEG4"))
    {
        return E_CVI_VIDEO_ENCODE_MPEG4;
    }
    else if(0 == str.compare("JPEG"))
    {
        return E_CVI_VIDEO_ENCODE_JPEG;
    }
    else
    {
        LOG(WARNING) << "unknown video encode type: " << str;
        return E_CVI_VIDEO_ENCODE_UNKNOWN;
    }
}

inline string videoEncodeTypeTransform2String(CVI_VIDEO_ENCODE_TYPE_E eType)
{
    switch(eType)
    {
    case E_CVI_VIDEO_ENCODE_JPEG:
        return string("JPEG");
    case E_CVI_VIDEO_ENCODE_MPEG4:
        return string("MPEG4");
    case E_CVI_VIDEO_ENCODE_H264:
        return string("H264");
    case E_CVI_VIDEO_ENCODE_H265:
        return string("H265");
    default:
        return string("UNKNOWN");
    }
}

typedef enum
{
	E_CVI_AUDIO_ENCODE_FIRST = 0,
	E_CVI_AUDIO_ENCODE_UNKNOWN = E_CVI_AUDIO_ENCODE_FIRST,
	E_CVI_AUDIO_ENCODE_G711,
	E_CVI_AUDIO_ENCODE_G726,
	E_CVI_AUDIO_ENCODE_AAC,
	E_CVI_AUDIO_ENCODE_NUM
}CVI_AUDIO_ENCODE_TYPE_E;

inline CVI_AUDIO_ENCODE_TYPE_E audioEncodeTypeTransform(string str)
{
    if(0 == str.compare("MP4A-LATM"))
    {
        return E_CVI_AUDIO_ENCODE_AAC;
    }
    else if(0 == str.compare("PCMU"))
    {
        return E_CVI_AUDIO_ENCODE_G711;
    }
    else if(0 == str.compare("G726"))
    {
        return E_CVI_AUDIO_ENCODE_G726;
    }
    else
    {
        LOG(WARNING) << "unknown audio encode type: " << str;
        return E_CVI_AUDIO_ENCODE_UNKNOWN;
    }
}

inline string audioEncodeTypeTransform2String(CVI_AUDIO_ENCODE_TYPE_E eType)
{
    switch(eType)
    {
    case E_CVI_AUDIO_ENCODE_G711:
        return string("PCMU");
    case E_CVI_AUDIO_ENCODE_G726:
        return string("G726");
    case E_CVI_AUDIO_ENCODE_AAC:
        return string("MP4A-LATM");
    default:
        return string("UNKNOWN");
    }
}

typedef enum
{
	E_CVI_AUDIO_SAMPLERATE_FIRST = 0,
	E_CVI_AUDIO_SAMPLERATE_8K = E_CVI_AUDIO_SAMPLERATE_FIRST,
	E_CVI_AUDIO_SAMPLERATE_16K,
	E_CVI_AUDIO_SAMPLERATE_32K,
	E_CVI_AUDIO_SAMPLERATE_44K,
	E_CVI_AUDIO_SAMPLERATE_48K,
	E_CVI_AUDIO_SAMPLERATE_UNKNOWN,
	E_CVI_AUDIO_SAMPLERATE_NUM
}CVI_AUDIO_SAMPLERATE_TYPE_E;

///////////////////////////////////////////////////////////////////////////
///   DEVICE
///////////////////////////////////////////////////////////////////////////
#define CVI_AV_ENCODER_NAME_LEN_MAX 64
#define CVI_RESOLUTIONS_NUM_MAX 20

#define CVI_DEV_MANUFACTOR_LEN_MAX 64
#define CVI_DEV_SERIALNO_LEN_MAX 64
#define CVI_DEV_FIRMWAREVER_LEN_MAX 64
#define CVI_DEV_MODEL_LEN_MAX 64
#define CVI_DEV_HARDWAREID_LEN_MAX 64
#define CVI_AUXILIARYCMD_NUM_MAX 10
#define CVI_AUXILIARYCMD_LEN_MAX 64

typedef enum
{
	E_CVI_DEVCAP_MODE_FIRST = 0,
	E_CVI_DEVCAP_MODE_NOTSURE = E_CVI_DEVCAP_MODE_FIRST,
	E_CVI_DEVCAP_MODE_DISABLE,
	E_CVI_DEVCAP_MODE_ENABLE,
	E_CVI_DEVCAP_MODE_NUM
}CVI_DEVCAP_MODE_E;

typedef enum
{
	E_CVI_DEV_SIGTRANSFER_MODE_FIRST = 0,
	E_CVI_DEV_SIGTRANSFER_MODE_AUTO = E_CVI_DEV_SIGTRANSFER_MODE_FIRST,
	E_CVI_DEV_SIGTRANSFER_MODE_TCP,
	E_CVI_DEV_SIGTRANSFER_MODE_UDP,
	E_CVI_DEV_SIGTRANSFER_MODE_NUM
}CVI_DEV_SIGTRANSFER_MODE_E;

typedef enum
{
	E_CVI_DEV_PROT_TYPE_FIRST = 0,
	E_CVI_DEV_PROT_TYPE_NULL = E_CVI_DEV_PROT_TYPE_FIRST,
	E_CVI_DEV_PROT_TYPE_ONVIF,
	E_CVI_DEV_PROT_TYPE_CVITEK,
	E_CVI_DEV_PROT_TYPE_SIMU,
	E_CVI_DEV_PROT_TYPE_RTSP_RAYSHINE,
	E_CVI_DEV_PROT_TYPE_NUM
}CVI_DEV_PROT_TYPE_E;

typedef enum
{
	E_CVI_H265_PROFILE_FIRST = 0,
	E_CVI_H265_PROFILE_MAIN10 = E_CVI_H265_PROFILE_FIRST,
	E_CVI_H265_PROFILE_EXTENDED,
	E_CVI_H265_PROFILE_HIGH,
	E_CVI_H265_PROFILE_NUM,
}CVI_H265_PROFILE_E;

typedef enum
{
	E_CVI_H264_PROFILE_FIRST = 0,
	E_CVI_H264_PROFILE_BASELINE = E_CVI_H264_PROFILE_FIRST,
	E_CVI_H264_PROFILE_MAIN,
	E_CVI_H264_PROFILE_EXTENDED,
	E_CVI_H264_PROFILE_HIGH,
	E_CVI_H264_PROFILE_NUM
}CVI_H264_PROFILE_E;

typedef enum
{
	E_CVI_MPEG4_PROFILE_FIRST = 0,
	E_CVI_MPEG4_PROFILE_SP = E_CVI_MPEG4_PROFILE_FIRST,
	E_CVI_MPEG4_PROFILE_ASP,
	E_CVI_MPEG4_PROFILE_NUM
}CVI_MPEG4_PROFILE_E;

typedef enum
{
	E_CVI_DEV_PROFILE_FIRST = 0,
	E_CVI_DEV_PROFILE_MAIN = E_CVI_DEV_PROFILE_FIRST,
	E_CVI_DEV_PROFILE_SUB1,
	E_CVI_DEV_PROFILE_SUB2,
	E_CVI_DEV_PROFILE_NUM
}CVI_DEV_PROFILE_E;

// BacklightCompensation mode
typedef enum
{
	E_CVI_BACKLIGHT_COMPENSATION_MODE_FIRST = 0,
	E_CVI_BACKLIGHT_COMPENSATION_MODE_OFF = E_CVI_BACKLIGHT_COMPENSATION_MODE_FIRST,
	E_CVI_BACKLIGHT_COMPENSATION_MODE_ON,
	E_CVI_BACKLIGHT_COMPENSATION_MODE_NUM
}CVI_BACKLIGHT_COMPENSATION_MODE_E;

// Exposure mode
typedef enum
{
	E_CVI_EXPOSURE_MODE_FIRST = 0,
	E_CVI_EXPOSURE_MODE_AUTO = E_CVI_EXPOSURE_MODE_FIRST,
	E_CVI_EXPOSURE_MODE_MANUAL,
	E_CVI_EXPOSURE_MODE_NUM
} CVI_EXPOSURE_MODE_E;

// Exposure Priority
typedef enum
{
	E_CVI_EXPOSURE_PRIORITY_FIRST = 0,
	E_CVI_EXPOSURE_PRIORITY_LOWNOISE = E_CVI_EXPOSURE_PRIORITY_FIRST,
	E_CVI_EXPOSURE_PRIORITY_FRAMERATE,
	E_CVI_EXPOSURE_PRIORITY_NUM
} CVI_EXPOSURE_PRIORITY_E;

// AutoFocus Mode
typedef enum
{
	E_CVI_AUTOFOCUS_MODE_FIRST = 0,
	E_CVI_AUTOFOCUS_MODE_AUTO = E_CVI_AUTOFOCUS_MODE_FIRST,
	E_CVI_AUTOFOCUS_MODE_MANUAL,
	E_CVI_AUTOFOCUS_MODE_NUM
} CVI_AUTOFOCUS_MODE_E;

typedef enum
{
	E_CVI_WIDEDYNAMIC_RANGE_FIRST = 0,
	E_CVI_WIDEDYNAMIC_RANGE_OFF = E_CVI_WIDEDYNAMIC_RANGE_FIRST,
	E_CVI_WIDEDYNAMIC_RANGE_ON,
	E_CVI_WIDEDYNAMIC_RANGE_NUM
} CVI_WIDEDYNAMIC_RANGE_E;

typedef enum
{
	E_CVI_IRCUTFILTER_MODE_FIRST = 0,
	E_CVI_IRCUTFILTER_MODE_ON = E_CVI_IRCUTFILTER_MODE_FIRST,
	E_CVI_IRCUTFILTER_MODE_OFF,
	E_CVI_IRCUTFILTER_MODE_AUTO,
	E_CVI_IRCUTFILTER_MODE_NUM
} CVI_IRCUTFILTER_MODE_E;

typedef enum WhiteBalanceMode
{
	E_CVI_WHITEBALANCE_MODE_FIRST = 0,
	E_CVI_WHITEBALANCE_MODE_AUTO = E_CVI_WHITEBALANCE_MODE_FIRST,
	E_CVI_WHITEBALANCE_MODE_MANUAL,
	E_CVI_WHITEBALANCE_MODE_NUM
} CVI_WHITEBALANCE_MODE_E;

typedef enum
{
	E_CVI_ROTATE_MODE_FIRST = 0,
    E_CVI_ROTATE_MODE_OFF = E_CVI_ROTATE_MODE_FIRST,
    E_CVI_ROTATE_MODE_ON,
    E_CVI_ROTATE_MODE_AUTO
} CVI_ROTATE_MODE_E;

#include "cvibsv_device_profile.h"


#ifdef __cplusplus
	}
#endif
#endif//_CVIBSV_DEVICE_H_
