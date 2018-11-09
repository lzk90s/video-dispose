#!/bin/sh

all_pids=`ps -A |grep "ffmpeg"| awk '{print $1}'`

if [ -z ${all_pids} ]; then
	echo "no process"
else
	kill -9 ${all_pids}
fi
