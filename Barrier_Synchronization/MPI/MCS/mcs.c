#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include "mcs.h"
#include <sys/time.h>
#include <math.h>

#define NUM_ITERATIONS 100
#define TIME
#define MF 1000000

unsigned long t_sec, t_usec, t_diff;
struct timeval t_time1, t_time2;

int main(int argc, char**argv){

	int i;
  int rank, numProcesses;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);


	init_mcs_barrier(numProcesses, rank);

#ifdef TIME
	if(rank==0)
		gettimeofday(&t_time1, NULL);
#endif

	for(i = 0; i < NUM_ITERATIONS; i++) {
  		//printf("Node %d entering barrier \n", rank);	
		mcs_barrier();
		//printf("Node %d done with barrier %d\n", rank, i);
	}

#ifdef TIME	
	if(rank==0)
	{
		gettimeofday(&t_time2,NULL);

		t_diff = (t_time2.tv_sec * MF - t_time1.tv_sec * MF) + (t_time2.tv_usec - t_time1.tv_usec);
		printf("T = %d time: %f usec\n", numProcesses, (t_diff * pow(10,-6) / (numProcesses * NUM_ITERATIONS)));		

		//t_sec = t_time2.tv_sec - t_time1.tv_sec;
		//t_usec = t_time2.tv_usec - t_time1.tv_usec;
		//if(t_time1.tv_usec > t_time2.tv_usec){
		//	t_usec = 1000000 - t_time1.tv_usec + t_time2.tv_usec;
		//	t_sec = t_sec - 1;
		//}
		//printf("T = %d time = %f \n", numProcesses , (t_sec + (pow(10,-6) * t_sec)) / 1.0);	
	}
#endif
	
  //printf("Node %d exiting\n", rank);
	
  MPI_Finalize();

	return 0;
	

}
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



