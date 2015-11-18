#!/bin/bash
echo $1

if [ $1  = 'bash' ]; then
  /bin/./bash
elif [ $1  = 'init' ]; then
  export PATH=/net/hj1/ihpcl/bin:$PATH
  echo $PATH
elif [ $1  = 'gow' ]; then
  cd /net/hp101/ihpcsc/maktas7
elif [ $1  = 'god' ]; then
  cd /net/hp101/ihpcsc/maktas7/dataspaces_wa
else
  echo "Argument did not match !"
fi
