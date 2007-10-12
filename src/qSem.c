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

#ifndef WITHOUT_IPC

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/sem.h>
#include "qDecoder.h"

/**
 * Under-development
 *
 * @since not released yet
 */
int qSemInit(char *keyfile, int keyid, int nsems, bool autodestroy) {
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
		if(autodestroy == false) return -1;

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
 * Under-development
 *
 * @since not released yet
 */
int qSemGetId(char *keyfile, int keyid) {
	int semid;

	/* generate unique key using ftok() */
	key_t semkey = ftok(keyfile, keyid);
	if (semkey == -1) return -1;

	/* get current semaphore id */
	if ((semid = semget(semkey, 0, 0)) == -1) return -1;

	return semid;
}

/**
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
 */
bool qSemFree(int semid) {
	if (semid < 0) return false;
	if (semctl(semid, 0, IPC_RMID, 0) != 0) return false;
	return true;
}

#endif /* WITHOUT_IPC */
