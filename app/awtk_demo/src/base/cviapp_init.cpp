#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cvi_sys.h"
#include "cvi_type.h"

#include "cvi_audio.h"
#include "cvi_buffer.h"
#include "cvi_comm_aio.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vo.h"
#include "cvi_comm_vpss.h"
#include "cvi_common.h"

#include "cvi_gdc.h"
#include "cvi_math.h"
#include "cvi_region.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vdec.h"
#include "cvi_venc.h"
#include "cvi_vi.h"
#include "cvi_vo.h"
#include "cvi_vpss.h"

#include "cviapp_init.h"
#include "cviapp_lt9611.h"

#define INPUT_VIDEO_MAX_WIDTH 1920
#define INPUT_VIDEO_MAX_HEIGHT 1080

#define INPUT_VIDEO_WIDTH_1 1280
#define INPUT_VIDEO_HEIGHT_1 720

static void vb_pool_init(void) {
  CVI_S32 s32Ret = CVI_FAILURE;

  VB_CONFIG_S stVbConfig;

  CVI_VB_Exit();

  memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));

#ifdef ENABLE_CVIAI
  stVbConfig.u32MaxPoolCnt = 3;
#else
  stVbConfig.u32MaxPoolCnt = 1;
  // stVbConfig.u32MaxPoolCnt = 2;
#endif

  stVbConfig.astCommPool[0].u32BlkSize =
      COMMON_GetPicBufferSize(INPUT_VIDEO_MAX_WIDTH, INPUT_VIDEO_MAX_HEIGHT,
                              PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8,
                              COMPRESS_MODE_NONE, DEFAULT_ALIGN);
  stVbConfig.astCommPool[0].u32BlkCnt = 18;
  stVbConfig.astCommPool[0].enRemapMode = VB_REMAP_MODE_NONE;

  // stVbConfig.astCommPool[1].u32BlkSize = COMMON_GetPicBufferSize(200, 200,\
    //                                         PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
  // stVbConfig.astCommPool[1].u32BlkCnt = 3;
  // stVbConfig.astCommPool[1].enRemapMode = VB_REMAP_MODE_NONE;

#ifdef ENABLE_CVIAI

  /* Prepare memory block for AI (RetinaFace,FaceQuality,FaceRecognition) */
  stVbConfig.astCommPool[1].u32BlkSize = COMMON_GetPicBufferSize(
      INPUT_VIDEO_MAX_WIDTH, INPUT_VIDEO_MAX_HEIGHT, PIXEL_FORMAT_RGB_888,
      DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
  stVbConfig.astCommPool[1].u32BlkCnt = 5;
  stVbConfig.astCommPool[2].u32BlkSize = COMMON_GetPicBufferSize(
      INPUT_VIDEO_MAX_WIDTH, INPUT_VIDEO_MAX_HEIGHT, PIXEL_FORMAT_RGB_888,
      DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
  stVbConfig.astCommPool[2].u32BlkCnt = 5;
#endif

  s32Ret = CVI_VB_SetConfig(&stVbConfig);
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_VB_SetConf\n");
  }

  s32Ret = CVI_VB_Init();
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_VB_Init failed!\n");
  }
}

static void mmf_sys_init(void) {
  CVI_S32 s32Ret = CVI_FAILURE;

  CVI_SYS_Exit();

  s32Ret = CVI_SYS_Init();
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_SYS_Init failed!\n");
  }
}

void CVIAPP_VoInit(VO_INTF_SYNC_E eVoType) {
  CVI_S32 s32Ret = CVI_SUCCESS;

  VO_DEV VoDev = 0;
  VO_PUB_ATTR_S stVoPubAttr;

  uint32_t w = 0x00;
  uint32_t h = 0x00;

  memset(&stVoPubAttr, 0, sizeof(VO_PUB_ATTR_S));

  stVoPubAttr.u32BgColor = 0xFFFFFFFF;
  stVoPubAttr.enIntfType = VO_INTF_MIPI;
  stVoPubAttr.enIntfSync = eVoType;

  switch (eVoType) {
  case VO_OUTPUT_720P60:
    w = 1280;
    h = 720;
    break;
  case VO_OUTPUT_1080P60:
    w = 1920;
    h = 1080;
    break;
  default:
    printf("not support!\n");
    break;
  }

  s32Ret = CVI_VO_SetPubAttr(VoDev, &stVoPubAttr);

  s32Ret = CVI_VO_Enable(VoDev);

  VO_LAYER VoLayer = 0;
  VO_VIDEO_LAYER_ATTR_S stLayerAttr;

  memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));

  /* Same as stVoPubAttr.enIntfSync setting */
  stLayerAttr.stDispRect.u32Width = w;
  stLayerAttr.stDispRect.u32Height = h;
  stLayerAttr.u32DispFrmRt = 60;

  stLayerAttr.stDispRect.s32X = 0;
  stLayerAttr.stDispRect.s32Y = 0;

  stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_PLANAR_420;

  stLayerAttr.stImageSize.u32Width = w;
  stLayerAttr.stImageSize.u32Height = h;

  s32Ret = CVI_VO_SetDisplayBufLen(VoLayer, 3);
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_VO_SetDisplayBufLen failed!\n");
  }

  s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_VO_SetVideoLayerAttr failed!\n");
  }

  s32Ret = CVI_VO_EnableVideoLayer(VoLayer);

  VO_CHN VoChn = 0;
  VO_CHN_ATTR_S stChnAttr;

  memset(&stChnAttr, 0, sizeof(VO_CHN_ATTR_S));

  stChnAttr.stRect.s32X = 0;
  stChnAttr.stRect.s32Y = 0;
  stChnAttr.stRect.u32Width = w;
  stChnAttr.stRect.u32Height = h;
  stChnAttr.u32Priority = 0;

  s32Ret = CVI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);

  s32Ret = CVI_VO_EnableChn(VoLayer, VoChn);

  // s32Ret = CVI_VO_SetChnRotation(0, VoChn, ROTATION_180);
}

void CVIAPP_VoDeinit(void) {
  CVI_S32 s32Ret = CVI_SUCCESS;

  VO_DEV VoDev = 0;
  VO_LAYER VoLayer = 0;
  VO_CHN VoChn = 0;

  s32Ret = CVI_VO_DisableChn(VoLayer, VoChn);

  s32Ret = CVI_VO_DisableVideoLayer(VoLayer);

  s32Ret = CVI_VO_Disable(VoDev);
}

void CVIAPP_GetVoSize(uint32_t *w, uint32_t *h) {
  if (w == NULL || h == NULL)
    return;

  CVI_S32 s32Ret = CVI_SUCCESS;
  VO_LAYER VoLayer = 0;
  VO_VIDEO_LAYER_ATTR_S stLayerAttr;
  memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));
  s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
  if (s32Ret != CVI_SUCCESS) {
    printf("CVI_VO_GetVideoLayerAttr failed!\n");
  }

  *w = stLayerAttr.stDispRect.u32Width;
  *h = stLayerAttr.stDispRect.u32Height;
}

#if 0
static void CVIAPP_AoInit(void)
{
    CVI_S32 s32Ret;

    //AUDIO_DEV AoDev = 2;   /* 2 external card */
    AUDIO_DEV AoDev = 0;
    AIO_ATTR_S stAioAttr;

    memset(&stAioAttr, 0, sizeof(AIO_ATTR_S));

    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_48000;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode =  AUDIO_SOUND_MODE_STEREO;
    stAioAttr.u32EXFlag = 0;
    stAioAttr.u32FrmNum = 3;
    //stAioAttr.u32PtNumPerFrm = 1024;
    stAioAttr.u32PtNumPerFrm = 960;
    stAioAttr.u32ChnCnt = 2; /* Refer to enSoundmode setting */
    stAioAttr.u32ClkSel = 0;
    stAioAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

    s32Ret = CVI_AO_SetPubAttr(AoDev, &stAioAttr);

    s32Ret = CVI_AO_Enable(AoDev);

    s32Ret = CVI_AO_DisableReSmp(AoDev, 0);

    s32Ret = CVI_AO_EnableChn(AoDev, 0);
}
#endif

CVI_S32 CVIAPP_Init(void) {
  vb_pool_init();
  mmf_sys_init();
  CVIAPP_Lt9611Init(E_CVIAPP_LT9611_VIDEO_1920x1080_60HZ);
  CVIAPP_VoInit(VO_OUTPUT_1080P60);
  // CVIAPP_AoInit();        //cvi_ao
  // CVIAPP_AO_Init(CVIAPP_AO_PCM_CARD_0);

  return CVI_SUCCESS;
}

void CVIAPP_Deinit() {
  CVI_SYS_Exit();

  CVI_VB_Exit();
}
