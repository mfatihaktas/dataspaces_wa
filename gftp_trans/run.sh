#!/bin/bash
echo $1

TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/gftp_trans/dummy
PORT=5000
# S_IP=192.168.2.152
S_IP=127.0.0.1
# S_FNAME=dummy.dat
S_FNAME=tx.dat
C_FNAME=recved_tx.dat

C2_FNAME=recved2_tx.dat

if [ $1  = 'c' ]; then  
  GLOG_logtostderr=1 ./exp --type="c" --src_url="$TRANS_DIR/$S_FNAME" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/$C_FNAME"
elif [ $1  = 'c2' ]; then  
  GLOG_logtostderr=1 ./exp --type="c2" --src_url="$TRANS_DIR/$S_FNAME" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/$C2_FNAME"
elif [ $1  = 'dc' ]; then  
  export GLOG_logtostderr=1
  gdb --args ./exp --src_url="$TRANS_DIR/$S_FNAME" --dst_url="ftp://127.0.0.1:5000$TRANS_DIR/$C_FNAME"
elif [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type="s" --port=5000
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
  globus-url-copy -vb -p $P -cc $CC \
                  ftp://"$S_IP:$PORT$TRANS_DIR/$S_FNAME" file://"$TRANS_DIR/$C_FNAME"
elif [ $1  = 'tc2' ]; then
  globus-url-copy -vb -p $P -cc $CC \
                  ftp://"$S_IP:$PORT$TRANS_DIR/$S_FNAME" file://"$TRANS_DIR/$C2_FNAME"
elif [ $1  = 'gf' ]; then
  dd if=/dev/urandom of="$TRANS_DIR/$S_FNAME" bs=1024 count=1000000 #outputs bs x count Bs
else
  echo "Argument did not match !"
fi