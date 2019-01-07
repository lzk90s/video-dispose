FROM nvidia/cuda:9.2-devel-ubuntu16.04

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC

RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    rm -rf /etc/apt/sources.list.d/* && \
    apt-get update --fix-missing && \
    apt-get install -y \
      curl pkg-config git build-essential make g++ cmake automake libtool \
      wget unzip libtbb2 libtbb-dev build-essential cmake git pkg-config \
      libjpeg8-dev libjasper-dev libpng12-dev libavcodec-dev libavformat-dev \
      libswscale-dev libv4l-dev libatlas-base-dev gfortran libfreetype6 libfreetype6-dev \
      libhdf5-dev  libssl-dev libgoogle-glog-dev &&\
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
    


#编译opencv
ADD package/opencv opencv
ADD package/opencv_contrib opencv_contrib
ADD package/ippicv opencv/3rdparty/ippicv
RUN \
    cd /root/opencv && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=RELEASE \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DINSTALL_C_EXAMPLES=OFF \
        -DINSTALL_PYTHON_EXAMPLES=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_SHARED_LIBS=0 \
        -DIPPROOT=~opencv/3rdparty/ippicv/ .. && \
    #build opencv
    cd ~/opencv/build && \
    make -j $(nproc) && \
    make install && \
    cp ~/opencv/build/3rdparty/lib/* /usr/local/lib && \
    cp ~/opencv/3rdparty/ippicv/ippicv_lnx/lib/intel64/* /usr/local/lib && \
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

#编译zlib    
RUN \
    cd ~ && \
	wget zlib.net/zlib-1.2.11.tar.gz && \
	tar xzf zlib*.tar.gz && \
	cd zlib-* && \ 
	./configure --static && \
	make -j$(nproc) && \
	make install

    
#编译gflags    
ADD package/gflags gflags
RUN \
    cd /root/gflags && \
	mkdir build && cd build && cmake .. && make  -j$(nproc) && make install
 
    
#编译protobuf    
ADD package/protobuf protobuf   
RUN \
    cd /root/protobuf && \
	./autogen.sh && \
	./configure --disable-shared && \
	 make -j$(nproc) && \
	 make install

#编译leveldb     
ADD package/leveldb leveldb   
RUN \
	cd /root/leveldb && \
	sed -i 's/3.9/3.2/g' CMakeLists.txt && \
	mkdir build && cd build && cmake .. && make  -j$(nproc) && make install

#编译brpc
ADD package/brpc brpc   
RUN \
	cd /root/brpc && \
	mkdir build && cd build && cmake .. -DBUILD_SHARED_LIBS=0 -DBUILD_STATIC_LIBS=1 -DWITH_DEBUG_SYMBOLS=OFF && make  -j$(nproc) && make install && \
    rm -rf /usr/local/lib/libbrpc.so


#编译x264   
 ADD package/x264 x264 
RUN cd ~  && \
    curl -O https://www.nasm.us/pub/nasm/releasebuilds/2.13.03/nasm-2.13.03.tar.gz && \
    tar -xzvf nasm-2.13.03.tar.gz && \
    cd /root/nasm-2.13.03 && \
    ./configure  --prefix=/usr &&\
    make -j$(nproc) && \
    make install && \
    cd /root/x264 && \
    ./configure --enable-static && \
    make -j$(nproc) && \
    make install

#编译libjpeg-turbo
ADD package/libjpeg-turbo libjpeg-turbo    
RUN \
    cd /root/libjpeg-turbo && \
    mkdir build && cd build && cmake .. -DENABLE_SHARED=0 -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make install

#编译libyuv
ADD package/libyuv libyuv   
RUN \
    cd /root/libyuv && \
    mkdir build && cd build  && cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install && \
    rm -rf /usr/local/lib/libyuv.so*
    
#编译FFmpeg
ADD package/nv-codec-headers nv-codec-headers
ADD package/FFmpeg FFmpeg          
RUN \
    cd /root/nv-codec-headers && \
    make install && \
    cd /root/FFmpeg && \
    ./configure --enable-static --enable-gpl --enable-nvenc --enable-cuda --enable-cuvid --enable-nonfree --enable-libnpp --enable-libx264 --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 && \
    make -j$(nproc) && \
    make install 
    
