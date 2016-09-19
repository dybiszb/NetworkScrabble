//==============================================================================
// A set of function that encapsulates calls related to sysV's semaphores.
// One can e.g. initialize, lock unlock particular semaphore from a given set.
// =============================================================================
// author: dybisz
//------------------------------------------------------------------------------

#ifndef SEMAPHORE_UTIL_H
#define SEMPAHORE_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "settings.h"
#include "errno.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>

/**
 * Needed for semctl() function calls.
 */
union semun {
 int                 val;
 struct semid_ds*    buf;
 unsigned short int* array;
 };

/**
 * Creates a set of semaphores and saves its id.
 * In addition all values are set to 1.
 *
 * @param semId   ID of a semaphores set.
 * @param semName Name of semaphores set.
 * @param n       Numbers of semaphores in a set.
 */
void semaphore_init(int* semId, char semName, int n);

/**
 * Destroys semaphore of a given id.
 *
 * @param semId A semaphore ID.
 */
void semaphore_remove(int semId);

/*
 * Lock semaphore until semaphore_unclock() call. 
 */

/**
 * Locks semaphore until semaphore_unclock() is called.
 * In case of error different than EINTR, a process that called the procedure
 * will be closed.
 *
 * @param semId    A semaphore set ID.
 * @param semIndex Index of a semaphore to lock (in a set).
 * @param flag     Redundant. struct sembuf's sem_flg will be set to 1.
 *
 * @return         ACUIRED or INTERRUPTED flag (depending on result).
 */
int semaphore_lock(int semId, short semIndex ,short flag);

/**
 * Unlocks specified semaphore.
 * In case of error different than EINTR, a process that called the procedure
 * will be closed.
 *
 * @param semId    A semaphore set ID.
 * @param semIndex Index of a semaphore to lock (in a set).
 * @param flag     Redundant. struct sembuf's sem_flg will be set to 1.
 *
 * @return         UNLOCKED or INTERRUPTED flag (depending on result).
 */
int semaphore_unlock(int semId,short semIndex, short flag);

#endif
