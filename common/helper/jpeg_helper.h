#pragma once

// 代码参考:  https://blog.csdn.net/subfate/article/details/46700675

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

        cinfo.out_color_space = JCS_EXT_BGR; //JCS_YCbCr;  // 设置输出格式

        if (!jpeg_start_decompress(&cinfo)) {
            printf("decompress error");
            return -1;
        }

        row_stride = cinfo.output_width * cinfo.output_components;
        width = cinfo.output_width;
        height = cinfo.output_height;

        rgb_size = row_stride * cinfo.output_height; // 总大小

        buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

        rgb_buffer.reset(new unsigned char[rgb_size + 1]);
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

    int SaveJpeg(const char *filename, unsigned char *bits, int width, int height) {
        struct jpeg_compress_struct jcinfo;  //申请jpeg压缩对象
        struct jpeg_error_mgr jerr;
        FILE * outfile;                 //target file
        JSAMPROW row_pointer[1];        //pointer to JSAMPLE row[s] 一行位图
        int     row_stride;             //每一行的字节数
        jcinfo.err = jpeg_std_error(&jerr);   //指定错误处理器
        jpeg_create_compress(&jcinfo);      //初始化jpeg压缩对象

        //指定压缩后的图像所存放的目标文件，注意，目标文件应以二进制模式打开
        if ((outfile = fopen(filename, "wb")) == NULL) {
            fprintf(stderr, "can't open %s/n", filename);
            return -1;
        }
        jpeg_stdio_dest(&jcinfo, outfile);   //指定压缩后的图像所存放的目标文件
        jcinfo.image_width = width;      // 为图的宽和高，单位为像素
        jcinfo.image_height = height;
        jcinfo.input_components = 3;         // 在此为3,表示彩色位图， 如果是灰度图，则为1
        jcinfo.in_color_space = JCS_EXT_BGR;         //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像
        /*
        需要注意的是，jpeg_set_defaults函数一定要等设置好图像宽、高、色彩通道数计色彩空间四个参数后才能调用，因为这个函数要用到这四个值，调用jpeg_set_defaults函数后，jpeglib 库采用默认的设置对图像进行压缩
        如果需要改变设置，如压缩质量，调用这个函数后，可以调用其它设置函数，如jpeg_set_quality函数。其实图像压缩时有好多参数可以设置
        但大部分我们都用不着设置，只需调用jpeg_set_defaults函数值为默认值即可
        */
        jpeg_set_defaults(&jcinfo);
        jpeg_set_quality(&jcinfo, 100, TRUE);//limit to baseline-JPEG values
        /*
        首先调用jpeg_start_compress，然后可以对每一行进行压缩，也可以对若干行进行压缩，甚至可以对整个的图像进行一次压缩，压缩完成后，记得要调用jpeg_finish_compress函数
        */
        jpeg_start_compress(&jcinfo, TRUE);

        row_stride = width * jcinfo.input_components; // JSAMPLEs per row in image_buffer(如果是索引图则不需要乘以3)
        //对每一行进行压缩
        while (jcinfo.next_scanline < jcinfo.image_height) {
            //这里我做过修改，由于jpg文件的图像是倒的，所以改了一下读的顺序
            //这是原代码：
            //row_pointer[0] = & bits[jcinfo.next_scanline * row_stride];
            row_pointer[0] = &bits[(jcinfo.image_height - jcinfo.next_scanline - 1) * row_stride];
            (void)jpeg_write_scanlines(&jcinfo, row_pointer, 1);
        }
        jpeg_finish_compress(&jcinfo);
        fclose(outfile);
        jpeg_destroy_compress(&jcinfo);
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
