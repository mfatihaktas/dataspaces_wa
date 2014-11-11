#!/bin/bash
echo $1 $2 $3

NUM_DSPACES_SERVERS=2
NUM_DSPACES_CLIENT_APPS=1
NUM_DSPACES_CNODES=$((NUM_DSPACES_CLIENT_APPS + 1)) #+1 for ri_manager

# DSPACES_DIR, DSPACESWA_DIR is set with ". run.sh init ?"
DSPACES_BIN_DIR=$DSPACES_DIR/bin
# DSPACES_BIN_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.4.0/install/bin
DSPACESWA_BIN_DIR=$DSPACESWA_DIR
# DSPACESWA_BIN_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
# 
TRANS_PROTOCOL="i" #i:infiniband, g:gridftp
RI_MANAGER_APP_ID=10 #$((NUM_DSPACES_CNODES+1))
RI_MANAGER_CONTROL_LINTF="em2" #"eth0"
RI_MANAGER_CONTROL_LPORT="65000"
RI_MANAGER_DATA_LINTF="ib0" #"em2" #"ib0"
RI_MANAGER_DATA_GFTP_LPORT="62000"
RI_MANAGER_DATA_TMPFS_DIR="/dev/shm"

if [ $1  = 'r' ]; then
  export GLOG_logtostderr=1
  #with openmpi
  rm conf               #dataspaces_server cannot overwrite this so before every new run this should be removed
  # /usr/lib64/openmpi/bin/mpirun -npernode 1 -hostfile $DSPACES_SERVER_HOSTFILE \
  #   $DSPACES_BIN_DIR/dataspaces_server --server $NUM_DSPACES_SERVERS --cnodes $NUM_DSPACES_CNODES &
  
  /usr/lib64/openmpi/bin/mpirun -npernode 1 -host dell02 -host dell03 \
    $DSPACES_BIN_DIR/dataspaces_server --server $NUM_DSPACES_SERVERS --cnodes $NUM_DSPACES_CNODES &
  
  sleep 2
  
  /usr/lib64/openmpi/bin/mpirun -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr -hostfile $RI_MANAGER_HOSTFILE \
    $DSPACESWA_BIN_DIR/exp --type="ri" --dht_id=2 --num_dscnodes=$NUM_DSPACES_CNODES --app_id=$RI_MANAGER_APP_ID \
                          --dht_lintf=$RI_MANAGER_CONTROL_LINTF --dht_lport=$RI_MANAGER_CONTROL_LPORT \
                          --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$RI_MANAGER_DATA_LINTF \
                          --gftp_lport=$RI_MANAGER_DATA_GFTP_LPORT --tmpfs_dir=$RI_MANAGER_DATA_TMPFS_DIR &
  
  # /usr/lib64/openmpi/bin/mpirun -npernode 1 -x LD_LIBRARY_PATH -x GLOG_logtostderr -hostfile $RI_MANAGER_HOSTFILE \
  #   valgrind $DSPACESWA_BIN_DIR/exp --type="ri" --dht_id=2 --num_dscnodes=$NUM_DSPACES_CNODES --app_id=$RI_MANAGER_APP_ID \
  #                         --dht_lintf=$RI_MANAGER_CONTROL_LINTF --dht_lport=$RI_MANAGER_CONTROL_LPORT \
  #                         --trans_protocol=$TRANS_PROTOCOL --wa_lintf=$RI_MANAGER_DATA_LINTF \
  #                         --gftp_lport=$RI_MANAGER_DATA_GFTP_LPORT --tmpfs_dir=$RI_MANAGER_DATA_TMPFS_DIR &
  
  # /usr/lib64/openmpi/bin/mpirun -npernode 1 -x LD_LIBRARY_PATH -hostfile $RI_MANAGER_HOSTFILE \
  #   $DSPACESWA_BIN_DIR/exp --type="get" --num_dscnodes=$NUM_DSPACES_CNODES --app_id=1
  #intelmpi
elif [ $1  = 'k' ]; then
  /usr/lib64/openmpi/bin/mpirun -npernode 1 -hostfile $RI_MANAGER_HOSTFILE /usr/bin/pkill -f ./exp
  # /usr/lib64/openmpi/bin/mpirun -npernode 1 -hostfile $DSPACES_SERVER_HOSTFILE /usr/bin/pkill -f dataspaces_server
  /usr/lib64/openmpi/bin/mpirun -npernode 1 -host dell02 -host dell03 /usr/bin/pkill -f dataspaces_server
  
else
  echo "Argument did not match !"
fi
