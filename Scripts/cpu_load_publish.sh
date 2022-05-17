#!/bin/bash

cpu=$(top -bn1 | awk '/^%Cpu/ {printf "%d",$2}')
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=cpu_load_percent value=${cpu}"