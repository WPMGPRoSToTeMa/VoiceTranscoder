#pragma once

#include <dproto_api.h>

extern dp_enginfo_api_t *g_pDprotoAPI;

extern bool DProtoAPI_Init(void);
extern bool DProtoAPI_Deinit(void);