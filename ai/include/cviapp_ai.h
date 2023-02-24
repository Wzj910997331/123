#ifndef CVIAPP_AI_APP_H_
#define CVIAPP_AI_APP_H_

#include "cvi_const_error.h"
#include "cvi_common_type.h"
#include "cviai.h"

#include "cvi_platform.h"
#include "cvi_const_error.h"
#include "cvi_nvrlog.h"

#define FACE_RETINA_PATH "/mnt/data/ai_lib/ai_models_output/retinaface_mnet0.25_608.cvimodel"
#define OBJECT_DETECT_MODEL_PATH "/mnt/data/ai_lib/ai_models_output/mobiledetv2-lite.cvimodel"
#define FACE_RECOGNITION_PATH "/mnt/data/ai_lib/ai_models_output/cviface-v5-s.cvimodel"
#define FACE_QUALITY_PATH "/mnt/data/ai_lib/ai_models_output/fqnet-v5_shufflenetv2-softmax.cvimodel"
#define FACE_ATTRIBUTE_PATH "/mnt/data/ai_lib/ai_models_output/cviface-v3-attribute.cvimodel"
#define PD_MODEL_PATH "/mnt/data/ai_lib/ai_models_output/mobiledetv2-pedestrian-d0.cvimodel"


#define DB_IMAGE_DIR "/mnt/data/ai_lib/facelib/db/"
#define DB_FEATURE_DIR "/mnt/data/ai_lib/face_featurel/db_feature/"

#define __VPSS_GRP_AI    6
#define __VPSS_GRP_CROP    4
#define __VPSS_GRP_CROP_1    5
#define __VPSS_CHN  0
#define __VPSS_DEV_AI  1
#define __VPSS_DEV_CROP  1
#define __VPSS_DEV_AI_HANDLE  1

#define __MAX_AI_PROCESS  8
#define OBJECT_DETECT_CONFIDENCE (0.7)
#define FEATURE_LENGTH 512
#define NAME_LENGTH 1024

#define JPEG_FILE_NAME_SIZE 128
#define CVI_NVR_AI_MAX_REGION_NUM 4
#define CVI_NVR_AI_MAX_PTS_NUM 10

typedef enum
{
    CVIAPP_AI_MODEL_RETINAFACE,
    CVIAPP_AI_MODEL_OBJECT_DETECTION,
    CVIAPP_AI_RUNONETHREAD,
    CVIAPP_AI_FACETRACKING,
    CVIAPP_AI_CVINVRTHREAD,
    CVIAPP_AI_CVINVRREGION,
    CVIAPP_AI_MODEL_MAX
} CVIAPP_AI_MODEL_E;

typedef struct {
  float x[CVI_NVR_AI_MAX_PTS_NUM];
  float y[CVI_NVR_AI_MAX_PTS_NUM];
  uint32_t size;
} cvi_nvr_ai_pts_t;

typedef struct {
  char cropJpegFileName[JPEG_FILE_NAME_SIZE];
  cvai_feature_t feature;
  int32_t channel;
} crop_face_info_t;

typedef struct
{
    u_int8_t isMatchFaceFeature;
    char matchnName[60];
} CVIAPP_MatchFace_S;

typedef struct
{
    CVIAPP_AI_MODEL_E eType;
    void *pResult;
} CVIAPP_AiResult_S;

typedef struct
{
    CVIAPP_MatchFace_S sMatchResult;
    cvai_face_t face;
}CVIAPP_AiResultInfo_S;

typedef struct
{
    int inRegion[512];
    cvai_object_t obj_meta;
}CVIAPP_AiRegionResultInfo_S;

typedef uint8_t (*CVIAPP_AI_GET_FRAME_FUN_T)(uint8_t u8Chn, VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
typedef void (*CVIAPP_AI_RELEASE_FRAME_FUN_T)(uint8_t u8Chn, VIDEO_FRAME_INFO_S* pstVideoFrameInfo);
typedef void (*CVIAPP_AI_NOTIFY_FUN_T)(uint8_t u8Chn, CVIAPP_AiResult_S* pResult);

typedef struct
{
    CVIAPP_AI_MODEL_E eType;
    const char *pModelPath;
    CVIAPP_AI_GET_FRAME_FUN_T pGetFrameFun;
    CVIAPP_AI_RELEASE_FRAME_FUN_T pReleaseFrameFun;
    CVIAPP_AI_NOTIFY_FUN_T pNotifyFun;
} CVIAPP_AiParam_S;

CVI_ERROR_CODE_E CVIAPP_AiInit(void);

CVI_ERROR_CODE_E CVIAPP_AiStart(CVIAPP_AiParam_S &param);
CVI_ERROR_CODE_E CVIAPP_AiStop(void);
CVI_ERROR_CODE_E _CVIAPP_AiGetFeature(const char *db_img_dir, const char *db_feature_dir);
CVI_ERROR_CODE_E _CVIAPP_AiSearchMatchFaceCount(const char *img_dir, int *matchNum, int *faceLibIndexs);

CVI_ERROR_CODE_E CVIAPP_SetRegionPts(uint8_t id,cvi_nvr_ai_pts_t& pts);
CVI_ERROR_CODE_E CVIAPP_GetRegionPts(uint8_t id,cvi_nvr_ai_pts_t& pts);

#endif

