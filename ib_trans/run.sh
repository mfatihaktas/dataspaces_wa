#!/bin/bash
echo $1

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type=server --port=1234
elif [ $1  = 'c' ]; then
  GLOG_logtostderr=1 ./exp --type=client --port=1234 --s_addr=127.0.0.1
elif [ $1  = 'ds' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type=server --port=1234
else
  echo "Argument did not match !"
fi