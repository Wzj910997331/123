#include <sys/time.h>
#include <time.h>

#include "strings.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "cvi_comm_sys.h"
#include "cvi_comm_vpss.h"
#include "cvi_math.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "cviapp_ai.h"

#include "cvi_audio.h"
#include "cvi_buffer.h"
#include "cvi_comm_aio.h"
#include "cvi_tracer.h"
#include "cvi_vdec.h"
#include "cvi_venc.h"

#include "app/cviai_app.h"
#include "core/utils/vpss_helper.h"
#include "cviai.h"
#include "core/cviai_core.h"
#include "ive/ive.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
// #include "sys_utils.hpp"

#ifdef APP_YUANWEI
#include "cvi_app_ai_handler.h" //yuanwei
#endif
#ifdef APP_CVINVR
#include "cviapp_ai_handler.h" //cvi_nvr
#endif

//#define ENABLE_PRINT_FPS  1
#define ENABLE_CROP 1

static CVI_Mutex_SC __aiImageMutex;

/*face capture*/
#define OUTPUT_BUFFER_SIZE 10
#define MODE_DEFINITION 0
#define FACE_FEAT_SIZE 256
typedef struct {
  uint64_t u_id;
  float quality;
  cvai_image_t image;
  tracker_state_e state;
  uint32_t counter;
  char name[128];
  float match_score;
  uint64_t frame_id;
} IOData;

static volatile bool bRunImageWriter = true;

#define SMT_MUTEXAUTOLOCK_INIT(mutex) pthread_mutex_t AUTOLOCK_##mutex = PTHREAD_MUTEX_INITIALIZER;

#define SMT_MutexAutoLock(mutex, lock)                                            \
  __attribute__((cleanup(AutoUnLock))) pthread_mutex_t *lock = &AUTOLOCK_##mutex; \
  pthread_mutex_lock(lock);

__attribute__((always_inline)) inline void AutoUnLock(void *mutex) {
  pthread_mutex_unlock(*(pthread_mutex_t **)mutex);
}
SMT_MUTEXAUTOLOCK_INIT(IOMutex);

int rear_idx = 0;
int front_idx = 0;
static IOData data_buffer[OUTPUT_BUFFER_SIZE];
std::string g_out_dir = ".";

typedef void *(*__AI_THREAD_FUN)(void *);

typedef struct {
  bool bEnable;
  uint8_t u8Chn;
  cviai_handle_t cviai_handle;
  __AI_THREAD_FUN pThreadFun;
  CVI_Thread_SC *pThread;
  CVIAPP_AiParam_S stParam;
} CVIAPP_AiContext_S;

typedef struct {
  bool bEnable;
  __AI_THREAD_FUN pThreadFun;
  CVI_Thread_SC *pThread;
  CVIAPP_AiParam_S stParam;
} CVIAPP_AiContext__S;

static CVI_Mutex_SC __ctxMutex;
static CVIAPP_AiContext_S __aiChnContextList[__MAX_AI_PROCESS]
                                            [CVIAPP_AI_MODEL_MAX];
static CVIAPP_AiContext__S __aiChnContext;

// RegionalInvasion
static cvi_nvr_ai_pts_t s_cvi_nvr_ai_pts[CVI_NVR_AI_MAX_REGION_NUM];

static CVI_ERROR_CODE_E VpssFRInit(void);
static CVI_ERROR_CODE_E VpssFRDeInit(void);
static CVI_ERROR_CODE_E CropVpssInit(void);
static CVI_ERROR_CODE_E CropVpssDeInit(void);
static CVI_ERROR_CODE_E CropJpegEncInit(int w, int h);
static CVI_ERROR_CODE_E CropJpegEncDeInit();
static void UpdateVpssCropGrpVdecParam(cvi_uint8 groupID, cvi_uint8 chnID,
                                       int w, int h);
static int loadCount(const char *dir_path);
static char **loadName(const char *dir_path, int count);
static int8_t *loadFeature(const char *dir_path, int count);

CVI_ERROR_CODE_E _CVIAPP_AiGetFeature(const char *db_img_dir,
                                      const char *db_feature_dir);

// YUV -> JPEG
static void __cropJpegEnc(char *fileName, VIDEO_FRAME_INFO_S *pstVFrame,
                          int8_t chn) {

  char jpgPath[JPEG_FILE_NAME_SIZE] = {0};
  char jpgFileName[64] = {0};

  __time_t tt;
  tt = time(NULL);

  struct tm stm;
  localtime_r(&tt, &stm);

  struct timeval tv;
  gettimeofday(&tv, NULL);
  int ms = tv.tv_usec / 1000;

  sprintf(jpgFileName, "%04d-%02d-%02d-%02d-%02d-%02d-%03d_%02d.jpeg",
          1900 + stm.tm_year, 1 + stm.tm_mon, stm.tm_mday, stm.tm_hour,
          stm.tm_min, stm.tm_sec, ms, chn);

  strcat(jpgPath, "/mnt/data/photo/");
  strcat(jpgPath, jpgFileName);
  strcpy(fileName, jpgPath);

  FILE *fp = fopen(jpgPath, "wb");
  if (NULL == fp) {
    CVI_NVRLOGE("fopen %s failed!", jpgPath);
    return;
  }

  CVI_S32 s32Ret = CVI_FAILURE;

  VENC_CHN VencChn = 6;

  VENC_CHN_STATUS_S stStat;
  VENC_STREAM_S stStream;
  // VENC_RECV_PIC_PARAM_S stRecvParam;

  memset(&stStat, 0, sizeof(VENC_CHN_STATUS_S));
  memset(&stStream, 0, sizeof(VENC_STREAM_S));
  // memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));

  // stRecvParam.s32RecvPicNum = -1;

  // s32Ret = CVI_VENC_StartRecvFrame(VencChn, &stRecvParam);
  // CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_StartRecvFrame
  // failed!");

  s32Ret = CVI_VENC_SendFrame(VencChn, pstVFrame, -1);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_SendFrame failed!");

  s32Ret = CVI_VENC_QueryStatus(VencChn, &stStat);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_QueryStatus failed!");

  if (0x00 != stStat.u32CurPacks) {
    stStream.pstPack =
        (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    CVI_NVRLOGW_CHECK((NULL == stStream.pstPack), "malloc VENC_PACK_S failed!");

    s32Ret = CVI_VENC_GetStream(VencChn, &stStream, -1);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_GetStream failed!");

    VENC_PACK_S *ppack = NULL;

    for (uint32_t i = 0; i < stStream.u32PackCount; i++) {
      ppack = &stStream.pstPack[i];
      fwrite(ppack->pu8Addr + ppack->u32Offset,
             ppack->u32Len - ppack->u32Offset, 1, fp);
    }

    free(stStream.pstPack);
    stStream.pstPack = NULL;

    s32Ret = CVI_VENC_ReleaseStream(VencChn, &stStream);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS),
                      "CVI_VENC_ReleaseStream failed!");
  } else {
    CVI_NVRLOGE("JPEG ENC: Current frame is NULL!");
  }

  // s32Ret = CVI_VENC_StopRecvFrame(VencChn);
  // CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_StopRecvFrame
  // failed!");

  fclose(fp);
}

static void initAiHandle_FaceTracking(cviai_handle_t *cviai_handle,
                                      int *aiHanleInitFlag, int i) {
  CVI_S32 s32Ret = CVI_SUCCESS;

  CVI_NVRLOGD("initAiHandle_FaceTracking  Start....");

  // s32Ret = CVI_AI_CreateHandle(cviai_handle);
  s32Ret = CVI_AI_CreateHandle2(cviai_handle, (2 + i), __VPSS_DEV_AI_HANDLE);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

  s32Ret = CVI_AI_OpenModel(*cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE,
                               FACE_RETINA_PATH);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

  s32Ret = CVI_AI_SetSkipVpssPreprocess(
      *cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS),
                    "CVI_AI_SetSkipVpssPreprocess failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

  s32Ret = CVI_AI_SetModelThreshold(*cviai_handle,
                                    CVI_AI_SUPPORTED_MODEL_RETINAFACE, 0.9);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS),
                    "CVI_AI_SetModelThreshold failed!");

  s32Ret = CVI_AI_SetVpssTimeout(*cviai_handle, 1200);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetVpssTimeout failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

  // Tracking
  s32Ret =
      CVI_AI_OpenModel(*cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN,
                          OBJECT_DETECT_MODEL_PATH);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

  s32Ret = CVI_AI_OpenModel(
      *cviai_handle, CVI_AI_SUPPORTED_MODEL_FACEQUALITY, FACE_QUALITY_PATH);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));
  s32Ret =
      CVI_AI_OpenModel(*cviai_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION,
                          FACE_RECOGNITION_PATH);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
  CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

  CVI_AI_SetModelThreshold(*cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN,
                           OBJECT_DETECT_CONFIDENCE);

  s32Ret = CVI_AI_SetSkipVpssPreprocess(
      *cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, false);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS),
                    "CVI_AI_SetSkipVpssPreprocess failed!");

  // Init DeepSORT
  CVI_AI_DeepSORT_Init(*cviai_handle, false);

  cvai_deepsort_config_t ds_conf;
  CVI_AI_DeepSORT_GetDefaultConfig(&ds_conf);
  ds_conf.ktracker_conf.max_unmatched_num = 10;
  ds_conf.ktracker_conf.accreditation_threshold = 10;
  ds_conf.ktracker_conf.P_beta[2] = 0.1;
  ds_conf.ktracker_conf.P_beta[6] = 2.5e-2;
  ds_conf.kfilter_conf.Q_beta[2] = 0.1;
  ds_conf.kfilter_conf.Q_beta[6] = 2.5e-2;
  ds_conf.kfilter_conf.R_beta[2] = 0.1;
  CVI_AI_DeepSORT_SetConfig(*cviai_handle, &ds_conf, -1, false);

  CVI_NVRLOGD("initAiHandle_FaceTracking  End....");

  *aiHanleInitFlag = 1;
}

// Tracking use
#define SAVE_TRACKER_NUM 32
#define QUALITY_THRESHOLD 0.95
#define COVER_RATE_THRESHOLD 0.9
#define MISS_TIME_LIMIT 100

typedef struct {
  uint64_t id;
  tracker_state_e state;
  cvai_feature_t feature;
  float quality;
  VIDEO_FRAME_INFO_S face;
  float pitch;
  float roll;
  float yaw;
} face_quality_tracker_t;

typedef struct {
  bool match;
  uint32_t idx;
} match_index_t;

float cover_rate_face2people(cvai_bbox_t face_bbox, cvai_bbox_t people_bbox) {
  float inter_x1 = MAX2(face_bbox.x1, people_bbox.x1);
  float inter_y1 = MAX2(face_bbox.y1, people_bbox.y1);
  float inter_x2 = MIN2(face_bbox.x2, people_bbox.x2);
  float inter_y2 = MIN2(face_bbox.y2, people_bbox.y2);
  float inter_w = MAX2(0.0f, inter_x2 - inter_x1);
  float inter_h = MAX2(0.0f, inter_y2 - inter_y1);
  float inter_area = inter_w * inter_h;
  float face_w = MAX2(0.0f, face_bbox.x2 - face_bbox.x1);
  float face_h = MAX2(0.0f, face_bbox.y2 - face_bbox.y1);
  float face_area = face_w * face_h;
  return inter_area / face_area;
}

void feature_copy(cvai_feature_t *src_feature, cvai_feature_t *dst_feature) {

  dst_feature->size = src_feature->size;
  dst_feature->type = src_feature->type;
  size_t type_size = getFeatureTypeSize(dst_feature->type);
  dst_feature->ptr = (int8_t *)malloc(dst_feature->size * type_size);
  memcpy(dst_feature->ptr, src_feature->ptr, dst_feature->size * type_size);
}

bool update_tracker(cviai_handle_t ai_handle, VIDEO_FRAME_INFO_S *frame,
                    face_quality_tracker_t *fq_trackers, cvai_face_t *face_meta,
                    cvai_tracker_t *tracker_meta, int *miss_time,
                    crop_face_info_t *cropFaceInfo, int8_t channel) {

  VPSS_CROP_INFO_S pstCropInfo;
  memset(&pstCropInfo, 0, sizeof(pstCropInfo));

  VIDEO_FRAME_INFO_S stCropFrameInfo;
  memset(&stCropFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));

  // miss_time++
  for (int j = 0; j < SAVE_TRACKER_NUM; j++) {
    if (fq_trackers[j].state == ALIVE) {
      miss_time[j] += 1;
    }
  }

  for (uint32_t i = 0; i < tracker_meta->size; i++) {
    /* we only consider the stable tracker in this sample code. */
    if (tracker_meta->info[i].state != CVI_TRACKER_STABLE) {
      continue;
    }
    uint64_t trk_id = face_meta->info[i].unique_id;
    /* check whether the tracker id exist or not. */
    int match_idx = -1;
    for (int j = 0; j < SAVE_TRACKER_NUM; j++) {
      if (fq_trackers[j].state == ALIVE && fq_trackers[j].id == trk_id) {
        match_idx = j;
        break;
      }
    }
    if (match_idx == -1) {
      /* if not found, create new one. */
      bool is_created = false;
      /* search available index for new tracker. */
      for (int j = 0; j < SAVE_TRACKER_NUM; j++) {
        if (fq_trackers[j].state == MISS) {
          miss_time[j] = 0;
          fq_trackers[j].state = ALIVE;
          fq_trackers[j].id = trk_id;
          fq_trackers[j].face.stVFrame.u32Height = 112;
          fq_trackers[j].face.stVFrame.u32Width = 112;
          for (int chn = 0; chn < 3; chn++) {
            fq_trackers[j].face.stVFrame.pu8VirAddr[chn] =
                (CVI_U8 *)malloc(112 * 112 * sizeof(CVI_U8));
            memset(fq_trackers[j].face.stVFrame.pu8VirAddr[chn], 0,
                   112 * 112 * sizeof(CVI_U8));
          }
          fq_trackers[j].pitch = face_meta->info[i].head_pose.pitch;
          fq_trackers[j].roll = face_meta->info[i].head_pose.roll;
          fq_trackers[j].yaw = face_meta->info[i].head_pose.yaw;

          // cropFaceInfo for database
          feature_copy(&face_meta->info[i].feature, &cropFaceInfo->feature);
          cropFaceInfo->channel = channel;
          // printf("feature.size:%d\n",cropFaceInfo->feature.size);

          if (face_meta->info[i].face_quality >= QUALITY_THRESHOLD) {
            fq_trackers[j].quality = face_meta->info[i].face_quality;
            feature_copy(&fq_trackers[j].feature, &face_meta->info[i].feature);
            // CVI_S32 ret = CVI_AI_GetAlignedFace(
            //     ai_handle, frame, &fq_trackers[j].face, &face_meta->info[i]);
            // if (ret != CVI_SUCCESS) {
            //   printf("AI get aligned face failed(1).\n");
            //   return false;
            // }
          }
          is_created = true;
          CVI_NVRLOGD("tracker_created!\n");

          // crop & encode jpeg
          pstCropInfo.bEnable = true;
          pstCropInfo.stCropRect.s32X = tracker_meta->info[i].bbox.x1;
          pstCropInfo.stCropRect.s32Y = tracker_meta->info[i].bbox.y1;
          pstCropInfo.stCropRect.u32Width =
              tracker_meta->info[i].bbox.x2 - tracker_meta->info[i].bbox.x1;
          pstCropInfo.stCropRect.u32Height =
              tracker_meta->info[i].bbox.y2 - tracker_meta->info[i].bbox.y1;

          if ((pstCropInfo.stCropRect.u32Width % 2) != 0)
            pstCropInfo.stCropRect.u32Width++;
          if ((pstCropInfo.stCropRect.u32Height % 2) != 0)
            pstCropInfo.stCropRect.u32Height++;

          cvi_uint8 groupID = 0;
          if (channel == 0) {
            groupID = __VPSS_GRP_CROP;
          } else if (channel == 1) {
            groupID = __VPSS_GRP_CROP_1;
          }

          UpdateVpssCropGrpVdecParam(groupID, 0,
                                     pstCropInfo.stCropRect.u32Width,
                                     pstCropInfo.stCropRect.u32Height);
          CVI_S32 ret = CVI_VPSS_SetChnCrop(groupID, 0, &pstCropInfo);
          CVI_NVRLOGW_CHECK((ret != 0), "CVI_VPSS_SetChnCrop failed!");

          ret = CVI_VPSS_SendFrame(groupID, frame, -1);
          CVI_NVRLOGW_CHECK((ret != CVI_SUCCESS), "CVI_VPSS_SendFrame failed!");

          ret = CVI_VPSS_GetChnFrame(groupID, 0, &stCropFrameInfo, 600);
          CVI_NVRLOGW_CHECK((ret != CVI_SUCCESS),
                            "CVI_VPSS_GetChnFrame failed!");

          // CropJpegEncInit(stCropFrameInfo.stVFrame.u32Width,
          // stCropFrameInfo.stVFrame.u32Height);
          __aiImageMutex.Lock();
          // YUV -> JPEG
          __cropJpegEnc(cropFaceInfo->cropJpegFileName, &stCropFrameInfo,
                        channel);
          __aiImageMutex.Unlock();
          // CropJpegEncDeInit();

          ret = CVI_VPSS_ReleaseChnFrame(__VPSS_GRP_CROP, 0, &stCropFrameInfo);
          CVI_NVRLOGW_CHECK((ret != CVI_SUCCESS),
                            "CVI_VPSS_ReleaseChnFrame failed!");

          break;
        }
      }
      /* if fail to create, return false. */
      if (!is_created) {
        printf("buffer overflow.\n");
        return false;
      }
    } else {
      /* if found, check whether the quality(or feature) need to be update. */
      miss_time[match_idx] = 0;
      fq_trackers[match_idx].pitch = face_meta->info[i].head_pose.pitch;
      fq_trackers[match_idx].roll = face_meta->info[i].head_pose.roll;
      fq_trackers[match_idx].yaw = face_meta->info[i].head_pose.yaw;
      if (face_meta->info[i].face_quality >= QUALITY_THRESHOLD &&
          face_meta->info[i].face_quality > fq_trackers[match_idx].quality) {
        fq_trackers[match_idx].quality = face_meta->info[i].face_quality;
        feature_copy(&fq_trackers[match_idx].feature,
                     &face_meta->info[i].feature);
        CVI_S32 ret = 0;
        // CVI_S32 ret = CVI_AI_GetAlignedFace(ai_handle, frame,
        //                                     &fq_trackers[match_idx].face,
        //                                     &face_meta->info[i]);
        if (ret != CVI_SUCCESS) {
          printf("AI get aligned face failed(2).\n");
          return false;
        }
      }
      // printf("update_tracker\n\n");
    }
  }
  return true;
}

void clean_tracker(face_quality_tracker_t *fq_trackers, int *miss_time) {
  for (int j = 0; j < SAVE_TRACKER_NUM; j++) {
    if (fq_trackers[j].state == ALIVE && miss_time[j] > MISS_TIME_LIMIT) {
      free(fq_trackers[j].face.stVFrame.pu8VirAddr[0]);
      free(fq_trackers[j].face.stVFrame.pu8VirAddr[1]);
      free(fq_trackers[j].face.stVFrame.pu8VirAddr[2]);

      memset(&fq_trackers[j], 0, sizeof(face_quality_tracker_t));
      miss_time[j] = -1;
    }
  }
}

// cvi_nvr Face Detection
static void *__cviAiBoxAiThread(void *args) {
  CVI_S32 s32Ret = CVI_SUCCESS;
  CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *)args;

  cviai_handle_t cviai_handle;

  CVI_NVRLOGD("__cviAiBoxAiThread  start....");
  CVI_AI_CreateHandle2(&cviai_handle, 3, __VPSS_DEV_AI_HANDLE);
  CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE,
                               FACE_RETINA_PATH);
  CVI_AI_SetSkipVpssPreprocess(
      cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);

  VIDEO_FRAME_INFO_S stVideoFrameInfo[CVIAPP_AIBOX_DEVICE_NUM_MAX];
  memset(&stVideoFrameInfo, 0,
         sizeof(VIDEO_FRAME_INFO_S) * CVIAPP_AIBOX_DEVICE_NUM_MAX);

#ifdef ENABLE_PRINT_FPS
  struct timespec tv1, tv2;
  clock_gettime(CLOCK_MONOTONIC, &tv1);
  struct timespec tv1m, tv2m;
  clock_gettime(CLOCK_MONOTONIC, &tv1m);

  int aiFrameCount[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
  int aiFrameCountMin[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
  int aiFrameCount_total = 0;
  int aiFrameCountMin_total = 0;
#endif

  CVI_NVRLOGD("__cviAiBoxAiThread  pCtx->bEnable=%d", pCtx->bEnable);
  while (pCtx->bEnable) {
    for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++) {
      if (NULL != pCtx->stParam.pGetFrameFun) {

        if (pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0) {
          // face
          cvai_face_t face;
          memset(&face, 0, sizeof(cvai_face_t));

          s32Ret = CVI_AI_RetinaFace(cviai_handle, &stVideoFrameInfo[i], &face);
          CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS),
                            "CVI_AI_RetinaFace failed!");

          if (CVI_SUCCESS == s32Ret && face.size != 0) {
            if (NULL != pCtx->stParam.pNotifyFun) {
              CVIAPP_AiResult_S result;
              CVIAPP_AiResultInfo_S resultInfo;

              memset(&result, 0, sizeof(CVIAPP_AiResult_S));
              memset(&resultInfo, 0, sizeof(CVIAPP_AiResultInfo_S));
              resultInfo.face = face;

              result.eType = CVIAPP_AI_CVINVRTHREAD;
              result.pResult = (void *)&resultInfo;
              pCtx->stParam.pNotifyFun(i, &result);
            } else {
              CVI_NVRLOGD("__cviAiBoxAiThread pNotifyFun is NULL");
            }
          }

          CVI_AI_Free(&face);

          if (NULL != pCtx->stParam.pReleaseFrameFun) {
            pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
          }

#ifdef ENABLE_PRINT_FPS
          aiFrameCount[i]++;
          aiFrameCount_total++;
          clock_gettime(CLOCK_MONOTONIC, &tv2);
          if (tv2.tv_sec > tv1.tv_sec) {
            printf("AI_total:%d\n", aiFrameCount_total);
            for (int j = 0; j < CVIAPP_AIBOX_DEVICE_NUM_MAX; j++) {
              if (aiFrameCount[j] != 0) {
                printf("AI_%d:%d\n", j, aiFrameCount[j]);
              }
            }

            aiFrameCount_total = 0;
            memset(aiFrameCount, 0, sizeof(aiFrameCount));
            clock_gettime(CLOCK_MONOTONIC, &tv1);
          }

          aiFrameCountMin[i]++;
          aiFrameCountMin_total++;
          clock_gettime(CLOCK_MONOTONIC, &tv2m);
          if ((tv2m.tv_sec - tv1m.tv_sec) >= 60) {
            printf("One min AI_total: %d \n", aiFrameCountMin_total);
            aiFrameCountMin_total = 0;
            for (int j = 0; j < CVIAPP_AIBOX_DEVICE_NUM_MAX; j++) {
              if (aiFrameCountMin[j] != 0) {
                printf("AI_min_%d:%d\n", j, aiFrameCountMin[j]);
              }
            }

            memset(aiFrameCountMin, 0, sizeof(aiFrameCountMin));
            clock_gettime(CLOCK_MONOTONIC, &tv1m);
          }
#endif
        }
      }
      //}
    }
  }

  s32Ret = CVI_AI_DestroyHandle(cviai_handle);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");

  CVI_NVRLOGD("\n#__cviAiBoxAiThread  end....");

  return NULL;
}

/*face capture*/
int COUNT_ALIVE(face_capture_t *face_cpt_info) {
  int counter = 0;
  for (uint32_t j = 0; j < face_cpt_info->size; j++) {
    if (face_cpt_info->data[j].state == ALIVE) {
      counter += 1;
    }
  }
  return counter;
}

/* Consumer */
static void *pImageWrite(void *args) {
  printf("[APP] Image Write Up\n");
  while (bRunImageWriter) {
    /* only consumer write front_idx */
    bool empty;
    {
      SMT_MutexAutoLock(IOMutex, lock);
      empty = front_idx == rear_idx;
    }
    if (empty) {
      printf("I/O Buffer is empty.\n");
      usleep(100 * 1000);
      continue;
    }
    int target_idx = (front_idx + 1) % OUTPUT_BUFFER_SIZE;
    char filename[128];

    if (data_buffer[target_idx].state != MISS) {
      sprintf(filename, "%s/frm_%d_face_%d_%u_score_%.3f_qua_%.3f_name_%s.png", g_out_dir.c_str(),
              int(data_buffer[target_idx].frame_id), int(data_buffer[target_idx].u_id),
              data_buffer[target_idx].counter, data_buffer[target_idx].match_score,
              data_buffer[target_idx].quality, data_buffer[target_idx].name);
    } else {
      sprintf(filename, "%s/frm_%d_face_%d_%u_score_%.3f_qua_%.3f_name_%s_out.png",
              g_out_dir.c_str(), int(data_buffer[target_idx].frame_id),
              int(data_buffer[target_idx].u_id), data_buffer[target_idx].counter,
              data_buffer[target_idx].match_score, data_buffer[target_idx].quality,
              data_buffer[target_idx].name);
    }
    if (data_buffer[target_idx].image.pix_format != PIXEL_FORMAT_RGB_888) {
      printf("[WARNING] Image I/O unsupported format: %d\n",
             data_buffer[target_idx].image.pix_format);
    } else {
      if (data_buffer[target_idx].image.width == 0) {
        printf("[WARNING] Target image is empty.\n");
      } else {
        printf(" > (I/O) Write Face (Q: %.2f): %s ...\n", data_buffer[target_idx].quality,
               filename);
        stbi_write_png(filename, data_buffer[target_idx].image.width,
                       data_buffer[target_idx].image.height, STBI_rgb,
                       data_buffer[target_idx].image.pix[0],
                       data_buffer[target_idx].image.stride[0]);
      }
    }

    CVI_AI_Free(&data_buffer[target_idx].image);
    {
      SMT_MutexAutoLock(IOMutex, lock);
      front_idx = target_idx;
    }
  }

  printf("[APP] free buffer data...\n");
  while (front_idx != rear_idx) {
    CVI_AI_Free(&data_buffer[(front_idx + 1) % OUTPUT_BUFFER_SIZE].image);
    {
      SMT_MutexAutoLock(IOMutex, lock);
      front_idx = (front_idx + 1) % OUTPUT_BUFFER_SIZE;
    }
  }

  return NULL;
}

std::string capobj_to_str(face_cpt_data_t *p_obj, float w, float h, int lb) {
  std::stringstream ss;
  // ss<<p_obj->_timestamp<<",4,";
  float ctx = (p_obj->info.bbox.x1 + p_obj->info.bbox.x2) / 2.0 / w;
  float cty = (p_obj->info.bbox.y1 + p_obj->info.bbox.y2) / 2.0 / h;
  float ww = (p_obj->info.bbox.x2 - p_obj->info.bbox.x1) / w;
  float hh = (p_obj->info.bbox.y2 - p_obj->info.bbox.y1) / h;
  ss << p_obj->_timestamp << "," << lb << "," << ctx << "," << cty << "," << ww << "," << hh << ","
     << p_obj->info.unique_id << "," << p_obj->info.bbox.score << "\n";
  return ss.str();
}

// cvi_nvr Face Detection And Capture
static void *__cviFaceCapThread(void *args) {
  CVI_S32 s32Ret = CVI_SUCCESS;
  CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *)args;
  cviai_handle_t ai_handle = NULL;
  cviai_service_handle_t service_handle = NULL;
  cviai_app_handle_t app_handle = NULL;
  uint32_t buffer_size = 10;
  float det_threshold = 0.5;
  bool write_image = true;
  int ret = 0;

  CVI_NVRLOGD("__cviFaceCapThread  start....");

  CVI_AI_CreateHandle2(&ai_handle, 3, __VPSS_DEV_AI_HANDLE);
  CVI_AI_APP_CreateHandle(&app_handle, ai_handle);
  CVI_AI_APP_FaceCapture_Init(app_handle, buffer_size);
  printf("to quick setup\n");
  
  CVI_AI_APP_FaceCapture_QuickSetUp(app_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, FACE_RETINA_PATH,
                                           NULL, NULL);
  CVI_AI_SetModelThreshold(ai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, det_threshold);

  std::cout << "to start:\n";
  CVI_AI_APP_FaceCapture_SetMode(app_handle, AUTO);

  face_capture_config_t app_cfg;
  CVI_AI_APP_FaceCapture_GetDefaultConfig(&app_cfg);

  app_cfg.thr_quality = 0.1;
  app_cfg.thr_quality_high = 0.95;
  app_cfg.thr_size_min = 20;
  app_cfg.miss_time_limit = 20;
  app_cfg.store_RGB888 = true;
  app_cfg.store_feature = true;
  app_cfg.qa_method = 0;
  CVI_AI_APP_FaceCapture_SetConfig(app_handle, &app_cfg);

  pthread_t io_thread;
  pthread_create(&io_thread, NULL, pImageWrite, NULL);

  VIDEO_FRAME_INFO_S stVideoFrameInfo[CVIAPP_AIBOX_DEVICE_NUM_MAX];
  memset(&stVideoFrameInfo, 0,
         sizeof(VIDEO_FRAME_INFO_S) * CVIAPP_AIBOX_DEVICE_NUM_MAX);

  CVI_NVRLOGD("__cviAiBoxAiThread  pCtx->bEnable=%d", pCtx->bEnable);

  bool empty_img = false;
  int num_append = 0;
  const int face_label = 11;
  typedef enum { fast = 0, interval, leave, intelligent } APP_MODE_e;
  
  while (pCtx->bEnable) {
    for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++) {
      if (NULL != pCtx->stParam.pGetFrameFun) {

        if (pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0) {
          int alive_person_num = COUNT_ALIVE(app_handle->face_cpt_info);
          ret = CVI_AI_APP_FaceCapture_Run(app_handle, &stVideoFrameInfo[i]);
          if (ret != CVI_SUCCESS) {
            printf("CVI_AI_APP_FaceCapture_Run failed with %#x\n", ret);
            break;
          }
        
          /* Producer */
          if (write_image) {
            for (uint32_t i = 0; i < app_handle->face_cpt_info->size; i++) {
              cvai_face_info_t *pface_info = &app_handle->face_cpt_info->data[i].info;
              tracker_state_e state = app_handle->face_cpt_info->data[i].state;
              uint32_t counter = app_handle->face_cpt_info->data[i]._out_counter;
              uint64_t u_id = app_handle->face_cpt_info->data[i].info.unique_id;
              float face_quality = app_handle->face_cpt_info->data[i].info.face_quality;
              if (state == MISS) {
                printf("Produce Face-%" PRIu64 "_out alive_person_num:%d\n", u_id, alive_person_num);
              } else {
                printf("Produce Face-%" PRIu64 "_%u alive_person_num:%d\n", u_id, counter, alive_person_num);
                continue;
              }
              std::string str_res =
                  capobj_to_str(&app_handle->face_cpt_info->data[i], stVideoFrameInfo[i].stVFrame.u32Width,
                                stVideoFrameInfo[i].stVFrame.u32Height, face_label);
              FILE *fp = fopen("cap_result.log", "w");
              fwrite(str_res.c_str(), 1, str_res.length(), fp);
              fclose(fp);
              /* Check output buffer space */
              bool full;
              int target_idx;
              {
                SMT_MutexAutoLock(IOMutex, lock);
                target_idx = (rear_idx + 1) % OUTPUT_BUFFER_SIZE;
                full = target_idx == front_idx;
              }
              if (full) {
                printf("[WARNING] Buffer is full! Drop out!");
                continue;
              }
              /* Copy image data to buffer */
              memset(&data_buffer[target_idx], 0, sizeof(data_buffer[target_idx]));
              data_buffer[target_idx].u_id = u_id;
              data_buffer[target_idx].quality = face_quality;
              data_buffer[target_idx].state = state;
              data_buffer[target_idx].counter = counter;
              data_buffer[target_idx].match_score = pface_info->recog_score;
              data_buffer[target_idx].frame_id = app_handle->face_cpt_info->data[i].cap_timestamp;
              memcpy(data_buffer[target_idx].name, pface_info->name, 128);
              /* NOTE: Make sure the image type is IVE_IMAGE_TYPE_U8C3_PACKAGE */

              CVI_AI_CopyImage(&app_handle->face_cpt_info->data[i].image, &data_buffer[target_idx].image);
              {
                SMT_MutexAutoLock(IOMutex, lock);
                rear_idx = target_idx;
              }
            }
          }

          // if (CVI_SUCCESS == s32Ret && face.size != 0) {
          //   if (NULL != pCtx->stParam.pNotifyFun) {
          //     CVIAPP_AiResult_S result;
          //     CVIAPP_AiResultInfo_S resultInfo;

          //     memset(&result, 0, sizeof(CVIAPP_AiResult_S));
          //     memset(&resultInfo, 0, sizeof(CVIAPP_AiResultInfo_S));
          //     resultInfo.face = face;

          //     result.eType = CVIAPP_AI_CVINVRTHREAD;
          //     // result.pResult = (void*) &face;
          //     result.pResult = (void *)&resultInfo;
          //     pCtx->stParam.pNotifyFun(i, &result);
          //   } else {
          //     CVI_NVRLOGD("__cviAiBoxAiThread pNotifyFun is NULL");
          //   }
          // }

          // CVI_AI_Free(&face);

          if (NULL != pCtx->stParam.pReleaseFrameFun) {
            pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
          }
        }
      }
    }
  }

  
  pthread_join(io_thread, NULL);
  CVI_AI_APP_DestroyHandle(app_handle);
  CVI_AI_DestroyHandle(ai_handle);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");

  CVI_NVRLOGD("\n#__cviAiBoxAiThread  end....");
  return NULL;
}

#if 0
//cvi_nvr_Region
static void *__cviNvrAiRegionalInvasionThread(void *args)
{

    //Support 4 channels
    int support_chn_num = CVI_NVR_AI_MAX_REGION_NUM;

    int32_t releaseFrameStatus[support_chn_num] = { 0 };
    CVI_S32 s32Ret = CVI_SUCCESS;
    //CVIAPP_AiContext_S *pCtx = (CVIAPP_AiContext_S *) args;
    CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *) args;

    cviai_service_handle_t ai_serviceHandle[support_chn_num];
    cviai_handle_t cviai_handle[support_chn_num];

    cvai_pts_t polygonPts[support_chn_num];
    memset(&polygonPts, 0 , support_chn_num * sizeof(cvai_pts_t));

    for (size_t i = 0; i < support_chn_num; i++)
    {
        s32Ret = CVI_AI_CreateHandle(&cviai_handle[i]);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle failed!");
        CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

        s32Ret = CVI_AI_Service_CreateHandle(&ai_serviceHandle[i], &cviai_handle[i]);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_CreateHandle failed!");
        CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

        s32Ret = CVI_AI_OpenModel(cviai_handle[i], CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, OBJECT_DETECT_MODEL_PATH);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
        CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

        s32Ret = CVI_AI_SetModelThreshold(cviai_handle[i], CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, OBJECT_DETECT_CONFIDENCE);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetModelThreshold failed!");
        CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

        s32Ret = CVI_AI_SetSkipVpssPreprocess(cviai_handle[i], CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, false);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetSkipVpssPreprocess failed!");
        CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

        if (s_cvi_nvr_ai_pts[i].size > 0)
        {
            polygonPts[i].size = s_cvi_nvr_ai_pts[i].size;
            polygonPts[i].x = s_cvi_nvr_ai_pts->x;
            polygonPts[i].y = s_cvi_nvr_ai_pts->y;

            s32Ret = CVI_AI_Service_Polygon_SetTarget(ai_serviceHandle[i], &polygonPts[i]);
            CVI_NVRLOGD("CVI_AI_Service_Polygon_SetTarget ret:%d", s32Ret);
        }
    }

    CVI_NVRLOGD("__cviNvrAiRegionalInvasionThread  start....");

    VIDEO_FRAME_INFO_S stVideoFrameInfo[support_chn_num];
    memset(&stVideoFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S) * support_chn_num);

    //追踪
    // // Init DeepSORT
    // s32Ret = CVI_AI_DeepSORT_Init(cviai_handle);

    // cvai_deepsort_config_t ds_conf;
    // s32Ret |= CVI_AI_DeepSORT_GetDefaultConfig(&ds_conf);
    // ds_conf.ktracker_conf.max_unmatched_num = 10;
    // ds_conf.ktracker_conf.accreditation_threshold = 10;
    // ds_conf.ktracker_conf.P_std_beta[2] = 0.1;
    // ds_conf.ktracker_conf.P_std_beta[6] = 2.5e-2;
    // ds_conf.kfilter_conf.Q_std_beta[2] = 0.1;
    // ds_conf.kfilter_conf.Q_std_beta[6] = 2.5e-2;
    // ds_conf.kfilter_conf.R_std_beta[2] = 0.1;
    // s32Ret |= CVI_AI_DeepSORT_SetConfig(cviai_handle, &ds_conf);

#ifdef ENABLE_PRINT_FPS
    struct timespec tv1, tv2;
    clock_gettime(CLOCK_MONOTONIC, &tv1);
    struct timespec tv1m, tv2m;
    clock_gettime(CLOCK_MONOTONIC, &tv1m);

    int aiFrameCount[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
    int aiFrameCountMin[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
    int aiFrameCount_total = 0;
    int aiFrameCountMin_total = 0;
#endif

    CVIBSV_RTSP_CLIENT_STATUS_E eStatus = E_CVIBSV_RTSP_CLIENT_STATUS_IDLE;

    while(pCtx->bEnable)
    {
        for (int i = 0; i < support_chn_num; i++)
        {
            eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);

            if(E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT == eStatus)
            {
                if(NULL != pCtx->stParam.pGetFrameFun  && polygonPts[i].size > 0)
                {
                    if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0)
                    {

                    //obj
                    cvai_object_t obj_meta;
                    memset(&obj_meta, 0, sizeof(cvai_object_t));
                    cvai_tracker_t tracker_meta;
                    memset(&tracker_meta, 0, sizeof(cvai_tracker_t));

                    //s32Ret = CVI_AI_MobileDetV2_D0(cviai_handle, &stVideoFrameInfo, &obj_meta, CVI_DET_TYPE_ALL);
                    s32Ret = CVI_AI_MobileDetV2_D0(cviai_handle[i], &stVideoFrameInfo[i], &obj_meta, CVI_DET_TYPE_PEOPLE);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_MobileDetV2_D0 failed!");

                    //CVI_AI_DeepSORT_Obj(cviai_handle, &obj_meta, &tracker_meta, true);
                    //updateObjectTracker(cviai_handle, pd_trackers, &obj_meta, &tracker_meta)
                    //cleanObjectTracker(pd_trackers);

                    int validNum = 0;
                    int *validIndex = (int *)malloc(obj_meta.size * sizeof(int));
                    memset(validIndex, 0, obj_meta.size * sizeof(int));

                    for (CVI_U32 a = 0; a < obj_meta.size; a++) {
                        bool hasIntersect = false;
                        CVI_AI_Service_Polygon_Intersect(ai_serviceHandle[i], &obj_meta.info[a].bbox, &hasIntersect);
                        if (hasIntersect == true) {
                            validIndex[a] = 1;
                            validNum++;
                        } else {
                            validIndex[a] = 0;
                        }
                    }
                    //printf("\n#in:%d\n",validNum);

                    if(CVI_SUCCESS == s32Ret && obj_meta.size != 0)
                    {
                        if(NULL != pCtx->stParam.pNotifyFun)
                        {
                            CVIAPP_AiResult_S result;
                            CVIAPP_AiRegionResultInfo_S regionResult;

                            memset(regionResult.inRegion, 0, sizeof(regionResult.inRegion));
                            memset(&regionResult.obj_meta, 0, sizeof(cvai_object_t));
                            regionResult.obj_meta = obj_meta;
                            for (size_t i = 0; i < obj_meta.size; i++)
                            {
                                if (validIndex[i])
                                    regionResult.inRegion[i] = 1;
                            }

                            result.eType = CVIAPP_AI_CVINVRREGION;
                            result.pResult = (void*) &regionResult;

                            pCtx->stParam.pNotifyFun(i, &result);
                        }
                    }
                    CVI_AI_Free(&obj_meta);
                    free(validIndex);

                    if(NULL != pCtx->stParam.pReleaseFrameFun)
                    {
                        pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
                        releaseFrameStatus[i] = 1;
                    }

#ifdef ENABLE_PRINT_FPS
                    aiFrameCount[i]++;
                    aiFrameCount_total++;
                    clock_gettime(CLOCK_MONOTONIC, &tv2);
                    if(tv2.tv_sec > tv1.tv_sec)
                    {
                        printf("AI_total:%d\n",aiFrameCount_total);
                        for (int j = 0; j < CVIAPP_AIBOX_DEVICE_NUM_MAX; j++)
                        {
                            if (aiFrameCount[j] != 0)
                            {
                                printf("AI_%d:%d\n",j,aiFrameCount[j]);
                            }
                        }

                        aiFrameCount_total = 0;
                        memset(aiFrameCount,0,sizeof(aiFrameCount));
                        clock_gettime(CLOCK_MONOTONIC, &tv1);
                    }

                    aiFrameCountMin[i]++;
                    aiFrameCountMin_total++;
                    clock_gettime(CLOCK_MONOTONIC, &tv2m);
                    if((tv2m.tv_sec - tv1m.tv_sec) >= 60)
                    {
                        printf("One min AI_total: %d \n",aiFrameCountMin_total);
                        aiFrameCountMin_total = 0;
                        for (int j = 0; j < CVIAPP_AIBOX_DEVICE_NUM_MAX; j++)
                        {
                            if (aiFrameCountMin[j] != 0)
                            {
                                printf("AI_min_%d:%d\n",j,aiFrameCountMin[j]);
                            }
                        }

                        memset(aiFrameCountMin,0,sizeof(aiFrameCountMin));
                        clock_gettime(CLOCK_MONOTONIC, &tv1m);
                    }
#endif
                    }
                    else
                    {
                        releaseFrameStatus[i] = 0;
                    }
                }
            }
        }
    }

    // make sure release
    // for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++)
    // {
    //     eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);
    //     if (releaseFrameStatus[i] == 0 && eStatus == E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT){
    //         while (1){
    //             if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0){
    //                 pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
    //                 releaseFrameStatus[i] = 1;
    //                 break;}
    //         }
    //     }
    // }


    for (size_t i = 0; i < support_chn_num; i++)
    {
        s32Ret = CVI_AI_DestroyHandle(cviai_handle[i]);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");
        s32Ret =CVI_AI_Service_DestroyHandle(ai_serviceHandle[i]);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_DestroyHandle failed!");
    }

    CVI_NVRLOGD("\n#__cviNvrAiRegionalInvasionThread  end....");

    return NULL;
}


static void *__cviFaceTrackingAndCropThread(void *args)
{
    //支持通道数2
    int support_chn_num = 2;

    int32_t releaseFrameStatus[support_chn_num] = { 0 };
    CVI_S32 s32Ret = CVI_SUCCESS;
    //CVIAPP_AiContext_S *pCtx = (CVIAPP_AiContext_S *) args;
    CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *) args;

    cviai_handle_t cviai_handle[support_chn_num];
    memset(cviai_handle,0,sizeof(cviai_handle_t) * support_chn_num);
    CVI_NVRLOGD("\n#__cviFaceTrackingAndCropThread  start....");

    VIDEO_FRAME_INFO_S stVideoFrameInfo[support_chn_num];
    memset(stVideoFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S) * support_chn_num);
    int aiHanleInitFlag[support_chn_num] = {0};

    face_quality_tracker_t fq_trackers[support_chn_num][SAVE_TRACKER_NUM];
    memset(fq_trackers, 0, sizeof(face_quality_tracker_t) * SAVE_TRACKER_NUM * support_chn_num);
    int miss_time[support_chn_num][SAVE_TRACKER_NUM];
    memset(miss_time, -1, sizeof(int) * SAVE_TRACKER_NUM *support_chn_num);

    CropVpssInit();
    VpssFRInit();     //VPSS For FACERECOGNITION: YUV420 To RGB888

    CropJpegEncInit(102, 122);  // w : 102  H : 122

#ifdef ENABLE_PRINT_FPS
    struct timespec tv1, tv2;
    clock_gettime(CLOCK_MONOTONIC, &tv1);
    struct timespec tv1m, tv2m;
    clock_gettime(CLOCK_MONOTONIC, &tv1m);

    int aiFrameCount[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
    int aiFrameCountMin[CVIAPP_AIBOX_DEVICE_NUM_MAX] = {0};
    int aiFrameCount_total = 0;
    int aiFrameCount_min_total = 0;
#endif

    CVIBSV_RTSP_CLIENT_STATUS_E eStatus = E_CVIBSV_RTSP_CLIENT_STATUS_IDLE;

    for (int i = 0; i < support_chn_num; i++)
    {
        if (aiHanleInitFlag[i] != 1)
        {
            initAiHandle_FaceTracking(&cviai_handle[i],&aiHanleInitFlag[i],i);
        }
    }

    while(pCtx->bEnable)
    {
        for (int i = 0; i < support_chn_num; i++)
        {

        eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);
        if(E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT == eStatus)
        {
            if(NULL != pCtx->stParam.pGetFrameFun)
            {
                if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0)
                {
                    VIDEO_FRAME_INFO_S stRGBFrameInfo;
                    memset(&stRGBFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));

                    // face
                    cvai_face_t face;
                    memset(&face, 0, sizeof(cvai_face_t));
                    cvai_object_t obj_meta;
                    memset(&obj_meta, 0, sizeof(cvai_object_t));
                    //track
                    cvai_tracker_t tracker_meta;
                    memset(&tracker_meta, 0, sizeof(cvai_tracker_t));

                    s32Ret = CVI_AI_RetinaFace(cviai_handle[i], &stVideoFrameInfo[i], &face);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_RetinaFace failed! ret = %d\n",s32Ret);

                    if(CVI_SUCCESS == s32Ret && face.size != 0)
                    {
                        if(NULL != pCtx->stParam.pNotifyFun)
                        {
                            CVIAPP_AiResult_S result;
                            CVIAPP_AiResultInfo_S resultInfo;
                            resultInfo.face = face;
                            result.eType = CVIAPP_AI_MODEL_RETINAFACE;
                            //result.pResult = (void*) &face;
                            result.pResult = (void*) &resultInfo;
                            pCtx->stParam.pNotifyFun(i, &result);
                        }
                    }

                    s32Ret = CVI_VPSS_SendFrame(__VPSS_GRP_AI,&stVideoFrameInfo[i], -1);  //YUV420 to RGB888
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SendFrame failed! ret = %d", s32Ret);
                    s32Ret = CVI_VPSS_GetChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo, 600);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_GetChnFrame failed! ret = %d", s32Ret);
                    s32Ret = CVI_AI_FaceRecognition(cviai_handle[i], &stRGBFrameInfo, &face);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_FaceRecognition failed! ret = %d", s32Ret);
                    CVI_AI_FaceQuality(cviai_handle[i], &stRGBFrameInfo, &face);          // input Format must be RGB888
                    CVI_AI_DeepSORT_Face(cviai_handle[i], &face, &tracker_meta, false);

                    crop_face_info_t cropFaceInfo;
                    memset(&cropFaceInfo, 0 ,sizeof(cropFaceInfo));

                    if (!update_tracker(cviai_handle[i], &stVideoFrameInfo[i], fq_trackers[i], &face, &tracker_meta, miss_time[i], &cropFaceInfo, i)) {
                        printf("update tracker failed.\n");
                        CVI_AI_Free(&face);
                        CVI_AI_Free(&tracker_meta);
                        CVI_AI_Free(&obj_meta);
                        break;
                    }
                    if (0 != strlen(cropFaceInfo.cropJpegFileName))
                    {
                        printf("%s\n",cropFaceInfo.cropJpegFileName);
                        if(NULL != pCtx->stParam.pNotifyFun)
                        {
                            CVIAPP_AiResult_S result;
                            memset(&result, 0 , sizeof(CVIAPP_AiResult_S));

                            result.eType = CVIAPP_AI_FACETRACKING;
                            //result.pResult = (void*) &face;
                            result.pResult = (void*) &cropFaceInfo;
                            pCtx->stParam.pNotifyFun(0, &result);
                        }
                    }

                    clean_tracker(fq_trackers[i], miss_time[i]);

                    cropFaceInfo.feature.size = 0;
                    cropFaceInfo.feature.type = TYPE_INT8;
                    free(cropFaceInfo.feature.ptr);

                    s32Ret = CVI_VPSS_ReleaseChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ReleaseChnFrame failed!");

                    if(NULL != pCtx->stParam.pReleaseFrameFun)
                    {
                        pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
                        releaseFrameStatus[i] = 1;
                    }

                    CVI_AI_Free(&face);
                    CVI_AI_Free(&tracker_meta);
                    CVI_AI_Free(&obj_meta);
                }
                else
                {
                    releaseFrameStatus[i] = 0;
                }
            }
        }
        }
    }

    // // make sure release
    // for (int i = 0; i < 2; i++)  // two chn
    // {
    //     eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);
    //     if (releaseFrameStatus[i] == 0 && eStatus == E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT){
    //         while (1){
    //             if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0){
    //                 pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
    //                 releaseFrameStatus[i] = 1;
    //                 break;}
    //         }
    //     }
    // }

    VpssFRDeInit();
    CropVpssDeInit();
    CropJpegEncDeInit();

    for (size_t i = 0; i < support_chn_num; i++)
    {
        s32Ret = CVI_AI_DestroyHandle(cviai_handle[i]);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");
    }

    CVI_NVRLOGD("\n#__cviFaceTrackingAndCropThread  end....");

    return NULL;

}





static void *__aiRFAndODThread(void *args)
{
    int32_t releaseFrameStatus[CVIAPP_AIBOX_DEVICE_NUM_MAX] = { 0 };
    CVI_S32 s32Ret = CVI_SUCCESS;
    //CVIAPP_AiContext_S *pCtx = (CVIAPP_AiContext_S *) args;
    CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *) args;

    cviai_service_handle_t service_handle = NULL;

    cviai_handle_t cviai_handle,cviai_handle_1;

    CVI_NVRLOGD("\n#__aiRFAndODThread  start....");

    //s32Ret = CVI_AI_CreateHandle(&pCtx->cviai_handle);
    s32Ret = CVI_AI_CreateHandle2(&cviai_handle,2,__VPSS_DEV_AI_HANDLE);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_CreateHandle2(&cviai_handle_1,3,__VPSS_DEV_AI_HANDLE);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle1 failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_Service_CreateHandle(&service_handle, &cviai_handle_1);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle1 failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_OpenModel(cviai_handle_1, CVI_AI_SUPPORTED_MODEL_RETINAFACE, FACE_RETINA_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_OpenModel(cviai_handle_1, CVI_AI_SUPPORTED_MODEL_FACEQUALITY, FACE_QUALITY_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_OpenModel(cviai_handle_1, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, FACE_RECOGNITION_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, OBJECT_DETECT_MODEL_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_SetModelThreshold(cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, OBJECT_DETECT_CONFIDENCE);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetModelThreshold failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_SetSkipVpssPreprocess(cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, false);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetSkipVpssPreprocess failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret = CVI_AI_SetSkipVpssPreprocess(cviai_handle_1, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetSkipVpssPreprocess failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret =  CVI_AI_SetVpssTimeout(cviai_handle_1, 1200);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetVpssTimeout failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    s32Ret =  CVI_AI_SetVpssTimeout(cviai_handle, 1200);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetVpssTimeout1 failed!");
    CVI_NVRLOG_ASSERT((s32Ret == CVI_SUCCESS));

    VIDEO_FRAME_INFO_S stVideoFrameInfo[CVIAPP_AIBOX_DEVICE_NUM_MAX];
    memset(&stVideoFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S) * CVIAPP_AIBOX_DEVICE_NUM_MAX);

#ifdef ENABLE_PRINT_FPS
    struct timespec tv1, tv2;
    clock_gettime(CLOCK_MONOTONIC, &tv1);
    int aiFrameCount = 0;
#endif
    printf("\n\n#init feature\n");
    face_recognition_db_count = loadCount(DB_FEATURE_DIR);
    face_recognition_db_feature = loadFeature(DB_FEATURE_DIR, face_recognition_db_count);
    face_recognition_db_name = loadName(DB_FEATURE_DIR, face_recognition_db_count);
    VpssFRInit();     //VPSS For FACERECOGNITION: YUV420 To RGB888

    printf("\n\n#init finish\n");

    CVIBSV_RTSP_CLIENT_STATUS_E eStatus = E_CVIBSV_RTSP_CLIENT_STATUS_IDLE;

    while(pCtx->bEnable)
    {
        for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++)
        {
            eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);

            if(E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT == eStatus)
            {
                if(NULL != pCtx->stParam.pGetFrameFun)
                {
                    if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0)
                    {

                    // face
                    cvai_face_t face;
                    memset(&face, 0, sizeof(cvai_face_t));

                    s32Ret = CVI_AI_RetinaFace(cviai_handle_1, &stVideoFrameInfo[i], &face);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_RetinaFace failed!");

                    //CVI_NVRLOGD("cvi ai chn: %d, face.size = %d", ctxIndex, face.size);

                    if(CVI_SUCCESS == s32Ret && face.size != 0)
                    {

                        CVIAPP_MatchFace_S matchResult;
                        VIDEO_FRAME_INFO_S stRGBFrameInfo;
                        memset(&stRGBFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));
                        memset(&matchResult, 0, sizeof(matchResult));

                        s32Ret = CVI_VPSS_SendFrame(__VPSS_GRP_AI,&stVideoFrameInfo[i], -1);  //YUV420 to RGB888
                        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SendFrame failed!,ch = %d",i);
                        if (s32Ret == CVI_SUCCESS)
                        {
                            s32Ret = CVI_VPSS_GetChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo, 600);
                            CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "stRGBFrameInfo failed!");
                            if (s32Ret == CVI_SUCCESS)
                            {
                                s32Ret = CVI_AI_FaceQuality(cviai_handle_1, &stRGBFrameInfo, &face);
                                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_FaceQuality failed!");

                                //s32Ret = CVI_AI_FaceRecognition(cviai_handle_1, &stRGBFrameInfo, &face);
                                s32Ret = CVI_AI_FaceRecognition(cviai_handle_1, &stRGBFrameInfo, &face);
                                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_FaceRecognition failed!");
                                if (s32Ret == CVI_SUCCESS)
                                {
                                    //int8_t *feature = (int8_t *) calloc(1 * FEATURE_LENGTH, sizeof(int8_t));
                                    int8_t feature[FEATURE_LENGTH] = {0};
                                    memset(feature,0,sizeof(feature));
                                    for (uint32_t  i = 0; i < face.info[0].feature.size; i++)
                                    {
                                        feature[i] = face.info[0].feature.ptr[i];
                                    }
                                    cvai_service_feature_array_t feature_array;
                                    feature_array.data_num = face_recognition_db_count;
                                    feature_array.feature_length = FEATURE_LENGTH;
                                    feature_array.ptr = face_recognition_db_feature;
                                    feature_array.type = TYPE_INT8;

                                    s32Ret = CVI_AI_Service_RegisterFeatureArray(service_handle, feature_array, COS_SIMILARITY);
                                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_RegisterFeatureArray failed!");

                                    int topk = 1;
                                    uint32_t indices[topk];
                                    float scores[topk];
                                    uint32_t score_size;
                                    s32Ret = CVI_AI_Service_RawMatching(service_handle, feature, TYPE_INT8,
                                                            0, 0.41, indices, scores, &score_size);
                                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_RawMatching failed!");

                                    if (score_size > 0)
                                    {
                                        //printf("%d\n",score_size);
                                        char *str = NULL;
                                        matchResult.isMatchFaceFeature = 1;
                                        strcpy(matchResult.matchnName,face_recognition_db_name[indices[0]]);
                                        str = strstr(matchResult.matchnName,".");
                                        for (int i = 0; i < 5; i++)      // File format eg: Kevin.jpeg
                                        {
                                            str[i] = 0;
                                        }
                                        //CVI_NVRLOGD("\n#AI.cpp chn = %d  matchname = %s\n",i,matchResult.matchnName);
                                    }else{
                                        matchResult.isMatchFaceFeature = 0;
                                        memset(matchResult.matchnName,0,sizeof(matchResult.matchnName));
                                    }
                                    //free(feature);
                                    memset(feature,0,sizeof(feature));
                                }
                                s32Ret = CVI_VPSS_ReleaseChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo);
                                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ReleaseChnFrame failed!");
                            }
                        }

                        if(NULL != pCtx->stParam.pNotifyFun)
                        {
                            CVIAPP_AiResult_S result;
                            CVIAPP_AiResultInfo_S resultInfo;

                            memset(&resultInfo.face, 0, sizeof(cvai_face_t));
                            resultInfo.face = face;
                            resultInfo.sMatchResult = matchResult;

                            result.eType = CVIAPP_AI_MODEL_RETINAFACE;
                            //result.pResult = (void*) &face;
                            result.pResult = (void*) &resultInfo;
                            pCtx->stParam.pNotifyFun(i, &result);
                        }
                    }

                    CVI_AI_Free(&face);
#if 0
                    //obj
                    cvai_object_t obj_meta;
                    memset(&obj_meta, 0, sizeof(cvai_object_t));

                    //s32Ret = CVI_AI_MobileDetV2_D0(cviai_handle, &stVideoFrameInfo, &obj_meta, CVI_DET_TYPE_ALL);
                    s32Ret = CVI_AI_MobileDetV2_D0(cviai_handle, &stVideoFrameInfo[i], &obj_meta, CVI_DET_TYPE_PEOPLE);
                    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_MobileDetV2_D0 failed!");

                    if(CVI_SUCCESS == s32Ret && obj_meta.size != 0)
                    {
                        if(NULL != pCtx->stParam.pNotifyFun)
                        {
                            CVIAPP_AiResult_S result;

                            result.eType = CVIAPP_AI_MODEL_OBJECT_DETECTION;
                            result.pResult = (void*) &obj_meta;

                            pCtx->stParam.pNotifyFun(i, &result);
                        }
                    }
                    CVI_AI_Free(&obj_meta);
#endif
                    if(NULL != pCtx->stParam.pReleaseFrameFun)
                    {
                        pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
                        releaseFrameStatus[i] = 1;
                    }

#ifdef ENABLE_PRINT_FPS
                    aiFrameCount++;
                    clock_gettime(CLOCK_MONOTONIC, &tv2);
                    unsigned long int nsec = 0;
                    if(tv2.tv_sec > tv1.tv_sec)
                    {
                        nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
                        // CVI_NVRLOGD("__aiRFAndODThread fps = %d   getframecount = %d", (int)((1 * 1000 * 1000 * 1000) / nsec),count);
                        printf("AI:%d\n",aiFrameCount);
                        aiFrameCount = 0;
                        clock_gettime(CLOCK_MONOTONIC, &tv1);
                    }
#endif

                    }
                    else
                    {
                        releaseFrameStatus[i] = 0;
                    }
                }
            }
        }
    }

    VpssFRDeInit();
    free(face_recognition_db_feature);
    for (int i = 0; i < face_recognition_db_count; i++) {
      free(face_recognition_db_name[i]);
    }
    free(face_recognition_db_name);


    // make sure release
    for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++)
    {
        eStatus = CVIAPP_GetRtspStatus(i, E_CVI_DEV_PROFILE_MAIN);
        if (releaseFrameStatus[i] == 0 && eStatus == E_CVIBSV_RTSP_CLIENT_STATUS_CONNECT){
            while (1){
                if(pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0){
                    pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
                    releaseFrameStatus[i] = 1;
                    break;}
            }
        }
    }


    s32Ret = CVI_AI_Service_DestroyHandle(service_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_DestroyHandle failed!");
    s32Ret = CVI_AI_DestroyHandle(cviai_handle_1);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");
    s32Ret = CVI_AI_DestroyHandle(cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");

    CVI_NVRLOGD("\n#__aiRFAndODThread  end....");

    return NULL;
}

static void *__aiRetinaFaceThread(void *args)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    CVIAPP_AiContext_S *pCtx = (CVIAPP_AiContext_S *) args;

    CVI_NVRLOGD("__aiRetinaFaceThread thread start....");

    s32Ret = CVI_AI_CreateHandle(&pCtx->cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle failed!");

    s32Ret = CVI_AI_OpenModel(pCtx->cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, pCtx->stParam.pModelPath);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_OpenModel(pCtx->cviai_handle, CVI_AI_SUPPORTED_MODEL_FACEQUALITY, FACE_QUALITY_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_OpenModel(pCtx->cviai_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, FACE_RECOGNITION_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_SetSkipVpssPreprocess(pCtx->cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetSkipVpssPreprocess failed!");

    VIDEO_FRAME_INFO_S stVideoFrameInfo;
    memset(&stVideoFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));

#ifdef ENABLE_PRINT_FPS
    struct timespec tv1, tv2;
    clock_gettime(CLOCK_MONOTONIC, &tv1);
#endif

    VpssFRInit();

    while(pCtx->bEnable)
    {
        if(NULL != pCtx->stParam.pGetFrameFun)
        {
            pCtx->stParam.pGetFrameFun(pCtx->u8Chn, &stVideoFrameInfo);

            cvai_face_t face;
            memset(&face, 0, sizeof(cvai_face_t));

            s32Ret = CVI_AI_RetinaFace(pCtx->cviai_handle, &stVideoFrameInfo, &face);
            CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_RetinaFace failed!");

            //CVI_NVRLOGD("cvi ai chn: %d, face.size = %d", ctxIndex, face.size);

            if(CVI_SUCCESS == s32Ret && face.size != 0)
            {

                CVIAPP_MatchFace_S matchResult;
                VIDEO_FRAME_INFO_S stRGBFrameInfo;
                memset(&stRGBFrameInfo, 0, sizeof(VIDEO_FRAME_INFO_S));

                s32Ret = CVI_VPSS_SendFrame(__VPSS_GRP_AI,&stVideoFrameInfo, -1);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SendFrame failed!");
                s32Ret = CVI_VPSS_GetChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo, -1);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "stRGBFrameInfo failed!");

                s32Ret = CVI_AI_FaceQuality(pCtx->cviai_handle, &stRGBFrameInfo, &face);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_FaceQuality failed!");

                s32Ret = CVI_AI_FaceRecognition(pCtx->cviai_handle, &stRGBFrameInfo, &face);
                if (s32Ret) {
                    printf("CVI_AI_FaceAttribute failed, ret:%d\n", s32Ret);
                }

                int8_t *feature = (int8_t *) calloc(1 * FEATURE_LENGTH, sizeof(int8_t));

                for (uint32_t  i = 0; i < face.info[0].feature.size; i++) {
                    feature[i] = face.info[0].feature.ptr[i];
                }

                s32Ret = CVI_VPSS_ReleaseChnFrame(__VPSS_GRP_AI, __VPSS_CHN, &stRGBFrameInfo);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ReleaseChnFrame failed!");

                float *db_f = (float *) calloc(face_recognition_db_count * FEATURE_LENGTH, sizeof(float));
                cvm_gen_db_i8_unit_length(face_recognition_db_feature, db_f, FEATURE_LENGTH, face_recognition_db_count);

                unsigned int *k_index = (unsigned int *) calloc(face_recognition_db_count, sizeof(unsigned int));
                float *k_value = (float *) calloc(face_recognition_db_count, sizeof(float));
                float *buffer = (float *) calloc(face_recognition_db_count * FEATURE_LENGTH, sizeof(float));

                cvm_cpu_i8data_ip_match(feature, face_recognition_db_feature, db_f, k_index, k_value,
                            buffer, FEATURE_LENGTH, face_recognition_db_count, 1);
                if (k_value[0] > 0.41)
                {
                    char *str = NULL;
                    matchResult.isMatchFaceFeature = true;
                    strcpy(matchResult.matchnName,face_recognition_db_name[k_index[0]]);
                    str = strstr(matchResult.matchnName,".");
                    for (int i = 0; i < 5; i++)      // File format eg: Kevin.jpeg
                    {
                        str[i] = 0;
                    }
                }else
                    matchResult.isMatchFaceFeature = false;

                free(feature);
                free(k_index);
                free(k_value);
                free(buffer);
                free(db_f);

                if(NULL != pCtx->stParam.pNotifyFun)
                {
                    CVIAPP_AiResult_S result;
                    CVIAPP_AiResultInfo_S resultInfo;
                    resultInfo.face = face;

                    resultInfo.sMatchResult = matchResult;

                    result.eType = pCtx->stParam.eType;
                    result.pResult = (void*) &resultInfo;
                    pCtx->stParam.pNotifyFun(pCtx->u8Chn, &result);
                }
            }

            if(NULL != pCtx->stParam.pReleaseFrameFun)
            {
                pCtx->stParam.pReleaseFrameFun(pCtx->u8Chn, &stVideoFrameInfo);
            }

            CVI_AI_Free(&face);

#ifdef ENABLE_PRINT_FPS
        clock_gettime(CLOCK_MONOTONIC, &tv2);

        unsigned long int nsec = 0;

        if(tv2.tv_sec > tv1.tv_sec)
        {
            nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
        }
        else
        {
            nsec = tv2.tv_nsec - tv1.tv_nsec;
        }

        CVI_NVRLOGD("__aiRetinaFaceThread fps = %d", (int)((1 * 1000 * 1000 * 1000) / nsec));

        clock_gettime(CLOCK_MONOTONIC, &tv1);
#endif
        }
    }

    VpssFRDeInit();

    s32Ret = CVI_AI_DestroyHandle(pCtx->cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");

    CVI_NVRLOGD("\n#__aiRetinaFaceThread thread end....");

    return NULL;
}
#endif
static void *__aiObjectDetectThread(void *args) {
  cviai_handle_t cviai_handle;
  CVI_S32 s32Ret = CVI_SUCCESS;
  CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *)args;

  CVI_NVRLOGD("__aiObjectDetectThread thread start...");

  CVI_AI_CreateHandle2(&cviai_handle, 3, __VPSS_DEV_AI_HANDLE);
  CVI_AI_SetSkipVpssPreprocess(
      cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, false);
  CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN,
                   PD_MODEL_PATH);

  VIDEO_FRAME_INFO_S stVideoFrameInfo[CVIAPP_AIBOX_DEVICE_NUM_MAX];
  memset(&stVideoFrameInfo, 0,
         sizeof(VIDEO_FRAME_INFO_S) * CVIAPP_AIBOX_DEVICE_NUM_MAX);

#ifdef ENABLE_PRINT_FPS
  struct timespec tv1, tv2;
  clock_gettime(CLOCK_MONOTONIC, &tv1);
#endif

  while (pCtx->bEnable) {
    for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++) {
      if (NULL != pCtx->stParam.pGetFrameFun) {
        if (pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0) {
          cvai_object_t obj_meta;
          memset(&obj_meta, 0, sizeof(cvai_object_t));
          s32Ret = CVI_AI_MobileDetV2_Pedestrian(cviai_handle, &stVideoFrameInfo[i], &obj_meta);

          if (CVI_SUCCESS == s32Ret && obj_meta.size != 0) {
            if (NULL != pCtx->stParam.pNotifyFun) {
              CVIAPP_AiResult_S result;
              result.eType = pCtx->stParam.eType;
              result.pResult = (void *)&obj_meta;
              pCtx->stParam.pNotifyFun(i, &result);
            }
          }

          CVI_AI_Free(&obj_meta);
          if (NULL != pCtx->stParam.pReleaseFrameFun) {
            pCtx->stParam.pReleaseFrameFun(i, &stVideoFrameInfo[i]);
          }

#ifdef ENABLE_PRINT_FPS
          clock_gettime(CLOCK_MONOTONIC, &tv2);
          unsigned long int nsec = 0;
          if (tv2.tv_sec > tv1.tv_sec) {
            nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) -
                   tv1.tv_nsec + tv2.tv_nsec;
          } else {
            nsec = tv2.tv_nsec - tv1.tv_nsec;
          }
          CVI_NVRLOGD("__aiObjectDetectThread fps = %d",
                      (int)((1 * 1000 * 1000 * 1000) / nsec));
          clock_gettime(CLOCK_MONOTONIC, &tv1);
#endif
        }
      }
    }
  }

  CVI_AI_DestroyHandle(cviai_handle);
  CVI_NVRLOGD("__aiObjectDetectThread thread end...");
  return NULL;
}

CVI_ERROR_CODE_E CVIAPP_SetRegionPts(uint8_t id, cvi_nvr_ai_pts_t &pts) {
  CVI_Scope_Mutex_SC __lock(__ctxMutex);
  if (pts.size > CVI_NVR_AI_MAX_PTS_NUM || pts.size == 0) {
    CVI_NVRLOGD("\n#CVIAPP_SetRegionPts Error");
    return E_CVI_ERROR_CODE_FAULT;
  } else {
    memset(&s_cvi_nvr_ai_pts[id], 0, sizeof(cvi_nvr_ai_pts_t));

    s_cvi_nvr_ai_pts[id].size = pts.size;
    memcpy(s_cvi_nvr_ai_pts[id].x, pts.x, pts.size * sizeof(float));
    memcpy(s_cvi_nvr_ai_pts[id].y, pts.y, pts.size * sizeof(float));
    return E_CVI_ERROR_CODE_SUCC;
  }
}

CVI_ERROR_CODE_E CVIAPP_GetRegionPts(uint8_t id, cvi_nvr_ai_pts_t &pts) {
  if (s_cvi_nvr_ai_pts[id].size == 0) {
    return E_CVI_ERROR_CODE_SUCC;
  } else {
    pts.size = s_cvi_nvr_ai_pts[id].size;
    memcpy(pts.x, s_cvi_nvr_ai_pts[id].x, pts.size * sizeof(float));
    memcpy(pts.y, s_cvi_nvr_ai_pts[id].y, pts.size * sizeof(float));
    return E_CVI_ERROR_CODE_SUCC;
  }
}

CVI_ERROR_CODE_E CVIAPP_AiInit(void) {
  for (uint8_t i = 0; i < __MAX_AI_PROCESS; i++) {
    for (uint8_t j = 0; j < CVIAPP_AI_MODEL_MAX; j++) {
      memset(&__aiChnContextList[i][j], 0, sizeof(CVIAPP_AiContext_S));
    }
  }

  memset(&__aiChnContext, 0, sizeof(CVIAPP_AiContext__S));

  for (size_t i = 0; i < CVI_NVR_AI_MAX_REGION_NUM; i++) {
    memset(&s_cvi_nvr_ai_pts[i], 0, sizeof(cvi_nvr_ai_pts_t));
  }

  //_CVIAPP_AiGetFeature(DB_IMAGE_DIR,DB_FEATURE_DIR);

  return E_CVI_ERROR_CODE_SUCC;
}

CVI_ERROR_CODE_E CVIAPP_AiStart(CVIAPP_AiParam_S &param) {
  CVI_Scope_Mutex_SC __lock(__ctxMutex);

  int8_t u8Chn = 0;

  if (u8Chn > __MAX_AI_PROCESS || param.eType > CVIAPP_AI_MODEL_MAX) {
    CVI_NVRLOGE("ERROR: CVIAPP_AiStart out of max size...");
    return E_CVI_ERROR_CODE_FAULT;
  }

  CVIAPP_AiContext__S *pCtx_ = &__aiChnContext;

  if (!pCtx_->bEnable) {
    pCtx_->bEnable = true;
    pCtx_->stParam = param;

    switch (param.eType) {
    case CVIAPP_AI_MODEL_RETINAFACE:
      // pCtx_->pThreadFun = __aiRetinaFaceThread;
      break;
    case CVIAPP_AI_MODEL_OBJECT_DETECTION:
      // pCtx_->pThreadFun = __aiObjectDetectThread;
      break;
    case CVIAPP_AI_RUNONETHREAD:
      // pCtx_->pThreadFun = __aiRFAndODThread;
      break;
    case CVIAPP_AI_FACETRACKING:
      // pCtx_->pThreadFun = __cviFaceTrackingAndCropThread;
      break;
    case CVIAPP_AI_CVINVRTHREAD:
      // pCtx_->pThreadFun = __aiObjectDetectThread;
      // pCtx_->pThreadFun = __cviAiBoxAiThread;
      pCtx_->pThreadFun = __cviFaceCapThread;
      break;
    case CVIAPP_AI_CVINVRREGION:
      // pCtx_->pThreadFun = __cviNvrAiRegionalInvasionThread;
      break;
    default:
      break;
    }

    pCtx_->pThread =
        new CVI_Thread_SC(pCtx_->pThreadFun, (void *)pCtx_, "aiThread",
                          CVI_Thread_SC::E_CVI_THREAD_PRIO_LOW);
    pCtx_->pThread->StartThread();
  }

  return E_CVI_ERROR_CODE_SUCC;
}

CVI_ERROR_CODE_E CVIAPP_AiStop(void) {
  CVI_Scope_Mutex_SC __lock(__ctxMutex);

  CVIAPP_AiContext__S *pCtx_ = &__aiChnContext;
  CVI_NVRLOGD("\n#CVIAPP_AiStop\n");

  if (pCtx_->bEnable) {
    pCtx_->bEnable = false;
    pCtx_->pThread->Join();
    delete pCtx_->pThread;
    pCtx_->pThread = NULL;
  }

  return E_CVI_ERROR_CODE_SUCC;
}

static CVI_ERROR_CODE_E VpssFRDeInit(void) {
  CVI_S32 s32Ret;

  VPSS_GRP VpssGrp = __VPSS_GRP_AI;
  VPSS_CHN VpssChn = __VPSS_CHN;

  s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DisableChn failed!");

  s32Ret = CVI_VPSS_StopGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StopGrp failed!");

  s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DestroyGrp failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

static CVI_ERROR_CODE_E VpssFRInit(void) {
  CVI_S32 s32Ret;

  VPSS_GRP VpssGrp = __VPSS_GRP_AI;
  VPSS_GRP_ATTR_S stVpssGrpAttr;

  memset(&stVpssGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));

  /*  recommend dual mode
      --------------------------------
      *  mode  * vpss dev * vpss chn *
      *-------------------------------
      * single *     0    *     4    *
      *-------------------------------
      *        *     0    *     1    *
      *  dual  *---------------------*
      *        *     1    *     3    *
      *------------------------------*
   */
  s32Ret = CVI_SYS_SetVPSSMode(VPSS_MODE_DUAL);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_SYS_SetVPSSMode failed!");

  /******************************* __VPSS_GRP_AI
   * ********************************/
  stVpssGrpAttr.u32MaxW = 1920;
  stVpssGrpAttr.u32MaxH = 1080;
  stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
  stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
  stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
  stVpssGrpAttr.u8VpssDev = __VPSS_DEV_AI;

  s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_CreateGrp failed!");

  s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ResetGrp failed!");

  VPSS_CHN_ATTR_S stVpssChnAttr;

  memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));

  stVpssChnAttr.u32Width = 1920;
  stVpssChnAttr.u32Height = 1080;

  stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
  stVpssChnAttr.enPixelFormat = PIXEL_FORMAT_RGB_888;
  stVpssChnAttr.stFrameRate.s32SrcFrameRate = 30;
  stVpssChnAttr.stFrameRate.s32DstFrameRate = 30;

  stVpssChnAttr.bMirror = false;
  stVpssChnAttr.bFlip = false;

  stVpssChnAttr.u32Depth = 1; /* ???????????? */

  // stVpssChnAttr.stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
  // stVpssChnAttr.stAspectRatio.bEnableBgColor = true;
  // stVpssChnAttr.stAspectRatio.stVideoRect.s32X = 0;
  // stVpssChnAttr.stAspectRatio.stVideoRect.s32Y = 0;
  // stVpssChnAttr.stAspectRatio.stVideoRect.u32Width  = 480;
  // stVpssChnAttr.stAspectRatio.stVideoRect.u32Height = 270;

  VPSS_CHN VpssChn = __VPSS_CHN;

  s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetChnAttr:%d failed!",
                    VpssChn);

  s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_EnableChn:%d failed!",
                    VpssChn);

  s32Ret = CVI_VPSS_StartGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StartGrp failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

static CVI_ERROR_CODE_E CropVpssDeInit(void) {
  CVI_S32 s32Ret;

  VPSS_GRP VpssGrp = __VPSS_GRP_CROP;
  VPSS_GRP VpssGrp_1 = __VPSS_GRP_CROP_1;

  s32Ret = CVI_VPSS_DisableChn(VpssGrp, 0);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DisableChn failed!");
  s32Ret = CVI_VPSS_DisableChn(VpssGrp_1, 0);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DisableChn failed!");
  // s32Ret = CVI_VPSS_DisableChn(VpssGrp, 1);
  // CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DisableChn failed!");
  // s32Ret = CVI_VPSS_DisableChn(VpssGrp, 2);
  // CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DisableChn failed!");

  s32Ret = CVI_VPSS_StopGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StopGrp failed!");
  s32Ret = CVI_VPSS_StopGrp(VpssGrp_1);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StopGrp failed!");

  s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DestroyGrp failed!");
  s32Ret = CVI_VPSS_DestroyGrp(VpssGrp_1);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_DestroyGrp failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

static CVI_ERROR_CODE_E CropVpssInit(void) {
  CVI_S32 s32Ret;

  VPSS_GRP VpssGrp = __VPSS_GRP_CROP;
  VPSS_GRP VpssGrp_1 = __VPSS_GRP_CROP_1;
  VPSS_GRP_ATTR_S stVpssGrpAttr;

  memset(&stVpssGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));

  /*  recommend dual mode
      --------------------------------
      *  mode  * vpss dev * vpss chn *
      *-------------------------------
      * single *     0    *     4    *
      *-------------------------------
      *        *     0    *     1    *
      *  dual  *---------------------*
      *        *     1    *     3    *
      *------------------------------*
   */
  s32Ret = CVI_SYS_SetVPSSMode(VPSS_MODE_DUAL);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_SYS_SetVPSSMode failed!");

  /******************************* __VPSS_GRP_CROP
   * ********************************/
  stVpssGrpAttr.u32MaxW = 1920;
  stVpssGrpAttr.u32MaxH = 1080;
  stVpssGrpAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
  stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
  stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
  stVpssGrpAttr.u8VpssDev = __VPSS_DEV_CROP;

  s32Ret = CVI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_CreateGrp failed!");
  s32Ret = CVI_VPSS_CreateGrp(VpssGrp_1, &stVpssGrpAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_CreateGrp failed!");

  s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ResetGrp failed!");
  s32Ret = CVI_VPSS_ResetGrp(VpssGrp_1);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_ResetGrp failed!");

  VPSS_CHN_ATTR_S stVpssChnAttr;

  memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));

  stVpssChnAttr.u32Width = 1920;
  stVpssChnAttr.u32Height = 1080;

  stVpssChnAttr.enVideoFormat = VIDEO_FORMAT_LINEAR;
  stVpssChnAttr.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
  stVpssChnAttr.stFrameRate.s32SrcFrameRate = 30;
  stVpssChnAttr.stFrameRate.s32DstFrameRate = 30;

  stVpssChnAttr.bMirror = false;
  stVpssChnAttr.bFlip = false;

  stVpssChnAttr.u32Depth = 1; /* ???????????? */

  VPSS_CHN VpssChn = __VPSS_CHN;

  s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, 0, &stVpssChnAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetChnAttr:%d failed!",
                    VpssChn);
  s32Ret = CVI_VPSS_SetChnAttr(VpssGrp_1, 0, &stVpssChnAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetChnAttr:%d failed!",
                    VpssChn);
  // s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, 1, &stVpssChnAttr);
  // CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetChnAttr:%d
  // failed!", VpssChn); s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, 2,
  // &stVpssChnAttr); CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS),
  // "CVI_VPSS_SetChnAttr:%d failed!", VpssChn);

  s32Ret = CVI_VPSS_EnableChn(VpssGrp, 0);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_EnableChn:%d failed!",
                    VpssChn);
  s32Ret = CVI_VPSS_EnableChn(VpssGrp_1, 0);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_EnableChn:%d failed!",
                    VpssChn);
  // s32Ret = CVI_VPSS_EnableChn(VpssGrp, 1);
  // CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_EnableChn:%d failed!",
  // VpssChn); s32Ret = CVI_VPSS_EnableChn(VpssGrp, 2);
  // CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_EnableChn:%d failed!",
  // VpssChn);

  s32Ret = CVI_VPSS_StartGrp(VpssGrp);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StartGrp failed!");
  s32Ret = CVI_VPSS_StartGrp(VpssGrp_1);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_StartGrp failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

/* ########################
    AI_FACE_LIB
##########################*/

typedef FILE *(*create_logger_file_fn)(const char *, const char *);
typedef void (*print_pass_fn)(FILE *, const char *, int, float, float);
typedef void (*print_fail_fn)(FILE *, const char *, int, float);
typedef void (*close_logger_file_fn)(FILE *);

static void log_reg_pass(FILE *fp, const char *id, int face_count,
                         float face_quality) {
  if (fp) {
    fprintf(fp, "baseid=%-100s facecount=%d quality_score=%f\n", id, face_count,
            face_quality);
  }
}

static void log_reg_fail(FILE *fp, const char *id, int face_count,
                         float face_quality, float quality_threshold) {
  if (fp) {
    fprintf(fp,
            "baseid=%-100s facecount=%d no_face=%-4s quality_score=%f "
            "low_quality=%-4s \n",
            id, face_count, face_count <= 0 ? "YES" : "NO", face_quality,
            face_quality < quality_threshold ? "YES" : "NO");
  }
}

static void log_comp_result(FILE *fp, const char *id, const char *comp_id,
                            float score) {
  if (fp) {
    fprintf(fp, "baseid=%-50s verity_file=%-50s score=%f\n", id, comp_id,
            score);
  }
}

static void removePreviousFile(const char *dir_path) {
  DIR *dirp;
  struct dirent *entry;
  dirp = opendir(dir_path);

  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type != 8 && entry->d_type != 0)
      continue;

    char base_name[500] = "\0";
    strcat(base_name, dir_path);
    strcat(base_name, entry->d_name);
    remove(base_name);
  }
  closedir(dirp);
}

static int loadCount(const char *dir_path) {
  DIR *dirp;
  struct dirent *entry;
  dirp = opendir(dir_path);

  int count = 0;
  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type != 8 && entry->d_type != 0)
      continue;
    count++;
  }
  closedir(dirp);

  return count;
}
#if 0
static int loadDBFeatureCount_(void) {

  cvi_aiDB_ctx_t aiDB_ctx;
  int featureCount = 0;

  int ret = cvi_aiDB_open(&aiDB_ctx, NVR_AI_DB);
  if (ret != E_CVI_ERROR_CODE_SUCC)
  {
        CVI_NVRLOGE("cvi_aiDB_open error! ret= %d",ret);
  }

  featureCount = get_total_feature_count(aiDB_ctx.db);

  cvi_aiDB_close(&aiDB_ctx);

  return featureCount;
}

static char **loadName(const char *dir_path, int count) {
  DIR *dirp;
  struct dirent *entry;
  dirp = opendir(dir_path);

  char **name = (char **) calloc(count, sizeof(char *));
  for (int i = 0; i < count; i++) {
    name[i] = (char *)calloc(NAME_LENGTH, sizeof(char));
  }

  int i = 0;
  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type != 8 && entry->d_type != 0) continue;

    strncpy(name[i], entry->d_name, strlen(entry->d_name) - 4);
    i++;
  }
  closedir(dirp);

  return name;
}

static int8_t *loadFeature(const char *dir_path, int count) {
  DIR *dirp;
  struct dirent *entry;
  dirp = opendir(dir_path);

  int8_t *feature = (int8_t *) calloc(count * FEATURE_LENGTH, sizeof(int8_t));
  int i = 0;
  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type != 8 && entry->d_type != 0) continue;

    char base_name[500] = "\0";
    strcat(base_name, dir_path);
    strcat(base_name, entry->d_name);

    FILE *fp_db;
    if ((fp_db = fopen(base_name, "r")) == NULL) {
      printf("file open error %s!\n", base_name);
      continue;
    }

    int line = 0;
    int idx = 0;
    while (fscanf(fp_db, "%d\n", &line) != EOF) {
      feature[i * FEATURE_LENGTH + idx] = line;
      idx++;
    }

    fclose(fp_db);
    i++;
  }

  closedir(dirp);

  return feature;
}


static int loadDBFeatures(int limit, int offset, int8_t *__feature)
{

struct timespec tv1, tv2;
clock_gettime(CLOCK_MONOTONIC, &tv1);


  cvi_aiDB_ctx_t aiDB_ctx;

  int featureCount = 0;

  int ret = cvi_aiDB_open(&aiDB_ctx, NVR_AI_DB);
  if (ret != E_CVI_ERROR_CODE_SUCC)
  {
        CVI_NVRLOGE("cvi_aiDB_open error! ret= %d",ret);
  }

  featureCount = get_features(aiDB_ctx.db, __feature, limit, offset);

  cvi_aiDB_close(&aiDB_ctx);


clock_gettime(CLOCK_MONOTONIC, &tv2);
{
    unsigned long int nsec = 0;
    if(tv2.tv_sec > tv1.tv_sec)
    {
        nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
    }
    else
    {
        nsec = tv2.tv_nsec - tv1.tv_nsec;
    }

    printf("loadDBFeatures cos_msec = %ld\n", nsec / 1000000);
}

  return 0;
}

int genFeatureFile(const char *img_dir, const char *feature_dir, bool do_face_quality,
                   float quality_thresh, FILE *fp_pass, FILE *fp_fail, cviai_handle_t facelib_handle) {

  CVI_NVRLOGD("genFeatureFile begin...\n");
  DIR *dirp;
  struct dirent *entry;
  dirp = opendir(img_dir);

  if (0 != mkdir(feature_dir, S_IRWXO) && EEXIST != errno) {
    CVI_NVRLOGD("Create %s failed.\n", feature_dir);
    return CVI_FAILURE;
  }
  removePreviousFile(feature_dir);

  while ((entry = readdir(dirp)) != NULL) {
    if (entry->d_type != 8 && entry->d_type != 0) continue;
    //if (entry->d_type != 8) continue;
    char line[2000] = "\0";
    strcat(line, img_dir);
    strcat(line, entry->d_name);

    //printf("%s\n", line);
    VB_BLK blk_fr;
    VIDEO_FRAME_INFO_S rgb_frame;
    CVI_S32 ret = CVI_AI_ReadImage(line, &blk_fr, &rgb_frame, PIXEL_FORMAT_RGB_888);
    if (ret != CVI_SUCCESS) {
      CVI_NVRLOGD("Read image failed with %#x!\n", ret);
      CVI_NVRLOGD("%s\n", line);
      //return ret;
    }

    cvai_face_t face;
    memset(&face, 0, sizeof(cvai_face_t));
    ret = CVI_AI_RetinaFace(facelib_handle, &rgb_frame, &face);
    if (face.size > 0 && do_face_quality == true) {
      ret = CVI_AI_FaceQuality(facelib_handle, &rgb_frame, &face);
      if (ret != CVI_SUCCESS)
      {
          CVI_NVRLOGD("CVI_AI_FaceQuality error ret = %d\n", ret);
      }else
      {
        //   CVI_NVRLOGD("%s",line);
        //   CVI_NVRLOGD("%f",face.info[0].face_quality);
        //   CVI_NVRLOGD("%f",fabs(face.info[0].head_pose.pitch));
        //   CVI_NVRLOGD("%f\n",fabs(face.info[0].head_pose.yaw));
      }
    }

    int face_idx = 0;
    float max_area = 0;
    for (uint32_t i = 0; i < face.size; i++) {
      cvai_bbox_t bbox = face.info[i].bbox;
      float curr_area = (bbox.x2 - bbox.x1) * (bbox.y2 - bbox.y1);
      if (curr_area > max_area) {
        max_area = curr_area;
        face_idx = i;
      }
    }

    char *file_name;
    file_name = strrchr(line, '/');
    file_name++;

    // if (face.size > 0 &&
    //     (do_face_quality == false || face.info[face_idx].face_quality.quality > quality_thresh)) {
    if (face.size > 0 &&
        (do_face_quality == false || face.info[face_idx].face_quality > quality_thresh)) {
      CVI_AI_FaceRecognitionOne(facelib_handle, &rgb_frame, &face, face_idx);

      char base_name[2000] = "\0";
      strcat(base_name, feature_dir);
      strcat(base_name, file_name);
      strcat(base_name, ".txt");

      FILE *fp_feature;
      if ((fp_feature = fopen(base_name, "w+")) == NULL) {
        CVI_NVRLOGD("Write file open error!");
        return CVI_FAILURE;
      }
      for (uint32_t i = 0; i < face.info[face_idx].feature.size; i++) {
        fprintf(fp_feature, "%d\n", (int)face.info[face_idx].feature.ptr[i]);
      }

      log_reg_pass(fp_pass, file_name, face.size, face.info[face_idx].face_quality);
      fclose(fp_feature);
    } else {
      log_reg_fail(fp_fail, file_name, face.size,
                   face.size > 0 ? face.info[face_idx].face_quality : 0, quality_thresh);
    }

    //printf("##genFeatureFile end\n\n");
    CVI_AI_Free(&face);
    CVI_VB_ReleaseBlock(blk_fr);
  }
  closedir(dirp);

  return CVI_SUCCESS;
}

CVI_ERROR_CODE_E _CVIAPP_AiGetFeature(const char *db_img_dir, const char *db_feature_dir)
{
  cviai_handle_t facelib_handle = NULL;
  CVI_S32 ret = CVI_SUCCESS;
  ret = CVI_AI_CreateHandle(&facelib_handle);
  if (ret != CVI_SUCCESS) {
    CVI_NVRLOGD("Create handle failed with %#x!\n", ret);
    return ret;
  }

  ret = CVI_AI_OpenModel(facelib_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, FACE_RETINA_PATH);
  ret |= CVI_AI_OpenModel(facelib_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, FACE_RECOGNITION_PATH);
  ret |= CVI_AI_OpenModel(facelib_handle, CVI_AI_SUPPORTED_MODEL_FACEQUALITY, FACE_QUALITY_PATH);
  if (ret != CVI_SUCCESS) {
    CVI_NVRLOGD("Set model retinaface failed with %#x!\n", ret);
    return ret;
  }
  CVI_AI_SetSkipVpssPreprocess(facelib_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);

  //printf("##CVI_AI_SetSkipVpssPreprocess commplete\n\n");
  char db_dir_full[1000] = "\0";
  strcpy(db_dir_full,db_img_dir);

  char db_feature_full[1000] = "\0";
  strcpy(db_feature_full,db_feature_dir);

  if (0 != mkdir(db_feature_full, S_IRWXO) && EEXIST != errno) {
    CVI_NVRLOGD("Create %s failed.\n", db_feature_full);
    return CVI_FAILURE;
  }

  float quality_thresh = 0.05;
  float sim_thresh = 0.41;
  CVI_NVRLOGD("face quality threshold: %f, face match threshold: %f\n", quality_thresh, sim_thresh);

  if(0x00 == access(db_dir_full, F_OK))
  {
    printf("DB_Image_Dir Exist!\n");

    DIR *dirp = opendir(db_dir_full);
    struct dirent *entry;
    int hasFile = 0;
    while ((entry = readdir(dirp)) != NULL)
    {
        if ((strcmp(".", entry->d_name)==0) || (strcmp("..", entry->d_name)==0))
            continue;
        if (entry->d_type == DT_REG)
        {
          hasFile = 1;
          break;
        }
    }
    closedir(dirp);
    if (0 == hasFile)
      CVI_NVRLOGD("Check DB_Image_Dir file\n");
    else
    {
      CVI_NVRLOGD("Remove old face feature and get feature from new file ...\n");
      FILE *fp_reg_pass = fopen("/tmp/reg_pass.txt", "w+");
      FILE *fp_reg_fail = fopen("/tmp/reg_fail.txt", "w+");
      genFeatureFile(db_dir_full, db_feature_full, true, quality_thresh, fp_reg_pass, fp_reg_fail,facelib_handle);
      fclose(fp_reg_pass);
      fclose(fp_reg_fail);
      CVI_NVRLOGD("Complete\n");
    }
  }
  else
  {
    CVI_NVRLOGD("No DB_Image_Dir\n");
  }

    //CVI_NVRLOGD("Load face feature\n");
    //db_count = loadCount(db_feature_full);
    //db_name = loadName(db_feature_full, db_count);
    //db_feature = loadFeature(db_feature_full, db_count);

    //free(db_feature);
    // for (int i = 0; i < db_count; i++) {
    //   free(db_name[i]);
    // }
    // free(db_name);

  CVI_AI_DestroyHandle(facelib_handle);
  return CVI_SUCCESS;
}

CVI_ERROR_CODE_E _CVIAPP_AiSearchMatchFaceCount(const char *img_dir, int *matchNum, int *faceLibIndexs)
{
    CVI_NVRLOGD("_CVIAPP_AiSearchMatchFace  ....");

    *matchNum = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;
    cviai_service_handle_t service_handle = NULL;
    cviai_handle_t cviai_handle;
    s32Ret = CVI_AI_CreateHandle(&cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle failed!");

    s32Ret = CVI_AI_Service_CreateHandle(&service_handle, &cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_CreateHandle1 failed!");

    s32Ret = CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, FACE_RETINA_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_FACEQUALITY, FACE_QUALITY_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_FACERECOGNITION, FACE_RECOGNITION_PATH);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_OpenModel failed!");

    s32Ret = CVI_AI_SetSkipVpssPreprocess(cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE, false);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetSkipVpssPreprocess failed!");

    s32Ret =  CVI_AI_SetVpssTimeout(cviai_handle, 1200);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_SetVpssTimeout failed!");

    VB_BLK blk_fr;
    VIDEO_FRAME_INFO_S rgb_frame;
    printf("img_dir = %s\n",img_dir);
    __aiImageMutex.Lock();
    s32Ret = CVI_AI_ReadImage(img_dir, &blk_fr, &rgb_frame, PIXEL_FORMAT_RGB_888);
    __aiImageMutex.Unlock();
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_ReadImage failed!");

    cvai_face_t face;
    memset(&face, 0, sizeof(cvai_face_t));
    CVI_AI_RetinaFace(cviai_handle, &rgb_frame, &face);
    if (face.size > 0)
    {
        printf("CVI_AI_RetinaFace succees~~\n");

        s32Ret = CVI_AI_FaceRecognition(cviai_handle, &rgb_frame, &face);
        CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_FaceRecognition failed!");
        if (s32Ret == CVI_SUCCESS)
        {
            int8_t feature[FEATURE_LENGTH] = {0};
            memset(feature,0,sizeof(feature));
            for (uint32_t i = 0; i < face.info[0].feature.size; i++)
            {
                feature[i] = face.info[0].feature.ptr[i];
            }

            search_by_image_db_count = loadDBFeatureCount_();

            int feature_array_Max = 8000;
            int cycleIndex = search_by_image_db_count / feature_array_Max ;
            int count_ = search_by_image_db_count % feature_array_Max;
            int limit = 0;
            int offset = feature_array_Max;

            if (count_ != 0)
                cycleIndex++;

            int matchIndex  = 0;

            struct timespec tv1, tv2;
            clock_gettime(CLOCK_MONOTONIC, &tv1);

#if 1
            for (int i = 0; i < cycleIndex; i++)
            {
                if (((i+1) == cycleIndex) && (count_ != 0))
                {
                    limit = count_;
                }else
                {
                    limit = feature_array_Max;
                }

                search_by_image_db_feature = (int8_t *) calloc(limit * FEATURE_LENGTH, sizeof(int8_t));
                loadDBFeatures(limit, offset * i, search_by_image_db_feature);

                cvai_service_feature_array_t feature_array;
                feature_array.data_num = limit;
                feature_array.feature_length = FEATURE_LENGTH;
                feature_array.ptr = search_by_image_db_feature;
                feature_array.type = TYPE_INT8;

                s32Ret = CVI_AI_Service_RegisterFeatureArray(service_handle, feature_array, COS_SIMILARITY);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_RegisterFeatureArray failed!");

                uint32_t indices[feature_array_Max];
                memset(indices, 0, sizeof(indices));
                float scores[feature_array_Max];
                memset(scores, 0, sizeof(scores));
                uint32_t score_size;
                float threshold = 0.41;

                //s32Ret = CVI_AI_Service_FaceInfoMatching(service_handle, face.info, 0, threshold, indices, scores, &score_size);

                struct timespec tv1_1, tv2_1;
                clock_gettime(CLOCK_MONOTONIC, &tv1_1);

                s32Ret = CVI_AI_Service_RawMatching(service_handle, feature, TYPE_INT8,
                                        0, threshold, indices, scores, &score_size);
                CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_Service_RawMatching failed!");

                clock_gettime(CLOCK_MONOTONIC, &tv2_1);
                {
                    unsigned long int nsec = 0;
                    if(tv2_1.tv_sec > tv1_1.tv_sec)
                    {
                        nsec = (tv2_1.tv_sec - tv1_1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1_1.tv_nsec + tv2_1.tv_nsec;
                    }
                    else
                    {
                        nsec = tv2_1.tv_nsec - tv1_1.tv_nsec;
                    }

                    printf("CVI_AI_Service_RawMatching cos_msec = %ld\n", nsec / 1000000);
                }

                for (uint32_t j = 0; j < score_size; j++)
                {
                    if (matchIndex > (100 * 1024))
                    {
                        printf("Out of mem!!\n");
                    }else
                    {
                        faceLibIndexs[matchIndex++] = (i *feature_array_Max) + indices[j] + 1; // id
                    }
                }

                *matchNum += score_size;

                free(search_by_image_db_feature);
            }
#endif
            printf("Match_num = %d\n",*matchNum);

            clock_gettime(CLOCK_MONOTONIC, &tv2);
            {
                unsigned long int nsec = 0;
                if(tv2.tv_sec > tv1.tv_sec)
                {
                    nsec = (tv2.tv_sec - tv1.tv_sec) * (1 * 1000 * 1000 * 1000) - tv1.tv_nsec + tv2.tv_nsec;
                }
                else
                {
                    nsec = tv2.tv_nsec - tv1.tv_nsec;
                }

                printf("cos_msec = %ld\n", nsec / 1000000);
            }

        }
    }
    CVI_AI_Free(&face);
    CVI_VB_ReleaseBlock(blk_fr);
    CVI_AI_Service_DestroyHandle(service_handle);
    s32Ret = CVI_AI_DestroyHandle(cviai_handle);
    CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_AI_DestroyHandle failed!");


    return CVI_SUCCESS;
}
#endif

static CVI_ERROR_CODE_E CropJpegEncDeInit() {
  CVI_S32 s32Ret = CVI_FAILURE;

  VENC_CHN VencChn = 6;

  s32Ret = CVI_VENC_StopRecvFrame(VencChn);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_StopRecvFrame failed!");

  CVI_VENC_DestroyChn(VencChn);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_DestroyChn failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

static CVI_ERROR_CODE_E CropJpegEncInit(int w, int h) {

  CVI_S32 s32Ret = CVI_FAILURE;

  VENC_CHN VencChn = 6;

  VENC_CHN_ATTR_S stVencChnAttr;

  VENC_RC_PARAM_S stRcParam;
  VENC_CHN_PARAM_S stChnParam;

  VENC_RECV_PIC_PARAM_S stRecvParam;

  memset(&stVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));

  memset(&stRcParam, 0, sizeof(VENC_RC_PARAM_S));
  memset(&stChnParam, 0, sizeof(VENC_CHN_PARAM_S));

  memset(&stRecvParam, 0, sizeof(VENC_RECV_PIC_PARAM_S));

  stVencChnAttr.stVencAttr.enType = PT_JPEG;

  stVencChnAttr.stVencAttr.u32MaxPicWidth = w;
  stVencChnAttr.stVencAttr.u32MaxPicHeight = h;
  stVencChnAttr.stVencAttr.u32PicWidth = w;
  stVencChnAttr.stVencAttr.u32PicHeight = h;

  // stVencChnAttr.stVencAttr.u32BufSize      = w * h * 2;
  stVencChnAttr.stVencAttr.u32BufSize = 1920 * 1080;

  stVencChnAttr.stVencAttr.u32Profile = 0;

  stVencChnAttr.stVencAttr.bByFrame =
      CVI_TRUE; /* get stream mode is slice mode or frame mode? */

  VENC_ATTR_JPEG_S *pstJpegAttr = &stVencChnAttr.stVencAttr.stAttrJpege;

  pstJpegAttr->bSupportDCF = CVI_FALSE;
  pstJpegAttr->stMPFCfg.u8LargeThumbNailNum = 0;
  pstJpegAttr->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;

  stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
  stVencChnAttr.stGopAttr.stNormalP.s32IPQpDelta = 0;

  stRecvParam.s32RecvPicNum = -1;

  s32Ret = CVI_VENC_CreateChn(VencChn, &stVencChnAttr);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_CreateChn failed!");

  s32Ret = CVI_VENC_GetRcParam(VencChn, &stRcParam);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_GetRcParam failed!");

  stRcParam.s32FirstFrameStartQp = 32;

  s32Ret = CVI_VENC_SetRcParam(VencChn, &stRcParam);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_SetRcParam failed!");

  s32Ret = CVI_VENC_GetChnParam(VencChn, &stChnParam);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_GetChnParam failed!");

  stChnParam.stCropCfg.bEnable = CVI_FALSE;

  s32Ret = CVI_VENC_SetChnParam(VencChn, &stChnParam);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_SetChnParam failed!");

  s32Ret = CVI_VENC_StartRecvFrame(VencChn, &stRecvParam);
  CVI_NVRLOGE_CHECK((s32Ret != CVI_SUCCESS), "CVI_VENC_StartRecvFrame failed!");

  return E_CVI_ERROR_CODE_SUCC;
}

static void UpdateVpssCropGrpVdecParam(cvi_uint8 groupID, cvi_uint8 chnID,
                                       int w, int h) {
  CVI_S32 s32Ret = CVI_SUCCESS;

  VPSS_GRP VpssGrp = groupID;
  static VPSS_GRP_ATTR_S stVpssGrpAttr;
  static VPSS_CHN_ATTR_S stChnAttr;

  memset(&stVpssGrpAttr, 0, sizeof(VPSS_GRP_ATTR_S));
  memset(&stChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));

  s32Ret = CVI_VPSS_GetGrpAttr(__VPSS_GRP_CROP, &stVpssGrpAttr);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_GetGrpAttr failed!");

  s32Ret = CVI_VPSS_SetGrpAttr(__VPSS_GRP_CROP, &stVpssGrpAttr);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetGrpAttr failed!");

  s32Ret = CVI_VPSS_GetChnAttr(__VPSS_GRP_CROP, chnID, &stChnAttr);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_GetChnAttr failed!");

  // stChnAttr.u32Width = w;
  // stChnAttr.u32Height = h;
  stChnAttr.u32Width = 102;
  stChnAttr.u32Height = 122;

  s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, chnID, &stChnAttr);
  CVI_NVRLOGW_CHECK((s32Ret != CVI_SUCCESS), "CVI_VPSS_SetChnAttr failed!");
}
