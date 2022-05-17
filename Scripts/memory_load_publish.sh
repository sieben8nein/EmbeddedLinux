#!/bin/bash
mem=$(free | awk '/^Mem/ {printf "%d",$3/$2*100}')
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=memory_load_percent value=${mem}"
