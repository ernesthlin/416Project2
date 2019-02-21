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
	int priority_level; //@author: Ernest - The priority level of the thread (for MLFQ).
	my_pthread_t *joined_on; //@author: Ernest - The thread ID of the thread this thread is waiting for/joined on.
	bool called_exit; //@author: Ernest - Initially, false, only true if thread explicitly calls pthread_exit().
} tcb; 

//Hash Table Structs
typedef struct hash_table {
	int size; //@author: Jake - how many threads are in this hash table
	struct hash_node * ht[HASH_SIZE]; //@author: Jake - creates an array of hash_node pointers with Hash_Size amount of consecutive space
} hash_table;

typedef struct hash_node {
	struct threadControlBlock * thread; //@author: Jake - the threadControlBlock that this node holds
	struct hash_node * next; //@author: Jake - the next node in the chain for this linked list
} hash_node;


//Priority Linked List (STCF) Structs
typedef struct list {
	struct list_node * head; // @author: Jake - head of list. The next thread to run
	int num_threads; // @author: Jake - tracks how many threads are in the list
	
} list;

typedef struct list_node {
	struct threadControlBlock * thread;
	struct list_node * next;
} list_node;


//Queue Linked List (MLFQ) Structs
typedef struct mlfq {
	struct queue * mlfq_scheduler[MLFQ_LEVELS]; //@author: Jake - an array of queues each with a different priority. Threads will be moved from one queue to another when they pass a certain amount of time quantums
} mlfq;

typedef struct queue {
	int size; //@author: Jake - how many threads are in this queue linked list
	struct queue_node * head; //@author: Jake - the head of the queue where we will be getting from
	struct queue_node * tail; //@author: Jake - the end of the queue where we will be adding to
} queue;

typedef struct queue_node {
	struct threadControlBlock * thread;
	struct queue_node * next;
} queue_node;

/* @author: Jake - descriptions in file 
*/
//Hash Table Prototypes
void create_hash_table(hash_table * ht);
int hashcode(my_pthread_t threadID);
struct threadControlBlock * get_hash_thread(my_pthread_t thread_id);
void add_hash_thread(hash_table * ht, threadControlBlock * thread);
void free_hash_nodes(hash_table * ht);

//Priority Linked List (STCF) Prototypes
void create_list(list * sched_list);
void add_list_thread(list * sched_list, hash_table * ht, my_pthread_t thread_id);
struct threadControlBlock * get_list_thread(list * sched_list);
bool listIsEmpty(list * sched_list);
void free_list_nodes(list* sched_list);

//Queue Linked List (MLFQ) Prototypes
void create_mlfq(mlfq * mlfq_table);
void free_mlfq_queues(mlfq * mlfq_table);
void create_queue(queue * queue_list);
void enqueue_thread(queue * queue_list, hash_table * ht, my_pthread_t thread_id);
struct threadControlBlock * dequeue_thread(queue * queue_list);
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
void add_hash_thread(hash_table * ht, threadControlBlock * thread) {
	hash_node * hn = (struct hash_node *) malloc(sizeof(struct hash_node));
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
struct threadControlBlock * get_hash_thread(hash_table * ht, my_pthread_t thread_id) {
	threadControlBlock * result = NULL;
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
			return result;
		}
		else {
			printf("Something went wrong with getting from hash table\n");
			exit(1);
		}
	}
	ht->size--;
	return result;
}

/* @author: Jake - frees the dynamically allocated nodes of the hash table */
void free_hash_nodes(hash_table * ht) {
	hash_node * temp;
	int i;
	for(i = 0; i < HASH_SIZE; i++) {
		while(ht->ht[i] != NULL) {
			temp = ht->ht[i];
			ht->ht[i] = ht->ht[i]->next;
			free(temp);
		}
	}
}


//Priority Linked List (STCF) Functions
/* @author: Jake - creates the priority list */
void create_list(list * sched_list) {
	sched_list->head = NULL;
	sched_list->num_threads = 0;
}

/* @author: Jake - creates a list_node for the given thread_id. Needs the hash table to get the info about the thread */
void add_list_thread(list * sched_list, hash_table * ht, my_pthread_t thread_id) {
	list_node * ln = (struct list_node *) malloc(sizeof(struct list_node));
	ln->thread = (get_hash_thread(ht, thread_id));
	ln->next = NULL;
	if(isEmpty(sched_list) || sched_list->head->thread->time_counter >= ln->thread->time_counter) {
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
	
	sched_list->num_threads++;
}

/* @author: Jake - grabs the TCB at the beginning of the priority list. Removes it from the list */
struct threadControlBlock * get_list_thread(list * sched_list) {
	if(listisEmpty(sched_list)) {
		printf("STCF List is empty, there is nothing to get\n");
		return NULL;
	}
	threadControlBlock * result = sched_list->head->thread;
	list_node * temp = sched_list->head;//Since we are disconnecting this node from the list, we have to free it. But we need to replace the head before we do that
	sched_list->head = sched_list->head->next;
	free(temp);
	
	
	sched_list->num_threads--;
	return result;
}

/* @author: Jake - quick check to see if the priority list is empty */
bool listIsEmpty(list * sched_list) {
	return (sched_list->num_threads == 0);
}

/* Frees the malloced list nodes that were created when adding to the priority queue*/
void free_list_nodes(list* sched_list) {
	list_node * temp;
	while(sched_list->head != NULL) {
		temp = sched_list->head;
		sched_list->head = sched_list->head->next;
		free(temp);
	}
}


//Queue Linked List (MLFQ) Functions
/* @author: Jake - creates a queue for the MLFQ. Initializes all the queues so they may be used as needed */
void create_mlfq(mlfq * mlfq_table) {
	int i = 0;
	for(i = 0; i < MLFQ_LEVELS; i++) {
		queue * temp = (struct * queue) malloc(sizeof(struct queue));
		mlfq_table->mlfq_scheduler[i] = temp;
	}
}

/* @author: Jake - frees all the queues in the MLFQ */
void free_mlfq_queues(mlfq * mlfq_table) {
	queue * temp = NULL;
	int i = 0;
	for(i = 0; i < MLFQ_LEVELS; i++) {
		temp = mlfq_table->mlfq_scheduler[i];
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
	queue_node * qn = (struct queue_node *) malloc(sizeof(struct queue_node));
	qn->thread = (get_hash_thread(ht, thread_id));
	qn->next = NULL;
	
	//If the queue is empty, this node is both the head and the tail
	if(queue_list->rear == NULL) {
		queue_list->head = qn;
		queue_list->tail = qn;
	} else {
		queue_list->tail->next = qn;
		queue_list->tail = qn;
	}
	queue_list->size++;
}
/* @author: Jake - grabs the TCB at the beginning of the queue. Removes it from the list */
struct threadControlBlock * dequeue_thread(queue * queue_list) {
	if(queue_list->size == 0) {
		printf("MLFQ Queue is empty, there is nothing to dequeue\n");
		return NULL;
	}
	threadControlBlock * result = sched_list->head->thread;
	queue_node * temp = sched_list->head;
	queue_list->head = queue_list->head->next;
	free(temp);
	
	queue_list->size--;
	return result;
}

/* @author: Jake - frees the remaining queue nodes in the queue */
void free_queue_nodes(queue * queue_list) {
	queue_node * temp;
	while(queue_list->head != queue_list->tail) {
		temp = queue_list->head;
		queue_list->head = queue_list->head->next;
		free(temp);
	}
	if(queue_list->head != NULL) {
		temp = queue_list->head;
		free(temp);
	}
}
/* @author: Jake */