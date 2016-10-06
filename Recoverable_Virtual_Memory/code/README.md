## Steps to compile code
1) 	Makefile contains the command for compiling the librvm library. Use `make` command to compile the library. It will create librvm.a  
2) 	Testcases are persent in a folder named testcases. Thus the testcases can be compiled using following command:  
      g++ -g -std=c++11 testcases/basic.c librvm.a -o basic  
3) 	The above command will create an executable. Run the executable using:  
      ./basic  
4) 	Note: Dont forget to innclude rvm.h in testcase files by adding line: #include "../rvm.h"  

## How log files are used to accomplish persistency and transaction semantics?  
Persistency is accomplished as follows:  
1)	We create a log file named redo.txt on disk to keep the data after commiting transactions. A undo log is kept in memory to restore the contents in case the transaction is aborted before getting commited.  
2)	Every rvm object has a transaction map and every transaction has a list of log structures. Every log structure has parameters such as transaction id, size of log, pointer to segment base, offset, name of segment and pointer to undo log in memory.  
3)	Mapping of a segment leads to creation of a segment file with a name of corresponding segment.  
4)	When the truncate() function is called, the data from redo.txt file is written to appropriate segment file. This details for transferring this data from redo.txt to correct segment txt file is achieved using a particular redolog entry format mentioned in the following section. At the end of this call, the redo.txt becomes empty.  
5)	If the process is aborted, the undo log data is restored to appropriate memory locations in 'last in first out' manner from the undo log structure list.  
6)	At the end of a process, the segment file will consist of data that was truncated from the redo log file and redo.txt will consist of data that was commited by all the transactions.  

## Transaction Semantics:
1)	Atomicity: Either the entire transaction is commited or nothing is commited. Thus the redo.txt consists of data that is entirely commited. If a transaction is aborted, the changes made in the memory are restored back to 'before transaction' state using undo log.  
2)	Serializability: The data in redo.txt is present in exact same order as the transactions are commited.  
3)	Durability:	The redo.txt keeps the log data unless it is transferred to segment file or destroyed using rvm_destroy() function. Thus the data once commited remains immune of power failure, memory corruption, abrupt process termination, etc.  
4)	Consistency: Memory consistency is maintained by use of undo log. Either the transaction is commited and written to segment file or it is aborted. In later case, the memory contents are restored to original state.  

## What is written in them? How do the files get cleaned up, so that they do not expand indefinitely?

1) 	Segment files and redo log files are stored on disk. For every transaction, whenever there is a call to about_to_modify(), we create an object of log structure in memory which is used as undo_log.  
2)	On rvm_commit_tran(), records of redo log entries of the commited transaction are written to a redo.txt file. redo.txt is shared across various transactions and segments and is unique to an RVM.  
3)	Typical redolog entry looks like this -  
    \<segname\>,\<offset\>,\<size\>,\<actual_data\>  
    For multiple entries in the redo.txt, the above entry format is repeated as:  
    \<segname1\>,\<offset1\>,\<size1\>,\<actual_data1\>\<segname2\>,\<offset2\>,\<size2\>,\<actual_data2\>....  
4)	truncate() function applies these redo logs to the respective segments. This redo.txt is truncated if   
    i. the segment is being mapped next time or   
    ii. On explicit call to truncate() function  
5)	The call to truncate() function transfers all the data from redo.txt to correct corresponding segment file. This data is deleted from the redo.txt file. Thus indefinite expansion of the log file is avoided.  