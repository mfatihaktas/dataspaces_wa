#!/bin/bash
echo $1 $2 $3
# A script to automate dataspaces_server and ri_manager running process.
# *** This script must be run from the same directory where the getter and putter binaries are run.
# *** Gridftp option is not fully tested, for now please use Infiniband.
# Suppose there are two applications, putter and getter.
#  
#  getter    RI_manager---------- TCP/IP ---------RI_manager    putter
#    |           |     \.......RDMA, Gridftp...../    |           |
#  dataspaces_server                                dataspaces_server
# In this example, there are two "sites": site 0 on which getter is served and site 1 on which putter
# is served.
# 
# As one can see below there are some lists defined to input bootstrap info. Brief explanations:
# NUM_DS_NODE_LIST[i]= Number of nodes dataspaces_server will be run on site i.
# DS_NODE_LIST[i]= \n separated list of node names where dataspaces_server will be run on site i.
# RI_MANAGER_NODE_LIST[i]= Node name where ri_manager on site i will be run. PS: Ri_manager 
# can run only on a single node.
# NUM_DSPACESWA_CLIENT_LIST[i]= Number of applications at each site. In this case there is only putter
# and getter at site 0 and 1.
# RI_MANAGER_CONTROL_LINTF_LIST[i]= Network interface Ri_manager[i] will listen for control connections.
# RI_MANAGER_CONTROL_LPORT_LIST[i]= TCP/IP port Ri_manager[i] will listen on for control connections.
# RI_MANAGER_CONTROL_CONNECT_TO_LADDR_LIST[i]= IP address of the Ri_manager, Ri_manager[i] will connect to.
# RI_MANAGER_CONTROL_CONNECT_TO_LPORT_LIST[i]= TCP/IP port of the Ri_manager listening for control 
# connections. Ri_manager[i] will connect to
# RI_MANAGER_CONTROL_CONNECT_TO_LADDR_LIST[i]:RI_MANAGER_CONTROL_CONNECT_TO_LPORT_LIST[i] 
# 
# Values given below are for the following setup at Ulam
# 
#  getter    RI_manager---------- TCP/IP ----------RI_manager   putter
# [ulam10]   [ulam10]  \                          /[ulam11]    [ulam11]
#    |           |     \.......RDMA, Gridftp...../    |           |
#  dataspaces_server                                dataspaces_server
# [ulam10,11,12,13]                                 [ulam14,15,16,17]
# 
# To test if the setup is working, one can do
# - Small test:
# Run site 0
# ulam10$ cd /home/sc14demo/common-apps/dataspaces_wa
# ulam10$ . run.sh init u
# ulam10$ ./intelmpirun.sh r 0
# ulam10$ ./run.sh g
# Run site 1
# ulam11$ cd /home/sc14demo/common-apps/dataspaces_wa
# ulam11$ . run.sh init u
# ulam11$ ./intelmpirun.sh r 1
# ulam11$ ./run.sh p
# - Nstx_demo test:
# Run site 0
# ulam10$ cd /home/sc14demo/fusion/dataspaces_wa_nstx-sc14-demo
# ulam10$ . run.sh init u
# ulam10$ ./intelmpirun.sh r 0
# ulam10$ ./run.sh g
# Run site 1
# ulam11$ cd /home/sc14demo/fusion/dataspaces_wa_nstx-sc14-demo
# ulam11$ . run.sh init u
# ulam11$ ./intelmpirun.sh r 1
# ulam11$ ./run.sh p

NUM_DS_NODE_LIST=( 4 4 )
DS_NODE_LIST=( "ulam10\nulam11\nulam12\nulam13" "ulam14\nulam15\nulam16\nulam17" )
RI_MANAGER_NODE_LIST=( "ulam10" "ulam11" )
NUM_DSPACESWA_CLIENT_LIST=( 1 1 )

DS_NODE_FILE_LIST=( ds_node_file_0 ds_node_file_1 )
RI_MANAGER_NODE_FILE_LIST=( ri_manager_node_file_0 ri_manager_node_file_1 )

DSPACES_BIN_DIR=$DSPACES_DIR/bin                                                                    # DSPACES_DIR, DSPACESWA_DIR is set with ". run.sh init ?"
DSPACESWA_BIN_DIR=$DSPACESWA_DIR

TRANS_PROTOCOL="i"                                                                                  #i:infiniband, g:gridftp
#
RI_MANAGER_APP_ID_LIST=( 10 10 )                                                                    #$((NUM_DSPACES_CNODES+1))
RI_MANAGER_CONTROL_LINTF_LIST=( "eth0" "eth0" )                                                       #"eth0"
RI_MANAGER_CONTROL_LPORT_LIST=( "60000" "61000" )
RI_MANAGER_CONTROL_CONNECT_TO_LADDR_LIST=( "" "192.168.100.111" )
RI_MANAGER_CONTROL_CONNECT_TO_LPORT_LIST=( "" "60000" )
RI_MANAGER_DATA_LINTF_LIST=( "ib0" "ib0" )                                                          #"em2"
RI_MANAGER_DATA_GFTP_LPORT_LIST=("60100" "61100" )
RI_MANAGER_DATA_TMPFS_DIR_LIST=("/dev/shm" "/dev/shm" )

MPIRUN=/opt/intel/impi/4.1.3.048/intel64/bin/mpirun
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
    
    echo -e ${DS_NODE_LIST[$2]} > ${DS_NODE_FILE_LIST[$2]}
    $MPIRUN -ppn 1 -f ${DS_NODE_FILE_LIST[$2]} \
      $DSPACES_BIN_DIR/dataspaces_server --server ${NUM_DS_NODE_LIST[$2]} \
                                         --cnodes $((${NUM_DSPACESWA_CLIENT_LIST[$2]}+1)) &
    sleep 2
    
    echo -e ${RI_MANAGER_NODE_LIST[$2]} > ${RI_MANAGER_NODE_FILE_LIST[$2]}
    $MPIRUN -ppn 1 -envall -f ${RI_MANAGER_NODE_FILE_LIST[$2]} \
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
  for i in "${DS_NODE_FILE_LIST[@]}"
  do
    if [ -a $i ]; then
      $MPIRUN -ppn 1 -f $i $PKILL -f dataspaces_server
    fi
  done
  
  for i in "${RI_MANAGER_NODE_FILE_LIST[@]}"
  do
    if [ -a $i ]; then
      $MPIRUN -ppn 1 -f $i $PKILL -f ./exp
    fi
  done
else
  echo "Argument did not match!"
fi
