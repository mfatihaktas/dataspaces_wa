#!/bin/bash
echo $1

TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/transfer
PORT=5000

if [ $1  = 'e' ]; then
  GLOG_logtostderr=1 ./exp --src_url="$TRANS_DIR/dummy.dat" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/recved_dummy.dat"
elif [ $1  = 'egs' ]; then
  GLOG_logtostderr=1 ./exp -s --port=5000
elif [ $1  = 'egc' ]; then  
  GLOG_logtostderr=1 ./exp --src_url="ftp://127.0.0.1:5000$TRANS_DIR/recved_dummy.dat" --dst_url="$TRANS_DIR/dummy.dat"
elif [ $1  = 'show' ]; then
  netstat -antu
elif [ $1  = 's' ]; then
  globus-gridftp-server -aa -password-file pwfile -c None \
                        -port $PORT \
                        -d error,warn,info,dump,all \
                        #-single
                        #-control-interface 127.0.0.1:$CPORT

elif [ $1  = 'c' ]; then
  F=dummy.dat
  S_FILE=127.0.0.1:$PORT$TRANS_DIR/$F
  C_FILE=$TRANS_DIR/_$F
  
  globus-url-copy -vb -p $P -cc $CC \
                  ftp://$S_FILE file://$C_FILE
else
  echo "Argument did not match !"
fi