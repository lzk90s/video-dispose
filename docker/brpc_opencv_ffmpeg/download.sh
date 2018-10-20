#!/bin/sh
mkdir -p package
cd package

wget http://raw.githubusercontent.com/opencv/opencv_3rdparty/ippicv/master_20180518/ippicv/ippicv_2017u3_lnx_intel64_general_20180518.tgz
rm -rf ippicv && mkdir ippicv && tar xvaf ippicv_*.tgz -C ippicv
git clone -v -b 3.4.3 http://github.com/opencv/opencv_contrib.git --depth 1
git clone -v -b 3.4.3 http://github.com/opencv/opencv.git --depth 1
git clone -v -b n4.0.2 http://github.com/FFmpeg/FFmpeg.git --depth 1
git clone -v -b n8.1.24.2 http://github.com/FFmpeg/nv-codec-headers.git
git clone -v http://github.com/libjpeg-turbo/libjpeg-turbo.git
git clone -v http://git.videolan.org/git/x264.git
git clone -v http://github.com/brpc/brpc.git --depth 1
git clone -v http://github.com/google/leveldb.git
git clone -v -b v3.2.0 http://github.com/google/protobuf.git
git clone -v -b v2.2.1 http://github.com/gflags/gflags.git
git clone -v https://github.com/lemenkov/libyuv.git

cd ..