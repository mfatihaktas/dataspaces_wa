#!/bin/bash
echo $1 $2 $3
# Note: This file can be used for running dataspaces_server and ri_manager for wa_dataspaces while both
# producer and consumer sites are on the same local cluster which is defined by the env variable DELL, ULAM, KID

if [ -n "$DELL" ]; then
  # DS_NODES_LIST=( "dell01" "dell02" )
  DS_NODES_LIST=( "dell11,dell12,dell13,dell14" "dell16,dell17,dell18,dell19" )
  RI_NODES_LIST=( "dell01" "dell02" )
  DSWA_CLIENT_NODES_LIST=( "dell21,dell22,dell23,dell24,dell25" "dell26,dell27,dell28,dell29,dell30" )
  
  LCONTROL_LINTF_LIST=( "em2" "em2" )
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "em2" "em2" )
  RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "em2" "em2" )
  GFTP_LINTF_LIST=( "em2" "em2" )
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
elif [ -n "$ULAM" ]; then
  DS_NODES_LIST=( "-host ulam -host archer1" "-host archer1 -host archer2" )
  # DS_NODES_LIST=( "-host ulam -host archer1 -host archer2" "-host ulam -host archer1 -host archer2" )
  RI_NODES_LIST=( "-host ulam" "-host archer2" )
  # DSWA_CLIENT_NODES_LIST=( "dell11" "dell12" )
  
  LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  # APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.100" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "eth0" "eth0" )
  GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/apps/openmpi/1.8.2/gcc-4.8.3/bin/mpirun
elif [ -n "$KID" ]; then
  DS_NODES_LIST=( "-host41 -host kid42 -host kid43 -host kid44 -host kid45 -host kid46 -host kid47 -host kid48" "-host kid49 -host kid50 -host kid51 -host kid52 -host kid53 -host kid54 -host kid55 -host kid56" )
  RI_NODES_LIST=( "-host kid42" "-host kid43" )
  # DSWA_CLIENT_NODES_LIST=( "dell11" "dell12" )
  
  LCONTROL_LINTF_LIST=( "eth0" "eth0" )
  # APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF_LIST=( "eth0" "eth0" )
  RI_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.52" )
  
  IB_LINTF_LIST=( "ib0" "ib0" )
  TCP_LINTF_LIST=( "eth0" "eth0" )
  GFTP_LINTF_LIST=( "eth0" "eth0" )
  
  MPIRUN=/net/hj1/ihpcl/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

NUM_CLIENT=1
NUM_DSWA_CLIENT_LIST=( $NUM_CLIENT $NUM_CLIENT )
NUM_PEER=1
NUM_PUTGET=4

RI_LCONTROL_LPORT_LIST=( 9000 9000 )

RI_CONTROL_LPORT_LIST=( 7000 7001 )
RI_JOIN_CONTROL_LPORT_LIST=( 0 7000 )

W_PREFETCH=1
TRANS_PROTOCOL="t"                                                                                  #i:infiniband, t:tcp, g:gridftp
TMPFS_DIR=""

TCP_LPORT_LIST=( 6000 6000 )
GFTP_LPORT_LIST=( 6000 6000 )
TMPFS_DIR_LIST=( $TMPFS_DIR"/put" $TMPFS_DIR"/get" )
# 
PKILL=/usr/bin/pkill
if [ -z "$2" ]; then
  echo "Which site? 0 | 1"
elif [ $1  = 'ar' ]; then
  _1=mg
  [[ $2 == 0 ]] && { _1=mp; rm *.log; }
  
  ./openmpirun.sh r $2
  sleep 1
  ./openmpirun.sh $_1 $2
elif [[ $1  == 'mp' || $1  == 'mg' ]]; then
  TYPE="mput"
  # [ $1  = 'mp' ] && rm *.log
  [ $1  = 'mg' ] && TYPE="mget"
  export GLOG_logtostderr=1
  
  NODE_LIST=(${DSWA_CLIENT_NODES_LIST[$2]//,/ } )
  for i in `seq 1 $NUM_CLIENT`; do
    NODE=${NODE_LIST[$(($(($i - 1)) % ${#NODE_LIST[@]} )) ] }
    LOG_F=$TYPE"_s_"$2"_cid_"$i"_"$NODE".log"
    echo "run dspaces_wa client $i on $NODE; TYPE= $TYPE"
    
    $MPIRUN -x LD_LIBRARY_PATH -x GLOG_logtostderr -npernode 1 -host $NODE \
      $DSPACESWA_DIR/mput_mget_test --type=$TYPE --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) --num_peer=$NUM_PEER \
        --lcontrol_lintf=${LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=$((${RI_LCONTROL_LPORT_LIST[$2]} + $i)) \
        --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2]} --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2]} \
        --num_putget=$NUM_PUTGET --inter_time_sec=0 --sleep_time_sec=0 > $LOG_F 2>&1 < /dev/null &
  done
elif [ $1  = 'r' ]; then
  [ -a conf ] && rm srv.lck conf
  
  NODE_LIST=(${DS_NODES_LIST[$2]//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    LOG_F="ds_s_"$2"_"$NODE".log"
    echo "run dataspaces_server on $NODE"
    
    $MPIRUN -npernode 1 -host $NODE \
      $DSPACES_DIR/bin/dataspaces_server --server ${#NODE_LIST[@]} \
                                         --cnodes $((${NUM_DSWA_CLIENT_LIST[$2]} + 1)) > $LOG_F 2>&1 < /dev/null &
  done
  
  # if [ $TRANS_PROTOCOL = "g" ]; then
  #   $MPIRUN -npernode 1 ${RI_NODES_LIST[$2]} \
  #     globus-gridftp-server -aa -password-file pwfile -c None \
  #                           -port ${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
  #                           -d error,warn,info,dump,all &
  # fi
  sleep 1
  
  LOG_F="ri_s_"$2".log"
  echo "run ri_manager on "${RI_NODES_LIST[$2]}
  
  export GLOG_logtostderr=1
  $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr -host ${RI_NODES_LIST[$2]} \
    $DSPACESWA_DIR/exp --type="ri" --cl_id=111 --num_peer=1 --base_client_id=$(($2*${NUM_DSWA_CLIENT_LIST[$2]} )) \
                      --lcontrol_lintf=${LCONTROL_LINTF_LIST[$2]} --lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2]} \
                      --ds_id=$2 --control_lintf=${CONTROL_LINTF_LIST[$2]} --control_lport=${RI_CONTROL_LPORT_LIST[$2]} \
                      --join_control_lip=${RI_JOIN_CONTROL_LIP_LIST[$2]} --join_control_lport=${RI_JOIN_CONTROL_LPORT_LIST[$2]} \
                      --trans_protocol=$TRANS_PROTOCOL --ib_lintf=${IB_LINTF_LIST[$2]} \
                      --tcp_lintf=${TCP_LINTF_LIST[$2]} --tcp_lport=${TCP_LPORT_LIST[$2]} \
                      --gftp_lintf=${GFTP_LINTF_LIST[$2]} --gftp_lport=${GFTP_LPORT_LIST[$2]} --tmpfs_dir=${TMPFS_DIR_LIST[$2]} \
                      --w_prefetch=$W_PREFETCH > $LOG_F 2>&1 < /dev/null &
elif [ $1  = 'k' ]; then
  for NODES in "${DS_NODES_LIST[@]}"; do
    NODE_LIST=(${NODES//,/ } )
    for NODE in "${NODE_LIST[@]}"; do
      $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
    done
  done
  for NODE in "${RI_NODES_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f ./exp
  done
  for NODES in "${DSWA_CLIENT_NODES_LIST[@]}"; do
    NODE_LIST=(${NODES//,/ } )
    for NODE in "${NODE_LIST[@]}"; do
      $MPIRUN -npernode 1 -host $node $PKILL -f ./mput
    done
  done
else
  echo "Argument did not match!"
fi
