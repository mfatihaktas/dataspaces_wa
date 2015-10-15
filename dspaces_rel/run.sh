#!/bin/bash
echo $1 $2 $3

NUM_SNODE=1
NUM_CLIENT=1
NUM_DSCNODE=$(($NUM_CLIENT+1)) #+1: RIManager

CONTROL_LINTF="lo" # "em2"
RI_MANAGER_CONTROL_LPORT_LIST=( "7000" "7001" "7002" )
RI_MANAGER_JOIN_CONTROL_LADDR_LIST=( "" "192.168.2.151" "192.168.2.151" )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( "" "7000" "7000" )

TRANS_PROTOCOL="i" # "g"
IB_LINTF="ib0"
TCP_LINTF="em2"
TCP_LPORT="6000"
GFTP_LINTF="em2"
GFTP_LPORT="6000"
TMPFS_DIR=$DSPACESWA_DIR/tmpfs

if [ $1  = 's' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  $DSPACES_DIR/bin/./dataspaces_server --server $NUM_SNODE --cnodes $NUM_DSCNODE
elif [ $1  = 'p' ]; then
  GLOG_logtostderr=1 ./exp --type="put" --cl_id=1 --num_client=$NUM_CLIENT --ds_id='p'
elif [ $1  = 'g' ]; then
  GLOG_logtostderr=1 ./exp --type="get" --cl_id=2 --num_client=$NUM_CLIENT --ds_id='g'
elif [ $1  = 'r' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./exp --type="ri" --cl_id=21 --num_client=$NUM_CLIENT --base_client_id=$(($(($2+1))*10)) \
                             --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                             --join_control_laddr=${RI_MANAGER_JOIN_CONTROL_LADDR_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                             --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                             --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                             --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
elif [ $1  = 'load' ]; then
  module load openmpi-x86_64
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
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
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
    export DSPACES_DIR=/home/sc14demo/common-apps/dataspaces-1.5.0/install
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
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.5.0/install
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