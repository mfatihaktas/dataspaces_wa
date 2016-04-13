#!/bin/bash
echo $1 $2 $3

if [ -n "$DELL" ]; then
  # echo "# hosts to run ds on
  # dell07 slots=16 max_slots=16
  # dell08 slots=16 max_slots=16
  # dell09 slots=16 max_slots=16
  # dell10 slots=16 max_slots=16
  # dell11 slots=16 max_slots=16
  # dell12 slots=16 max_slots=16
  # dell13 slots=16 max_slots=16
  # dell14 slots=16 max_slots=16
  # dell15 slots=16 max_slots=16
  # dell16 slots=16 max_slots=16
  # " > DS_HOST_FILE
  echo "# hosts to run ds on
  dell07 slots=16 max_slots=16
  dell08 slots=16 max_slots=16
  dell09 slots=16 max_slots=16
  dell10 slots=16 max_slots=16
  dell11 slots=16 max_slots=16
  dell12 slots=16 max_slots=16
  dell13 slots=16 max_slots=16
  dell14 slots=16 max_slots=16
  dell15 slots=16 max_slots=16
  dell16 slots=16 max_slots=16
  " > DS_HOST_FILE_0
  echo "# hosts to run ds on
  dell17 slots=16 max_slots=16
  dell18 slots=16 max_slots=16
  dell19 slots=16 max_slots=16
  dell20 slots=16 max_slots=16
  dell21 slots=16 max_slots=16
  dell22 slots=16 max_slots=16
  dell23 slots=16 max_slots=16
  dell24 slots=16 max_slots=16
  dell25 slots=16 max_slots=16
  dell26 slots=16 max_slots=16
  " > DS_HOST_FILE_1
  echo "# hosts to run apps on
  dell03 slots=16 max_slots=16
  dell04 slots=16 max_slots=16
  dell05 slots=16 max_slots=16
  dell06 slots=16 max_slots=16
  " > APP_HOST_FILE
  
  # For site 0
  DS_NODES_0=""
  BEGIN_DSNODE_ID=7
  END_DSNODE_ID=$(($BEGIN_DSNODE_ID + 9))
  for i in `seq $BEGIN_DSNODE_ID $END_DSNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    DS_NODES_0+="dell$STR$i"
    [ "$i" -lt $END_DSNODE_ID ] && DS_NODES_0+=","
  done
  echo "DS_NODES_0= $DS_NODES_0"
  # For site 1
  DS_NODES_1=""
  BEGIN_DSNODE_ID=17
  END_DSNODE_ID=$(($BEGIN_DSNODE_ID + 9))
  for i in `seq $BEGIN_DSNODE_ID $END_DSNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    DS_NODES_1+="dell$STR$i"
    [ "$i" -lt $END_DSNODE_ID ] && DS_NODES_1+=","
  done
  echo "DS_NODES_1= $DS_NODES_1"
  
  PUTTER_NODES=""
  BEGIN_PNODE_ID=3
  END_PNODE_ID=$(($BEGIN_PNODE_ID + 3))
  for i in `seq $BEGIN_PNODE_ID $END_PNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    PUTTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_PNODE_ID ] && PUTTER_NODES+=","
  done
  echo "PUTTER_NODES= $PUTTER_NODES"
    
  GETTER_NODES=""
  BEGIN_GNODE_ID=3
  END_GNODE_ID=$(($BEGIN_GNODE_ID + 3))
  for i in `seq $BEGIN_GNODE_ID $END_GNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    GETTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_GNODE_ID ] && GETTER_NODES+=","
  done
  echo "GETTER_NODES= $GETTER_NODES"
  
  # APP_NODES=""
  # BEGIN_ANODE_ID=4
  # END_ANODE_ID=$(($BEGIN_ANODE_ID + 2))
  # for i in `seq $BEGIN_ANODE_ID $END_ANODE_ID`; do
  #   STR=""
  #   [ "$i" -lt 10 ] && STR+="0"
  #   APP_NODES+="dell$STR$i"
  #   [ "$i" -lt $END_ANODE_ID ] && APP_NODES+=","
  # done
  # echo "APP_NODES= $APP_NODES"
  
  LCONTROL_LINTF="em2"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

RI_LCONTROL_LPORT_LIST=( 8000 9000 )

NUM_DS=1 # 32
NUM_CLIENT=2
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PUTGET=5

PKILL=/usr/bin/pkill

if [ -z "$2" ]; then
echo "Which site [0, *]?"
elif [[ $1  == 's' || $1  == 'ds' ]]; then
  [ $2 = 0 ] && rm *.log
  [ -a conf ] && rm srv.lck conf dataspaces.conf
  DEBUG=
  GDB="xterm -e gdb --args"
  VALGRIND="valgrind --vgdb=yes"
  [ $1  = 'ds' ] && DEBUG=$GDB
  
  echo "# Config file for DataSpaces
  ndim = 3
  dims = 1024,1024,1024
  max_versions = 1
  max_readers = 1
  lock_type = 1
  " > dataspaces.conf
  
  # Note: This may cause conf file to fail to assign who is master -- all may decide to be a master
  # LOG_F="ds_$2.log"
  # $MPIRUN --hostfile DS_HOST_FILE_$2 -n $NUM_DS --bynode $DEBUG \
  #   $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
  #                                     --cnodes $NUM_DSCNODE > $LOG_F 2>&1 < /dev/null & # < /dev/null 2>&1 | tee $LOG_F & #
  NODES=DS_NODES_$2
  eval NODES=\$$NODES
  NODE_LIST=(${NODES//,/ } )
  for i in `seq 1 $NUM_DS`; do
    NODE=${NODE_LIST[$(($(($i - 1)) % ${#NODE_LIST[@]} )) ] }
    echo "run dataspaces_server $i on $NODE"
    LOG_F="ds_"$2"_"$i".log"
    $MPIRUN -n 1 -host $NODE $DEBUG \
      $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
                                         --cnodes $NUM_DSCNODE < /dev/null 2>&1 | tee $LOG_F & # > $LOG_F 2>&1 < /dev/null & #
    if [ $i == 1 ]; then
      sleep 1 # to make sure only one ds_master
    fi
  done
# elif [ $1 = "den" ]; then
elif [[ $1 == 'map' || $1 == 'dmap' || $1 == 'mag' || $1 == 'dmag' ]]; then
  GDB=
  TYPE="mput"
  [[ $1 = 'mag' || $1 = 'dmag' ]] && TYPE="mget"
  [[ $1 == 'dmap' || $1 == 'dmag' ]] && GDB="xterm -e gdb --args"
  
  export GLOG_logtostderr=1
  # Note: This causes dspaces_put/get block mpi processes
  # LOG_F="$1.log"
  # $MPIRUN -x LD_LIBRARY_PATH -x GLOG_logtostderr --hostfile APP_HOST_FILE -n $NUM_CLIENT --bynode $GDB \
  #   ./mput_mget_test --type=$TYPE --cl_id=0 --base_client_id=$(($2*$NUM_CLIENT)) \
  #                   --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
  #                   --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
  #                   --num_putget=$NUM_PUTGET < /dev/null 2>&1 | tee $LOG_F & # > $LOG_F 2>&1 < /dev/null &
  
  NODES=$PUTTER_NODES
  [[ $1 = 'mag' || $1 = 'dmag' ]] && NODES=$GETTER_NODES
  NODE_LIST=(${NODES//,/ } )
  for i in `seq 1 $NUM_CLIENT`; do
    NODE=${NODE_LIST[$(($(($i - 1)) % ${#NODE_LIST[@]} )) ] }
    LOG_F="s_"$2"_"$TYPE"_cid_"$i"_"$NODE".log"
    echo "run s= $2, type= $1, c_id= $i on $NODE"
    
    $MPIRUN -x LD_LIBRARY_PATH -x GLOG_logtostderr -npernode 1 -host $NODE $GDB \
      ./mput_mget_test --type=$TYPE --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) \
                      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
                      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
                      --num_putget=$NUM_PUTGET > $LOG_F 2>&1 < /dev/null & # < /dev/null 2>&1 | tee $LOG_F & # 
  done
elif [ $1  = 'k' ]; then
  # Sometimes works sometimes not!
  # $MPIRUN --hostfile DS_HOST_FILE -np 32 --bynode $PKILL -f dataspaces_server
  # $MPIRUN --hostfile APP_HOST_FILE -np 32 --bynode $PKILL -f mput_mget_test
  NODE_LIST=(${DS_NODES_0//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    echo "$NODE $PKILL -f dataspaces_server"
    $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
  done
  NODE_LIST=(${DS_NODES_1//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    echo "$NODE $PKILL -f dataspaces_server"
    $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
  done
  NODE_LIST=(${PUTTER_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f mput_mget_test
  done
  NODE_LIST=(${GETTER_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f mput_mget_test
  done
  # NODE_LIST=(${APP_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f mput_mget_test
  # done
else
  echo "Argument did not match!"
fi
