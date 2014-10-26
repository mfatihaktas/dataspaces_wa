#!/bin/bash
echo $1

DSPACES_BIN=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

BOOST_LIB=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib

NUM_SNODES=1
NUM_DSCNODES=$((1+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BIN/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
  #$DSPACES_BIN/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p1' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \ 
    ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'g1' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \ 
    ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rm1' ]; then
  GLOG_logtostderr=1 ./exp --type="ri" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                           --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000 \
                           --ib_lintf="ib0"
elif [ $1  = 'drm1' ]; then
  export GLOG_logtostderr=1
  export LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH
  gdb --args ./exp --type="ri" --dht_id='1' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=8000 --ipeer_lip="192.168.2.152" --ipeer_lport=7000 \
                   --ib_lintf="ib0"
elif [ $1  = 'rm2' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \
    ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                      --lintf="em2" --lport=7000 \
                      --ib_lintf="ib0"
elif [ $1  = 'drm2' ]; then
  export GLOG_logtostderr=1 
  export LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH
  gdb --args ./exp --type="ri" --dht_id='2' --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                   --lintf="em2" --lport=7000 \
                   --ib_lintf="ib0"
elif [ $1  = 'p2' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \
    ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'p22' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \ 
    ./exp --type="put_2" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'g2' ]; then
  GLOG_logtostderr=1 LD_LIBRARY_PATH=$BOOST_LIB:$LD_LIBRARY_PATH \
    ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'show' ]; then
  netstat -antu
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # ENV VARIABLES FOR MAKE
    export CC=/opt/gcc-4.8.2/bin/gcc
    export CPP=/opt/gcc-4.8.2/bin/g++
    export MPICPP=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    export GLOG_DIR=/cac/u01/mfa51/Desktop/glog-0.3.3/install
    export BOOST_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0/install
    export BOOSTASIO_DIR=/opt/matlab/R2013a/bin/glnxa64
    export DATASPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
    export DSPACES_WA_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
  
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  elif [ $1  = 'u' ]; then
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  fi
else
  echo "Argument did not match !"
fi