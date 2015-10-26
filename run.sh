#!/bin/bash
echo $1 $2 $3

NUM_SNODE=1
NUM_CLIENT=2
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PEER=1
NUM_PUTGET=10

LCONTROL_LINTF="em2"
RI_MANAGER_LCONTROL_LPORT_LIST=( 8000 8000 8000 )
APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" "192.168.2.153" )

CONTROL_LINTF="em2" # "lo"
RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 7002 )
RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" "192.168.2.151" )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 7000 )

TRANS_PROTOCOL="t" # "i" # "g"
IB_LINTF="ib0"
TCP_LINTF="em2"
TCP_LPORT=6000
GFTP_LINTF="em2"
GFTP_LPORT=6000
TMPFS_DIR=$DSPACESWA_DIR/tmpfs

if [ $1  = 's' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  export MPIRUN_OPTIONS="-e MPI_RDMA_MSGSIZE=32768,1048576,1048576"
  $DSPACES_DIR/bin/./dataspaces_server --server $NUM_SNODE --cnodes $NUM_DSCNODE
elif [ -z "$2" ]; then
  echo "Which site?"
elif [ $1  = 'p' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./exp --type="put" --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] }
  fi
elif [ $1  = 'g' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./exp --type="get" --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] }
  fi
elif [ $1  = 'dp' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="put" --cl_id=1 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] }
  fi
elif [ $1  = 'dg' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="get" --cl_id=1 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] }
  fi
elif [ $1  = 'mp' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./mput_mget_test --type="mput" --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0
  fi
elif [ $1  = 'mg' ]; then
  if [ -z "$3" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./mput_mget_test --type="mget" --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $3)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0
  fi
elif [ $1  = 'map' ]; then
  if [ -a log ]; then
    rm log
  fi
  
  for i in `seq 1 $NUM_CLIENT`;
  do
    GLOG_logtostderr=1 ./mput_mget_test --type="mput" --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $i)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 &
  done
  
  read -p "[Enter]"
  echo "killing..."
  pkill -f mput_mget_test
elif [ $1  = 'mag' ]; then
  # if [ -a log ]; then
  #   rm log
  # fi
  
  for i in `seq 1 $NUM_CLIENT`;
  do
    GLOG_logtostderr=1 ./mput_mget_test --type="mget" --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $i)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 &
  done
  
  read -p "[Enter]"
  echo "killing..."
  pkill -f mput_mget_test
elif [ $1  = 'r' ]; then
  # if [ $TRANS_PROTOCOL  = 'g' ]; then
  #   echo "Starting Gftps..."
  #   globus-gridftp-server -aa -password-file pwfile -c None \
  #                         -port $GFTP_LPORT \
  #                         -d error,warn,info,dump,all &
  #                         # -data-interface $WA_LINTF \
  # fi
  GLOG_logtostderr=1 ./exp --type="ri" --cl_id=111 --num_peer=$NUM_PEER --base_client_id=$(($2*$NUM_CLIENT)) \
                           --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
                           --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                           --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                           --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                           --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                           --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  read -p "[Enter]"
  echo "Killing stubborns..."
  fuser -k -n tcp $GFTP_LPORT
  # fuser -k -n tcp $RM1_DHT_LPORT
elif [ $1  = 'dr' ]; then
  # if [ $TRANS_PROTOCOL  = 'g' ]; then
  #   echo "Starting Gftps..."
  #   globus-gridftp-server -aa -password-file pwfile -c None \
  #                         -port $GFTP_LPORT \
  #                         -d error,warn,info,dump,all &
  #                         # -data-interface $WA_LINTF \
  # fi
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --cl_id=111 --num_peer=$NUM_PEER --base_client_id=$(($2*$NUM_CLIENT)) \
                   --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
                   --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                   --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                   --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                   --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                   --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  read -p "[Enter]"
  echo "Killing stubborns..."
  fuser -k -n tcp $GFTP_LPORT
  # fuser -k -n tcp $RM1_DHT_LPORT
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
    # export GRIDFTP=GRIDFTP
    # export CC=/opt/gcc-4.8.2/bin/gcc
    # export CPP=/opt/gcc-4.8.2/bin/g++
    # export MPICPP=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    export CC=gcc #/opt/gcc-4.8.2/bin/gcc
    export CPP=g++ #/opt/gcc-4.8.2/bin/g++
    export MPICPP=mpicxx #/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    # export MPI_DIR=/cac/u01/mfa51/Desktop/mpich-3.1.2/install
    unset MPI_DIR
    export GLOG_DIR=/cac/u01/mfa51/Desktop/glog-0.3.3/install
    echo $GLOG_DIR
    export BOOST_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0/install
    echo $BOOST_DIR
    export GFTPINC_DIR=/usr/include/globus
    echo $GFTPINC_DIR
    export GFTPLIB_DIR=/usr/lib64
    echo $GFTPLIB_DIR
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
    echo $DSPACES_DIR
    export DSPACESWA_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
    echo $DSPACESWA_DIR
    
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
    export ULAM=ULAM
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx # module openmpi/intel-14.0.2/1.8.2
    # export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    # export MPI_DIR=/apps/intel/impi/4.1.3.048/intel64
    export GLOG_DIR=/home/sc14demo/common-apps/glog-0.3.3/install
    export BOOST_DIR=/home/sc14demo/common-apps/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/home/sc14demo/common-apps/dataspaces-1.5.0/install
    export DSPACESWA_DIR=/home/sc14demo/common-apps/dataspaces_wa
    
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/boost_1_56_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/home/sc14demo/common-apps/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
  elif [ $2  = 'k' ]; then
    export MAQUIS=MAQUIS
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export GLOG_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install
    export BOOST_DIR=/net/hp101/ihpcsc/maktas7/boost_1_59_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.5.0/install
    export DSPACESWA_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa
    
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/boost_1_59_0/install/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH="
    echo $LD_LIBRARY_PATH
    
    export PATH=/net/hj1/ihpcl/bin:/sbin:$PATH
    echo "PATH="
    echo $PATH
  fi
else
  echo "Argument did not match !"
fi