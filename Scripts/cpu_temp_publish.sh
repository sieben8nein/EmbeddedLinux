#!/bin/bash
temp="`vcgencmd measure_temp |tr -d "=temp'C"`"
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=temperature value=${temp}"
