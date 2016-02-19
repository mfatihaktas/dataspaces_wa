#!/bin/bash
echo $1 $2 $3

DSPACES_TEST_BIN_DIR=$DSPACES_DIR/tests/C

if [ -n "$DELL" ]; then
  # DS_NODES=( "dell01" "dell02" )
  DS_NODES="dell11,dell12,dell13,dell14"
  PUTTER_NODES="dell21"
  GETTER_NODES="dell22"
  
  NUM_PUTTER=64
  NUM_GETTER=32
  
  MPIRUN=/usr/lib64/openmpi/bin/mpirun
else
  echo "Which system DELL | ULAM | KID"
fi

PKILL=/usr/bin/pkill

if [ $1  = 's' ]; then
  [ -a conf ] && rm srv.lck conf *.log
  
  NODE_LIST=(${DS_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    LOG_F="ds_"$NODE".log"
    echo "run dataspaces_server on $NODE"
    
    $MPIRUN -npernode 1 -host $NODE \
      $DSPACES_DIR/bin/dataspaces_server --server ${#NODE_LIST[@]} \
                                         --cnodes $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &
  done
elif [ $1  = 'p' ]; then
  # NODE_LIST=(${PUTTER_NODES//,/ } )
  # for i in `seq 1 $NUM_PUTTER`; do
  #   NODE=${NODE_LIST[$(($(($i - 1)) % ${#NODE_LIST[@]} )) ] }
  #   LOG_F=putter"_id_"$i"_"$NODE".log"
  #   echo "run putter $i on $NODE"
    
  #   # ./test_writer type npapp dims np[0] ... np[dims-1] sp[0] ... sp[dims-1] timestep appid
  #   $MPIRUN -host $NODE -n $NUM_PUTTER \
  #     $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 20 1 # > $LOG_F 2>&1 < /dev/null &
  #   break
  # done
  
  $MPIRUN -n $NUM_PUTTER \
    $DSPACES_DIR/bin/test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 20 1 # > $LOG_F 2>&1 < /dev/null &
  
elif [ $1  = 'g' ]; then
  # NODE_LIST=(${GETTER_NODES//,/ } )
  # for i in `seq 1 $NUM_GETTER`; do
  #   NODE=${NODE_LIST[$(($(($i - 1)) % ${#NODE_LIST[@]} )) ] }
  #   LOG_F=getter"_id_"$i"_"$NODE".log"
  #   echo "run getter $i on $NODE"
    
  #   $MPIRUN -host $NODE -n $NUM_GETTER \
  #     $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 20 2 # > $LOG_F 2>&1 < /dev/null &
  #   break
  # done
  
  $MPIRUN -n $NUM_GETTER \
    $DSPACES_DIR/bin/test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 20 2 # > $LOG_F 2>&1 < /dev/null &
elif [ $1  = 'k' ]; then
  NODE_LIST=(${DS_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f dataspaces_server
  done
  
  NODE_LIST=(${PUTTER_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f test_writer
  done
  
  NODE_LIST=(${GETTER_NODES//,/ } )
  for NODE in "${NODE_LIST[@]}"; do
    $MPIRUN -npernode 1 -host $NODE $PKILL -f test_reader
  done
else
  echo "Argument did not match!"
fi