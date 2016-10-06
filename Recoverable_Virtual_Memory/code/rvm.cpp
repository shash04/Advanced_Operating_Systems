#include "rvm.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <sstream>

int globalrvm_Id = 0;
int globalt_id = 0;

map<int, rvm_t*> global_rvm_map;

// Check and create directory.
void createDir(const char *directory){
  int status;
  status = mkdir(directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  if(status != 0){
    if(!(errno == EEXIST)){
      cout<<"ERROR: Could not create directory!"<<endl;
      exit(1);
    }
    else{
      cout<<"RVM Directory already exists!"<<endl;
    }
  }
}

// Check if file with filename exists
bool fexists(const string& filename) {
  ifstream ifile(filename.c_str());
  return ifile;
}

// Checks if the rvm object's map has seg_Name. If found and if it is already mapped, return -2.
// If found and not mapped then return its size. if not found return -1.
int alreadyExists(string seg_Name, rvm_t *l_rvm){
  map<string, segment>::iterator it;
  it = l_rvm->rvm_map.find(seg_Name);
  if(it != l_rvm->rvm_map.end()){
    if(it->second.mapped == 0)
      return it->second.size;
    else{
      cout<<"ERROR: Trying to map same segment twice"<<endl;
      return -2;
    }
  }
  else
    return -1;
}

rvm_t* isRVMobjInitialized(int rvm_id){
  map<int, rvm_t*>::iterator it_l;
  it_l = global_rvm_map.find(rvm_id);
  if(it_l == global_rvm_map.end()){
    cout<<"RVM object with the given id isn't initialized"<<endl;
    exit(1);
  }
  return it_l->second;
}

void wasMemAllocated(void* new_ptr, string obj_type){
  if(new_ptr == NULL){
    cout<<"ERROR: Could not allocate memory to object of = "<<obj_type<<endl;
    exit(1);
  }
}

// Check if [segname].txt exists. If no, create one. If yes, check size and expand it.
void modifySegmentFile(segment *new_Segment){
  string segFilePath = new_Segment->seg_file;
  FILE *fp;
  fp = fopen(segFilePath.c_str(), "ab+"); 
  if (fp == NULL){
    cout <<"ERROR: Could not open the file!"<<endl;
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  if( file_size == -1 ){
    fclose(fp);
    cout << "ERROR: File size error!"<<endl;
    exit(1);
  }
  fseek(fp, 0, SEEK_SET);

  // cout << "Old file size : " << file_size << "\tNew file size : " << new_Segment->size << endl;
  if(file_size == 0){
    memset(new_Segment->original_seg, ' ', new_Segment->size);
  }
  else if(file_size < new_Segment->size){
    memset((char*)new_Segment->original_seg + file_size, ' ', new_Segment->size - file_size);
    fwrite((char*)new_Segment->original_seg + file_size, new_Segment->size  - file_size, 1, fp);
  }

  fseek(fp, 0, SEEK_SET);
  //There were 2 modifications here
  //1 : We have to read in whatever is there in the file. Hence file_size and not segment size
  //2 : If we read in everything then the null terminating char causes a new line to be inserted. Hence -1.
  fread(new_Segment->original_seg, file_size, 1, fp);
  fclose(fp);
}

// Checks if all segbases in array **segbases belong to rvm. If yes, check if they are mapped and their tid == current transn id
bool checkAllSeg(rvm_t rvm, int numsegs, void **segbases){
  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);

  for(int i=0; i<numsegs; i++){
    auto it = l_rvm->rvm_map.begin();
    for(; it != l_rvm->rvm_map.end(); it++){
      if(it->second.original_seg == segbases[i]){
        if(it->second.mapped != 1 || it->second.tid != -1){ //&& it->second.tid != rvm_id)){
          cout << "mapped : " << it->second.mapped << "tid : " << it->second.tid << endl;
          return false;
        }
        break;
      }
    }
    if(it == l_rvm->rvm_map.end())
      return false;
  }
  return true;
}

rvm_t* findRVMobjPtr(map<trans_t, transaction>::iterator &it_trans, trans_t tid){
  rvm_t *l_rvm;
  for(auto it = global_rvm_map.begin(); it != global_rvm_map.end(); ++it){
    it_trans = it->second->rvm_trans_map.find(tid);
    if(it_trans != it->second->rvm_trans_map.end()){
      l_rvm = it->second;
      return l_rvm;
    }
  }
  return NULL;
}

/*
  Initialize the library with the specified directory as backing store.
  Create an directory where we put all file for disk segments and return RVM object passed to other methods
*/
rvm_t rvm_init(const char *directory){

  createDir(directory);
  rvm_t *rvm = new rvm_t;
  wasMemAllocated((void*)rvm, "rvm");

  rvm->rvm_id = ++globalrvm_Id;
  rvm->directory = directory;
  global_rvm_map.insert(pair<int, rvm_t*>(rvm->rvm_id, rvm));

  return *rvm;
}

/*
  map a segment from disk into memory. If the segment does not already exist, then create it and give it size size_to_create. 
  If the segment exists but is shorter than size_to_create, then extend it until it is long enough. 
  It is an error to try to map the same segment twice.
  Note: Changes made in memory aren't immediately written to disk. This happens only when a transaction is completed
*/
void *rvm_map(rvm_t rvm, const char *segname, int size_to_create){

  if(&rvm == NULL || segname == NULL || size_to_create < 0){
    cout<<"ERROR: Parameters to rvm_map are wrong"<<endl;
    exit(1);
  }

  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);

  rvm_truncate_log(*l_rvm);

  string seg_Name(segname);
  int ret_exist = alreadyExists(seg_Name, l_rvm);
  if(ret_exist == -2){
    cout<<"Mapping same segment twice!"<<endl;
    return (void*) -1;
  }
  if(ret_exist != -1){
    l_rvm->rvm_map[seg_Name].size = size_to_create;
    l_rvm->rvm_map[seg_Name].original_seg = realloc(l_rvm->rvm_map[seg_Name].original_seg, size_to_create);
    l_rvm->rvm_map[seg_Name].mapped = 1;
  }
  else{
    segment *new_Segment = new segment;
    wasMemAllocated((void*)new_Segment, "new_segment in rvm_map()");
    new_Segment->seg_name = seg_Name;
    new_Segment->size = size_to_create;
    new_Segment->mapped = 1;
    new_Segment->original_seg = malloc(size_to_create);
    new_Segment->tid = -1;
    new_Segment->seg_file = string(rvm.directory) + "/" + seg_Name + ".txt";
    l_rvm->rvm_map.insert(pair<string, segment>(seg_Name, *new_Segment));
  }

  modifySegmentFile(&l_rvm->rvm_map[seg_Name]);
  return (void*)l_rvm->rvm_map[seg_Name].original_seg;
}

/*
  unmap a segment from memory.
*/
void rvm_unmap(rvm_t rvm, void *segbase){
  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);

  for(auto it = l_rvm->rvm_map.begin(); it != l_rvm->rvm_map.end(); it++){
    // cout<<it->first<<"  of size = "<<it->second.size<<endl;
    if(it->second.original_seg == segbase){
      it->second.mapped = 0;
      // cout<<"Unmapped the segment"<<endl;
      return;
    }
  }

  //If control reaches here then it means segbase was not found!
  cout << "Trying to unmap segment which was never mapped \n";
  //exit(0);
}

/*
  destroy a segment completely, erasing its backing store. This function should not be called on a segment that is currently mapped.
 */
void rvm_destroy(rvm_t rvm, const char *segname){
  if(&rvm == NULL || segname == NULL){
    cout<<"ERROR: NULL parameters passed to rvm_destroy()"<<endl;
    exit(1);
  }
  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);
  string seg_Name(segname);
  string filePath = string(rvm.directory) + "/" + seg_Name + ".txt";

  // Free the segment memory, free log memory, delete log entry from modSegParts list and delete segment entry from rvm_map
  auto it_seg = l_rvm->rvm_map.begin();
  for(; it_seg != l_rvm->rvm_map.end(); it_seg++){
    if(it_seg->second.seg_name == seg_Name){
      if(it_seg->second.mapped == 1){
        cout<<"Destroy can not be called on a mappped segment"<<endl;
        exit(1);
      }
      break;
    }
  }
  if(it_seg == l_rvm->rvm_map.end()){
    // cout<<"Segment with name "<<seg_Name<<" doesn't exist"<<endl;
    return;
  }
  else{
    free(it_seg->second.original_seg);
    int tid = it_seg->second.tid;
    auto it_log = l_rvm->rvm_trans_map[tid].modSegParts.begin();
    for(;it_log != l_rvm->rvm_trans_map[tid].modSegParts.end(); ++it_log){
      if((*it_log).seg_name == seg_Name){
        free((*it_log).logbase);
        l_rvm->rvm_trans_map[tid].modSegParts.erase(it_log);
        it_log--;
      }
    }
    l_rvm->rvm_map.erase(seg_Name);
  }

  string redoFilePath = string(l_rvm->directory) + "/redo.txt";
  int offset, size;
  string fsegname;
  ifstream inFile;
  inFile.open(redoFilePath);          // open the input file
  stringstream strStream;
  strStream << inFile.rdbuf();        // read the file
  string redoData = strStream.str();
  string new_redoData = "";
  FILE *fpr;
  fpr = fopen(redoFilePath.c_str(), "w"); 
  if (fpr == NULL){
    return;
  }
  fclose(fpr);
  std::string delim = ",";
  auto start = 0U;
  while(redoData.size() != 0){
    // cout << "Redo Data = "<< redoData << endl;
    auto end = redoData.find(delim);
    fsegname = redoData.substr(start, end - start);
    redoData.erase(start, end - start + 1);
       
    end = redoData.find(delim, start);
    offset = stoi(redoData.substr(start, end - start));
    redoData.erase(start, end - start + 1);

    end = redoData.find(delim, start);
    size = stoi(redoData.substr(start, end - start));
    redoData.erase(start, end - start + 1);

    string data = redoData.substr(start, size);

    // cout << "Segname : " << fsegname << "\tOffset : " << offset << "\tsize : " << size << endl;
    if(fsegname == seg_Name){
      redoData.erase(start, size);
      continue;
    }

    new_redoData += fsegname + ',' + to_string(offset) + ',' + to_string(size) + ',' + data;
    redoData.erase(start, size);
    // cout << "New Redo Data = "<< new_redoData << endl;
  }
  fpr = fopen(redoFilePath.c_str(), "w"); 
  fseek(fpr, 0, SEEK_SET);
  fwrite((void*)(new_redoData.c_str()), new_redoData.size(), 1, fpr);
  fclose(fpr);

  // Delete the segment file from the directory
  if(fexists(filePath)){
    if(remove(filePath.c_str()) != 0)
      cout<<"Error destroying segment file!"<<endl;
  }

}

// ****** Transaction part of the system ******
/*
  begin a transaction that will modify the segments listed in segbases.
  **segbases = array of memory addresses, each one of which is returned by call to rvm_map.
  If any of the specified segments is already being modified by a transaction, then the call should fail and return (trans_t) -1.
  Note that trant_t needs to be able to be typecasted to an integer type.
 */
trans_t rvm_begin_trans(rvm_t rvm, int numsegs, void **segbases){

  if((numsegs < 0) || (*segbases == NULL)){
    return (trans_t)-1;
  }

  if(&rvm == NULL){
    cout<<"ERROR: NULL rvm passed to rvm_begin_trans"<<endl;
    exit(1);
  }
  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);
  
  transaction *new_transaction = new transaction;
  wasMemAllocated((void*)new_transaction, "new_transaction in rvm_begin_trans()");
  new_transaction->tid = ++globalt_id;
  new_transaction->finish = 0;

  // cout << "Now checking if all segs are mapped \n";
  if(!checkAllSeg(rvm, numsegs, segbases)){
    cout << "Check all segments failed \n";
    return (trans_t)-1;
  }

  for(int i=0; i<numsegs; i++){
    auto it = l_rvm->rvm_map.begin();
    for(; it != l_rvm->rvm_map.end(); it++){
      if(it->second.original_seg == segbases[i]){
        it->second.tid = new_transaction->tid;
      }
    }
  }

  l_rvm->rvm_trans_map.insert(pair<trans_t, transaction>(new_transaction->tid , *new_transaction));
  return (trans_t)new_transaction->tid;
}

/*
  declare that the library is about to modify a specified range of memory in the specified segment.
  The segment must be one of the segments specified in the call to rvm_begin_trans. *segbase = range of addresses that we want to change. 
  Your library needs to ensure that the old memory has been saved, in case an abort is executed.
  Thus done before the data is changed so that 'undo' log has correct info.
  It is legal to call rvm_about_to_modify multiple times on the same memory area.
*/
void rvm_about_to_modify(trans_t tid, void *segbase, int offset, int size){
  // segbase not declared = exit(1)
  map<trans_t, transaction>::iterator it_trans;
  rvm_t *l_rvm = findRVMobjPtr(it_trans, tid);
  string currSegName;
  
  if(l_rvm == NULL){
    cout<<"Transaction with id "<<tid<<" not found in any rvm object!"<<endl;
    exit(1);
  }

  auto it_seg = l_rvm->rvm_map.begin();
  for(; it_seg != l_rvm->rvm_map.end(); it_seg++){
    if(it_seg->second.original_seg == segbase){
      if(it_seg->second.mapped != 1 || it_seg->second.tid != tid){
        cout<<"segbase found but it isnt mapped or tid is not correct"<<endl;
        exit(1);
      }
      currSegName = it_seg->second.seg_name;
      break;
    }
  }

  if(it_seg->second.size < (offset + size)){
    cout << "Trying to modify outside segment limits" << endl;
    return;
  }
  //it_seg->second : segment with original_seg = segbase

  logInfo *undo_log = new logInfo;
  wasMemAllocated((void*)undo_log, "undo_log structure in about_to_modify()");
  undo_log->offset = offset;
  undo_log->log_size = size;
  undo_log->tid = tid;
  undo_log->segbase = segbase;
  undo_log->seg_name = currSegName;
  undo_log->logbase = malloc(size);
  wasMemAllocated((void*)undo_log->logbase, "undo_log copy of data in about_to_modify()");
  memcpy((char*)undo_log->logbase, (char*)segbase + offset, size);
  it_trans->second.modSegParts.push_back(*undo_log);
}

/*
  commit all changes that have been made within the specified transaction. 
  When the call returns, then enough information should have been saved to disk so that, even if the program crashes, 
  the changes will be seen by the program when it restarts.
  Thus this method should not return until all the changes made by transaction have been recorded in 'redo' log
*/
void rvm_commit_trans(trans_t tid){
  map<trans_t, transaction>::iterator it_trans;
  rvm_t *l_rvm = findRVMobjPtr(it_trans, tid);

  string redoFilePath = string(l_rvm->directory) + "/redo.txt";
  FILE *fp;
  fp = fopen(redoFilePath.c_str(), "ab+"); 
  if (fp == NULL){
    cout <<"ERROR: Could not open the file!"<<endl;
    exit(1);
  }
  fseek(fp, 0, SEEK_END);
  long file_size = ftell(fp);
  if( file_size == -1 ){
    fclose(fp);
    cout << "ERROR: File size error!"<<endl;
    exit(1);
  }    

  fseek(fp, file_size, SEEK_SET);

  if(it_trans->second.modSegParts.size() == 0){
    cout << "We are calling commit without calling about to modify!!" << endl;
    //Test unlogged_modific1.c
  }

  for(auto it = it_trans->second.modSegParts.begin(); it != it_trans->second.modSegParts.end(); it++){
    // cout << "segname " << (*it).seg_name << "\tOffset :" << (*it).offset << "\tSize : " << (*it).log_size <<endl;

    //Write segname
    string temp = (*it).seg_name + ',';
    fwrite((char*)temp.c_str(), temp.size(), 1, fp);
    //Write offset
    temp = std::to_string((*it).offset) + ',';
    fwrite((char*)temp.c_str(), temp.size(), 1, fp);
    //Write size
    temp = to_string((*it).log_size) + ',';
    fwrite((char*)temp.c_str(), temp.size(), 1, fp);
    //Write actual data
    fwrite((char*)(*it).segbase + (*it).offset, (*it).log_size, 1, fp);
    (*it).logbase = realloc((*it).logbase, (*it).log_size);
    memcpy((char*)(*it).logbase, (char*)(*it).segbase + (*it).offset , (*it).log_size);
  }
  fclose(fp);

  auto it_seg = l_rvm->rvm_map.begin();
  for(; it_seg != l_rvm->rvm_map.end(); it_seg++){
    if(it_seg->second.tid == tid){
      it_seg->second.tid = -1;
    }
  }

  it_trans->second.finish = 1;
  it_trans->second.tid = -1;      // Not sure if this is the correct place for this
}

/*
  undo all changes that have happened within the specified transaction.
 */
void rvm_abort_trans(trans_t tid){
  map<trans_t, transaction>::iterator it_trans;
  rvm_t *l_rvm = findRVMobjPtr(it_trans, tid);

  // Returns iterator which points AFTER the last element. Hence had to use do while
  auto it = it_trans->second.modSegParts.end();
  do{
    it--;
    int offset = (*it).offset;
    int size = (*it).log_size;
    memcpy((char*)(*it).segbase + offset , (char*)(*it).logbase, size);

    // Reset tid so that this segment can be resused.
    l_rvm->rvm_map[(*it).seg_name].tid = -1;

  } while(it != it_trans->second.modSegParts.begin());
  l_rvm->rvm_trans_map.erase(tid);
}

/*
 Play through any committed or aborted items in the log file(s) and shrink the log file(s) as much as possible.
 Apply all the changes in 'redo' log to data segments
*/
void rvm_truncate_log(rvm_t rvm){
  int rvm_id = rvm.rvm_id;
  rvm_t* l_rvm = isRVMobjInitialized(rvm_id);
  string redoFilePath = string(l_rvm->directory) + "/redo.txt";
  int offset, size;
  string segname;

  ifstream inFile;
  inFile.open(redoFilePath);                //open the input file
  stringstream strStream;
  strStream << inFile.rdbuf();              //read the file
  string redoData = strStream.str();
  FILE *fpr;
  fpr = fopen(redoFilePath.c_str(), "w"); 
  if (fpr == NULL){
    return;
  }
  fclose(fpr);
  std::string delim = ",";
  auto start = 0U;

  while(redoData.size() != 0){
        auto end = redoData.find(delim);
        segname = redoData.substr(start, end - start);
        redoData.erase(start, end - start + 1);
           
        end = redoData.find(delim, start);
        offset = stoi(redoData.substr(start, end - start));
        redoData.erase(start, end - start + 1);

        end = redoData.find(delim, start);
        size = stoi(redoData.substr(start, end - start));
        redoData.erase(start, end - start + 1);

        // cout << "Segname : " << segname << "\tOffset : " << offset << "\tsize : " << size << endl;
        
        string segFilePath = string(rvm.directory) + "/" + segname + ".txt";
        FILE *fp;
        fp = fopen(segFilePath.c_str(), "r+"); 

        if (fp == NULL){
          cout <<"ERROR: Could not open the file!"<<endl;
          exit(1);
        }

        fseek(fp, offset, SEEK_SET);
        fwrite((void*)(redoData.c_str()), size, 1, fp);
        redoData.erase(start, size);
        // cout << redoData << endl;
        fclose(fp);
  }
}