#ifndef COMBINED_BARRIER_H

#include <mpi.h>
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "mcs.h"
#include "dissemination.h"  

#define NUM_THREADS 4

// **** shared variable 'globalsense' amongst multiple nodes ****
volatile int globalsense;

void combined_barrier_init(int no_threads, int num_processes,int rank);
void combined_barrier();

#endif