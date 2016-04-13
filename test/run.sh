#!/bin/bash
echo $1 $2 $3

# To see why the process got killed
# dmesg | egrep -i -B100 'killed process'

NUM_PUTGET_THREADS=1
# DATA_SIZE=$((1024*1024))
# DATA_SIZE=$((100*1024*1024))
# DATA_SIZE=$((256))
DATA_SIZE=$((1024*1024*1024))

NUM_DSCNODES=2 # 96

if [[ $1  == 's' || $1  == 'ds' ]]; then
  [ -a conf ] && rm srv.lck conf dataspaces.conf
  GDB=
  [ $1  = 'ds' ] && GDB="gdb --args"
  
  echo "## Config file for DataSpaces
  ndim = 3
  dims = 1024,1024,1024
  max_versions = 1
  max_readers = 1
  lock_type = 1
  " > dataspaces.conf
  
  $GDB $DSPACES_DIR/bin/./dataspaces_server --server 1 --cnodes $NUM_DSCNODES
elif [[ $1  == 'p' || $1  == 'dp' || $1  == 'g' || $1  == 'dg' ]]; then
  TYPE=put_test
  APP_ID=1
  GDB=
  [ $1  = 'dp' ] && GDB="gdb --args"
  [[ $1  == 'g' || $1  == 'dg' ]] && { TYPE=get_test; APP_ID=2; }
  
  # export MALLOC_CHECK_=2
  # unset MALLOC_CHECK_
  export GLOG_logtostderr=1
  $GDB ./exp --type=$TYPE --app_id=$APP_ID --num_peer=1 \
             --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces/install
    echo "DSPACES_DIR= "$DSPACES_DIR
  fi
else
  echo "Argument did not match !"
fi
