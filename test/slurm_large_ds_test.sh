#!/bin/bash

#SBATCH -J ds_test
#SBATCH -p development
#SBATCH -o ds_test.node_%N.job_%j.out
#SBATCH -e ds_test.node_%N.job_%j.err
#SBATCH -N 2
#SBATCH -n 32
#SBATCH -t 00:10:00
#SBATCH -A TG-CCR150034

DS_BIN_DIR=/home1/03016/mfatih/dataspaces-1.6.0/install/bin
# DS_BIN_DIR=/home1/03016/mfatih/dataspaces/install/bin

NUM_DS_SERVER=4;
NUM_PUTTER=8;
NUM_GETTER=8;

[ -a conf ] && rm srv.lck conf dataspaces.conf *.out *.err *.log

echo "## Config file for DataSpaces
ndim = 3 
dims = 256,256,256
max_versions = 1
max_readers = 1 
lock_type = 2
" > dataspaces.conf

LOG_F="ds.log"
ibrun -o 0 -n $NUM_DS_SERVER $DS_BIN_DIR/./dataspaces_server -s $NUM_DS_SERVER -c $(($NUM_PUTTER + $NUM_GETTER)) > $LOG_F 2>&1 < /dev/null &

sleep 5

APP_NUM_CORE_LOWER_BOUND=$(($NUM_DS_SERVER>16?$NUM_DS_SERVER:16))
LOG_F="writer.log"
ibrun -o $APP_NUM_CORE_LOWER_BOUND -n $NUM_PUTTER $DS_BIN_DIR/./test_writer DATASPACES $NUM_PUTTER 3 4 4 4 64 64 64 20 1 > $LOG_F 2>&1 < /dev/null &

APP_NUM_CORE_LOWER_BOUND=$(($APP_NUM_CORE_LOWER_BOUND + $NUM_PUTTER))
LOG_F="reader.log"
ibrun -o $APP_NUM_CORE_LOWER_BOUND -n $NUM_GETTER $DS_BIN_DIR/./test_reader DATASPACES $NUM_GETTER 3 2 4 4 128 64 64 20 2 > $LOG_F 2>&1 < /dev/null &

wait
