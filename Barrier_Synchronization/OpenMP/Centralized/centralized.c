#include "centralized.h"

void OpenMP_CentralizedBarrierInit(int no_threads){
  // **** Initialize number of threads ****
  num_threads = no_threads;
  omp_set_num_threads(num_threads);
  // **** Initialize count varibale to number of threads and globalsense to 1 ****
  count = num_threads;
  globalsense = 1;
  // **** Create an array for maintaining localsense for every thread ****
  localsense = (int*)malloc(num_threads*sizeof(int));
  int i;
  // **** Initialize localsense for every thread to 1 ****
  for(i=0; i<count; i++)
    localsense[i] = 1;
}

void OpenMP_CentralizedBarrier(){
  int thread_num = omp_get_thread_num();
  // printf("Current thread = %d\n", thread_num);
  localsense[thread_num] = !localsense[thread_num];
  // **** critical section guards parallel access to 'count' variable ****
#pragma omp critical
  {
    count = count - 1;
    if(count == 0){                                             // Last thread reaching barrier
      count = num_threads;                                      // Reset 'count' variable
      globalsense = localsense[thread_num];                     // Invert globalsense
      // printf("globalsense set to = %d \n", globalsense);
    }
  }
  while(globalsense != localsense[thread_num]);                 // Spin till globalsense inverted by last thread
}
