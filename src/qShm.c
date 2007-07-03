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
 * @file qShm.c Shared Memory Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>
#include "qDecoder.h"

/**
 * Under-development
 *
 * @since not released yet
 */
int qShmInit(char *keyfile, size_t size, bool autodestroy) {
	int shmid;

	/* generate unique key using ftok() */
	key_t nShmKey = ftok(keyfile, 'Q');
	if (nShmKey == -1) return -1;

	/* create shared memory */
	if ((shmid = shmget(nShmKey, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(autodestroy == false) return -1;

		/* destroy & re-create */
		if(qShmDestroy(keyfile) == false) return -1;
		if ((shmid = shmget(nShmKey, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) return -1;
	}

	return shmid;
}

/**
 * Under-development
 *
 * @since not released yet
 */
void *qShmGet(int shmid) {
	void *pShm;

	if (shmid < 0) return NULL;
	pShm = shmat(shmid, 0, 0);
	if(pShm == (void *)-1) return NULL;
	return pShm;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qShmFree(int shmid) {
	if (shmid < 0) return false;
	if (shmctl(shmid, IPC_RMID, 0) != 0) return false;
	return true;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qShmDestroy(char *keyfile) {
	int shmid;

	/* generate unique key using ftok() */
	key_t nShmKey = ftok(keyfile, 'Q');
	if (nShmKey == -1) return false;

	/* get current shared memory id */
	if ((shmid = shmget(nShmKey, 0, 0)) == -1) return false;

	/* destory current shared memory */
	return qShmFree(shmid);
}
