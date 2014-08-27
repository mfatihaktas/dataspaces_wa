#!/bin/bash
echo $1

if [ $1  = 's' ]; then
  ./den_server
elif [ $1  = 'c' ]; then
  ./den_client 127.0.0.1 dummy
else
  echo "Argument did not match !"
fi