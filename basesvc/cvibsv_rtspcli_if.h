/**
 * File Name: cvibsv_rtspcli.h
 *
 * Version: V1.0
 *
 * Brief: rtsp client APIs and data structures.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description: more detail descriptions.
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 * ====================================================================================*/


#ifndef _CVIBSV_RTSPCLI_H_
#define _CVIBSV_RTSPCLI_H_
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const.h"
#include "cvibsv_device.h"
#include "cvibsv_rtspcli_if.h"
#include "cvi_nbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
///
///			RTSP Global
///
////////////////////////////////////////////////////////////////////////////////////////
#define CVI_RTSP_SESSION_LEN_MAX 64
#define CVI_RTSP_URI_LEN_MAX 128
#define CVI_RTSP_USER_LEN_MAX 32
#define CVI_RTSP_PASSWD_LEN_MAX 32

typedef enum
{
	E_CVI_RTSP_RTPTRANS_MODE_FISRT = 0,
	E_CVI_RTSP_RTPTRANS_MODE_RTP_OVER_AUTO = E_CVI_RTSP_RTPTRANS_MODE_FISRT,// rtsp
	E_CVI_RTSP_RTPTRANS_MODE_RTP_OVER_TCP,
	E_CVI_RTSP_RTPTRANS_MODE_RTP_OVER_UDP,
	E_CVI_RTSP_RTPTRANS_MODE_RTP_OVER_RTSP,
	E_CVI_RTSP_RTPTRANS_MODE_NUM
}CVI_RTSP_RTPTRANS_MODE_E;

// TODO: is JPEG/EVENT media type needed?
typedef enum
{
	CVI_RTSP_MEDIA_TYPE_FISRT = 0,
	CVI_RTSP_MEDIA_TYPE_VIDEO = CVI_RTSP_MEDIA_TYPE_FISRT,
	CVI_RTSP_MEDIA_TYPE_AUDIO,
	CVI_RTSP_MEDIA_TYPE_DATA,
	CVI_RTSP_MEDIA_TYPE_APPLICATION,
	CVI_RTSP_MEDIA_TYPE_CONTROL,
	CVI_RTSP_MEDIA_TYPE_NUM
}CVI_RTSP_MEDIA_TYPE_E;

typedef enum
{
	CVI_RTSP_AUTHEN_MODE_FISRT = 0,
	CVI_RTSP_AUTHEN_MODE_BASIC = CVI_RTSP_AUTHEN_MODE_FISRT,
	CVI_RTSP_AUTHEN_MODE_DIGEST,
	CVI_RTSP_AUTHEN_MODE_NUM
}CVI_RTSP_AUTHEN_MODE_E;

typedef enum
{
	E_CVI_RTSP_ENCODER_TYPE_FIRST = 0,
	E_CVI_RTSP_ENCODER_TYPE_H265 = E_CVI_RTSP_ENCODER_TYPE_FIRST,
	E_CVI_RTSP_ENCODER_TYPE_H264,
	E_CVI_RTSP_ENCODER_TYPE_MJPEG,
	E_CVI_RTSP_ENCODER_TYPE_MPEG4,
	E_CVI_RTSP_ENCODER_TYPE_AAC,
	E_CVI_RTSP_ENCODER_TYPE_G711,
	E_CVI_RTSP_ENCODER_TYPE_G726,
	// pictrue/metadata/...
	E_CVI_RTSP_ENCODER_TYPE_UNKNOWN,
    E_CVI_RTSP_ENCODER_TYPE_NUM
}CVI_RTSP_ENCODER_TYPE_E;

// Main Stream: onvif profile1's uri
// Sub Stream: onvif profile2's uri
// Sub2 Stream: onvif profile3's uri

// RTSP client =>
//    MediaSession
//       SubMediaSession1:(Track1/Video)
//       SubMediaSession2:(Track2/Audio)
//       SubMediaSession3:(Track3/MetaData)

typedef enum
{
	E_CVIBSV_RTSP_CLIENT_STATUS_FIRST = 0,
	E_CVIBSV_RTSP_CLIENT_STATUS_IDLE = E_CVIBSV_RTSP_CLIENT_STATUS_FIRST,
	E_CVIBSV_RTSP_CLIENT_STATUS_CONNECTING, //
	E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT, //
	E_CVIBSV_RTSP_CLIENT_STATUS_ERROR, //
	E_CVIBSV_RTSP_CLIENT_STATUS_DESTORY, //
	E_CVIBSV_RTSP_CLIENT_STATUS_NUM
}CVIBSV_RTSP_CLIENT_STATUS_E;

typedef void *(*CVIBSV_RTSPCLI_CALLBACK_FN_T)(void *pExData,cvi_uint32 u32Size);

class CVIBSV_StreamStsInfo_C
{
public:
	// bitrate
	cvi_uint32 m_u32Bitrate; // in kB
	// frame rate
	cvi_uint8 m_u8FrameRate;
	// encode type
	CVI_VIDEO_ENCODE_TYPE_E m_eEncodeType;
	// resolution
	CVI_VideoResolution_C m_oRes;
};

class CVIBSV_RtspConnParams_C
{
public:
	string m_strUser;
	string m_strPasswd;
	string m_strUriStr;// server uri
	cvi_uint16 m_u16RemotePort; // remote rtsp server's port(tcp)
	//cvi_bool m_bIsRtspTunnelHttp; // (X)use rtsp tunnel over http
	//cvi_uint16 m_u16RemoteHttpPort;// (X)remote rtsp server's port(over http)
	cvi_uint16 m_u16MaxRetryTimes; // max retry time to reconnect rtp/rtcp
	cvi_uint16 m_u16Timeout; // timeout of rtsp/rtp/rtcp,unit in ms

	//CVI_RTSP_RTPTRANS_MODE_E m_eRtpMode; // (X)rtp transfer mode

	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnError; // on error callback
	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnNew; // on change of medias
	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnBye; // on remote server's a/v channel say good bye

	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnVideoData; //
	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnAudioData; //
	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnMetaData;
	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnOnPicData;

	//CVIBSV_RTSPCLI_CALLBACK_FN_T m_fnStatusRep;

	//cvi_bool m_bIsEnableVideoBuffer;
	//cvi_bool m_bIsEnableAudioBuffer;
	//cvi_bool m_bIsEnableMetaBuffer;
	//cvi_bool m_bIsEnablePicBuffer;

    cvi_uint16 m_u16ChnNo;
    CVI_DEV_PROFILE_E m_eDevProfile;

	CVIBSV_RtspConnParams_C()
	{
		m_u16RemotePort = 0;
		//cvi_bool m_bIsRtspTunnelHttp; // use rtsp tunnel over http
		//cvi_uint16 m_u16RemoteHttpPort;// remote rtsp server's port(over http)
		m_u16MaxRetryTimes = 3;
		m_u16Timeout = 3000;

		//CVI_RTSP_RTPTRANS_MODE_E m_eRtpMode; // rtp transfer mode

		//m_fnOnError = NULL;
		//m_fnOnNew = NULL;
		//m_fnOnBye = NULL;
		//m_fnOnVideoData = NULL;
		//m_fnOnAudioData = NULL;
		//m_fnOnMetaData = NULL;
		//m_fnOnPicData = NULL;
		//m_fnStatusRep = NULL;

		//m_bIsEnableVideoBuffer = CVI_FALSE;
		//m_bIsEnableAudioBuffer = CVI_FALSE;
		//m_bIsEnableMetaBuffer = CVI_FALSE;
		//m_bIsEnablePicBuffer = CVI_FALSE;

        m_u16ChnNo = 1;
        m_eDevProfile = E_CVI_DEV_PROFILE_MAIN;
	}
};

class CVIBSV_RtspCliTrackInfo_C
{
public:
	cvi_bool m_bIsValid; // is it not null?
	cvi_bool m_bIsSendAvail; // is it back channel
	cvi_bool m_bIsRecvAvail; // is it normal channel
	CVI_RTSP_MEDIA_TYPE_E m_eMediaType; // video/audio/metadata/picture
	cvi_uint16 m_u16RTPPort,m_u16RTCPPort;
	CVI_RTSP_RTPTRANS_MODE_E  m_eRtpMode;
	cvi_int32 m_i32Scale; // scale*10

	cvi_uint16 m_u16VidWidth;
	cvi_uint16 m_u16VidHeight;
	cvi_uint8 m_u8VidFrameRate;

	CVI_AUDIO_SAMPLERATE_TYPE_E m_eAudSampleRate;

	cvi_uint8 m_u8AudChannelNum;
	cvi_uint8 m_u8AudDeepth;
	CVI_RTSP_ENCODER_TYPE_E m_eEncodeType;
//	CVI_RingBuffer_SC *m_poBuffer;   // TODO

    void* priv;  /* AVStream* for ffmpeg record */

	CVIBSV_RtspCliTrackInfo_C()
	{
		m_bIsValid = CVI_FALSE;
		m_bIsSendAvail = CVI_FALSE;
		m_bIsRecvAvail = CVI_FALSE;
		m_eMediaType = CVI_RTSP_MEDIA_TYPE_VIDEO;
		m_u16RTPPort = m_u16RTCPPort = 0;
		m_eRtpMode = E_CVI_RTSP_RTPTRANS_MODE_RTP_OVER_AUTO;
		m_i32Scale = 10;

		m_u16VidWidth = 0;
		m_u16VidHeight = 0;
		m_u8VidFrameRate = 0;

		m_eAudSampleRate = E_CVI_AUDIO_SAMPLERATE_8K;

		m_u8AudChannelNum = 0;
		m_u8AudDeepth = 0;
		m_eEncodeType = E_CVI_RTSP_ENCODER_TYPE_H265;
	}
};


class CVIBSV_RtspClient_SC
{
public:

//    typedef enum
//    {
//        E_CUSTOMER_PREVIEW = 1,
//        E_CUSTOMER_RECORD,
//        E_CUSTOMER_RTSP_SERVER,
//        E_CUSTOMER_END
//
//    }CVI_RTSP_DATA_CUSTOMER_E;
//
//    typedef enum
//    {
//        E_PREVIEW_MODE_REAL_TIME = 1,
//        E_PREVIEW_MODE_AVERAGE,
//        E_PREVIEW_MODE_SMOOTH,
//        E_PREVIEW_MODE_END
//    }CVI_PREVIEW_MODE_E;

public:
	CVIBSV_RtspClient_SC(){}
	virtual ~CVIBSV_RtspClient_SC(){}

	// command
	virtual CVI_ERROR_CODE_E Start(CVIBSV_RtspConnParams_C &oParam) = 0;
	virtual CVI_ERROR_CODE_E Stop() = 0; // destroy

	// apis
	virtual CVIBSV_RTSP_CLIENT_STATUS_E GetRtspStatus() = 0;
	virtual CVI_ERROR_CODE_E GetStreamStsInfo(CVIBSV_StreamStsInfo_C &oInfo) = 0;
	virtual CVI_ERROR_CODE_E GetTrackSetInfo(vector<CVIBSV_RtspCliTrackInfo_C> &olTrackInfoList) = 0;

    //virtual CVI_ERROR_CODE_E SetPreviewMode(CVI_PREVIEW_MODE_E eMode) = 0;
    //virtual CVI_ERROR_CODE_E GetVideoFrame(cvi_uint8 *pBuff, cvi_uint32 buffSize, CVI_RTSP_DATA_CUSTOMER_E eCustomer, cvi_uint32 &frameSize) = 0;
    //virtual CVI_ERROR_CODE_E GetAudioFrame(cvi_uint8 *pBuff, cvi_uint32 buffSize, CVI_RTSP_DATA_CUSTOMER_E eCustomer, cvi_uint32 &frameSize) = 0;

	virtual CVI_ERROR_CODE_E GetVideoBuffer(CVI_RingBuffer_SC* &oBuffer) = 0;
	virtual CVI_ERROR_CODE_E GetAudioBuffer(CVI_RingBuffer_SC* &oBuffer) = 0;
	virtual CVI_ERROR_CODE_E GetMetaBuffer(CVI_RingBuffer_SC* &oBuffer) = 0;
	virtual CVI_ERROR_CODE_E GetPicMetaBuffer(CVI_RingBuffer_SC* &oBuffer) = 0;

	virtual CVI_ERROR_CODE_E AudioChannelWriteData(cvi_uint8 u8TrackIdx,cvi_uint32 u32Size,cvi_uint8 *pu8Data,cvi_uint32 &u32WriteBytes) = 0;

protected:

    inline CVI_ERROR_CODE_E MakeUriWithAuth(string srcUri, string username, string password, string &destUri)
    {
        size_t pos = srcUri.find("rtsp://");

        if(string::npos != pos)
        {
            string temp = srcUri.substr(7, srcUri.size());
            destUri = "rtsp://" + username + ":" + password + "@" + temp;
        }
        else
        {
            return E_CVI_ERROR_CODE_FAULT;
        }

        return E_CVI_ERROR_CODE_SUCC;
    }

    CVIBSV_RTSP_CLIENT_STATUS_E m_eStatus;

    CVI_RingBuffer_SC *m_prbVideoBuff;
    CVI_RingBuffer_SC *m_prbAudioBuff;

    CVIBSV_RtspConnParams_C m_oRtspConnParams;
    CVIBSV_StreamStsInfo_C m_oStreamStsInfo;
    vector<CVIBSV_RtspCliTrackInfo_C> m_olTracInfoList;

    string m_strUriWithAuthStr;
};

#ifdef __cplusplus
}
#endif
#endif//_CVIBSV_RTSPCLI_H_
