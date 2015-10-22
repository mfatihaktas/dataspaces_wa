#!/bin/bash
echo $1 $2 $3

NUM_DS_NODE_LIST=( 2 2 )
DS_NODE_LIST=( "-host archer1 -host archer2" "-host archer1 -host archer2" )
RI_MANAGER_NODE_LIST=( "-host archer1" "-host archer2" )
NUM_DSPACESWA_CLIENT_LIST=( 1 1 )

DSPACES_BIN_DIR=$DSPACES_DIR/bin                                                                    # DSPACES_DIR, DSPACESWA_DIR is set with ". run.sh init ?"
DSPACESWA_BIN_DIR=$DSPACESWA_DIR

TRANS_PROTOCOL="t"                                                                                  #i:infiniband, t:tcp, g:gridftp
TMPFS_DIR=""
# 
RI_MANAGER_LCONTROL_LINTF_LIST=( "eth0" "eth0" )
RI_MANAGER_LCONTROL_LPORT_LIST=( 8000 8000 )
RI_MANAGER_CONTROL_LINTF_LIST=( "eth0" "eth0" )
RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 )
RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 )
RI_MANAGER_IB_LINTF_LIST=( "ib0" "ib0" )
RI_MANAGER_TCP_LINTF_LIST=( "eth0" "eth0" )
RI_MANAGER_TCP_LPORT_LIST=( 6000 6000 )
RI_MANAGER_GFTP_LINTF_LIST=( "eth0" "eth0" )
RI_MANAGER_GFTP_LPORT_LIST=( 6000 6000 )
RI_MANAGER_TMPFS_DIR_LIST=( $TMPFS_DIR"/put" $TMPFS_DIR"/get" )

MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
PKILL=/usr/bin/pkill
if [ $1  = 'r' ]; then
  export GLOG_logtostderr=1
  #with openmpi
  if [ -z "$2" ]; then
    echo "which site? 0-1"
  else
    if [ -a conf ]; then
      rm conf                                                                                       #dataspaces_server cannot overwrite this so before every new run this should be removed
    fi
    
    $MPIRUN -npernode 1 ${DS_NODE_LIST[$2]} \
      /home/sc14demo/common-apps/dataspaces-1.5.0/install/bin/dataspaces_server \
      --server ${NUM_DS_NODE_LIST[$2]} --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]}+1)) &
    # if [ $TRANS_PROTOCOL = "g" ]; then
    #   $MPIRUN -npernode 1 ${RI_MANAGER_NODE_LIST[$2]} \
    #     globus-gridftp-server -aa -password-file pwfile -c None \
    #                           -port ${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
    #                           -d error,warn,info,dump,all &
    # fi
    # sleep 1
    
    # $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr ${RI_MANAGER_NODE_LIST[$2]} \
    #   $DSPACESWA_BIN_DIR/exp --type="ri" --cl_id=111 --num_client=${NUM_DSPACESWA_CLIENT_LIST[$2]} --base_client_id=$(($2*10)) \
    #                         --lcontrol_lintf=${RI_MANAGER_LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2]} \
    #                         --ds_id=$2 --control_lintf=${RI_MANAGER_CONTROL_LINTF_LIST[$2]} --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2]} --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2]} --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2]} \
    #                         --trans_protocol=$TRANS_PROTOCOL --ib_lintf=${RI_MANAGER_IB_LINTF_LIST[$2]} \
    #                         --tcp_lintf=${RI_MANAGER_TCP_LINTF_LIST[$2]} --tcp_lport=${RI_MANAGER_TCP_LPORT_LIST[$2]} \
    #                         --gftp_lintf=${RI_MANAGER_GFTP_LINTF_LIST[$2]} --gftp_lport=${RI_MANAGER_GFTP_LPORT_LIST[$2]} --tmpfs_dir=${RI_MANAGER_TMPFS_DIR_LIST[$2]} &
  fi
elif [ $1  = 'k' ]; then
  for i in "${DS_NODE_LIST[@]}"
  do
    $MPIRUN -npernode 1 $i $PKILL -f dataspaces_server
  done
  
  for i in "${RI_MANAGER_NODE_LIST[@]}"
  do
    $MPIRUN -npernode 1 $i $PKILL -f ./exp
    $MPIRUN -npernode 1 $i $PKILL -f globus-gridftp-server
  done
else
  echo "Argument did not match!"
fi
