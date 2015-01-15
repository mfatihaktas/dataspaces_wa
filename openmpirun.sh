#!/bin/bash
echo $1 $2 $3

NUM_DS_NODE_LIST=( 1 1 )
# DS_NODE_LIST=( "-host dell01 -host dell02 -host dell03 -host dell04" "-host dell05 -host dell06 -host dell07 -host dell08" )
DS_NODE_LIST=( "-host dell01" "-host dell02" )
RI_MANAGER_NODE_LIST=( "-host dell01" "-host dell02" )
NUM_DSPACESWA_CLIENT_LIST=( 1 1 )

DSPACES_BIN_DIR=$DSPACES_DIR/bin                                                                    # DSPACES_DIR, DSPACESWA_DIR is set with ". run.sh init ?"
DSPACESWA_BIN_DIR=$DSPACESWA_DIR

TRANS_PROTOCOL="g"                                                                                  #i:infiniband, g:gridftp
TMPFS_DIR="/cac/u01/mfa51/Desktop/dataspaces_wa/cache"
#
RI_MANAGER_APP_ID_LIST=( 10 10 )                                                                    #$((NUM_DSPACES_CNODES+1))
RI_MANAGER_CONTROL_LINTF_LIST=( "em2" "em2" )                                                       #"eth0"
RI_MANAGER_CONTROL_LPORT_LIST=( "60000" "61000" )
RI_MANAGER_CONTROL_CONNECT_TO_LADDR_LIST=( "" "192.168.2.151" )
RI_MANAGER_CONTROL_CONNECT_TO_LPORT_LIST=( "" "60000" )
# RI_MANAGER_DATA_LINTF_LIST=( "ib0" "ib0" )                                                          #"em2"
RI_MANAGER_DATA_LINTF_LIST=( "em2" "em2" )
RI_MANAGER_DATA_GFTP_LPORT_LIST=( "60100" "61100" )
RI_MANAGER_DATA_TMPFS_DIR_LIST=( $TMPFS_DIR"/put" $TMPFS_DIR"/get" )

MPIRUN=/usr/lib64/openmpi/bin/mpirun
PKILL=/usr/bin/pkill
if [ $1  = 'r' ]; then
  export GLOG_logtostderr=1
  #with openmpi
  if [ -z "$2" ]; then
    echo "which site? 0-1"
  else
    if [ -a conf ]; then
      rm conf                                                                                         #dataspaces_server cannot overwrite this so before every new run this should be removed
    fi
    
    $MPIRUN -npernode 1 ${DS_NODE_LIST[$2]} \
      $DSPACES_BIN_DIR/dataspaces_server --server ${NUM_DS_NODE_LIST[$2]} \
                                         --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]}+1)) &
    if [ $TRANS_PROTOCOL = "g" ]; then
      $MPIRUN -npernode 1 ${RI_MANAGER_NODE_LIST[$2]} \
        globus-gridftp-server -aa -password-file pwfile -c None \
                              -port ${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
                              -d error,warn,info,dump,all &
    fi
    
    sleep 1
    
    $MPIRUN -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr ${RI_MANAGER_NODE_LIST[$2]} \
      $DSPACESWA_BIN_DIR/exp --type="ri" --dht_id=$2 \
                             --num_dscnodes=$((${NUM_DSPACESWA_CLIENT_LIST[$2]}+1)) \
                             --app_id=${RI_MANAGER_APP_ID_LIST[$2]} \
                             --dht_lintf=${RI_MANAGER_CONTROL_LINTF_LIST[$2]} \
                             --dht_lport=${RI_MANAGER_CONTROL_LPORT_LIST[$2]} \
                             --ipeer_dht_laddr=${RI_MANAGER_CONTROL_CONNECT_TO_LADDR_LIST[$2]} \
                             --ipeer_dht_lport=${RI_MANAGER_CONTROL_CONNECT_TO_LPORT_LIST[$2]} \
                             --trans_protocol=$TRANS_PROTOCOL \
                             --wa_lintf=${RI_MANAGER_DATA_LINTF_LIST[$2]} \
                             --gftp_lport=${RI_MANAGER_DATA_GFTP_LPORT_LIST[$2]} \
                             --tmpfs_dir=${RI_MANAGER_DATA_TMPFS_DIR_LIST[$2]} &
    #intelmpi
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
  echo "Argument did not match !"
fi
