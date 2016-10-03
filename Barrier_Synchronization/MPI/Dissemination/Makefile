FLAGS = -g -Wall -lm -fopenmp -o

#MPICC = mpicc
MPICC = /opt/openmpi-1.4.3-gcc44/bin/mpicc

all: dissemination default

dissemination: dissemination.c
	$(MPICC) dissemination.c $(FLAGS) dissemination

default: default.c
	$(MPICC) default.c $(FLAGS) default

clean:
	rm -f *.o dissemination default
