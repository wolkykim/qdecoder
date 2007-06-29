/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

/**
 * @file qShm.c Shared Memory Handling API
 */

#include <sys/shm.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Under-development
 *
 * @since 8.1R
 */
int qShmInit(char *keyfile, size_t nSize, Q_BOOL autodestroy) {
	int shmid;

	// generate unique key using ftok();
	key_t nShmKey = ftok(keyfile, 'Q');
	if (nShmKey == -1) return -1;

	// create shared memory
	if ((shmid = shmget(nShmKey, nSize, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(autodestroy == Q_FALSE) return -1;

		// destroy & re-create
		if(qShmDestroy(keyfile) == 0) return -1;
		if ((shmid = shmget(nShmKey, nSize, IPC_CREAT | IPC_EXCL | 0666)) == -1) return -1;
	}

	return shmid;
}

/**
 * Under-development
 *
 * @since 8.1R
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
 * @since 8.1R
 */
Q_BOOL qShmFree(int shmid) {
	if (shmid < 0) return Q_FALSE;
	if (shmctl(shmid, IPC_RMID, 0) != 0) return Q_FALSE;
	return Q_TRUE;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qShmDestroy(char *keyfile) {
	int shmid;

	// generate unique key using ftok();
	key_t nShmKey = ftok(keyfile, 'Q');
	if (nShmKey == -1) return Q_FALSE;

	// get current shared memory id
	if ((shmid = shmget(nShmKey, 0, 0)) == -1) return Q_FALSE;

	// destory current shared memory
	return qShmFree(shmid);
}
