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
#define HASH_SIZE 1000 /* @author: Jake - size of hash table array */

/*
#ifdef _LP64
	#define STACK_SIZE 2097152 + 16384
#else
	#define STACK_SIZE 16384
#endif
*/

#define STACK_SIZE 64000

#define INTERVAL 30 //MILLISECONDS
#define MLFQ_LEVELS 4 // Comment: Jake - technically 5 since 0 is also a level. That's how it's implemented in the mlfq struct
/* @author: Ernest */

/* @author: Ernest
Initialization of Custom Variable Types and Variables
bool: Just a variable type that can be used as a regular boolean.
state: The state type is the state of the thread, whether it's ready/waiting to be scheduled, currently running, blocked by some call, 
or done executing.
*/
typedef enum {false, true} bool;
typedef enum {READY, RUNNING, BLOCKED, DONE} state;
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
	ucontext_t *context; //@author: Ernest - The thread's context, which also will contain the stack.
	int time_counter; //@author: Ernest - The number of time quantum the thread has run.
	int priority_level; //@author: Ernest - The priority level of the thread (for MLFQ).
	my_pthread_t *joined_on; //@author: Ernest - The thread ID of the thread this thread is waiting for/joined on.
	bool called_exit; //@author: Ernest - Initially, false, only true if thread explicitly calls pthread_exit().
	void *returned_value; //@author: Ernest - The returned value of the thread when DONE.
} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
	
	// YOUR CODE HERE
	int mutex_id; //@author: Jake - this mutex's unique id
	int in_use_flag; //@author: Jake - Tracks if mutex is currently in use. Will be changed to 1 on creation by a thread, changed to 0 on release
	my_pthread_t owner; //@author: Jake - the thread that created this mutex
} my_pthread_mutex_t;


/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE

//##############################################################################################################
/* @author: Jake - Data Structures we will be using */
//Hash Table Structs
/* @author: Jake - The Hash Table will store all of our TCBs in hash_nodes. Our scheduler data structures will be pulling for this structure when given a thread_id*/
typedef struct hash_table {
	int size; //@author: Jake - how many threads are in this hash table
	struct hash_node * ht[HASH_SIZE]; //@author: Jake - creates an array of hash_node pointers with Hash_Size amount of consecutive space
} hash_table;

typedef struct hash_node {
	tcb * thread; //@author: Jake - the threadControlBlock that this node holds
	struct hash_node * next; //@author: Jake - the next node in the chain for this linked list
} hash_node;

//##############################################################################################################
//Priority Linked List (STCF) Structs
/* @author: Jake - Linked list that keeps threads in order of time_counter. If a thread that is being added has the same time_counter as an existing thread in the list, the new thread is added before it*/
typedef struct list {
	struct list_node * head; // @author: Jake - head of list. The next thread to run
	int size; // @author: Jake - tracks how many threads are in the list
	
} list;

typedef struct list_node {
	tcb * thread;
	struct list_node * next;
} list_node;

//##############################################################################################################
//Queue Linked List (MLFQ) Structs
/* @author: Jake - basically an array of queues. mlfq_scheduler[0] will be the highest priority queue at priority_level == 0 while mlfq_scheduler[MLFQ_LEVELS] will be the lowest priority queue */
typedef struct mlfq {
	int size;
	struct queue * mlfq_scheduler[(MLFQ_LEVELS + 1)]; //@author: Jake - an array of queues each with a different priority. Threads will be moved from one queue to another when they pass a certain amount of time quantums. The +1 is because we count mlfq_scheduler[0] as a 
} mlfq;

typedef struct queue {
	int size; //@author: Jake - how many threads are in this queue linked list
	struct queue_node * head; //@author: Jake - the head of the queue where we will be getting from
	struct queue_node * tail; //@author: Jake - the end of the queue where we will be adding to
} queue;

typedef struct queue_node {
	tcb * thread;
	struct queue_node * next;
} queue_node;

//##############################################################################################################
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

//##############################################################################################################
/* @author: Ernest
My Function Declarations (descriptions in my_pthread.c file)

test_and_set(): This is an atomic operation to use in mutex lock/unlock functions.
*/
void pthread_create_helper(tcb *, void *(*)(void *), void *);
void start_timer(struct itimerval *, int);
void stop_timer(struct itimerval *);

/*
bool test_and_set()
{
	bool old_value = mutex_locked;
	mutex_locked = true;
	return old_value;
}
*/

void init_tcb(tcb *);
void print_tcb(tcb *);
void enable_handler(struct sigaction *);
void disable_handler(struct sigaction *);
/* @author: Ernest */

//##############################################################################################################
/* @author: Jake - descriptions in file 
*/
//Hash Table Prototypes
void create_hash_table(hash_table * ht);
int hashcode(my_pthread_t threadID);
tcb * get_hash_thread(hash_table * ht, my_pthread_t thread_id);
void add_hash_thread(hash_table * ht, tcb * thread);
void free_hash_nodes(hash_table * ht);

//Priority Linked List (STCF) Prototypes
void create_list(list * sched_list);
void add_list_thread(list * sched_list, hash_table * ht, my_pthread_t thread_id);
tcb * get_list_thread(list * sched_list);
const tcb * peek_list_thread(list * sched_list);
bool listIsEmpty(list * sched_list);
void free_list_nodes(list* sched_list);

//Queue Linked List (MLFQ) Prototypes
void create_mlfq(mlfq * mlfq_table);
void add_to_mlfq(mlfq * mlfq_table, hash_table * ht, my_pthread_t thread_id);
tcb * get_from_mlfq(mlfq * mlfq_table, int priority);
void free_mlfq_queues(mlfq * mlfq_table);
void create_queue(queue * queue_list);
void enqueue_thread(queue * queue_list, hash_table * ht, my_pthread_t thread_id);
tcb * dequeue_thread(queue * queue_list);
void free_queue_nodes(queue * queue_list);

/* @author: Jake */

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

