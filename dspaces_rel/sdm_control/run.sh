#!/bin/bash
echo $1 $2 $3

LINTF="em2"
JOINHOST_LIP="192.168.2.151"
LPORT=6633

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type="server"
elif [ $1  = 'cl' ]; then
  GLOG_logtostderr=1 ./exp --type="client"
elif [ $1  = 'n' ]; then
  if [ $2  = 0 ]; then
    GLOG_logtostderr=1 ./exp --type="node" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    GLOG_logtostderr=1 ./exp --type="node" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                             --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'dn' ]; then
  export GLOG_logtostderr=1
  if [ $2  = 0 ]; then
    gdb --args ./exp --type="node" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2))
  else
    gdb --args ./exp --type="node" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                     --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'c' ]; then
  if [ $2  = 0 ]; then
    GLOG_logtostderr=1 ./exp --type="control" --id=$2 --node_type="m" --lintf=$LINTF --lport=$((LPORT+$2)) \
          #--joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  else
    GLOG_logtostderr=1 ./exp --type="control" --id=$2 --node_type="s" --lintf=$LINTF --lport=$((LPORT+$2)) \
                             --joinhost_lip=$JOINHOST_LIP --joinhost_lport=$LPORT
  fi
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # ENV VARIABLES FOR MAKE
    export GRIDFTP=GRIDFTP
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
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
  fi
else
  echo "Argument did not match !"
fi