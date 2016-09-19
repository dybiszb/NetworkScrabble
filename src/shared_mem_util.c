#include "../include/shared_mem_util.h"

void shared_mem_init(int* shmId, int size, char shmName)
{
	key_t key;
	
	if((key = ftok("./", shmName)) == -1)
	{
		ERR("shared_mem_init() : ftok ");
	}
	
	if((*shmId=shmget(key, size, IPC_CREAT | 0666)) == -1 )
	{
		ERR("shared_mem_init(): shmget error");
	}
}

char* shared_mem_attach(int shmId)
{
	char* pShm = NULL;

	if((pShm=shmat(shmId, NULL,0)) == (char *)-1)
	{
		ERR("shared_mem_attach(): shmat ");
	}
	return pShm;
}

void shared_mem_detach(char* shmAddress)
{
    if (shmdt(shmAddress) == -1) 
    {
        ERR("shared_mem_detach(): shmdt ");
    }
}

void shared_mem_delete(int shmId)
{
	if((shmctl(shmId, IPC_RMID, NULL)) == -1)
	{
		ERR("shared_mem_delete() : shmId ");
	}
}
