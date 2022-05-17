#!/bin/bash
disk=$(df --output=pcent / | tr -dc '0-9')
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=disk_usage_percent value=${disk}"