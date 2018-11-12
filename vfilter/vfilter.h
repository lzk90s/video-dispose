#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t VFilter_Init();

int32_t VFilter_Destroy();

int32_t VFilter_Routine(uint32_t channelId, uint8_t *y, uint8_t *u, uint8_t *v, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif
