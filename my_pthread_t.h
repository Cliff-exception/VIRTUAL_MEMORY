// File:    my_pthread_t.h
// Author:  Yujie REN
// Date:    09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h> 
#include <signal.h>
#include <time.h>
#include <ucontext.h>
#include <string.h>
#include <sys/time.h>


#define pthread_create(x, y, z, w) my_pthread_create(x, y, z, w)
#define pthread_exit(x) my_pthread_exit(x)
#define pthread_yield() my_pthread_yield()
#define pthread_join(x, y) my_pthread_join(x, y)
#define pthread_mutex_init(x, y) my_pthread_mutex_init(x, y)
#define pthread_mutex_lock(x) my_pthread_mutex_lock(x)
#define pthread_mutex_unlock(x) my_pthread_mutex_unlock(x)
#define pthread_mutex_destroy(x) my_pthread_mutex_destroy(x)

#define STACK_SIZE 1024*64

// define the time quantum for the three levels in the multi-level priority queue
#define LEVEL1 25000
#define LEVEL2 50000
#define LEVEL3 100000

// this is me being generous
#define MAX_THREADS 100  

// define the number of ranks we have
#define RANKS 3 
#define MAX_PRIORITY 2 

// enumeration of states 
typedef enum { READY, WAITING, RUNNING, EXITED } state; 


typedef uint my_pthread_t;

// this helps us assign thread IDS and keep count of thread numbers  

typedef struct threadControlBlock {

    my_pthread_t tid; 
    my_pthread_t join; 
    unsigned int run_time; 
    unsigned int priority;  
    ucontext_t * thread_context; 
    state thread_state; 
    struct threadControlBlock * next; 

} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {

    unsigned int lock; 
    unsigned int destroyed; 
    my_pthread_t owner; 
    unsigned int initialized; 

} my_pthread_mutex_t;

/* define your data structures here: */

typedef struct PriorityQueue {      // priority queue

    tcb * head; 
    tcb * tail;

} queue; 

// definition of the multi_level priority queue that holds the threads
queue * Pqueue[3]; 

queue * WaitQueue; 

// create a list of all return values

typedef struct retVal {

    my_pthread_t id; 
    void * ret_val; 
    struct retVal *next;

}return_value;


typedef struct ret_list {

    return_value * head; 
    return_value * tail;

} ret_list;


 my_pthread_mutex_t * count_lock;

// a global list of all return values, when a thread exits, their return value gets added to this list 
// a thread that wishes to join has to iterate through this list and get the return value
ret_list * return_list; 

// this thread is meant for calling pthread_exit

ucontext_t exit_caller; 


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

// new functions
tcb * get_running_thread(); 
void priority_boost ();
void my_queue_init ();
int Insert_to_qeueue ( tcb * member );
// helps us grab the context of main and schedule it. main is not special, it is just like every other thread 
void init_main (); 

void schedule_threads(); // this function acts as the main scheduler, its job is to schedule threads, duh 
tcb * search_by_tid ( my_pthread_t tid ); 
void timer_interrupt () ; 
tcb * get_tcb ();


#endif
