#!/bin/bash

. /etc/profile

export ENABLE_GOSUN_ALGO=0

run_ffmpeg_cmd(){
	local ip=$1
	local user=$2
	local password=$3
	local stream_id=$4

	nohup sh ${ffmpeg_cmd_file} ${ip} ${user} ${password} ${stream_id} > tmp/${stream_id} &
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
max_num=100

if [ $# -eq 1 ]; then
	max_num=$1
fi

sh ${stop_all_file}
rm -rf ${stream_id_file}
rm -rf tmp && mkdir tmp

count=0
while read line
do
	str=`echo ${line} | grep -v "^#"`
	if [ -z "${str}" ]; then
		continue
	fi

	count=$((${count}+1))
	if [ ${count} -gt ${max_num} ]; then
		break
	fi

	ip=`echo ${str} | cut -d' ' -f1`
	user=`echo ${str} | cut -d' ' -f2`
	password=`echo ${str} | cut -d' ' -f3`
	stream_id=$(get_latest_stream_id)

	echo "process line ${str}"
	run_ffmpeg_cmd ${ip} ${user} ${password} ${stream_id}
	sleep 1
done < ${camcfg_file}
