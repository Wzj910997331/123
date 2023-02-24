/**
 * File Name: cvi_const_sysconf.h
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
#ifndef _CVI_CONST_SYSCONF_H_
#define _CVI_CONST_SYSCONF_H_

#ifdef __cplusplus
extern "C" {
#endif
///////////////////////////////////////////////////////////////////////////
///   SYSTEM DEFINE
///////////////////////////////////////////////////////////////////////////
#define CVI_VIEWPORT_NUM 16

#define CVI_LOCALIOOUTPUT_NUM 4
#define CVI_LOCALIOINPUT_NUM 4

#define CVI_REMOTEIOOUTPUT_NUM_MAX 8
#define CVI_REMOTEIOINPUT_NUM_MAX 8

#define CVI_CHANNEL_NUM 8

#define CVI_PHYDISK_NUM_MAX 4
#define CVI_LOGDISK_NUM 16

#define CVI_NVR_DEVNAME_LEN_MAX 32

#define CVI_UART_PORT_NUM 1

#define CVI_CHANNEL_NAME_LEN_MAX 16

#define CVI_DEV_PROFILE_NUM 2

#define CVIBSV_USE_SENDCMD_REPLACE_POSTCMD                                     \
  1 // use send command to archive sync call functions

#ifdef __cplusplus
}
#endif
#endif //_CVI_CONST_SYSCONF_H_