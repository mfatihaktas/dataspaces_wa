#!/bin/bash
echo $1

DSBIN_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dataspaces/dataspaces-1.3.0/install/bin

if [ $1  = 'e' ]; then
  GLOG_logtostderr=1 ./exp --intf "lo" --lport 7000
elif [ $1  = 'e2' ]; then
  GLOG_logtostderr=1 ./exp2 --intf "lo" --lport 8000 --ipeer_lip "127.0.0.1" --ipeer_lport 7000
elif [ $1  = 's' ]; then
  $DSBIN_DIR/./dataspaces_server -s 1 -c 2
elif [ $1  = 'g' ]; then
  ./exp_get
elif [ $1  = 'p' ]; then
  ./exp_put
elif [ $1  = 'cs' ]; then
  ./chat_server 6000
elif [ $1  = 'cc' ]; then
  ./chat_client localhost 6000
else
	echo "Argument did not match !"
fi
