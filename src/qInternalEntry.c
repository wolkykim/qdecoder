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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/**********************************************
** Linked List(Entry) Routines
**********************************************/

/**********************************************
** Usage : _EntryAdd(first entry, name, value, replace);
** Return: New entry pointer.
** Do    : Add entry at last.
**         flag = 0 : just append.
**         flag = 1 : if same name exists, replace it.
**         flag = 2 : same as flag 0 but the name and value are binary pointer.
**                    so the pointer will be used instead strdup().
**********************************************/
Q_ENTRY *_EntryAdd(Q_ENTRY *first, char *name, char *value, int flag) {
	Q_ENTRY *entries;

	if (!strcmp(name, "")) return NULL;

	/* check same name */
	if (flag == 1) {
		for (entries = first; entries; entries = entries->next) {
			if (!strcmp(entries->name, name)) {
				free(entries->value);
				entries->value = strdup(value);
				return entries;
			}
		}
	}

	/* new entry */
	entries = (Q_ENTRY *)malloc(sizeof(Q_ENTRY));
	if (flag == 2) {
		entries->name  = name;
		entries->value = value;
	} else {
		entries->name  = strdup(name);
		entries->value = strdup(value);
	}
	entries->next  = NULL;

	/* If first is not NULL, find last entry then make a link*/
	if (first) {
		for (; first->next; first = first->next);
		first->next = entries;
	}

	return entries;
}

/**********************************************
** Usage : _EntryRemove(first entry, name to remove);
** Return: first entry pointer.
** Do    : Remove entry if same name exists, remove all.
**********************************************/
Q_ENTRY *_EntryRemove(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries, *prev_entry;

	if (!strcmp(name, "")) return first;

	for (prev_entry = NULL, entries = first; entries;) {
		if (!strcmp(entries->name, name)) { /* found */
			Q_ENTRY *next;

			next = entries->next;

			/* remove entry itself*/
			free(entries->name);
			free(entries->value);
			free(entries);

			/* remove entry link from linked-list */
			if (prev_entry == NULL) first = next;
			else prev_entry->next = next;
			entries = next;
		} else { /* next */
			prev_entry = entries;
			entries = entries->next;
		}
	}

	return first;
}

/**********************************************
** Usage : _EntryValue(pointer of the first entry, name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find the value string pointer in linked list.
**********************************************/
char *_EntryValue(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;

	for (entries = first; entries; entries = entries->next) {
		if (!strcmp(name, entries->name)) return entries->value;
	}
	return NULL;
}

/**********************************************
** Usage : _EntryValue(pointer of the first entry, name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find the last value string pointer in linked list.
**********************************************/
char *_EntryValueLast(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;
	char *value = NULL;

	for (entries = first; entries; entries = entries->next) {
		if (!strcmp(name, entries->name)) value = entries->value;
	}
	return value;
}

/**********************************************
** Usage : _EntryiValue(pointer of the first entry, name);
** Return: Success integer of value string, Fail 0.
** Do    : Find the value string pointer and convert to integer.
**********************************************/
int _EntryiValue(Q_ENTRY *first, char *name) {
	char *str;

	str = _EntryValue(first, name);
	if (str == NULL) return 0;
	return atoi(str);
}

/**********************************************
** Usage : _EntryiValue(pointer of the first entry, name);
** Return: Success integer of value string, Fail 0.
** Do    : Find the last value string pointer and convert to integer.
**********************************************/
int _EntryiValueLast(Q_ENTRY *first, char *name) {
	char *str;

	str = _EntryValueLast(first, name);
	if (str == NULL) return 0;
	return atoi(str);
}

/**********************************************
** Usage : _EntryNo(pointer of the first entry, name);
** Return: Success no. Fail 0;
** Do    : Find sequence number of value string pointer.
**********************************************/
int _EntryNo(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;
	int no;

	for (no = 1, entries = first; entries; no++, entries = entries->next) {
		if (!strcmp(name, entries->name)) return no;
	}
	return 0;
}

/**********************************************
** Usage : _EntryReverse(pointer of the first entry);
** Return: first entry pointer
** Do    : Reverse the entries
**********************************************/
Q_ENTRY *_EntryReverse(Q_ENTRY *first) {
	Q_ENTRY *entries, *last, *next;

	last = NULL;
	for (entries = first; entries;) {
		next = entries->next;
		entries->next = last;
		last = entries;
		entries = next;
	}
	return last;
}

/**********************************************
** Usage : _EntryPrint(pointer of the first entry);
** Return: Amount of entries.
** Do    : Print all parsed value & name for debugging.
**********************************************/
int _EntryPrint(Q_ENTRY *first) {
	Q_ENTRY *entries;
	int amount;

	qContentType("text/plain");

	for (amount = 0, entries = first; entries; amount++, entries = entries->next) {
		printf("'%s' = '%s'\n" , entries->name, entries->value);
	}

	return amount;
}

/**********************************************
** Usage : _EntryFree(pointer of the first entry);
** Do    : Make free of linked list memory.
**********************************************/
void _EntryFree(Q_ENTRY *first) {
	Q_ENTRY *entries;

	for (; first; first = entries) {
		entries = first->next; /* copy next to tmp */
		free(first->name);
		free(first->value);
		free(first);
	}
}

/**********************************************
** Usage : _EntrySave(pointer of the first entry, filename);
** Return: Success 1, Fail 0.
** Do    : Save entries into file.
**********************************************/
int _EntrySave(Q_ENTRY *first, char *filename) {
	FILE *fp;
	char gmt[32];

	qGetGMTime(gmt, (time_t)0);
	if ((fp = qfopen(filename, "w")) == NULL) return 0;

	fprintf(fp, "# automatically generated by qDecoder at %s.\n", gmt);
	fprintf(fp, "# %s\n", filename);
	for (; first; first = first->next) {
		char *encvalue;

		encvalue = qURLencode(first->value);
		fprintf(fp, "%s=%s\n", first->name, encvalue);
		free(encvalue);
	}

	qfclose(fp);
	return 1;
}

/**********************************************
** Usage : _EntryLoad(filename);
** Return: Success pointer of first entry, Fail NULL.
** Do    : Load entries from given filename.
**********************************************/
Q_ENTRY *_EntryLoad(char *filename) {
	Q_ENTRY *first, *entries;

	if ((first = qfDecoder(filename)) == NULL) return NULL;

	for (entries = first; entries; entries = entries->next) {
		qURLdecode(entries->value);
	}

	return first;
}
