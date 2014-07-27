#!/bin/bash
echo $1

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

NUM_SNODES=1
NUM_DSCNODES=$((1+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
elif [ $1  = 'ep' ]; then
  GLOG_logtostderr=1 ./exp --type=put --dht_id='p' --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'eg' ]; then
  GLOG_logtostderr=1 ./exp --type=get --dht_id='g' --num_dscnodes=$NUM_DSCNODES --app_id=2
  #--type=get --num_DScnodes=2 --app_id=3
elif [ $1  = 'er' ]; then
  GLOG_logtostderr=1 ./exp --type=ri --dht_id='r' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf=lo --lport=7000
elif [ $1  = 'load' ]; then
  module load openmpi-x86_64
elif [ $1  = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi