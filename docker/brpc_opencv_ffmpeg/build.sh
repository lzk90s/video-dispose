#!/bin/bash

if [ ! -d package/brpc ]; then
    sh download.sh
fi

REGISTRY=registry.cn-hangzhou.aliyuncs.com
NAMESPACE=gosun
IMAGE=brpc_opencv_ffmpeg:latest

docker build -t ${REGISTRY}/${NAMESPACE}/${IMAGE} .
#docker push ${REGISTRY}/${NAMESPACE}/${IMAGE}