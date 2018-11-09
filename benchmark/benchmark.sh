#!/bin/bash

. /etc/profile

export ENABLE_GOSUN_ALGO=0

run_ffmpeg_cmd(){
	local ip=$1
	local user=$2
	local password=$3
	local stream_id=$4

	nohup sh ${ffmpeg_cmd_file} ${ip} ${user} ${password} ${stream_id} &
}

get_latest_stream_id(){
	if [ -f ${stream_id_file} ]; then
		stream_id=`cat ${stream_id_file}`
	else
		stream_id=0
	fi
	stream_id=$((${stream_id}+1))
	echo ${stream_id} > ${stream_id_file}

	echo ${stream_id}
}

camcfg_file=cam.txt
stream_id_file=.stream_id
ffmpeg_cmd_file=ffmpeg_cmd.sh
stop_all_file=stop_all.sh

sh ${stop_all_file}
rm -rf ${stream_id_file}

while read line
do
	ip=`echo ${line} | cut -d' ' -f1`
	user=`echo ${line} | cut -d' ' -f2`
	password=`echo ${line} | cut -d' ' -f3`
	stream_id=$(get_latest_stream_id)

	run_ffmpeg_cmd ${ip} ${user} ${password} ${stream_id}
	sleep 1
done < ${camcfg_file}
