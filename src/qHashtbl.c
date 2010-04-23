/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
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
 *
 * $Id$
 */

/**
 * @file qHashtbl.c Hash-table Data Structure API
 *
 * @note
 * Use "--enable-threadsafe" configure script option to use under multi-threaded environments.
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

#ifndef _DOXYGEN_SKIP

// member methods
static void	_lock(Q_HASHTBL *tbl);
static void	_unlock(Q_HASHTBL *tbl);

static bool	_put(Q_HASHTBL *tbl, const char *name, const void *data, size_t size);
static bool	_putStr(Q_HASHTBL *tbl, const char *name, const char *str);
static bool	_putInt(Q_HASHTBL *tbl, const char *name, int num);

static void*	_get(Q_HASHTBL *tbl, const char *name, size_t *size, bool newmem);
static char*	_getStr(Q_HASHTBL *tbl, const char *name, bool newmem);
static int	_getInt(Q_HASHTBL *tbl, const char *name);

static bool	_getNext(Q_HASHTBL *tbl, Q_NOBJ_T *obj, int *idx, bool newmem);

static bool	_remove(Q_HASHTBL *tbl, const char *name);

static int	_getNum(Q_HASHTBL *tbl);
static int	_getMax(Q_HASHTBL *tbl);
static bool	_truncate(Q_HASHTBL *tbl);
static bool	_resize(Q_HASHTBL *tbl, int max);
static bool	_print(Q_HASHTBL *tbl, FILE *out, bool print_data);
static bool	_free(Q_HASHTBL *tbl);

// internal usages
static int	_findEmpty(Q_HASHTBL *tbl, int startidx);
static int	_getIdx(Q_HASHTBL *tbl, const char *name, int hash);
static bool	_putData(Q_HASHTBL *tbl, int idx, int hash, Q_NOBJ_T *obj, int count);
static bool	_removeData(Q_HASHTBL *tbl, int idx);
static bool	_setThreshold(Q_HASHTBL *tbl, int max, int threshold);

#endif

/**
 * Initialize dynamic-hash table
 *
 * @param max		a number of initial number of slots of Q_HASHTBL
 * @param resize	enable or disable auto incremental resizing
 * @param threshold	a persent of threshold for resizing. 0 for default.
 *
 * @return		a pointer of malloced Q_HASHTBL, otherwise returns false
 *
 * @code
 *   // initial table size is 1000, enable resizing, use default threshold
 *   Q_HASHTBL *hashtbl = qHashtblInit(1000, true, 0);
 *   qHashtblFree(hashtbl);
 * @endcode
 *
 * @note
 * If the total number of used slots is reached at threshold parameter, hash table is automatically enlarged to double size(_Q_HASHTBL_RESIZE_MAG).
 * The default threshold is 80% and is defined in header with _Q_HASHTBL_DEF_THRESHOLD.
 */
Q_HASHTBL *qHashtbl(int max, bool resize, int threshold) {
	if(max <= 0) return NULL;

	Q_HASHTBL *tbl = (Q_HASHTBL *)malloc(sizeof(Q_HASHTBL));
	if(tbl == NULL) return NULL;

	memset((void *)tbl, 0, sizeof(Q_HASHTBL));

	// calculate and set threshold
	if(resize == true) {
		if(threshold <= 0 || threshold > 100) threshold = _Q_HASHTBL_DEF_THRESHOLD;
		_setThreshold(tbl, max, threshold);
	}

	// allocate table space
	tbl->count = (int*)malloc(sizeof(int) * max);
	if(tbl->count != NULL) memset((void*)(tbl->count), 0, (sizeof(int) * max));
	tbl->hash = (int*)malloc(sizeof(int) * max);
	if(tbl->hash != NULL) memset((void*)(tbl->hash), 0, (sizeof(int) * max));
	tbl->obj = (Q_NOBJ_T*)malloc(sizeof(Q_NOBJ_T) * max);
	if(tbl->obj != NULL) memset((void*)(tbl->obj), 0, (sizeof(Q_NOBJ_T) * max));

	// check allocation was successful
	if(tbl->count == NULL || tbl->hash == NULL || tbl->obj == NULL) {
		_free(tbl);
		return NULL;
	}

	// assign methods
	tbl->lock	= _lock;
	tbl->unlock	= _unlock;

	tbl->put	= _put;
	tbl->putStr	= _putStr;
	tbl->putInt	= _putInt;

	tbl->get	= _get;
	tbl->getStr	= _getStr;
	tbl->getInt	= _getInt;

	tbl->getNext	= _getNext;

	tbl->remove	= _remove;

	tbl->getNum	= _getNum;
	tbl->getMax	= _getMax;
	tbl->truncate	= _truncate;
	tbl->resize	= _resize;
	tbl->print	= _print;
	tbl->free	= _free;

	// initialize recrusive mutex
	Q_MUTEX_INIT(tbl->qmutex, true);

	// now table can be used
	tbl->max = max;

	return tbl;
}

/**
 * Q_HASHtbl->lock(): Enter critical section.
 *
 * @return	none
 *
 * @note
 * Q_HASHTBL uses recursive mutex lock mechanism. And it uses lock at least as possible.
 * User can raise lock to proctect data modification during Q_HASHTBL->getNext() operation.
 */
static void _lock(Q_HASHTBL *tbl) {
	Q_MUTEX_ENTER(tbl->qmutex);
}

/**
 * Q_HASHTBL->unlock(): Leave critical section.
 *
 * @return	none
 */
static void _unlock(Q_HASHTBL *tbl) {
	Q_MUTEX_LEAVE(tbl->qmutex);
}

/**
 * Q_HASHTBL->put(): Put object into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 * @param data		data object
 * @param size		size of data object
 *
 * @return		true if successful, otherwise returns false
 */
static bool _put(Q_HASHTBL *tbl, const char *name, const void *data, size_t size) {
	if(tbl == NULL || name == NULL || data == NULL) return false;

	// get hash integer
	int hash = (int)qHashFnv32(tbl->max, name, strlen(name));
	Q_NOBJ_T obj;
	obj.name = (char*)name;
	obj.data = (char*)data;
	obj.size = size;

	Q_MUTEX_ENTER(tbl->qmutex);

	// check, is slot empty
	if (tbl->count[hash] == 0) { // empty slot
		// put data
		if(_putData(tbl, hash, hash, &obj, 1) == false) {
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}

		DEBUG("hashtbl: put(new) %s (idx=%d,hash=%d,tot=%d)", name, hash, hash, tbl->num);
	} else if (tbl->count[hash] > 0) { // same key exists or hash collision
		// check same key;
		int idx = _getIdx(tbl, name, hash);
		if (idx >= 0) { // same key
			// remove and recall
			_remove(tbl, name);
			bool ret = _put(tbl, name, data, size);
			Q_MUTEX_LEAVE(tbl->qmutex);
			return ret;
		} else { // no same key, just hash collision
			// find empty slot
			int idx = _findEmpty(tbl, hash);
			if (idx < 0) {
				Q_MUTEX_LEAVE(tbl->qmutex);
				return false;
			}

			// put data
			if(_putData(tbl, idx, hash, &obj, -1) == false) { // -1 is used for collision resolution idx != hash;
				Q_MUTEX_LEAVE(tbl->qmutex);
				return false;
			}

			// increase counter from leading slot
			tbl->count[hash]++;

			DEBUG("hashtbl: put(col) %s (idx=%d,hash=%d,tot=%d)", name, idx, hash, tbl->num);
		}
	} else { // in case of -1. used for collided data, move it
		// find empty slot
		int idx = _findEmpty(tbl, hash);
		if (idx < 0) {
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}

		// move collided data
		if(_putData(tbl, idx, tbl->hash[hash], &(tbl->obj[hash]), tbl->count[hash]) == false) {
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}
		_removeData(tbl, hash);

		// store data
		if(_putData(tbl, hash, hash, &obj, 1) == false) {
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}

		DEBUG("hashtbl: put(swp) %s (idx=%d,hash=%d,tot=%d)", name, hash, hash, tbl->num);
	}

	// check threshold
	if(tbl->threshold > 0 && tbl->num >= tbl->resizeat) {
		DEBUG("hashtbl: resizing %d/%d/%d,%d%%->%d/%d", tbl->num, tbl->resizeat, tbl->max, tbl->threshold, tbl->num, tbl->max * _Q_HASHTBL_RESIZE_MAG);
		_resize(tbl, (tbl->max * _Q_HASHTBL_RESIZE_MAG));
	}

	Q_MUTEX_LEAVE(tbl->qmutex);
	return true;
}

/**
 * Q_HASHTBL->putStr(): Put string into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 * @param data		data object
 *
 * @return		true if successful, otherwise returns false
 */
static bool _putStr(Q_HASHTBL *tbl, const char *name, const char *str) {
	size_t size = (str != NULL) ? (strlen(str) + 1) : 0;
	return _put(tbl, name, str, size);
}

/**
 * Q_HASHTBL->putInt(): Put integer into hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 * @param data		data object
 *
 * @return		true if successful, otherwise returns false
 */
static bool _putInt(Q_HASHTBL *tbl, const char *name, const int num) {
	char data[10+1];
	sprintf(data, "%d", num);
	return _put(tbl, name, data, strlen(data) + 1);
}

/**
 * Q_HASHTBL->get(): Get object from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 * @param size		if not NULL, oject size will be stored
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of data if the key is found, otherwise returns NULL.
 *
 * @code
 *   Q_HASHTBL *tbl = qHashtbl();
 *   (...codes...)
 *
 *   // with newmem flag unset
 *   size_t size;
 *   const char *data = tbl->get(tbl, "key_name", &size, false);
 *
 *   // with newmem flag set
 *   size_t size;
 *   char *data = tbl->get(tbl, "key_name", &size, true);
 *   if(data != NULL) free(data);
 * @endcode
 *
 * @note
 * If newmem flag is set, returned data will be malloced and should be deallocated by user.
 * Otherwise returned pointer will point internal buffer directly and should not be de-allocated by user.
 * In thread-safe mode, newmem flag always should be true.
 */
static void *_get(Q_HASHTBL *tbl, const char *name, size_t *size, bool newmem) {
	if(tbl == NULL || name == NULL) return NULL;

	Q_MUTEX_ENTER(tbl->qmutex);

	int hash = (int)qHashFnv32(tbl->max, name, strlen(name));
	int idx = _getIdx(tbl, name, hash);
	if (idx < 0) {
		Q_MUTEX_LEAVE(tbl->qmutex);
		return NULL;
	}

	void *data = NULL;
	if(newmem == true) {
		data = (char *)malloc(tbl->obj[idx].size);
		if(data != NULL) memcpy(data, tbl->obj[idx].data, tbl->obj[idx].size);
	} else {
		data = tbl->obj[idx].data;
	}
	if(size != NULL) *size = tbl->obj[idx].size;

	Q_MUTEX_LEAVE(tbl->qmutex);
	return data;
}

/**
 * Q_HASHTBL->getStr(): Get string from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of data if the key is found, otherwise returns NULL.
 *
 * @note
 * If newmem flag is set, returned data will be malloced and should be deallocated by user.
 */
static char *_getStr(Q_HASHTBL *tbl, const char *name, bool newmem) {
	return _get(tbl, name, NULL, newmem);
}

/**
 * Q_HASHTBL->getInt(): Get integer from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 *
 * @return		value integer if successful, otherwise(not found) returns 0
 *
 * @note
 * If newmem flag is true, returned data will be malloced and should be deallocated by user.
 */
static int _getInt(Q_HASHTBL *tbl, const char *name) {
	int num = 0;
	char *data = _get(tbl, name, NULL, true);
	if(data != NULL) {
		num = atoi(data);
		free(data);
	}

	return num;
}

/**
 * Q_HASHTBL->getNext(): Get next key name
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param idx		index pointer (must be set to 0 when first call)
 *
 * @return		true if found otherwise returns false
 *
 * @code
 *   Q_HASHTBL *tbl = qHashtbl();
 *   tbl->putStr(tbl, "key1", "hello world 1", false);
 *   tbl->putStr(tbl, "key2", "hello world 2", false);
 *   tbl->putStr(tbl, "key3", "hello world 3", false);
 *
 *   // non-thread usages
 *   int idx = 0;
 *   Q_NOBJ_T obj;
 *   while(tbl->getNext(tbl, &obj, &idx, false) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *   }
 *
 *   // thread model
 *   int idx = 0;
 *   Q_NOBJ_T obj;
 *   tbl->lock();
 *   while(tbl->getNext(tbl, &obj, &idx, false) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *   }
 *   tbl->unlock();
 *
 *   // thread model 2 with newmem flag
 *   int idx = 0;
 *   Q_NOBJ_T obj;
 *   tbl->lock();
 *   while(tbl->getNext(tbl, &obj, &idx, false) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *     free(obj.name);
 *     free(obj.data);
 *   }
 *   tbl->unlock();
 * @endcode
 */
static bool _getNext(Q_HASHTBL *tbl, Q_NOBJ_T *obj, int *idx, bool newmem) {
	if(tbl == NULL || obj == NULL || idx == NULL) return NULL;

	Q_MUTEX_ENTER(tbl->qmutex);
	bool found = false;
	for (; *idx < tbl->max; (*idx)++) {
		if (tbl->count[*idx] == 0) continue;

		if(newmem == true) {
			obj->name = strdup(tbl->obj[*idx].name);
			obj->data = malloc(tbl->obj[*idx].size);
			if(obj->name == NULL || obj->data == NULL) {
				DEBUG("Q_HASHTBL->getNext(): Unable to allocate memory.");
				if(obj->name != NULL) free(obj->name);
				if(obj->data != NULL) free(obj->data);
				break;
			}
			memcpy(obj->data, tbl->obj[*idx].data, tbl->obj[*idx].size);
			obj->size = tbl->obj[*idx].size;
		} else {
			obj->name = tbl->obj[*idx].name;
			obj->data = tbl->obj[*idx].data;
			obj->size = tbl->obj[*idx].size;
		}

		found = true;
		(*idx)++;
		break;
	}
	Q_MUTEX_LEAVE(tbl->qmutex);

	return found;
}

/**
 * Q_HASHTBL->remove(): Remove key from hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param name		key name
 *
 * @return		true if successful, otherwise(not found) returns false
 */
static bool _remove(Q_HASHTBL *tbl, const char *name) {
	if(tbl == NULL || name == NULL) return false;

	Q_MUTEX_ENTER(tbl->qmutex);

	int hash = (int)qHashFnv32(tbl->max, name, strlen(name));
	int idx = _getIdx(tbl, name, hash);
	if (idx < 0) {
		Q_MUTEX_LEAVE(tbl->qmutex);
		return false;
	}

	if (tbl->count[idx] == 1) {
		// just remove
		_removeData(tbl, idx);

		DEBUG("hashtbl: rem %s (idx=%d,tot=%d)", name, idx, tbl->num);
	} else if (tbl->count[idx] > 1) { // leading slot and has dup
		// find dup
		int idx2;
		for (idx2 = idx + 1; ; idx2++) {
			if (idx2 >= tbl->max) idx2 = 0;
			if (idx2 == idx) {
				DEBUG("hashtbl: BUG remove failed %s. dup not found.", name);
				Q_MUTEX_LEAVE(tbl->qmutex);
				return false;
			}
			if (tbl->count[idx2] == -1 && tbl->hash[idx2] == idx) break;
		}

		// move to leading slot
		int backupcount = tbl->count[idx];
		_removeData(tbl, idx); // remove leading slot
		if(_putData(tbl, idx, tbl->hash[idx2], &(tbl->obj[idx2]), backupcount - 1) == false) { // copy to leading slot
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}
		_removeData(tbl, idx2); // remove dup slot

		DEBUG("hashtbl: rem(lead) %s (idx=%d,tot=%d)", name, idx, tbl->num);
	} else { // in case of -1. used for collided data
		// decrease counter from leading slot
		if(tbl->count[tbl->hash[idx]] <= 1) {
			DEBUG("hashtbl: BUG remove failed %s. counter of leading slot mismatch.", name);
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}
		tbl->count[tbl->hash[idx]]--;

		// remove dup
		_removeData(tbl, idx);

		DEBUG("hashtbl: rem(dup) %s (idx=%d,tot=%d)", name, idx, tbl->num);
	}

	Q_MUTEX_LEAVE(tbl->qmutex);
	return true;
}

/**
 * Q_HASHTBL->getNum(): get number of objects stored
 *
 * @param tbl		a pointer of Q_HASHTBL
 *
 * @return		number of objects stored
 */
static int _getNum(Q_HASHTBL *tbl) {
	if(tbl == NULL) return 0;
	return tbl->num;
}

/**
 * Q_HASHTBL->getMax(): Get number of object slots
 *
 * @param tbl		a pointer of Q_HASHTBL
 *
 * @return		maximum number of object slots
 */
static int _getMax(Q_HASHTBL *tbl) {
	if(tbl == NULL) return 0;
	return tbl->max;
}

/**
 * Q_HASHTBL->truncate(): Truncate hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 *
 * @return		true if successful, otherwise returns false
 */
bool _truncate(Q_HASHTBL *tbl) {
	if(tbl == NULL) return false;

	Q_MUTEX_ENTER(tbl->qmutex);
	int idx;
	for (idx = 0; idx < tbl->max && tbl->num > 0; idx++) {
		if (tbl->count[idx] == 0) continue;
		free(tbl->obj[idx].name);
		free(tbl->obj[idx].data);
		tbl->count[idx] = 0;
		tbl->num--;
	}
	if(tbl->num != 0) {
		DEBUG("qHashtblTruncate: BUG DETECTED");
		tbl->num = 0;
	}
	Q_MUTEX_LEAVE(tbl->qmutex);

	return true;
}

/**
 * Q_HASHTBL->resize(): Resize dynamic-hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param max		a number of initial number of slots of Q_HASHTBL
 *
 * @return		true if successful, otherwise returns false
 *
 * @code
 *   // initial table size is 1000, enable resizing, use default threshold
 *   qHashtblResize(tbl, 3000);
 * @endcode
 *
 * @note
 * Auto resize flag and resize threshold parameter will be set to same as previous.
 */
bool _resize(Q_HASHTBL *tbl, int max) {
	if(max <= 0) return false;

	Q_MUTEX_ENTER(tbl->qmutex);

	if(max == tbl->max) { // don't need to resize, just set threshold
		Q_MUTEX_LEAVE(tbl->qmutex);
		return true;
	}

	// create new table
	Q_HASHTBL *newtbl = qHashtbl(max, false, 0);
	if(newtbl == NULL) {
		Q_MUTEX_LEAVE(tbl->qmutex);
		return false;
	}

	// copy entries
	int idx, num;
	for (idx = 0, num = 0; idx < tbl->max && num < tbl->num; idx++) {
		if (tbl->count[idx] == 0) continue;
		if(_put(newtbl, tbl->obj[idx].name, tbl->obj[idx].data, tbl->obj[idx].size) == false) {
			DEBUG("hashtbl: resize failed");
			_free(newtbl);
			Q_MUTEX_LEAVE(tbl->qmutex);
			return false;
		}
		num++;
	}

	if(tbl->num != newtbl->num) {
		DEBUG("hashtbl: BUG DETECTED");
		_free(newtbl);
		Q_MUTEX_LEAVE(tbl->qmutex);
		return false;
	}

	// set threshold
	if(tbl->threshold > 0) {
		_setThreshold(newtbl, max, tbl->threshold);
	}

	// de-allocate dynamic contents from the legacy table schema
	_truncate(tbl);
	if(tbl->count != NULL) free(tbl->count);
	if(tbl->hash != NULL) free(tbl->hash);
	if(tbl->obj != NULL) free(tbl->obj);

	// move contents of newtable schema then remove schema itself
	tbl->max = newtbl->max;
	tbl->num = newtbl->num;
	tbl->threshold = newtbl->threshold;
	tbl->resizeat = newtbl->resizeat;

	tbl->count = newtbl->count;
	tbl->hash = newtbl->hash;
	tbl->obj = newtbl->obj;

	Q_MUTEX_DESTROY(newtbl->qmutex);
	free(newtbl);

	Q_MUTEX_LEAVE(tbl->qmutex);
	return true;
}

/**
 * Q_HASHTBL->print(): Print hash table for debugging purpose
 *
 * @param tbl		a pointer of Q_HASHTBL
 * @param out		output stream
 * @param print_data	print out value if set to true
 *
 * @return		true if successful, otherwise returns false
 */
bool _print(Q_HASHTBL *tbl, FILE *out, bool print_data) {
	if(tbl == NULL || out == NULL) return false;

	Q_MUTEX_ENTER(tbl->qmutex);
	int idx, num;
	for (idx = 0, num = 0; idx < tbl->max && num < tbl->num; idx++) {
		if (tbl->count[idx] == 0) continue;
		fprintf(out, "%s=%s (idx=%d,hash=%d,size=%zu,count=%d)\n", tbl->obj[idx].name, (print_data)?(char*)tbl->obj[idx].data:"_binary_", idx, tbl->hash[idx], tbl->obj[idx].size, tbl->count[idx]);
		num++;
	}
	Q_MUTEX_LEAVE(tbl->qmutex);

	return true;
}

/**
 * Q_HASHTBL->free(): De-allocate hash table
 *
 * @param tbl		a pointer of Q_HASHTBL
 *
 * @return		true if successful, otherwise returns false
 */
bool _free(Q_HASHTBL *tbl) {
	if(tbl == NULL) return false;

	Q_MUTEX_ENTER(tbl->qmutex);
	_truncate(tbl);
	if(tbl->count != NULL) free(tbl->count);
	if(tbl->hash != NULL) free(tbl->hash);
	if(tbl->obj != NULL) free(tbl->obj);
	Q_MUTEX_LEAVE(tbl->qmutex);
	Q_MUTEX_DESTROY(tbl->qmutex);

	free(tbl);

	return true;
}

#ifndef _DOXYGEN_SKIP

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

static int _getIdx(Q_HASHTBL *tbl, const char *name, int hash) {
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
			if (!strcmp(tbl->obj[idx].name, name)) return idx;

			idx++;
			if (idx >= tbl->max) idx = 0;
			if(idx == hash) break;
		}
	}

	return -1;
}

static bool _putData(Q_HASHTBL *tbl, int idx, int hash, Q_NOBJ_T *obj, int count) {
	// check if used
	if(tbl->count[idx] != 0) return false;

	// store
	tbl->hash[idx] = hash;
	tbl->obj[idx].name = strdup(obj->name);
	tbl->obj[idx].data = malloc(obj->size);
	if(tbl->obj[idx].name == NULL || tbl->obj[idx].data == NULL) {
		if(tbl->obj[idx].name != NULL) free(tbl->obj[idx].name);
		if(tbl->obj[idx].data != NULL) free(tbl->obj[idx].data);
		return false;
	}
	memcpy(tbl->obj[idx].data, obj->data, obj->size);
	tbl->obj[idx].size = obj->size;

	tbl->count[idx] = count;

	// increase used counter
	tbl->num++;

	return true;
}

static bool _removeData(Q_HASHTBL *tbl, int idx) {
	if(tbl->count[idx] == 0) return false;

	if(tbl->obj[idx].name != NULL) free(tbl->obj[idx].name);
	if(tbl->obj[idx].data != NULL) free(tbl->obj[idx].data);
	tbl->count[idx] = 0;

	// decrease used counter
	tbl->num--;

	return true;
}

static bool _setThreshold(Q_HASHTBL *tbl, int max, int threshold) {
	if(threshold <= 0 || threshold > 100) return false;

	int num = ((max * threshold) / 100);
	if(num >= max) num = max - 1;

	tbl->threshold = threshold;
	tbl->resizeat = num;

	return true;
}

#endif /* _DOXYGEN_SKIP */

#endif /* DISABLE_DATASTRUCTURE */
