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
        rgb_buffer = nullptr;
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

        cinfo.out_color_space = JCS_EXT_BGR; //JCS_YCbCr;  // ���������ʽ

        if (!jpeg_start_decompress(&cinfo)) {
            printf("decompress error");
            return -1;
        }

        row_stride = cinfo.output_width * cinfo.output_components;
        width = cinfo.output_width;
        height = cinfo.output_height;

        rgb_size = row_stride * cinfo.output_height; // �ܴ�С

        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

        rgb_buffer.reset(new unsigned char[rgb_size + 1]);
        tmp_buffer = rgb_buffer.get();
        while (cinfo.output_scanline < cinfo.output_height) { // ��ѹÿһ��
            jpeg_read_scanlines(&cinfo, buffer, 1);
            // ���Ƶ��ڴ�
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


class Bgr2JpegConverter {
public:
    Bgr2JpegConverter() {
        jpeg_buffer = nullptr;
        jpeg_size = 0;
    }

    ~Bgr2JpegConverter() {
        free(jpeg_buffer);
        jpeg_buffer = nullptr;
        jpeg_size = 0;
    }

    int Convert(unsigned char* rgb_buffer, int width, int height, int quality) {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        int row_stride = 0;
        JSAMPROW row_pointer[1];

        if (rgb_buffer == NULL) {
            printf("you need a pointer for rgb buffer.\n");
            return -1;
        }

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_compress(&cinfo);

        jpeg_mem_dest(&cinfo, &jpeg_buffer, &jpeg_size);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_EXT_BGR;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, true);  // todo 1 == true
        jpeg_start_compress(&cinfo, TRUE);
        row_stride = width * cinfo.input_components;

        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        return 0;
    }


    unsigned char *GetImgBuffer() {
        return jpeg_buffer;
    }

    unsigned long GetSize() {
        return jpeg_size;
    }

private:
    unsigned char* jpeg_buffer;
    unsigned long jpeg_size;
};