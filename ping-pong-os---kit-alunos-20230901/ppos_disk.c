#include "ppos_disk.h"
#include "ppos_data.h"
#include "ppos.h"
#include "disk.h"

#define DISK_BLOCK_SIZE 64
#define DISK_SIZE 256

//============================= GLOBALS =================================== // 

task_t* disk_suspended_queue;
semaphore_t* disk_mgr_sem;
semaphore_t* disk_sem;
disk_t* disk;

//============================= GLOBALS =================================== // 

typedef struct{
  task_t* task;
  int op;
  void* buffer;
  struct disk_task_t* prev;
  struct disk_task_t* next;
} disk_task_t;

//============================= GLOBALS =================================== // 

void disk_manager(void* args){
  
  while(1){

  }
}


int disk_mgr_init(int *numblocks, int *blockSize){
  //initiate disk manager
  //1- disk_task_queue
  //2- disk_semaphore
  //3- disk_manager_semaphore

  //disk_task_queue
  disk_suspended_queue = NULL;
  
  // create disk_semaphore
  sem_create(disk_sem, 0);

  //create disk_mgr_semaphore
  sem_create(disk_mgr_sem, 0);
  
  //setup signal handler
  
  //attribute values to numblocks and blockSize
  *numblocks = DISK_SIZE;
  *blockSize = DISK_BLOCK_SIZE;
  
  //launch disk_manager task

  //return operation status
  return 0;
}

int disk_block_read(int block, void *buffer){
  //Appends disk read task to disk task queue 

  //suspends task until disk block is read 

  //return status of operation
  return 0;
}

int disk_block_write(int block, void *buffer){
  //Appends write task to disk task queue

  //suspends task until disk block is written 

  //return operation status
  return 0;
}

void disk_sig_handler(){

}
