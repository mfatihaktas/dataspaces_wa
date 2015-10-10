#!/bin/bash
echo $1 $2 $3

NUM_SNODE=1
NUM_CLIENT=1
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager

CONTROL_LINTF=eth0 # "lo"
RI_MANAGER_CONTROL_LPORT_LIST=( "7000" "7001" "7002" )
RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.201" "192.168.2.201" )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( "0" "7000" "7000" )

TRANS_PROTOCOL="i" # "g"
IB_LINTF="ib0"
GFTP_LINTF="eth0"
GFTP_LPORT="6000"
TMPFS_DIR=$DSPACESWA_DIR/tmpfs

if [ $1  = 's' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  $DSPACES_DIR/bin/./dataspaces_server --server $NUM_SNODE --cnodes $NUM_DSCNODE
elif [ $1  = 'p' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./exp --type="put" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT
  fi
elif [ $1  = 'mp' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./ds_wa_test --type="mput" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT \
                                    --num_putget=10 --inter_time_sec=0
  fi
elif [ $1  = 'g' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./exp --type="get" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT
  fi
elif [ $1  = 'mg' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./ds_wa_test --type="mget" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT \
                                    --num_putget=10 --inter_time_sec=1
  fi
elif [ $1  = 'dp' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="put" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT
  fi
elif [ $1  = 'dg' ]; then
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    export GLOG_logtostderr=1
    export MALLOC_CHECK_=2
    gdb --args ./exp --type="get" --cl_id=1 --base_client_id=$(($2*10)) --num_client=$NUM_CLIENT
  fi
elif [ $1  = 'r' ]; then
  # if [ $TRANS_PROTOCOL  = 'g' ]; then
  #   echo "Starting Gftps..."
  #   globus-gridftp-server -aa -password-file pwfile -c None \
  #                         -port $GFTP_LPORT \
  #                         -d error,warn,info,dump,all &
  #                         # -data-interface $WA_LINTF \
  # fi
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    GLOG_logtostderr=1 ./exp --type="ri" --cl_id=111 --num_client=$NUM_CLIENT --base_client_id=$(($2*10)) \
                             --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                             --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                             --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
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
  if [ -z "$2" ]; then
    echo "Which site?"
  else
    export GLOG_logtostderr=1
    gdb --args ./exp --type="ri" --cl_id=111 --num_client=$NUM_CLIENT --base_client_id=$(($2*10)) \
                     --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2] } \
                     --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2] } \
                     --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR
  fi
  read -p "[Enter]"
  echo "Killing stubborns..."
  fuser -k -n tcp $GFTP_LPORT
  # fuser -k -n tcp $RM1_DHT_LPORT
else
  echo "Argument did not match !"
fi