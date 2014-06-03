#!/bin/bash
echo $1

if [ $1  = 'ssh' ]; then
  ssh mfatih@lonestar.tacc.utexas.edu
elif [ $1  = 'tr' ]; then
  scp -r $WHATEVER mfatih@lonestar.tacc.utexas.edu:~/
elif [ $1  = 'fr' ]; then
  scp -r mfatih@lonestar.tacc.utexas.edu:~/$WHATEVER $WHATEVER
else
	echo "Argument did not match !"
fi
