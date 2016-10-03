#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "mcs.h"
#include <sys/time.h>
#include <math.h>

void init_mcs_barrier(int numProcesses,int rank){

  int i;
  tNode t;

  currentNode = (tNode*) malloc(sizeof(t));
  
  for(i=0 ; i < 4; i++){
    if((rank*4 + i + 1) < numProcesses){
      currentNode->haveChild[i] = 1;
      currentNode->childNotReady[i] = 1;
    }
    else{
      currentNode->haveChild[i] = 0;
      currentNode->childNotReady[i] = 0;
    }
  }

  if(rank == 0){
    currentNode->wakeupParent = NO_PARENT;
  }
  else{ 
    currentNode->wakeupParent = (rank-1)/2;
  }

  for(i=0; i < 2; i++) {
    if(rank * 2 + i + 1 < numProcesses)
      currentNode->childPointers[i] = rank * 2 + i + 1;
    else
      currentNode->childPointers[i] = NO_PARENT;
  }

}

void mcs_barrier(){

  int rank,i,msg,rank_to_send, rank_to_receive;
  MPI_Status mpi_result;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  for(i=0; i< 4; i++){
    if(currentNode->haveChild[i] == 1){
      rank_to_receive = rank*4 + i + 1;
      MPI_Recv(&msg, 1, MPI_INT, rank_to_receive, ARRIVAL_MESSAGE_CODE, MPI_COMM_WORLD, &mpi_result);
      currentNode->childNotReady[i] = 0;
    }
  }

  if(rank != 0){
    rank_to_send = (rank-1)/4;
    MPI_Send(&rank, 1, MPI_INT, rank_to_send, ARRIVAL_MESSAGE_CODE, MPI_COMM_WORLD);
  }

  if(rank != 0){ 
    rank_to_receive = currentNode->wakeupParent;
    MPI_Recv(&msg, 1, MPI_INT, rank_to_receive, WAKEUP_MESSAGE_CODE, MPI_COMM_WORLD, &mpi_result);
  }
  
  for(i=0;i<2;i++){
    rank_to_send = currentNode->childPointers[i];
    if(rank_to_send != NO_PARENT){
      MPI_Send(&rank , 1, MPI_INT, rank_to_send, WAKEUP_MESSAGE_CODE, MPI_COMM_WORLD);
    }
  } 

}



