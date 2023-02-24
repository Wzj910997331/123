/**
 * File Name: cvi_utility.h
 *
 * Version: V1.0
 *
 * Brief: utility apis.
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
#ifndef _CVI_UTILITY_H_
#define _CVI_UTILITY_H_
#include "cvi_base_type.h"
#include "cvi_common_type.h"
#include "cvi_const.h"
#include "cvi_nvrlog.h"
#include <cstdlib>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

class CVI_Utility_C {
public:
  static cvi_bool CheckHour(cvi_uint8 u8SHour) {
    if ((u8SHour >= 0) && (u8SHour <= 24)) {
      return CVI_TRUE;
    } else {
      return CVI_FALSE;
    }
  }
  static cvi_bool CheckMin(cvi_uint8 u8SMin) {
    if ((u8SMin >= 0) && (u8SMin <= 60)) {
      return CVI_TRUE;
    } else {
      return CVI_FALSE;
    }
  }
  static cvi_bool CheckSec(cvi_uint8 u8SSec) {
    if ((u8SSec >= 0) && (u8SSec <= 60)) {
      return CVI_TRUE;
    } else {
      return CVI_FALSE;
    }
  }
  static string IPv4ToStr(const cvi_uint8 *pu8IPV4) {
    char IPv4_str[16] = {0};
    sprintf(IPv4_str, "%d.%d.%d.%d", pu8IPV4[0], pu8IPV4[1], pu8IPV4[2],
            pu8IPV4[3]);
    return (string)IPv4_str;
  }
  static void StrToIPv4(string str, cvi_uint8 *pu8IPV4) {
    char t_buffer[16] = {0};
    char *tokenp;
    char *savep;

    strcpy(t_buffer, str.c_str());
    tokenp = strtok_r(t_buffer, ".", &savep);
    if (tokenp) {
      *pu8IPV4 = atoi(tokenp);
    } else {
      CVI_NVRLOGE("please check IPV4 string , like \"192.168.1.1\"\n");
    }

    for (size_t i = 1; i < 4; i++) {
      tokenp = strtok_r(savep, ".", &savep);
      if (tokenp) {
        *(pu8IPV4 + i) = atoi(tokenp);
      } else {
        CVI_NVRLOGE("please check IPV4 string , like \"192.168.1.1\"\n");
      }
    }

    return;
  }
  static string Md5Encode(string strClearContent) {
    // string md5_encode;
    // MD5 *md5 = new MD5();
    // md5_encode = md5->Md5Encrypt(strClearContent, TYPE_32);
    // delete md5;
    // return md5_encode;    //3rd/md5d
  }
  static string Md5Decode(string strMd5Content) { return strMd5Content; }
  static cvi_uint32 GenerateUniqueId() { return 0x01; }
  static cvi_uint32 GeneratUniqueMsgId() { return 0x01; }
  static cvi_uint64 GeneratUniqueDBId() { return 0x01; }
  static cvi_uint64 GeneratUniqueDiskId() { return 0x01; }
  static cvi_uint64 GenerateGroupId() { return 0x01; }
  static cvi_uint64 GenerateUserId() { return 0x01; }

  static cvi_bool CheckIpOnline(string ip);
  static uint8_t num2Array(uint32_t num, char *array, uint8_t digit);
};
#ifdef __cplusplus
}
#endif

#endif //_CVI_UTILITY_H_
