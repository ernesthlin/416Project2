/* Header Section */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#define HASH_SIZE 1000
#ifdef _LP64
	#define STACK_SIZE 2097152 + 16384
#else
	#define STACK_SIZE 16384
#endif
#define INTERVAL 30 //MILLISECONDS
#define MLFQ_LEVELS 4 

typedef uint my_pthread_t;
typedef enum {false, true} bool;
typedef enum {READY, RUNNING, BLOCKED, DONE} state;

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
	int priority_level; //@author: Ernest - The priority level of the thread (for MLFQ). Starts at 0, ends before MLFQ_LEVELS
	my_pthread_t *joined_on; //@author: Ernest - The thread ID of the thread this thread is waiting for/joined on.
	bool called_exit; //@author: Ernest - Initially, false, only true if thread explicitly calls pthread_exit().
} tcb; 

/* @author: Jake - Data Structures we will be using */
//Hash Table Structs
typedef struct hash_table {
	int size; //@author: Jake - how many threads are in this hash table
	struct hash_node * ht[HASH_SIZE]; //@author: Jake - creates an array of hash_node pointers with Hash_Size amount of consecutive space
} hash_table;

typedef struct hash_node {
	tcb * thread; //@author: Jake - the threadControlBlock that this node holds
	struct hash_node * next; //@author: Jake - the next node in the chain for this linked list
} hash_node;


//Priority Linked List (STCF) Structs
typedef struct list {
	struct list_node * head; // @author: Jake - head of list. The next thread to run
	int size; // @author: Jake - tracks how many threads are in the list
	
} list;

typedef struct list_node {
	tcb * thread;
	struct list_node * next;
} list_node;


//Queue Linked List (MLFQ) Structs
typedef struct mlfq {
	int size;
	struct queue * mlfq_scheduler[MLFQ_LEVELS]; //@author: Jake - an array of queues each with a different priority. Threads will be moved from one queue to another when they pass a certain amount of time quantums.
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



/* File section */
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


//Queue Linked List (MLFQ) Functions
/* @author: Jake - creates a queue for the MLFQ. Initializes all the queues so they may be used as needed */
void create_mlfq(mlfq * mlfq_table) {
	mlfq_table->size = 0;
	int i;
	for(i = 0; i < MLFQ_LEVELS; i++) {
		queue * temp = (queue *) malloc(sizeof(queue));
		create_queue(temp);
		mlfq_table->mlfq_scheduler[i] = temp;
	}
}

/* @author: Jake - adds the thread noted by the thread_id into the appropriate q */
void add_to_mlfq(mlfq * mlfq_table, hash_table * ht, my_pthread_t thread_id) {
	tcb * temp = get_hash_thread(ht, thread_id);
	if(temp->priority_level >= MLFQ_LEVELS) {
		printf("Error, thread priority levels should not exceed MLFQ_LEVELS. Adjusting priority_level to be equal to MLFQ_LEVELS\n");
		temp->priority_level = MLFQ_LEVELS - 1;
	}
	queue * priority_queue = mlfq_table->mlfq_scheduler[temp->priority_level]; //grabs the appropriate priority queue according to the thread's priority level
	//printf("TESTING - IN add_to_mlfq - before enqueue: threadid: %d\n", thread_id);
	enqueue_thread(priority_queue, ht, thread_id);
	//printf("TESTING - IN add_to_mlfq - after enqueue: threadid: %d\n", thread_id);
	mlfq_table->size++;
}

/* @author: Jake - retrieves the tcb at the indicated priority queue */
tcb * get_from_mlfq(mlfq * mlfq_table, int priority) {
	if(priority < 0 || priority >= MLFQ_LEVELS) {
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
	for(i = 0; i < MLFQ_LEVELS; i++) {
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

int main(int argc, char ** argv) {

	//We will be testing our code in here
	//Create 5 test tcb structs
	tcb * thread0 = (tcb *) malloc(sizeof(tcb));
	thread0->threadID = 0;
	thread0->time_counter = 1;
	thread0->priority_level = 0;
	printf("thread0, threadID: %d, time_counter: %d, priority_level: %d\n", thread0->threadID, thread0->time_counter, thread0->priority_level);
	
	tcb * thread1 = (tcb *) malloc(sizeof(tcb));
	thread1->threadID = 1;
	thread1->time_counter = 2;
	thread1->priority_level = 1;
	printf("thread1, threadID: %d, time_counter: %d, priority_level: %d\n", thread1->threadID, thread1->time_counter, thread1->priority_level);
	
	tcb * thread2 = (tcb *) malloc(sizeof(tcb));
	thread2->threadID = 2;
	thread2->time_counter = 3;
	thread2->priority_level = 2;
	printf("thread2, threadID: %d, time_counter: %d, priority_level: %d\n", thread2->threadID, thread2->time_counter, thread2->priority_level);
	
	tcb * thread3 = (tcb *) malloc(sizeof(tcb));
	thread3->threadID = 3;
	thread3->time_counter = 4;
	thread3->priority_level = 3;
	printf("thread3, threadID: %d, time_counter: %d, priority_level: %d\n", thread3->threadID, thread3->time_counter, thread3->priority_level);
	
	tcb * thread4 = (tcb *) malloc(sizeof(tcb));
	thread4->threadID = 4;
	thread4->time_counter = 4;
	thread4->priority_level = 3;
	printf("thread4, threadID: %d, time_counter: %d, priority_level: %d\n", thread4->threadID, thread4->time_counter, thread4->priority_level);
	printf("\n");
	
	//Create hash table
	hash_table * my_hash_table = (hash_table *) malloc(sizeof(hash_table));
	create_hash_table(my_hash_table);
	//Test all has functions
	add_hash_thread(my_hash_table, thread0);
	bool empty = (my_hash_table->ht[0] == NULL);
	printf("Is hash table[0] empty: %d\n", empty);
	printf("Hash table size: %d\n", my_hash_table->size); //Should be 1
	printf("ThreadID at hash table[0]: %d\n", get_hash_thread(my_hash_table, 0)->threadID);
	add_hash_thread(my_hash_table, thread1);
	printf("ThreadID at hash table[1]: %d\n", get_hash_thread(my_hash_table, 1)->threadID);
	add_hash_thread(my_hash_table, thread2);
	printf("ThreadID at hash table[2]: %d\n", get_hash_thread(my_hash_table, 2)->threadID);
	add_hash_thread(my_hash_table, thread3);
	printf("ThreadID at hash table[3]: %d\n", get_hash_thread(my_hash_table, 3)->threadID);
	add_hash_thread(my_hash_table, thread4);
	printf("ThreadID at hash table[4]: %d\n", get_hash_thread(my_hash_table, 4)->threadID);
	printf("Hash table size: %d\n", my_hash_table->size); //Should be 5
	printf("\n");
	
	
	//Create priority list
	list * my_list = (list *) malloc(sizeof(list));
	create_list(my_list);
	//Test all priority list functions
	printf("Is list empty: %d\n", listIsEmpty(my_list)); //Should be 1
	add_list_thread(my_list, my_hash_table, 0);
	add_list_thread(my_list, my_hash_table, 1);
	add_list_thread(my_list, my_hash_table, 2);
	add_list_thread(my_list, my_hash_table, 3);
	add_list_thread(my_list, my_hash_table, 4);
	printf("Is list empty: %d\n", listIsEmpty(my_list)); //Should be 0
	int i;
	tcb * temp;
	for(i = 0; i < 4; i++) {
		temp = get_list_thread(my_list);
		printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
		//SKIPS 3 BECAUSE 4 and 3 HAVE THE SAME TIME COUNTER. 4 GETS STORES BEFORE 3 IN THE LIST AS IT SHOULD
	}
	printf("Size of list: %d\n", my_list->size); //Should be 1
	printf("\n");
	
	
	//Create mlfq struct
	mlfq * my_mlfq = (mlfq *) malloc(sizeof(mlfq));
	create_mlfq(my_mlfq);
	//Test all priority list functions
	printf("Size of mlfq: %d\n", my_mlfq->size); //Should be 0
	add_to_mlfq(my_mlfq, my_hash_table, 0);
	add_to_mlfq(my_mlfq, my_hash_table, 1);
	add_to_mlfq(my_mlfq, my_hash_table, 2);
	add_to_mlfq(my_mlfq, my_hash_table, 3);
	add_to_mlfq(my_mlfq, my_hash_table, 4);
	printf("Size of mlfq: %d\n", my_mlfq->size); //Should be 5
	temp = get_from_mlfq(my_mlfq, 0);
	printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
	temp = get_from_mlfq(my_mlfq, 1);
	printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
	temp = get_from_mlfq(my_mlfq, 2);
	printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
	temp = get_from_mlfq(my_mlfq, 3);
	printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
	temp = get_from_mlfq(my_mlfq, 4);	//Should be an error
	
	//Since the above returns NULL, the following line causes a seg fault, this is a good reminder to always check to see if the returned thread is NULL or not before executing anything else
	printf("temp thread, threadID: %d, time_counter: %d, priority_level: %d\n", temp->threadID, temp->time_counter, temp->priority_level);
	printf("Size of mlfq: %d\n", my_mlfq->size); //Should be 0
	printf("\n");
	
	
	//Free all memory
	free_hash_nodes(my_hash_table);
	printf("Hash table size: %d\n", my_hash_table->size); //Should be 0
	free(my_hash_table);
	
	free_list_nodes(my_list);
	printf("Size of list: %d\n", my_list->size); //Should be 0
	free(my_list);
	
	free_mlfq_queues(my_mlfq);
	free(my_mlfq);
	
	free(thread0);
	free(thread1);
	free(thread2);
	free(thread3);
	free(thread4);
	
	return 0;
}
