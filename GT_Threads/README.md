# GT_Threads
## Overview:
GTThreads is a preemptive user-level thread package with an API similar to Pthreads. This project also includes demonstration of the created library by implementing a solution to the classic Dining Philosophers problem.
The project implements the core of the thread library as well as a thread scheduler. The scheduler is a preemptive round robin scheduler. Each thread is assigned a time slice (its quantum) for which it is allowed to run; a thread is preempted if it used up its quantum. Preemption is achieved by using an alarm signal as a timer.

## GTThreads API:
GTThreads API contains the following functions:
void gtthread_init(long period);  
int  gtthread_create(gtthread_t *thread, void *(*start_routine)(void *), void *arg);  
int  gtthread_join(gtthread_t thread, void **status);  
void gtthread_exit(void *retval);  
int  gtthread_yield(void);  
int  gtthread_equal(gtthread_t t1, gtthread_t t2);  
int  gtthread_cancel(gtthread_t thread);  
gtthread_t gtthread_self(void);  

Also included is mutex synchronization primitive to enable safe concurrent manipulation of shared data. The API for the mutex primitive is as follows:
int  gtthread_mutex_init(gtthread_mutex_t *mutex);  
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);  
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);  

## The Dining Philosophers Problem:
Five philosophers are sitting at a round table doing their two favorite things: eating and thinking. Each philosopher has their own bowl of rice, but unfortunately there are only five chopsticks for the entire table. The chopsticks are spaced equally around the table, one between each pair of neighboring rice bowls. Whenever a philosopher wants to eat, they must acquire a chopstick from the both the left and the right; if one of the two chopsticks is already in use, then the philosopher must wait hungrily until it is released. Whenever a philosopher wants to stop eating and think for a while, they must release any chopsticks they hold and put them back on the table.
In your implementation, each philosopher should have a thread which alternates between eating and thinking, spending a random amount of time in each mode. Chopsticks are a shared resource and each must be protected by a mutex. Each thread should print out status information, such as "Philosopher #1 is hungry", "Philosopher #1 tries to acquire left chopstick", "Philosopher #1 releases right chopstick", etc.
VERY IMPORTANT: Your algorithm should not have deadlocks regardless of the scheduling period used by your thread system and the order that threads run.

## Testing:
Project includes the tests used for validating the accuracy of the library. They are present in tests folder.
