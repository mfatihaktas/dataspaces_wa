#!/bin/bash
echo $1

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

NUM_SNODES=1
NUM_DSCNODES=$((1+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
  #$DSPACES_BINDIR/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'lp1' ]; then
  GLOG_logtostderr=1 ./exp --type="l_put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rp1' ]; then
  GLOG_logtostderr=1 ./exp --type="r_put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rg1' ]; then
  GLOG_logtostderr=1 ./exp --type="r_get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  #--type=get --num_DScnodes=2 --app_id=3
elif [ $1  = 'rm1' ]; then
  GLOG_logtostderr=1 ./exp --type="ri_t" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000
elif [ $1  = 'drm1' ]; then
  gdb --args ./exp --type="ri_t" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000
elif [ $1  = 'rm2' ]; then
  GLOG_logtostderr=1 ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf="em2" --lport=7000
elif [ $1  = 'drm2' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=7000
elif [ $1  = 'lp2' ]; then
  GLOG_logtostderr=1 ./exp --type="l_put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rg2' ]; then
  GLOG_logtostderr=1 ./exp --type="r_get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'drg2' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="r_get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi