/**
 * File:   lcd_linux_fb.h
 * Author: AWTK Develop Team
 * Brief:  linux framebuffer lcd
 *
 * Copyright (c) 2018 - 2020  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-09-07 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include "awtk_global.h"
#include "base/lcd.h"
#include "fb_info.h"
#include "tkc/mem.h"
#include "tkc/thread.h"
#include "tkc/time_now.h"
#include <signal.h>
//#include "tkc/mutex.h"
#include "base/system_info.h"
#include "blend/image_g2d.h"
#include "lcd/lcd_mem_bgr565.h"
#include "lcd/lcd_mem_bgra8888.h"
#include "lcd/lcd_mem_rgb565.h"
#include "lcd/lcd_mem_rgba8888.h"
#include "lcd_mem_others.h"
#include "tkc/semaphore.h"

#if !defined(WITH_LINUX_DRM) && !defined(WITH_LINUX_DRM)

#ifndef DISPLAY_WAIT_TIME
#define DISPLAY_WAIT_TIME 5000
#endif

extern uint32_t g_gui_width;
extern uint32_t g_gui_height;

static fb_info_t s_fb;
// static int s_ttyfd = -1;
// static lcd_t* s_lcd = NULL;

#if 0
static void on_app_exit(void) {
  fb_info_t* fb = &s_fb;

  //if (s_ttyfd >= 0) {
  //  ioctl(s_ttyfd, KDSETMODE, KD_TEXT);
  //}

  fb_close(fb);
}
#else
void lcd_linux_fb_close(void) {
  fb_info_t *fb = &s_fb;
  fb_close(fb);
}
#endif

// static void on_signal_int(int sig) {
//  tk_quit();
//}

static ret_t lcd_mem_fb_sync(lcd_t *lcd) {
  fb_info_t *fb = &s_fb;
  fb_sync(fb);
  return RET_OK;
}

static lcd_t *lcd_linux_create(fb_info_t *fb) {
  lcd_t *lcd = NULL;
  int w = fb->var.xres;
  int h = fb->var.yres;
  int bpp = fb_bpp(fb); // 32
  int line_length = w * 4;
  uint32_t fb_size = w * h * 4;

  uint8_t *online_fb = fb->fbmem0;
  uint8_t *offline_fb = (uint8_t *)TKMEM_ALLOC(fb_size);

  if (bpp == 16) {
    if (fb_is_bgr565(fb)) {
      // lcd = lcd_mem_bgr565_create_single_fb(w, h, buff);
      assert(!"not supported framebuffer format.");
    } else if (fb_is_rgb565(fb)) {
      // lcd = lcd_mem_rgb565_create_single_fb(w, h, buff);
      assert(!"not supported framebuffer format.");
    } else {
      assert(!"not supported framebuffer format.");
    }
  } else if (bpp == 32) {
    if (fb_is_bgra8888(fb)) {
      // lcd = lcd_mem_bgra8888_create_single_fb(w, h, buff);
      lcd = lcd_mem_bgra8888_create_double_fb(w, h, online_fb, offline_fb);
    } else if (fb_is_rgba8888(fb)) {
      // lcd = lcd_mem_rgba8888_create_single_fb(w, h, buff);
      lcd = lcd_mem_rgba8888_create_double_fb(w, h, online_fb, offline_fb);
    } else {
      assert(!"not supported framebuffer format.");
    }
  } else if (bpp == 24) {
    assert(!"not supported framebuffer format.");
  } else {
    assert(!"not supported framebuffer format.");
  }

  if (lcd != NULL) {
    // s_lcd = lcd;
    lcd->sync = lcd_mem_fb_sync;
    ((lcd_mem_t *)lcd)->own_offline_fb = TRUE;
    lcd_mem_set_line_length(lcd, line_length);
  }
  return lcd;
}

static const char *s_fb_path = NULL;

lcd_t *lcd_linux_fb_create(const char *filename) {
  lcd_t *lcd = NULL;
  fb_info_t *fb = &s_fb;
  return_value_if_fail(filename != NULL, NULL);

  s_fb_path = filename;

  if (fb_open(fb, filename, g_gui_width, g_gui_height) == 0) {

    // s_ttyfd = open("/dev/tty1", O_RDWR);
    // if (s_ttyfd >= 0) {
    //  ioctl(s_ttyfd, KDSETMODE, KD_GRAPHICS);
    //}

    // fix FBIOPUT_VSCREENINFO block issue when run in vmware double fb mode
    // if (check_if_run_in_vmware()) {
    //  log_info("run in vmware and fix FBIOPUT_VSCREENINFO block issue\n");
    //  fb->var.activate = FB_ACTIVATE_INV_MODE;
    //  fb->var.pixclock = 60;
    //}

    lcd = lcd_linux_create(fb);
  }

  // atexit(on_app_exit);
  // signal(SIGINT, on_signal_int);

  return lcd;
}

#endif
