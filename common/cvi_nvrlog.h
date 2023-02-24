/**
 * File Name: cvi_nvrlog.h
 *
 * Version: V1.0
 *
 * Brief: log definition.
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *
 * History			:
 *
 =====================================================================================
   1.Date 			:	2020/07/16
   Author 			:	garry.xin
   Modification		:	Created file
 *
 ====================================================================================*/
#ifndef _CVI_NVRLOG_H_
#define _CVI_NVRLOG_H_

#if defined(CVI_NVRLOG_LEVEL)
#define ZF_LOG_LEVEL CVI_NVRLOG_LEVEL
#endif
#include "zf_log.h"

#define CVI_NVRLOG_VERBOSE ZF_LOG_VERBOSE // 1
#define CVI_NVRLOG_DEBUG ZF_LOG_DEBUG     // 2
#define CVI_NVRLOG_INFO ZF_LOG_INFO       // 3
#define CVI_NVRLOG_WARN ZF_LOG_WARN       // 4
#define CVI_NVRLOG_ERROR ZF_LOG_ERROR     // 5
#define CVI_NVRLOG_FATAL ZF_LOG_FATAL     // 6
#define CVI_NVRLOG_NONE ZF_LOG_NONE       // 0xFF

/* Message logging macros:
 * - CVI_LOGV("format string", args, ...)
 * - CVI_LOGD("format string", args, ...)
 * - CVI_LOGI("format string", args, ...)
 * - CVI_LOGW("format string", args, ...)
 * - CVI_LOGE("format string", args, ...)
 * - CVI_LOGF("format string", args, ...)
 */
#define CVI_NVRLOGV(...) ZF_LOGV(__VA_ARGS__)
#define CVI_NVRLOGD(...) ZF_LOGD(__VA_ARGS__)
#define CVI_NVRLOGI(...) ZF_LOGI(__VA_ARGS__)
#define CVI_NVRLOGW(...) ZF_LOGW(__VA_ARGS__)
#define CVI_NVRLOGE(...) ZF_LOGE(__VA_ARGS__)
#define CVI_NVRLOGF(...) ZF_LOGF(__VA_ARGS__)

/*
 * Memory logging macros:
 * - CVI_LOGV_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGD_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGI_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGW_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGE_MEM(data_ptr, data_sz, "format string", args, ...)
 * - CVI_LOGF_MEM(data_ptr, data_sz, "format string", args, ...)
 */
#define CVI_NVRLOGV_MEM(...) ZF_LOGV_MEM(__VA_ARGS__)
#define CVI_NVRLOGD_MEM(...) ZF_LOGD_MEM(__VA_ARGS__)
#define CVI_NVRLOGI_MEM(...) ZF_LOGI_MEM(__VA_ARGS__)
#define CVI_NVRLOGW_MEM(...) ZF_LOGW_MEM(__VA_ARGS__)
#define CVI_NVRLOGE_MEM(...) ZF_LOGE_MEM(__VA_ARGS__)
#define CVI_NVRLOGF_MEM(...) ZF_LOGF_MEM(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * when called, setup log according to env variables
 * otherwise, logging to stdout, and start with CVI_NVRLOG_LEVEL by default
 *
 * Supported env variables are
 *   - CVI_NVRLOG_SYSLOG : true|false
 *   - CVI_NVRLOG_FILE   : filename
 *   - CVI_NVRLOG_LEVEL  : F|E|W|I|D|V|N
 */
#define CVI_NVRLOG_LEVEL_VAR_NAME "CVI_NVRLOG_LEVEL"
#define CVI_NVRLOG_FILE_VAR_NAME "CVI_NVRLOG_FILE"
#define CVI_NVRLOG_SYSLOG_VAR_NAME "CVI_NVRLOG_SYSLOG"
void CVI_NVRLOG_INIT(void);

/*
 * to set the logging output level
 * by default, CVI_LOG_INFO is used
 */
void CVI_NVRLOG_SET_LEVEL(const int lvl);

/*
 * (Optional) to set a TAG
 */
void CVI_NVRLOG_SET_TAG(const char *const tag);

#ifdef __cplusplus
}
#endif
/*
#ifndef CVI_NVRLOG_ASSERT_STR
#define CVI_NVRLOG_ASSERT_STR(x, ...)     \
    do {                           \
        if (!(x)) {                \
            CVI_NVRLOGE(__VA_ARGS__); \
                        abort();               \
        }                          \
    } while(0)
#endif
#ifndef CVI_NVRLOG_ASSERT
#define CVI_NVRLOG_ASSERT(x)     \
    do {                           \
        if (!(x)) {                \
                        abort();               \
        }                          \
    } while(0)
#endif
*/

#ifndef CVI_NVRLOG_ASSERT
#define CVI_NVRLOG_ASSERT(x)                                                   \
  do {                                                                         \
    if (!(x)) {                                                                \
      CVI_NVRLOGE("%s", __FILE__);                                             \
      abort();                                                                 \
    }                                                                          \
  } while (0)
#endif

#ifndef UNUSED
#define UNUSED(x) x = x
#endif

#define CVI_NVRLOGE_CHECK(x, fmt, arg...)                                      \
  do {                                                                         \
    if (x) {                                                                   \
      CVI_NVRLOGE(fmt, ##arg);                                                 \
      exit(-1);                                                                \
    }                                                                          \
  \
} while (0)

#define CVI_NVRLOGW_CHECK(x, fmt, arg...)                                      \
  do {                                                                         \
    if (x) {                                                                   \
      CVI_NVRLOGW(fmt, ##arg);                                                 \
    }                                                                          \
  \
} while (0)

#endif //_CVI_NVRLOG_H_