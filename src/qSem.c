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
 */

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
int qSemInit(char *keyfile, int nsems, bool autodestroy) {
	int semid;

	// generate unique key using ftok();
	key_t semkey = ftok(keyfile, 'q');
	if (semkey == -1) return -1;

	// create semaphores
	if ((semid = semget(semkey, nsems, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(autodestroy == false) return -1;

		// destroy & re-create
		if(qSemDestroy(keyfile) == false) return -1;
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
bool qSemFree(int semid) {
	if (semid < 0) return false;
	if (semctl(semid, 0, IPC_RMID, 0) != 0) return false;
	return true;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qSemDestroy(char *keyfile) {
	int semid;

	/* generate unique key using ftok() */
	key_t semkey = ftok(keyfile, 'q');
	if (semkey == -1) return false;

	/* get current semaphore id */
	if ((semid = semget(semkey, 0, 0)) == -1) return false;

	/* destory semaphore */
	return qSemFree(semid);
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qSemCriticalEnter(int semid, int semno) {
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
bool qSemCriticalLeave(int semid, int semno) {
	struct sembuf sbuf;

	/* set sbuf */
	sbuf.sem_num = semno;
	sbuf.sem_op = 1;
	sbuf.sem_flg = SEM_UNDO;

	/* unlock */
	if (semop(semid, &sbuf, 1) != 0) return false;
	return true;
}
