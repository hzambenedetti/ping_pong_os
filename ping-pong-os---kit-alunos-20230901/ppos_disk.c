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
  int block;
  struct disk_task_t* prev;
  struct disk_task_t* next;
} disk_task_t;

//============================= GLOBALS =================================== // 

task_t* disk_suspended_queue;
disk_task_t* disk_task_queue;

semaphore_t disk_mgr_sem;
semaphore_t disk_sem;
semaphore_t disk_task_sem;

task_t disk_mgr_task;

disk_t* disk;

struct sigaction disk_sig;
int disk_sig_flag;

//============================= FUNCTION DEFINITIONS =================================== // 

void append_disk_task(disk_task_t* task);

void disk_append_ready_queue(task_t* task);

void task_suspend_disk(task_t* task);

task_t* pop_suspend_queue();

disk_task_t* pop_disk_queue();

//============================= FUNCTION IMPLEMENTATION =================================== // 

void disk_manager(void* args){

  while(1){
    //take exclusive control over disk
    sem_down(&disk_task_sem);
    
    //if a disk operation was completed
    if(disk_sig_flag){
      task_t* ready_task = pop_suspend_queue();

      //wake up task
      disk_append_ready_queue(ready_task);
      disk_sig_flag = 0;
    }
    
    int disk_idle = disk_cmd(DISK_CMD_STATUS, 0 ,0) == DISK_STATUS_IDLE;
    if(disk_idle && disk_task_queue != NULL){
      disk_task_t* next_task = disk_task_queue;

      //launch next disk task
      if(disk_cmd(next_task->op, next_task->block, next_task->buffer) >= 0){
        //if task was launched, remove head of queue
        free(pop_disk_queue());
      }
    }
    
    //release control over disk
    sem_up(&disk_task_sem);

    //suspend disk_manager until a task raises the semaphore
    sem_down(&disk_mgr_sem);
   
  }
}


int disk_mgr_init(int *numblocks, int *blockSize){

  //initiate disk manager
  //1- disk_task_queue
  //2- disk_semaphore
  //3- disk_manager_semaphore
  //4- init disk
  
  PPOS_PREEMPT_DISABLE;

  //disk_suspended_queue
  disk_suspended_queue = NULL;
  disk_task_queue = NULL;
  disk_sig_flag = 0;

  // create disk_semaphore
  if (sem_create(&disk_sem, 1) < 0){return -1;}

  //create disk_mgr_semaphore
  if (sem_create(&disk_mgr_sem, 0) < 0){return -1;}
  
  //create disk_task_sem
  if(sem_create(&disk_task_sem, 1) < 0){return -1;}
  
  //setup signal handler
  if(disk_sig_handler_setup() < 0){ return -1;} 
  
  //launch disk_manager task
  if(task_create(&disk_mgr_task, disk_manager, NULL) < 0){return -1;}
  //set disk_manager as a system task so it cannot be preempted
  disk_mgr_task.sys_task = 1;

  //init disk 
  if (disk_cmd(DISK_CMD_INIT, 0, 0) < 0){return -1;};

  //attribute values to numblocks and blockSize
  *numblocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
  *blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

  PPOS_PREEMPT_ENABLE;
  
  //return operation status
  return 0;
}

int disk_block_read(int block, void *buffer){
  PPOS_PREEMPT_DISABLE
  //create disk_task
  disk_task_t* d_task = (disk_task_t*) malloc(sizeof(disk_task_t));
  
  //set disk task values
  d_task->task = taskExec;
  d_task->buffer = buffer;
  d_task->op = DISK_CMD_READ;
  d_task->block = block;
  d_task->next = NULL;
  d_task->prev = NULL;

  //Appends disk read task to disk task queue 
  append_disk_task(d_task);
  
  //call disk_manager task
  sem_up(&disk_mgr_sem); 
  
  //suspends task until disk block is read
  task_suspend_disk(taskExec);

  PPOS_PREEMPT_ENABLE

  task_switch(taskDisp);

  //return status of operation
  return 0;
}

int disk_block_write(int block, void *buffer){
  PPOS_PREEMPT_DISABLE

  //create disk_task
  disk_task_t* d_task = (disk_task_t*) malloc(sizeof(disk_task_t));
  
  //set disk task values
  d_task->task = taskExec;
  d_task->buffer = buffer;
  d_task->op = DISK_CMD_WRITE;
  d_task->block = block;
  d_task->next = NULL;
  d_task->prev = NULL;

  //Appends disk read task to disk task queue 
  append_disk_task(d_task);
  
  //call disk_manager task
  sem_up(&disk_mgr_sem); 
  
  //suspends task until disk block is read
  task_suspend_disk(taskExec);

  PPOS_PREEMPT_ENABLE
  task_switch(taskDisp);
  
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
  sem_up(&disk_mgr_sem);
}

void append_disk_task(disk_task_t* task){
  if (disk_task_queue == NULL){
    disk_task_queue = task;
    return;
  }
  
  disk_task_t* it = disk_task_queue;
  while(it->next != NULL){
    it = it->next;
  }

  it->next = task;
  task->prev = it;
}

void task_suspend_disk(task_t* task){
  if(disk_suspended_queue == NULL){
    disk_suspended_queue = task;
    return;
  }
  
  task_t* it = disk_suspended_queue;
  while(it->next != NULL){
    it = it->next;
  }

  it->next = task;
  task->prev = it;
}

task_t* pop_suspend_queue(){
  if(disk_suspended_queue == NULL){
    return NULL;
  }

  task_t* popped = disk_suspended_queue;
  disk_suspended_queue = disk_suspended_queue->next;
  return popped;
}

disk_task_t* pop_disk_queue(){
  if(disk_task_queue == NULL){
    return NULL;
  }

  disk_task_t* popped = disk_task_queue;
  disk_task_queue = disk_task_queue->next;

  return popped;
}

void disk_append_ready_queue(task_t* task){
  if(readyQueue == NULL){
    readyQueue = task;
    readyQueue->next = readyQueue;
    readyQueue->prev = readyQueue;
    return;
  }

  task_t* last = readyQueue->prev;
  last->next = task;
  task->prev = last;
  readyQueue->prev = task;
  task->next = readyQueue;
}
