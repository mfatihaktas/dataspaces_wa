#!/bin/bash
echo $1

VMUSRNAME=ubuntu
VMPUBIP=149.165.159.16

VMKEYDIR=./keys/mininet-key
VM_USRNAMES=( ubuntu ubuntu ubuntu ubuntu ubuntu ubuntu )
VM_PIVIPS=( 10.39.1.2 10.39.1.14 10.39.1.26 10.39.1.52 10.39.1.63 10.39.1.65)
VM_PUBIPS=( 149.165.159.17 149.165.159.35 149.165.159.36 149.165.159.37 149.165.159.38 149.165.159.39)

GFTP_DIR=./../gftp

if [ $1  = 'ssh' ]; then
  ssh mfa51@india.futuregrid.org
elif [ $1  = 'tr' ]; then
  scp -r misc mfa51@india.futuregrid.org:~/
elif [ $1  = 'fr' ]; then
  scp -r mfa51@india.futuregrid.org:~/misc .
elif [ $1 = 'uk' ]; then
  rm ./keys/mininet-key;
  scp -r mfa51@india.futuregrid.org:~/.ssh/mininet-key ./keys
elif [ $1  = 'scpkeys' ]; then
  scp mfa51@india.futuregrid.org:~/.ssh/m* ./keys
#######################################################
elif [ $1  = 'sshvm' ]; then
  ssh -X -l ${VM_USRNAMES[$2]} -i $VMKEYDIR ${VM_PUBIPS[$2]}
elif [ $1  = 'fvm' ]; then
  scp -i $VMKEYDIR ${VM_USRNAMES[$2]}@${VM_PUBIPS[$2]}:~/gftp/* $GFTP_DIR
elif [ $1  = 'tvm' ]; then
  tar czf - $GFTP_DIR | ssh -l ${VM_USRNAMES[$2]} -i $VMKEYDIR ${VM_PUBIPS[$2]} "tar xzf -"
else
	echo "Argument did not match !"
fi
