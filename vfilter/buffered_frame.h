#pragma once

#include "opencv/cv.h"
#include "opencv2/opencv.hpp"

#include "common/helper/jpeg_helper.h"
#include "libyuv.h"

#include "vfilter/setting.h"

namespace vf {

class BufferedFrame {
public:
    virtual ~BufferedFrame() {}

    virtual void Put(cv::Mat &frame) {}

    virtual cv::Mat Get() {
        return cv::Mat();
    }
};


//原始数据buffer，未经过任何压缩处理
class OriginBufferedFrame : public BufferedFrame {
public:
    void Put(cv::Mat &frame) override {
        mat_ = frame;
    }

    cv::Mat Get() override {
        return mat_;
    }

private:
    cv::Mat mat_;
};


//bgr转为yuv buffer，yuv的空间占用比rgb小一半，这里使用libyuv进行转换，不使用opencv自带的cvtColor，libyuv性能更好
class YUVBufferedFrame : public BufferedFrame {
public:
    YUVBufferedFrame() {
        width_ = 0;
        height_ = 0;
    }


    void Put(cv::Mat &frame) override {
        width_ = frame.cols;
        height_ = frame.rows;

        yuv_.reset(new uint8_t[width_*height_ * 3 / 2] { 0 });

        //cvmat默认虽然是bgr排列，不过直接用rgb也没有关系，转回来的时候用相应的方法就不会有问题
        int width = width_;
        int height = height_;
        const uint8_t* src_rgb24 = frame.data;
        int src_stride_rgb24 = width * 3;
        uint8_t* dst_y = yuv_.get();
        int dst_stride_y = width;
        uint8_t* dst_u = yuv_.get() + height * width * 5 / 4;
        int dst_stride_u = width / 2;
        uint8_t* dst_v = yuv_.get() + height * width;
        int dst_stride_v = width / 2;
        libyuv::RGB24ToI420(src_rgb24, src_stride_rgb24, dst_y, dst_stride_y, dst_u, dst_stride_u, dst_v, dst_stride_v, width,
                            height);
    }

    cv::Mat Get() override {
        unique_ptr<uint8_t[]> rgb(new uint8_t[width_*height_ * 3] { 0 });

        int width = width_;
        int height = height_;
        const uint8_t* src_y = yuv_.get();
        int src_stride_y = width;
        const uint8_t* src_u = yuv_.get() + width * height * 5 / 4;
        int src_stride_u = width / 2;
        const uint8_t* src_v = yuv_.get() + width * height;
        int src_stride_v = width / 2;
        uint8_t* dst_rgb24 = rgb.get();
        int dst_stride_rgb24 = width * 3;
        libyuv::I420ToRGB24(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v, dst_rgb24, dst_stride_rgb24, width,
                            height);

        cv::Mat mat = cv::Mat(height, width, CV_8UC3);
        memcpy(mat.data, rgb.get(), width*height * 3);

        return mat;
    }

private:
    uint32_t width_;
    uint32_t height_;
    unique_ptr<uint8_t[]> yuv_;
};


//jpeg压缩buffer，把bgr压缩为jpeg图片，压缩比很大
class JpegBufferedFrame : public BufferedFrame {
public:
    JpegBufferedFrame() {
        compressed_ = false;
    }

    void Put(cv::Mat &frame) override {
        if (!compressed_) {
            Bgr2JpegConverter conv;
            conv.Convert(frame.data, frame.cols, frame.rows, 100);
            jpgSize_ = conv.GetSize();
            jpg_.reset(new uint8_t[jpgSize_], [](uint8_t*ptr) {
                delete[]ptr;
            });
            memcpy(jpg_.get(), conv.GetImgBuffer(), conv.GetSize());
            compressed_ = true;
        }
    }

    cv::Mat Get() override {
        if (compressed_) {
            Jpeg2BgrConverter conv;
            conv.Convert(jpg_.get(), jpgSize_);
            uint32_t len = conv.GetWidth()*conv.GetHeight() * 3;
            decompressedImg_.reset(new uint8_t[len], [](uint8_t*ptr) {
                delete[] ptr;
            });
            memcpy(decompressedImg_.get(), conv.GetImgBuffer(), len);
            mat_ = cv::Mat(conv.GetHeight(), conv.GetWidth(), CV_8UC3, (void*)decompressedImg_.get());
            jpg_.reset();
            compressed_ = false;
        }
        return mat_;
    }

private:
    cv::Mat mat_;
    shared_ptr<uint8_t> jpg_;
    uint32_t jpgSize_;
    shared_ptr<uint8_t> decompressedImg_;
    bool compressed_;
};


///factory
class BufferedFrameFactory {
public:
    static BufferedFrame * Create(uint32_t type) {
        switch (type) {
        case 0:
            return new OriginBufferedFrame();
        case 1:
            return new YUVBufferedFrame();
        case 2:
            return new JpegBufferedFrame();
        default:
            return new OriginBufferedFrame();
        }
    }

    static void Free(BufferedFrame *&bf) {
        if (nullptr != bf) {
            delete bf;
            bf = nullptr;
        }
    }
};

}