
#ifndef __CVIAPP_INIT_H__
#define __CVIAPP_INIT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// mw header
#include "cvi_comm_vo.h"
#include "cvi_defines.h"
#include "cvi_type.h"

// awtk header
#include "awtk.h"

CVI_S32 CVIAPP_Init(void);
void CVIAPP_Deinit();

void CVIAPP_VoInit(VO_INTF_SYNC_E eVoType);
void CVIAPP_VoDeinit(void);
void CVIAPP_GetVoSize(uint32_t *w, uint32_t *h);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
