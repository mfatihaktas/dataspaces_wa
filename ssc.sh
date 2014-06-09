#!/bin/bash
echo $1

DELL_NAMES=( dell01 dell02 )

DIR=""

if [ $1  = 'ssh' ]; then
  #ssh -A -t ${DELL_NAMES[$2]} hostname
  ssh -A -t spring ssh ${DELL_NAMES[$2]}
  #ssh -X ${DELL_NAMES[$2]} 'module load openmpi-x86_64'
elif [ $1  = 'conf' ]; then
  module load openmpi-x86_64
elif [ $1  = 'tr' ]; then
  scp -r misc mfa51@india.futuregrid.org:~/
elif [ $1  = 'fr' ]; then
  scp -r mfa51@india.futuregrid.org:~/misc .
else
  echo "Argument did not match !"
fi
