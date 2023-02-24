/**
 * File Name: cvi_const_event.h
 *
 * Version: V1.0
 *
 * Brief: const of event definition.
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
#ifndef _CVI_CONST_EVENT_H_
#define _CVI_CONST_EVENT_H_

#include "cvi_common_type.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
///   EVENT TYPE
///////////////////////////////////////////////////////////////////////////
typedef cvi_uint32 CVI_EVENT_TYPE_E;

///////////////////////////////////////////////////////////////////////////
/// SYSFAULT EVENT
///////////////////////////////////////////////////////////////////////////
#define E_CVI_EVENT_TYPE_SYSFAULT_BASE (20000)
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE (E_CVI_EVENT_TYPE_SYSFAULT_BASE)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE                                 \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE + 2000)

// SYSFAULT-DISK
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_NONE                                    \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE)
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_ERROR                                   \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE + 1)
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_FULL                                    \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE + 2)
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_LAST                                    \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE + 3)
#define E_CVI_EVENT_TYPE_SYSFAULT_DISK_NUM                                     \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_LAST - E_CVI_EVENT_TYPE_SYSFAULT_DISK_BASE + \
   1)

// SYSFAULT-NETWORK
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_DISCONN                              \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_IPCONFLIC                            \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE + 1)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_MACCONFLIC                           \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE + 2)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_INVALIDACCESS                        \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE + 3)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_LAST                                 \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE + 4)
#define E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_NUM                                  \
  (E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_LAST -                                    \
   E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_BASE + 1)

#define E_CVI_EVENT_TYPE_SYSFAULT_NUM                                          \
  (E_CVI_EVENT_TYPE_SYSFAULT_DISK_NUM + E_CVI_EVENT_TYPE_SYSFAULT_NETWORK_NUM)

///////////////////////////////////////////////////////////////////////////
/// ALARM EVENT
///////////////////////////////////////////////////////////////////////////
#define E_CVI_EVENT_TYPE_ALARM_BASE (30000)

#define E_CVI_EVENT_TYPE_ALARM_IOINPUT (E_CVI_EVENT_TYPE_ALARM_BASE)
#define E_CVI_EVENT_TYPE_ALARM_MDET (E_CVI_EVENT_TYPE_ALARM_BASE + 1)
#define E_CVI_EVENT_TYPE_ALARM_TAMPER (E_CVI_EVENT_TYPE_ALARM_BASE + 2)
#define E_CVI_EVENT_TYPE_ALARM_VIDEOLOSS (E_CVI_EVENT_TYPE_ALARM_BASE + 3)
#define E_CVI_EVENT_TYPE_ALARM_SCENECHG (E_CVI_EVENT_TYPE_ALARM_BASE + 4)
#define E_CVI_EVENT_TYPE_ALARM_REC_FAULT (E_CVI_EVENT_TYPE_ALARM_BASE + 5)
#define E_CVI_EVENT_TYPE_ALARM_IPCHN_CONFLIC (E_CVI_EVENT_TYPE_ALARM_BASE + 6)
// SUBSTREAM'S BITRATE OVERFLOW
#define E_CVI_EVENT_TYPE_ALARM_SUBSTRM_BROVERFLOW                              \
  (E_CVI_EVENT_TYPE_ALARM_BASE + 7)
#define E_CVI_EVENT_TYPE_ALARM_LAST (E_CVI_EVENT_TYPE_ALARM_BASE + 7)

// OTHER SMART DETECT ALARM
#define E_CVI_EVENT_TYPE_ALARM_NUM                                             \
  (E_CVI_EVENT_TYPE_ALARM_LAST - E_CVI_EVENT_TYPE_ALARM_BASE + 1)

#ifdef __cplusplus
}
#endif
#endif //_CVI_CONST_EVENT_H_