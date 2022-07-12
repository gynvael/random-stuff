#!/bin/bash

DISKS="/dev/sd?"

for i in $DISKS
do
  echo -n "$i: "
  res=`smartctl -n standby,123 -A $i`
  retval=$?
  if [ $retval -eq 123 ]
  then
    echo Zzz...
  else
    echo "$res" | grep Celsius | sed -e 's/^.* - *//'
  fi
done
