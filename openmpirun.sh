#!/bin/bash
echo $1 $2 $3
# Note: This file can be used for running dataspaces_server and ri_manager for wa_dataspaces while both
# producer and consumer sites are on the same local cluster which is defined by the env variable DELL, ULAM, KID

if [ -n "$DELL" ]; then
  NUM_DS_NODE_LIST=( 4 4 )
  # DS_NODE_LIST=( "-host dell11 -host dell12 -host dell13 -host dell14" "-host dell16 -host dell17 -host dell18 -host dell19" )
  # DS_NODE_LIST=( "-host dell01" "-host dell02" )
  DS_NODE_LIST=( "dell11,dell12,dell13,dell14" "dell16,dell17,dell18,dell19" )
  RI_MANAGER_NODE_LIST=( "dell01" "dell02" )
  DSPACESWA_CNODE_LIST=( "dell21,dell22,dell23,dell24,dell25" "dell26,dell27,dell28,dell29,dell30" )
  
  LCONTROL_LINTF_LIST=( "em2" "em2" )
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "em2" "em2" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "em2" "em2" )
  GFTP_LINTF_LIST=( "em2" "em2" )
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
elif [ -n "$ULAM" ]; then
  NUM_DS_NODE_LIST=( 2 2 )
  DS_NODE_LIST=( "-host ulam -host archer1" "-host archer1 -host archer2" )
  # DS_NODE_LIST=( "-host ulam -host archer1 -host archer2" "-host ulam -host archer1 -host archer2" )
  RI_MANAGER_NODE_LIST=( "-host ulam" "-host archer2" )
  # DSPACESWA_CNODE_LIST=( "dell11" "dell12" )
  
  LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  # APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.100" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "eth0" "eth0" )
  GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
elif [ -n "$KID" ]; then
  NUM_DS_NODE_LIST=( 8 8 )
  DS_NODE_LIST=( "-host41 -host kid42 -host kid43 -host kid44 -host kid45 -host kid46 -host kid47 -host kid48" "-host kid49 -host kid50 -host kid51 -host kid52 -host kid53 -host kid54 -host kid55 -host kid56" )
  RI_MANAGER_NODE_LIST=( "-host kid42" "-host kid43" )
  # DSPACESWA_CNODE_LIST=( "dell11" "dell12" )
  
  LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  # APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_MANAGER_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.52" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "eth0" "eth0" )
  GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/net/hj1/ihpcl/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

NUM_CLIENT=10
NUM_DSPACESWA_CLIENT_LIST=( $NUM_CLIENT $NUM_CLIENT )
NUM_PEER=1
NUM_PUTGET=10

RI_MANAGER_LCONTROL_LPORT_LIST=( 9000 9000 )

RI_MANAGER_CONTROL_LPORT_LIST=( 7000 7001 )
RI_MANAGER_JOIN_CONTROL_LPORT_LIST=( 0 7000 )

W_PREFETCH=1
TRANS_PROTOCOL="t"                                                                                  #i:infiniband, t:tcp, g:gridftp
TMPFS_DIR=""

TCP_LPORT_LIST=( 6000 6000 )
GFTP_LPORT_LIST=( 6000 6000 )
TMPFS_DIR_LIST=( $TMPFS_DIR"/put" $TMPFS_DIR"/get" )
# 
PKILL=/usr/bin/pkill
if [ -z "$2" ]; then
  echo "which site? 0 | 1"
elif [[ $1  == 'mp' || $1  == 'mg' ]]; then
  TYPE="mput"
  [ $1  = 'mp' ] && rm *.log || echo
  [ $1  = 'mg' ] && TYPE="mget" || echo
  export GLOG_logtostderr=1
  
  CNODES=(${DSPACESWA_CNODE_LIST[$2]//,/ } )
  for i in `seq 1 $NUM_CLIENT`; do
    NODE=${CNODES[$(($(($i - 1)) % ${#CNODES[@]} )) ] }
    LOG_F="mput_s_"$2"_cid_"$i"_"$NODE".log"
    echo "will run dspaces_wa client $i on $NODE; TYPE= $TYPE"
    
    $MPIRUN -x LD_LIBRARY_PATH -x GLOG_logtostderr -npernode 1 -host $NODE \
      $DSPACESWA_DIR/mput_mget_test --type=$TYPE --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
        --lcontrol_lintf=${LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=$((${RI_MANAGER_LCONTROL_LPORT_LIST[$2]} + $i)) \
        --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2]} --join_lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2]} \
        --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 > $LOG_F 2>&1 < /dev/null &
  done
elif [ $1  = 'r' ]; then
  if [ -a conf ]; then
    rm srv.lck
    rm conf # dataspaces_server cannot overwrite this so before every new run this should be removed
  fi
  
  # $MPIRUN -npernode 1 ${DS_NODE_LIST[$2]} \
  #   $DSPACES_DIR/bin/dataspaces_server --server ${NUM_DS_NODE_LIST[$2]} \
  #                                     --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]} + 1)) &
  DSNODES=(${DS_NODE_LIST[$2]//,/ } )
  for node in "${DSNODES[@]}"; do
    LOG_F="ds_s_"$2"_"$node".log"
    echo "will run dataspaces_server on $node"
    
    $MPIRUN -npernode 1 -host $node \
      $DSPACES_DIR/bin/dataspaces_server --server ${#DSNODES[@]} \
                                         --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]} + 1)) > $LOG_F 2>&1 < /dev/null &
  done
  
  # if [ $TRANS_PROTOCOL = "g" ]; then
  #   $MPIRUN -npernode 1 ${RI_MANAGER_NODE_LIST[$2]} \
  #     globus-gridftp-server -aa -password-file pwfile -c None \
  #                           -port ${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
  #                           -d error,warn,info,dump,all &
  # fi
  sleep 1
  
  LOG_F="ri_s_"$2".log"
  echo "will run ri_manager on "${RI_MANAGER_NODE_LIST[$2]}
  
  export GLOG_logtostderr=1
  $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr -host ${RI_MANAGER_NODE_LIST[$2]} \
    $DSPACESWA_DIR/exp --type="ri" --cl_id=111 --num_peer=1 --base_client_id=$(($2*${NUM_DSPACESWA_CLIENT_LIST[$2]} )) \
                      --lcontrol_lintf=${LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=${RI_MANAGER_LCONTROL_LPORT_LIST[$2]} \
                      --ds_id=$2 --control_lintf=${CONTROL_LINTF_LIST[$2]} --control_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2]} \
                      --join_control_lip=${RI_MANAGER_JOIN_CONTROL_LIP_LIST[$2]} --join_control_lport=${RI_MANAGER_JOIN_CONTROL_LPORT_LIST[$2]} \
                      --trans_protocol=$TRANS_PROTOCOL --ib_lintf=${IB_LINTF_LIST[$2]} \
                      --tcp_lintf=${TCP_LINTF_LIST[$2]} --tcp_lport=${TCP_LPORT_LIST[$2]} \
                      --gftp_lintf=${GFTP_LINTF_LIST[$2]} --gftp_lport=${GFTP_LPORT_LIST[$2]} --tmpfs_dir=${TMPFS_DIR_LIST[$2]} \
                      --w_prefetch=$W_PREFETCH > $LOG_F 2>&1 < /dev/null &
elif [ $1  = 'k' ]; then
  # for node in "${DS_NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 $node $PKILL -f dataspaces_server
  # done
  for nodes in "${DS_NODE_LIST[@]}"; do
    DSNODES=(${nodes//,/ } )
    for node in "${DSNODES[@]}"; do
      $MPIRUN -npernode 1 -host $node $PKILL -f dataspaces_server
    done
  done
  for node in "${RI_MANAGER_NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $node $PKILL -f ./exp
    # $MPIRUN -npernode 1 $node $PKILL -f ./mput
    # $MPIRUN -npernode 1 $node $PKILL -f ./dataspaces_server
    # $MPIRUN -npernode 1 $node $PKILL -f globus-gridftp-server
  done
  for nodes in "${DSPACESWA_CNODE_LIST[@]}"; do
    CNODES=(${nodes//,/ } )
    for node in "${CNODES[@]}"; do
      $MPIRUN -npernode 1 -host $node $PKILL -f ./mput
    done
  done
else
  echo "Argument did not match!"
fi
