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
 * @file qEntry.c Linked-list Data Structure API
 *
 * @code
 *   [Code sample - String]
*
 *   // sample data
 *   struct MY_OBJ *my_obj = getNewMyOjb(); // sample object
 *   char *my_str = "hello"; // sample string
 *   int my_int = 1; // sample integer
 *
 *   // store into linked-list
 *   Q_ENTRY *entry = qEntry();
 *   entries = entry->put(entry, "obj", (void*)my_obj, sizeof(struct MY_OBJ), true);
 *   entries = entry->putStr(entry, "obj", my_str, true);
 *   entries = entry->putInt(entry, "obj", my_int, true);
 *
 *   // print out
 *   entry->print(entry, stdout, false);
 *
 *   // free object
 *   entry->free(entry);
 *
 *   [Result]
 * @endcode
 *
 * @note
 * Use "--enable-threadsafe" configure script option to use under multi-threaded environments.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "qDecoder.h"
#include "qInternal.h"

/*
 * Member method protos
 */
#ifndef _DOXYGEN_SKIP

#define _VAR			'$'
#define _VAR_OPEN		'{'
#define _VAR_CLOSE		'}'
#define _VAR_CMD		'!'
#define _VAR_ENV		'%'

static	void		_lock(Q_ENTRY *entry);
static	void		_unlock(Q_ENTRY *entry);

static bool		_put(Q_ENTRY *entry, const char *name, const void *data, size_t size, bool replace);
static bool		_putStr(Q_ENTRY *entry, const char *name, const char *str, bool replace);
static bool		_putStrParsed(Q_ENTRY *entry, const char *name, const char *str, bool replace);
static bool		_putInt(Q_ENTRY *entry, const char *name, int num, bool replace);

static void*		_get(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
static void*		_getCase(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
static void*		_getLast(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
static bool		_getNext(Q_ENTRY *entry, Q_NLOBJ_T *obj, const char *name, bool newmem);

static char*		_getStr(Q_ENTRY *entry, const char *name, bool newmem);
static char*		_getStrCase(Q_ENTRY *entry, const char *name, bool newmem);
static char*		_getStrLast(Q_ENTRY *entry, const char *name, bool newmem);
static int		_getInt(Q_ENTRY *entry, const char *name);
static int		_getIntCase(Q_ENTRY *entry, const char *name);
static int 		_getIntLast(Q_ENTRY *entry, const char *name);

static int		_remove(Q_ENTRY *entry, const char *name);

static int 		_getNum(Q_ENTRY *entry);
static int		_getNo(Q_ENTRY *entry, const char *name);
static char*		_parseStr(Q_ENTRY *entry, const char *str);
static void*		_merge(Q_ENTRY *entry, size_t *size);

static bool		_save(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode);
static int		_load(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode);
static bool		_reverse(Q_ENTRY *entry);
static bool		_print(Q_ENTRY *entry, FILE *out, bool print_data);
static bool		_free(Q_ENTRY *entry);

#endif

/**
 * Create new Q_ENTRY linked-list object
 *
 * @return	a pointer of malloced Q_ENTRY structure in case of successful, otherwise returns NULL.
 *
 * @code
 *   Q_ENTRY *entry = qEntry();
 * @endcode
 */
Q_ENTRY *qEntry(void) {
	Q_ENTRY *entry = (Q_ENTRY *)malloc(sizeof(Q_ENTRY));
	if(entry == NULL) return NULL;

	memset((void *)entry, 0, sizeof(Q_ENTRY));

	/* initialize non-recrusive mutex */
	Q_LOCK_INIT(entry->qlock, false);

	/* member methods */
	entry->lock		= _lock;
	entry->unlock		= _unlock;

	entry->put		= _put;
	entry->putStr		= _putStr;
	entry->putStrParsed	= _putStrParsed;
	entry->putInt		= _putInt;

	entry->get		= _get;
	entry->getCase		= _getCase;
	entry->getLast		= _getLast;
	entry->getNext		= _getNext;

	entry->getStr		= _getStr;
	entry->getStrCase	= _getStrCase;
	entry->getStrLast	= _getStrLast;
	entry->getInt		= _getInt;
	entry->getIntCase	= _getIntCase;
	entry->getIntLast	= _getIntLast;

	entry->remove		= _remove;

	entry->getNum		= _getNum;
	entry->getNo		= _getNo;
	entry->parseStr		= _parseStr;
	entry->merge		= _merge;

	entry->save		= _save;
	entry->load		= _load;
	entry->reverse		= _reverse;
	entry->print		= _print;
	entry->free		= _free;

	return entry;
}

/**
 * Q_entry->qlock(): Enter critical section.
 *
 * @note
 * Q_ENTRY uses recursive mutex lock mechanism. And it uses lock at least as possible.
 * User can raise lock to proctect data modification during Q_ENTRY->getNext() operation.
 */
static void _lock(Q_ENTRY *entry) {
	Q_LOCK_ENTER(entry->qlock);
}

/**
 * Q_ENTRY->unlock(): Leave critical section.
 */
static void _unlock(Q_ENTRY *entry) {
	Q_LOCK_LEAVE(entry->qlock);
}

/**
 * Q_ENTRY->put(): Store object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param object	object pointer
 * @param size		size of the object
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
static bool _put(Q_ENTRY *entry, const char *name, const void *data, size_t size, bool replace) {
	/* check arguments */
	if(entry == NULL || name == NULL || data == NULL || size < 0) return false;

	/* duplicate name */
	char *dup_name = strdup(name);
	if(dup_name == NULL) return false;

	/* duplicate object */
	void *dup_data = (size>0?malloc(size):strdup(""));
	if(dup_data == NULL) {
		free(dup_name);
		return false;
	}
	memcpy(dup_data, data, size);

	/* make new object entry */
	Q_NLOBJ_T *obj = (Q_NLOBJ_T*)malloc(sizeof(Q_NLOBJ_T));
	if(obj == NULL) {
		free(dup_name);
		free(dup_data);
		return false;
	}
	obj->name = dup_name;
	obj->data = dup_data;
	obj->size = size;
	obj->next = NULL;

	Q_LOCK_ENTER(entry->qlock);

	/* if replace flag is set, remove same key */
	if (replace == true) _remove(entry, dup_name);

	/* make chain link */
	if(entry->first == NULL) entry->first = entry->last = obj;
	else {
		entry->last->next = obj;
		entry->last = obj;
	}

	entry->size += size;
	entry->num++;

	Q_LOCK_LEAVE(entry->qlock);

	return true;
}

/**
 * Q_ENTRY->putStr(): Add string object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param str		string value
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
static bool _putStr(Q_ENTRY *entry, const char *name, const char *str, bool replace) {
	size_t size = (str!=NULL) ? (strlen(str) + 1) : 0;
	return _put(entry, name, (const void *)str, size, replace);
}

/**
 * Q_ENTRY->putStrParsed(): Add string object with parsed into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param str		string value which may contain variables like ${...}
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 *
 * @code
 *   ${key_name}		variable replacement
 *   ${!system_command}		external command results
 *   ${%PATH}			get environments
 * @endcode
 *
 * @code
 *   entry->putStrParsed(entry, "BASE", "/usr/local", true);
 *   entry->putStrParsed(entry, "BIN", "${BASE}/bin", true);
 *   entry->putStrParsed(entry, "HOSTNAME", "${!/bin/hostname -s}", true);
 *   entry->putStrParsed(entry, "USER", "${%USER}", true);
 * @endcode
 */
static bool _putStrParsed(Q_ENTRY *entry, const char *name, const char *str, bool replace) {
	char *newstr = _parseStr(entry, str);
	size_t size = (newstr!=NULL) ? (strlen(newstr) + 1) : 0;
	bool ret = _put(entry, name, (const void *)newstr, size, replace);
	if(newstr != NULL) free(newstr);
	return ret;
}

/**
 * Q_ENTRY->putInt(): Add integer object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param num		number value
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
static bool _putInt(Q_ENTRY *entry, const char *name, int num, bool replace) {
	char str[10+1];
	sprintf(str, "%d", num);
	return _put(entry, name, (void *)str, strlen(str) + 1, replace);
}

/**
 * Q_ENTRY->get(): Find object with given name
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of data if key is found, otherwise returns NULL.
 *
 * @code
 *   Q_ENTRY *entry = qEntry();
 *   (...codes...)
 *
 *   // with newmem flag unset
 *   size_t size;
 *   const char *data = entry->get(entry, "key_name", &size, false);
 *
 *   // with newmem flag set
 *   size_t size;
 *   char *data = entry->get(entry, "key_name", &size, true);
 *   if(data != NULL) free(data);
 * @endcode
 *
 * @note
 * If newmem flag is set, returned data will be malloced and should be deallocated by user.
 * Otherwise returned pointer will point internal buffer directly and should not be de-allocated by user.
 * In thread-safe mode, newmem flag always should be true.
 */
static void *_get(Q_ENTRY *entry, const char *name, size_t *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

	Q_LOCK_ENTER(entry->qlock);
	void *data = NULL;
	Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcmp(obj->name, name)) {
			if(size != NULL) *size = obj->size;

			if(newmem == true) {
				data = malloc(obj->size);
				memcpy(data, obj->data, obj->size);
			} else {
				data = obj->data;
			}

			break;
		}
	}
	Q_LOCK_LEAVE(entry->qlock);

	return data;
}

/**
 * Q_ENTRY->getCase(): Find object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 */
static void *_getCase(Q_ENTRY *entry, const char *name, size_t *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

	Q_LOCK_ENTER(entry->qlock);
	void *data = NULL;
	Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcasecmp(name, obj->name)) {
			if(size != NULL) *size = obj->size;
			if(newmem == true) {
				data = malloc(obj->size);
				memcpy(data, obj->data, obj->size);
			} else {
				data = obj->data;
			}

			break;
		}
	}
	Q_LOCK_LEAVE(entry->qlock);

	return data;
}

/**
 * Q_ENTRY->getLast(): Find lastest matched object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 *
 * @note
 * If you have multiple objects with same name. this method can be used to
 * find out lastest matched object.
 */
static void *_getLast(Q_ENTRY *entry, const char *name, size_t *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

	Q_LOCK_ENTER(entry->qlock);

	Q_NLOBJ_T *lastobj = NULL;
	Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if (!strcmp(name, obj->name)) lastobj = obj;
	}

	void *data = NULL;
	if(lastobj != NULL) {
		if(size != NULL) *size = lastobj->size;
		if(newmem == true) {
			data = malloc(lastobj->size);
			memcpy(data, lastobj->data, lastobj->size);
		} else {
			data = lastobj->data;
		}
	}

	Q_LOCK_LEAVE(entry->qlock);
	return data;
}

/**
 * Q_ENTRY->getStr(): Find string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 */
static char *_getStr(Q_ENTRY *entry, const char *name, bool newmem) {
	return (char *)_get(entry, name, NULL, newmem);
}

/**
 * Q_ENTRY->getStrCase(): Find string object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 */
static char *_getStrCase(Q_ENTRY *entry, const char *name, bool newmem) {
	return (char *)_getCase(entry, name, NULL, newmem);
}

/**
 * Q_ENTRY->getStrLast(): Find lastest matched string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 */
static char *_getStrLast(Q_ENTRY *entry, const char *name, bool newmem) {
	return (char *)_getLast(entry, name, NULL, newmem);
}

/**
 * Q_ENTRY->getInt(): Find integer object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the integer object, otherwise returns 0.
 */
static int _getInt(Q_ENTRY *entry, const char *name) {
	char *str = _get(entry, name, NULL, true);
	int n = 0;
	if(str != NULL) {
		n = atoi(str);
		free(str);
	}
	return n;
}

/**
 * Q_ENTRY->getIntCase(): Find integer object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
static int _getIntCase(Q_ENTRY *entry, const char *name) {
	char *str =_getCase(entry, name, NULL, true);
	int n = 0;
	if(str != NULL) {
		n = atoi(str);
		free(str);
	}
	return n;
}

/**
 * Q_ENTRY->getIntLast(): Find lastest matched integer object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
static int _getIntLast(Q_ENTRY *entry, const char *name) {
	char *str =_getLast(entry, name, NULL, true);
	int n = 0;
	if(str != NULL) {
		n = atoi(str);
		free(str);
	}
	return n;

}

/**
 * Q_ENTRY->getNext(): Get next object structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param obj		found data will be stored in this object
 * @param name		key name, if key name is NULL search every key in the list.
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		true if found otherwise returns false
 *
 * @note
 * if newmem flag is true, user should de-allocate obj.name and obj.data resources.
 *
 * @code
 *   Q_ENTRY *entry = qEntry();
 *   entry->putStr(entry, "key1", "hello world 1", false);
 *   entry->putStr(entry, "key2", "hello world 2", false);
 *   entry->putStr(entry, "key3", "hello world 3", false);
 *
 *   // non-thread usages
 *   Q_NLOBJ_T obj;
 *   memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *   while((entry->getNext(entry, &obj, NULL, false) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *   }
 *
 *   // thread model with specific key search
 *   Q_NLOBJ_T obj;
 *   memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *   entry->qlock();
 *   while((entry->getNext(entry, &obj, "key2", false) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *   }
 *   entry->unlock();
 *
 *   // thread model 2 with newmem flag
 *   Q_NLOBJ_T obj;
 *   memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
 *   entry->qlock();
 *   while((entry->getNext(entry, &obj, NULL, true) == true) {
 *     printf("NAME=%s, DATA=%s", SIZE=%zu", obj.name, obj.data, obj.size);
 *     free(obj.name);
 *     free(obj.data);
 *   }
 *   entry->unlock();
 */
static bool _getNext(Q_ENTRY *entry, Q_NLOBJ_T *obj, const char *name, bool newmem) {
	if(entry == NULL || obj == NULL) return NULL;

	Q_LOCK_ENTER(entry->qlock);

	// if obj->next is NULL, it means start over.
	if(obj->next == NULL) obj->next = entry->first;

	Q_NLOBJ_T *cont;
	bool ret = false;
	for(cont = obj->next; cont; cont = cont->next) {
		if(name != NULL && strcmp(cont->name, name)) continue;

		if(newmem == true) {
			obj->name = strdup(cont->name);
			obj->data = malloc(cont->size);
			memcpy(obj->data, cont->data, cont->size);
		} else {
			obj->name = cont->name;
			obj->data = cont->data;
		}
		obj->size = cont->size;
		obj->next = cont->next;

		ret = true;
		break;
	}

	Q_LOCK_LEAVE(entry->qlock);
	return ret;
}

/**
 * Q_ENTRY->remove(): Remove matched objects as given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a number of removed objects.
 */
static int _remove(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	Q_LOCK_ENTER(entry->qlock);

	int removed = 0;
	Q_NLOBJ_T *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		if (!strcmp(obj->name, name)) { /* found */
			/* copy next chain */
			Q_NLOBJ_T *next = obj->next;

			/* adjust counter */
			entry->size -= obj->size;
			entry->num--;
			removed++;

			/* remove entry itself*/
			free(obj->name);
			free(obj->data);
			free(obj);

			/* adjust chain links */
			if(next == NULL) entry->last = prev;	/* if the object is last one */
			if(prev == NULL) entry->first = next;	/* if the object is first one */
			else prev->next = next;			/* if the object is middle or last one */

			/* set next entry */
			obj = next;
		} else {
			/* remember prev object */
			prev = obj;

			/* set next entry */
			obj = obj->next;
		}
	}

	Q_LOCK_LEAVE(entry->qlock);
	return removed;
}

/**
 * Q_ENTRY->getNum(): Get total number of stored objects
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		total number of stored objects.
 */
static int _getNum(Q_ENTRY *entry) {
	if(entry == NULL) return 0;

	return entry->num;
}

/**
 * Q_ENTRY->getNo(): Get stored logical sequence number for the object.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		stored sequence number of the object if found, otherwise 0 will be returned.
 *
 * @note
 * Sequence number starts from 1.
 */
static int _getNo(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	Q_LOCK_ENTER(entry->qlock);
	int ret = 0;
	int no;
	Q_NLOBJ_T *obj;
	for(obj = entry->first, no = 1; obj; obj = obj->next, no++) {
		if (!strcmp(name, obj->name)) {
			ret = no;
			break;
		}
	}
	Q_LOCK_LEAVE(entry->qlock);
	return ret;
}

/**
 * Q_ENTRY->parseStr(): Parse string with given entries.
 *
 * @param entry		Q_ENTRY pointer
 * @param str		string value which may contain variables like ${...}
 *
 * @return		malloced string if successful, otherwise returns NULL.
 *
 * @code
 *   ${key_name}		variable replacement
 *   ${!system_command}		external command results
 *   ${%PATH}			get environments
 * @endcode
 *
 * @code
 *   char *info = entry->parseStr(entry, "${SOME_KEY} ${!uname -a} ${%PATH}", true);
 *   if(info != NULL) free(info);
 * @endcode
 */
static char *_parseStr(Q_ENTRY *entry, const char *str) {
	if(str == NULL) return NULL;
	if(entry == NULL) return strdup(str);

	Q_LOCK_ENTER(entry->qlock);

	bool loop;
	char *value = strdup(str);
	do {
		loop = false;

		/* find ${ */
		char *s, *e;
		int openedbrakets;
		for(s = value; *s != '\0'; s++) {
			if(!(*s == _VAR && *(s+1) == _VAR_OPEN)) continue;

			/* found ${, try to find }. s points $ */
			openedbrakets = 1; /* braket open counter */
			for(e = s + 2; *e != '\0'; e++) {
				if(*e == _VAR && *(e+1) == _VAR_OPEN) { /* found internal ${ */
					s = e - 1; /* e is always bigger than s, so negative overflow never occured */
					break;
				}
				else if(*e == _VAR_OPEN) openedbrakets++;
				else if(*e == _VAR_CLOSE) openedbrakets--;
				else continue;

				if(openedbrakets == 0) break;
			}
			if(*e == '\0') break; /* braket mismatch */
			if(openedbrakets > 0) continue; /* found internal ${ */

			/* pick string between ${, } */
			int varlen = e - s - 2; /* length between ${ , } */
			char *varstr = (char*)malloc(varlen + 3 + 1);
			if(varstr == NULL) continue;
			strncpy(varstr, s + 2, varlen);
			varstr[varlen] = '\0';

			/* get the new string to replace */
			char *newstr = NULL;
			switch (varstr[0]) {
				case _VAR_CMD : {
					if ((newstr = qStrTrim(qSysCmd(varstr + 1))) == NULL) newstr = strdup("");
					break;
				}
				case _VAR_ENV : {
					newstr = strdup(qSysGetEnv(varstr + 1, ""));
					break;
				}
				default : {
					if ((newstr = _getStr(entry, varstr, true)) == NULL) {
						s = e; /* not found */
						continue;
					}
					break;
				}
			}

			/* replace */
			strncpy(varstr, s, varlen + 3); /* ${str} */
			varstr[varlen + 3] = '\0';

			s = qStrReplace("sn", value, varstr, newstr);
			free(newstr);
			free(varstr);
			free(value);
			value = s;

			loop = true;
			break;
		}
	} while(loop == true);

	Q_LOCK_LEAVE(entry->qlock);

	return value;
}

/**
 * Q_ENTRY->merge(): Merge all objects data into single object.
 *
 * @param entry		Q_ENTRY pointer
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a malloced pointer, otherwise(if there is no data to merge) returns NULL.
 *
 * @note
 * For the convenience, it allocates 1 byte more than actual total data size and store '\0' at the end.
 * But size parameter will have actual size(allocated size - 1).
 * Returned memory should be de-allocated by user.
 */
static void *_merge(Q_ENTRY *entry, size_t *size) {
	if(entry == NULL || entry->num <= 0) return NULL;

	void *dp;
	void *final;
	final = dp = (void*)malloc(entry->size + 1);

	Q_LOCK_ENTER(entry->qlock);
	const Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		memcpy(dp, obj->data, obj->size);
		dp += obj->size;
	}
	*((char *)dp) = '\0';
	Q_LOCK_LEAVE(entry->qlock);

	if(size != NULL) *size = entry->size;
	return final;
}

/**
 * Q_ENTRY->save(): Save Q_ENTRY as plain text format
 *
 * @param entry		Q_ENTRY pointer
 * @param filepath	save file path
 * @param sepchar	separator character between name and value. normally '=' is used.
 * @param encode	flag for encoding value object. false can be used if the value object
 *			is string or integer and has no new line. otherwise true must be set.
 *
 * @return		true if successful, otherwise returns false.
 */
static bool _save(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode) {
	if(entry == NULL) return false;

	int fd;
	if ((fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE)) < 0) {
		DEBUG("Q_ENTRY->save(): Can't open file %s", filepath);
		return false;
	}

	char *gmtstr = qTimeGetGmtStr(0);
	_q_writef(fd, "# automatically generated by qDecoder at %s.\n", gmtstr);
	_q_writef(fd, "# %s\n", filepath);
	free(gmtstr);

	Q_LOCK_ENTER(entry->qlock);
	Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		char *encval;
		if(encode == true) encval = qEncodeUrl(obj->data);
		else encval = obj->data;
		_q_writef(fd, "%s%c%s\n", obj->name, sepchar, encval);
		if(encode == true) free(encval);
	}
	Q_LOCK_LEAVE(entry->qlock);

	close(fd);

	return true;
}

/**
 * Q_ENTRY->load(): Load and append entries from given filepath
 *
 * @param entry		Q_ENTRY pointer
 * @param filepath	save file path
 * @param sepchar	separator character between name and value. normally '=' is used
 * @param decode	flag for decoding value object
 *
 * @return		a number of loaded entries.
 */
static int _load(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode) {
	if(entry == NULL) return 0;

	Q_ENTRY *loaded;
	if ((loaded = qConfigParseFile(NULL, filepath, sepchar)) == NULL) return false;

	Q_LOCK_ENTER(entry->qlock);
	int cnt;
	Q_NLOBJ_T *obj;
	for(cnt = 0, obj = loaded->first; obj; obj = obj->next) {
		if(decode == true) qDecodeUrl(obj->data);
		_put(entry, obj->name, obj->data, obj->size, false);
		cnt++;
	}

	_free(loaded);
	Q_LOCK_LEAVE(entry->qlock);

	return cnt;
}

/**
 * Q_ENTRY->reverse(): Reverse-sort internal stored object.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @note
 * This method can be used to improve look up performance.
 * if your application offen looking for last stored object.
 */
static bool _reverse(Q_ENTRY *entry) {
	if(entry == NULL) return false;

	Q_LOCK_ENTER(entry->qlock);
	Q_NLOBJ_T *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		Q_NLOBJ_T *next = obj->next;
		obj->next = prev;
		prev = obj;
		obj = next;
	}

	entry->last = entry->first;
	entry->first = prev;
	Q_LOCK_LEAVE(entry->qlock);

	return true;
}


/**
 * Q_ENTRY->print(): Print out stored objects for debugging purpose.
 *
 * @param entry		Q_ENTRY pointer
 * @param out		output stream FILE descriptor such like stdout, stderr.
 * @param print_data	true for printing out object value, false for disable printing out object value.
 */
static bool _print(Q_ENTRY *entry, FILE *out, bool print_data) {
	if(entry == NULL || out == NULL) return false;

	Q_LOCK_ENTER(entry->qlock);
	const Q_NLOBJ_T *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		fprintf(out, "%s=%s (%zu)\n" , obj->name, (print_data?(char*)obj->data:"(data)"), obj->size);
	}
	Q_LOCK_LEAVE(entry->qlock);

	return true;
}

/**
 * Q_ENTRY->free(): Free Q_ENTRY
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		always returns true.
 */
static bool _free(Q_ENTRY *entry) {
	if(entry == NULL) return false;

	Q_LOCK_ENTER(entry->qlock);
	Q_NLOBJ_T *obj;
	for(obj = entry->first; obj;) {
		Q_NLOBJ_T *next = obj->next;
		free(obj->name);
		free(obj->data);
		free(obj);
		obj = next;
	}
	Q_LOCK_LEAVE(entry->qlock);
	Q_LOCK_DESTROY(entry->qlock);

	free(entry);
	return true;
}
