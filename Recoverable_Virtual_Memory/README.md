# Recoverable Virtual Memory
## Introduction
The project includes implementation of recoverable virtual memory system like the ones described in the LRVM (https://web.eecs.umich.edu/~prabal/teaching/resources/eecs582/satya93lrvm.pdf) and Rio Vista (https://web.eecs.umich.edu/virtual/papers/lowell97.pdf) papers. Persistent segments of memory could be created and then accessed in a sequence of transactions.
Making the memory persistent is simple: mirror each segment of memory to a backup file on disk. The difficult part is implementing transactions. If the client crashes, or if the client explicitly requests an abort, the memory should be returned to the state it was in before the transaction started.
To implement a recoverable virtual memory system, log files are used. Before writing changes directly to the backup file, intended changes are first written to a log file. Then, if the program crashes, it is possible to read the log and see what changes were in progress.  

## Test Cases
In order to check how the project API is used, some test cases are included in the tests folder. 

