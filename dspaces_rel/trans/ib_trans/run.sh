#!/bin/bash
echo $1 $2 $3

S_LIP=192.168.210.161 # 10.0.0.151 #192.168.200.120
S_LPORT=1234

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type=server --s_lport=$S_LPORT
elif [ $1  = 'c' ]; then
  GLOG_logtostderr=1 ./exp --type=client --s_lport=$S_LPORT --s_lip=$S_LIP
elif [ $1  = 'rc' ]; then
  GLOG_logtostderr=1 ./exp --type=client --s_lport=$S_LPORT --s_lip=$S_LIP
elif [ $1  = 'bq' ]; then
  GLOG_logtostderr=1 ./exp --type=bqueue
elif [ $1  = 'ds' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type=server --s_lport=$S_LPORT
elif [ $1  = 'dc' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type=client --s_lport=$S_LPORT --s_lip=$S_LIP
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
    export ADIOS_DIR=/home/sc14demo/common-apps/adios-1.7.0/install
    export OPENCV_DIR=/home/sc14demo/common-apps/opencv/2.4.9/gcc4.8.3
    export MXML_DIR=/home/sc14demo/common-apps/mxml/2.7/gcc4.8.3
    export NSTXDEMO_DIR=/home/sc14demo/fusion/dataspaces_wa_nstx-sc14-demo
    
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/opencv/2.4.9/gcc4.8.3/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/mxml/2.7/gcc4.8.3/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/dataspaces_wa/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'm' ]; then
    # export MAQUIS=MAQUIS
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
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
    
    export PATH=/net/hj1/ihpcl/bin:$PATH
    echo "PATH="
    echo $PATH
  fi
else
  echo "Argument did not match !"
fi