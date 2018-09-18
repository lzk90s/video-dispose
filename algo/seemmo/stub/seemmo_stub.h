#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int32_t SeemmoStubOpen();

int32_t SeemmoStubClose();

int32_t SeemmoStubVideoTrailAndRec(int32_t videoChl, uint64_t timeStamp, const uint8_t *rgbImg, uint32_t height, uint32_t width);

#ifdef __cplusplus
}
#endif