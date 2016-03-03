#!/bin/bash
echo $1 $2 $3

# To see why the process got killed
# dmesg | egrep -i -B100 'killed process'

NUM_PUTGET_THREADS=1
# DATA_SIZE=$((1024*1024))
DATA_SIZE=$((100*1024*1024))
# DATA_SIZE=$((256))
# DATA_SIZE=$((1024*1024*1024))

NUM_DSCNODES=2 # 96
NUM_PEER=1

if [[ $1  == 's' || $1  == 'ds' ]]; then
  # [ -a conf ] && rm srv.lck conf dataspaces.conf
  GDB=
  [ $1  = 'ds' ] && GDB="gdb --args"
  
  # echo "## Config file for DataSpaces
  # ndim = 2
  # dims = 128012,128012
  # max_versions = 1
  # max_readers = 1 
  # lock_type = 1
  # " > dataspaces.conf
  
  $GDB $DSPACES_DIR/bin/./dataspaces_server --server 2 --cnodes $NUM_DSCNODES
elif [[ $1  == 'p' || $1  == 'dp' ]]; then
  GDB=
  [ $1  = 'dp' ] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type=put_test --app_id=1 --num_peer=$NUM_PEER \
             --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [[ $1  == 'g' || $1  == 'dg' ]]; then
  GDB=
  [ $1  = 'dg' ] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type=get_test --app_id=2 --num_peer=$NUM_PEER \
             --num_putget_threads=$NUM_PUTGET_THREADS --data_size=$DATA_SIZE
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces/install
    echo "DSPACES_DIR= "$DSPACES_DIR
  fi
# elif [ $1  = 'tp' ]; then
#   NUM_PUTTER=64
#   $MPIRUN -n $NUM_PUTTER -host $NODE \
#     $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 20 1
# elif [ $1  = 'tg' ]; then
#   NUM_GETTER=32
#   $MPIRUN -n $NUM_GETTER -host $NODE \
#     $DSPACES_DIR/bin/test_writer DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 20 2
else
  echo "Argument did not match !"
fi

# ## Config file for DataSpaces
# # ndim = 3
# # dims = 12800,12800,12800
# ndim = 2
# dims = 12800,12800
# # ndim = 1
# # dims = 1280123123
# # 
# max_versions = 1
# max_readers = 100

# # Lock type: 1 - generic, 2 - custom
# lock_type = 1