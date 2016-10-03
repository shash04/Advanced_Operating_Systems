#include <sys/time.h>
#include <math.h>
#include "centralized.h"

#define MF 1000000
#define NUMBER_OF_ITER 1000

unsigned long t_diff;
struct timeval t_time1,t_time2;

int main(int argc, char **argv)
{
  // **** Pass the value of num_threads as an argument ****
  if(argc >= 2){
    num_threads = atoi(argv[1]);
  }
  else{
    num_threads = NUM_THREADS;
  }
  // **** Initialize the Barrier ****
  OpenMP_CentralizedBarrierInit(num_threads);

  // **** Store the time t1 just before entering parallel section ****
  gettimeofday(&t_time1,NULL);

#pragma omp parallel
  {
    int i;
    for(i=0; i<NUMBER_OF_ITER; i++){
      OpenMP_CentralizedBarrier();
    }
  }
  // **** Store the value of time t2 just after the end of parallel section ****
  gettimeofday(&t_time2,NULL);
  // **** Compute the time difference in usec ****
  t_diff = (t_time2.tv_sec * MF + t_time2.tv_usec) - (t_time1.tv_sec * MF + t_time1.tv_usec);
  // **** Distplay the time difference in usec ****
  printf("Threads = %d avg time: %f \n",num_threads, t_diff/((double)NUMBER_OF_ITER));
  return 0;
}
