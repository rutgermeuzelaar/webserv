#!/usr/bin/env bash

(./webserv conf/basic.conf > /dev/null)&
webserv_pid=$!
(siege -c 5 -t5S -f myurls.txt -i)&

wait $!
if ps -p $webserv_pid > /dev/null
then
    kill -s SIGINT $webserv_pid
fi
wait $webserv_pid
savedir="graphs"
timestamp=$(date +%Y%m%d_%H%M%S)
timestamp+=".svg"

if [ ! -d $savedir ]; then
    mkdir $savedir
fi

gprof webserv | gprof2dot | dot -Tsvg -o "${savedir}/${timestamp}"
echo "file saved: ${timestamp}"
