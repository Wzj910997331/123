/**
 * File Name: cvi_const.h
 *
 * Version: V1.0
 *
 * Brief: const definition.
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
#ifndef _CVI_CONST_H_
#define _CVI_CONST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cvi_const_error.h"
#include "cvi_const_event.h"
#include "cvi_const_sysconf.h"
#include "cvi_const_syslog.h"

#define CVI_PTZ_PRESET_NO_NULL 0
#define CVI_PTZ_TOUR_NO_NULL 0
#define CVI_PTZ_TRACK_NO_NULL 0

#define cvi_true 1
#define cvi_false 0

#define CVI_TRUE 1
#define CVI_FALSE 0

#define CVI_U64TIME_MIN 0
#define CVI_U64TIME_MAX (0xFFFFFFFF)

typedef enum {
  E_CVI_FILE_SEEKSET_TYPE_FIRST = 0,
  E_CVI_FILE_SEEKSET_TYPE_START = E_CVI_FILE_SEEKSET_TYPE_FIRST,
  E_CVI_FILE_SEEKSET_TYPE_CURR,
  E_CVI_FILE_SEEKSET_TYPE_END,
  E_CVI_FILE_SEEKSET_TYPE_NUM
} CVI_FILE_SEEKSET_TYPE_E;

typedef enum {
  E_CVIBSV_STORAGE_TYPE_FIRST = 0,
  E_CVIBSV_STORAGE_TYPE_SATA = E_CVIBSV_STORAGE_TYPE_FIRST,
  E_CVIBSV_STORAGE_TYPE_SD,
  E_CVIBSV_STORAGE_TYPE_USB,
  E_CVIBSV_STORAGE_TYPE_ESATA,
  E_CVIBSV_STORAGE_TYPE_NAS,
  E_CVIBSV_STORAGE_TYPE_SAN,
  E_CVIBSV_STORAGE_TYPE_NUM
} CVIBSV_STORAGE_TYPE_E;

typedef enum {
  E_CVI_MSGCLASS_FIRST = 0,
  E_CVI_MSGCLASS_NULL = E_CVI_MSGCLASS_FIRST,
  E_CVI_MSGCLASS_COMMAND,
  E_CVI_MSGCLASS_RESPONSE,
  E_CVI_MSGCLASS_EVENT,
  E_CVI_MSGCLASS_NUM,
  E_CVI_MSGCLASS_DEFAULT = E_CVI_MSGCLASS_NULL
} CVI_MSGCLASS_E;

#define E_CVI_MSGTYPE_DEFAULT 0

#ifdef __cplusplus
}
#endif

#endif //_CVI_CONST_H_
