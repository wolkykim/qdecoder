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
 * @file qEntry.c Linked-list Data Structure API
 *
 * @code
 *   [Code sample - String]
*
 *   // sample data
 *   struct MY_OBJ *my_obj = getNewMyOjb();;
 *   char *my_str = "hello";
 *   int my_int = 1;
 *
 *   // store into linked-list
 *   Q_ENTRY *entries = NULL;
 *   entries = qEntryPut(entries, "obj", (void*)my_obj, sizeof(struct MY_OBJ), true);
 *   entries = qEntryPutStr(entries, "obj", my_str, true);
 *   entries = qEntryPutInt(entries, "obj", my_int, true);
 *
 *   // print out
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
 * Under-development
 *
 * @since not released yet
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
 * @return		a number of removed object.
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
			/* set next entry */
			prev = obj;
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
 * @param update	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPut(Q_ENTRY *entry, const char *name, const void *object, int size, bool update) {
	/* check arguments */
	if(entry == NULL || name == NULL || object == NULL || size <= 0) return false;

	/* duplicate name */
	char *dup_name = strdup(name);
	if(dup_name == NULL) return false;

	/* duplicate object */
	void *dup_object = malloc(size);
	if(dup_object == NULL) {
		free(dup_name);
		return false;
	}
	memcpy(dup_object, object, size);

	/* check same name */
	if (update == true) qEntryRemove(entry, dup_name);

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
 * @param update	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutStr(Q_ENTRY *entry, const char *name, const char *str, bool update) {
	int size = (str!=NULL) ? (strlen(str) + 1) : 0;
	return qEntryPut(entry, name, (void *)str, size, update);
}

/**
 * Add formatted string object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param update	in case of false, just insert. in case of true, remove all same key then insert object if found.
 * @param format	formatted string value
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutStrf(Q_ENTRY *entry,  const char *name, bool update, char *format, ...) {
	char str[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(str, sizeof(str), format, arglist);
	va_end(arglist);

	return qEntryPutStr(entry, name, str, update);
}

/**
 * Add integer object into linked-list structure.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name.
 * @param num		number value
 * @param update	in case of false, just insert. in case of true, remove all same key then insert object if found.
 *
 * @return		true if successful, otherwise returns false.
 */
bool qEntryPutInt(Q_ENTRY *entry, const char *name, int num, bool update) {
	char str[10+1];
	sprintf(str, "%d", num);
	return qEntryPut(entry, name, (void *)str, strlen(str) + 1, update);
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

	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(entry); obj; obj = qEntryNext(entry)) {
		if(!strcmp(obj->name, name)) {
			if(size != NULL) *size = obj->size;
			return obj->object;
		}
	}

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
	for(obj = qEntryNext(entry); obj; obj = qEntryNext(entry)) {
		if(!strcmp(obj->name, name)) {
			entry->next = obj->next;
			if(size != NULL) *size = obj->size;
			return obj->object;
		}
	}

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
const void *qEntryGetNoCase(Q_ENTRY *entry, const char *name, int *size) {
	if(entry == NULL || name == NULL) return NULL;

	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(entry); obj; obj = qEntryNext(entry)) {
		if(!strcasecmp(name, obj->name)) {
			if(size != NULL) *size = obj->size;
			return obj->object;
		}
	}

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
	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(entry); obj; obj = qEntryNext(entry)) {
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
 */
const char *qEntryGetStr(Q_ENTRY *entry, const char *name) {
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
 * Find string object with given name. (case-insensitive)
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
 *
 * @return		a pointer of the stored string object.
 */
const char *qEntryGetStrNoCase(Q_ENTRY *entry, const char *name) {
	return (char *)qEntryGetNoCase(entry, name, NULL);
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
 */
int qEntryGetInt(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGet(entry, name, NULL);
	if(str != NULL) return atoi((char *)str);
	return 0;
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
int qEntryGetIntNoCase(Q_ENTRY *entry, const char *name) {
	const char *str =qEntryGetNoCase(entry, name, NULL);
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
	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(entry), no = 1; obj; obj = qEntryNext(entry), no++) {
		if (!strcmp(name, obj->name)) return no;
	}
	return 0;
}

/**
 * Reverse-sort internal stored object.
 *
 * @param entry		Q_ENTRY pointer
 * @param name		key name
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
	for(obj = qEntryFirst(entry); obj; obj = qEntryNext(entry)) {
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
	for(obj = (Q_NLOBJ*)qEntryFirst(entry); obj; obj = (Q_NLOBJ*)qEntryNext(entry)) {
		free(obj->name);
		free(obj->object);
		free(obj);
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

	_writef(fd, "# automatically generated by qDecoder at %s.\n", qGetGmtimeStr(0));
	_writef(fd, "# %s\n", filepath);

	const Q_NLOBJ *obj;
	for(obj = qEntryFirst(entry); obj; obj = qEntryNext(entry)) {
		char *encval;
		if(encode == true) encval = qEncodeUrl(obj->object);
		else encval = obj->object;
		_writef(fd, "%s%c%s\n", obj->name, sepchar, encval);
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
	for(obj = (Q_NLOBJ*)qEntryFirst(loaded); obj; obj = (Q_NLOBJ*)qEntryNext(loaded)) {
		if(decode == true) qDecodeUrl(obj->object);
		qEntryPut(entry, obj->name, obj->object, obj->size, false);
		cnt++;
	}

	qEntryFree(loaded);

	return cnt;
}
