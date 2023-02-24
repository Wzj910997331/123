/**
 * File Name: cvi_rtspcli_ffmpeg_test.h
 *
 * Version: V1.0
 *
 * Brief: ffmpeg of rtsp client implements.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description: more detail descriptions.
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2022/6/30
   Author 			:	kevin.xu
   Modification		:	Created file
 * ====================================================================================*/
#ifndef _CVI_RTSPCLI_FFMPEG_T_H_
#define _CVI_RTSPCLI_FFMPEG_T_H_
#include "cvi_common_type.h"
#include "cvi_platform.h"
#include "cvi_const.h"
#include "cvibsv_rtspcli_if.h"
#include "cvi_nbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

class CVI_FFMPEGRtspClient_T_SC : public CVIBSV_RtspClient_SC
{
public:
	CVI_FFMPEGRtspClient_T_SC();
	virtual ~CVI_FFMPEGRtspClient_T_SC();

	// command
	virtual CVI_ERROR_CODE_E Start(CVIBSV_RtspConnParams_C &oParam);
	virtual CVI_ERROR_CODE_E Stop(); // destroy

	// apis
	virtual CVIBSV_RTSP_CLIENT_STATUS_E GetRtspStatus();
	virtual CVI_ERROR_CODE_E GetStreamStsInfo(CVIBSV_StreamStsInfo_C &oInfo);
	virtual CVI_ERROR_CODE_E GetTrackSetInfo(vector<CVIBSV_RtspCliTrackInfo_C> &olTrackInfoList);

    //virtual CVI_ERROR_CODE_E SetPreviewMode(CVI_PREVIEW_MODE_E eMode);
    //virtual CVI_ERROR_CODE_E GetVideoFrame(cvi_uint8 *pBuff, cvi_uint32 buffSize, CVI_RTSP_DATA_CUSTOMER_E eCustomer, cvi_uint32 &frameSize);
    //virtual CVI_ERROR_CODE_E GetAudioFrame(cvi_uint8 *pBuff, cvi_uint32 buffSize, CVI_RTSP_DATA_CUSTOMER_E eCustomer, cvi_uint32 &frameSize);

	virtual CVI_ERROR_CODE_E GetVideoBuffer(CVI_RingBuffer_SC* &oBuffer);
	virtual CVI_ERROR_CODE_E GetAudioBuffer(CVI_RingBuffer_SC* &oBuffer);
	virtual CVI_ERROR_CODE_E GetMetaBuffer(CVI_RingBuffer_SC* &oBuffer);
	virtual CVI_ERROR_CODE_E GetPicMetaBuffer(CVI_RingBuffer_SC* &oBuffer);

	virtual CVI_ERROR_CODE_E AudioChannelWriteData(cvi_uint8 u8TrackIdx,cvi_uint32 u32Size,cvi_uint8 *pu8Data,cvi_uint32 &u32WriteBytes);

private:

    static void *__workThreadMain(void *pParams);

    CVI_Thread_SC *m_pWorkThread;

private:
    void setStatus(CVIBSV_RTSP_CLIENT_STATUS_E eStatus);
};

#ifdef __cplusplus
}
#endif

#endif//_CVIBSV_RTSPCLI_FFMPEG_H_

