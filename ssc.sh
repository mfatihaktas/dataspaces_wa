#!/bin/bash
echo $1
echo $2

TMAQUIS_DIR=/cac/u01/mfa51/Desktop/nstx-sc14-demo_on_dell_cluster
MAQUIS_DIR=/home/sc14demo/common-apps

ULAM_DIR=/home/sc14demo/common-apps
# ULAM_DIR=~
# TULAM_DIR=/cac/u01/mfa51/Desktop/boost_1_56_0
TULAM_DIR=/cac/u01/mfa51/Desktop/dataspaces_wa

if [ $1  = 'initssh' ]; then
  if [ $2 = 'm' ]; then
    cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
  elif [ $2 = 'u' ]; then
    # cat ~/.ssh/id_rsa.pub | ssh maktas7@maquis$3.cc.gatech.edu 'cat >> /net/rd7/maktas7/.ssh/authorized_keys'
    ssh -p 2222 maktas@202.83.248.123
  fi
elif [ $1  = 'ssh' ]; then
  if [ $2 = 'm' ]; then
    ssh maktas7@maquis$3.cc.gatech.edu
  elif [ $2 = 'u' ]; then
    ssh maktas@202.83.248.123
  fi
elif [ $1  = 'tr' ]; then #scp only source code
  if [ $2 = 'm' ]; then
    # scp -r $TMAQUIS_DIR maktas7@maquis$3.cc.gatech.edu:$MAQUIS_DIR
    rsync -avz $TULAM_DIR maktas7@maquis$3.cc.gatech.edu:$MAQUIS_DIR
  elif [ $2 = 'u' ]; then
    # scp -r $TULAM_DIR maktas@202.83.248.123:$ULAM_DIR
    rsync -avz $TULAM_DIR maktas@202.83.248.123:$ULAM_DIR
  fi
else
  echo "Argument did not match !"
fi
