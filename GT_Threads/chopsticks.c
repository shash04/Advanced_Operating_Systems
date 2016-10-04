#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <pthread.h>

#include "philosopher.h"

pthread_mutex_t pchopstick[5];

/*
 * Performs necessary initialization of mutexes.
 */
void chopsticks_init(){
	int i;
	for(i=0; i<5; i++)
	  pthread_mutex_init(&pchopstick[i], NULL);
}

/*
 * Cleans up mutex resources.
 */
void chopsticks_destroy(){
  int i;
  for(i = 0; i < 5; i++)
    pthread_mutex_destroy(&pchopstick[i]);
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void pickup_chopsticks(int phil_id){
  if(!(phil_id%2)){
    pthread_mutex_lock(&pchopstick[phil_id]);
    pickup_left_chopstick(phil_id);
    pthread_mutex_lock(&pchopstick[(phil_id + 1) % 5]);
    pickup_right_chopstick(phil_id);
  }
  else{
    pthread_mutex_lock(&pchopstick[(phil_id + 1) % 5]);
    pickup_right_chopstick(phil_id);
    pthread_mutex_lock(&pchopstick[phil_id]);
    pickup_left_chopstick(phil_id);
  }
}

/*
 * Uses pickup_left_chopstick and pickup_right_chopstick
 * to pick up the chopsticks
 */   
void putdown_chopsticks(int phil_id){
  putdown_left_chopstick(phil_id);
  pthread_mutex_unlock(&pchopstick[phil_id]);
  putdown_right_chopstick(phil_id);
  pthread_mutex_unlock(&pchopstick[(phil_id + 1) % 5]);
}
