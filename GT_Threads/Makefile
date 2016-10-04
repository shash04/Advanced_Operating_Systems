CC = gcc            # default is CC = cc
CFLAGS = -g -Wall   # default is CFLAGS = [blank]

GTTHREADS_SRC = gtthread_sched.c gtthread_mutex.c steque.c
GTTHREADS_OBJ = $(patsubst %.c,%.o,$(GTTHREADS_SRC))

DINING_SRC = philosopher.c chopsticks.c
DINING_OBJ = $(patsubst %.c,%.o,$(DINING_SRC))

# pattern rule for object files
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: gtthread_main dining_main

#### GTThreads ####
gtthread_main: gtthread_main.o $(GTTHREADS_OBJ)
	$(CC) -o gtthread_main gtthread_main.o $(GTTHREADS_OBJ)

#### Dining ####
.PHONY: dining_main
dining_main: dining_main.o $(DINING_OBJ)
	$(CC) -o dining_main dining_main.o $(DINING_OBJ) -lpthread

clean:
	$(RM) -f *.o dining_main gtthread_main
