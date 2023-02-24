/**
 * File Name: cvi_const_error.h
 *
 * Version: V1.0
 *
 * Brief: error code definition.
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
#ifndef _CVI_CONST_ERROR_H_
#define _CVI_CONST_ERROR_H_
#include "cvi_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
///   ERROR CODE
///////////////////////////////////////////////////////////////////////////
typedef cvi_uint32 CVI_ERROR_CODE_E;

#define E_CVI_ERROR_CODE_SUCC (0)
#define E_CVI_ERROR_CODE_FAULT (1)
#define E_CVI_ERROR_CODE_FAULT_SVCNOTINIT (2)

#define CVI_ERROR_CODE_MODULE_QUEUE_BASE (1000)

// functional thread
#define CVI_ERROR_CODE_MODULE_SIGNAL_BASE (20000)
#define CVI_ERROR_CODE_MODULE_MEDIA_BASE (22000)
#define CVI_ERROR_CODE_MODULE_THREADTIMER_BASE (24000)
#define CVI_ERROR_CODE_MODULE_RECORD_BASE (26000)
#define CVI_ERROR_CODE_MODULE_DEVPROBE_BASE (28000)
#define CVI_ERROR_CODE_MODULE_SVCCMD_BASE (30000)
#define CVI_ERROR_CODE_MODULE_SVCEVENT_BASE (32000)

// system config
#define CVI_ERROR_CODE_MODULE_HOLIDAY_BASE (50000)
#define CVI_ERROR_CODE_MODULE_CHNPREVIEW_BASE (52000)
#define CVI_ERROR_CODE_MODULE_CHNSTORAGE_BASE (54000)
#define CVI_ERROR_CODE_MODULE_TIMESCHED_BASE (56000)
#define CVI_ERROR_CODE_MODULE_IO_BASE (58000)
#define CVI_ERROR_CODE_MODULE_EVENTTYPE_BASE (60000)
#define CVI_ERROR_CODE_MODULE_SYSFAULT_BASE (62000)
#define CVI_ERROR_CODE_MODULE_NWCONFIG_BASE (64000)
#define CVI_ERROR_CODE_MODULE_SYSCONFIG_BASE (66000)
#define CVI_ERROR_CODE_MODULE_VIEWSPLIT_BASE (68000)
#define CVI_ERROR_CODE_MODULE_USER_BASE (70000)
#define CVI_ERROR_CODE_MODULE_DISK_BASE (72000)
#define CVI_ERROR_CODE_MODULE_BUFFER_BASE (74000)
#define CVI_ERROR_CODE_MODULE_BUFFER_SIZE_ERROR (74001)
#define CVI_ERROR_CODE_MODULE_BUFFER_INVALID (74002)
#define CVI_ERROR_CODE_MODULE_CHNINFO_BASE (76000)

// log&db
#define CVI_ERROR_CODE_MODULE_SYSLOG_BASE (100000)
#define CVI_ERROR_CODE_MODULE_RECSEG_BASE (102000)
#define CVI_ERROR_CODE_MODULE_RECTAG_BASE (104000)

// base function module
#define CVI_ERROR_CODE_MODULE_AUTHEN_BASE (150000)
#define CVI_ERROR_CODE_MODULE_APNOTIFY_BASE (152000)

// DevSigProto
#define CVI_ERROR_CODE_MODULE_DEVSIGPROTO_BASE (200000)

// DB option api
#define CVI_ERROR_CODE_NVRDB_BASE (200100)
#define CVI_ERROR_CODE_NVRDB_GEN_ERR (200101)
#define CVI_ERROR_CODE_NVRDB_PARAM_ERR (200102)
#define CVI_ERROR_CODE_NVRDB_OPEN_FAIL (200103)
#define CVI_ERROR_CODE_NVRDB_SQL_FAIL (200104)
#define CVI_ERROR_CODE_NVRDB_NOT_FOUND (200105)
#define CVI_ERROR_CODE_NVRDB_NO_ADPT (200106)

#ifdef __cplusplus
}
#endif
#endif //_CVI_CONST_ERROR_H_
