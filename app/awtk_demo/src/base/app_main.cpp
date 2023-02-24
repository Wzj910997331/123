// Notes: gui_app_start() is defined in awtk_main.inc
// 1.include awtk.h,if ext_widgets used,include awtk_ext_widgets.h too
// 2.include awtk_app_conf.h,and config the correct params
// 3.include awtk_main.inc(awtk/src/)
// 4.call gui_app_start(lcd_width,lcd_height) in main()
#include "awtk.h"
#include "awtk_app_conf.h"
//#include "awtk_main.inc"

#include "lcd_linux_fb.h"
#include <signal.h>
#include <stdio.h>

#include "assets.inc"

#ifndef APP_DEFAULT_FONT
#define APP_DEFAULT_FONT "default"
#endif /*APP_DEFAULT_FONT*/

#ifndef APP_TYPE
#define APP_TYPE APP_SIMULATOR
#endif /*APP_TYPE*/

#ifndef GLOBAL_INIT
#define GLOBAL_INIT()
#endif /*GLOBAL_INIT*/

#ifndef GLOBAL_EXIT
#define GLOBAL_EXIT()
#endif /*GLOBAL_EXIT*/

#ifndef APP_NAME
#define APP_NAME "awtk"
#endif /*APP_NAME*/

#ifndef APP_RES_ROOT
#define APP_RES_ROOT NULL
#endif /*APP_RES_ROOT*/

#ifdef IOS
#include "base/asset_loader_zip.h"
#endif /*IOS*/

#include "cviapp_init.h"
#include "cviapp_lt9611.h"
#include "home.h"

#include "cviapp_interface.h"

static bool g_bRestartFlag = false;
uint32_t g_gui_width = CVIAPP_DEFAULT_GUI_WIDTH;
uint32_t g_gui_height = CVIAPP_DEFAULT_GUI_HEIGHT;

void restart_app(uint32_t w, uint32_t h) {
  g_gui_width = w;
  g_gui_height = h;
  g_bRestartFlag = true;
}

int gui_app_start(bool restartFlag, int lcd_w, int lcd_h) {

  tk_init(lcd_w, lcd_h, APP_TYPE, APP_NAME, APP_RES_ROOT);

#ifdef ASSETS_ZIP
  assets_manager_set_loader(assets_manager(),
                            asset_loader_zip_create(ASSETS_ZIP));
#endif /*ASSETS_ZIP*/

#if defined(WITH_LCD_PORTRAIT)
  if (lcd_w > lcd_h) {
    tk_set_lcd_orientation(LCD_ORIENTATION_90);
  }
#endif /*WITH_LCD_PORTRAIT*/

#ifdef WITH_LCD_LANDSCAPE
  if (lcd_w < lcd_h) {
    tk_set_lcd_orientation(LCD_ORIENTATION_90);
  }
#endif /*WITH_LCD_PORTRAIT*/

  system_info_set_default_font(system_info(), APP_DEFAULT_FONT);
  assets_init();

#ifndef WITHOUT_EXT_WIDGETS
  tk_ext_widgets_init();
#endif /*WITHOUT_EXT_WIDGETS*/

#ifdef NDEBUG
  log_set_log_level(LOG_LEVEL_INFO);
#else
  log_set_log_level(LOG_LEVEL_DEBUG);
#endif /*NDEBUG*/

#if defined(ENABLE_CURSOR)
  window_manager_set_cursor(window_manager(), "cursor");
#endif

  printf("Build at: %s %s\n", __DATE__, __TIME__);

  if (!restartFlag) {
    GLOBAL_INIT();
    application_init();

    CVIAPP_SYS_Init();

    open_home_page();

    tk_run();
    application_exit();
    GLOBAL_EXIT();
  } else {

    open_home_page();

    tk_run();
  }

  return 0;
}

ret_t application_init(void) { return RET_OK; }

ret_t application_exit(void) { return RET_OK; }

ret_t handle_respMsg() { return RET_OK; }

BEGIN_C_DECLS
extern void main_loop_exit_app_clean(void);
END_C_DECLS

int main(int argc, char *argv[]) {
  if (CVIAPP_Init() != CVI_SUCCESS) {
    printf("CVIAPP_Init() failed\n");
    return -1;
  }

  bool restartFlag = g_bRestartFlag;

RESTART_APP:
  gui_app_start(restartFlag, g_gui_width, g_gui_height);

  if (g_bRestartFlag) {
    if (g_gui_width == 1920) {
    } else if (g_gui_width == 1280) {
    }
    main_loop_exit_app_clean();
    restartFlag = g_bRestartFlag;
    g_bRestartFlag = false;
    goto RESTART_APP;
  }

  main_loop_exit_app_clean();

  CVIAPP_Deinit();

  return 0;
}
