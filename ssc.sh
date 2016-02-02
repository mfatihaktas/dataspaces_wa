#!/bin/bash
echo $1 $2 $3

# SSH_OPTS="-vXY"

BOOTH_IP=140.221.161.132
ULAM_HEAD_NODE_IP=202.83.248.124
PORT_KNOCKING_NUM=46022

FKID_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa_nstx-sc14-demo/img-chunk
# TKID_DIR=/cac/u01/mfa51/Desktop/adios-1.7.0
# TKID_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0
# TKID_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
TKID_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa_nstx-sc14-demo
KID_DIR=/net/hp101/ihpcsc/maktas7

# TULAM_DIR=/cac/u01/mfa51/Desktop/ib_verbs_test
# TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces-1.6.0
TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces/dataspaces
# TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
# TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa_nstx-sc14-demo
ULAM_DIR=/home/sc14demo/common-apps

FULAM_TARCHER_DIR=/home/sc14demo/common-apps/dataspaces_wa
ARCHER_DIR=$ULAM_DIR

# FULAM_TROMEO_DIR=/home/sc14demo/common-apps/dataspaces_wa
FULAM_TROMEO_DIR=/home/sc14demo/common-apps/mpich-3.1.2
ARCHER_DIR=/home/jongchoi

# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/jchoi446/sw/adios
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/jchoi446/sw/opencv
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/jchoi446/sw/mxml
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/maktas7/dataspaces-1.6.0
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/maktas7/glog-0.3.3
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/maktas7/boost_1_56_0
# FKID_TBOOTH_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa_nstx-sc14-demo
FKID_TBOOTH_DIR=/net/hp101/ihpcsc/maktas7/dataspaces_wa

# TBOOTH_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
TBOOTH_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa_nstx-sc14-demo
BOOTH_DIR=/home/jchoi/project

if [ $1  = 'issh' ]; then
  if [ $2 = 'm' ]; then
    cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
  elif [ $2 = 'u' ]; then
    # cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
    ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP
  elif [ $2 = 'b' ]; then
    ssh -p $PORT_KNOCKING_NUM jchoi@$BOOTH_IP
  fi
elif [ $1  = 'ssh' ]; then
  if [ $2 = 'd' ]; then
    if [ -z "$3" ]; then
      echo "which dell node? 1-32"
    else
      if [ "$3" -lt "10" ]; then
        echo "sshing to dell0$3"
        ssh -A -t mfa51@spring.rutgers.edu ssh dell0$3
      else
        echo "sshing to dell$3"
        ssh -A -t mfa51@spring.rutgers.edu ssh dell$3
      fi
    fi
  elif [ $2 = 'm' ]; then
    if [ -z "$3" ]; then
      echo "which Maquis node? 1-16"
    else
      echo "sshing to maquis$3"
      ssh $SSH_OPTS maktas7@maquis$3.cc.gatech.edu
    fi
  elif [ $2 = 'k' ]; then
    if [ -z "$3" ]; then
      echo "which Kid node? 1-16"
    else
      NODE_NUM=$((40+$3))
      echo "sshing to kid$NODE_NUM"
      ssh $SSH_OPTS maktas7@kid$NODE_NUM.cc.gatech.edu
    fi
  elif [ $2 = 'u' ]; then
    ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    PIDSAVE=$!
    sleep 0.1; kill $PIDSAVE
    if [ -z "$3" ]; then
      echo "sshing to ulam$3"
      ssh maktas@$ULAM_HEAD_NODE_IP
    else
      echo "sshing to ulam$3"
      ssh -A -t maktas@$ULAM_HEAD_NODE_IP ssh ulam$3
    fi
  elif [ $2 = 'r' ]; then
    ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    PIDSAVE=$!
    sleep 0.1; kill $PIDSAVE
    if [ -z "$3" ]; then
      echo "which Romeo node? 1-15"
    else
      NODE_NUM=$((42+$3))
      echo "sshing to romeo$NODE_NUM over ulam"
      ssh -A -t maktas@$ULAM_HEAD_NODE_IP ssh jongchoi@romeo$NODE_NUM
    fi
  elif [ $2 = 'a' ]; then
    # ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    # PIDSAVE=$!
    # sleep 0.1; kill $PIDSAVE
    if [ -z "$3" ]; then
      echo "which Archer node? 1-4, 5-6"
    else
      echo "sshing to archer$3"
      # ssh $SSH_OPTS -A -t maktas@$ULAM_HEAD_NODE_IP ssh $SSH_OPTS archer$3
      ssh -AXY -t maktas@$ULAM_HEAD_NODE_IP ssh -XY archer$3
    fi
  elif [ $2 = 'b' ]; then
    ssh -p $PORT_KNOCKING_NUM jchoi@$BOOTH_IP &
    PIDSAVE=$!
    sleep 0.1; kill $PIDSAVE
    
    ssh $SSH_OPTS jchoi@$BOOTH_IP
  fi
elif [ $1  = 'tr' ]; then #scp only source code
  if [ $2 = 'm' ]; then
    rsync -avz --exclude-from=$TKID_DIR/.gitignore $TKID_DIR maktas7@maquis1.cc.gatech.edu:$KID_DIR
  elif [ $2 = 'k' ]; then
    rsync -avz --exclude-from=$TKID_DIR/.gitignore $TKID_DIR maktas7@kid42.cc.gatech.edu:$KID_DIR
  elif [ $2 = 'u' ]; then
    ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    PIDSAVE=$!
    sleep 0.1; kill $PIDSAVE
    #
    rsync -avz --exclude-from=$TULAM_DIR/.gitignore $TULAM_DIR maktas@$ULAM_HEAD_NODE_IP:$ULAM_DIR
  elif [ $2 = 'a' ]; then
    # ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    # PIDSAVE=$!
    # sleep 0.1; kill $PIDSAVE
    #
    ssh maktas@$ULAM_HEAD_NODE_IP rsync --exclude-from=$FULAM_TARCHER_DIR/.rsyncignore -avz $FULAM_TARCHER_DIR archer1:$ARCHER_DIR
  elif [ $2 = 'r' ]; then
    # ssh -p $PORT_KNOCKING_NUM maktas@$ULAM_HEAD_NODE_IP &
    # PIDSAVE=$!
    # sleep 0.1; kill $PIDSAVE
    
    ssh maktas@$ULAM_HEAD_NODE_IP rsync -avz $FULAM_TROMEO_DIR romeo43:$ROMEO_DIR
  elif [ $2 = 'b' ]; then
    # ssh maktas7@kid42.cc.gatech.edu ssh -p $PORT_KNOCKING_NUM jchoi@$BOOTH_IP
    # PIDSAVE=$!
    # sleep 0.1; kill $PIDSAVE
    
    # ssh maktas7@kid42.cc.gatech.edu rsync -avz $FKID_TBOOTH_DIR jchoi@$BOOTH_IP:$BOOTH_DIR
    # 
    ssh -p $PORT_KNOCKING_NUM jchoi@$BOOTH_IP &
    PIDSAVE=$!
    sleep 0.1; kill $PIDSAVE
    
    rsync -avz --exclude-from=$TBOOTH_DIR/.gitignore $TBOOTH_DIR jchoi@$BOOTH_IP:$BOOTH_DIR
  fi
elif [ $1  = 'fr' ]; then #scp only source code
  if [ $2 = 'd' ]; then
    scp mfa51@spring.rutgers.edu:~/Desktop/dataspaces_wa/dspaces_rel/sdm_control/prefetch/*.png ~/Desktop
  elif [ $2 = 'k' ]; then
    rsync -avz --exclude-from=$TKID_DIR/.gitignore maktas7@kid42.cc.gatech.edu:$FKID_DIR $TKID_DIR 
  fi
else
  echo "Argument did not match !"
fi
