#ifndef DISSEMINATION_H

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct node{
  int round_no;
  // int send_to;
  int msg_received;
}node;

node *barrier_array;
int num_threads;

void OpenMP_DisseminationBarrierInit(int nthreads);
// void resetBeforeNextRound(int* thread_num);
void OpenMP_DisseminationBarrier();

#endif