/*
 * Copyright 2008 The qDecoder Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file qHashtbl.c Hash-table Data Structure API
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

static int _findEmpty(Q_HASHTBL *tbl, int startidx);
static int _getIdx(Q_HASHTBL *tbl, const char *key, int hash);
static bool _putData(Q_HASHTBL *tbl, int idx, int hash, const char *key, const void *value, int size, int count);
static bool _removeData(Q_HASHTBL *tbl, int idx);

/**
 * Initialize dynamic-hash table
 *
 * @param max		a number of maximum  size of Q_HASHARR
 *
 * @return		a pointer of malloced Q_HASHTBL, otherwise returns false
 *
 * @code
 *   Q_HASHTBL *hashtbl = qHashtblInit(1000);
 *   qHashtblFree(hashtbl);
 * @endcode
 */
Q_HASHTBL *qHashtblInit(int max) {
	if(max <= 0) return NULL;

	Q_HASHTBL *tbl = (Q_HASHTBL *)malloc(sizeof(Q_HASHTBL));
	if(tbl == NULL) return NULL;

	memset((void *)tbl, 0, sizeof(Q_HASHTBL));

	tbl->count = (int *)malloc(sizeof(int) * max);
	if(tbl->count != NULL) memset((void *)(tbl->count), 0, (sizeof(int) * max));
	tbl->hash = (int *)malloc(sizeof(int) * max);
	if(tbl->hash != NULL) memset((void *)(tbl->hash), 0, (sizeof(int) * max));

	tbl->key = (char **)malloc(sizeof(char *) * max);
	if(tbl->key != NULL) memset((void *)(tbl->key), 0, (sizeof(char *) * max));
	tbl->value = (void **)malloc(sizeof(char *) * max);
	if(tbl->value != NULL) memset((void *)(tbl->value), 0, (sizeof(void *) * max));
	tbl->size = (int *)malloc(sizeof(int) * max);
	if(tbl->size != NULL) memset((void *)(tbl->size), 0, (sizeof(int) * max));

	if(tbl->count == NULL || tbl->hash == NULL || tbl->key == NULL || tbl->value == NULL || tbl->size == NULL) {
		qHashtblFree(tbl);
		return NULL;
	}

	tbl->max = max;
	return tbl;
}

/**
 * De-allocate hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 *
 * @return		true if successful, otherwise returns false
 */
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
 * Put object into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 * @param value		value object data
 * @param size		size of value
 *
 * @return		true if successful, otherwise returns false
 */
bool qHashtblPut(Q_HASHTBL *tbl, const char *key, const void *value, int size) {
	if(tbl == NULL || key == NULL || value == NULL) return false;

	// get hash integer
	int hash = (int)qHashFnv32(tbl->max, key, strlen(key));

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
 * Put string into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 * @param value		value string
 *
 * @return		true if successful, otherwise returns false
 */
bool qHashtblPutStr(Q_HASHTBL *tbl, const char *key, const char *value) {
	int size = (value != NULL) ? (strlen(value) + 1) : 0;
	return qHashtblPut(tbl, key, value, size);
}

/**
 * Put integer into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 * @param value		value integer
 *
 * @return		true if successful, otherwise returns false
 */
bool qHashtblPutInt(Q_HASHTBL *tbl, const char *key, const int value) {
	char data[10+1];
	sprintf(data, "%d", value);
	return qHashtblPut(tbl, key, data, strlen(data) + 1);
}

/**
 * Get object from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 * @param size		if not NULL, oject size will be stored
 *
 * @return		malloced object pointer if successful, otherwise(not found) returns NULL
 *
 * @note
 * returned object must be freed after done using.
 */
void *qHashtblGet(Q_HASHTBL *tbl, const char *key, int *size) {
	if(tbl == NULL || key == NULL) return NULL;

	int hash = (int)qHashFnv32(tbl->max, key, strlen(key));
	int idx = _getIdx(tbl, key, hash);
	if (idx < 0) return NULL;

	void *value = (char *)malloc(tbl->size[idx]);
	memcpy(value, tbl->value[idx], tbl->size[idx]);

	if(size != NULL) *size = tbl->size[idx];
	return value;
}

/**
 * Get string from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 *
 * @return		string pointer if successful, otherwise(not found) returns NULL
 *
 * @note
 * returned object must be freed after done using.
 */
char *qHashtblGetStr(Q_HASHTBL *tbl, const char *key) {
	return qHashtblGet(tbl, key, NULL);
}

/**
 * Get integer from hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 *
 * @return		value integer if successful, otherwise(not found) returns 0
 */
int qHashtblGetInt(Q_HASHTBL *tbl, const char *key) {
	char *data = qHashtblGet(tbl, key, NULL);
	if(data == NULL) return 0;

	int value = atoi(data);
	free(data);

	return value;
}

/**
 * Get first key name
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param idx		index pointer
 *
 * @return		key name string if successful, otherwise returns NULL
 *
 * @note
 * Do not free returned key string.
 *
 * @code
 *   char *key;
 *   int idx;
 *   for(key = qHashtblGetFirstKey(tbl, &idx); key != NULL; key = qHashtblGetNextKey(tbl, &idx) {
 *     char *value = qHashtblGetStr(tbl, key);
 *     if(value != NULL) free(value);
 *   }
 * @endcode
 */
const char *qHashtblGetFirstKey(Q_HASHTBL *tbl, int *idx) {
	if(idx != NULL) *idx = -1;
	return qHashtblGetNextKey(tbl, idx);
}

/**
 * Get next key name
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param idx		index pointer
 *
 * @return		key name string if successful, otherwise(end of table) returns NULL
 *
 * @note
 * Do not free returned key string.
 */
const char *qHashtblGetNextKey(Q_HASHTBL *tbl, int *idx) {
	if(tbl == NULL || idx == NULL) return NULL;

	for ((*idx)++; *idx < tbl->max; (*idx)++) {
		if (tbl->count[*idx] == 0) continue;
		return tbl->key[*idx];
	}

	*idx = tbl->max;
	return NULL;
}

/**
 * Remove key from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param key		key string
 *
 * @return		true if successful, otherwise(not found) returns false
 */
bool qHashtblRemove(Q_HASHTBL *tbl, const char *key) {
	if(tbl == NULL || key == NULL) return false;

	int hash = (int)qHashFnv32(tbl->max, key, strlen(key));
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
 * Print hash table for debugging purpose
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param out		output stream
 * @param showvalue	print out value if set to true
 *
 * @return		true if successful, otherwise returns false
 */
bool qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue) {
	if(tbl == NULL || out == NULL) return false;

	int idx, num;
	for (idx = 0, num = 0; idx < tbl->max && num < tbl->num; idx++) {
		if (tbl->count[idx] == 0) continue;
		fprintf(out, "%s=%s (idx=%d,hash=%d,size=%d)\n", tbl->key[idx], (showvalue)?(char*)tbl->value[idx]:"_binary_", idx, tbl->hash[idx], tbl->size[idx]);
		num++;
	}

	return true;
}

/**
 * Get hash table internal status
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param used		if not NULL, a number of used keys will be stored
 * @param max		if not NULL, the maximum usable number of keys will be stored
 *
 * @return		true if successful, otherwise returns false
 */
bool qHashtblStatus(Q_HASHTBL *tbl, int *used, int *max) {
	if(tbl == NULL) return false;

	if(used != NULL) *used = tbl->num;
	if(max != NULL) *max = tbl->max;

	return true;
}

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////

// find empty slot : return empty slow number, otherwise returns -1.
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

static int _getIdx(Q_HASHTBL *tbl, const char *key, int hash) {
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

				idx++;
				if(idx == hash) return -1;
			}

			// is same key?
			if (!strcmp(tbl->key[idx], key)) return idx;

			idx++;
			if (idx >= tbl->max) idx = 0;
			if(idx == hash) break;
		}
	}

	return -1;
}

static bool _putData(Q_HASHTBL *tbl, int idx, int hash, const char *key, const void *value, int size, int count) {
	// check if used
	if(tbl->count[idx] != 0) return false;

	// store
	tbl->hash[idx] = hash;
	tbl->key[idx] = strdup(key);
	tbl->value[idx] = malloc(size);
	if(tbl->value[idx] == NULL) {
		free(tbl->key[idx]);
		return false;
	}
	memcpy(tbl->value[idx], value, size);
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

#endif /* DISABLE_DATASTRUCTURE */
