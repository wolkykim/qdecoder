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
 * @file qObstack.c Another simple implementation of GNU obstack
 *
 * @note
 * This API can be disabled(excluded in the library) with compile option
 * -DWITHOUT_OBSTACK.
 *
 * @code
 *   Q_OBSTACK *obstack = qObstackInit();
 *   char *final;
 *
 *   qObstackGrow(obstack, "A", 1);
 *   qObstackGrow(obstack, "BC", 2);
 *   qObstackGrow(obstack, "DEF", 3);
 *
 *   final = (char *)qObstackFinish(obstack);
 *   printf("%s\n", final);
 *
 *   qObstackFree(obstack);
 * @endcode
 */

#ifndef WITHOUT_OBSTACK

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"

/**
 * Under-development
 *
 * @since not released yet
 */
Q_OBSTACK *qObstackInit(void) {
	Q_OBSTACK *obstack;

	obstack = (Q_OBSTACK *)malloc(sizeof(Q_OBSTACK));
	if(obstack == NULL) return NULL;

	memset((void *)obstack, 0, sizeof(Q_OBSTACK));
	return obstack;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackGrow(Q_OBSTACK *obstack, void *data, int size) {
	char *name;
	void *value;

	if(size <= 0) return false;

	name = (char *)malloc(10+1);
	if(name == NULL) return false;
	sprintf(name, "%d", size);

	value = (void *)malloc(size);
	if(value == NULL) { free(name); return false; }
	memcpy(value, data, size);

	obstack->last = qEntryAdd(obstack->last, name, (char *)value, 2);
	if(obstack->first == NULL) obstack->first = obstack->last;

	obstack->size += size;
	obstack->num++;

	return true;
}

/**
 * Under-development
 *
 * @since not released yet
 */
void *qObstackFinish(Q_OBSTACK *obstack) {
	Q_ENTRY *entries;
	void *dp;

	if(obstack->final != NULL) free(obstack->final);

	obstack->final = dp = (void *)malloc(obstack->size + 1);
	if(obstack->final == NULL) return NULL;

	for (entries = obstack->first; entries; entries = entries->next) {
		int size = atoi(entries->name);
		memcpy(dp, (void *)entries->value, size);
		dp += size;
	}
	*((char *)dp) = '\0';

	return obstack->final;
}

/**
 * Under-development
 *
 * @since not released yet
 */
void *qObstackGetFinal(Q_OBSTACK *obstack) {
	return obstack->final;
}

/**
 * Under-development
 *
 * @since not released yet
 */
int qObstackGetSize(Q_OBSTACK *obstack) {
	return obstack->size;
}

/**
 * Under-development
 *
 * @since not released yet
 */
bool qObstackFree(Q_OBSTACK *obstack) {
	qEntryFree(obstack->first);
	free(obstack);
	return true;
}

#endif /* WITHOUT_OBSTACK */
