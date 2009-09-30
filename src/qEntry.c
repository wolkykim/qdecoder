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
 *   Q_ENTRY *entries = qEntryInit();
 *   entries = qEntryPut(entries, "obj", (void*)my_obj, sizeof(struct MY_OBJ), true);
 *   entries = qEntryPutStr(entries, "obj", my_str, true);
 *   entries = qEntryPutInt(entries, "obj", my_int, true);
 *
 *   // print out
 *   qEntryPrint(entries, stdout, false);
 *
 *   // free object
 *   qEntryFree(entries);
 *
 *   [Result]
 * @endcode
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

/**
 * Initialize linked-list structure
 *
 * @return	a pointer of malloced Q_ENTRY structure in case of successful, otherwise returns NULL.
 *
 * @code
 *   Q_ENTRY *entries = qEntryInit();
 * @endcode
 */
Q_ENTRY *qEntryInit(void) {
	Q_ENTRY *entry = (Q_ENTRY *)malloc(sizeof(Q_ENTRY));
	if(entry == NULL) return NULL;

	memset((void *)entry, 0, sizeof(Q_ENTRY));
	return entry;
}

/**
 * Get first object structure.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		a pointer to internal Q_NLOBJ object structure if successful, otherwise returns NULL
 */
const Q_NLOBJ *qEntryFirst(Q_ENTRY *entry) {
	if(entry == NULL) return NULL;

	entry->next = entry->first;
	return qEntryNext(entry);
}

/**
 * Get next object structure.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		a pointer to internal Q_NLOBJ object structure if successful, otherwise returns NULL
 */
const Q_NLOBJ *qEntryNext(Q_ENTRY *entry) {
	if(entry == NULL) return NULL;

	Q_NLOBJ *obj = entry->next;
	if(obj != NULL) {
		entry->next = obj->next;
	}
	return obj;
}

/**
 * Remove matched objects as given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a number of removed objects.
 */
int qEntryRemove(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	int entrynum = entry->num;
	Q_NLOBJ *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		if (!strcmp(obj->name, name)) { /* found */
			/* copy next chain */
			Q_NLOBJ *next = obj->next;

			/* adjust counter */
			entry->size -= obj->size;
			entry->num--;

			/* remove entry itself*/
			free(obj->name);
			free(obj->object);
			free(obj);

			/* make a chain then set next entry */
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

	return (entrynum - entry->num);
}

/**
 * Store object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param object	object pointer
 * @param size		size of the object
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPut(Q_ENTRY *entry, const char *name, const void *object, int size, bool replace) {
	/* check arguments */
	if(entry == NULL || name == NULL || object == NULL || size < 0) return false;

	/* duplicate name */
	char *dup_name = strdup(name);
	if(dup_name == NULL) return false;

	/* duplicate object */
	void *dup_object = (size>0?malloc(size):strdup(""));
	if(dup_object == NULL) {
		free(dup_name);
		return false;
	}
	memcpy(dup_object, object, size);

	/* check same name */
	if (replace == true) qEntryRemove(entry, dup_name);

	/* new entry */
	Q_NLOBJ *obj = (Q_NLOBJ*)malloc(sizeof(Q_NLOBJ));
	if(obj == NULL) {
		free(dup_name);
		free(dup_object);
		return false;
	}
	obj->name  = dup_name;
	obj->object = dup_object;
	obj->size  = size;
	obj->next  = NULL;

	/* make chain link */
	if(entry->first == NULL) entry->first = entry->last = obj;
	else {
		entry->last->next = obj;
		entry->last = obj;
	}

	entry->size += size;
	entry->num++;
	return true;
}

/**
 * Add string object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param str		string value
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutStr(Q_ENTRY *entry, const char *name, const char *str, bool replace) {
	int size = (str!=NULL) ? (strlen(str) + 1) : 0;
	return qEntryPut(entry, name, (const void *)str, size, replace);
}

/**
 * Add formatted string object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 * @param format	formatted string value
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutStrf(Q_ENTRY *entry, const char *name, bool replace, char *format, ...) {
	char str[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(str, sizeof(str), format, arglist);
	va_end(arglist);

	return qEntryPutStr(entry, name, str, replace);
}

/**
 * Add integer object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param num		number value
 * @param replace	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutInt(Q_ENTRY *entry, const char *name, int num, bool replace) {
	char str[10+1];
	sprintf(str, "%d", num);
	return qEntryPut(entry, name, (void *)str, strlen(str) + 1, replace);
}

/**
 * Find object with given name
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a pointer of the stored object.
 *
 * @note
 * Because of qEntryGet() returns internal object pointer, you do not
 * modify it directly and do not free it. So if you would like to modify
 * that object, use it after duplicating.
 */
const void *qEntryGet(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcmp(obj->name, name)) {
			if(size != NULL) *size = obj->size;
			entry->cont = obj->next;
			return obj->object;
		}
	}

	entry->cont = NULL;
	return NULL;
}

/**
 * Find object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a pointer of the stored object.
 */
const void *qEntryGetCase(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if(!strcasecmp(name, obj->name)) {
			if(size != NULL) *size = obj->size;
			entry->cont = obj->next;
			return obj->object;
		}
	}

	entry->cont = NULL;
	return NULL;
}

/**
 * Find next object with given name
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a pointer of the stored object.
 *
 * @note
 * qEntryGet() should be called before.
 */
const void *qEntryGetNext(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	const Q_NLOBJ *obj;
	for(obj = entry->cont; obj; obj = obj->next) {
		if(!strcmp(obj->name, name)) {
			entry->next = obj->next;
			if(size != NULL) *size = obj->size;
			entry->cont = obj->next;
			return obj->object;
		}
	}

	entry->cont = NULL;
	return NULL;
}

/**
 * Find next object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a pointer of the stored object.
 *
 * @note
 * qEntryGet() should be called before.
 */
const void *qEntryGetNextCase(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	const Q_NLOBJ *obj;
	for(obj = entry->cont; obj; obj = obj->next) {
		if(!strcasecmp(name, obj->name)) {
			entry->next = obj->next;
			if(size != NULL) *size = obj->size;
			entry->cont = obj->next;
			return obj->object;
		}
	}

	entry->cont = NULL;
	return NULL;
}

/**
 * Find lastest matched object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 * @param size		if size is not NULL, object size will be stored.
 *
 * @return		a pointer of the stored object.
 *
 * @note
 * If you have objects which have same name. this method can be used to
 * find out lastest matched object.
 */
const void *qEntryGetLast(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	void *object = NULL;
	Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		if (!strcmp(name, obj->name)) {
			object = obj->object;
			if(size != NULL) *size = obj->size;
		}
	}

	return object;
}

/**
 * Find string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 *
 * @note
 * String object should be stored by qEntryPutStr().
 */
const char *qEntryGetStr(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGet(entry, name, NULL);
}

/**
 * Find string object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrCase(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGetCase(entry, name, NULL);
}

/**
 * Find string object with formatted name.
 *
 * @param entry		Q_ENTRY pointer
 * @param format	name format
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrf(Q_ENTRY *entry, char *format, ...) {
	char name[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name), format, arglist);
	va_end(arglist);

	return (char *)qEntryGet(entry, name, NULL);
}

/**
 * Find next string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrNext(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGetNext(entry, name, NULL);
}

/**
 * Find next string object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrNextCase(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGetNextCase(entry, name, NULL);
}

/**
 * Find lastest matched string object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrLast(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGetLast(entry, name, NULL);
}

/**
 * Find integer object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 *
 * @note
 * Integer object should be stored by qEntryPutInt().
 */
int qEntryGetInt(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGet(entry, name, NULL);
	if(str != NULL) return atoi((char *)str);
	return 0;
}

/**
 * Find integer object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
int qEntryGetIntCase(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGetCase(entry, name, NULL);
	if(str != NULL) return atoi((char *)str);
	return 0;
}

/**
 * Find integer object with formatted name.
 *
 * @param entry		Q_ENTRY pointer
 * @param format	name format
 *
 * @return		a integer value of the object.
 */
int qEntryGetIntf(Q_ENTRY *entry, char *format, ...) {
	char name[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name), format, arglist);
	va_end(arglist);

	return qEntryGetInt(entry, name);
}

/**
 * Find next integer object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
int qEntryGetIntNext(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGetNext(entry, name, NULL);
	if(str != NULL) return atoi((char *)str);
	return 0;
}

/**
 * Find next integer object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
int qEntryGetIntNextCase(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGetNextCase(entry, name, NULL);
	if(str != NULL) return atoi((char *)str);
	return 0;
}

/**
 * Find lastest matched integer object with given name.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a integer value of the object.
 */
int qEntryGetIntLast(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGetLast(entry, name, NULL);
	if(str != NULL) return atoi(str);
	return 0;
}

/**
 * Get total number of stored objects
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		total number of stored objects.
 */
int qEntryGetNum(Q_ENTRY *entry) {
	if(entry == NULL) return 0;

	return entry->num;
}

/**
 * Get the stored sequence number of the object.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		stored sequence number of the object if found, otherwise 0 will be returned.
 */
int qEntryGetNo(Q_ENTRY *entry, const char *name) {
	if(entry == NULL || name == NULL) return 0;

	int no;
	Q_NLOBJ *obj;
	for(obj = entry->first, no = 1; obj; obj = obj->next, no++) {
		if (!strcmp(name, obj->name)) return no;
	}
	return 0;
}

/**
 * Reverse-sort internal stored object.
 *
 * @param entry		Q_ENTRY pointer
 *
 * @note
 * This method can be used to improve look up performance.
 * if your application offen looking for last stored object.
 */
bool qEntryReverse(Q_ENTRY *entry) {
	if(entry == NULL) return false;

	Q_NLOBJ *prev, *obj;
	for (prev = NULL, obj = entry->first; obj;) {
		Q_NLOBJ *next = obj->next;
		obj->next = prev;
		prev = obj;
		obj = next;
	}

	entry->last = entry->first;
	entry->first = prev;

	return true;
}

/**
 * Print out stored objects for debugging purpose.
 *
 * @param entry		Q_ENTRY pointer
 * @param out		output stream FILE descriptor such like stdout, stderr.
 * @param print_object	true for printing out object value, false for disable printing out object value.
 */
bool qEntryPrint(Q_ENTRY *entry, FILE *out, bool print_object) {
	if(entry == NULL || out == NULL) return false;

	const Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		fprintf(out, "%s=%s (%d)\n" , obj->name, (print_object?(char*)obj->object:"(object)"), obj->size);
	}

	return true;
}

/**
 * Free Q_ENTRY
 *
 * @param entry		Q_ENTRY pointer
 *
 * @return		always returns true.
 */
bool qEntryFree(Q_ENTRY *entry) {
	if(entry == NULL) return false;

	Q_NLOBJ *obj;
	for(obj = entry->first; obj;) {
		Q_NLOBJ *next = obj->next;
		free(obj->name);
		free(obj->object);
		free(obj);
		obj = next;
	}
	free(entry);

	return true;
}

/**
 * Save Q_ENTRY as plain text format
 *
 * @param entry		Q_ENTRY pointer
 * @param filepath	save file path
 * @param sepchar	separator character between name and value. normally '=' is used.
 * @param encode	flag for encoding value object. false can be used if the value object
 *			is string or integer and has no new line. otherwise true must be set.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntrySave(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode) {
	if(entry == NULL) return false;

	int fd;
	if ((fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE)) < 0) return false;

	char *gmtstr = qTimeGetGmtStr(0);
	_q_writef(fd, "# automatically generated by qDecoder at %s.\n", gmtstr);
	_q_writef(fd, "# %s\n", filepath);
	free(gmtstr);

	const Q_NLOBJ *obj;
	for(obj = entry->first; obj; obj = obj->next) {
		char *encval;
		if(encode == true) encval = qEncodeUrl(obj->object);
		else encval = obj->object;
		_q_writef(fd, "%s%c%s\n", obj->name, sepchar, encval);
		if(encode == true) free(encval);
	}

	close(fd);
	return true;
}

/**
 * Load and append entries from given filepath
 *
 * @param entry		Q_ENTRY pointer
 * @param filepath	save file path
 * @param sepchar	separator character between name and value. normally '=' is used
 * @param decode	flag for decoding value object
 *
 * @return		a number of loaded entries.
 */
int qEntryLoad(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode) {
	if(entry == NULL) return 0;

	Q_ENTRY *loaded;
	if ((loaded = qConfigParseFile(NULL, filepath, sepchar)) == NULL) return false;

	int cnt = 0;
	Q_NLOBJ *obj;
	for(obj = loaded->first; obj; obj = obj->next) {
		if(decode == true) qDecodeUrl(obj->object);
		qEntryPut(entry, obj->name, obj->object, obj->size, false);
		cnt++;
	}

	qEntryFree(loaded);

	return cnt;
}
