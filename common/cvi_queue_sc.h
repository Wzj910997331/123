/**
 * File Name: cvi_queue_sc.h
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
   1.Date 			:	2020/09/03
   Author 			:	mason.zou
   Modification		:	Created file
 *
 ====================================================================================*/

#ifndef _CVI_QUEUE_SC_
#define _CVI_QUEUE_SC_

#define E_CVI_ERROR_CODE_MODULE_QUEUE_TIMEOUT (CVI_ERROR_CODE_MODULE_QUEUE_BASE)

template <class T> class CVI_Queue_SC {
public:
  CVI_Queue_SC(const cvi_uint32 size = 64);
  virtual ~CVI_Queue_SC();

  CVI_ERROR_CODE_E TryPush(T &oMsg,
                           cvi_uint32 u32Timeout); // u32Timeout = xx ms
  CVI_ERROR_CODE_E Push(T &oMsg);
  CVI_ERROR_CODE_E Pop(T &oMsg);
  CVI_ERROR_CODE_E TryPop(T &oMsg, cvi_uint32 u32Timeout); // u32Timeout = xx ms
  CVI_ERROR_CODE_E Peek(T &oMsg);
  CVI_ERROR_CODE_E Empty();

private:
  cvi_uint32 m_MaxSize;

  pthread_mutex_t m_oMutex;

  pthread_condattr_t m_attr;

  pthread_cond_t m_condFull;
  pthread_cond_t m_condEmpty;

  queue<T> m_oQueue;

private:
  CVI_Queue_SC(const CVI_Queue_SC &p);
  CVI_Queue_SC &operator=(const CVI_Queue_SC &p);
};

template <class T>
CVI_Queue_SC<T>::CVI_Queue_SC(const cvi_uint32 size) : m_MaxSize(size) {
  pthread_mutex_init(&m_oMutex, NULL);

  pthread_condattr_init(&m_attr);
  pthread_condattr_setclock(&m_attr, CLOCK_MONOTONIC);

  pthread_cond_init(&m_condFull, &m_attr);

  pthread_cond_init(&m_condEmpty, &m_attr);
}

template <class T> CVI_Queue_SC<T>::~CVI_Queue_SC() {
  pthread_mutex_destroy(&m_oMutex);

  pthread_condattr_destroy(&m_attr);

  pthread_cond_destroy(&m_condFull);

  pthread_cond_destroy(&m_condEmpty);
}

template <class T>
CVI_ERROR_CODE_E CVI_Queue_SC<T>::TryPush(T &oMsg, cvi_uint32 u32Timeout) {
  pthread_mutex_lock(&m_oMutex);

  while (m_oQueue.size() == m_MaxSize) {
    struct timespec tv;

    clock_gettime(CLOCK_MONOTONIC, &tv);

    unsigned long int nsec = tv.tv_nsec + (u32Timeout % 1000) * 1000000;

    tv.tv_nsec = nsec % 1000000000;

    tv.tv_sec = tv.tv_sec + u32Timeout / 1000 + nsec / 1000000000;

    if (pthread_cond_timedwait(&m_condFull, &m_oMutex, &tv) != 0) {
      pthread_mutex_unlock(&m_oMutex);

      return E_CVI_ERROR_CODE_MODULE_QUEUE_TIMEOUT;
    }
  }

  m_oQueue.push(oMsg);

  if (m_oQueue.size() == 1) {
    pthread_cond_signal(&m_condEmpty);
  }

  pthread_mutex_unlock(&m_oMutex);

  return E_CVI_ERROR_CODE_SUCC;
}

template <class T> CVI_ERROR_CODE_E CVI_Queue_SC<T>::Push(T &oMsg) {
  pthread_mutex_lock(&m_oMutex);

  while (m_oQueue.size() == m_MaxSize) {
    pthread_cond_wait(&m_condFull, &m_oMutex);
  }

  m_oQueue.push(oMsg);

  if (m_oQueue.size() == 1) {
    pthread_cond_signal(&m_condEmpty);
  }

  pthread_mutex_unlock(&m_oMutex);

  return E_CVI_ERROR_CODE_SUCC;
}

template <class T> CVI_ERROR_CODE_E CVI_Queue_SC<T>::Peek(T &oMsg) {
  pthread_mutex_lock(&m_oMutex);

  if (m_oQueue.size() == 0) {
    pthread_mutex_unlock(&m_oMutex);
    return E_CVI_ERROR_CODE_FAULT;
  } else {
    oMsg = m_oQueue.front();
    pthread_mutex_unlock(&m_oMutex);
    return E_CVI_ERROR_CODE_SUCC;
  }
}

template <class T> CVI_ERROR_CODE_E CVI_Queue_SC<T>::Pop(T &oMsg) {
  pthread_mutex_lock(&m_oMutex);

  while (m_oQueue.size() == 0) {
    pthread_cond_wait(&m_condEmpty, &m_oMutex);
  }

  oMsg = m_oQueue.front();
  m_oQueue.pop();

  if (m_oQueue.size() == (m_MaxSize - 1)) {
    pthread_cond_signal(&m_condFull);
  }

  pthread_mutex_unlock(&m_oMutex);

  return E_CVI_ERROR_CODE_SUCC;
}

template <class T>
CVI_ERROR_CODE_E CVI_Queue_SC<T>::TryPop(T &oMsg, cvi_uint32 u32Timeout) {
  pthread_mutex_lock(&m_oMutex);

  while (m_oQueue.size() == 0) {
    struct timespec tv;

    clock_gettime(CLOCK_MONOTONIC, &tv);

    unsigned long int nsec = tv.tv_nsec + (u32Timeout % 1000) * 1000000;

    tv.tv_nsec = nsec % 1000000000;

    tv.tv_sec = tv.tv_sec + u32Timeout / 1000 + nsec / 1000000000;

    if (pthread_cond_timedwait(&m_condEmpty, &m_oMutex, &tv) != 0) {
      pthread_mutex_unlock(&m_oMutex);

      return E_CVI_ERROR_CODE_MODULE_QUEUE_TIMEOUT;
    }
  }

  // LOG(INFO) << "oliver:" << m_oQueue.size();
  oMsg = m_oQueue.front();
  m_oQueue.pop();

  if (m_oQueue.size() == (m_MaxSize - 1)) {
    pthread_cond_signal(&m_condFull);
  }

  pthread_mutex_unlock(&m_oMutex);

  return E_CVI_ERROR_CODE_SUCC;
}

template <class T> CVI_ERROR_CODE_E CVI_Queue_SC<T>::Empty() {
  pthread_mutex_lock(&m_oMutex);

  while (m_oQueue.size()) {
    m_oQueue.pop();
  }

  pthread_cond_signal(&m_condFull);

  pthread_mutex_unlock(&m_oMutex);

  return E_CVI_ERROR_CODE_SUCC;
}

#endif
