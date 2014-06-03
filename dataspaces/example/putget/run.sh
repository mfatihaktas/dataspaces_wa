#!/bin/bash
echo $1

BIN_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/bin

if [ $1  = 's' ]; then
  $BIN_DIR/./dataspaces_server -s 1 -c 2
elif [ $1  = 'g' ]; then
  ./test_get
elif [ $1  = 'p' ]; then
  ./test_put
else
	echo "Argument did not match !"
fi
