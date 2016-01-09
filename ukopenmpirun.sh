#!/bin/bash
echo $1 $2 $3
# Note: This file can be used for running dataspaces_server and ri_manager for wa_dataspaces while
# producer, consumer sites are on ULAM, KID

if [ -n "$ULAM" ]; then
  NUM_DS_NODE=3
  DS_NODE="-host ulam -host archer1 -host archer2"
  RI_MANAGER_NODE="-host ulam"
  NUM_DSPACESWA_CLIENT=1
  
  RI_MANAGER_LCONTROL_LINTF="eth0"
  
  RI_MANAGER_CONTROL_LINTF="ib0:2"
  RI_MANAGER_CONTROL_LPORT=8800
  RI_MANAGER_JOIN_CONTROL_LIP=""
  RI_MANAGER_JOIN_CONTROL_LPORT=0
  
  RI_MANAGER_IB_LINTF="ib0:2"
  RI_MANAGER_TCP_LINTF="ib0:2"
  RI_MANAGER_GFTP_LINTF="eth0"
  
  MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
elif [ -n "$KID" ]; then
  NUM_DS_NODE=8
  DS_NODE="-host kid41 -host kid42 -host kid43 -host kid44 -host kid45 -host kid46 -host kid47 -host kid48"
  RI_MANAGER_NODE="-host kid42"
  NUM_DSPACESWA_CLIENT=1
  
  RI_MANAGER_LCONTROL_LINTF="eth0"
  
  RI_MANAGER_CONTROL_LINTF="ib0"
  RI_MANAGER_CONTROL_LPORT=8801
  RI_MANAGER_JOIN_CONTROL_LIP="192.168.210.100"
  RI_MANAGER_JOIN_CONTROL_LPORT=8800
  
  RI_MANAGER_IB_LINTF="ib0"
  RI_MANAGER_TCP_LINTF="ib0"
  RI_MANAGER_GFTP_LINTF="eth0"
  
  MPIRUN=/net/hj1/ihpcl/bin/mpirun
elif [ -n "$BOOTH" ]; then
  NUM_DS_NODE=1
  DS_NODE="-host santa-clara"
  RI_MANAGER_NODE="-host santa-clara"
  NUM_DSPACESWA_CLIENT=1
  
  RI_MANAGER_LCONTROL_LINTF="eth0"
  
  RI_MANAGER_CONTROL_LINTF="ib0"
  RI_MANAGER_CONTROL_LPORT=8801
  RI_MANAGER_JOIN_CONTROL_LIP="192.168.210.100"
  RI_MANAGER_JOIN_CONTROL_LPORT=8800
  
  RI_MANAGER_IB_LINTF="ib0"
  RI_MANAGER_TCP_LINTF="ib0"
  RI_MANAGER_GFTP_LINTF="eth0"
  RI_MANAGER_TMPFS_DIR=$TMPFS_DIR"/put"
  
  PSEUDO_2=1
  MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
else
  echo "Unexpected system!"
fi

RI_MANAGER_LCONTROL_LPORT=80000

RI_MANAGER_TCP_LPORT=6000
RI_MANAGER_GFTP_LPORT=6000
RI_MANAGER_TMPFS_DIR=$TMPFS_DIR"/put"

W_PREFETCH=1
TRANS_PROTOCOL="t"                                                                                  #i:infiniband, t:tcp, g:gridftp
# 
PKILL=/usr/bin/pkill
if [ $1  = 'r' ]; then
  if [ -a conf ]; then
    rm conf                                                                                       #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  
  $MPIRUN -npernode 1 $DS_NODE \
    $DSPACES_DIR/bin/dataspaces_server \
    --server $NUM_DS_NODE --cnodes $(($NUM_DSPACESWA_CLIENT+1)) &
  # if [ $TRANS_PROTOCOL = "g" ]; then
  #   $MPIRUN -npernode 1 $RI_MANAGER_NODE \
  #     globus-gridftp-server -aa -password-file pwfile -c None \
  #                           -port RI_MANAGER_DATA_GFTP_LPORT \
  #                           -d error,warn,info,dump,all &
  # fi
  # sleep 1
  
  # export GLOG_logtostderr=1
  # $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr $RI_MANAGER_NODE \
  #   $DSPACESWA_DIR/exp --type="ri" --cl_id=111 --num_peer=1 --base_client_id=$(($2*$NUM_DSPACESWA_CLIENT)) \
  #                         --lcontrol_lintf=$RI_MANAGER_LCONTROL_LINTF --lcontrol_lport=$RI_MANAGER_LCONTROL_LPORT \
  #                         --ds_id=$2 --control_lintf=$RI_MANAGER_CONTROL_LINTF --control_lport=$RI_MANAGER_CONTROL_LPORT \
  #                         --join_control_lip=$RI_MANAGER_JOIN_CONTROL_LIP --join_control_lport=$RI_MANAGER_JOIN_CONTROL_LPORT \
  #                         --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$RI_MANAGER_IB_LINTF \
  #                         --tcp_lintf=$RI_MANAGER_TCP_LINTF --tcp_lport=$RI_MANAGER_TCP_LPORT \
  #                         --w_prefetch=$W_PREFETCH &
  #                         --gftp_lintf=$RI_MANAGER_GFTP_LINTF --gftp_lport=$RI_MANAGER_GFTP_LPORT --tmpfs_dir=$RI_MANAGER_TMPFS_DIR \
elif [ $1  = 'k' ]; then
  for i in "$DS_NODE"
  do
    $MPIRUN -npernode 1 $i $PKILL -f dataspaces_server
  done
  
  for i in "$RI_MANAGER_NODE"
  do
    $MPIRUN -npernode 1 $i $PKILL -f ./exp
    $MPIRUN -npernode 1 $i $PKILL -f ./mput
    $MPIRUN -npernode 1 $i $PKILL -f ./dataspaces_server
    # $MPIRUN -npernode 1 $i $PKILL -f globus-gridftp-server
  done
else
  echo "Argument did not match!"
fi
