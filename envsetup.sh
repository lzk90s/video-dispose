#!/bin/sh

TOP_DIR=`pwd`
#export SEEMMO_SDK_HOME=${TOP_DIR}/../seemideo_sdk/
#export OPENCV_HOME=${SEEMMO_SDK_HOME}/demo/thirdlib/opencv/opencv-2.4.13/
#export FFMPEG_HOME=${TOP_DIR}/../FFmpeg/FFmpeg

export LD_LIBRARY_PATH=/usr/local/lib:${SEEMMO_SDK_HOME}/lib:${GOSUN_SDK_HOME}/lib:${TOP_DIR}/pkg
export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/usr/local/lib/pkg-config
