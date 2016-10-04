/**********************************************************************
gtthread_sched.c.  

This file contains the implementation of the scheduling subset of the
gtthreads library.  A simple round-robin queue should be used.
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
   Students should define global variables and helper functions as
   they see fit.
 */

// global thread id to assign new thread unique thread_id
int global_thread_id = 0;
gtthread* main_thread;
static sigset_t vtalrm;
struct itimerval *T;
struct sigaction act;

// Puts front to back. Pops if new front is scheduled to be cancelled. Swaps conteext with new front

void print_queue(void){
  gtthread* ret;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  int len = steque_size(&threads_queue);
  printf("Queue size: %d\n", steque_size(&threads_queue));
  int i;
  for(i=0; i<len; i++){
    ret = (gtthread*) steque_pop(&threads_queue);
    steque_enqueue(&threads_queue, ret);
    printf("%d->", (int)ret->thread_id);
    fflush(stdout);
  }
  printf("\n\n");
  fflush(stdout);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}


// Function to check if there is a thread in queue and give control to next in line
void run_next(gtthread* current_thread){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  if(!steque_isempty(&threads_queue)){   
    gtthread* next_to_run = (gtthread*) steque_front(&threads_queue);
    while(next_to_run->cancelled){
      next_to_run->completed = 1;
      if(next_to_run->waiting_for_me){
        next_to_run->waiting_for_me = -1;
      }
      steque_pop(&threads_queue);
      if(steque_isempty(&threads_queue))
        break;
      next_to_run = (gtthread*) steque_front(&threads_queue);
    }
    if(steque_isempty(&threads_queue))
        return;
    swapcontext(&current_thread->thread_context, &next_to_run->thread_context);
    if(steque_isempty(&threads_queue))
      exit(0);
  }
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}

// Function to place front thread to last place and run new front
void cycle_and_run_front(){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread* current_thread = (gtthread*) steque_front(&threads_queue);
  steque_cycle(&threads_queue);
  run_next(current_thread);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}

// Function to get the pointer to the front thread of queue
gtthread* getFrontThread(){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread* curr = (gtthread*) steque_front(&threads_queue);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return curr;
}

// Executes the function passed as an argument to gtthread_create
static void execute_function(void *(*start_routine)(void *), void *arg) {
  void *ret_val;
  ret_val = start_routine(arg);
  gtthread_exit(ret_val);
}

// Function that executes when a timer signal is raised
void alrm_handler(int sig){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  cycle_and_run_front();
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}

// Function to search a thread in threads_queue using thread_id
gtthread* search_thread(gtthread_t thread_id){
  gtthread* ret;
  gtthread* temp;
  ret = NULL;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  int len = steque_size(&threads_queue);
  int i;
  for(i=0; i<len; i++) {
    temp = (gtthread*) steque_pop(&threads_queue);
    steque_enqueue(&threads_queue, temp);
    if (temp->thread_id == thread_id) {
      sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
      ret = temp;
    }
  }
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return ret;
}

// Function to search a thread in all_threads using thread_id
gtthread* search_thread_in_all_threads(gtthread_t thread_id){
  gtthread* ret;
  gtthread* temp;
  ret = NULL;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  int len = steque_size(&all_threads);
  int i;
  for(i=0; i<len; i++) {
    temp = (gtthread*) steque_pop(&all_threads);
    steque_enqueue(&all_threads, temp);
    if (temp->thread_id == thread_id) {
      sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
      ret = temp;
    }
  }
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return ret;
}


/*
  The gtthread_init() function does not have a corresponding pthread equivalent.
  It must be called from the main thread before any other GTThreads
  functions are called. It allows the caller to specify the scheduling
  period (quantum in micro second), and may also perform any other
  necessary initialization.  If period is zero, then thread switching should
  occur only on calls to gtthread_yield().

  Recall that the initial thread of the program (i.e. the one running
  main() ) is a thread like any other. It should have a
  gtthread_t that clients can retrieve by calling gtthread_self()
  from the initial thread, and they should be able to specify it as an
  argument to other GTThreads functions. The only difference in the
  initial thread is how it behaves when it executes a return
  instruction. You can find details on this difference in the man page
  for pthread_create.
 */
void gtthread_init(long period){

  main_thread = malloc(sizeof(gtthread));

  if(main_thread != NULL){

    main_thread->thread_id = global_thread_id++;
    main_thread->cancelled = 0;
    main_thread->completed = 0;
    main_thread->waiting_for_me = -1;

    // ******** Initialize thread queue ********
    steque_init(&threads_queue);
    steque_init(&all_threads);
    steque_enqueue(&threads_queue, main_thread);
    steque_enqueue(&all_threads, main_thread);

    // ******** Create context ********
    if (getcontext(&main_thread->thread_context) == -1){
      perror("getcontext error");
      exit(EXIT_FAILURE);
    }

    main_thread->thread_context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
    main_thread->thread_context.uc_stack.ss_size = SIGSTKSZ;


    if (period != 0) {
      // ******** Setting up the signal mask ********
      sigemptyset(&vtalrm);
      sigaddset(&vtalrm, SIGVTALRM);
      sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);

      // ******** Setting up the alarm ********
      T = (struct itimerval*) malloc(sizeof(struct itimerval));
      T->it_value.tv_sec = T->it_interval.tv_sec = 0;
      T->it_value.tv_usec = T->it_interval.tv_usec = period;

      setitimer(ITIMER_VIRTUAL, T, NULL);

      // ******** Setting up the handler ********
      memset(&act, '\0', sizeof(act));
      act.sa_handler = &alrm_handler;
      if (sigaction(SIGVTALRM, &act, NULL) < 0){
        perror ("sigaction");
      }
    }
  }
}

/*
  The gtthread_create() function mirrors the pthread_create() function,
  only default attributes are always assumed.
 */
int gtthread_create(gtthread_t *thread,
        void *(*start_routine)(void *),
        void *arg){

  gtthread* new_thread = malloc(sizeof(gtthread));

  if(new_thread != NULL){

    // ******** Initialize new_thread parameters ********
    new_thread->thread_id = global_thread_id++;
    *thread = new_thread->thread_id;
    new_thread->completed = 0;
    new_thread->cancelled = 0;
    new_thread->waiting_for_me = -1;

    if (getcontext(&new_thread->thread_context) == -1) {
      perror("getcontext error");
      exit(EXIT_FAILURE);
    }
     
    // ******** Allocate a new_thread stack ********
    new_thread->thread_context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
    new_thread->thread_context.uc_stack.ss_size = SIGSTKSZ;

    sigprocmask(SIG_BLOCK, &vtalrm, NULL);
    gtthread* current_running = (gtthread*) steque_front(&threads_queue);

    // ******** Next context link is currently running thread ********
    new_thread->thread_context.uc_link = &current_running->thread_context;   
    makecontext(&new_thread->thread_context, (void (*)(void))execute_function, 2, start_routine, arg);
    steque_enqueue(&threads_queue, new_thread);
    steque_enqueue(&all_threads, new_thread);
    sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);

  }
  return 0;

}

/*
  The gtthread_join() function is analogous to pthread_join.
  All gtthreads are joinable.
 */
int gtthread_join(gtthread_t thread, void **status){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread* target_thread = search_thread(thread);
  gtthread* target_thread_in_all_threads = search_thread_in_all_threads(thread);
  int self_tid = gtthread_self();

  if(target_thread != NULL){
    // ******** Deadlock if current thread calls gtthread_join on itself ********
    if (gtthread_equal(thread, gtthread_self()))
      return -1;

    gtthread* calling_thread = search_thread(self_tid);
    target_thread->waiting_for_me = self_tid;

    // ******** Deadlock if join called on target thread that is already waiting for current calling thread ********
    if(target_thread->waiting_for_me == calling_thread->thread_id && calling_thread->waiting_for_me == target_thread->thread_id){
      // ******** Exception case where the target thread cancellation is in pipe and thus no deadlock ********
      if(target_thread->cancelled == 1){
        *status = target_thread->ret_val;
        return 0;
      }
      return -1;
    }

    // ******** Keep yielding till the target thread is completetd or cancelled ********
    while (!target_thread->completed && !target_thread->cancelled){
      gtthread_yield();
    }

    if(status != NULL){
      *status = target_thread->ret_val;
    }
  }
  else{
    if(status != NULL){
      *status = target_thread_in_all_threads->ret_val;
    }
  }
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return 0;
}

/*
  The gtthread_exit() function is analogous to pthread_exit.
 */
void gtthread_exit(void* ret_val){
  gtthread* current_running;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);

  current_running = (gtthread*) steque_pop(&threads_queue);           // Remove the thread from queue
  current_running->completed = 1;                                     // Mark thread as completed
  current_running->ret_val = ret_val;                                 // Set the return value

  gtthread* thread_in_all_threads = search_thread_in_all_threads(current_running->thread_id);
  thread_in_all_threads->ret_val = ret_val;

  if(current_running->waiting_for_me){
    current_running->waiting_for_me = -1;
  }

  if(!steque_isempty(&threads_queue)) run_next(current_running);
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);    
}


/*
  The gtthread_yield() function is analogous to pthread_yield, causing
  the calling thread to relinquish the cpu and place itself at the
  back of the schedule queue.
 */
void gtthread_yield(void){
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  cycle_and_run_front();
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
}

/*
  The gtthread_yield() function is analogous to pthread_equal,
  returning zero if the threads are the same and non-zero otherwise.
 */
int  gtthread_equal(gtthread_t t1, gtthread_t t2){
  return (t1 == t2);
}

/*
  The gtthread_cancel() function is analogous to pthread_cancel,
  allowing one thread to terminate another asynchronously.
 */
int  gtthread_cancel(gtthread_t thread){
  int return_val;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread* to_cancel = search_thread(thread);
  gtthread* to_cancel_in_all_threads = search_thread_in_all_threads(thread);

  if(to_cancel != NULL){
    // ******** Case where gtthread_cancel called on self ********
    if(to_cancel->thread_id == gtthread_self()){
      to_cancel->cancelled = 1;
      to_cancel_in_all_threads->cancelled = 1;
      return_val = ((void *)(size_t) -1);                   // return value is equivalent to PTHREAD_CANCELLED
      gtthread_exit(return_val);
    }
    else{
      to_cancel->cancelled = 1;
      to_cancel_in_all_threads->cancelled = 1;
      return_val = 0;
      to_cancel_in_all_threads->ret_val = ((void *)(size_t) -1);
      to_cancel->ret_val = ((void *)(size_t) -1);
    }
  }
  else{
    return_val = -1;
  }
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return return_val;
}

/*
  Returns calling thread.
 */
gtthread_t gtthread_self(void){
  gtthread_t return_val;
  sigprocmask(SIG_BLOCK, &vtalrm, NULL);
  gtthread* curr = (gtthread*) steque_front(&threads_queue);
  return_val = curr->thread_id;
  sigprocmask(SIG_UNBLOCK, &vtalrm, NULL);
  return return_val;
}
