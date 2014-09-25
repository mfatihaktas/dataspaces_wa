#!/bin/bash
echo $1

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

NUM_SNODES=1
NUM_DSCNODES=$((2+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
  #$DSPACES_BINDIR/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p1' ]; then
  GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'g1' ]; then
  GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rm1' ]; then
  GLOG_logtostderr=1 ./exp --type="ri" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000 \
                           --ib_lintf="ib0"
elif [ $1  = 'drm1' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000 \
                   --ib_lintf="ib0"
elif [ $1  = 'rm2' ]; then
  GLOG_logtostderr=1 ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf="em2" --lport=7000 \
                           --ib_lintf="ib0"
elif [ $1  = 'drm2' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=7000 \
                   --ib_lintf="ib0"
elif [ $1  = 'p2' ]; then
  GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'p22' ]; then
  GLOG_logtostderr=1 ./exp --type="put_2" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'g2' ]; then
  GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi