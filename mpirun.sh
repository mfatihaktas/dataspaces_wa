#!/bin/bash
echo $1 $2 $3

NUM_DS=12 # 32
NUM_CLIENT=10
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PEER=1
NUM_PUTGET=10

if [ -n "$DELL" ]; then
  echo "# hosts to run ds on
  dell11 slots=16 max_slots=16
  dell12 slots=16 max_slots=16
  dell13 slots=16 max_slots=16
  dell14 slots=16 max_slots=16
  dell15 slots=16 max_slots=16
  dell16 slots=16 max_slots=16
  dell17 slots=16 max_slots=16
  dell18 slots=16 max_slots=16
  dell19 slots=16 max_slots=16
  dell20 slots=16 max_slots=16
  " > DS_HOST_FILE_0
  echo "# hosts to run ds on
  dell21 slots=16 max_slots=16
  dell22 slots=16 max_slots=16
  dell23 slots=16 max_slots=16
  dell24 slots=16 max_slots=16
  dell25 slots=16 max_slots=16
  dell26 slots=16 max_slots=16
  dell27 slots=16 max_slots=16
  dell28 slots=16 max_slots=16
  dell29 slots=16 max_slots=16
  dell30 slots=16 max_slots=16
  dell31 slots=16 max_slots=16
  dell32 slots=16 max_slots=16
  " > DS_HOST_FILE_1
  # echo "# hosts to run apps on
  # dell01 slots=16 max_slots=16
  # dell02 slots=16 max_slots=16
  # # dell03 slots=16 max_slots=16
  # dell04 slots=16 max_slots=16
  # dell05 slots=16 max_slots=16
  # dell06 slots=16 max_slots=16
  # dell07 slots=16 max_slots=16
  # dell08 slots=16 max_slots=16
  # dell09 slots=16 max_slots=16
  # dell10 slots=16 max_slots=16
  # " > APP_HOST_FILE
  
  BEGIN_DSNODE_ID=11
  END_DSNODE_ID=$(($BEGIN_DSNODE_ID + 2))
  DS_NODES=""
  for i in `seq $BEGIN_DSNODE_ID $END_DSNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    DS_NODES+="dell$STR$i"
    [ "$i" -lt $END_DSNODE_ID ] && DS_NODES+=","
  done
  echo "DS_NODES= $DS_NODES"
  
  BEGIN_PNODE_ID=4
  END_PNODE_ID=$(($BEGIN_PNODE_ID + 2))
  PUTTER_NODES=""
  for i in `seq $BEGIN_PNODE_ID $END_PNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    PUTTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_PNODE_ID ] && PUTTER_NODES+=","
  done
  echo "PUTTER_NODES= $PUTTER_NODES"
  
  BEGIN_GNODE_ID=7
  END_GNODE_ID=$(($BEGIN_GNODE_ID + 2))
  GETTER_NODES=""
  for i in `seq $BEGIN_GNODE_ID $END_GNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    GETTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_GNODE_ID ] && GETTER_NODES+=","
  done
  echo "GETTER_NODES= $GETTER_NODES"
  
  # PUTTER_NODES="dell01,dell02,dell03,dell04,dell05"
  # GETTER_NODES="dell06,dell07,dell08,dell09,dell10"
  
  LCONTROL_LINTF="em2"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

RI_LCONTROL_LPORT_LIST=( 9000 9000 )

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
  dims = 2048,2048,2048
  max_versions = 1
  max_readers = 1 
  lock_type = 2
  " > dataspaces.conf
  
  LOG_F="ds.log"
  $MPIRUN --hostfile DS_HOST_FILE_$2 -n $NUM_DS --byslot $DEBUG \
    $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
                                       --cnodes $NUM_DSCNODE > $LOG_F 2>&1 < /dev/null &
# elif [ $1 = "den" ]; then
elif [[ $1 == 'map' || $1 == 'dmap' || $1 == 'mag' || $1 == 'dmag' ]]; then
  GDB=
  TYPE="mput"
  [[ $1 = 'mag' || $1 = 'dmag' ]] && TYPE="mget"
  [[ $1 == 'dmap' || $1 == 'dmag' ]] && GDB="xterm -e gdb --args"
  
  export GLOG_logtostderr=1
  # This causes dspaces_put/get block mpi processes
  # LOG_F="$1.log"
  # $MPIRUN -x LD_LIBRARY_PATH -x GLOG_logtostderr --hostfile APP_HOST_FILE -n $NUM_CLIENT --byslot $GDB \
  #   ./mput_mget_test --type=$TYPE --cl_id=0 --base_client_id=$(($2*$NUM_CLIENT)) \
  #                   --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
  #                   --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
  #                   --num_putget=$NUM_PUTGET > $LOG_F 2>&1 < /dev/null &
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
                    --num_putget=$NUM_PUTGET > $LOG_F 2>&1 < /dev/null &
  done
elif [ $1  = 'k' ]; then
  # Sometimes works sometimes not!
  # $MPIRUN --hostfile DS_HOST_FILE -np 32 --bynode $PKILL -f dataspaces_server
  # $MPIRUN --hostfile APP_HOST_FILE -np 32 --bynode $PKILL -f mput_mget_test
  NODE_LIST=(${DS_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
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
else
  echo "Argument did not match!"
fi