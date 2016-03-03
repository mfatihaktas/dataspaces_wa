#!/bin/bash
echo $1 $2 $3

if [ $1  = 'r' ]; then
  sbatch slurm.sh
elif [ $1  = 'k' ]; then
  scancel --user=mfatih
elif [ $1  = 'l' ]; then
  squeue -u mfatih
else
  echo "Argument did not match!"
fi