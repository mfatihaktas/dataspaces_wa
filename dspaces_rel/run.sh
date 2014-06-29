#!/bin/bash
echo $1

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server 1 --cnodes 2
elif [ $1  = 'e' ]; then
  GLOG_logtostderr=1 ./exp
elif [ $1  = 'load' ]; then
  module load openmpi-x86_64
elif [ $1  = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi