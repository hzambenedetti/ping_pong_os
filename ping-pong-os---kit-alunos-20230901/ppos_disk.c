#include "ppos_disk.h"

int disk_mgr_init(int *numblocks, int *blockSize){
  //initiate disk manager
  // which consists of ???
  
  //attribute values to numblocks and blockSize
  *numblocks = 0;
  *blockSize = 0;
  
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
