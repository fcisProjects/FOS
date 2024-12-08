// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value) {
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-----------------------------------create_semaphore-------------------------------------------------\n");
	char name[256];

	int i = 0;
	while ("SemFor"[i] != '\0') {
	    name[i] = "SemFor"[i];
	    i++;
	}

	int j = 0;
	while (semaphoreName[j] != '\0') {
	    name[i] = semaphoreName[j];
	    i++;
	    j++;
	}

	// Null-terminate the concatenated string
	name[i] = '\0';

	struct semaphore* sem = (struct semaphore*) smalloc(name,
			sizeof(struct semaphore), 0);
		if (sem == NULL) {
	    panic("Failed to allocate memory for semaphore");
	}
	cprintf("sem address: %x\n", (uint32)sem);

	cprintf("sizeof(struct __semdata) = %u\n", sizeof(struct __semdata));
	sem->semdata = (struct __semdata*) smalloc(semaphoreName,
				sizeof(struct __semdata), 0);
	if (sem->semdata == NULL) {
		panic("Failed to allocate memory for semaphore data!");
	}
	cprintf("semdata address: %x\n", (uint32)sem->semdata);

	cprintf("sem created. Initializing...\n");
	strcpy(sem->semdata->name, semaphoreName);
	cprintf("Semaphore name: %s\n", sem->semdata->name);
	sem->semdata->count = value;
	cprintf("Semaphore count: %d\n", sem->semdata->count);
	sem->semdata->lock = 0;
	cprintf("Semaphore lock: %d\n", sem->semdata->lock);
	uint32 queue_addr = (uint32)&(sem->semdata->queue);
	sys_init_queue(queue_addr);
	cprintf("Semaphore queue addr: %x\n", &(sem->semdata->queue));
	//init_queue(sem.semdata->queue);
	return *sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName) {
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...

	struct semaphore sem;
	sem.semdata = sget(ownerEnvID, semaphoreName);
	if (sem.semdata == NULL) {
		panic("Semaphore not found");
	}
	cprintf("-----------------------------------get_semaphore-------------------------------------------------\n");
	cprintf("Semaphore name: %s\n", sem.semdata->name);
	cprintf("Semaphore count: %d\n", sem.semdata->count);
	cprintf("Semaphore queue addr: %x\n", &(sem.semdata->queue));
	cprintf("Semaphore lock: %d\n", sem.semdata->lock);
	return sem;
}

void wait_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-----------------------------------wait_semaphore-------------------------------------------------");
	cprintf("Entering wait_semaphore\n");
	cprintf("Environment %d: Semaphore count before wait: %d\n", sys_getenvid(), sem.semdata->count);
	sys_pushcli();
	uint32 keyw = 1;
	do{
		xchg(&keyw, sem.semdata->lock);
	}while(keyw != 0);

	cprintf("Lock acquired. Current count: %d\n", sem.semdata->count);
	sem.semdata->count--;
	cprintf("Semaphore count after decrement: %d\n", sem.semdata->count);
	if (sem.semdata->count < 0) {

		// Pass the queue address as an integer
		uint32 queue_addr = (uint32) &(sem.semdata->queue);
		cprintf("Count is negative, enqueuing process. Queue addr: %x\n", queue_addr);
		sys_enqueue(queue_addr);
		//enqueue(&sem.semdata->queue, myEnv);

		sem.semdata->lock = 0;

		cprintf("Environment enqueued. Removing...\n");
		cprintf("Environment %d blocked and enqueued.\n", myEnv->env_id);
		uint32 e = (uint32) &myEnv;
		sys_sched_remove_ready(e);
		cprintf("Removed.\n");
	}

	sem.semdata->lock = 0;
	/*else{
		sem.semdata->lock = 0; // release
		cprintf("Lock released. Count after wait: %d\n", sem.semdata->count);
	}*/
	cprintf("Exiting wait_semaphore for semaphore: %s\n", sem.semdata->name);
}

void signal_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-----------------------------------signal_semaphore-------------------------------------------------\n");
	cprintf("Entering signal_semaphore for semaphore: %s\n", sem.semdata->name);

	uint32 keyw = 1;
	do{
		xchg(&keyw, sem.semdata->lock);
	}while(keyw != 0);

	sem.semdata->count++;
	cprintf("Semaphore count incremented: %d\n", sem.semdata->count);
	if (sem.semdata->count < 0) {
		uint32 queue_addr = (uint32) &(sem.semdata->queue);
		sys_dequeue(queue_addr);
		//denqueue(&sem.semdata->queue, myEnv);
		uint32 e = (uint32) myEnv;
		cprintf("Dequeued env: %d. Making it ready...\n", e);
		sys_sched_insert_ready(e);
	}

	sem.semdata->lock = 0;	//release
	sys_popcli();
	cprintf("Exiting signal_semaphore\n");
}

int semaphore_count(struct semaphore sem) {
	return sem.semdata->count;
}
