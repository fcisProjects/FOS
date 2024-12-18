// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value) {
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("----------create_semaphore %s---------------\n",semaphoreName);

	struct semaphore sem;
	sem.semdata = (struct __semdata*) smalloc(semaphoreName,
			sizeof(struct __semdata), 1);
	sem.semdata->count = value;
	sem.semdata->lock = 0;
	strcpy(sem.semdata->name, semaphoreName);
	//cprintf("semaphore %s before calling the sys \n",semaphoreName);
	sys_init_queue(&sem.semdata->queue);
	//cprintf("-------finished creating %s\n", semaphoreName);
	return sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName) {
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("---------get_semaphore -----------------\n");
	struct semaphore sem;
	sem.semdata=NULL;
	sem.semdata = sget(ownerEnvID, semaphoreName);
	//cprintf("$$$$$sem data name = %s\n",sem.semdata->name);
	if (sem.semdata == NULL) {
		panic("sem.semdata  not found\n\n");
	}
	/*cprintf("Semaphore name: %s count: %d queue addr: %x\n",
			sem.semdata->name,sem.semdata->count,&(sem.semdata->queue));*/
	return sem;
}

void wait_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("--------wait_semaphore-----------------\n");
/*	cprintf("\t wait for %s, count = %d, lock = %d\n",
	            sem.semdata->name, sem.semdata->count, sem.semdata->lock);*/
	while(xchg(&(sem.semdata->lock), 1) != 0) ;
	//cprintf("***** wait sem lock after xchg %d\n",sem.semdata->lock);
	sem.semdata->count--;

	if (sem.semdata->count < 0) {
		//cprintf("\n\n `_` sem addr bef sys %x\n",&sem.semdata);
		sys_wait_semaphore(&sem.semdata);
		//sys_enqueue((uint32)&(sem.semdata));
	}
	sem.semdata->lock = 0;
	/*cprintf("\t ## finished wait for %s, count = %d, lock = %d\n",
	            sem.semdata->name, sem.semdata->count, sem.semdata->lock);*/
}

void signal_semaphore(struct semaphore sem) {
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	//cprintf("---------------- signal_semaphore -------------------\n");

	//cprintf("&& signal sem lock %d \n",sem.semdata->lock);
	sem.semdata->lock=0;
	while(xchg(&(sem.semdata->lock), 1) != 0) ;

	sem.semdata->count++;
	//cprintf("Semaphore count incremented: %d\n", sem.semdata->count);

	if (sem.semdata->count <= 0) {
		//cprintf("\n\n `_` sem addr bef sys %x\n",&sem.semdata);
		sys_signal_semaphore(&sem.semdata);
	}

	sem.semdata->lock = 0;
	//cprintf("Exiting signal_semaphore\n");
}

int semaphore_count(struct semaphore sem) {
	return sem.semdata->count;
}
