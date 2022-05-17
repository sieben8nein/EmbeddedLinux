#!/bin/bash
ping -q -c 1 -w 1 www.google.com >/dev/null 2>&1
ONLINE=$?
if [ $ONLINE -ne 0 ] 
then
ONLINE=0
else 
ONLINE=1
fi
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=hasInternet value=${ONLINE}"
