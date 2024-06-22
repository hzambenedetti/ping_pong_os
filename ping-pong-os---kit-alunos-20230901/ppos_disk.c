#include<signal.h>

#include "ppos_disk.h"
#include "ppos-core-globals.h"
#include "ppos_data.h"
#include "ppos.h"
#include "disk.h"

// #define DISK_BLOCK_SIZE 64
// #define DISK_SIZE 256

//============================= STRUCTS =================================== // 

typedef struct disk_task_t{
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
semaphore_t* disk_task_sem;

disk_t* disk;

struct sigaction disk_sig;
int disk_sig_flag;

//============================= FUNCTION DEFINITIONS =================================== // 

void append_disk_task(disk_task_t* task);

void task_suspend_disk(task_t* task);

//============================= FUNCTION IMPLEMENTATION =================================== // 

void disk_manager(void* args){
  
  while(1){

  }
}


int disk_mgr_init(int *numblocks, int *blockSize){
  //initiate disk manager
  //1- disk_task_queue
  //2- disk_semaphore
  //3- disk_manager_semaphore
  //4- init disk

  //disk_suspended_queue
  disk_suspended_queue = NULL;
  disk_task_queue = NULL;
  disk_sig_flag = 0;

  // create disk_semaphore
  if (sem_create(disk_sem, 1) < 0){return -1;}

  //create disk_mgr_semaphore
  if (sem_create(disk_mgr_sem, 0) < 0){return -1;}
  
  //create disk_task_sem
  if(sem_create(disk_task_sem, 1) < 0){return -1;}
  
  //setup signal handler
  if(disk_sig_handler_setup() < 0){ return -1;} 
  
  //launch disk_manager task
  if(task_create(disk_mgr_task, disk_manager, NULL) < 0){return -1;}

  //init disk 
  if (disk_cmd(DISK_CMD_INIT, 0, 0) < 0){return -1;};

  //attribute values to numblocks and blockSize
  *numblocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
  *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);
  
  //return operation status
  return 0;
}

int disk_block_read(int block, void *buffer){
  //create disk_task
  disk_task_t* d_task = (disk_task_t*) malloc(sizeof(disk_task_t));
  
  //set disk task values
  d_task->task = taskExec;
  d_task->buffer = buffer;
  d_task->op = DISK_CMD_READ;
  d_task->next = NULL;
  d_task->prev = NULL;

  //Appends disk read task to disk task queue 
  append_disk_task(d_task);
  
  //call disk_manager task
  sem_up(disk_mgr_sem); 
  
  //suspends task until disk block is read
  task_suspend_disk(taskExec);
  task_switch(taskDisp);

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

void append_disk_task(disk_task_t* task){
  if (disk_task_queue == NULL){
    disk_task_queue = task;
    return;
  }
  
  disk_task_t* it = disk_task_queue;
  while(it->next == NULL){
    it = it->next;
  }

  it->next = task;
  task->prev = it;
}
