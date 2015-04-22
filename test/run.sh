#!/bin/bash
echo $1 $2 $3

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install/bin
NUM_PUTGET_THREADS=2

NUM_SNODES=1
NUM_DSCNODES=2

if [ $1  = 's' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
elif [ $1  = 'ds' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  gdb --args $DSPACES_BINDIR/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p' ]; then
  ./exp --type=put_test --app_id=1 --num_dscnodes=$NUM_DSCNODES --num_putget_threads=$NUM_PUTGET_THREADS
  # gdb --args ./exp --type=put_test --app_id=1 --num_dscnodes=$NUM_DSCNODES --num_putget_threads=$NUM_PUTGET_THREADS
elif [ $1  = 'g' ]; then
  ./exp --type=get_test --app_id=2 --num_dscnodes=$NUM_DSCNODES --num_putget_threads=$NUM_PUTGET_THREADS
  # gdb --args ./exp --type=get_test --app_id=2 --num_dscnodes=$NUM_DSCNODES --num_putget_threads=$NUM_PUTGET_THREADS
else
  echo "Argument did not match !"
fi
