FLAGS = -g -Wall -lm -fopenmp -o

#MPICC = mpicc
MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc

all: mcs

mcs: mcs.c
	$(MPICC) mcs.c $(FLAGS) mcs

clean:
	rm -f *.o mcs
