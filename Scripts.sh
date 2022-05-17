#!/bin/bash
sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE

#!/bin/bash
MyTemp="`vcgencmd measure_temp |tr -d "=temp'C"`"
curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=temperature value=${MyTemp}"


#!/bin/bash
vcgencmd measure_temp | grep -Eo '[+-]?[0-9]+([.][0-9]+)?'


#!/bin/bash
#hasInternet=ping -q -c1 google.com &>/dev/null && echo 1 || echo 0
hasInternet

curl -XPOST "localhost:8086/api/v2/write?bucket=status_info&precision=s" \
  --data-raw "temp,controller=pi,attribute=temperature value=${hasInternet}"

  #!/bin/bash
hasInternetping -q -c1 google.com &>/dev/null && echo 1 || echo 0


#!/bin/bash
echo "26" > /sys/class/gpio/export
while true; do
  echo "out" > /sys/class/gpio/gpio26/direction
  temp=$(sudo /home/pi/bin/cpu_temp.sh)
  wifi=$(sudo /home/pi/bin/has_internet.sh)
  echo $temp
  echo $wifi
  if [ "$temp" \> "100" ]
    then
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
  elif [ "$wifi" = "0" ]
    then
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
  else
    echo "1" > /sys/class/gpio/gpio26/value
    sleep 0.5
    echo "0" > /sys/class/gpio/gpio26/value
    sleep 0.5
  fi
  sleep 2
done
echo "26" > /sys/class/gpio/unexport


#!/bin/bash
echo "26" > /sys/class/gpio/export
while true; do
  echo "out" > /sys/class/gpio/gpio26/direction
  echo "1" > /sys/class/gpio/gpio26/value
  sleep 1
  echo "0" > /sys/class/gpio/gpio26/value
  sleep 1
done
echo "26" > /sys/class/gpio/unexport


#!/bin/bash


while true; do
  a=$(sudo /home/pi/bin/read_switch.sh);
  if [ "$a" = "1" ] 
  then
    sleep 1
    for i in {1..3}
    do
       b=$(sudo /home/pi/bin/read_switch.sh);
       if [ "$b" = "1" ]
       then
         sleep 1
         if [ "$i" = "3" ]
         then
           $(sudo poweroff)
         fi
       else
         break
       fi
    done
  fi
done


#!/bin/bash
#hasInternet=ping -q -c1 google.com &>/dev/null && echo 1 || echo 0
# hasInternet=$(ping -q -c 1 www.google.com | grep "1 received")


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
