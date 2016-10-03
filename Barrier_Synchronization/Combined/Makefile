FLAGS = -g -Wall -lm -fopenmp -o

MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc
# MPICC = /usr/bin/mpicc

all: test_combined

test_combined: test_combined.c combined_barrier.c dissemination.c mcs.c
	$(MPICC) test_combined.c combined_barrier.c dissemination.c mcs.c $(FLAGS) test_combined

clean:
	rm -f *.o test_combined
