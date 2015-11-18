#!/bin/bash
echo $1 $2 $3
# Note: This file can be used for running dataspaces_server and ri_manager for wa_dataspaces while both
# producer and consumer sites are on the same local cluster which is defined by the env variable DELL, ULAM, KID

if [ -n "$DELL" ]; then
  NUM_DS_NODE_LIST=( 1 1 )
  # DS_NODE_LIST=( "-host dell01 -host dell03 -host dell04 -host dell05" "-host dell02 -host dell06 -host dell07 -host dell08" )
  DS_NODE_LIST=( "-host dell01" "-host dell02" )
  RI_MANAGER_NODE_LIST=( "-host dell01" "-host dell02" )

  RI_MANAGER_LCONTROL_LINTF_LIST=( "em2" "em2" )

  RI_MANAGER_CONTROL_LINTF_LIST=( "em2" "em2" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  RI_MANAGER_IB_LINTF_LIST=( "ib0" "ib0" )
  RI_MANAGER_TCP_LINTF_LIST=( "em2" "em2" )
  RI_MANAGER_GFTP_LINTF_LIST=( "em2" "em2" )
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
elif [ -n "$ULAM" ]; then
  NUM_DS_NODE_LIST=( 2 2 )
  DS_NODE_LIST=( "-host ulam -host archer1" "-host archer1 -host archer2" )
  # DS_NODE_LIST=( "-host ulam -host archer1 -host archer2" "-host ulam -host archer1 -host archer2" )
  RI_MANAGER_NODE_LIST=( "-host ulam" "-host archer2" )
  
  RI_MANAGER_LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  
  RI_MANAGER_CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.100" )
  
  RI_MANAGER_IB_LINTF_LIST=( "ib0" "ib0" )
  RI_MANAGER_TCP_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
elif [ -n "$KID" ]; then
  NUM_DS_NODE_LIST=( 8 8 )
  DS_NODE_LIST=( "-host41 -host kid42 -host kid43 -host kid44 -host kid45 -host kid46 -host kid47 -host kid48" "-host kid49 -host kid50 -host kid51 -host kid52 -host kid53 -host kid54 -host kid55 -host kid56" )
  RI_MANAGER_NODE_LIST=( "-host kid42" "-host kid43" )
  
  RI_MANAGER_LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  
  RI_MANAGER_CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.52" )
  
  RI_MANAGER_IB_LINTF_LIST=( "ib0" "ib0" )
  RI_MANAGER_TCP_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/net/hj1/ihpcl/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

NUM_DSPACESWA_CLIENT_LIST=( 1 1 )
TMPFS_DIR=""

RI_MANAGER_LCONTROL_LPORT_LIST=( 9000 9000 )

RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 )


RI_MANAGER_TCP_LPORT_LIST=( 6000 6000 )
RI_MANAGER_GFTP_LPORT_LIST=( 6000 6000 )
RI_MANAGER_TMPFS_DIR_LIST=( $TMPFS_DIR"/put" $TMPFS_DIR"/get" )

W_PREFETCH=1
TRANS_PROTOCOL="t"                                                                                  #i:infiniband, t:tcp, g:gridftp
# 
PKILL=/usr/bin/pkill
if [ -z "$2" ]; then
  echo "which site? 0 | 1"
elif [ $1  = 'r' ]; then
  if [ -a conf ]; then
    rm conf                                                                                       #dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  
  $MPIRUN -npernode 1 ${DS_NODE_LIST[$2]} \
    $DSPACES_DIR/bin/dataspaces_server --server ${NUM_DS_NODE_LIST[$2]} \
                                       --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]}+1)) &
  # if [ $TRANS_PROTOCOL = "g" ]; then
  #   $MPIRUN -npernode 1 ${RI_MANAGER_NODE_LIST[$2]} \
  #     globus-gridftp-server -aa -password-file pwfile -c None \
  #                           -port ${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
  #                           -d error,warn,info,dump,all &
  # fi
  # sleep 1
  
  # export GLOG_logtostderr=1
  # $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr ${RI_MANAGER_NODE_LIST[$2]} \
  #   $DSPACESWA_DIR/exp --type="ri" --cl_id=111 --num_peer=1 --base_client_id=$(($2*${NUM_DSPACESWA_CLIENT_LIST[$2]} )) \
  #                     --lcontrol_lintf=${RI_MANAGER_LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2]} \
  #                     --ds_id=$2 --control_lintf=${RI_MANAGER_CONTROL_LINTF_LIST[$2]} --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2]} \
  #                     --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2]} --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2]} \
  #                     --trans_protocol=$TRANS_PROTOCOL --ib_lintf=${RI_MANAGER_IB_LINTF_LIST[$2]} \
  #                     --tcp_lintf=${RI_MANAGER_TCP_LINTF_LIST[$2]} --tcp_lport=${RI_MANAGER_TCP_LPORT_LIST[$2]} \
  #                     --gftp_lintf=${RI_MANAGER_GFTP_LINTF_LIST[$2]} --gftp_lport=${RI_MANAGER_GFTP_LPORT_LIST[$2]} --tmpfs_dir=${RI_MANAGER_TMPFS_DIR_LIST[$2]} \
  #                     --w_prefetch=$W_PREFETCH &
elif [ $1  = 'k' ]; then
  for i in "${DS_NODE_LIST[@]}"
  do
    $MPIRUN -npernode 1 $i $PKILL -f dataspaces_server
  done
  
  for i in "${RI_MANAGER_NODE_LIST[@]}"
  do
    $MPIRUN -npernode 1 $i $PKILL -f ./exp
    $MPIRUN -npernode 1 $i $PKILL -f ./mput
    $MPIRUN -npernode 1 $i $PKILL -f ./dataspaces_server
    # $MPIRUN -npernode 1 $i $PKILL -f globus-gridftp-server
  done
else
  echo "Argument did not match!"
fi
