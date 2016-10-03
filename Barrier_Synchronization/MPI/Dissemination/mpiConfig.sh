#!/bin/bash

for j in `seq 2 12`
do
	echo "Dissemination" 
	MPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $j /nethome/rankalikar3/MPI/dissemination >> MPI/result.diss

	echo "Default" 
	MPI_MCA_mpi_yield_when_idle=0 /opt/openmpi-1.4.3-gcc44/bin/mpirun --hostfile $PBS_NODEFILE -np $j /nethome/rankalikar3/MPI/default >> MPI/result.default
	echo "Done For $j"
done
