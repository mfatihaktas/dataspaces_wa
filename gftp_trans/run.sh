#!/bin/bash
echo $1

TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/gftp_trans/dummy
PORT=5000
S_IP=127.0.0.1
# S_IP=192.168.2.152

if [ $1  = 'c' ]; then  
  #GLOG_logtostderr=1 ./exp_gridftp --src_url="ftp://127.0.0.1:5000$TRANS_DIR/recved_dummy.dat" --dst_url="$TRANS_DIR/dummy.dat"
  GLOG_logtostderr=1 ./exp --src_url="$TRANS_DIR/dummy.dat" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/recved_dummy.dat"
elif [ $1  = 'dc' ]; then  
  export GLOG_logtostderr=1
  gdb --args ./exp --src_url="$TRANS_DIR/dummy.dat" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/recved_dummy.dat"
elif [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp -s --port=5000
elif [ $1  = 'ds' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp -s --port=5000
elif [ $1  = 'show' ]; then
  netstat -antu
elif [ $1  = 'ts' ]; then
  globus-gridftp-server -aa -password-file pwfile -c None \
                        -port $PORT \
                        -d error,warn,info,dump,all \
                        #-single
                        #-control-interface 127.0.0.1:$CPORT

elif [ $1  = 'tc' ]; then
  F=dummy.dat
  S_FILE=$S_IP:$PORT$TRANS_DIR/$F
  C_FILE=$TRANS_DIR/_$F
  
  globus-url-copy -vb -p $P -cc $CC \
                  ftp://$S_FILE file://$C_FILE
else
  echo "Argument did not match !"
fi