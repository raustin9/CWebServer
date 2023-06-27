#!bin/bash

echo Port to Kill:
read PORT
echo Killing Server...
netstat_res=`netstat -lntp | grep ":"$PORT`

server_pid=`echo "$netstat_res" | sed -r 's/.*LISTEN//' | sed -r 's/[^0-9]+//g'`

kill -9 $server_pid
