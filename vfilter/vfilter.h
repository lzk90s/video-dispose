#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t VFilter_Open();

int32_t VFilter_Close();

int32_t VFilter_Routine(uint8_t *bgr24, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif
