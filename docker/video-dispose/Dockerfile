#------------ 算法sdk ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/gosun_sdk:latest as gosun_sdk

#------------ 编译video-dispose ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:latest as base_builder

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC
ENV GOSUN_SDK_HOME /root/gosun_sdk
ENV FFMPEG_HOME /root/FFmpeg
ENV LC_ALL en_US.UTF-8
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GOSUN_SDK_HOME}/lib

#拷贝算法sdk
COPY --from=gosun_sdk $GOSUN_SDK_HOME $GOSUN_SDK_HOME

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update --fix-missing && \
    apt-get install -y libssl-dev libfreetype6 libfreetype6-dev \
      locales libjasper1 libjasper-dev libdc1394-22 libavcodec-ffmpeg56 \
      libswscale-ffmpeg3 libavformat-ffmpeg56 libcairo2 libgdk-pixbuf2.0-0 \
      libgtk2.0-0 libopenexr22 libopenblas-base libopenblas-dev uuid-dev \
      libmpich12  libgoogle-glog-dev   liblmdb0 libleveldb1v5 libopenmpi1.10 && \
    apt-get autoclean && apt-get clean && \
    locale-gen $LC_ALL

ADD . /root/video-dispose
    
RUN cd /root/video-dispose && \
    cd FFmpeg-patch && \
    sh patch_ffmpeg.sh && \
    ##打补丁后重新编译ffmpeg
    cd /root/FFmpeg && \
    ./configure --enable-static --enable-gpl --enable-nvenc --enable-cuda --enable-cuvid --enable-nonfree --enable-libnpp --enable-libx264 --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 && \
    make -j$(nproc) && \
    make install && \
    ##编译video-dispose插件
    cd /root/video-dispose && \
    make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_stub install && \
    make -C algo/vendor/gosun -j$(nproc) -B -f Makefile_stub install && \
    make -C vfilter -j$(nproc) -B install && \
    make -C vfilter install_font
    

#------------ 生成runtime包 ------------
FROM nvidia/cuda:9.2-runtime-ubuntu16.04 as cuda_runtime

ENV GOSUN_SDK_HOME /root/gosun_sdk
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64
ENV PATH $PATH:${JAVA_HOME}/jre/bin:${JAVA_HOME}/bin
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GOSUN_SDK_HOME}/lib
ENV LC_ALL en_US.UTF-8

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    ##cuda和nvidia-ml下载会失败，直接删除掉
    rm -rf /etc/apt/sources.list.d/* && \
    apt-get update  --fix-missing && \ 
    apt-get install --fix-missing -y --no-install-recommends openjdk-8-jdk libfreetype6 \
      tzdata net-tools inetutils-ping libgomp1 locales libjasper1 libjasper-dev \ 
      libdc1394-22 libavcodec-ffmpeg56 libswscale-ffmpeg3 libavformat-ffmpeg56 \
      libcairo2 libgdk-pixbuf2.0-0 libgtk2.0-0 libopenexr22 uuid-dev \
      libopenblas-base libopenblas-dev libgflags-dev \
      libmpich12  libgoogle-glog-dev   liblmdb0 libleveldb1v5 libopenmpi1.10 && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    ##修改时区
    ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone && \
    ##设置locale为en_US.UTF8
    locale-gen $LC_ALL
    
COPY --from=base_builder /root/gosun_sdk /root/gosun_sdk
COPY --from=base_builder /usr/local/bin/ /usr/local/bin/
COPY --from=base_builder /usr/local/lib/*.so /usr/local/lib/
COPY --from=base_builder /usr/share/fonts/truetype /usr/share/fonts/truetype
