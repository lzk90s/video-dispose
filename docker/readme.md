# 镜像说明

## Dockerfile-brpc_opencv_ffmpeg

brpc + opencv + ffmpeg 开发环境


## Dockerfile-gosun_sdk

保存高创算法sdk库镜像，当sdk更新后，把Dockerfile-gosun_sdk放到高创算法根目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/gosun_sdk:latest .
> docker push registry.cn-hangzhou.aliyuncs.com/gosun/gosun_sdk:latest



## Dockerfile-seemmo_sdk


保存深瞐算法sdk库镜像，当sdk更新后，把Dockerfile-seemmo_sdk放到深瞐算法根目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:latest .
> docker push registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:latest
