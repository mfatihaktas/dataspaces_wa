#!/bin/bash
echo $1 $2 $3

S_LIP=10.0.0.151 # 192.168.210.161 #192.168.200.120
S_LPORT=1234

if [[ $1  = 's' || $1  = 'ds' || $1  = 'c' || $1  = 'dc' ]]; then
  TYPE="server"
  GDB=""
  [[ $1 == 'c' || $1 == 'dc' ]] && TYPE="client"
  [[ $1  == 'ds' || $1  == 'dc' ]] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type=$TYPE --s_lport=$S_LPORT --s_lip=$S_LIP
elif [ $1  = 'bq' ]; then
  GLOG_logtostderr=1 ./exp --type=bqueue
else
  echo "Argument did not match !"
fi