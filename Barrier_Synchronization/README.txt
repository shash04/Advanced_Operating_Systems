### Barrier Synchronization
## Overview:
The goal of this project is to introduce OpenMP, MPI, and barrier synchronization concepts. You will implement several barriers using OpenMP and MPI, and synchronize between multiple threads and machines.
OpenMP allows you to run parallel algorithms on shared-memory multiprocessor/multicore machines. MPI allows you to run parallel algorithms on distributed memory systems, such as compute clusters. The project consists of implementation of two spin barriers using OpenMP and MPI each. It also includes an implementation with combination of MPI-OpenMP program in order to synchronize between multiple cluster nodes that are running multiple threads themselves. 
Experimentation was done to evaluate the performance of your barrier implementations. (Information about compute resources for running experiments is below.) OpenMP barriers were run on an 8-way SMP (symmetric multi-processor) system, and MPI and MPI-OpenMP combined experiments on a (up to) 24 12-core cluster (24 nodes, and each node has two six-core processors).

## Steps to run the project files:

# OpenMP:
Contains subfolders Centralized and Dissemination for individual barrier implementations. Both folders have respective Makefiles.

OpenMP->Centralized: 
To run experiments locally, use python script as: python local_centralized_script.py. 
The results will be stored in centralized_results.txt file.
To run experiments on jinx, use bash script 'script.sh' as: qsub ./script.sh

OpenMP->Dissemination:
To run experiments locally, use python script as: python local_dissemination_script.py. The results will be stored in OpenMP_dissemination_results.txt file.
To run experiments on jinx, use bash script 'script.sh' as: qsub ./script.sh

# MPI:
Both folders contain Makefiles. Path for MPICC is set to the one needed to build the program on jinx cluster.

MPI->Dissemination: 
This folder contains default.c and default.h files which use the internal MPI barriers.
MPI->Dissemination->runscript.pbs / MPI->MCS->runscript.pbs : This script was used to schdule jobs on the cluster.
MPI->Dissemination->mpiConfig.sh / MPI->MCS->mpiConfig.sh : These scripts were used to actually run the MPI programs with different input parameters.

# Combined:
To run experiments locally, use command: mpirun -np [number_of_nodes] ./test_combined [number_of_threads]
To run experiments on jinx, use bash script 'script.sh' as: qsub ./script.sh
