// File:	my_pthread.c
// Author:	Yujie REN
// Date:	January 2019

// List all group member's name: Ernest Lin, Jake Zhou
// username of iLab: ehl32, xz346
// iLab Server:

#include "my_pthread_t.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE
/* @author: Ernest (GENERAL NOTES)
*/

ucontext_t *scheduler = NULL;
const my_pthread_t *current_running_thread = NULL; //@author: Ernest - Points to current running threadID, starts off as NULL.
int mutex_id = 0; //@author: Jake - very similar to thread id. The first created mutex has id 0, incremented and assigned for every new mutex
my_pthread_t currentID = 0; // @author: Ernest - currentID is our mechanism for giving out threadIDs; for every new thread, 
// assign its threadID to currentID, and increment currentID by 1.

hash_table *tcb_hash_table = NULL;

list *stcf_ready_list = NULL;
list *stcf_blocked_list = NULL;

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
	tc_block->context = (ucontext_t *) malloc(sizeof(ucontext_t));
	getcontext(tc_block->context);
	tc_block->context->uc_link = 0; 
	tc_block->context->uc_stack.ss_sp = (char *) malloc(STACK_SIZE);
	tc_block->context->uc_stack.ss_size = STACK_SIZE;
	tc_block->context->uc_stack.ss_flags = 0;
	makecontext(tc_block->context, pthread_create_helper, 3, tc_block, function, arg);

	// Initialize the globals: TCB table and (priority) queues.
	if(tcb_hash_table == NULL)
	{
		tcb_hash_table = (hash_table *) malloc(sizeof(hash_table));
		create_hash_table(tcb_hash_table);
	}
	add_hash_thread(tcb_hash_table, tc_block);

	if(stcf_ready_list == NULL)
	{
		stcf_ready_list = (list *) malloc(sizeof(list));
		create_list(stcf_ready_list);
	}
	add_list_thread(stcf_ready_list, tcb_hash_table, tc_block->threadID);

	if(stcf_blocked_list = NULL)
	{
		stcf_blocked_list = (list *) malloc(sizeof(list));
		create_list(stcf_blocked_list);
	}

	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	// Change thread state from Running to Ready
	// Save context of this thread to its thread control block
	// Switch from thread context to scheduler context

	// YOUR CODE HERE
	tcb *yielding_thread = get_hash_thread(tcb_hash_table, *current_running_thread);
	yielding_thread->thread_state = READY;
	swapcontext(yielding_thread->context, scheduler);

	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	// Deallocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
	tcb *exiting_thread = get_hash_thread(tcb_hash_table, *current_running_thread);
	exiting_thread->called_exit = false;
	exiting_thread->thread_state = DONE;
	if(value_ptr != NULL)
	{
		exiting_thread->returned_value = (void *) malloc(sizeof(void *));
		exiting_thread->returned_value = value_ptr;
	}
	free(exiting_thread->context->uc_stack.ss_sp);
	free(exiting_thread->context);

	// remove it from priority list

	// call scheduler ????
}


/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	// Waiting for a specific thread to terminate
	// Once this thread finishes,
	// Deallocate any dynamic memory created when starting this thread
  
	// YOUR CODE HERE
	if(scheduler == NULL)
	{
		scheduler = (ucontext_t *) malloc(sizeof(ucontext_t));
		//schedule();
	}

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

//##############################################################################################################
//Hash Table Functions
/* @author: Jake - Initializes the hash table that we will be using. Must be given a pointer to where that hash table will be. Hash table is stored on stack since it is fixed since. Only the nodes will be dynamically allocated*/
void create_hash_table(hash_table * ht) {
	ht->size = 0;
	int i = 0;
	for(i = 0; i < HASH_SIZE; i++) {
		ht->ht[i] = NULL;
	}
}

/* @author: Jake - returns the hash code of the given thread_id */
int hashcode(my_pthread_t threadID) {
	return (threadID % HASH_SIZE);
}

/* @author: Jake - adds the threadControlBlock into the hash table */
void add_hash_thread(hash_table * ht, tcb * thread) {
	hash_node * hn = (hash_node *) malloc(sizeof(hash_node));
	hn->thread = thread;
	hn->next = NULL;
	int hashCode = hashcode(thread->threadID);
	if(ht->ht[hashCode] == NULL) {
		ht->ht[hashCode] = hn;
	}
	else {
		hn->next = ht->ht[hashCode];
		ht->ht[hashCode] = hn;
	}
	ht->size++;
}

/* @author: Jake - finds the hash_node that contains the thread we are looking for */
tcb * get_hash_thread(hash_table * ht, my_pthread_t thread_id) {
	tcb * result = NULL;
	hash_node * temp = NULL;
	int hashCode = hashcode(thread_id);
	if(ht->ht[hashCode] == NULL) {
		printf("Could not find thread with given threadID, returning NULL\n");
		return NULL;
	}
	else {
		temp = ht->ht[hashCode];
		while(temp != NULL && temp->thread->threadID != thread_id) {
			temp = temp->next;
		}
		if(temp == NULL) {
			printf("Could not find thread with given threadID, returning NULL\n");
			return NULL;
		}
		if(temp->thread->threadID == thread_id) {
			result = temp->thread;
		}
		else {
			printf("Something went wrong with getting from hash table\n");
			exit(1);
		}
	}
	return result;
}

/* @author: Jake - frees the dynamically allocated nodes of the hash table */
void free_hash_nodes(hash_table * ht) {
	hash_node * temp;
	int i;
	for(i = 0; i < HASH_SIZE; i++) {
		while(ht->ht[i] != NULL) {
			temp = ht->ht[i];
			ht->size--;
			ht->ht[i] = ht->ht[i]->next;
			free(temp);
		}
	}
}

//##############################################################################################################
//Priority Linked List (STCF) Functions
/* @author: Jake - creates the priority list */
void create_list(list * sched_list) {
	sched_list->head = NULL;
	sched_list->size = 0;
}

/* @author: Jake - creates a list_node for the given thread_id. Needs the hash table to get the info about the thread */
void add_list_thread(list * sched_list, hash_table * ht, my_pthread_t thread_id) {
	list_node * ln = (list_node *) malloc(sizeof(list_node));
	ln->thread = (get_hash_thread(ht, thread_id));
	ln->next = NULL;
	if(listIsEmpty(sched_list) || sched_list->head->thread->time_counter >= ln->thread->time_counter) {
		ln->next = sched_list->head;
		sched_list->head = ln;
	} else {
		list_node * cur = sched_list->head;
		list_node * prev = NULL;
		while(cur != NULL && cur->thread->time_counter < ln->thread->time_counter) {
			prev = cur;
			cur = cur->next;
		}
		ln->next = cur;
		prev->next = ln;
	}
	
	sched_list->size++;
}

/* @author: Jake - grabs the TCB at the beginning of the priority list. Removes it from the list */
tcb * get_list_thread(list * sched_list) {
	if(listIsEmpty(sched_list)) {
		printf("STCF List is empty, there is nothing to get\n");
		return NULL;
	}
	tcb * result = sched_list->head->thread;
	list_node * temp = sched_list->head;//Since we are disconnecting this node from the list, we have to free it. But we need to replace the head before we do that
	sched_list->head = sched_list->head->next;
	free(temp);
	
	
	sched_list->size--;
	return result;
}

/* @author: Jake - quick check to see if the priority list is empty */
bool listIsEmpty(list * sched_list) {
	return (sched_list->size == 0);
}

/* Frees the malloced list nodes that were created when adding to the priority queue*/
void free_list_nodes(list* sched_list) {
	list_node * temp;
	while(sched_list->head != NULL) {
		temp = sched_list->head;
		sched_list->size--;
		sched_list->head = sched_list->head->next;
		free(temp);
	}
}

//##############################################################################################################
//Queue Linked List (MLFQ) Functions
/* @author: Jake - creates a queue for the MLFQ. Initializes all the queues so they may be used as needed */
void create_mlfq(mlfq * mlfq_table) {
	mlfq_table->size = 0;
	int i;
	for(i = 0; i < MLFQ_LEVELS + 1; i++) {
		queue * temp = (queue *) malloc(sizeof(queue));
		create_queue(temp);
		mlfq_table->mlfq_scheduler[i] = temp;
	}
}

/* @author: Jake - adds the thread noted by the thread_id into the appropriate q */
void add_to_mlfq(mlfq * mlfq_table, hash_table * ht, my_pthread_t thread_id) {
	tcb * temp = get_hash_thread(ht, thread_id);
	if(temp->priority_level > MLFQ_LEVELS) {
		printf("Error, thread priority levels should not exceed MLFQ_LEVELS. Adjusting priority_level to be equal to MLFQ_LEVELS\n");
		temp->priority_level = MLFQ_LEVELS;
	}
	queue * priority_queue = mlfq_table->mlfq_scheduler[temp->priority_level]; //grabs the appropriate priority queue according to the thread's priority level
	//printf("TESTING - IN add_to_mlfq - before enqueue: threadid: %d\n", thread_id);
	enqueue_thread(priority_queue, ht, thread_id);
	//printf("TESTING - IN add_to_mlfq - after enqueue: threadid: %d\n", thread_id);
	mlfq_table->size++;
}

/* @author: Jake - retrieves the tcb at the indicated priority queue */
tcb * get_from_mlfq(mlfq * mlfq_table, int priority) {
	if(priority < 0 || priority > MLFQ_LEVELS) {
		printf("Error, only call get_from_mlfq with an appropriate priority level. Returning NULL\n");
		return NULL;
	}
	queue * priority_queue = mlfq_table->mlfq_scheduler[priority];
	mlfq_table->size--;
	return dequeue_thread(priority_queue);
}

/* @author: Jake - frees all the queues in the MLFQ */
void free_mlfq_queues(mlfq * mlfq_table) {
	queue * temp = NULL;
	int i = 0;
	for(i = 0; i < MLFQ_LEVELS + 1; i++) {
		temp = mlfq_table->mlfq_scheduler[i];
		free_queue_nodes(temp);
		free(temp);
	}
}

void create_queue(queue * queue_list) {
	queue_list->size = 0;
	queue_list->head = NULL;
	queue_list->tail = NULL;
}

/* @author: Jake - creates a queue_node for the given thread_id. */
void enqueue_thread(queue * queue_list, hash_table * ht, my_pthread_t thread_id) {
	queue_node * qn = (queue_node *) malloc(sizeof(queue_node));
	qn->thread = get_hash_thread(ht, thread_id);
	qn->next = NULL;
	//printf("TESTING - IN enqueue_thread: threadid: %d\n", thread_id);
	
	//If the queue is empty, this node is both the head and the tail
	if(queue_list->tail == NULL) {
		queue_list->head = qn;
		queue_list->tail = qn;
		//printf("TESTING - IN enqueue_thread - if condition: threadid: %d\n", thread_id);
	} else {
		//printf("TESTING - IN enqueue_thread - before else condition: threadid: %d\n", thread_id);
		queue_list->tail->next = qn;
		queue_list->tail = qn;
		//printf("TESTING - IN enqueue_thread - after else condition: threadid: %d\n", thread_id);
	}
	queue_list->size++;
}
/* @author: Jake - grabs the TCB at the beginning of the queue. Removes it from the list */
tcb * dequeue_thread(queue * queue_list) {
	if(queue_list->size == 0) {
		printf("MLFQ Queue is empty, there is nothing to dequeue\n");
		return NULL;
	}
	tcb * result = queue_list->head->thread;
	queue_node * temp = queue_list->head;
	queue_list->head = queue_list->head->next;
	free(temp);
	
	queue_list->size--;
	return result;
}

/* @author: Jake - frees the remaining queue nodes in the queue */
void free_queue_nodes(queue * queue_list) {
	queue_node * temp;
	while(queue_list->head != NULL) {
		temp = queue_list->head;
		queue_list->size--;
		queue_list->head = queue_list->head->next;
		free(temp);
	}
}
/* @author: Jake */

/* @author: Ernest
My Function Declarations 
start_timer: timer is the interval timer, time is the time (in milliseconds) until timer goes off (and SIGALRM is thrown).
	-NOTE that this timer is finished after the time is up, so invoke initTimer again on the same timer struct to reset the timer.
	-Reset the timer right after finishing the context switch in the SIGALRM handler.
stop_timer: Stops existing timer using timer struct.
init_tcb: Initialize the thread control block contents of target. Thread ID is determiend by currentID, the thread starts off as ready, and the rest are initialized to zero.
print_tcb: Print the contents of the thread control block, except for the context.
*/
void pthread_create_helper(tcb *tc_block, void *(*function)(void *), void *arg)
{
	void *return_value = function(arg);
	if(tc_block->called_exit == false)
	{
		my_pthread_exit(return_value);
	}
	return;
}

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
	target->joined_on = NULL;
	target->called_exit = false;
	target->returned_value = NULL;
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
	printf("Priority Level: %d, ", target->priority_level);
	printf("Joined on: ");
	if(target->joined_on != NULL) printf("%ld, ", *target->joined_on);
	else printf("NULL, ");
	printf("Called exit: ");
	if(target->called_exit == true) printf("True");
	else printf("False");
	printf(" }\n");
}

//##############################################################################################################
// Signal enable/disable

void enable_handler(struct sigaction *sa)
{
	// sa->sa_handler = WHAT
	sigaction(SIGALRM, sa, NULL);
}

void disable_handler(struct sigaction *sa)
{
	sa->sa_handler = SIG_DFL;
	sigaction(SIGALRM, sa, NULL);
}