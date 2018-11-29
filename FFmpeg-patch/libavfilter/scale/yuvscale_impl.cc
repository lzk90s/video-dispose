#include "libyuv.h"

#ifdef __cplusplus
extern "C" {
#endif

int Scale_i420(const uint8_t* src_y,
               const uint8_t* src_u,
               const uint8_t* src_v,
               int src_width,
               int src_height,
               uint8_t* dst_y,
               uint8_t* dst_u,
               uint8_t* dst_v,
               int dst_width,
               int dst_height) {
    int src_stride_y = src_width;
    int src_stride_u = src_width / 2;
    int src_stride_v = src_width / 2;
    int dst_stride_y = dst_width;
    int dst_stride_u = dst_width / 2;
    int dst_stride_v = dst_width / 2;
    return libyuv::I420Scale(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, src_width, src_height, dst_y,
                             dst_stride_y, dst_u, dst_stride_u, dst_v, dst_stride_v, dst_width, dst_height,
                             libyuv::FilterModeEnum::kFilterNone);
}


#ifdef __cplusplus
}
#endif