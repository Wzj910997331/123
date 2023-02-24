/**
 * File Name: cvi_buffer.h
 *
 * Version: V1.0
 *
 * Brief: buffer.
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
#ifndef _CVI_BUFFER_H_
#define _CVI_BUFFER_H_
#include <iostream>
#include <vector>
using namespace std;

#include "cvi_base_type.h"
#include "cvi_common_type.h"
#include "cvi_const.h"
#include "cvi_platform.h"
#include "glog/logging.h"
class CVI_Buffer_C : public CVI_Object_C {
public:
  CVI_Buffer_C(){};
  virtual ~CVI_Buffer_C(){};
};

// ms
#define CVI_NBUFFER_DIFF_TIME_OF_REAL_TIME 500
#define CVI_NBUFFER_DIFF_TIME_OF_AVERAGE 1000
#define CVI_NBUFFER_DIFF_TIME_OF_SMOOTH 2000

typedef enum {
  E_CVI_BUFFER_QUEUE_ID_FIRST = 1,
  E_CVI_BUFFER_QUEUE_ID_RECORD = 1,
  E_CVI_BUFFER_QUEUE_ID_PREVIEW = 2,
  E_CVI_BUFFER_QUEUE_ID_FORWARD = 3,
  E_CVI_BUFFER_QUEUE_ID_NUM = 3
} CVI_BUFFER_QUEUE_ID;
typedef enum {
  E_CVI_BUFFER_PLAY_MODE_FIRST = 1,
  E_CVI_BUFFER_PLAY_MODE_REAL_TIME = 1,
  E_CVI_BUFFER_PLAY_MODE_AVERAGE = 2,
  E_CVI_BUFFER_PLAY_MODE_SMOOTH = 3,
  E_CVI_BUFFER_PLAY_MODE_NO_ACTION = 4,
  E_CVI_BUFFER_PLAY_MODE_NUM
} CVI_BUFFER_PLAY_MODE;

typedef struct {
  cvi_uint8 m_u8KeyFrameFlag;
  cvi_uint64 m_u64Pts;
  cvi_uint32 m_u32DataSize;
  cvi_uint8 *m_pu8Buffer;
  cvi_uint32 m_u32BufferMaxSize; // use read pkt
} CVI_RING_BUFFER_PKT_S;

typedef void (*CVI_RING_BUFFER_DESTROY_CALLBACK)(void *pCtx);

typedef struct {
  void *m_pCtx;
  CVI_RING_BUFFER_DESTROY_CALLBACK m_pFun;
} CVI_RingBufferDestroyCallback_S;

class CVI_RingBuffer_SC {
public:
  CVI_ERROR_CODE_E ReadPkt(CVI_BUFFER_QUEUE_ID queueID,
                           CVI_RING_BUFFER_PKT_S &stRbPkt);
  CVI_ERROR_CODE_E WritePkt(CVI_RING_BUFFER_PKT_S &stRbPkt);

  CVI_ERROR_CODE_E SetPlayMode(CVI_BUFFER_QUEUE_ID queueID,
                               CVI_BUFFER_PLAY_MODE eMode);

  cvi_uint32 PeekRemainSize(CVI_BUFFER_QUEUE_ID queueID);

  CVI_ERROR_CODE_E SetTimeBase(cvi_float ftb);
  CVI_ERROR_CODE_E UpdateReadPktSize(CVI_BUFFER_QUEUE_ID queueID,
                                     cvi_uint32 u32Diffms);
  CVI_ERROR_CODE_E UpdateReadIndexToKeyFrame(CVI_BUFFER_QUEUE_ID queueID);

  CVI_ERROR_CODE_E
  RegisterDestroyCallback(const CVI_RingBufferDestroyCallback_S &stCallback);
  CVI_ERROR_CODE_E
  UnregisterDestroyCallback(const CVI_RingBufferDestroyCallback_S &stCallback);

  CVI_RingBuffer_SC(cvi_uint32 u32Size);
  virtual ~CVI_RingBuffer_SC();

private:
  CVI_Mutex_SC m_oMutex;

  cvi_uint32 m_u32WPtr;

  deque<cvi_uint32> m_frameDeque;

  cvi_uint32 m_u32RecordIndex;
  cvi_uint32 m_u32PreviewIndex;
  cvi_uint32 m_u32ForwardIndex;

  CVI_BUFFER_PLAY_MODE m_playModePre, m_playModeFow;

  cvi_uint32 GetIndex(CVI_BUFFER_QUEUE_ID queueID);
  void SetIndex(CVI_BUFFER_QUEUE_ID queueID, cvi_uint32 index);

  CVI_ERROR_CODE_E RemoveBrokenPkt(const cvi_uint32 &requestSize);
  CVI_ERROR_CODE_E RemovePktReferPlayMode(const cvi_uint64 &u64CurrentPts,
                                          CVI_BUFFER_QUEUE_ID queueID);

  cvi_uint8 *m_pu8Buffer;
  cvi_uint32 m_u32Size;

  cvi_float m_ftb;

  vector<CVI_RingBufferDestroyCallback_S> m_stCallback;
};

class CVI_BufferMemPool_GSC {
public:
  static void Init();
  // static void Deinit();
  static CVI_ERROR_CODE_E AllocBuffer(cvi_uint32 u32Bytes,
                                      cvi_uint8 *&pu8Buffer);
  static CVI_ERROR_CODE_E FreeBuffer(cvi_uint8 *pu8Buffer);
};

#endif //_CVI_BUFFER_H_
