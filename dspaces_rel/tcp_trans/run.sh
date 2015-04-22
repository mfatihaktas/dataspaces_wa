#!/bin/bash
echo $1 $2 $3

S_LIP=192.168.2.151 #10.0.0.151
LPORT=1234

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type=server --lintf=em2 --lport=$LPORT
elif [ $1  = 'c' ]; then
  GLOG_logtostderr=1 ./exp --type=client --s_lip=$S_LIP --lport=$LPORT
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
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
    export ADIOS_DIR=/cac/u01/mfa51/Desktop/adios-1.7.0/install
    export OPENCV_DIR=/cac/u01/mfa51/Desktop/opencv/install
    export MXML_DIR=/cac/u01/mfa51/Desktop/mxml-2.7/install
    export NSTXDEMO_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa_nstx-sc14-demo
  
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/opencv/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mxml-2.7/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/dataspaces_wa/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  fi
else
  echo "Argument did not match !"
fi