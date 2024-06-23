#include "ppos.h"
#include "ppos-core-globals.h"


// ****************************************************************************
// Coloque aqui as suas modificações, p.ex. includes, defines variáveis, 
// estruturas e funções
#include<signal.h>
#include<sys/time.h>
#include<limits.h>

#define DEFAULT_TICKS 20
#define DEFAULT_EET 99999
#define MAX_PRIO -200
#define MIN_PRIO 200

///////////////////////////////// GLOBAL VARIABLES /////////////////////////////////

struct sigaction action;
struct itimerval timer;
int ticks = DEFAULT_TICKS;

///////////////////////////////// FUNCTION DEFINITIONS /////////////////////////////////


void tick_sys_clock();

void tick_preemp_timer();

void setup_signal();

void setup_timer();

int task_get_eet(task_t* task);

void task_set_eet(task_t* task, int time);

void task_set_prio(task_t* task, int prio);

int task_get_ret(task_t* task);

void task_set_sys_task(task_t* task);

task_t* strf_scheduler();

task_t* fcfs_scheduler();

task_t* priod_scheduler();

void preempt_task();

void append_ready_queue(task_t* task);

void age_tasks();

///////////////////////////////// FUNCTION IMPLEMENTATION /////////////////////////////////

void tick_sys_clock(){
  systemTime++;
  tick_preemp_timer();
}

void tick_preemp_timer(){
  ticks--;
  taskExec->running_time++;
  taskExec->ret--;
  if (ticks < 0 && PPOS_IS_PREEMPT_ACTIVE && !taskExec->sys_task){
    ticks = DEFAULT_TICKS;
    task_yield();
    // preempt_task();
  }
}

void setup_signal(){
  action.sa_handler = tick_sys_clock;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  if(sigaction(SIGALRM, &action, 0) < 0){
    perror("Erro");
    exit(1);
  }
}


void setup_timer(){
  timer.it_value.tv_usec = 1000; //1ms
  timer.it_value.tv_sec = 0;
  timer.it_interval.tv_usec = 1000; // 1ms
  timer.it_interval.tv_sec = 0;
  if(setitimer(ITIMER_REAL, &timer, 0) < 0){
    perror("Erro");
    exit(1);
  }

}

int task_get_eet(task_t* task){
  if (task != NULL){
    return task->eet;
  }
  return taskExec->eet;
}

void task_set_eet(task_t* task, int time){
  if(task != NULL){
    task->eet = time;
    task->ret = time;
  }
  else{
    taskExec->eet = time;
    taskExec->ret = time;
  }
}

void task_set_prio(task_t* task, int prio){
  if(task != NULL){
    task->static_prio = prio;
    task->dyn_prio = prio;
  }
  else{
    taskExec->static_prio = prio;
    taskExec->dyn_prio = prio;
  }
}

int task_get_ret(task_t* task){
  if(task != NULL){
    return task->ret;
  }
  return taskExec->ret;
}

void task_set_sys_task(task_t* task){
  if (task != NULL)
    task->sys_task = 1;
  else
   taskExec->sys_task = 1;
}

task_t* strf_scheduler(){
  //avoid accessing fields of a null pointer..
  if (readyQueue == NULL) return NULL;

  task_t* scheduled = readyQueue;
  task_t* queue_begin = scheduled;
  task_t* iter = readyQueue->next;


  int min = scheduled->ret;
  while(iter != queue_begin && iter != NULL){
    if(iter->ret < min){
      scheduled = iter;
      min = scheduled->ret;
    }
    iter = iter->next;
  }
  return scheduled;
}

task_t* fcfs_scheduler(){
  return readyQueue;
}

task_t* priod_scheduler(){
  if (readyQueue == NULL) return NULL;

  task_t* scheduled = readyQueue;
  task_t* queue_begin = scheduled;
  task_t* iter = readyQueue->next;


  int min = scheduled->dyn_prio;
  while(iter != queue_begin){
    if(iter->dyn_prio < min){
      scheduled = iter;
      min = scheduled->dyn_prio;
    }
    iter = iter->next;
  }
  return scheduled;
}

void preempt_task(){
  //1- get current running task 
  task_t* running_task = taskExec;

  //2- get dispatcher task
  task_t* dispatcher = taskDisp;

  //3- append current running task to end of queue 
  append_ready_queue(running_task); 
  
  //4- switch task to dispatcher
  task_switch(dispatcher);
}

void append_ready_queue(task_t* task){
  task_t* last = readyQueue->prev;
  last->next = task;
  task->prev = last;
  readyQueue->prev = task;
  task->next = readyQueue;

}

void age_tasks(){
  if(readyQueue != NULL){
    task_t* iter = readyQueue->next;
    task_t* begin = readyQueue;

    begin->dyn_prio--;
    while(iter != begin){
      iter->dyn_prio--;
      iter = iter->next;
    }
  }
}

// ****************************************************************************



void before_ppos_init () {
    // put your customization here
  setup_signal();
  setup_timer();
#ifdef DEBUG
    printf("\ninit - BEFORE");
#endif
}

void after_ppos_init () {
    // put your customization here
    taskExec->eet = INT_MAX - 1;
    taskExec->ret = INT_MAX - 1;
#ifdef DEBUG
    printf("\ninit - AFTER");
#endif
}

void before_task_create (task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_create - BEFORE - [%d]", task->id);
#endif
}

void after_task_create (task_t *task ) {
    // put your customization here
    task->running_time = 0;
    task->eet = DEFAULT_EET;
    task->ret = DEFAULT_EET;
    task->sys_task = 0;
    task->activations = 0;
    task->static_prio = 0;
    task->dyn_prio = 0;
    task->launch_timestamp = systime();
#ifdef DEBUG
    printf("\ntask_create - AFTER - [%d]", task->id);
#endif
}

void before_task_exit () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_exit - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_exit () {
    // put your customization here
    printf("\n\nTask[%d] -- Execution time: %d ms || CPU time: %d ms || activations: %d\n\n", 
           taskExec->id,systime() - taskExec->launch_timestamp, 
           taskExec->running_time,
           taskExec->activations);
#ifdef DEBUG
    printf("\ntask_exit - AFTER- [%d]", taskExec->id);
#endif
}

void before_task_switch ( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_switch - BEFORE - [%d -> %d]", taskExec->id, task->id);
#endif
}

void after_task_switch ( task_t *task ) {
    // put your customization here
    ticks = DEFAULT_TICKS;
    task->activations++;
    task->dyn_prio = task->static_prio;
#ifdef DEBUG
    printf("\ntask_switch - AFTER - [%d -> %d]", taskExec->id, task->id);
#endif
}

void before_task_yield () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - BEFORE - [%d]", taskExec->id);
#endif
}
void after_task_yield () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_yield - AFTER - [%d]", taskExec->id);
#endif
}


void before_task_suspend( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - BEFORE - [%d]", task->id);
#endif
}

void after_task_suspend( task_t *task ) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_suspend - AFTER - [%d]", task->id);
#endif
}

void before_task_resume(task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - BEFORE - [%d]", task->id);
#endif
}

void after_task_resume(task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_resume - AFTER - [%d]", task->id);
#endif
}

void before_task_sleep () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - BEFORE - [%d]", taskExec->id);
#endif
}

void after_task_sleep () {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_sleep - AFTER - [%d]", taskExec->id);
#endif
}

int before_task_join (task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_task_join (task_t *task) {
    // put your customization here
#ifdef DEBUG
    printf("\ntask_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}


int before_sem_create (semaphore_t *s, int value) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_create (semaphore_t *s, int value) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_down (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_down (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_down - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_up (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_up (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_up - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_sem_destroy (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_sem_destroy (semaphore_t *s) {
    // put your customization here
#ifdef DEBUG
    printf("\nsem_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_create (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_create (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_lock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_lock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_lock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_unlock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_unlock (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_unlock - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mutex_destroy (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mutex_destroy (mutex_t *m) {
    // put your customization here
#ifdef DEBUG
    printf("\nmutex_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_create (barrier_t *b, int N) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_create (barrier_t *b, int N) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_join (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_join (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_join - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_barrier_destroy (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_barrier_destroy (barrier_t *b) {
    // put your customization here
#ifdef DEBUG
    printf("\nbarrier_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_create (mqueue_t *queue, int max, int size) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_create (mqueue_t *queue, int max, int size) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_create - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_send (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_send (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_send - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_recv (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_recv (mqueue_t *queue, void *msg) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_recv - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_destroy (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_destroy (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_destroy - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

int before_mqueue_msgs (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - BEFORE - [%d]", taskExec->id);
#endif
    return 0;
}

int after_mqueue_msgs (mqueue_t *queue) {
    // put your customization here
#ifdef DEBUG
    printf("\nmqueue_msgs - AFTER - [%d]", taskExec->id);
#endif
    return 0;
}

task_t * scheduler() {
  return strf_scheduler();
}


