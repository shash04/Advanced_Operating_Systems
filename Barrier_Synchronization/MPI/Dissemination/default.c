#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include "dissemination.h"

#define NUM_ITERATIONS 100
#define MF 1000000

unsigned long t_diff;
struct timeval t_time1,t_time2;

void my_mpi_barrier(void);

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int i=0;
	if(rank==0)
	{
		gettimeofday(&t_time1,NULL);
	}
	for(i=0; i<NUM_ITERATIONS ; i++)
	{
		//printf("Enter: %d, rank : %d \n", i, rank);
		//my_mpi_barrier(); 
		MPI_Barrier(MPI_COMM_WORLD);  
	}

	if(rank==0)
	{
		gettimeofday(&t_time2,NULL);
		t_diff = (t_time2.tv_sec * MF - t_time1.tv_sec * MF) + (t_time2.tv_usec - t_time1.tv_usec);
		printf("T = %d time: %f usec\n", numProcesses, (t_diff * pow(10,-6) / (numProcesses * NUM_ITERATIONS)));		
	}
	MPI_Finalize();

  return 0;
}

/*Dissemination barrier*/
void my_mpi_barrier()
{
	int i, num_rounds, rank2send, rank2receive, msg;
	num_rounds=ceil(log2(numProcesses));

 	for(i=0;i<num_rounds;i++){
			//printf("------Round %d-------\n",i);

			rank2send = rank + pow(2,i);
			rank2send = rank2send % numProcesses;

			//Sending message
			MPI_Send(&rank, 1, MPI_INT, rank2send, 1, MPI_COMM_WORLD);
			//printf("Round: %d, sent to:%d from %d\n", i, rank2send, rank);

			rank2receive = numProcesses - pow(2,i) + rank;
			rank2receive = rank2receive % numProcesses;

			//Receiving message
			MPI_Recv(&msg, 1, MPI_INT, rank2receive, 1, MPI_COMM_WORLD, &mpiResult);
			//printf("Round: %d, I %d rxd from %d \n", i, rank, rank2receive);
	}
}
