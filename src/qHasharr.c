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
 * @file qHasharr.c Array based Hash-table Data Structure API
 *
 * @note
 * This Aarray-hash-table does not support thread-safe feature because this table is designed for statically assigned array like shared memory
 * and most of this cases use fork() & semaphore based program model.
 * In case of multiple access of this table, you should provide some kind of lock mechanism to prevent multiple modification of table at same time.
 * qDecoder also have thread-safe Dynamic Hash-table, please refer qHashtbl().
 *
 * In this array hash-table, we use some technics to effectively use memory. To verify key we use two way,
 * if the key is smaller than (_Q_HASHARR_MAX_KEYSIZE - 1), we compare key itself. But if the key is bigger than
 * (_Q_HASHARR_MAX_KEYSIZE - 1), we compare md5 of key and key length. If the key length and md5 of key are same
 * we consider it's same key. So we don't need to store full key string. Actually it's not necessary to keep
 * original key string, but we keep this because of two reasons. 1) if the length of the key is smaller than 16,
 * it will be little bit quicker to compare key. 2) debugging reason.
 *
 * Basically this hash-table based on array defines small size slot then it can links several slot for one data.
 * This mechanism can save some wastes of memory. You can adjust default slot size to modify _Q_HASHARR_DEF_VALUESIZE.
 *
 * @code
 *   // generally allocate more of actual maximum slots to avoid heavy hash collisions
 *   int maxslots = 1000;
 *
 *   // calculate how many memory do we need
 *   size_t memsize = qHasharrSize(maxslots);
 *
 *   // allocate memory
 *   void *memory = malloc(memsize);
 *   if(memory == NULL) return -1;
 *
 *   // initialize hash-table
 *   Q_HASHARR *hasharr = qHasharr(memory, memsize);
 *   if(hasharr == NULL) {
 *     free(memory);
 *     return -1;
 *   }
 *
 *   // put some sample data
 *   hasharr->put(hasharr, "sample1", "binary", 6);
 *   hasharr->putStr(hasharr, "sample2", "string");
 *   hasharr->putInt(hasharr, "sample3", 3);
 *
 *   // fetch data
 *   size_t size;
 *   char *sample_bin = hasharr->get(hasharr, "sample1", &size);
 *   char *sample_str = hasharr->getStr(hasharr, "sample2");
 *   int  sample_int  = hasharr->getInt(hasharr, "sample3");
 *   free(sample_bin); // returned data must be released by user after use.
 *   free(sample_str);
 *
 *   // debug output
 *    hasharr->print(hasharr, stdout);
 *
 *   // release table
 *   free(memory);
 *
 * @endcode
 *
 * Another simple way to initialize hash-table.
 *
 * @code
 *   // define data memory as much as you needed.
 *   char datamem[10 * 1024];
 *
 *   // initialize hash-table
 *   Q_HASHARR *hasharr = qHasharr(datamem, sizeof(datamem));
 *   if(hasharr == NULL) return -1;
 *
 *   (...your codes here...)
 *
 *   // no need to free unless you use malloc()
 * @endcode
 *
 * You can create hash table on shared memory like below.
 *
 * @code
 *   int maxslots = 1000;
 *   int memsize = qHasharrSize(maxslots);
 *
 *   // create shared memory
 *   int shmid = qShmInit(g_conf.szEgisdavdPidfile, 's', memsize, true);
 *   if(shmid < 0) return -1; // creation failed
 *   void *memory = qShmGet(shmid);
 *
 *   // initialize hash-table
 *   Q_HASHARR *hasharr = qHasharr(memory, memsize);
 *   if(hasharr == NULL) return -1;
 *
 *   (...your codes here...)
 *
 *   // destroy shared memory
 *   qShmFree(shmid);
 * @endcode
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

#ifndef _DOXYGEN_SKIP

static bool	_put(Q_HASHARR *tbl, const char *key, const void *value, size_t size);
static bool	_putStr(Q_HASHARR *tbl, const char *key, const char *str);
static bool	_putInt(Q_HASHARR *tbl, const char *key, int num);

static void*	_get(Q_HASHARR *tbl, const char *key, size_t *size);
static char*	_getStr(Q_HASHARR *tbl, const char *key);
static int	_getInt(Q_HASHARR *tbl, const char *key);

static const char* _getNext(Q_HASHARR *tbl, int *idx);
static bool	_remove(Q_HASHARR *tbl, const char *key);
static int	_getNum(Q_HASHARR *tbl);
static int	_getMaxSlots(Q_HASHARR *tbl);
static int	_getUsedSlots(Q_HASHARR *tbl);
static bool	_truncate(Q_HASHARR *tbl);

static bool	_print(Q_HASHARR *tbl, FILE *out);

// internal usages
static int	_findEmpty(Q_HASHARR *tbl, int startidx);
static int	_getIdx(Q_HASHARR *tbl, const char *key, unsigned int hash);
static bool	_putData(Q_HASHARR *tbl, int idx, unsigned int hash, const char *key, const void *value, size_t size, int count);
static bool	_copySlot(Q_HASHARR *tbl, int idx1, int idx2);
static bool	_removeSlot(Q_HASHARR *tbl, int idx);
static bool	_removeData(Q_HASHARR *tbl, int idx);

#endif

/**
 * Get how much memory is needed for N slots.
 *
 * @param max		a number of maximum internal slots
 *
 * @return		memory size needed
 *
 * @note
 * This is useful when you decide how much memory(shared-memory) should be allocated for N slots.
 * Be sure, a single object can be stored in several slots if it exceeds _Q_HASHARR_DEF_VALUESIZE.
 */
size_t qHasharrSize(int max) {
	size_t memsize = sizeof(Q_HASHARR) + (sizeof(struct _Q_HASHARR_SLOT) * (max));
	return memsize;
}

/**
 * Initialize static hash table
 *
 * @param memory	a pointer of buffer memory.
 * @param memsize	a size of buffer memory.
 *
 * @return		a pointer of Q_HASHARR structure(same as buffer pointer), otherwise returns NULL.
 *
 * @code
 *   // calculate how many memory do we need
 *   size_t memsize = qHasharrSize(1000);
 *
 *   // allocate memory
 *   void *memory = malloc(memsize);
 *   if(memory == NULL) return -1;
 *
 *   // initialize hash-table
 *   Q_HASHARR *hasharr = qHasharr(memory, memsize);
 *   if(hasharr == NULL) {
 *     free(memory);
 *     return -1;
 *   }
 *
 *   // release resources
 *   free(memory);
 * @endcode
 */
Q_HASHARR *qHasharr(void *memory, size_t memsize) {
	// calculate max
	int maxslots = (memsize - sizeof(Q_HASHARR)) / sizeof(struct _Q_HASHARR_SLOT);
	if(maxslots < 1 || memsize <= sizeof(Q_HASHARR)) return NULL;

	// clear memory
	Q_HASHARR *tbl = (Q_HASHARR*)memory;
	memset((void *)tbl, 0, memsize);

	tbl->maxslots = maxslots;
	tbl->usedslots = 0;
	tbl->num = 0;

	tbl->slots = (struct _Q_HASHARR_SLOT*)(memory + sizeof(Q_HASHARR)); // data slot pointer

	// assign methods
	tbl->put		= _put;
	tbl->putStr		= _putStr;
	tbl->putInt		= _putInt;

	tbl->get		= _get;
	tbl->getStr		= _getStr;
	tbl->getInt		= _getInt;
	tbl->getNext		= _getNext;

	tbl->remove		= _remove;
	tbl->getNum		= _getNum;
	tbl->getMaxSlots	= _getMaxSlots;
	tbl->getUsedSlots	= _getUsedSlots;
	tbl->truncate		= _truncate;

	tbl->print		= _print;

	return (Q_HASHARR*)memory;
}

/**
 * Q_HASHARR->put(): Put object into hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 * @param value		value object data
 * @param size		size of value
 *
 * @return		true if successful, otherwise returns false
 */
static bool _put(Q_HASHARR *tbl, const char *key, const void *value, size_t size) {
	if(tbl == NULL || key == NULL || value == NULL) return false;

	// check full
	if (tbl->usedslots >= tbl->maxslots) {
		DEBUG("hasharr: put %s - FULL", key);
		return false;
	}

	// get hash integer
	unsigned int hash = qHashFnv32(tbl->maxslots, key, strlen(key));

	// check, is slot empty
	if (tbl->slots[hash].count == 0) { // empty slot
		// put data
		if(_putData(tbl, hash, hash, key, value, size, 1) == false) {
			DEBUG("hasharr: FAILED put(new) %s", key);
			return false;
		}
		DEBUG("hasharr: put(new) %s (idx=%d,hash=%u,tot=%d)", key, hash, hash, tbl->usedslots);
	} else if (tbl->slots[hash].count > 0) { // same key exists or hash collision
		// check same key;
		int idx = _getIdx(tbl, key, hash);
		if (idx >= 0) { // same key
			// remove and recall
			_remove(tbl, key);
			return _put(tbl, key, value, size);
		} else { // no same key, just hash collision
			// find empty slot
			int idx = _findEmpty(tbl, hash);
			if (idx < 0) return false;

			// put data. -1 is used for collision resolution (idx != hash);
			if(_putData(tbl, idx, hash, key, value, size, -1) == false) {
				DEBUG("hasharr: FAILED put(col) %s", key);
				return false;
			}

			// increase counter from leading slot
			tbl->slots[hash].count++;

			DEBUG("hasharr: put(col) %s (idx=%d,hash=%u,tot=%d)", key, idx, hash, tbl->usedslots);
		}
	} else { // in case of -1 or -2. used for collision resolution or oversized value data, move it

		// find empty slot
		int idx = _findEmpty(tbl, hash + 1);
		if (idx < 0) {
			return false;
		}

		// move dup slot to empty
		_copySlot(tbl, idx, hash);
		_removeSlot(tbl, hash);

		// in case of -2, adjust link of mother
		if(tbl->slots[idx].count == -2) {
			tbl->slots[ tbl->slots[idx].hash ].link = idx;
		}

		// store data
		if(_putData(tbl, hash, hash, key, value, size, 1) == false) {
			DEBUG("hasharr: FAILED put(swp) %s", key);
			return false;
		}

		DEBUG("hasharr: put(swp) %s (idx=%u,hash=%u,tot=%d)", key, hash, hash, tbl->usedslots);
	}

	return true;
}

/**
 * Q_HASHARR->putStr(): Put string into hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 * @param value		value string
 *
 * @return		true if successful, otherwise returns false
 */
static bool _putStr(Q_HASHARR *tbl, const char *key, const char *str) {
	int size = (str != NULL) ? (strlen(str) + 1) : 0;
	return _put(tbl, key, (void *)str, size);
}

/**
 * Q_HASHARR->putInt(): Put integer into hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 * @param value		value integer
 *
 * @return		true if successful, otherwise returns false
 */
static bool _putInt(Q_HASHARR *tbl, const char *key, int num) {
	char data[10+1];
	sprintf(data, "%d", num);
	return _put(tbl, key, (void *)data, (strlen(data) + 1));
}

/**
 * Q_HASHARR->get(): Get object from hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 * @param size		if not NULL, oject size will be stored
 *
 * @return		malloced object pointer if successful, otherwise(not found) returns NULL
 *
 * @note
 * returned object must be freed after done using.
 */
static void *_get(Q_HASHARR *tbl, const char *key, size_t *size) {
	if(tbl == NULL || key == NULL) return NULL;

	// get hash integer
	unsigned int hash = qHashFnv32(tbl->maxslots, key, strlen(key));

	int idx = _getIdx(tbl, key, hash);
	if (idx < 0) return NULL;

	int newidx;
	size_t valsize;
	for(newidx = idx, valsize = 0; ; newidx = tbl->slots[newidx].link) {
		valsize += tbl->slots[newidx].size;
		if(tbl->slots[newidx].link == -1) break;
	}

	void *value, *vp;
	value = malloc(valsize);
	if(value == NULL) return NULL;

	for(newidx = idx, vp = value; ; newidx = tbl->slots[newidx].link) {
		memcpy(vp, (void *)tbl->slots[newidx].value, tbl->slots[newidx].size);
		vp += tbl->slots[newidx].size;
		if(tbl->slots[newidx].link == -1) break;
	}

	if(size != NULL) *size = valsize;
	return value;
}

/**
 * Q_HASHARR->getStr(): Get string from hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 *
 * @return		string pointer if successful, otherwise(not found) returns NULL
 *
 * @note
 * returned object must be freed after done using.
 */
static char *_getStr(Q_HASHARR *tbl, const char *key) {
	return (char*)_get(tbl, key, NULL);
}

/**
 * Q_HASHARR->getInt(): Get integer from hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 *
 * @return		value integer if successful, otherwise(not found) returns 0
 */
static int _getInt(Q_HASHARR *tbl, const char *key) {
	char *data = _get(tbl, key, NULL);
	if(data == NULL) return 0;

	int value = atoi(data);
	free(data);

	return value;
}

/**
 * Q_HASHARR->getNext(): Get next key name
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param idx		index pointer
 *
 * @return		key name string if successful, otherwise(end of table) returns NULL
 *
 * @code
 *   int idx = 0;
 *   while(tbl->getNext(tbl, &idx) != NULL) {
 *     (... codes ...)
 *   }
 * @endcode
 *
 * @note
 * Do not free returned key string.
 */
static const char *_getNext(Q_HASHARR *tbl, int *idx) {
	if(tbl == NULL || idx == NULL) return NULL;

	for (*idx += 1; *idx <= tbl->maxslots; (*idx)++) {
		if (tbl->slots[*idx].count == 0 || tbl->slots[*idx].count == -2) continue;
		return tbl->slots[*idx].key;
	}

	*idx = tbl->maxslots; // set to lastest key number
	return NULL;
}

/**
 * Q_HASHARR->remove(): Remove key from hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param key		key string
 *
 * @return		true if successful, otherwise(not found) returns false
 */
static bool _remove(Q_HASHARR *tbl, const char *key) {
	if(tbl == NULL || key == NULL) return false;

	// get hash integer
	unsigned int hash = qHashFnv32(tbl->maxslots, key, strlen(key));

	int idx = _getIdx(tbl, key, hash);
	if (idx < 0) {
		DEBUG("not found %s", key);
		return false;
	}

	if (tbl->slots[idx].count == 1) {
		// just remove
		_removeData(tbl, idx);

		DEBUG("hasharr: rem %s (idx=%d,tot=%d)", key, idx, tbl->usedslots);
	} else if (tbl->slots[idx].count > 1) { // leading slot and has dup
		// find dup
		int idx2;
		for (idx2 = idx + 1; ; idx2++) {
			if (idx2 >= tbl->maxslots) idx2 = 0;
			if (idx2 == idx) {
				DEBUG("BUG: failed to remove dup key %s.", key);
				return false;
			}
			if (tbl->slots[idx2].count == -1 && tbl->slots[idx2].hash == idx) break;
		}

		// move to leading slot
		int backupcount = tbl->slots[idx].count;
		_removeData(tbl, idx); // remove leading data
		_copySlot(tbl, idx, idx2); // copy slot
		tbl->slots[idx].count = backupcount - 1; // adjust collision counter
		_removeSlot(tbl, idx2); // remove moved slot

		DEBUG("hasharr: rem(lead) %s (idx=%d,tot=%d)", key, idx, tbl->usedslots);
	} else { // in case of -1. used for collision resolution
		// decrease counter from leading slot
		if(tbl->slots[ tbl->slots[idx].hash ].count <= 1) {
			DEBUG("hasharr: BUG remove failed %s. counter of leading slot mismatch.", key);
			return false;
		}
		tbl->slots[ tbl->slots[idx].hash ].count--;

		// remove data
		_removeData(tbl, idx);

		DEBUG("hasharr: rem(dup) %s (idx=%d,tot=%d)", key, idx, tbl->usedslots);
	}

	return true;
}

/**
 * Q_HASHARR->getNum(): Get a number of keys stored
 *
 * @param tbl		a pointer of Q_HASHARR
 *
 * @return		a number of key stored
 */
static int _getNum(Q_HASHARR *tbl) {
	if(tbl == NULL) return false;

	return tbl->usedslots;
}

/**
 * Q_HASHARR->getMaxSlots(): Get a number of maximum slots
 *
 * @param tbl		a pointer of Q_HASHARR
 *
 * @return		a number of maximum slots
 *
 * @note
 * It is different with dynamic hash-table. In array-hash table, a value object can be stored
 * across several slots if the size of the object is bigger than the size of slot. So used and max
 * slot number is not equal to actual stored and maximum storable objects number.
 */
static int _getMaxSlots(Q_HASHARR *tbl) {
	if(tbl == NULL) return false;

	return tbl->maxslots;
}

/**
 * Q_HASHARR->getUsedSlots(): Get a number of used slots
 *
 * @param tbl		a pointer of Q_HASHARR
 *
 * @return		a number of used slots
 */

static int _getUsedSlots(Q_HASHARR *tbl) {
	if(tbl == NULL) return false;

	return tbl->usedslots;
}

/**
 * Q_HASHARR->truncate(): Truncate array-hash table
 *
 * @param tbl		a pointer of Q_HASHARR
 *
 * @return		true if successful, otherwise returns false
 */
static bool _truncate(Q_HASHARR *tbl) {
	if(tbl->usedslots == 0) return true;

	tbl->usedslots = 0;
	tbl->num = 0;

	// clear memory
	memset((void *)tbl->slots, 0, (tbl->maxslots * sizeof(struct _Q_HASHARR_SLOT)));

	return true;
}

/**
 * Q_HASHARR->print(): Print hash table for debugging purpose
 *
 * @param tbl		a pointer of Q_HASHARR
 * @param out		output stream
 *
 * @return		true if successful, otherwise returns false
 */
static bool _print(Q_HASHARR *tbl, FILE *out) {
	if(tbl == NULL || out == NULL) return false;

	int idx, num;
	for (idx = 0, num = 0; idx < tbl->maxslots && num < tbl->usedslots; idx++) {
		if (tbl->slots[idx].count == 0) continue;
		fprintf(out, "idx=%d,count=%d,hash=%u,key=%s,keylen=%zu,size=%zu,link=%d\n",
			idx, tbl->slots[idx].count, tbl->slots[idx].hash, tbl->slots[idx].key, tbl->slots[idx].keylen, tbl->slots[idx].size, tbl->slots[idx].link);
		num++;
	}

	return true;
}

#ifndef _DOXYGEN_SKIP

// find empty slot : return empty slow number, otherwise returns -1.
static int _findEmpty(Q_HASHARR *tbl, int startidx) {
	if(startidx >= tbl->maxslots) startidx = 0;

	int idx = startidx;
	while (true) {
		if (tbl->slots[idx].count == 0) return idx;

		idx++;
		if(idx >= tbl->maxslots) idx = 0;
		if(idx == startidx) break;
	}

	return -1;
}

static int _getIdx(Q_HASHARR *tbl, const char *key, unsigned int hash) {
	if (tbl->slots[hash].count > 0) {
		size_t keylen = strlen(key);
		unsigned char keymd5[16];

		unsigned char *tmpmd5 = qHashMd5(key, keylen);
		memcpy((char*)keymd5, (char*)tmpmd5, 16);
		free(tmpmd5);

		int count, idx;
		for (count = 0, idx = hash; count < tbl->slots[hash].count; ) {
			// find same hash
			while(true) {
				if (idx >= tbl->maxslots) idx = 0;

				if ((tbl->slots[idx].count > 0 || tbl->slots[idx].count == -1) && tbl->slots[idx].hash == hash) {
					// found same hash
					count++;
					break;
				}

				idx++;
				if(idx == hash) return -1;
			}

			// is same key?
			if (keylen == tbl->slots[idx].keylen) {	// first check fast way
				if (keylen <= (_Q_HASHARR_MAX_KEYSIZE - 1)) {
					// key is not truncated, use original key
					if (!strcmp(key, tbl->slots[idx].key)) return idx;
				} else {
					// key is truncated, use keymd5 instead.
					if (!memcmp(keymd5, tbl->slots[idx].keymd5, 16)) return idx;
				}
			}

			// increase idx
			idx++;
			if(idx >= tbl->maxslots) idx = 0;

			// check loop
			if(idx == hash) break;
		}
	}

	return -1;
}

static bool _putData(Q_HASHARR *tbl, int idx, unsigned int hash, const char *key, const void *value, size_t size, int count) {
	// check if used
	if(tbl->slots[idx].count != 0) {
		DEBUG("hasharr: BUG found.");
		return false;
	}

	size_t keylen = strlen(key);
	unsigned char *keymd5 = qHashMd5(key, keylen);

	// store key
	tbl->slots[idx].count = count;
	tbl->slots[idx].hash = hash;
	qStrCpy(tbl->slots[idx].key, sizeof(tbl->slots[idx].key), key);
	memcpy((char*)tbl->slots[idx].keymd5, (char*)keymd5, 16);
	tbl->slots[idx].keylen = keylen;
	tbl->slots[idx].link = -1;

	free(keymd5);

	// store value
	int newidx;
	size_t savesize, copysize;
	for (newidx = idx, savesize = 0, copysize = 0; savesize < size; savesize += copysize) {
		copysize = size - savesize;
		if(copysize > _Q_HASHARR_DEF_VALUESIZE) copysize = _Q_HASHARR_DEF_VALUESIZE;

		if(savesize > 0) { // find next empty slot
			int tmpidx = _findEmpty(tbl, newidx + 1);

			tmpidx = _findEmpty(tbl, newidx + 1);
			if(tmpidx < 0) {
				DEBUG("hasharr: Can't expand slot for key %s.", key);
				_removeData(tbl, idx);
				return false;
			}

			// clear & set
			memset((void *)(&tbl->slots[tmpidx]), 0, sizeof(Q_HASHARR));

			tbl->slots[tmpidx].count = -2;
			tbl->slots[tmpidx].hash = newidx; // prev link
			tbl->slots[tmpidx].link = -1;
			tbl->slots[tmpidx].size = 0;

			tbl->slots[newidx].link = tmpidx; // link chain

			DEBUG("hasharr: slot %d is linked to slot %d for key %s.", tmpidx, newidx, key);
			newidx = tmpidx;
		}

		memcpy(tbl->slots[newidx].value, value + savesize, copysize);
		tbl->slots[newidx].size = copysize;

		// increase used slot counter
		tbl->usedslots++;
	}

	// increase stored key counter
	tbl->num++;

	return true;
}

static bool _copySlot(Q_HASHARR *tbl, int idx1, int idx2) {
	if(tbl->slots[idx1].count != 0 || tbl->slots[idx2].count == 0) {
		DEBUG("hasharr: BUG found.");
		return false;
	}

	memcpy((void *)(&tbl->slots[idx1]), (void *)(&tbl->slots[idx2]), sizeof(struct _Q_HASHARR_SLOT));

	// increase used slot counter
	tbl->usedslots++;

	return true;
}

static bool _removeSlot(Q_HASHARR *tbl, int idx) {
	if(tbl->slots[idx].count == 0) {
		DEBUG("hasharr: BUG found.");
		return false;
	}

	tbl->slots[idx].count = 0;

	// decrease used slot counter
	tbl->usedslots--;

	return true;
}

static bool _removeData(Q_HASHARR *tbl, int idx) {
	if(tbl->slots[idx].count == 0) {
		DEBUG("hasharr: BUG found.");
		return false;
	}

	while(true) {
		int link = tbl->slots[idx].link;
		_removeSlot(tbl, idx);

		if(link == -1) {
			// decrease stored key counter
			tbl->num--;
			break;
		}
		idx = link;
	}

	return true;
}

#endif /* _DOXYGEN_SKIP */

#endif /* DISABLE_DATASTRUCTURE */
