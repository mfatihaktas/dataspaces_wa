#!/bin/bash
echo $1

if [ $1  = 'p' ]; then
  ./misc_put
elif [ $1  = 'g' ]; then
  ./misc_get
else
  echo "Argument did not match !"
fi
