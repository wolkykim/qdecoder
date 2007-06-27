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

#include <sys/shm.h>
#include "qDecoder.h"
#include "qInternal.h"

/**********************************************
** Usage :
** Return: In case of success returns shared memory
**         identifier. Otherwise returns -1;
** Do    :
**********************************************/
int qShmInit(char *pszKeyPath, size_t nSize, int nIfExistsDestroy) {
	int nShmId;

	// generate unique key using ftok();
	key_t nShmKey = ftok(pszKeyPath, 'Q');
	if (nShmKey == -1) return -1;

	// create shared memory
	if ((nShmId = shmget(nShmKey, nSize, IPC_CREAT | IPC_EXCL | 0666)) == -1) {
		if(nIfExistsDestroy == 0) return -1;

		// destroy & re-create
		if(qShmDestroy(pszKeyPath) == 0) return -1;
		if ((nShmId = shmget(nShmKey, nSize, IPC_CREAT | IPC_EXCL | 0666)) == -1) return -1;
	}

	return nShmId;
}

/**********************************************
** Usage :
** Return: In case of success returns shared memory
**         pointer. Otherwise returns NULL.
** Do    :
**********************************************/
void *qShmGet(int nShmId) {
	void *pShm;

	if (nShmId < 0) return NULL;
	pShm = shmat(nShmId, 0, 0);
	if(pShm == (void *)-1) return NULL;
	return pShm;
}

/**********************************************
** Usage :
** Return: In case of success returns 1.
**         Otherwise returns 0;
** Do    :
**********************************************/
int qShmFree(int nShmId) {
	if (nShmId < 0) return 0;
	if (shmctl(nShmId, IPC_RMID, 0) != 0) return 0;
	return 1;
}

/**********************************************
** Usage :
** Return: In case of success returns 1,
**         Otherwise returns 0;
** Do    :
**********************************************/
int qShmDestroy(char *pszKeyPath) {
	int nShmId;

	// generate unique key using ftok();
	key_t nShmKey = ftok(pszKeyPath, 'Q');
	if (nShmKey == -1) return 0;

	// get current shared memory id
	if ((nShmId = shmget(nShmKey, 0, 0)) == -1) return 0;

	// destory current shared memory
	return qShmFree(nShmId);
}
