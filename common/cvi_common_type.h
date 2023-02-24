/**
 * File Name: cvi_common_type.h
 *
 * Version: V1.0
 *
 * Brief: common type definitions for platform compatabilty.
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
#ifndef _CVI_COMMON_TYPE_H_
#define _CVI_COMMON_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>

typedef struct timeval cvi_timeval;
typedef time_t cvi_time;
typedef char cvi_int8;
typedef unsigned char cvi_uint8;
typedef short int cvi_int16;
typedef unsigned short int cvi_uint16;
typedef int cvi_int32;
typedef unsigned int cvi_uint32;
typedef float cvi_float;
typedef double cvi_double;

#if __WORDSIZE == 64
typedef long int cvi_int64;
typedef unsigned long int cvi_uint64;
#else
typedef long long int cvi_int64;
typedef unsigned long long int cvi_uint64;
#endif

typedef cvi_int8 cvi_bool;

typedef cvi_uint32 CVI_MSGTYPE_E;
typedef cvi_uint32 CVI_OPER_PERM_E;

// Warnings:
// in 64bit platform,the pointer is 8bytes size.
// size_t and ssize_t is 8 bytes too.

#ifdef __cplusplus
}
#endif

#endif //_CVI_COMMON_TYPE_H_
