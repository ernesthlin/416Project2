// File:	my_pthread.c
// Author:	Yujie REN
// Date:	January 2019

// List all group member's name: Ernest Lin, Jake Zhou
// username of iLab: ehl32
// iLab Server:

#include "my_pthread_t.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
const tcb *current_running_thread = NULL; //@author: Ernest -
bool scheduler_started = false; //@author: Ernest -

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
	// Create Thread Control Block
	// Create and initialize the context of this thread
	// Allocate space of stack for this thread to run
	// After everything is all set, push this thread into run queue
	
	// YOUR CODE HERE
	tcb *tc_block = (tcb *) malloc(sizeof(tcb));
	init_tcb(tc_block);
	*thread = tc_block->threadID;
	getcontext(&tc_block->context);
	tc_block->context.uc_link = 0; //SHOULD LINK TO BEGINNING OF my_pthread_exit()
	tc_block->context.uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
	tc_block->context.uc_stack.ss_size = STACK_SIZE;
	tc_block->context.uc_stack.ss_flags = 0;
	makecontext(&tc_block->context, function, 1, arg);

	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	// Change thread state from Running to Ready
	// Save context of this thread to its thread control block
	// Switch from thread context to scheduler context

	// YOUR CODE HERE
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	// Deallocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	// Waiting for a specific thread to terminate
	// Once this thread finishes,
	// Deallocate any dynamic memory created when starting this thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	// Initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* acquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	// Use the built-in test-and-set atomic function to test the mutex
	// If mutex is acquired successfuly, enter critical section
	// If acquiring mutex fails, push current thread into block list 
	// and context switch to scheduler 

	// YOUR CODE HERE
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in my_pthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrupt happens, your thread library 
	// should be context switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (STCF or MLFQ)

	// if (sched == STCF)
	//		sched_stcf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose STCF
#else 
	// Choose MLFQ
#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

/* @author: Ernest
My Function Declarations 
start_timer: timer is the interval timer, time is the time (in milliseconds) until timer goes off (and SIGALRM is thrown).
	-NOTE that this timer is finished after the time is up, so invoke initTimer again on the same timer struct to reset the timer.
	-Reset the timer right after finishing the context switch in the SIGALRM handler.
stop_timer: Stops existing timer using timer struct.
init_tcb: Initialize the thread control block contents of target. Thread ID is determiend by currentID, the thread starts off as ready, and the rest are initialized to zero.
print_tcb: Print the contents of the thread control block, except for the context.
*/
void start_timer(struct itimerval *timer, int time)
{
	timer->it_value.tv_sec = time / 1000;
	timer->it_value.tv_usec = (time * 1000) % 1000000;
	timer->it_interval.tv_sec = 0;
	timer->it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, timer, NULL);
}

void stop_timer(struct itimerval *timer)
{
	timer->it_value.tv_sec = 0;
	timer->it_value.tv_usec = 0;
	timer->it_interval.tv_sec = 0;
	timer->it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, timer, NULL);
}

void init_tcb(tcb *target)
{
	target->threadID = currentID++;
	target->thread_state = READY;
	target->time_counter = 0;
	target->priority_level = 0;
}

void print_tcb(tcb *target)
{
	printf("{ Thread ID: %d, ", target->threadID);
	printf("Thread State: ");
	switch(target->thread_state)
	{
		case READY:
			printf("READY, ");
			break;
		case RUNNING:
			printf("RUNNING, ");
			break;
		case BLOCKED:
			printf("BLOCKED, ");
			break;
		case DONE:
			printf("DONE, ");
			break;
		default:
			printf("ERROR, ");
			break;
	}
	printf("Time Counter: %d, ", target->time_counter);
	printf("Priority Level: %d }\n", target->priority_level);
}
