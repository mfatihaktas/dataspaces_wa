#!/bin/bash
echo $1

GFTP_DIR=/home/ubuntu/gftp
PORT=5000
CPORT=6000
F=ltx.dat
C_FILE=$GFTP_DIR/_$F
S_FILE=10.39.1.12:$PORT$GFTP_DIR/$F

LOG_FILE=$GFTP_DIR/gftps.log
P=1
CC=1

if [ $1  = 'c' ]; then
  globus-url-copy -vb -p $P -cc $CC \
                  ftp://$S_FILE file://$C_FILE
                  #-tcp-buffer-size 1024000 \
elif [ $1  = 's' ]; then
  globus-gridftp-server -aa -password-file pwfile -c None \
                        -control-interface 127.0.0.1:$CPORT
                        -log-module stdio -log-level info,warn,error -logfile $LOG_FILE \
                        -port $PORT
elif [ $1  = 'glf' ]; then
	dd if=/dev/urandom of=$F bs=1024 count=1000000
else
	echo "Argument did not match !"
fi
