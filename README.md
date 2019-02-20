# 416Project2

CS416 Project2 – Planning

pthread_create():
- create TCB
- make the context
- push thread to data structure (ordered linked list and TCB hash table)

pthread_yield():
if NOT main:
- change current thread state from RUNNING to READY
- increment thread_counter by 1
- 
pthread_join(thread T2, void **value_ptr):
- if called for first time:
	- run scheduler
	- done
- fetch T2 from TCB table structure
- if T2 state is DONE:
	-if value_ptr is not null:
		- get T2’s return value (from where??) 	
