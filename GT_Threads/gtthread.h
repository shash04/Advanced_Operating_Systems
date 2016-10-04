#ifndef GTTHREAD_H
#define GTTHREAD_H

#include "steque.h"
#include <ucontext.h>

/* Define gtthread_t and gtthread_mutex_t types here */

typedef int gtthread_t;

// ******** Queues for tracking runnning threads and all threads ********
static steque_t threads_queue;
static steque_t all_threads;

// ******** Structure for thread ********
typedef struct gtthread{
	ucontext_t thread_context;	// to store context of current thread
	int thread_id;				// to store id of current thread
	void* ret_val;				// to store return value of thread
	int cancelled;				
	int completed;
	int waiting_for_me;
}gtthread;

// ******** Structure for mutex ********
typedef struct gtthread_mutex_t {
  steque_t lock_queue; 
  int lock;
  int locked_thread_id;
}gtthread_mutex_t;

void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);


int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
int  gtthread_mutex_destroy(gtthread_mutex_t *mutex);
#endif
