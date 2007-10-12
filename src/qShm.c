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
 *
 * @note
 * @code
 *   [your header file]
 *   struct SharedData {
 *     (... structrue definitions ...)
 *   }
 *
 *   [shared memory creater]
 *   // create shared memory
 *   int shmid = qShmInit("/some/file/for/generating/unique/key", sizeof(struct SharedData), true);
 *   if(shmid < 0) {
 *     printf("ERROR: Can't initialize shared memory.\n");
 *     return -1;
 *   }
 *
 *   // get shared memory pointer
 *   struct SharedData *sdata = (SharedData *)qShmGet(shmid);
 *   if(sdata == NULL) {
 *     printf("ERROR: Can't get shared memory.\n");
 *     return -1;
 *   }
 *
 *   [shared memory user]
 *   // get shared memory id
 *   int shmid = qShmGetId("/some/file/for/generating/unique/key");
 *   if(shmid < 0) {
 *     printf("ERROR: Can't get shared memory id.\n");
 *     return -1;
 *   }
 *
 *   // get shared memory pointer
 *   struct SharedData *sdata = (SharedData *)qShmGet(shmid);
 *   if(sdata == NULL) {
 *     printf("ERROR: Can't get shared memory.\n");
 *     return -1;
 *   }
 * @endcode
 */

#ifndef WITHOUT_IPC

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
int qShmInit(char *keyfile, int keyid, size_t size, bool autodestroy) {
	key_t semkey;
	int shmid;

	/* generate unique key using ftok() */
	if(keyfile != NULL) {
		semkey = ftok(keyfile, keyid);
		if (semkey == -1) return -1;
	} else {
		semkey = IPC_PRIVATE;
	}

	/* create shared memory */
	if ((shmid = shmget(semkey, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(autodestroy == false) return -1;

		/* destroy & re-create */
		if((shmid = qShmGetId(keyfile)) >= 0) qShmFree(shmid);
		if ((shmid = shmget(semkey, size, IPC_CREAT | IPC_EXCL | 0666)) == -1) return -1;
	}

	return shmid;
}

/**
 * Under-development
 *
 * @since not released yet
 */
int qShmGetId(char *keyfile) {
	int shmid;

	/* generate unique key using ftok() */
	key_t semkey = ftok(keyfile, 'Q');
	if (semkey == -1) return -1;

	/* get current shared memory id */
	if ((shmid = shmget(semkey, 0, 0)) == -1) return -1;

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

#endif /* WITHOUT_IPC */
