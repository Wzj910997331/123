/**
 * File Name: cviapp_audio_output.h
 *
 * Version: V1.0
 *
 * Brief: 
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * Description: 
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2021/02/20
   Author 			:   mason.zou	
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVIAPP_AUDIO_OUTPUT_H_
#define CVIAPP_AUDIO_OUTPUT_H_

#include "cvi_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define CVIAPP_AO_CHN_MAX       8
#define CVIAPP_AO_CARD_NUM_MAX  3

#define CVIAPP_AO_PCM_CARD_0    1

CVI_S32 CVIAPP_AO_Init(uint8_t u8Card);
CVI_S32 CVIAPP_AO_GetPcmFrameSize(uint8_t u8Card, uint32_t& u32Size);
CVI_S32 CVIAPP_AO_SoftMute(uint8_t u8Card, uint8_t u8Chn, bool bMute);
/* one channel only used by one thread */
CVI_S32 CVIAPP_AO_SendFrame(uint8_t u8Card, uint8_t u8Chn, uint8_t* pcmData);

#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif

