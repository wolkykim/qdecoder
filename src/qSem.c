/**************************************************************************
 * qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

/**
 * @file qSem.c Semaphore Handling API
 *
 * @note
 * @code
 *   [daemon main]
 *   #define MAX_SEMAPHORES (2)
 *
 *   // create semaphores
 *   int semid = qSemInit("/some/file/for/generating/unique/key", 'q', MAX_SEMAPHORES, true);
 *   if(semid < 0) {
 *     printf("ERROR: Can't initialize semaphores.\n");
 *     return -1;
 *   }
 *
 *   // fork childs
 *   (... child forking codes ...)
 *
 *   // at the end of daemon, free semaphores
 *   if(semid >= 0) qSemFree(semid);
 *
 *   [forked child]
 *   // critical section for resource 0
 *   qSemEnter(0);
 *   (... guaranteed as atomic procedure ...)
 *   qSemLeave(0);
 *
 *   (... some codes ...)
 *
 *   // critical section for resource 1
 *   qSemEnter(1);
 *   (... guaranteed as atomic procedure ...)
 *   qSemLeave(1);
 *
 *   [other program which uses resource 1]
 *   int semid = qSemGetId("/some/file/for/generating/unique/key", 'q');
 *   if(semid < 0) {
 *     printf("ERROR: Can't get semaphore id.\n");
 *     return -1;
 *   }
 *
 *   // critical section for resource 1
 *   qSemEnter(1);
 *   (... guaranteed as atomic procedure ...)
 *   qSemLeave(1);
 *
 * @endcode
 */

#ifndef DISABLE_IPC

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/sem.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Initialize semaphore
 *
 * @param keyfile	seed for generating unique IPC key
 * @param keyid		seed for generating unique IPC key
 * @param nsems		number of semaphore to initialize
 * @param ifexistdestroy set to true to destroy if semaphore already exists
 *
 * @return		non-negative shared memory identifier if successful, otherwise returns -1
 */
int qSemInit(const char *keyfile, int keyid, int nsems, bool ifexistdestroy) {
	key_t semkey;
	int semid;

	// generate unique key using ftok();
	if(keyfile != NULL) {
		semkey = ftok(keyfile, keyid);
		if (semkey == -1) return -1;
	} else {
		semkey = IPC_PRIVATE;
	}

	// create semaphores
	if ((semid = semget(semkey, nsems, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(ifexistdestroy == false) return -1;

		// destroy & re-create
		if((semid = qSemGetId(keyfile, keyid)) >= 0) qSemFree(semid);
		if ((semid = semget(semkey, nsems, IPC_CREAT | IPC_EXCL | 0666)) == -1) return -1;
	}

	// initializing
	int i;
	for (i = 0; i < nsems; i++) {
		struct sembuf sbuf;

		/* set sbuf */
		sbuf.sem_num = i;
		sbuf.sem_op = 1;
		sbuf.sem_flg = 0;

		/* initialize */
		if (semop(semid, &sbuf, 1) != 0) {
			qSemFree(semid);
			return -1;
		}
	}

	return semid;
}


/**
 * Get semaphore identifier by keyfile and keyid for the existing semaphore
 *
 * @param keyfile	seed for generating unique IPC key
 * @param keyid		seed for generating unique IPC key
 *
 * @return		non-negative shared memory identifier if successful, otherwise returns -1
 */
int qSemGetId(const char *keyfile, int keyid) {
	int semid;

	/* generate unique key using ftok() */
	key_t semkey = ftok(keyfile, keyid);
	if (semkey == -1) return -1;

	/* get current semaphore id */
	if ((semid = semget(semkey, 0, 0)) == -1) return -1;

	return semid;
}

/**
 * Turn on the flag of semaphore then entering critical section
 *
 * @param semid		semaphore identifier
 * @param semno		semaphore number
 *
 * @return		true if successful, otherwise returns false
 *
 * @note If the semaphore is already turned on, this will wait until released
 */
bool qSemEnter(int semid, int semno) {
	struct sembuf sbuf;

	/* set sbuf */
	sbuf.sem_num = semno;
	sbuf.sem_op = -1;
	sbuf.sem_flg = SEM_UNDO;

	/* lock */
	if (semop(semid, &sbuf, 1) != 0) return false;
	return true;
}

/**
 * Try to turn on the flag of semaphore. If it is already turned on, do not wait.
 *
 * @param semid		semaphore identifier
 * @param semno		semaphore number
 *
 * @return		true if successful, otherwise(already turned on by other) returns false
 */
bool qSemEnterNowait(int semid, int semno) {
	struct sembuf sbuf;

	/* set sbuf */
	sbuf.sem_num = semno;
	sbuf.sem_op = -1;
	sbuf.sem_flg = SEM_UNDO | IPC_NOWAIT;

	/* lock */
	if (semop(semid, &sbuf, 1) != 0) return false;
	return true;
}

/**
 * Force to turn on the flag of semaphore.
 *
 * @param semid		semaphore identifier
 * @param semno		semaphore number
 * @param maxwaitms	maximum waiting micro-seconds to release
 * @param forceflag	status will be stored, it can be NULL if you don't need this information
 *
 * @return		true if successful, otherwise returns false
 *
 * @note
 * This will wait the semaphore to be released with in maxwaitms.
 * If it it released by locker normally with in maxwaitms, forceflag will be set to false.
 * But if maximum maxwaitms is exceed and the semaphore is released forcely, forceflag will
 * be set to true.
 */
bool qSemEnterForce(int semid, int semno, int maxwaitms, bool *forceflag) {
	int wait;
	for(wait = 0; wait < maxwaitms; wait += 10) {
		if(qSemEnterNowait(semid, semno) == true) {
			if(forceflag != NULL) *forceflag = false;
			return true;
		}
		usleep(10*1000); // sleep 10ms
	}

	DEBUG("force to unlock semaphore %d-%d", semid, semno);
	while(true) {
		qSemLeave(semid, semno);
		if(qSemEnterNowait(semid, semno) == true) break;
	}

	if(forceflag != NULL) *forceflag = true;
	return true;
}

/**
 * Turn off the flag of semaphore then leaving critical section
 *
 * @param semid		semaphore identifier
 * @param semno		semaphore number
 *
 * @return		true if successful, otherwise returns false
 */
bool qSemLeave(int semid, int semno) {
	struct sembuf sbuf;

	/* set sbuf */
	sbuf.sem_num = semno;
	sbuf.sem_op = 1;
	sbuf.sem_flg = SEM_UNDO;

	/* unlock */
	if (semop(semid, &sbuf, 1) != 0) return false;
	return true;
}

/**
 * Get the status of semaphore
 *
 * @param semid		semaphore identifier
 * @param semno		semaphore number
 *
 * @return		true for the flag on, false for the flag off
 */
bool qSemCheck(int semid, int semno) {
	if(semctl(semid, semno, GETVAL, 0) == 0) return true; // locked
	return false; // unlocked
}

/**
 * Release semaphore to system
 *
 * @param semid		semaphore identifier
 *
 * @return		true if successful, otherwise returns false
 */
bool qSemFree(int semid) {
	if (semid < 0) return false;
	if (semctl(semid, 0, IPC_RMID, 0) != 0) return false;
	return true;
}

#endif /* DISABLE_IPC */
