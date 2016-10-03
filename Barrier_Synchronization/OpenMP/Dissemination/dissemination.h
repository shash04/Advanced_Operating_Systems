#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_THREADS 4
/* Structure to represent a node. Contains variable to store current
number of the node and also the node from whom a message is received.
*/
typedef struct node{
  int round_no;
  int msg_received;
}node;

/*barrier_array is used to create an array of nodes depending on number of threads
num_threads is equal to number of threads created 
*/
node *barrier_array;
int num_threads;

void OpenMP_DisseminationBarrierInit(int no_threads);
void OpenMP_DisseminationBarrier();