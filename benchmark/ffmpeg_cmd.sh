#!/bin/sh

ip=$1
user=$2
password=$3
stream_id=$4
nms_host=172.18.18.158

echo "ip=${ip}"
echo "user=${user}"
echo "password=${password}"
echo "stream_id=${stream_id}"

ffmpeg  -rtsp_transport tcp -r 25 -c:v h264_cuvid -i rtsp://${user}:${password}@${ip} -an -threads 4 -vf "[in]scale=1920*1080[tmp];[tmp]algo=${stream_id}[out]"  -c:v libx264  -preset ultrafast  -b:v 4000k -minrate 2000k -maxrate 5000k -bf 1 -r 25 -g 50 -f flv rtmp://${nms_host}/live/${stream_id} 
