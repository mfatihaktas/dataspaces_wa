#!/bin/bash
echo $1 $2 $3
# Note: This file can be used for running dataspaces_server and ri_manager for wa_dataspaces while
# producer, consumer sites are on ULAM, KID

NUM_CLIENT=1
NUM_PEER=1
NUM_PUTGET=10

# TMPFS_DIR=$DSPACESWA_DIR/tmpfs
if [ -n "$ULAM" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP="192.168.2.100"
  
  RI_MANAGER_CONTROL_LINTF="ib0:2"
  RI_MANAGER_CONTROL_LPORT=8800
  # RI_MANAGER_CONTROL_LPORT=8801
  RI_MANAGER_JOIN_CONTROL_LIP=""
  RI_MANAGER_JOIN_CONTROL_LPORT=0
  # RI_MANAGER_JOIN_CONTROL_LIP="192.168.210.162"
  # RI_MANAGER_JOIN_CONTROL_LPORT=8800
  
  IB_LINTF="ib0:2"
  TCP_LINTF="ib0:2"
  GFTP_LINTF="eth0"
  
  PSEUDO_2=0
elif [ -n "$KID" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP="130.207.110.52"
  
  RI_MANAGER_CONTROL_LINTF="ib0"
  RI_MANAGER_CONTROL_LPORT=8801
  # RI_MANAGER_CONTROL_LPORT=8800
  RI_MANAGER_JOIN_CONTROL_LIP="192.168.210.100"
  RI_MANAGER_JOIN_CONTROL_LPORT=8800
  # RI_MANAGER_JOIN_CONTROL_LIP=""
  # RI_MANAGER_JOIN_CONTROL_LPORT=0
  
  IB_LINTF="ib0"
  TCP_LINTF="ib0"
  GFTP_LINTF="eth0"
  
  PSEUDO_2=1
elif [ -n "$BOOTH" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP="192.168.2.241"
  
  RI_MANAGER_CONTROL_LINTF="ib0"
  RI_MANAGER_CONTROL_LPORT=8801
  RI_MANAGER_JOIN_CONTROL_LIP="192.168.210.100"
  RI_MANAGER_JOIN_CONTROL_LPORT=8800
  
  IB_LINTF="ib0"
  TCP_LINTF="ib0"
  GFTP_LINTF="eth0"
  
  PSEUDO_2=1
else
  echo "Unexpected system!"
fi

RI_MANAGER_LCONTROL_LPORT=80000
TMPFS_DIR=$TMPFS_DIR"/put"

TCP_LPORT=6000
GFTP_LPORT=6000

TRANS_PROTOCOL="t" # "i" # "g"
W_PREFETCH=1
# 
if [ $1  = 'p' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./exp --type="put" --cl_id=$2 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT
  fi
elif [ $1  = 'g' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./exp --type="get" --cl_id=$2 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT
  fi
elif [ $1  = 'dp' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="put" --cl_id=1 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT
  fi
elif [ $1  = 'dg' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="get" --cl_id=1 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT
  fi
elif [ $1  = 'mp' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./mput_mget_test --type="mput" --cl_id=$2 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0
  fi
elif [ $1  = 'mg' ]; then
  if [ -z "$2" ]; then
    echo "Which app?"
  else
    GLOG_logtostderr=1 ./mput_mget_test --type="mget" --cl_id=$2 --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT+$2)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0
  fi
elif [ $1  = 'map' ]; then
  if [ -a log ]; then
    rm log mput_log
  fi
  
  for i in `seq 1 $NUM_CLIENT`;
  do
    export GLOG_logtostderr=1
    ./mput_mget_test --type="mput" --cl_id=$i --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT + $i)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
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
    ./mput_mget_test --type="mget" --cl_id=$i --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((RI_MANAGER_LCONTROL_LPORT + $i)) \
      --join_lcontrol_lip=$APP_JOIN_LCONTROL_LIP --join_lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
      --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 &
  done
  
  read -p "[Enter]"
  echo "Killing..."
  pkill -f mput_mget_test
elif [ $1  = 'r' ]; then
  # if [ $TRANS_PROTOCOL  = 'g' ]; then
  #   echo "Starting Gftps..."
  #   globus-gridftp-server -aa -password-file pwfile -c None \
  #                         -port $GFTP_LPORT \
  #                         -d error,warn,info,dump,all &
  #                         # -data-interface $WA_LINTF \
  # fi
  GLOG_logtostderr=1 ./exp --type="ri" --cl_id=111 --num_peer=$NUM_PEER --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) \
                           --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
                           --ds_id=$PSEUDO_2 --control_lintf=$RI_MANAGER_CONTROL_LINTF --control_lport=$RI_MANAGER_CONTROL_LPORT \
                           --join_control_lip=$RI_MANAGER_JOIN_CONTROL_LIP --join_control_lport=$RI_MANAGER_JOIN_CONTROL_LPORT \
                           --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                           --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                           --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR \
                           --w_prefetch=$W_PREFETCH
  read -p "[Enter]"
  # echo "Killing stubborns..."
  # fuser -k -n tcp $GFTP_LPORT
elif [ $1  = 'dr' ]; then
  export GLOG_logtostderr=1
  gdb --args ./exp --type="ri" --cl_id=111 --num_peer=$NUM_PEER --base_client_id=$(($PSEUDO_2*$NUM_CLIENT)) \
                   --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
                   --ds_id=$PSEUDO_2 --control_lintf=$RI_MANAGER_CONTROL_LINTF --control_lport=$RI_MANAGER_CONTROL_LPORT \
                   --join_control_lip=$RI_MANAGER_JOIN_CONTROL_LIP --join_control_lport=$RI_MANAGER_JOIN_CONTROL_LPORT \
                   --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
                   --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
                   --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR \
                   --w_prefetch=$W_PREFETCH
  read -p "[Enter]"
else
  echo "Argument did not match !"
fi
