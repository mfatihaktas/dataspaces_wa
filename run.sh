#!/bin/bash
echo $1 $2 $3

DHT_LINTF="em2" #"em2" #"eth0"
WA_LINTF="ib0" #"em2" #"ib0"

RM1_DHT_LPORT="60000"
RM2_DHT_LPORT="65000"
RM2_DHT_LIP="192.168.100.120" #"192.168.100.120" #"192.168.100.120" "192.168.2.152" #

GFTP_LPORT="62002"
TMPFS_DIR="/dev/shm"

TRANS_PROTOCOL="i" #"g" #"i"

# DSPACES_BIN=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install/bin
DSPACES_BIN=$DSPACES_DIR/bin

NUM_SNODES=1
NUM_DSCNODES=$((1+1)) #+1: RIManager

if [ $1  = 's' ]; then
  $DSPACES_BIN/./dataspaces_server --server $NUM_SNODES --cnodes $NUM_DSCNODES
  #$DSPACES_BIN/./dataspaces_server -s $NUM_SNODES -c $NUM_DSCNODES
elif [ $1  = 'p' ]; then
  GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # if [ $2  = '1' ]; then
  #   GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # elif [ $2  = '2' ]; then
  #   GLOG_logtostderr=1 ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # elif [ $2  = '22' ]; then
  #   GLOG_logtostderr=1 ./exp --type="put_2" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # fi
elif [ $1  = 'g' ]; then
  GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # if [ $2  = '1' ]; then
  #   GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # elif [ $2  = '2' ]; then
  #   GLOG_logtostderr=1 ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
  # fi
elif [ $1  = 'dp' ]; then
  export GLOG_logtostderr=1 
  export MALLOC_CHECK_=2
  gdb --args ./exp --type="put" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'dg' ]; then
  export GLOG_logtostderr=1 
  export MALLOC_CHECK_=2
  gdb --args ./exp --type="get" --num_dscnodes=$NUM_DSCNODES --app_id=1
elif [ $1  = 'rm' ]; then
  if [ $TRANS_PROTOCOL  = 'g' ]; then
    echo "Starting Gftps..."
    globus-gridftp-server -aa -password-file pwfile -c None \
                          -port $GFTP_LPORT \
                          -d error,warn,info,dump,all &
                          # -data-interface $WA_LINTF \
  fi
  if [ $2  = '1' ]; then
    GLOG_logtostderr=1 ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                             --dht_lintf=$DHT_LINTF --dht_lport=$RM1_DHT_LPORT --ipeer_dht_laddr=$RM2_DHT_LIP --ipeer_dht_lport=$RM2_DHT_LPORT \
                             --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$WA_LINTF \
                             --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  elif [ $2  = '2' ]; then
    GLOG_logtostderr=  ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                             --dht_lintf=$DHT_LINTF --dht_lport=$RM2_DHT_LPORT \
                             --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$WA_LINTF \
                             --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
  read -p "[Enter]"
  echo "killing stubborns..."
  fuser -k -n tcp $GFTP_LPORT
  fuser -k -n tcp $RM1_DHT_LPORT
  fuser -k -n tcp $RM2_DHT_LPORT
elif [ $1  = 'drm' ]; then
  if [ $TRANS_PROTOCOL  = 'g' ]; then
    echo "Starting Gftps..."
    globus-gridftp-server -aa -password-file pwfile -c None \
                          -port $GFTP_LPORT \
                          -d error,warn,info,dump,all &
                          # -data-interface $WA_LINTF \
  fi
  if [ $2  = '1' ]; then
    export GLOG_logtostderr=1
    gdb --args ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                     --dht_lintf=$DHT_LINTF --dht_lport=$RM1_DHT_LPORT --ipeer_dht_laddr=$RM2_DHT_LIP --ipeer_dht_lport=$RM2_DHT_LPORT \
                     --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$WA_LINTF \
                     --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  elif [ $2  = '2' ]; then
    export GLOG_logtostderr=1 
    gdb --args ./exp --type="ri" --dht_id=$2 --num_dscnodes=$NUM_DSCNODES --app_id=10 \
                     --dht_lintf=$DHT_LINTF --dht_lport=$RM2_DHT_LPORT \
                     --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$WA_LINTF \
                     --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
  read -p "[Enter]"
  echo "killing Gftps..."
  fuser -k -n tcp $GFTP_LPORT
  fuser -k -n tcp $RM1_DHT_LPORT
  fuser -k -n tcp $RM2_DHT_LPORT
elif [ $1  = 'gc' ]; then
  TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/gftp_trans/dummy
  # S_IP=192.168.2.152
  S_IP=127.0.0.1
  # S_FNAME=dummy.dat
  S_FNAME=tx.dat
  C_FNAME=recved_tx.dat
  globus-url-copy -vb \
                  ftp://"$S_IP:$GFTP_LPORT$TRANS_DIR/$S_FNAME" file://"$TRANS_DIR/$C_FNAME"
elif [ $1  = 'show' ]; then
  netstat -antu
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
  
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'u' ]; then
    # export ULAM=ULAM
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export MPI_DIR=/opt/intel/impi/4.1.3.048/intel64
    export GLOG_DIR=/home/sc14demo/common-apps/glog-0.3.3/install
    export BOOST_DIR=/home/sc14demo/common-apps/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/home/sc14demo/common-apps/dataspaces-1.4.0/install
    export DSPACESWA_DIR=/home/sc14demo/common-apps/dataspaces_wa
    
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'm' ]; then
    # export MAQUIS=MAQUIS
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export GLOG_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install
    export BOOST_DIR=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.4.0/install
    export DSPACESWA_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa
    
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
    
    export PATH=/net/hj1/ihpcl/bin:$PATH
    echo "PATH="
    echo $PATH
  fi
else
  echo "Argument did not match !"
fi