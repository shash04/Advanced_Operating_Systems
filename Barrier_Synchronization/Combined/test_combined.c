#include <sys/time.h>
#include "combined_barrier.h"

#define NUM_ITERATIONS 1000
#define MF 1000000

unsigned long t_diff;
struct timeval t_time1,t_time2;

int main(int argc, char**argv){
  // **** Pass the value of num_threads as an argument ****
  if(argc >= 2){
    num_threads = atoi(argv[1]);
  }
  else{
    num_threads = NUM_THREADS;
  }
 
  int numProcesses, rank;
  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // **** Initialize the Barrier ****
  combined_barrier_init(num_threads, numProcesses, rank);
  
  int i;
  if(rank == 0){
    // **** Store the time t1 just before entering parallel section. Only Node with rank = 0 stores time ****
    gettimeofday(&t_time1,NULL);
  }

  #pragma omp parallel private(i)
  {
    for(i=0;i<NUM_ITERATIONS;i++){
      combined_barrier();
    }
  }
  if(rank == 0){
    // **** Store the value of time t2 just after the end of parallel section. Only Node with rank = 0 stores time ****
    gettimeofday(&t_time2,NULL);
    // **** Compute the time difference in usec ****
    t_diff = (t_time2.tv_sec * MF + t_time2.tv_usec) - (t_time1.tv_sec * MF + t_time1.tv_usec);
    // **** Distplay the time difference in usec ****
    printf("No_of_processes = %d No_of_threads = %d avg time: %f usec\n", numProcesses, num_threads, t_diff/((double)NUM_ITERATIONS));  
  }

  MPI_Finalize();
  return 0;
}
