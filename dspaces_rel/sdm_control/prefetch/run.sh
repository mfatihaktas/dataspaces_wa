#!/bin/bash
echo $1 $2 $3

if [ $1  = 'm' ]; then
  GLOG_logtostderr=1 ./exp --type="markov"
elif [ $1  = 'dm' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="markov"
elif [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type="sfc"
elif [ $1  = 'ds' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="sfc"
elif [ $1  = 'vs' ]; then
  export GLOG_logtostderr=1
  valgrind -v --leak-check=yes ./exp --type="sfc"
else
  echo "Argument did not match !"
fi