// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	January 2019

// List all group member's name: Ernest Lin, Jake Zhou
// username of iLab: ehl32
// iLab Server:

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* To use real pthread Library in Benchmark, you have to comment the USE_MY_PTHREAD macro */
// #define USE_MY_PTHREAD 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

typedef uint my_pthread_t;

/* @author: Ernest
STACK_SIZE: Number of bytes to allocate for a context's stack for invoking makecontext().
INTERVAL: Number of milliseconds the timer goes off for the scheduler to intervene.
MLFQ_LEVELS: Number of queues/priority levels for MLFQ scheduler.
*/
#ifdef _LP64
	#define STACK_SIZE 2097152 + 16384
#else
	#define STACK_SIZE 16384
#endif

#define INTERVAL 15 //MILLISECONDS
#define MLFQ_LEVELS 4 
/* @author: Ernest */

/* @author: Ernest
Initialization of Custom Variable Types and Variables
bool: Just a variable typevthat can be used as a regular boolean.
state: The state type is the state of the thread, whether it's ready/waiting to be scheduled, currently running, blocked by some call, 
or done executing.
currentID: This is our mechanism for giving out threadIDs; for every new thread, assign its threadID to currentID, and increment
currentID by 1.
*/
typedef enum {false, true} bool;
typedef enum {READY, RUNNING, BLOCKED, DONE} state;
my_pthread_t currentID = 0;
bool mutex_locked = false;
/* @author: Ernest */

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	my_pthread_t threadID; //@author: Ernest - The thread's ID.
	state thread_state; //@author: Ernest - The state of the thread.
	ucontext_t context; //@author: Ernest - The thread's context, which also will contain the stack.
	int time_counter; //@author: Ernest - The number of time quantum the thread has run.
	int priority_level; //@author: Ernest - The priority level of the thread (for MLFQ).
	my_pthread_t *joined_on; //@author: Ernest - The thread ID of the thread this thread is waiting for/joined on.

} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */

	// YOUR CODE HERE
} my_pthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE

/* Function Declarations: */

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

/* @author: Ernest
My Function Declarations (descriptions in my_pthread.c file)

test_and_set(): This is an atomic operation to use in mutex lock/unlock functions.
*/
void start_timer(struct itimerval *, int);
void stop_timer(struct itimerval *);
bool test_and_set()
{
	bool old_value = mutex_locked;
	mutex_locked = true;
	return old_value;
}

void init_tcb(tcb *);
void print_tcb(tcb *);
/* @author: Ernest */

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif
