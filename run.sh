#!/bin/bash
echo $1 $2 $3

NUM_SNODE=1
NUM_CLIENT=3
NUM_DSCNODE=$(($NUM_CLIENT+1)) # +1: RIManager
NUM_PUTGET=3

# TMPFS_DIR=$DSPACESWA_DIR/tmpfs
if [ -n "$DELL" ]; then
  LCONTROL_LINTF="em2"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  CONTROL_LINTF="em2"
  RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  IB_LINTF="ib0"
  TCP_LINTF="em2"
  GFTP_LINTF="em2"
# elif [ -n "$ELF" ]; then
  # LCONTROL_LINTF="em2"
  # APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.151" "192.168.2.152" )
  
  # CONTROL_LINTF="em2"
  # RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.151" )
  
  # IB_LINTF="ib0"
  # TCP_LINTF="em2"
  # GFTP_LINTF="em2"
elif [ -n "$ULAM" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.100" "192.168.2.202" )
  
  CONTROL_LINTF="eth0"
  RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.100" )
  
  IB_LINTF="ib0"
  TCP_LINTF="eth0"
  GFTP_LINTF="eth0"
elif [ -n "$KID" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP_LIST=( "130.207.110.52" "130.207.110.53" )
  
  CONTROL_LINTF="eth0"
  RI_JOIN_CONTROL_LIP_LIST=( "" "130.207.110.52" )
  
  IB_LINTF="ib0"
  TCP_LINTF="eth0"
  GFTP_LINTF="eth0"
elif [ -n "$BOOTH" ]; then
  LCONTROL_LINTF="eth0"
  APP_JOIN_LCONTROL_LIP_LIST=( "192.168.2.241" "192.168.2.241" )
  
  CONTROL_LINTF="eth0"
  RI_JOIN_CONTROL_LIP_LIST=( "" "192.168.2.241" )
  
  IB_LINTF="ib0"
  TCP_LINTF="eth0"
  GFTP_LINTF="eth0"
else
  echo "Which system DELL | ELF | ULAM | KID | BOOTH"
fi

RI_LCONTROL_LPORT_LIST=( 8000 9000 )

RI_CONTROL_LPORT_LIST=( 7000 7001 )
RI_JOIN_CONTROL_LPORT_LIST=( 0 7000 7000 )

TCP_LPORT=6000
GFTP_LPORT=6000

TRANS_PROTOCOL="i" # "t" # "i" # "g"
W_PREFETCH=1

if [ $1 = 's' ]; then
  [ -a conf ] && rm srv.lck conf dataspaces.conf rm *.log
  
  echo "## Config file for DataSpaces
  ndim = 3
  dims = 1024,1024,1024
  max_versions = 1
  max_readers = 1
  lock_type = 1
  " > dataspaces.conf
  
  $DSPACES_DIR/bin/./dataspaces_server --server 1 --cnodes $NUM_DSCNODE
elif [ -z "$2" ]; then
  echo "Which site [0, *]?"
elif [[ $1 == 'p' || $1 == 'dp' || $1 == 'g' || $1 == 'dg' ]]; then
  [ -z "$3" ] && { echo "Which app [1, *] ?"; exit 1; }
  TYPE="put"
  GDB=""
  [[ $1 == 'g' || $1 == 'dg' ]] && TYPE="get"
  [[ $1 == 'dp' || $1 == 'dg' ]] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./exp --type=$TYPE --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) \
    --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_LCONTROL_LPORT_LIST[$2] } + $3)) \
    --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] }
elif [[ $1 == 'mp' || $1 == 'dmp' || $1 == 'mg' || $1 == 'dmg' ]]; then
  [ -z "$3" ] && { echo "Which app [1, *] ?"; exit 1; }
  TYPE="mput"
  GDB=""
  [[ $1 == 'mg' || $1 == 'dmg' ]] && TYPE="mget"
  [[ $1 == 'dmp' || $1 == 'dmg' ]] && GDB="gdb --args"
  
  export GLOG_logtostderr=1
  $GDB ./mput_mget_test --type=$TYPE --cl_id=$3 --base_client_id=$(($2*$NUM_CLIENT)) \
    --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_LCONTROL_LPORT_LIST[$2] } + $3)) \
    --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
    --num_putget=$NUM_PUTGET
elif [[ $1 == 'map' || $1 == 'mag' ]]; then
  # rm $1_*.log
  TYPE="mput"
  [ $1 = 'mag' ] && TYPE="mget"
  
  export GLOG_logtostderr=1
  for i in `seq 1 $NUM_CLIENT`; do
    ./mput_mget_test --type=$TYPE --cl_id=$i --base_client_id=$(($2*$NUM_CLIENT)) \
      --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=$((${RI_LCONTROL_LPORT_LIST[$2] } + $i)) \
      --join_lcontrol_lip=${APP_JOIN_LCONTROL_LIP_LIST[$2] } --join_lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
      --num_putget=$NUM_PUTGET 2>&1 < /dev/null | tee $1_$i.log &
  done
  
  read -p "[Enter]"
  echo "killing..."
  pkill -f mput_mget_test
elif [[ $1 == 'r' || $1 == 'dr' ]]; then
  GDB=""
  [ $1 = 'dr' ] && GDB="gdb --args"
  
  # if [ $TRANS_PROTOCOL = 'g' ]; then
  #   echo "Starting Gftps..."
  #   globus-gridftp-server -aa -password-file pwfile -c None \
  #                         -port $GFTP_LPORT \
  #                         -d error,warn,info,dump,all &
  #                         # -data-interface $WA_LINTF \
  # fi
  export GLOG_logtostderr=1
  $GDB ./exp --type="ri" --cl_id=111 --num_peer=1 --base_client_id=$(($2*$NUM_CLIENT)) \
             --lcontrol_lintf=$LCONTROL_LINTF --lcontrol_lport=${RI_LCONTROL_LPORT_LIST[$2] } \
             --ds_id=$2 --control_lintf=$CONTROL_LINTF --control_lport=${RI_CONTROL_LPORT_LIST[$2] } \
             --join_control_lip=${RI_JOIN_CONTROL_LIP_LIST[$2] } --join_control_lport=${RI_JOIN_CONTROL_LPORT_LIST[$2] } \
             --trans_protocol=$TRANS_PROTOCOL --ib_lintf=$IB_LINTF \
             --tcp_lintf=$TCP_LINTF --tcp_lport=$TCP_LPORT \
             --gftp_lintf=$GFTP_LINTF --gftp_lport=$GFTP_LPORT --tmpfs_dir=$TMPFS_DIR \
             --w_prefetch=$W_PREFETCH 2>&1 | tee ri_$2.log
  read -p "[Enter]"
  # echo "Killing stubborns..."
  # fuser -k -n tcp $GFTP_LPORT
elif [ $1 = 'init' ]; then
  if [ $2 = 'd' ]; then
    export DELL=DELL
    # ENV VARIABLES FOR MAKE
    # export GRIDFTP=GRIDFTP
    export CC=gcc #/opt/gcc-4.8.2/bin/gcc
    export CPP=g++ #/opt/gcc-4.8.2/bin/g++
    export MPICPP=mpicxx #/cac/u01/mfa51/Desktop/mpich-3.1.2/install/bin/mpicxx
    # export MPI_DIR=/cac/u01/mfa51/Desktop/mpich-3.1.2/install
    export GLOG_DIR=/cac/u01/mfa51/Desktop/glog-0.3.3/install
    export BOOST_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    # export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0/install
    export DSPACES_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces/install
    export DSPACESWA_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
    
    # source /opt/rh/devtoolset-2/enable
    unset LD_LIBRARY_PATH
    # LD_LIBRARY_PATH=/cac/u01/mfa51/Desktop/mpich-3.1.2/install/lib:$LD_LIBRARY_PATH
    # LD_LIBRARY_PATH=/opt/gcc-4.8.2/lib64:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
  elif [ $2 = 'e' ]; then
    export ELF=ELF
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    # export MPI_DIR=/cac/u01/mfa51/Desktop/mpich-3.1.2/install
    export GLOG_DIR=/home1/mfa51/glog-0.3.3/install
    export BOOST_DIR=/home1/mfa51/boost_1_56_0/install
    export GFTPINC_DIR=
    export GFTPLIB_DIR=
    export DSPACES_DIR=/home1/mfa51/dataspaces-1.6.0/install
    export DSPACESWA_DIR=/home1/mfa51/dataspaces_wa
    
    unset LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
  elif [ $2 = 'u' ]; then
    export ULAM=ULAM
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx # module openmpi/intel-14.0.2/1.8.2
    # export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    # export MPI_DIR=/apps/intel/impi/4.1.3.048/intel64
    export GLOG_DIR=/home/sc14demo/common-apps/glog-0.3.3/install
    export BOOST_DIR=/home/sc14demo/common-apps/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/home/sc14demo/common-apps/dataspaces-1.5.0/install
    export DSPACESWA_DIR=/home/sc14demo/common-apps/dataspaces_wa
    
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
  elif [ $2 = 'k' ]; then
    export KID=KID
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export MPICPP_OPTS='-DMPICH_IGNORE_CXX_SEEK -DMPICH_SKIP_MPICXX'
    export GLOG_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3/install
    export BOOST_DIR=/net/hp101/ihpcsc/maktas7/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.6.0/install
    export DSPACESWA_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa
    
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
    
    export PATH=/net/hj1/ihpcl/bin:/sbin:$PATH
    echo "PATH=" $PATH
  elif [ $2 = 'b' ]; then
    export BOOTH=BOOTH
    export CC=gcc
    export CPP=g++
    export MPICPP=mpicxx
    export GLOG_DIR=/home/jchoi/project/glog-0.3.3/install
    export BOOST_DIR=/home/jchoi/project/boost_1_56_0/install
    export GFTPINC_DIR=/usr/include/globus
    export GFTPLIB_DIR=/usr/lib64
    export DSPACES_DIR=/home/jchoi/project/dataspaces-1.6.0/install
    export DSPACESWA_DIR=/home/jchoi/project/dataspaces_wa
    
    LD_LIBRARY_PATH=$BOOST_DIR/lib:$LD_LIBRARY_PATH
    LD_LIBRARY_PATH=$GLOG_DIR/lib:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH
    echo "LD_LIBRARY_PATH= " $LD_LIBRARY_PATH
  fi
elif [ $1 = 'iperf' ]; then
  if [ -n "$DELL" ]; then
    echo "No iperf on DELL"
  elif [ -n "$ULAM" ]; then
    IPERF_BIN_DIR=/home/sc14demo/common-apps/iperf/2.0.5/bin
  elif [ -n "$KID" ]; then
    IPERF_BIN_DIR=/net/hp101/ihpcsc/jchoi446/sw/iperf/2.0.5/bin
  fi
  if [ -z "$2" ]; then
    echo "Which s | c"
  else
    if [ $2 = 's' ]; then
      $IPERF_BIN_DIR/./iperf -s
    elif [ $2 = 'c' ]; then
      if [ -z "$3" ]; then
        echo "Client ip?"
      else
        $IPERF_BIN_DIR/./iperf -c $3
      fi
    fi
  fi
elif [ $1 = 'gc' ]; then
  TRANS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa/dspaces_rel/gftp_trans/dummy
  # S_IP=192.168.2.152
  S_IP=127.0.0.1
  # S_FNAME=dummy.dat
  S_FNAME=tx.dat
  C_FNAME=recved_tx.dat
  globus-url-copy -vb \
                  ftp://"$S_IP:$GFTP_LPORT$TRANS_DIR/$S_FNAME" file://"$TRANS_DIR/$C_FNAME"
elif [ $1 = 'show' ]; then
  netstat -antu
else
  echo "Argument did not match !"
fi
