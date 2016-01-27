#!/bin/bash
echo $1 $2 $3

LINTF="ib0" #"em2"
JOINHOST_LIP="192.168.210.100" #"192.168.2.152"
LPORT=6633

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type="server"
elif [ $1  = 'cl' ]; then
  GLOG_logtostderr=1 ./exp --type="client"
elif [ $1  = 'n' ]; then
  if [ $2  = 0 ]; then
    GLOG_logtostderr=1 ./exp --type="node" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    GLOG_logtostderr=1 ./exp --type="node" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                             --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'dn' ]; then
  export GLOG_logtostderr=1
  if [ $2  = 0 ]; then
    gdb --args ./exp --type="node" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    gdb --args ./exp --type="node" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                     --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'c' ]; then
  if [ $2  = 0 ]; then
    GLOG_logtostderr=1 ./exp --type="control" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    GLOG_logtostderr=1 ./exp --type="control" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                             --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'dc' ]; then
  export GLOG_logtostderr=1
  if [ $2  = 0 ]; then
    gdb --args ./exp --type="control" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    gdb --args ./exp --type="control" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                     --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
else
  echo "Argument did not match !"
fi