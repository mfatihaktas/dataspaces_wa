#!/bin/bash
echo $1 $2 $3

SSH_OPTS="-vXY"

# TMAQUIS_DIR=/cac/u01/mfa51/Desktop/nstx-sc14-demo_on_dell_cluster
# TMAQUIS_DIR=/cac/u01/mfa51/Desktop/ib_verbs_test
# TMAQUIS_DIR=/cac/u01/mfa51/Desktop/adios-1.7.0
TMAQUIS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa

MAQUIS_DIR=/net/hp101/ihpcsc/maktas7

# ULAM_DIR=~
ULAM_DIR=/home/sc14demo/common-apps
TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
# TULAM_DIR=/cac/u01/mfa51/Desktop/ib_verbs_test
FULAM_TARCHER_DIR=/home/sc14demo/common-apps/dataspaces_wa
ARCHER_DIR=$ULAM_DIR

if [ $1  = 'initssh' ]; then
  if [ $2 = 'm' ]; then
    cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
  elif [ $2 = 'u' ]; then
    # cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
    ssh -p 2222 maktas@202.83.248.123
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
  elif [ $2 = 'u' ]; then
    ssh -p 2222 maktas@202.83.248.123 &
    PIDSAVE=$!
    sleep 1; kill $PIDSAVE
    if [ -z "$3" ]; then
      echo "sshing to ulam$3"
      ssh maktas@202.83.248.123
    else
      echo "sshing to ulam$3"
      ssh -A -t maktas@202.83.248.123 ssh ulam$3
    fi
  elif [ $2 = 'a' ]; then
    # ssh -p 2222 maktas@202.83.248.123 &
    # PIDSAVE=$!
    # sleep 1; kill $PIDSAVE
    if [ -z "$3" ]; then
      echo "which Archer node? 1-4, 5-6"
    else
      echo "sshing to archer$3"
      # ssh $SSH_OPTS -A -t maktas@202.83.248.123 ssh $SSH_OPTS archer$3
      ssh -AXY -t maktas@202.83.248.123 ssh -XY archer$3
      # ssh -AXY -t maktas7@maquis1.cc.gatech.edu ssh -AXY -t maktas@202.83.248.123 ssh -XY archer$3
    fi
  fi
elif [ $1  = 'tr' ]; then #scp only source code
if [ $2 = 'm' ]; then
  rsync -avz --exclude-from .gitignore $TMAQUIS_DIR maktas7@maquis1.cc.gatech.edu:$MAQUIS_DIR
elif [ $2 = 'u' ]; then
  # ssh -p 2222 maktas@202.83.248.123 &
  # PIDSAVE=$!
  # sleep 1; kill $PIDSAVE
  #
  # rsync -avz --exclude-from .gitignore $TULAM_DIR maktas@202.83.248.123:$ULAM_DIR
  # ssh maktas@202.83.248.123 rsync --exclude-from $FULAM_TARCHER_DIR/.gitignore -avz $FULAM_TARCHER_DIR archer5:$ARCHER_DIR
  rsync -avz --exclude-from .gitignore $TMAQUIS_DIR maktas7@maquis1.cc.gatech.edu:$MAQUIS_DIR
  ssh maktas7@maquis1.cc.gatech.edu rsync -avz --exclude-from $FMAQUIS_TULAM_DIR/.gitignore $FMAQUIS_TULAM_DIR maktas@202.83.248.123:$ULAM_DIR
  
elif [ $2 = 'a' ]; then
  if [ -z "$3" ]; then
    echo "which Archer node? 1-4, 5-6"
  else
    echo "sshing to archer$3"
    ssh maktas@202.83.248.123 rsync --exclude-from $FULAM_TARCHER_DIR/.rsyncignore -avz $FULAM_TARCHER_DIR archer$3:$ARCHER_DIR
    # ssh maktas@202.83.248.123 rsync -avz /home/sc14demo/fusion archer$3:/home/sc14demo
    # ssh maktas@202.83.248.123 rsync -avz /home/sc14demo/common-apps archer$3:/home/sc14demo
  fi
fi
else
  echo "Argument did not match !"
fi