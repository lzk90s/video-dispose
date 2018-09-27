#------------ brpc ------------
FROM ubuntu:16.04 as brpc_builder

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && apt-get install -y \
      build-essential autoconf git pkg-config \
      automake libtool curl make g++ unzip cmake libssl-dev wget && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


WORKDIR ~

ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC

RUN cd ~ && \
	wget zlib.net/zlib-1.2.11.tar.gz && \
	tar xzf zlib*.tar.gz && \
	cd zlib-* && \ 
	./configure --static && \
	make -j$(nproc) && \
	make install

RUN cd ~ && \
	git clone -b v2.2.1 https://github.com/gflags/gflags.git && \
	cd gflags && \
	mkdir build && cd build && cmake .. && make  -j$(nproc) && make install

RUN   cd ~ && \
   	git clone -b v3.6.1 https://github.com/google/protobuf.git && \
	cd protobuf && \
	./autogen.sh && \
	./configure --disable-shared && \
	 make -j$(nproc) && \
	 make install

RUN  cd ~ && \
  	git clone https://github.com/google/leveldb.git && \
	cd  leveldb && \
	sed -i 's/3.9/3.2/g' CMakeLists.txt && \
	mkdir build && cd build && cmake .. && make  -j$(nproc) && make install

RUN  cd ~ && \
  	git clone -v https://github.com/brpc/brpc.git && \ 
	cd brpc && \
	mkdir build && cd build && cmake .. -DBUILD_SHARED_LIBS=0 -DBUILD_STATIC_LIBS=1 -DWITH_DEBUG_SYMBOLS=OFF && make  -j$(nproc) && make install



    
#------------ opencv ------------
FROM ubuntu:16.04 as opencv_builder
MAINTAINER Victor Melo <victorhcmelo@gmail.com>

# Built using PyImageSearch guide: 
# http://www.pyimagesearch.com/2015/06/22/install-opencv-3-0-and-python-2-7-on-ubuntu/

# Install dependencies
RUN  sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get -qq update && \
    apt-get -qq install -y \
        wget \
        unzip \
        libtbb2 \
        libtbb-dev && \
    apt-get -qq install -y \
        build-essential \ 
        cmake \
        git \
        pkg-config \
        libjpeg8-dev \
        libjasper-dev \
        libpng12-dev \
        libavcodec-dev \
        libavformat-dev \
        libswscale-dev \
        libv4l-dev \
        libatlas-base-dev \
        gfortran \
	libfreetype6 \
	libfreetype6-dev \
        libhdf5-dev && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


# Download OpenCV 3.2.0 and install
# step 10 
RUN \
    cd ~ && \
    wget https://github.com/Itseez/opencv/archive/3.2.0.zip && \
    unzip 3.2.0.zip && \
    mv ~/opencv-3.2.0/ ~/opencv/ && \
    rm -rf ~/3.2.0.zip && \
    #download opencv
    cd ~ && \
    wget https://github.com/opencv/opencv_contrib/archive/3.2.0.zip -O 3.2.0-contrib.zip && \
    unzip 3.2.0-contrib.zip && \
    mv opencv_contrib-3.2.0 opencv_contrib && \
    rm -rf ~/3.2.0-contrib.zip && \
    #configure opencv
    cd /root/opencv && \
    mkdir build && \
    cd build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D INSTALL_C_EXAMPLES=OFF \
        -D INSTALL_PYTHON_EXAMPLES=OFF \
        -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
        -D BUILD_EXAMPLES=OFF \
	-D BUILD_SHARED_LIBS=0 .. && \
    #build opencv
    cd ~/opencv/build && \
    make -j $(nproc) && \
    make install && \
    cp ~/opencv/build/3rdparty/lib/* /usr/local/lib && \
    cp ~/opencv/build/3rdparty/ippicv/ippicv_lnx/lib/intel64/* /usr/local/lib && \
    ldconfig  && \
    # clean opencv repos
    rm -rf ~/opencv/build && \
    rm -rf ~/opencv/3rdparty && \
    rm -rf ~/opencv/doc && \
    rm -rf ~/opencv/include && \
    rm -rf ~/opencv/platforms && \
    rm -rf ~/opencv/modules && \
    rm -rf ~/opencv_contrib/build && \
    rm -rf ~/opencv_contrib/doc


    
#------------ ffmpeg ------------
FROM nvidia/cuda:9.2-devel-ubuntu16.04 as FFmpeg_builder

WORKDIR /root

#编译x264
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y curl pkg-config git && \
    cd /root && \
    curl -O https://www.nasm.us/pub/nasm/releasebuilds/2.13.03/nasm-2.13.03.tar.gz && \
    tar -xzvf nasm-2.13.03.tar.gz && \
    cd /root/nasm-2.13.03 && \
    ./configure  --prefix=/usr &&\
    make -j$(nproc) && \
    make install && \
    cd /root && \
    git clone http://git.videolan.org/git/x264.git && \
    cd /root/x264 && \
    ./configure --enable-shared && \
    make -j$(nproc) && \
    make install

#编译FFmpeg    
RUN cd /root && \
    git clone -b n8.1.24.2 https://github.com/FFmpeg/nv-codec-headers.git && \
    cd /root/nv-codec-headers && \
    make install && \
    cd /root && \
    git clone -b n4.0.2 https://github.com/FFmpeg/FFmpeg.git && \
    cd /root/FFmpeg && \
    ./configure  --enable-gpl --enable-nvenc --enable-cuda --enable-cuvid --enable-nonfree --enable-libnpp --enable-libx264 --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 && \
    make -j$(nproc) && \
    make install 

#编译FFmpeg补丁
RUN cd /root && \
    git clone http://docker:docker123456@47.97.174.151:10080/service/video-dispose.git && \
    cd /root/video-dispose && \
    export FFMPEG_HOME='/root/FFmpeg' && \
    cd FFmpeg-patch && \
    sh patch_ffmpeg.sh && \
    cd /root/FFmpeg && \
    ./configure  --enable-gpl --enable-nvenc --enable-cuda --enable-cuvid --enable-nonfree --enable-libnpp --enable-libx264 --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 && \
    make -j$(nproc) && \
    make install 


#------------ 深瞐sdk ------------
FROM registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:v1.4.0.7 as seemmo_sdk

   
#------------ 高创sdk ------------

   
#------------ video_dispose ------------
FROM ubuntu:16.04 as video_dispose_builder

WORKDIR /root

#拷贝brpc
COPY --from=brpc_builder /usr/local/bin /usr/local/bin
COPY --from=brpc_builder /usr/local/lib /usr/local/lib
COPY --from=brpc_builder /usr/local/include /usr/local/include

#拷贝opencv
COPY --from=opencv_builder /usr/local/bin /usr/local/bin
COPY --from=opencv_builder /usr/local/lib /usr/local/lib
COPY --from=opencv_builder /usr/local/include /usr/local/include

#拷贝ffmpeg
COPY --from=FFmpeg_builder /usr/local/bin /usr/local/bin
COPY --from=FFmpeg_builder /usr/local/include /usr/local/include
COPY --from=FFmpeg_builder /usr/local/lib /usr/local/lib

#拷贝深瞐算法
COPY --from=seemmo_sdk /root/seemmo_sdk /root/seemmo_sdk

env SEEMMO_SDK_HOME /root/seemmo_sdk
env GOSUN_SDK_HOME /root/gosun_sdk
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC

#编译libjpeg
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update --fix-missing && \
    apt-get install -y build-essential git pkg-config make g++ cmake automake \
      libtool nasm vim libssl-dev libfreetype6 libfreetype6-dev && \
    apt-get autoclean && apt-get clean && \
    cd /root/ && \
    git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git && \
    cd /root/libjpeg-turbo && \
    mkdir build && cd build && cmake .. -DENABLE_SHARED=0 -DCMAKE_INSTALL_PREFIX=/usr/local && make install

#编译视频处理插件    
RUN rm -rf /usr/local/lib/libbrpc.so && \
    cd /root && \
    git clone http://docker:docker123456@47.97.174.151:10080/service/video-dispose.git
       
    