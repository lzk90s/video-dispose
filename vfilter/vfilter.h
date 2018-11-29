#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
初始化
*/
int32_t VFilter_Init();

/*
销毁
*/
int32_t VFilter_Destroy();

/**
过滤
@param channelId 通道id
@param y YUV y通道
@param stride_y 每行Y所占的字节数
@param u YUV u通道
@param stride_u 每行U所占的字节数
@param v YUV v通道
@param stride_v 每行V所占的字节数
@param width 图像宽
@param height 图像高
*/
int32_t VFilter_Routine(uint32_t channelId,
                        uint8_t *y, uint32_t stride_y,
                        uint8_t *u, uint32_t stride_u,
                        uint8_t *v, uint32_t stride_v,
                        uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif
