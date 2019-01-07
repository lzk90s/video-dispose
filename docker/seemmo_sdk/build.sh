#!/bin/bash

REGISTRY=registry.cn-hangzhou.aliyuncs.com
NAMESPACE=gosun
IMAGE=seemmo_sdk:latest

docker build -t ${REGISTRY}/${NAMESPACE}/${IMAGE} .
docker push ${REGISTRY}/${NAMESPACE}/${IMAGE}