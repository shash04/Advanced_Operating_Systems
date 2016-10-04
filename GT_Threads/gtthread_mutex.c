/**********************************************************************
gtthread_mutex.c.  

This file contains the implementation of the mutex subset of the
gtthreads library.  The locks can be implemented with a simple queue.
 **********************************************************************/

/*
  Include as needed
*/

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "gtthread.h"

/*
  The gtthread_mutex_init() function is analogous to
  pthread_mutex_init with the default parameters enforced.
  There is no need to create a static initializer analogous to
  PTHREAD_MUTEX_INITIALIZER.
 */
int gtthread_mutex_init(gtthread_mutex_t* mutex){
  steque_init(&mutex->lock_queue);
  mutex->lock = 0;
  mutex->locked_thread_id = -1;
  return 0;
}

/*
  The gtthread_mutex_lock() is analogous to pthread_mutex_lock.
  Returns zero on success.
 */
int gtthread_mutex_lock(gtthread_mutex_t* mutex){
  gtthread* curr = (gtthread*) getFrontThread();
  
  // ******** If lock available, lock it ********
  if (mutex->lock == 0) {
    mutex->lock = 1;
    mutex->locked_thread_id = curr->thread_id; 
    return 0;
  }
  // ******** Enueue the thread on the lock_queue ********
  steque_enqueue(&mutex->lock_queue, &curr->thread_id);
 
  // ******** spin while locked ********
  while (mutex->lock == 1){
      gtthread_yield();
  }

  // ******** Lock unlocked, front of lock_queue gets the lock ********
  mutex->lock = 1;
  gtthread* next_thread_wanting_lock = (gtthread*)steque_front(&mutex->lock_queue);
  mutex->locked_thread_id = next_thread_wanting_lock->thread_id;
  steque_pop(&mutex->lock_queue);
  return 0;
}

/*
  The gtthread_mutex_unlock() is analogous to pthread_mutex_unlock.
  Returns zero on success.
 */
int gtthread_mutex_unlock(gtthread_mutex_t *mutex){
  mutex->locked_thread_id = -1;
  mutex->lock = 0;
  return 0;
}

/*
  The gtthread_mutex_destroy() function is analogous to
  pthread_mutex_destroy and frees any resourcs associated with the mutex.
*/
int gtthread_mutex_destroy(gtthread_mutex_t *mutex){
  steque_destroy(&mutex->lock_queue);
  return 0;
}
