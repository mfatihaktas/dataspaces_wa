#!/bin/bash
echo $1 $2 $3

DSPACES_BINDIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.4.0/install/bin

NUM_SNODES=1
NUM_DSCNODES=1

if [ $1  = 's' ]; then
  $DSPACES_BINDIR/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
elif [ $1  = 'p' ]; then
  ./exp --type=put_test --app_id=1 --num_dscnodes=$NUM_DSCNODES
elif [ $1  = 'g' ]; then
  ./exp --type=get_test --app_id=2 --num_dscnodes=$NUM_DSCNODES
else
  echo "Argument did not match !"
fi