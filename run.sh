#!/bin/bash
$1 $2 $3

NUM_SNODE=1
NUM_CLIENT=1
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PEER=1
NUM_PUTGET=10

# TMPFS_DIR=$DSPACESWA_DIR/tmpfs
if [ -n "$DELL" ]; then
  LCONTROL_LINTF="em2"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF="em2"
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  IB_LINTF="ib0"
  TCP_LINTF="em2"
  GFTP_LINTF="em2"
elif [ -n "$ULAM" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.100" "192.168.2.202" )
  
  CONTROL_LINTF="eth0"
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.100" )
  
  IB_LINTF="ib0"
  TCP_LINTF="eth0"
  GFTP_LINTF="eth0"
elif [ -n "$KID" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP_LIST=( "130.207.110.52" "130.207.110.53" )
  
  CONTROL_LINTF="eth0"
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.52" )
  
  IB_LINTF="ib0"
  TCP_LINTF="eth0"
  GFTP_LINTF="eth0"
else
  echo "Which system DELL | ULAM | KID"
fi

RI_MANAGER_LCONTROL_LPORT_LIST=( 9000 9000 )

RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 7000 )

TCP_LPORT=6000
GFTP_LPORT=6000

TRANS_PROTOCOL="t" # "i" # "g"
W_PREFETCH=1

if [ $1  = 's' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  # export MPIRUN_OPTIONS="-e MPI_RDMA_MSGSIZE=32768,1048576,1048576"
  $DSPACES_DIR/bin/./dataspaces_server --server 1 --cnodes $NUM_DSCNODE
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
    rm log mput_log
  fi
  
  for i in `seq 1 $NUM_CLIENT`;
  do
    export GLOG_logtostderr=1
    ./mput_mget_test --type="mput" --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } + $i)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 &
  done
  
  read -p "[Enter]"
  echo "killing..."
  pkill -f mput_mget_test
elif [ $1  = 'mag' ]; then
  if [ -a mget_log ]; then
    rm mget_log
  fi
  
  for i in `seq 1 $NUM_CLIENT`;
  do
    export GLOG_logtostderr=1
    ./mput_mget_test --type="mget" --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
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
                           --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR \
                           --w_prefetch=$W_PREFETCH
  read -p "[Enter]"
  echo "Killing stubborns..."
  # fuser -k -n tcp $GFTP_LPORT
elif [ $1  = 'dr' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --cl_id=111 --num_peer=$NUM_PEER --base_client_id=$(($2*$NUM_CLIENT)) \
                   --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2] } \
                   --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                   --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                   --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                   --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                   --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR \
                   --w_prefetch=$W_PREFETCH
  read -p "[Enter]"
  echo "Killing stubborns..."
  fuser -k -n tcp $GFTP_LPORT
elif [ $1  = 'init' ]; then
  if [ $2  = 'd' ]; then
    export DELL=DELL
    # ENV VARIABLES FOR MAKE
    # export GRIDFTP=GRIDFTP
    export CC=gcc #/opt/gcc-4.8.2/bin/gcc
    export CPP=g++ #/opt/gcc-4.8.2/bin/g++
    export MPICPP=mpicxx #/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    # export MPI_DIR=/cac/u01/mfa51/Desktop/mpich-3.1.2/install
    export GLOG_DIR=/cac/u01/mfa51/Desktop/glog-0.3.3/install
    export BOOST_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.5.0/install
    export DSPACESWA_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
    
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    # LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    # LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
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
    
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
  elif [ $2  = 'k' ]; then
    export KID=KID
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export GLOG_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install
    export BOOST_DIR=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.6.0/install
    export DSPACESWA_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa
    
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
    
    export PATH=/net/hj1/ihpcl/bin:/sbin:$PATH
    echo "PATH=" $PATH
  fi
elif [ $1  = 'iperf' ]; then
  if [ -n "$DELL" ]; then
    echo "No iperf on DELL"
  elif [ -n "$ULAM" ]; then
    IPERF_BIN_DIR=/home/sc14demo/common-apps/iperf/2.0.5/bin
  elif [ -n "$KID" ]; then
    IPERF_BIN_DIR=/net/hp101/ihpcsc/jchoi446/sw/iperf/2.0.5/bin
  fi
  if [ -z "$2" ]; then
    echo "Which s | c"
  else
    if [ $2  = 's' ]; then
      $IPERF_BIN_DIR/./iperf -s
    elif [ $2  = 'c' ]; then
      if [ -z "$3" ]; then
        echo "Client ip?"
      else
        $IPERF_BIN_DIR/./iperf -c $3
      fi
    fi
  fi
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
else
  echo "Argument did not match !"
fi
