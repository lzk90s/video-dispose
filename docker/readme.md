# 镜像说明

## brpc_opencv_ffmpeg

brpc + opencv + ffmpeg 开发环境

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:latest .


## gosun_sdk

高创算法sdk库镜像，当sdk更新后，把Dockerfile-gosun_sdk放到高创算法根目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/gosun_sdk:latest .


## seemmo_sdk


深瞐算法sdk库镜像，当sdk更新后，把Dockerfile-seemmo_sdk放到深瞐算法根目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:latest .


## video-dispose

视频处理插件

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/video-dispose:latest -f docker/video-dispose/Dockerfile .

