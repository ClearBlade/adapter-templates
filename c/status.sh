#!/bin/bash
if [ "$EUID" -ne 0 ]
  then 
  	echo "---------Permissions Error---------"
  	echo "STOPPING: Please run as root or sudo"
  	echo "-----------------------------------"
  exit
fi

cmd="pgrep adapter"
check=$(eval $cmd)

if [ "$check" != "" ]; then
    STATUS="Running"
else
    STATUS="Stopped"
fi

echo $STATUS