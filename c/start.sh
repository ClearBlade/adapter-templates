#!/bin/bash
if [ "$EUID" -ne 0 ]
  then 
  	echo "---------Permissions Error---------"
  	echo "STOPPING: Please run as root or sudo"
  	echo "-----------------------------------"
  exit
fi

./adapter -systemKey=928cddb30bacace5aa9df4c8cd6b -systemSecret=928CDDB30BE0E3818FC0A2DCFD44 -deviceID=pythonAdapter -deviceActiveKey=85RFUeSlJXP952L2mT21659t3Y20 -logLevel=debug