#ifndef MCS_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define NO_PARENT -1
#define ARRIVAL_MESSAGE_CODE 1111
#define WAKEUP_MESSAGE_CODE 9999

//Node associated with every parent in MCS arrival tree
typedef struct tNode{
  //Following two elements needed for binary wakeup
  int wakeupParent;
  int childPointers[2];
  
  //Following two elements required for 4-ary arrival
  int haveChild[4];
  int childNotReady[4];

}tNode;

tNode *currentNode;
 
void init_mcs_barrier(int num_processes,int rank);
void mcs_barrier();

#endif