#!/bin/bash
echo $1 $2 $3

TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/trans/gftp_trans/dummy
S_LINTF="em2"
# S_LADDR=127.0.0.1
S_LADDR=192.168.2.151
S_LPORT=62000
TMPFS_DIR="/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/trans/gftp_trans/dummy"
# S_FNAME=dummy.dat
S_FNAME=tx.dat
C_FNAME=recved_tx.dat

C2_FNAME=recved2_tx.dat

if [ $1  = 's' ]; then
  GLOG_logtostderr=1 ./exp --type="s" --s_lintf=$S_LINTF --s_lport=$S_LPORT --s_tmpfs_dir=$TMPFS_DIR"/server"
elif [ $1  = 'ds' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp -s --port=$PORT
elif [ $1  = 'g' ]; then
  # GLOG_logtostderr=1 ./exp --type="g" --src_url="ftp://$S_LADDR:$PORT$TRANS_DIR/$S_FNAME" --dst_url="$TRANS_DIR/$C_FNAME"
  # export GLOG_logtostderr=1
  # valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./exp --type="g" --s_laddr=$S_LADDR --s_lport=$S_LPORT --tmpfs_dir=$TMPFS_DIR"/get" --s_tmpfs_dir=$TMPFS_DIR"/server"
  GLOG_logtostderr=1 ./exp --type="g" --s_laddr=$S_LADDR --s_lport=$S_LPORT --tmpfs_dir=$TMPFS_DIR"" --s_tmpfs_dir=$TMPFS_DIR"/server"
elif [ $1  = 'dg' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="g" --s_laddr=$S_LADDR --s_lport=$S_LPORT --tmpfs_dir=$TMPFS_DIR"/get" --s_tmpfs_dir=$TMPFS_DIR"/server"
elif [ $1  = 'p' ]; then
  # GLOG_logtostderr=1 ./exp --type="p" --src_url="$TRANS_DIR/$S_FNAME" --dst_url="ftp://$S_LADDR:$PORT$TRANS_DIR/$C_FNAME"
  GLOG_logtostderr=1 ./exp --type="p" --s_laddr=$S_LADDR --s_lport=$S_LPORT --tmpfs_dir=$TMPFS_DIR"" --s_tmpfs_dir=$TMPFS_DIR"/server"
elif [ $1  = 'dp' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="p" --s_laddr=$S_LADDR --s_lport=$S_LPORT --tmpfs_dir=$TMPFS_DIR"/put" --s_tmpfs_dir=$TMPFS_DIR"/server"
elif [ $1  = 'show' ]; then
  netstat -antu
elif [ $1  = 'bs' ]; then
  globus-gridftp-server -aa -password-file pwfile -c None \
                        -port $S_LPORT \
                        -d error,warn,info,dump,all \
                        #-single
                        #-control-interface 127.0.0.1:$CPORT
elif [ $1  = 'bc' ]; then
  # echo "globus-url-copy -vb -p $P -cc $CC ftp://$S_LADDR:$S_LPORT$TRANS_DIR/$S_FNAME file://$TRANS_DIR/$C_FNAME"
  globus-url-copy -vb \
                  ftp://$S_LADDR:$S_LPORT$TRANS_DIR/$S_FNAME file://$TRANS_DIR/$C_FNAME
elif [ $1  = 'bc2' ]; then
  globus-url-copy -vb \
                  ftp://$S_LADDR:$S_LPORT$TRANS_DIR/$S_FNAME file://$TRANS_DIR/$C2_FNAME
elif [ $1  = 'gf' ]; then
  dd if=/dev/urandom of="$TRANS_DIR/$S_FNAME" bs=1024 count=1000 #outputs bs x count Bs
elif [ $1  = 'cf' ]; then
  rm ./dummy/*
  rm ./dummy/server/*
  rm ./dummy/put/*
  rm ./dummy/get/*
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    # ENV VARIABLES FOR MAKE
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
  
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo LD_LIBRARY_PATH
    echo $LD_LIBRARY_PATH
  fi
else
  echo "Argument did not match !"
fi