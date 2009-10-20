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

static Q_NOBJ*		_getObjFirst(Q_ENTRY *entry);
static Q_NOBJ*		_getObjNext(Q_ENTRY *entry);
static void		_getObjFinish(Q_ENTRY *entry);

static bool		_put(Q_ENTRY *entry, const char *name, const void *data, int size, bool replace);
static bool		_putStr(Q_ENTRY *entry, const char *name, const char *str, bool replace);
static bool		_putStrParsed(Q_ENTRY *entry, const char *name, const char *str, bool replace);
static bool		_putInt(Q_ENTRY *entry, const char *name, int num, bool replace);

static void*		_get(Q_ENTRY *entry, const char *name, int *size, bool newmem);
static void*		_getCase(Q_ENTRY *entry, const char *name, int *size, bool newmem);
static void*		_getLast(Q_ENTRY *entry, const char *name, int *size, bool newmem);
static Q_OBJ*		_getMulti(Q_ENTRY *entry, const char *name, int *num);

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

static bool		_save(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode);
static int		_load(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode);
static bool		_reverse(Q_ENTRY *entry);
static bool		_print(Q_ENTRY *entry, FILE *out, bool print_object);
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
	MUTEX_INIT(entry->mutex, false);

	/* member methods */
	entry->getObjFirst	= _getObjFirst;
	entry->getObjNext	= _getObjNext;
	entry->getObjFinish	= _getObjFinish;

	entry->put		= _put;
	entry->putStr		= _putStr;
	entry->putStrParsed	= _putStrParsed;
	entry->putInt		= _putInt;

	entry->get		= _get;
	entry->getCase		= _getCase;
	entry->getLast		= _getLast;
	entry->getMulti		= _getMulti;

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

	entry->save		= _save;
	entry->load		= _load;
	entry->reverse		= _reverse;
	entry->print		= _print;
	entry->free		= _free;

	return entry;
}

/**
 * Q_ENTRY->getObjFirst(): Get first object structure.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		a pointer to internal Q_NLOBJ object structure if successful, otherwise returns NULL
 *
 * @note
 * In case of thread usages, this raise mutex lock. Q_ENTRY->getObjFinish() must be called to release lock.
 */
static Q_NOBJ *_getObjFirst(Q_ENTRY *entry) {
	if(entry == NULL) return NULL;

	MUTEX_LOCK(entry->mutex);

	entry->cont = entry->first;
	return _getObjNext(entry);
}

/**
 * Q_ENTRY->getObjNext(): Get next object structure.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		a pointer to internal Q_NLOBJ object structure if successful, otherwise returns NULL
 */
static Q_NOBJ *_getObjNext(Q_ENTRY *entry) {
	if(entry == NULL) return NULL;

	Q_NOBJ *nobj = NULL;
	Q_NLOBJ *obj = entry->cont;
	if(obj != NULL) {
		nobj = (Q_NOBJ*)malloc(sizeof(Q_NOBJ));
		memset((void*)nobj, sizeof(Q_NOBJ), 0);
		nobj->name = strdup(obj->name);
		nobj->data = malloc(obj->size);
		nobj->size = obj->size;
		memcpy(nobj->data, obj->data, obj->size);

		/* keep next object*/
		entry->cont = obj->next;
	}

	return nobj;
}

/**
 * Q_ENTRY->getObjFinish(): Finish next object search.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @note
 * This method should be called after using getObjFirst() and getObjNext() method to release lock.
 */
static void _getObjFinish(Q_ENTRY *entry) {
	MUTEX_UNLOCK(entry->mutex);
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
static bool _put(Q_ENTRY *entry, const char *name, const void *data, int size, bool replace) {
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
	Q_NLOBJ *obj = (Q_NLOBJ*)malloc(sizeof(Q_NLOBJ));
	if(obj == NULL) {
		free(dup_name);
		free(dup_data);
		return false;
	}
	obj->name = dup_name;
	obj->data = dup_data;
	obj->size = size;
	obj->next = NULL;

	MUTEX_LOCK(entry->mutex);

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

	MUTEX_UNLOCK(entry->mutex);

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
	int size = (str!=NULL) ? (strlen(str) + 1) : 0;
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
	int size = (newstr!=NULL) ? (strlen(newstr) + 1) : 0;
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
 * @note
 * If newmem flag is true, returned data will be malloced and should be deallocated by user.
 * Otherwise returned pointer will point internal buffer directly and should not be de-allocated by user.
 * In thread-safe mode, newmem flag always should be true.
 */
static void *_get(Q_ENTRY *entry, const char *name, int *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

#ifdef ENABLE_THREADSAFE
	if(newmem == false) return "Q_ENTRY->get(): newmem flag should be true in thread-safe mode.";
#endif

	MUTEX_LOCK(entry->mutex);

	Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcmp(obj->name, name)) {
			if(size != NULL) *size = obj->size;
			void *data;
			if(newmem == true) {
				data = malloc(obj->size);
				memcpy(data, obj->data, obj->size);
			} else {
				data = obj->data;
			}

			MUTEX_UNLOCK(entry->mutex);
			return data;
		}
	}

	MUTEX_UNLOCK(entry->mutex);
	return NULL;
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
static void *_getCase(Q_ENTRY *entry, const char *name, int *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

#ifdef ENABLE_THREADSAFE
	if(newmem == false) return "Q_ENTRY->getCase(): newmem flag should be true in thread-safe mode.";
#endif

	MUTEX_LOCK(entry->mutex);

	Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcasecmp(name, obj->name)) {
			if(size != NULL) *size = obj->size;
			void *data;
			if(newmem == true) {
				data = malloc(obj->size);
				memcpy(data, obj->data, obj->size);
			} else {
				data = obj->data;
			}

			MUTEX_UNLOCK(entry->mutex);
			return data;
		}
	}

	MUTEX_UNLOCK(entry->mutex);
	return NULL;
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
static void *_getLast(Q_ENTRY *entry, const char *name, int *size, bool newmem) {
	if(entry == NULL || name == NULL) return NULL;

#ifdef ENABLE_THREADSAFE
	if(newmem == false) return "Q_ENTRY->getCase(): newmem flag should be true in thread-safe mode.";
#endif

	MUTEX_LOCK(entry->mutex);

	Q_NLOBJ *lastobj = NULL;
	Q_NLOBJ *obj;
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

	MUTEX_UNLOCK(entry->mutex);
	return data;
}

/**
 * Q_ENTRY->getMulti(): Find multiple objects with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param num		the number of objects found will be stored
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 *
 * @note
 * Q_OBJ* should be de-allocated by user.
 */
static Q_OBJ *_getMulti(Q_ENTRY *entry, const char *name, int *num) {


	return NULL;
}

/**
 * Q_ENTRY->getStr(): Find string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param newmem	whether or not to allocate memory for the data.
 *
 * @return		a pointer of malloced data if key is found, otherwise returns NULL.
 *
 * @note
 * String object should be stored by _putStr().
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
 * Q_ENTRY->remove(): Remove matched objects as given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a number of removed objects.
 */
static int _remove(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	MUTEX_LOCK(entry->mutex);

	int removed = 0;
	Q_NLOBJ *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		if (!strcmp(obj->name, name)) { /* found */
			/* copy next chain */
			Q_NLOBJ *next = obj->next;

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

	MUTEX_UNLOCK(entry->mutex);
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
 */
static int _getNo(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	MUTEX_LOCK(entry->mutex);
	int no;
	Q_NLOBJ *obj;
	for(obj = entry->first, no = 1; obj; obj = obj->next, no++) {
		if (!strcmp(name, obj->name)) {
			MUTEX_UNLOCK(entry->mutex);
			return no;
		}
	}
	MUTEX_UNLOCK(entry->mutex);
	return 0;
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

	MUTEX_LOCK(entry->mutex);

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

	MUTEX_UNLOCK(entry->mutex);

	return value;
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

	MUTEX_LOCK(entry->mutex);

	int fd;
	if ((fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE)) < 0) {
		DEBUG("Q_ENTRY->save(): Can't open file %s", filepath);
		MUTEX_UNLOCK(entry->mutex);
		return false;
	}

	char *gmtstr = qTimeGetGmtStr(0);
	_q_writef(fd, "# automatically generated by qDecoder at %s.\n", gmtstr);
	_q_writef(fd, "# %s\n", filepath);
	free(gmtstr);

	const Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		char *encval;
		if(encode == true) encval = qEncodeUrl(obj->data);
		else encval = obj->data;
		_q_writef(fd, "%s%c%s\n", obj->name, sepchar, encval);
		if(encode == true) free(encval);
	}

	close(fd);

	MUTEX_UNLOCK(entry->mutex);
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

	MUTEX_LOCK(entry->mutex);

	int cnt;
	Q_NLOBJ *obj;
	for(cnt = 0, obj = loaded->first; obj; obj = obj->next) {
		if(decode == true) qDecodeUrl(obj->data);
		_put(entry, obj->name, obj->data, obj->size, false);
		cnt++;
	}

	_free(loaded);

	MUTEX_UNLOCK(entry->mutex);

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

	MUTEX_LOCK(entry->mutex);

	Q_NLOBJ *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		Q_NLOBJ *next = obj->next;
		obj->next = prev;
		prev = obj;
		obj = next;
	}

	entry->last = entry->first;
	entry->first = prev;

	MUTEX_UNLOCK(entry->mutex);

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

	MUTEX_LOCK(entry->mutex);

	const Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		fprintf(out, "%s=%s (%d)\n" , obj->name, (print_data?(char*)obj->data:"(data)"), obj->size);
	}

	MUTEX_UNLOCK(entry->mutex);

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

	MUTEX_LOCK(entry->mutex);

	Q_NLOBJ *obj;
	for(obj = entry->first; obj;) {
		Q_NLOBJ *next = obj->next;
		free(obj->name);
		free(obj->data);
		free(obj);
		obj = next;
	}

	MUTEX_UNLOCK(entry->mutex);
	MUTEX_DESTROY(entry->mutex);

	free(entry);

	return true;
}
