#ifndef __AWTK_APP_CONF_H__
#define __AWTK_APP_CONF_H__
#define WITHOUT_WINDOW_ANIMATORS 1

//#define WITHOUT_INPUT_METHOD 1
//WITH_TRUETYPE_FONT
//AWTK_WEB
#define WITHOUT_DIALOG_HIGHLIGHTER 1
#define WITHOUT_WIDGET_ANIMATORS 1
#define WITHOUT_CLIPBOARD 1
#define WITH_BITMAP_BGRA 1
//WITH_NULL_IM

#define WITH_NANOVG_AGGE
#define WITH_VGCANVAS

#define HAS_STD_MALLOC 1
#define WITH_UNICODE_BREAK 1
#define LINUX 1
#define WITH_ASSET_LOADER 1
#define WITH_FS_RES 1
#define STBTT_STATIC 1
#define STB_IMAGE_STATIC 1
#define WITH_STB_IMAGE 1
#define WITH_STB_FONT 1

//#define HAS_STDIO 1
#define WITH_IME_PINYIN 1
#define HAS_PTHREAD 1
#define ENABLE_CURSOR 1
#define WITH_DATA_READER_WRITER 1
#define WITH_EVENT_RECORDER_PLAYER 1
#define WITH_WIDGET_TYPE_CHECK 1
#define USE_GUI_MAIN 1
//#define WITHOUT_EXT_WIDGETS 1
#define APP_TYPE APP_MOBILE

#ifndef APP_THEME
#define APP_THEME "default"
#endif /*APP_THEME*/

//it will See the ghosting when a widget is placed on a transparent background
//we should clear buffer before refresh widget
#define CVI_CUSTOMIZATION
#define WITH_LCD_CLEAR_ALPHA 1

extern int gui_app_start(int lcd_w, int lcd_h);
extern ret_t application_init();
extern ret_t application_exit();

#else
#error "include awtk_app_conf twice"
#endif /* End of #ifndef __AWTK_APP_CONF_H__*/

