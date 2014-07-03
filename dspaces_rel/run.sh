#!/bin/bash
echo $1

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

NUM_SNODES=1
NUM_CNODES=2

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_CNODES
elif [ $1  = 'ep' ]; then
  GLOG_logtostderr=1 ./exp --type=put --num_cnodes=$NUM_CNODES --app_id=2
elif [ $1  = 'eg' ]; then
  GLOG_logtostderr=1 ./exp --type=get --num_cnodes=$NUM_CNODES --app_id=3
  #--type=get --num_cnodes=2 --app_id=3
elif [ $1  = 'load' ]; then
  module load openmpi-x86_64
elif [ $1  = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi