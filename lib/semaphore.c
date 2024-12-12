// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-------creating %s\n", semaphoreName);
	struct semaphore sem;
	sem.semdata = (struct __semdata*) smalloc(semaphoreName, sizeof(struct __semdata), 0);
	sem.semdata->count = value;
	sem.semdata->lock = 0;
	strcpy(sem.semdata->name, semaphoreName);
	sys_init_queue(&sem.semdata->queue);
	cprintf("-------finished creating %s\n", semaphoreName);
	return sem;
}
struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-------getting %s\n", semaphoreName);
	struct semaphore sem;
	sem.semdata = sget(ownerEnvID, semaphoreName);
	cprintf("-------finished getting %s\n", semaphoreName);
	return sem;
}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
    cprintf("-------wait for %s, count = %d, lock = %d\n",
            sem.semdata->name, sem.semdata->count, sem.semdata->lock);

    while (xchg(&(sem.semdata->lock), 1) != 0);

    sem.semdata->count--;
    if (sem.semdata->count < 0) {
        // Block the current process
        sys_Block_and_enqueue(&sem);
    }

    sem.semdata->lock = 0;
    cprintf("-------finished wait for %s, count = %d, lock = %d\n",
            sem.semdata->name, sem.semdata->count, sem.semdata->lock);
}

void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	cprintf("-------signal for %s\n", sem.semdata->name);
	while(xchg(&(sem.semdata->lock), 1) != 0);

	sem.semdata->count++;
	if(sem.semdata->count <= 0){
		sys_Ready_and_dequeue(&sem);
	}
	sem.semdata->lock = 0;
	cprintf("-------finished signal for %s\n", sem.semdata->name);
}

int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
