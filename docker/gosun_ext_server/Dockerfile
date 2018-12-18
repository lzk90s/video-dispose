#------------ 算法sdk ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/gosun_ext_sdk:latest as gosun_ext_sdk

#------------ 编译video-dispose ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:latest as base_builder

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC
ENV GOSUN_EXT_SDK_HOME /opt/GsFaceLib
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GOSUN_EXT_SDK_HOME}/face_libs

#拷贝算法sdk
COPY --from=gosun_ext_sdk $GOSUN_EXT_SDK_HOME $GOSUN_EXT_SDK_HOME

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update --fix-missing && \
    apt-get install -y libssl-dev libfreetype6 libfreetype6-dev \
      libgomp1 locales && \
    apt-get autoclean && apt-get clean 

ADD . /root/video-dispose
  
RUN cd /root/video-dispose && \   
    make -C algo/vendor/gosun_ext -j$(nproc) -B -f Makefile_server install && \
    make -C algo/vendor/gosun_ext -j$(nproc) -B -f Makefile_wrapper install

    
# -------------------- runtime --------------------
FROM nvidia/cuda:9.2-runtime-ubuntu16.04 as cuda_runtime

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    ##cuda和nvidia-ml下载会失败，直接删除掉
    rm -rf /etc/apt/sources.list.d/* && \
    apt-get update  --fix-missing && \ 
    apt-get install --fix-missing -y --no-install-recommends \
      tzdata net-tools inetutils-ping locales \
      libdc1394-22 libavcodec-ffmpeg56 libswscale-ffmpeg3 libavformat-ffmpeg56 \
      libcairo2 libgdk-pixbuf2.0-0 libgtk2.0-0 libopenexr22 uuid-dev \
      libopenblas-base libopenblas-dev libgflags-dev libjasper1 \
      libmpich12  libgoogle-glog-dev   liblmdb0 libleveldb1v5 libopenmpi1.10 && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    ##修改时区
    ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone

ENV GOSUN_EXT_SDK_HOME /opt/GsFaceLib
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${GOSUN_EXT_SDK_HOME}/face_libs:/usr/local/lib/
    
COPY --from=base_builder ${GOSUN_EXT_SDK_HOME} ${GOSUN_EXT_SDK_HOME}
COPY --from=base_builder /usr/local/bin/gosun_ext_algo_server /usr/local/bin/
COPY --from=base_builder /usr/local/lib/*.so /usr/local/lib/

CMD ["gosun_ext_algo_server"]