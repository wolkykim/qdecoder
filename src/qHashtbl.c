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
 * @file qHashtable.c Hash-table Data Structure API
 */

#ifndef WITHOUT_HASHTBL

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
/////////////////////////////////////////////////////////////////////////

static int _findEmpty(Q_HASHTBL *tbl, int startidx);
static int _getIdx(Q_HASHTBL *tbl, char *key, int hash);
static bool _putData(Q_HASHTBL *tbl, int idx, int hash, char *key, char *value, int size, int count);
static bool _removeData(Q_HASHTBL *tbl, int idx);

/////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
/////////////////////////////////////////////////////////////////////////

Q_HASHTBL *qHashtblInit(int max) {
	Q_HASHTBL *tbl;

	tbl = (Q_HASHTBL *)malloc(sizeof(Q_HASHTBL));
	if(tbl == NULL) return NULL;

	memset((void *)tbl, 0, sizeof(Q_HASHTBL));

	tbl->count = (int *)malloc(sizeof(int) * max);
	if(tbl->count != NULL) memset((void *)(tbl->count), 0, sizeof(sizeof(int) * max));
	tbl->hash = (int *)malloc(sizeof(int) * max);
	if(tbl->hash != NULL) memset((void *)(tbl->hash), 0, sizeof(sizeof(int) * max));

	tbl->key = (char **)malloc(sizeof(char *) * max);
	if(tbl->key != NULL) memset((void *)(tbl->key), 0, sizeof(sizeof(char *) * max));
	tbl->value = (char **)malloc(sizeof(char *) * max);
	if(tbl->value != NULL) memset((void *)(tbl->value), 0, sizeof(sizeof(char *) * max));
	tbl->size = (int *)malloc(sizeof(int) * max);
	if(tbl->size != NULL) memset((void *)(tbl->size), 0, sizeof(sizeof(int) * max));

	if(tbl->key == NULL || tbl->value == NULL || tbl->size == NULL || tbl->count == NULL) {
		qHashtblFree(tbl);
		return NULL;
	}

	tbl->max = max;
	return tbl;
}

bool qHashtblFree(Q_HASHTBL *tbl) {
	if(tbl == NULL) return false;

	int idx, num;
	for (idx = 0, num = 0; idx < tbl->max && num < tbl->num; idx++) {
		if (tbl->count[idx] == 0) continue;
		free(tbl->key[idx]);
		free(tbl->value[idx]);

		num++;
		if(num >= tbl->num) break;
	}

	if(tbl->count != NULL) free(tbl->count);
	if(tbl->hash != NULL) free(tbl->hash);
	if(tbl->key != NULL) free(tbl->key);
	if(tbl->value != NULL) free(tbl->value);
	if(tbl->size != NULL) free(tbl->size);
	free(tbl);

	return true;
}

/**
 * @return true or false
 */

bool qHashtblPut(Q_HASHTBL *tbl, char *key, char *value, int size) {
	// get hash integer
	int hash = (int)qFnv32Hash(key, tbl->max);

	// if size is less than 0, we assume that the value is null terminated string.
	if(size < 0) size = strlen(value) + 1;

	// check, is slot empty
	if (tbl->count[hash] == 0) { // empty slot
		// put data
		_putData(tbl, hash, hash, key, value, size, 1);

		DEBUG("hashtbl: put(new) %s (idx=%d,hash=%d,tot=%d)", key, hash, hash, tbl->num);
	} else if (tbl->count[hash] > 0) { // same key exists or hash collision
		// check same key;
		int idx = _getIdx(tbl, key, hash);
		if (idx >= 0) { // same key
			// remove and recall
			qHashtblRemove(tbl, key);
			return qHashtblPut(tbl, key, value, size);
		} else { // no same key, just hash collision
			// find empty slot
			int idx = _findEmpty(tbl, hash);
			if (idx < 0) return false;

			// put data
			_putData(tbl, idx, hash, key, value, size, -1); // -1 is used for idx != hash;

			// increase counter from leading slot
			tbl->count[hash]++;

			DEBUG("hashtbl: put(col) %s (idx=%d,hash=%d,tot=%d)", key, idx, hash, tbl->num);
		}
	} else { // in case of -1. used for dup data, move it
		// find empty slot
		int idx = _findEmpty(tbl, hash);
		if (idx < 0) return false;

		// move dup data
		_putData(tbl, idx, tbl->hash[hash], tbl->key[hash], tbl->value[hash], tbl->size[hash], tbl->count[hash]);
		_removeData(tbl, hash);

		// store data
		_putData(tbl, hash, hash, key, value, size, 1);

		DEBUG("hashtbl: put(swp) %s (idx=%d,hash=%d,tot=%d)", key, hash, hash, tbl->num);
	}

	return true;
}

/**
 * @return true or false
 */
bool qHashtblRemove(Q_HASHTBL *tbl, char *key) {
	int hash = (int)qFnv32Hash(key, tbl->max);
	int idx = _getIdx(tbl, key, hash);
	if (idx < 0) return false;

	if (tbl->count[idx] == 1) {
		// just remove
		_removeData(tbl, idx);

		DEBUG("hashtbl: rem %s (idx=%d,tot=%d)", key, idx, tbl->num);
	} else if (tbl->count[idx] > 1) { // leading slot and has dup
		// find dup
		int idx2;
		for (idx2 = idx + 1; ; idx2++) {
			if (idx2 >= tbl->max) idx2 = 0;
			if (idx2 == idx) {
				DEBUG("hashtbl: BUG remove failed %s. dup not found.", key);
				return false;
			}
			if (tbl->count[idx2] == -1 && tbl->hash[idx2] == idx) break;
		}

		// move to leading slot
		int backupcount = tbl->count[idx];
		_removeData(tbl, idx); // remove leading slot
		_putData(tbl, idx, tbl->hash[idx2], tbl->key[idx2], tbl->value[idx2], tbl->size[idx2], backupcount - 1); // copy to leading slot
		_removeData(tbl, idx2); // remove dup slot

		DEBUG("hashtbl: rem(lead) %s (idx=%d,tot=%d)", key, idx, tbl->num);
	} else { // in case of -1. used for dup data
		// decrease counter from leading slot
		if(tbl->count[tbl->hash[idx]] <= 1) {
			DEBUG("hashtbl: BUG remove failed %s. counter of leading slot mismatch.", key);
			return false;
		}
		tbl->count[tbl->hash[idx]]--;

		// remove dup
		_removeData(tbl, idx);

		DEBUG("hashtbl: rem(dup) %s (idx=%d,tot=%d)", key, idx, tbl->num);
	}

	return true;
}

/**
 * @return value
 */
char *qHashtblGet(Q_HASHTBL *tbl, char *key, int *size) {
	int hash = (int)qFnv32Hash(key, tbl->max);
	int idx = _getIdx(tbl, key, hash);
	if (idx < 0) return NULL;

	if(size != NULL) *size = tbl->size[idx];
	return tbl->value[idx];
}

void qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue) {
	int idx, num;
	for (idx = 0, num = 0; idx < tbl->max && num < tbl->num; idx++) {
		if (tbl->count[idx] == 0) continue;
		fprintf(out, "%s=%s (idx=%d,hash=%d,size=%d)", tbl->key[idx], (showvalue)?tbl->value[idx]:"_binary_", idx, tbl->hash[idx], tbl->size[idx]);
		num++;
	}
}

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////

/*
 * find empty slot
 * @return empty slow number, otherwise returns -1.
 */
static int _findEmpty(Q_HASHTBL *tbl, int startidx) {
	if (startidx >= tbl->max) startidx = 0;

	int idx = startidx;
	while (true) {
		if (tbl->count[idx] == 0) return idx;

		idx++;
		if (idx >= tbl->max) idx = 0;
		if(idx == startidx) break;
	}

	return -1;
}

static int _getIdx(Q_HASHTBL *tbl, char *key, int hash) {
	if (tbl->count[hash] > 0) {
		int count, idx;
		for (count = 0, idx = hash; count < tbl->count[hash]; ) {
			// find same hash
			while(true) {
				if (idx >= tbl->max) idx = 0;

				if (tbl->count[idx] != 0 && tbl->hash[idx] == hash) {
					// found same hash
					count++;
					break;
				}

				idx = (idx + 1) % tbl->max;
				if(idx == hash) return -1;
			}

			// is same key?
			if (!strcmp(tbl->key[idx], key)) return idx;

			idx = (idx + 1) % tbl->max;
			if(idx == hash) break;
		}
	}

	return -1;
}

static bool _putData(Q_HASHTBL *tbl, int idx, int hash, char *key, char *value, int size, int count) {
	// check if used
	if(tbl->count[idx] != 0) return false;

	// store
	tbl->hash[idx] = hash;
	tbl->key[idx] = strdup(key);
	tbl->value[idx] = (char *)malloc(size);
	if(tbl->value[idx] == NULL) {
		free(tbl->key[idx]);
		return false;
	}
	memcpy((void *)tbl->value[idx], (void *)value, size);
	tbl->size[idx] = size;
	tbl->count[idx] = count;

	// increase used counter
	tbl->num++;

	return true;
}

static bool _removeData(Q_HASHTBL *tbl, int idx) {
	if(tbl->count[idx] == 0) return false;

	free(tbl->key[idx]);
	free(tbl->value[idx]);
	tbl->count[idx] = 0;

	// decrease used counter
	tbl->num--;

	return true;
}

#endif /* WITHOUT_HASHTBL */
