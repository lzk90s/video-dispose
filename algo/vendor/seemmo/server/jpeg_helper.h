#pragma once

// code from:  https://blog.csdn.net/subfate/article/details/46700675

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <memory>

#include "jpeglib.h"
#include "jerror.h"

namespace algo {
namespace seemmo {

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

void my_error_exit(j_common_ptr cinfo) {
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    (*cinfo->err->output_message) (cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}

class Jpeg2BgrConverter {
public:
    Jpeg2BgrConverter() {
        height = 0;
        width = 0;
    }

    int Convert(unsigned char* jpeg_buffer, int jpeg_size) {
        struct jpeg_decompress_struct cinfo;
        struct my_error_mgr jerr;

        JSAMPARRAY buffer;
        int row_stride = 0;
        unsigned char* tmp_buffer = NULL;
        int rgb_size;

        if (jpeg_buffer == NULL) {
            printf("no jpeg buffer here.\n");
            return -1;
        }

        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = my_error_exit;

        if (setjmp(jerr.setjmp_buffer)) {
            jpeg_destroy_decompress(&cinfo);
            return -1;
        }

        jpeg_create_decompress(&cinfo);

        jpeg_mem_src(&cinfo, jpeg_buffer, jpeg_size);

        jpeg_read_header(&cinfo, TRUE);

        cinfo.out_color_space = JCS_EXT_BGR; //JCS_YCbCr;  // 设置输出格式

        jpeg_start_decompress(&cinfo);

        row_stride = cinfo.output_width * cinfo.output_components;
        width = cinfo.output_width;
        height = cinfo.output_height;

        rgb_size = row_stride * cinfo.output_height; // 总大小

        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

        rgb_buffer.reset(new unsigned char[rgb_size+1]);
        tmp_buffer = rgb_buffer.get();
        while (cinfo.output_scanline < cinfo.output_height) { // 解压每一行
            jpeg_read_scanlines(&cinfo, buffer, 1);
            // 复制到内存
            memcpy(tmp_buffer, buffer[0], row_stride);
            tmp_buffer += row_stride;
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);

        return 0;
    }

    unsigned char *GetImgBuffer() {
        return rgb_buffer.get();
    }

    int GetWidth() {
        return width;
    }

    int GetHeight() {
        return height;
    }

private:
    std::unique_ptr<unsigned char[]> rgb_buffer;
    int width;
    int height;
};


}
}