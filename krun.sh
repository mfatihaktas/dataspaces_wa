#!/bin/bash
echo $1 $2 $3

NUM_SNODE=1
NUM_CLIENT=20
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PEER=1
NUM_PUTGET=10

LCONTROL_LINTF="eth0"
RI_MANAGER_LCONTROL_LPORT_LIST=( 8000 8000 )
APP_JOIN_LCONTROL_LIP_LIST=( "130.207.110.51" "130.207.110.52" )

CONTROL_LINTF="eth0" # "lo"
RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 7002 )
RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.51" "130.207.110.51" )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 7000 )

TRANS_PROTOCOL="t" # "i" # "g"
IB_LINTF="ib0"
TCP_LINTF="eth0"
TCP_LPORT=6000
GFTP_LINTF="eth0"
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
  # if [ -a log ]; then
  #   rm log
  # fi
  
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
elif [ $1  = 'iperf' ]; then
  IPERF_BIN_DIR=/net/hp101/ihpcsc/jchoi446/sw/iperf/2.0.5/bin
  if [ -z "$2" ]; then
    echo "Which s-c"
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
else
  echo "Argument did not match !"
fi
