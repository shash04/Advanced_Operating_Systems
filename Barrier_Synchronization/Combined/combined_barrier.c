#include "combined_barrier.h"
#include <stdio.h>

void combined_barrier_init(int no_threads, int num_processes,int rank){
  // **** Initialize globalsense to value 1 ****
  globalsense = 1;
  // **** Call initialization functions of both OpenMP and MPI barriers ****
  OpenMP_DisseminationBarrierInit(no_threads);
  init_mcs_barrier(num_processes,rank);
}

void combined_barrier(){
  // **** Variables to store rank of current processor, current thread number and localsense of each node ****
  int rank, thread_num, localsense;
  // **** Get rank of current processor ****
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  // **** Get thread number ****
  thread_num = omp_get_thread_num();
  
  // **** Initialize localsense to globalsense ****
  localsense = globalsense;

  // **** Function call to OpenMP Barrier to achieve synchronization between threads in single node ****
  OpenMP_DisseminationBarrier();
  
  // **** Only thread no 0 from every node implements MPI barrier. Meanwhile other threads spin on globalsense. ****
  if(thread_num == 0){
    mcs_barrier();
#pragma omp critical
    {
      globalsense = !globalsense;
      // printf("globalsense value inverted = %d by rank %dn\n",globalsense,rank);
    }
  }
  else{
    while(localsense == globalsense);           // Other threads wait for nodes synchronization
  }
}
