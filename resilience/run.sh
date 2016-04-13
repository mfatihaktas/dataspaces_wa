#!/bin/bash
echo $1 $2 $3

if [[ $1 == 'x' || $1 == 'dx' ]]; then
  GDB=""
  [ $1 = 'dx' ] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type="xor"
else
  echo "Argument did not match!"
fi