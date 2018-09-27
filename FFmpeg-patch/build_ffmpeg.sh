#!/bin/sh

MYDIR=${TOP_DIR}/FFmpeg-patch

if [ -z ${FFMPEG_HOME} ]; then
	echo "FFMPEG_HOME not set, please set FFMPEG_HOME at first!"
	exit 1
fi

echo "Copy filters source file"
cp -rf libavfilter/* ${FFMPEG_HOME}/libavfilter

# patch 
allfilters_c_file=${FFMPEG_HOME}/libavfilter/allfilters.c
fftool_makefile=${FFMPEG_HOME}/fftools/Makefile
already_exist=`grep ff_vf_algo ${allfilters_c_file} | wc -w`
if [ 0 -eq ${already_exist} ]; then
	echo "Patch allfilters.c"
	sed -i 's/extern AVFilter ff_af_abench;/extern AVFilter ff_af_abench;\nextern AVFilter ff_vf_algo;/g' ${allfilters_c_file}
	sed -i 's/vsink_nullsink.o/vsink_nullsink.o\nOBJS-$(CONFIG_ALGO_FILTER) += vf_algo.o\n/g' 
else
	echo "Alreay patch, skip"
fi
# patch success


cd ${FFMPEG_HOME}

echo "Configure ffmpeg"

./configure \
    --enable-static \
    --enable-gpl \
    --enable-nvenc \
    --enable-cuda \
    --enable-cuvid \
    --enable-nonfree \
    --enable-libnpp \
    --enable-libx264 \
    --extra-cflags=-I/usr/local/cuda/include \
    --extra-ldflags=-L/usr/local/cuda/lib64

echo "Build ffmpeg, please wait"

make clean && make -j$(nproc) && make install

echo "Build commplete"
