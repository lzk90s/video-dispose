#include "libyuv.h"
#include <iostream>

#ifdef __cplusplus
extern "C" {
#endif

//YUV420p 缩放
int Scale_i420(const uint8_t* src_y,
               int src_stride_y,
               const uint8_t* src_u,
               int src_stride_u,
               const uint8_t* src_v,
               int src_stride_v,
               int src_width,
               int src_height,
               uint8_t* dst_y,
               int dst_stride_y,
               uint8_t* dst_u,
               int dst_stride_u,
               uint8_t* dst_v,
               int dst_stride_v,
               int dst_width,
               int dst_height) {
    return libyuv::I420Scale(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, src_width, src_height, dst_y,
                             dst_stride_y, dst_u, dst_stride_u, dst_v, dst_stride_v, dst_width, dst_height,
                             libyuv::FilterModeEnum::kFilterNone);
}


#ifdef __cplusplus
}
#endif