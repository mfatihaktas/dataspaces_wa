#!/bin/bash
echo $1 $2 $3

if [ $1 = 'p' ]; then
  ./exp --type="plot"
else
  echo "Argument did not match !"
fi