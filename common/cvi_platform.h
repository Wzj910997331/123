/**
 * File Name: cvi_platform.h
 *
 * Version: V1.0
 *
 * Brief: platform portable APIs.
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
#ifndef _CVI_PLATFORM_H_
#define _CVI_PLATFORM_H_
#include "cvi_base_type.h"
#include "cvi_common_type.h"
#include "cvi_const_error.h"
#include <errno.h>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
using namespace std;

#include "glog/logging.h"

// mutex
class CVI_Mutex_SC {
public:
  void Lock() { pthread_mutex_lock(&m_oMutex); }
  void Unlock() { pthread_mutex_unlock(&m_oMutex); }
  bool Trylock() {
    return (0x00 == pthread_mutex_trylock(&m_oMutex)) ? true : false;
  }
  int Timedlock() {
    struct timespec tout;
    unsigned long int nsec = 20 * 1000 * 1000; // ns
    memset(&tout, 0, sizeof(timespec));
    clock_gettime(CLOCK_REALTIME, &tout);

    if ((tout.tv_nsec + nsec) < 999999999) {
      tout.tv_nsec = tout.tv_nsec + nsec;
    } else {
      tout.tv_sec += 1;
      tout.tv_nsec = tout.tv_nsec - nsec;
    }

    return (pthread_mutex_timedlock(&m_oMutex, &tout));
  }
  CVI_Mutex_SC() { pthread_mutex_init(&m_oMutex, NULL); }
  virtual ~CVI_Mutex_SC() { pthread_mutex_destroy(&m_oMutex); }

private:
  pthread_mutex_t m_oMutex;

private:
  CVI_Mutex_SC(const CVI_Mutex_SC &p);
  CVI_Mutex_SC &operator=(const CVI_Mutex_SC &p);
};

// scope mutex
class CVI_Scope_Mutex_SC {
public:
  CVI_Scope_Mutex_SC(CVI_Mutex_SC &mutex) : m_Mutex(mutex) { m_Mutex.Lock(); }

  virtual ~CVI_Scope_Mutex_SC() { m_Mutex.Unlock(); }

private:
  CVI_Mutex_SC &m_Mutex;

private:
  CVI_Scope_Mutex_SC(const CVI_Scope_Mutex_SC &p);
  CVI_Scope_Mutex_SC &operator=(const CVI_Scope_Mutex_SC &p);
};

// semaphore
class CVI_Semaphore_SC {
public:
  void Post() { sem_post(&m_Sem); }
  void Wait() { sem_wait(&m_Sem); }
  CVI_Semaphore_SC(cvi_int32 u32InitNum = 0) {
    sem_init(&m_Sem, 0, u32InitNum);
  }
  virtual ~CVI_Semaphore_SC() { sem_destroy(&m_Sem); }

private:
  sem_t m_Sem;

private:
  CVI_Semaphore_SC(const CVI_Semaphore_SC &p);
  CVI_Semaphore_SC &operator=(const CVI_Semaphore_SC &p);
};

// queue
#include "cvi_queue_sc.h"

// thread
typedef void *(*CVI_THREAD_ROUTINE_FN_T)(void *);

class CVI_Thread_SC {
public:
  // Not used yet, use if the audio is not smooth.
  typedef enum {
    E_CVI_THREAD_PRIO_DEFAULT = 20, // The thread default run at this priority.
    E_CVI_THREAD_PRIO_LOW =
        10, // rtsp client thread default run at this priority.
    E_CVI_THREAD_PRIO_MEDIUM = 0, // GUI run at this priority.
    E_CVI_THREAD_PRIO_AUDIO =
        -10, // Foreground audio thread, aoThread and foreground adecThread.
    E_CVI_THREAD_PRIO_HIGH = -15, // Foreground rtsp client thread, receive
                                  // audio data required by foreground audio
                                  // thread.
  } CVI_THREAD_PRIO_E;

  static CVI_ERROR_CODE_E AdjustPrio(cvi_uint64 u64Pid,
                                     CVI_THREAD_PRIO_E ePrio);
  static CVI_ERROR_CODE_E AdjustSelfPrio(CVI_THREAD_PRIO_E ePrio);

public:
  // method APIs
  virtual CVI_ERROR_CODE_E StartThread();
  virtual CVI_ERROR_CODE_E StopThread();
  virtual CVI_ERROR_CODE_E Join();
  virtual CVI_ERROR_CODE_E DestroyThread();
  virtual CVI_ERROR_CODE_E PauseThread();
  virtual CVI_ERROR_CODE_E ResumeThread();

  CVI_Thread_SC(CVI_THREAD_ROUTINE_FN_T pFn, void *pParam,
                const char *pThreadName = "cvi_thread",
                CVI_THREAD_PRIO_E ePrio = E_CVI_THREAD_PRIO_DEFAULT);

  virtual ~CVI_Thread_SC();

private:
  pthread_t m_pThread;

  CVI_THREAD_ROUTINE_FN_T m_pFn;
  void *m_pParam;
  cvi_bool m_bStart;
  const char *m_pThreadName;
  CVI_THREAD_PRIO_E m_ePrio;

  static void *run(void *pParam);

private:
  CVI_Thread_SC(const CVI_Thread_SC &p);
  CVI_Thread_SC &operator=(const CVI_Thread_SC &p);
};

class CVI_TraceFunCall_C {
public:
  CVI_TraceFunCall_C(const char *fileName, const char *funName)
      : m_fileName(fileName), m_funName(funName) {
    // LOG(INFO) << m_fileName << "::" << m_funName << "++++++++++++++++++";
  }

  ~CVI_TraceFunCall_C() {
    // LOG(INFO) << m_fileName << "::" << m_funName << "------------------";
  }

private:
  const char *m_fileName;
  const char *m_funName;
};

#define TRACE_FUN_CALL() CVI_TraceFunCall_C trace(__FILE__, __func__)

#endif //_CVI_PLATFORM_H_
