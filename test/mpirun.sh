#!/bin/bash
echo $1 $2 $3

if [ -n "$DELL" ]; then
  # DS_NODES="dell11,dell12,dell13,dell14,dell15,dell16,dell17,dell18,dell19,dell20,dell21,dell22,dell23,dell24,dell25,dell26"
  # DS_NODES="dell11,dell12,dell13,dell14"
  # DS_NODES="dell11,dell12,dell13"
  DS_NODES="dell11,dell12"
  # DS_NODES="dell11"
  PUTTER_NODES="dell01"
  GETTER_NODES="dell02"
  
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
  " > DS_HOST_FILE
  
  echo "# hosts to run apps on
  dell01 slots=16 max_slots=16
  dell02 slots=16 max_slots=16
  dell03 slots=16 max_slots=16
  dell04 slots=16 max_slots=16
  dell05 slots=16 max_slots=16
  dell06 slots=16 max_slots=16
  dell07 slots=16 max_slots=16
  dell08 slots=16 max_slots=16
  dell09 slots=16 max_slots=16
  dell10 slots=16 max_slots=16
  " > APP_HOST_FILE
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

# NUM_DS=352
# NUM_PUTTER=80
# NUM_GETTER=80
NUM_DS=1 # 32
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
  
  echo "# Config file for DataSpaces
  ndim = 3 
  dims = 256,256,256
  max_versions = 1
  max_readers = 1 
  lock_type = 2
  " > dataspaces.conf
  
  # NODE_LIST=(${DS_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   LOG_F="ds_"$NODE".log"
  #   echo "run dataspaces_server on $NODE"
    
  #   $MPIRUN -npernode 1 -host $NODE $GDB \
  #     $DSPACES_DIR/bin/dataspaces_server --server ${#NODE_LIST[@]} \
  #                                       --cnodes $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &
  # done
  
  LOG_F="ds.log"
  # $MPIRUN --hostfile DS_HOST_FILE -n $NUM_DS --byslot \
  #   $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
  #                                     --cnodes $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &
  $MPIRUN --hostfile DS_HOST_FILE -n $NUM_DS --byslot $DEBUG \
    $DSPACES_DIR/bin/dataspaces_server --server $NUM_DS \
                                      --cnodes $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &
    
elif [[ $1  == 'p' || $1  == 'dp' ]]; then
  GDB=
  [ $1  = 'dp' ] && GDB="xterm -e gdb --args"
  LOG_F="putter.log"
  
  #   # ./test_writer type npapp dims np[0] ... np[dims-1] sp[0] ... sp[dims-1] timestep appid
  # $GDB $MPIRUN -n $NUM_PUTTER \
  #   $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 $NUM_TIMESTEP 1 # > $LOG_F 2>&1 < /dev/null &
  # $MPIRUN -n $NUM_PUTTER $GDB \
  #   $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 $NUM_TIMESTEP 1 # > $LOG_F 2>&1 < /dev/null &
  
  $MPIRUN --hostfile APP_HOST_FILE -n $NUM_PUTTER --byslot $GDB \
    $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 $NUM_TIMESTEP 1 # > $LOG_F 2>&1 < /dev/null &
elif [[ $1  == 'g' || $1  == 'dg' ]]; then
  GDB=
  [ $1  = 'dg' ] && GDB="xterm -e gdb --args"
  LOG_F="getter.log"
  
  # $GDB $MPIRUN -n $NUM_GETTER \
  #   $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 $NUM_TIMESTEP 2 # > $LOG_F 2>&1 < /dev/null &
  # $MPIRUN -n $NUM_GETTER $GDB \
  #   $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 $NUM_TIMESTEP 2 # > $LOG_F 2>&1 < /dev/null &
  
  $MPIRUN --hostfile APP_HOST_FILE -n $NUM_GETTER --byslot $GDB \
    $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 $NUM_TIMESTEP 2 # > $LOG_F 2>&1 < /dev/null &
elif [ $1  = 'k' ]; then
  # NODE_LIST=(${DS_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
  # done
  # NODE_LIST=(${PUTTER_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f test_writer
  # done
  # NODE_LIST=(${GETTER_NODES//,/ } )
  # for NODE in "${NODE_LIST[@]}"; do
  #   $MPIRUN -npernode 1 -host $NODE $PKILL -f test_reader
  # done
  
  $MPIRUN --hostfile DS_HOST_FILE -np 32 --bynode $PKILL -f dataspaces_server
  $MPIRUN --hostfile APP_HOST_FILE -np 32 --bynode $PKILL -f test_
else
  echo "Argument did not match!"
fi
