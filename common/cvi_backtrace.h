/**
 * File Name: cvi_backtrace.h
 *
 * Version: V1.0
 *
 * Brief: 
 *
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description: 
 *
 * History			:
 * =====================================================================================
   1.Date 			:	2020/10/28
   Author 			:	mason.zou
   Modification		:	Created file
 * ====================================================================================*/

#ifndef CVI_BACKTRACE_H_
#define CVI_BACKTRACE_H_

#include "cvi_nvrlog.h"

#ifdef __cplusplus
	extern "C" {
#endif

void RegisterBacktraceHandler(void);

#ifdef __cplusplus
	}
#endif

#endif

