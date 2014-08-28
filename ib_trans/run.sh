#!/bin/bash
echo $1

if [ $1  = 's' ]; then
  ./server
elif [ $1  = 'c' ]; then
  ./client 127.0.0.1 dummy
elif [ $1  = 'ds' ]; then
  gdb --args ./server
else
  echo "Argument did not match !"
fi