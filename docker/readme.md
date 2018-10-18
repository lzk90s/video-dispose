# 镜像说明


## brpc_opencv_ffmpeg

brpc + opencv + ffmpeg 开发环境

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:latest .


## gosun_sdk

高创算法sdk库镜像，把算法sdk库放到sdk目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/gosun_sdk:latest .


## seemmo_sdk


深瞐算法sdk库镜像，把算法sdk库放到sdk目录下，执行命令

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/seemmo_sdk:latest .


## seemmo_server


深瞐算法服务，深瞐sdk一台机器上只能跑一个

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/seemmo-algo-server:latest -f docker/seemmo_server/Dockerfile .

生成深瞐机器码命令，生成后的机器码文件hdinfo.data在当前文件夹下

> docker run --rm --net host --privileged -v `pwd`:/tmp/hdinfo -it registry.cn-hangzhou.aliyuncs.com/gosun/seemmo-algo-server:latest  /root/seemmo_sdk/tool_hdinfo -p /tmp/hdinfo/hdinfo.data


## video-dispose

视频处理插件

> docker build -t registry.cn-hangzhou.aliyuncs.com/gosun/video-dispose:latest -f docker/video-dispose/Dockerfile .

