/**
 * File:   main_loop_linux.c
 * Author: AWTK Develop Team
 * Brief:  linux implemented main_loop interface
 *
 * Copyright (c) 2018 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * license file for more details.
 *
 */

/**
 * history:
 * ================================================================
 * 2018-09-09 li xianjing <xianjimli@hotmail.com> created
 *
 */
#include <stdio.h>
#include "base/idle.h"
#include "base/timer.h"
#include "base/font_manager.h"
#include "base/window_manager.h"
#include "main_loop/main_loop_simple.h"
#include "native_window/native_window_raw.h"

#include "tslib_thread.h"
#include "input_thread.h"
#include "mouse_thread.h"
#include "lcd_linux_fb.h"
#include "main_loop_linux.h"

#include "tkc/cond.h"

#ifndef FB_DEVICE_FILENAME
#define FB_DEVICE_FILENAME "/dev/fb0"
#endif /*FB_DEVICE_FILENAME*/

#ifndef DRM_DEVICE_FILENAME
#define DRM_DEVICE_FILENAME "/dev/dri/card0"
#endif /*DRM_DEVICE_FILENAME*/

#ifndef TS_DEVICE_FILENAME
#define TS_DEVICE_FILENAME "/dev/input/event0"
#endif /*TS_DEVICE_FILENAME*/

#ifndef KB_DEVICE_FILENAME
#define KB_DEVICE_FILENAME "/dev/input/event1"
#endif /*KB_DEVICE_FILENAME*/

#ifndef MICE_DEVICE_FILENAME
#define MICE_DEVICE_FILENAME "/dev/input/mouse0"
#endif /*MICE_DEVICE_FILENAME*/

static ret_t main_loop_linux_destroy(main_loop_t* l) {
  main_loop_simple_t* loop = (main_loop_simple_t*)l;

  main_loop_simple_reset(loop);
  native_window_raw_deinit();

  return RET_OK;
}

ret_t input_dispatch_to_main_loop(void* ctx, const event_queue_req_t* evt, const char* msg) {
  main_loop_t* l = (main_loop_t*)ctx;
  event_queue_req_t event = *evt;
  event_queue_req_t* e = &event;

  if (l != NULL && l->queue_event != NULL) {
    switch (e->event.type) {
      case EVT_KEY_DOWN:
      case EVT_KEY_UP:
      case EVT_KEY_LONG_PRESS: {
        e->event.size = sizeof(e->key_event);
        break;
      }
      case EVT_CONTEXT_MENU:
      case EVT_POINTER_DOWN:
      case EVT_POINTER_MOVE:
      case EVT_POINTER_UP: {
        e->event.size = sizeof(e->pointer_event);
        break;
      }
      case EVT_WHEEL: {
        e->event.size = sizeof(e->wheel_event);
        break;
      }
      default:
        break;
    }

    main_loop_queue_event(l, e);
    //input_dispatch_print(ctx, e, msg);
  } else {
    return RET_BAD_PARAMS;
  }
  return RET_OK;

}

static tk_thread_t* s_kb_thread = NULL;
static tk_thread_t* s_mice_thread = NULL;
static tk_thread_t* s_mice_thread1 = NULL;
static tk_thread_t* s_ts_thread = NULL;

#if 0
static void on_app_exit(void) {
  if (s_kb_thread != NULL) {
    tk_thread_destroy(s_kb_thread);
  }
  if (s_mice_thread != NULL) {
    tk_thread_destroy(s_mice_thread);
  }
  if (s_ts_thread != NULL) {
    tk_thread_destroy(s_ts_thread);
  }
}
#endif

extern void exit_input_thread(tk_thread_t* thread);
extern void exit_mouse_thread(tk_thread_t* thread);
extern void exit_tslib_thread(tk_thread_t* thread);
extern bool_t lcd_linux_fb_close(void);
void main_loop_exit_app_clean(void) {
  if (s_kb_thread != NULL) {
    exit_input_thread(s_kb_thread);
    tk_thread_destroy(s_kb_thread);
  }
  if (s_mice_thread != NULL) {
    exit_mouse_thread(s_mice_thread);
    tk_thread_destroy(s_mice_thread);
  }
  if (s_ts_thread != NULL) {
    exit_tslib_thread(s_ts_thread);
    tk_thread_destroy(s_ts_thread);
  }

  lcd_linux_fb_close();
}

static tk_cond_t* __tk_cond;
static tk_mutex_t* __tk_mutex;

static ret_t __main_loop_simple_queue_event_mutex(main_loop_t* l, const event_queue_req_t* r) {
  ret_t ret = RET_FAIL;
  main_loop_simple_t* loop = (main_loop_simple_t*)l;

  tk_mutex_lock(__tk_mutex);

  if(r->event.type == EVT_CONTEXT_MENU ||
        r->event.type == EVT_POINTER_DOWN ||
        r->event.type == EVT_POINTER_MOVE ||
        r->event.type == EVT_POINTER_UP)
  {
    ret = event_queue_send(loop->queue, r);
  }
  else
  {
CHECK_AGAIN:
    if(loop->queue->full)
    {
      printf("awtk queue full, wait...\n");
      tk_cond_wait(__tk_cond, __tk_mutex);
      goto CHECK_AGAIN;
    }

    ret = event_queue_send(loop->queue, r);
  }

  tk_mutex_unlock(__tk_mutex);

  return ret;
}

static ret_t __main_loop_simple_recv_event_mutex(main_loop_t* l, event_queue_req_t* r) {
  ret_t ret = RET_FAIL;
  main_loop_simple_t* loop = (main_loop_simple_t*)l;

  bool_t isFull = FALSE;

  tk_mutex_lock(__tk_mutex);

  if(loop->queue->full)
  {
      isFull = TRUE;
  }

  ret = event_queue_recv(loop->queue, r);

  if(isFull)
  {
    printf("cond signal...\n");
    tk_cond_signal(__tk_cond);
  }

  tk_mutex_unlock(__tk_mutex);

  return ret;
}

main_loop_t* main_loop_init(int w, int h) {

  main_loop_simple_t* loop = NULL;

  lcd_t* lcd = lcd_linux_fb_create(FB_DEVICE_FILENAME);

  return_value_if_fail(lcd != NULL, NULL);

  native_window_raw_init(lcd);

  __tk_mutex = tk_mutex_create();
  __tk_cond = tk_cond_create();

  loop = main_loop_simple_init(w, h, __main_loop_simple_queue_event_mutex, __main_loop_simple_recv_event_mutex);
  loop->base.destroy = main_loop_linux_destroy;

#ifndef ENABLE_HDMI_OUTPUT
  s_ts_thread =
      tslib_thread_run(TS_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, lcd->w, lcd->h);
#endif

  s_kb_thread =
      input_thread_run(KB_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, lcd->w, lcd->h);

#ifdef ENABLE_HDMI_OUTPUT

    s_mice_thread =
        mouse_thread_run(KB_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, lcd->w, lcd->h);

    s_mice_thread1 =
        mouse_thread_run(TS_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, lcd->w, lcd->h);



#endif

  //atexit(on_app_exit);

  return (main_loop_t*)loop;
}
