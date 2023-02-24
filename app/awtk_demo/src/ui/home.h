
#ifndef HOME_PAGE_H_
#define HOME_PAGE_H_

#include "awtk.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CVIAPP_AI_VIEW_SCREEN_W CVIAPP_DEFAULT_GUI_WIDTH
#define CVIAPP_AI_VIEW_SCREEN_H CVIAPP_DEFAULT_GUI_HEIGHT

#define CVIAPP_IPC_VIEW_SCREEN_W CVIAPP_AI_VIEW_SCREEN_W
#define CVIAPP_IPC_VIEW_SCREEN_H CVIAPP_AI_VIEW_SCREEN_H

#define CVIAPP_IPC_VIEW_SCREEN_DEFAULT_NUM 4

void open_home_page(void);

#ifdef __cplusplus
}
#endif

#endif
