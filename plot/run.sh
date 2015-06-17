#!/bin/bash
echo $1 $2 $3

if [ $1  = 'e' ]; then
  GLOG_logtostderr=1 ./exp
elif [ $1  = 'de' ]; then
  GLOG_logtostderr=1
  gdb --args ./exp
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