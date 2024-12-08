// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value) {
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...

	struct semaphore* sem = (struct semaphore*) smalloc(semaphoreName,
			sizeof(struct semaphore), 1);
	if (sem == NULL) {
		panic("Failed to allocate memory for semaphore");
	}

	strcpy(sem->semdata->name, semaphoreName);
	sem->semdata->count = value;
	sem->semdata->lock = 0;

	struct Env_Queue myQueue;
	uint32 queue_addr = (uint32) &myQueue;
	sys_init_queue(queue_addr);
	sem->semdata->queue = myQueue;

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

	return sem;
}

void wait_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...

	int keyw = 1;
	do {
		int temp = keyw;
		keyw = sem.semdata->lock;
		sem.semdata->lock = temp;
	} while (keyw != 0);

	sem.semdata->count--;
	if (sem.semdata->count < 0) {
		sys_enqueue((uint32) &sem.semdata->queue);
		//block // Properly block the current process
	}
	sem.semdata->lock = 0; // Release the loc
}

void signal_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	int keyw = 1;
	do {
		int temp = keyw;
		keyw = sem.semdata->lock;
		sem.semdata->lock = temp;
	} while (keyw != 0);

	sem.semdata->count++;
	if (sem.semdata->count <= 0) {
		struct Env* env = (struct Env*) sys_dequeue(
				(uint32) &sem.semdata->queue);
		if (env == NULL) {
			panic("the env not found");
		}
		//unblock // Properly unblock the environment
	}

	sem.semdata->lock = 0; // Release the lock
}

int semaphore_count(struct semaphore sem) {
	return sem.semdata->count;
}
