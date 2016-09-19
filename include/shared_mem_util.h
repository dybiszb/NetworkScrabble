#ifndef SHARED_MEM_UTIL_H
#define SHARED_MEM_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "settings.h"

/*
 * Basically wraps shmget() to provide error check.
 */
void shared_mem_init(int*,int, char);

/*
 * Error checking for shmat().
 */
char* shared_mem_attach(int);

/*
 * Error checking for shmdt().
 */
void shared_mem_detach(char*);

/*
 * Deletes preserved shared memory.
 */
 void shared_mem_delete(int );
 
#endif
