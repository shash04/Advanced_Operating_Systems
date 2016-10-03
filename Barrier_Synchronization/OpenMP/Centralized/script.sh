#!/bin/bash

#PBS -q cs6210
#PBS -l nodes=1:fourcore
#PBS -l walltime=00:15:00
#PBS -N openmp_cent

for i in `seq 1 3`;
do
  echo "******** Readings no. $i *******" >> /nethome/sphalke3/final/OpenMP/Centralized/results_final.txt
  for j in `seq 2 8`;
  do
    ./final/OpenMP/Centralized/test_centralized $j >> /nethome/sphalke3/final/OpenMP/Centralized/results_final.txt
    echo "done for $j"
  done
done
