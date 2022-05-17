#!/bin/bash

banned=$(sudo fail2ban-client status sshd | awk 'NR==7{ print; }' | grep -o '[[:digit:]]\+')
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=banned value=${banned}"
