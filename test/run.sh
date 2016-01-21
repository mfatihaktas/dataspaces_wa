#!/bin/bash
echo $1 $2 $3

# DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install/bin
DSPACES_BINDIR=$DSPACES_DIR/bin
NUM_PUTGET_THREADS=1
# DATA_SIZE=$((1024*1024))
DATA_SIZE=$((100*1024*1024))
# DATA_SIZE=$((12*1000))
# DATA_SIZE=$((1000*1000)) # 0

NUM_SNODES=1
NUM_DSCNODES=2

if [ $1  = 's' ]; then
  #if [ -a conf ]; then
  #  rm srv.lck
  #  rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  #fi
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
elif [ $1  = 'ds' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  gdb --args $DSPACES_BINDIR/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p' ]; then
  GLOG_logtostderr=1 ./exp --type=put_test --app_id=1 --num_dscnodes=$NUM_DSCNODES \
                           --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'dp' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type=put_test --app_id=1 --num_dscnodes=$NUM_DSCNODES \
                   --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'g' ]; then
  GLOG_logtostderr=1 ./exp --type=get_test --app_id=2 --num_dscnodes=$NUM_DSCNODES \
                           --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'dg' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type=get_test --app_id=2 --num_dscnodes=$NUM_DSCNODES \
                   --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces/install
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install
    echo "DSPACES_DIR= "$DSPACES_DIR
  fi
else
  echo "Argument did not match !"
fi
