#include "dissemination.h"

void OpenMP_DisseminationBarrierInit(int no_threads){
  // **** Initialize number of threads ****
  num_threads = no_threads;
  omp_set_num_threads(num_threads);
  // **** Create an barrier_arrray to store status of each node ****
  barrier_array = (node*)malloc(num_threads*sizeof(node));
  int i;
  // **** Initialize round number of every node to 0 and message received from to -1 ****
  for(i=0; i<num_threads; i++){
    barrier_array[i].round_no = 0;
    barrier_array[i].msg_received = -1;
  }
}

void OpenMP_DisseminationBarrier(){
  // **** Compute total number of rounds ****
  int total_rounds = ceil(log2(num_threads));
  int i;
  // **** Iterate over all rounds. In every round each node sends and receives a message ****
  for(i=0; i<total_rounds; i++){
    int thread_num = omp_get_thread_num();
    // **** Increment the round number. Should be done at beginning to avoid problems with consecutive barrier calls ****
    barrier_array[thread_num].round_no += 1;

    int send_to, recv_from;
    // **** Compute the node to whom message needs to be sent ****
    send_to = (thread_num + (int)(pow(2,i))) % num_threads;
    
    // **** Spin till round number of current and 'send_to' threads is same ****
    while(barrier_array[send_to].round_no != barrier_array[thread_num].round_no);

    // **** Set the value of msg_received to the thread_num to indicate message sent to 'send_to' node ****
    barrier_array[send_to].msg_received = thread_num;

    recv_from = (num_threads + thread_num - (int)(pow(2,i))) % num_threads;

    // **** Spin till current thread receives message ****
    while(barrier_array[thread_num].msg_received != recv_from);
    
    // printf("round_no = %d Message sent and received by thread: %d \n", i, thread_num);

    // **** Reset the msg_received variable before going to next round ****
    barrier_array[thread_num].msg_received = -1;
  }
}
