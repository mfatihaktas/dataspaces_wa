#!/bin/bash
echo $1 $2 $3

TRANS_PROTOCOL="i" # "i" # "g"
IB_LINTF="ib0"
TCP_LINTF="eth0" # "em2"
GFTP_LINTF="eth0" # "em2"
TMPFS_DIR=""

S_LIP=10.0.0.151 # 192.168.2.151 # 10.0.0.151
S_LPORT=1234

if [[ $1 == 'p' || $1 == 'dp' || $1 == 'g' || $1 == 'dg' ]]; then
  TYPE="put"
  GDB=""
  [[ $1 == 'g' || $1 == 'dg' ]] && TYPE="get"
  [[ $1  == 'dp' || $1  == 'dg' ]] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type=$TYPE --trans_protocol=$TRANS_PROTOCOL \
             --ib_lintf=$IB_LINTF --tcp_lintf=$TCP_LINTF \
             --s_lip=$S_LIP --s_lport=$S_LPORT \
             --gftp_lintf=$GFTP_LINTF --tmpfs_dir=$TMPFS_DIR
else
  echo "Argument did not match !"
fi