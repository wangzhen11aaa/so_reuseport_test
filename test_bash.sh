#!/usr/bin/bash
argc=$#
if [ $argc -lt 1 ]; then
  echo "udp_client cnt"
  exit 1
fi
CNT=$1
i=0
while [ $i -lt $CNT ]; do
  i=$(($i+1))
  nohup ./udp_client 127.0.0.1 8000 10 2>&1 &
done
