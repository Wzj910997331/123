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
static CVIAPP_AiContext__S __aiChnContextBuf[3];

// RegionalInvasion
static cvi_nvr_ai_pts_t s_cvi_nvr_ai_pts[CVI_NVR_AI_MAX_REGION_NUM];

static CVI_ERROR_CODE_E CropJpegEncInit(int w, int h);
static CVI_ERROR_CODE_E CropJpegEncDeInit();
static void UpdateVpssCropGrpVdecParam(cvi_uint8 groupID, cvi_uint8 chnID,
                                       int w, int h);
static int loadCount(const char *dir_path);
static char **loadName(const char *dir_path, int count);
static int8_t *loadFeature(const char *dir_path, int count);

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


/*cvi_nvr Face Detection*/
static void *__FaceDetectThread(void *args) {
  CVI_S32 s32Ret = CVI_SUCCESS;
  CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *)args;

  cviai_handle_t cviai_handle;
  CVI_NVRLOGD("__FaceDetectThread  start....");
  CVI_AI_CreateHandle2(&cviai_handle, 3, __VPSS_DEV_AI_HANDLE);
  CVI_AI_OpenModel(cviai_handle, CVI_AI_SUPPORTED_MODEL_RETINAFACE,
                               pCtx->stParam.pModelPath);
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

  CVI_NVRLOGD("__FaceDetectThread  pCtx->bEnable=%d", pCtx->bEnable);
  while (pCtx->bEnable) {
    for (int i = 0; i < CVIAPP_AIBOX_DEVICE_NUM_MAX; i++) {
      if (NULL != pCtx->stParam.pGetFrameFun) {
        if (pCtx->stParam.pGetFrameFun(i, &stVideoFrameInfo[i]) == 0) {
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
              result.eType = pCtx->stParam.eType;
              result.pResult = (void *)&resultInfo;
              pCtx->stParam.pNotifyFun(i, &result);
            } else {
              CVI_NVRLOGD("__FaceDetectThread pNotifyFun is NULL");
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
  CVI_NVRLOGD("\n#__FaceDetectThread  end....");

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
      // printf("I/O Buffer is empty.\n");
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
  CVI_NVRLOGD("__cviFaceCapThread  start.... %s", FACE_RETINA_PATH);

  CVI_AI_CreateHandle2(&ai_handle, 5, __VPSS_DEV_AI_HANDLE);
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

  CVI_NVRLOGD("__cviFaceCapThread  pCtx->bEnable=%d", pCtx->bEnable);

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
              if (state != MISS) {
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

  CVI_NVRLOGD("\n#__cviFaceCapThread  end....");
  return NULL;
}

static void *__aiObjectDetectThread(void *args) {
  cviai_handle_t cviai_handle;
  CVI_S32 s32Ret = CVI_SUCCESS;
  CVIAPP_AiContext__S *pCtx = (CVIAPP_AiContext__S *)args;
  CVI_NVRLOGD("__aiObjectDetectThread thread start...");

  CVI_AI_CreateHandle2(&cviai_handle, 4, __VPSS_DEV_AI_HANDLE);
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
  memset(&__aiChnContextBuf, 0, sizeof(CVIAPP_AiContext__S)*3);

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

  
  CVIAPP_AiContext__S *pCtx_ = &__aiChnContextBuf[param.eType];

  if (!pCtx_->bEnable) {
    pCtx_->bEnable = true;
    pCtx_->stParam = param;

    switch (param.eType) {
    case CVIAPP_AI_FACE_CAPTURE:
      pCtx_->pThreadFun = __cviFaceCapThread;
      break;
    case CVIAPP_AI_OBJECT_DETECTION:
      pCtx_->pThreadFun = __aiObjectDetectThread;
      break;
    case CVIAPP_AI_FACE_DETECION:
      pCtx_->pThreadFun = __FaceDetectThread;
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

