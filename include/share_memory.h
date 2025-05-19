
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H
#include "process.h"
typedef struct{
  int size;
  int free;
  Process process[100];
  int number_of_proccess;

  
}Shared_memory;

#endif