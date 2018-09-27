#!/bin/bash

docker build -f Dockerfile-brpc -t gosun/brpc:latest .

docker build -f Dockerfile-opencv_dev -t gosun/opencv:latest .

docker build -f Dockerfile-ffmpeg_dev -t gosun/ffmpeg:latest .

docker build -f Dockerfile-video_dispose_dev -t gosun/video_dispose:latest .

docker build -f Dockerfile-brpc -t gosun/brpc:latest .