#include<signal.h>

#include "ppos_disk.h"
#include "ppos_data.h"
#include "ppos.h"
#include "disk.h"

#define DISK_BLOCK_SIZE 64
#define DISK_SIZE 256

//============================= STRUCTS =================================== // 

typedef struct{
  task_t* task;
  int op;
  void* buffer;
  struct disk_task_t* prev;
  struct disk_task_t* next;
} disk_task_t;

//============================= GLOBALS =================================== // 

task_t* disk_suspended_queue;
task_t* disk_mgr_task;
disk_task_t* disk_task_queue;
semaphore_t* disk_mgr_sem;
semaphore_t* disk_sem;
disk_t* disk;

struct sigaction disk_sig;
int disk_sig_flag;
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

  //disk_suspended_queue
  disk_suspended_queue = NULL;
  disk_task_queue = NULL;
  disk_sig_flag = 0;

  // create disk_semaphore
  if (sem_create(disk_sem, 1) < 0){return -1;}

  //create disk_mgr_semaphore
  if (sem_create(disk_mgr_sem, 0) < 0){return -1;}
  
  //setup signal handler
  if(disk_sig_handler_setup() < 0){ return -1;} 

  //attribute values to numblocks and blockSize
  *numblocks = DISK_SIZE;
  *blockSize = DISK_BLOCK_SIZE;
  
  //launch disk_manager task
  if(task_create(disk_mgr_task, disk_manager, NULL) < 0){return -1;} 

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

int disk_sig_handler_setup(){
  disk_sig.sa_handler = disk_sig_handler;
  sigemptyset(&disk_sig.sa_mask);
  disk_sig.sa_flags = 0;
  
  if(sigaction(SIGUSR1, &disk_sig, 0) < 0){return -1;}
  return 0;
}

void disk_sig_handler(){
  //raise signal flag
  disk_sig_flag = 1;

  //wake disk_manager
  sem_up(disk_mgr_sem);
}
