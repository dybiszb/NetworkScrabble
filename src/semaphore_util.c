// author: dybisz

#include "../include/semaphore_util.h"

void semaphore_init(int* semId, char semName, int n)
{
	int i;
	key_t key;
	union semun tmp;
	
	key = ftok("./", semName);
	tmp.val = 1;
	
	/* Create semaphore */
	if((*semId = semget(key, n, 0666 | IPC_CREAT)) < 0)
		ERR("semaphore_init() : semget");

	/* Set value of each semaphore in the set to 1 */
	for(i = 0; i < n; i++)
	{
		if(semctl(*semId, i, SETVAL, tmp) == -1)
			ERR("semaphore_Init() : semctl ");
	}
}

void semaphore_remove(int semId)
{
	if(semctl(semId, 0, IPC_RMID) == -1)
		ERR("semaphore_remove() : semctl ");
}

int semaphore_lock(int semId, short semIndex ,short flag)
{
	struct sembuf tmp;
	
	/* Set operations */
	tmp.sem_num = semIndex;
	tmp.sem_op  = -1;
    tmp.sem_flg = 1;
    
    /* perform value change */
    if(semop(semId, &tmp, 1) == -1)
	{
		/* EINTR  While blocked in this system call, the thread caught a signal */
		if(errno == EINTR)
			return INTERRUPTED;
		else ERR("semaphore_lock(): semop error.");
	}
	return ACUIRED;
}

int semaphore_unlock(int semId,short semIndex, short flag)
{
	struct sembuf tmp;
	
	/* Set operations */
	tmp.sem_num = semIndex;
	tmp.sem_op  = 1;
    tmp.sem_flg = flag;
    
    /* perform value change */
    if(semop(semId, &tmp, 1) == -1)
	{
		/* EINTR  While blocked in this system call, the thread caught a signal */
		if(errno == EINTR)
			return INTERRUPTED;
		else ERR("semaphore_unlock(): semop ");
	}
	return UNLOCKED;
}
