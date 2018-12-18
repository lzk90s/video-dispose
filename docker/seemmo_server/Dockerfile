#------------ 算法sdk ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:latest as seemmo_sdk

#------------ 编译video-dispose ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:latest as base_builder

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC
ENV SEEMMO_SDK_HOME /root/seemmo_sdk
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${SEEMMO_SDK_HOME}/lib

#拷贝算法sdk
COPY --from=seemmo_sdk $SEEMMO_SDK_HOME $SEEMMO_SDK_HOME

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update --fix-missing && \
    apt-get install -y libssl-dev libfreetype6 libfreetype6-dev \
      libgomp1 locales && \
    apt-get autoclean && apt-get clean 

ADD . /root/video-dispose
   
RUN cd /root/video-dispose && \   
    make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_server install && \
    make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_wrapper install
    

#生成runtime    
FROM nvidia/cuda:9.2-runtime-ubuntu16.04 as cuda_runtime

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    ##cuda和nvidia-ml下载会失败，直接删除掉
    rm -rf /etc/apt/sources.list.d/* && \
    apt-get update  --fix-missing && \ 
    apt-get install --fix-missing -y --no-install-recommends \
      tzdata net-tools inetutils-ping libgomp1 locales  && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    ##修改时区
    ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone

ENV SEEMMO_SDK_HOME /root/seemmo_sdk
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${SEEMMO_SDK_HOME}/ubuntu/lib:/usr/local/lib
    
COPY --from=base_builder /root/seemmo_sdk /root/seemmo_sdk
COPY --from=base_builder /usr/local/bin/seemmo_algo_server /usr/local/bin/
COPY --from=base_builder /usr/local/lib/*.so /usr/local/lib/
    
CMD ["seemmo_algo_server"]