#!/bin/bash
echo $1 $2 $3

DHT_LINTF="em2"
IB_INTF="ib0"

RM1_DHT_LPORT=8000
RM2_DHT_LPORT=7000
RM2_DHT_LIP="192.168.2.152"

GFTP_LPORT=10000
TMPFS_DIR="/dev/shm"

# DSPACES_BIN=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install/bin
DSPACES_BIN=$DSPACES_DIR/bin

NUM_SNODES=1
NUM_DSCNODES=$((1+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BIN/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
  #$DSPACES_BIN/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p' ]; then
  if [ $2  = '1' ]; then
    GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
  elif [ $2  = '2' ]; then
    GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
  elif [ $2  = '22' ]; then
    GLOG_logtostderr=1 ./exp --type="put_2" --num_dscnodes=$NUM_DSCNODES --app_id=1
  fi
elif [ $1  = 'g' ]; then
  if [ $2  = '1' ]; then
    GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  elif [ $2  = '2' ]; then
    GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  fi
elif [ $1  = 'rm' ]; then
  if [ $2  = '1' ]; then
    GLOG_logtostderr=1 ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                             --dht_lintf=$DHT_LINTF --dht_lport=$RM1_DHT_LPORT --ipeer_dht_lip=$RM2_DHT_LIP --ipeer_dht_lport=$RM2_DHT_LPORT \
                             --trans_protocol="i" --ib_lintf=$IB_INTF \
                             --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  elif [ $2  = '2' ]; then
    GLOG_logtostderr=  ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                             --dht_lintf=$DHT_LINTF --dht_lport=$RM2_DHT_LPORT \
                             --trans_protocol="i" --ib_lintf=$IB_INTF \
                             --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
elif [ $1  = 'drm' ]; then
  if [ $2  = '1' ]; then
    export GLOG_logtostderr=1
    gdb --args ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                     --lintf=$DHT_LINTF --dht_lport=$RM1_DHT_LPORT --ipeer_dht_lip=$RM2_DHT_LIP --ipeer_dht_lport=$RM2_DHT_LPORT \
                     --ib_lintf=$IB_INTF
  elif [ $2  = '2' ]; then
    export GLOG_logtostderr=1 
    gdb --args ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                     --lintf=$DHT_LINTF --dht_lport=$RM2_DHT_LPORT \
                     --ib_lintf=$IB_INTF
  fi
elif [ $1  = 'show' ]; then
  netstat -antu
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # ENV VARIABLES FOR MAKE
    export CC=/opt/gcc-4.8.2/bin/gcc
    export CPP=/opt/gcc-4.8.2/bin/g++
    export MPICPP=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    export MPI_DIR=/cac/u01/mfa51/Desktop/mpich-3.1.2/install
    export GLOG_DIR=/cac/u01/mfa51/Desktop/glog-0.3.3/install
    export BOOST_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install
    export DSPACESWA_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
  
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'u' ]; then
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export MPI_DIR=/opt/intel/impi/4.1.3.048/intel64
    export GLOG_DIR=/home/sc14demo/common-apps/glog-0.3.3/install
    export BOOST_DIR=/home/sc14demo/common-apps/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/home/sc14demo/common-apps/dataspaces-1.4.0/install
    export DSPACESWA_DIR=/home/sc14demo/common-apps/dataspaces_wa
    
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'm' ]; then
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export GLOG_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install
    export BOOST_DIR=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.4.0/install
    export DSPACESWA_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa
    
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  fi
else
  echo "Argument did not match !"
fi