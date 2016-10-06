#ifndef _RVM_INTERNAL_H
#define _RVM_INTERNAL_H

#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

using namespace std;

typedef int trans_t;

struct logInfo{
  int tid;
  void *logbase;
  void *segbase;
  int offset;
  int log_size;
  string seg_name;
};

struct segment{
  string seg_name;
  int size;
  int mapped;
  void* original_seg;
  string seg_file;
  int tid;
};

struct transaction{
  int tid;
  list<logInfo> modSegParts; 
  bool finish;
};

struct rvm_t{
  int rvm_id;
  map<string, segment> rvm_map;
  map<trans_t, transaction> rvm_trans_map;
  const char* directory;
};


#endif