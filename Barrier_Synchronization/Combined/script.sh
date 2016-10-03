#!/bin/bash

#PBS -q class
#PBS -l nodes=8:sixcore
#PBS -l walltime=00:30:00
#PBS -N COMB_test

for k in `seq 1 2`;
do
  echo "******** Readings number $k ********" >>  /nethome/sphalke3/final/Combined/results_final.txt
  for i in `seq 2 8`;
  do
    echo "==== Number of Processes = $i ====" >>  /nethome/sphalke3/final/Combined/results_final.txt
    for j in `seq 2 12`;
    do
      OMPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $i /nethome/sphalke3/final/Combined/test_combined $j >> /nethome/sphalke3/final/Combined/results_final.txt
      echo "done for $i processeses and $j threads"
    done
  done
done
