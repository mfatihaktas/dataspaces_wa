#!/bin/bash
echo $1 $2 $3

# TMAQUIS_DIR=/cac/u01/mfa51/Desktop/nstx-sc14-demo_on_dell_cluster
# TMAQUIS_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa
TMAQUIS_DIR=/cac/u01/mfa51/Desktop/ib_verbs_test

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
      ssh maktas7@maquis$3.cc.gatech.edu
    fi
  elif [ $2 = 'u' ]; then
    if [ -z "$3" ]; then
      echo "sshing to ulam$3"
      ssh maktas@202.83.248.123
    else
      echo "sshing to ulam$3"
      ssh -A -t maktas@202.83.248.123 ssh ulam$3
    fi
  elif [ $2 = 'a' ]; then
    if [ -z "$3" ]; then
      echo "which Archer node? 1-4"
    else
      ssh -A -t maktas@202.83.248.123 ssh archer$3
    fi
  fi
elif [ $1  = 'tr' ]; then #scp only source code
  if [ $2 = 'm' ]; then
    if [ -z "$3" ]; then
      echo "which Maquis node? 1-16"
    else
      rsync -avz $TMAQUIS_DIR maktas7@maquis$3.cc.gatech.edu:$MAQUIS_DIR
    fi
  elif [ $2 = 'u' ]; then
    rsync -avz $TULAM_DIR maktas@202.83.248.123:$ULAM_DIR
    ssh maktas@202.83.248.123 rsync -avz $FULAM_TARCHER_DIR archer5:$ARCHER_DIR
  fi
else
  echo "Argument did not match !"
fi
