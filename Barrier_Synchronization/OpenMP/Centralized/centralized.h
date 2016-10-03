#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

/*  num_threads = Total number of threads
    count = variable to determine if all threads reach the barrier
    globalsense = shared variable for spinning
    localsense = private variable of each thread used to compare with shared globalsense
*/
int num_threads, count, globalsense;
int *localsense;

void OpenMP_CentralizedBarrierInit(int no_threads);
void OpenMP_CentralizedBarrier();