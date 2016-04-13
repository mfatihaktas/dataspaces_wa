#!/bin/bash
echo $1 $2 $3

if [ -n "$DELL" ]; then
  PUTTER_NODES="dell01"
  GETTER_NODES="dell02"
  
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
  " > DS_HOST_FILE
  
  echo "# hosts to run apps on
  dell01 slots=16 max_slots=16
  dell02 slots=16 max_slots=16
  # dell03 slots=16 max_slots=16
  dell04 slots=16 max_slots=16
  dell05 slots=16 max_slots=16
  dell06 slots=16 max_slots=16
  " > APP_HOST_FILE
  
  DS_NODES=""
  # For site 0
  BEGIN_DSNODE_ID=7
  END_DSNODE_ID=$(($BEGIN_DSNODE_ID + 9))
  for i in `seq $BEGIN_DSNODE_ID $END_DSNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    DS_NODES+="dell$STR$i"
    [ "$i" -lt $END_DSNODE_ID ] && DS_NODES+=","
  done
  echo "DS_NODES= $DS_NODES"
  
  PUTTER_NODES=""
  BEGIN_PNODE_ID=4
  END_PNODE_ID=$(($BEGIN_PNODE_ID + 2))
  for i in `seq $BEGIN_PNODE_ID $END_PNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    PUTTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_PNODE_ID ] && PUTTER_NODES+=","
  done
  echo "PUTTER_NODES= $PUTTER_NODES"
    
  GETTER_NODES=""
  BEGIN_GNODE_ID=4
  END_GNODE_ID=$(($BEGIN_GNODE_ID + 2))
  for i in `seq $BEGIN_GNODE_ID $END_GNODE_ID`; do
    STR=""
    [ "$i" -lt 10 ] && STR+="0"
    GETTER_NODES+="dell$STR$i"
    [ "$i" -lt $END_GNODE_ID ] && GETTER_NODES+=","
  done
  echo "GETTER_NODES= $GETTER_NODES"
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

# NUM_DS=352
# NUM_PUTTER=80
# NUM_GETTER=80
NUM_DS=16 # 32
NUM_PUTTER=1 # 8 # 64
NUM_GETTER=1 # 8 # 32
NUM_TIMESTEP=5

PKILL=/usr/bin/pkill

if [[ $1  == 's' || $1  == 'ds' ]]; then
  [ -a conf ] && rm srv.lck conf dataspaces.conf *.log
  DEBUG=
  GDB="xterm -e gdb --args"
  VALGRIND="valgrind --vgdb=yes"
  [ $1  = 'ds' ] && DEBUG=$GDB
  
  # dims = 256,256,256
  echo "# Config file for DataSpaces
  ndim = 3 
  dims = 1024,1024,1024
  max_versions = 1
  max_readers = 1 
  lock_type = 2
  " > dataspaces.conf
  
  # NODE_LIST=(${DS_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   LOG_F="ds_$NODE.log"
  #   echo "run dataspaces_server on $NODE"
    
  #   $MPIRUN -npernode 1 -host $NODE $GDB \
  #     $DSPACES_DIR/bin/dataspaces_server --server ${#NODE_LIST[@]} \
  #                                       --cnodes $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &
  # done
  
  LOG_F="ds.log"
  $MPIRUN --hostfile DS_HOST_FILE -n $NUM_DS --bynode $DEBUG \
    $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
                                       --cnodes $(($NUM_PUTTER + $NUM_GETTER)) < /dev/null 2>&1 | tee $LOG_F & # > $LOG_F 2>&1 < /dev/null &
elif [[ $1  == 'p' || $1  == 'dp' ]]; then
  GDB=
  [ $1  = 'dp' ] && GDB="xterm -e gdb --args"
  LOG_F="putter.log"
  
  #   # ./test_writer type npapp dims np[0] ... np[dims-1] sp[0] ... sp[dims-1] timestep appid
  $MPIRUN --hostfile APP_HOST_FILE -n $NUM_PUTTER --bynode $GDB \
    $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 $NUM_TIMESTEP 1 # > $LOG_F 2>&1 < /dev/null &
elif [[ $1  == 'g' || $1  == 'dg' ]]; then
  GDB=
  [ $1  = 'dg' ] && GDB="xterm -e gdb --args"
  LOG_F="getter.log"
  
  $MPIRUN --hostfile APP_HOST_FILE -n $NUM_GETTER --bynode $GDB \
    $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 $NUM_TIMESTEP 2 # > $LOG_F 2>&1 < /dev/null &
elif [ $1  = 'k' ]; then
  # $MPIRUN --hostfile DS_HOST_FILE -np 32 --bynode $PKILL -f dataspaces_server
  # $MPIRUN --hostfile APP_HOST_FILE -np 32 --bynode $PKILL -f test_
  NODE_LIST=(${DS_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
  done
  # NODE_LIST=(${PUTTER_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f test_writer
  # done
  # NODE_LIST=(${GETTER_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f test_reader
  # done
else
  echo "Argument did not match!"
fi
