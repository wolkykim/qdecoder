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
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/**********************************************
** Usage : qEntryAdd(first entry, name, value, replace);
** Return: New entry pointer.
** Do    : Add entry at last.
**         flag = 0 : just append.
**         flag = 1 : if same name exists, replace it.
**         flag = 2 : same as flag 0 but the name and value are binary pointer.
**                    so the pointer will be used instead strdup().
**********************************************/
Q_ENTRY *qEntryAdd(Q_ENTRY *first, char *name, char *value, int flag) {
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
	if (first != NULL) {
		for (; first->next; first = first->next);
		first->next = entries;
	}

	return entries;
}

/**********************************************
** Usage : qEntryAddInt(first entry, name, value, replace);
** Return: New entry pointer.
** Do    : Add entry at last.
**         flag = 0 : just append.
**         flag = 1 : if same name exists, replace it.
**********************************************/
Q_ENTRY *qEntryAddInt(Q_ENTRY *first, char *name, int value, int flag) {
	char buf[10+1];

	sprintf(buf, "%d", value);
	return qEntryAdd(first, name, buf, flag);
}

/**********************************************
** Usage : qEntryRemove(first entry, name to remove);
** Return: first entry pointer.
** Do    : Remove entry if same name exists, remove all.
**********************************************/
Q_ENTRY *qEntryRemove(Q_ENTRY *first, char *name) {
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
** Usage : qEntryValue(pointer of the first entry, name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find the value string pointer in linked list.
**********************************************/
char *qEntryGetValue(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;

	for (entries = first; entries; entries = entries->next) {
		if (!strcmp(name, entries->name)) return entries->value;
	}
	return NULL;
}

/**********************************************
** Usage : qEntryValue(pointer of the first entry, name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find the last value string pointer in linked list.
**********************************************/
char *qEntryGetValueLast(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;
	char *value = NULL;

	for (entries = first; entries; entries = entries->next) {
		if (!strcmp(name, entries->name)) value = entries->value;
	}
	return value;
}

char *qEntryGetValueNoCase(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;

	for (entries = first; entries; entries = entries->next) {
		if (!strcasecmp(name, entries->name)) return entries->value;
	}
	return NULL;
}

/**********************************************
** Usage : qEntryiValue(pointer of the first entry, name);
** Return: Success integer of value string, Fail 0.
** Do    : Find the value string pointer and convert to integer.
**********************************************/
int qEntryGetInt(Q_ENTRY *first, char *name) {
	char *str;

	str = qEntryGetValue(first, name);
	if (str == NULL) return 0;
	return atoi(str);
}

/**********************************************
** Usage : qEntryiValue(pointer of the first entry, name);
** Return: Success integer of value string, Fail 0.
** Do    : Find the last value string pointer and convert to integer.
**********************************************/
int qEntryGetIntLast(Q_ENTRY *first, char *name) {
	char *str;

	str = qEntryGetValueLast(first, name);
	if (str == NULL) return 0;
	return atoi(str);
}

int qEntryGetIntNoCase(Q_ENTRY *first, char *name) {
	char *str;

	str = qEntryGetValueNoCase(first, name);
	if (str == NULL) return 0;
	return atoi(str);
}

/**********************************************
** Usage : qEntryNo(pointer of the first entry, name);
** Return: Success no. Fail 0;
** Do    : Find sequence number of value string pointer.
**********************************************/
int qEntryGetNo(Q_ENTRY *first, char *name) {
	Q_ENTRY *entries;
	int no;

	for (no = 1, entries = first; entries; no++, entries = entries->next) {
		if (!strcmp(name, entries->name)) return no;
	}
	return 0;
}

/**********************************************
** Usage : qEntryReverse(pointer of the first entry);
** Return: first entry pointer
** Do    : Reverse the entries
**********************************************/
Q_ENTRY *qEntryReverse(Q_ENTRY *first) {
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
** Usage : qEntryPrint(pointer of the first entry);
** Return: Amount of entries.
** Do    : Print all parsed value & name for debugging.
**********************************************/
int qEntryPrint(Q_ENTRY *first, FILE *out) {
	Q_ENTRY *entries;
	int amount;

	if(out == NULL) out = stdout;
	for (amount = 0, entries = first; entries; amount++, entries = entries->next) {
		fprintf(out, "%s=%s\n" , entries->name, entries->value);
	}

	return amount;
}

/**********************************************
** Usage : qEntryFree(pointer of the first entry);
** Do    : Make free of linked list memory.
**********************************************/
int qEntryFree(Q_ENTRY *first) {
	Q_ENTRY *entries;
	int i;

	for (i = 0; first; first = entries, i++) {
		entries = first->next; /* copy next to tmp */
		free(first->name);
		free(first->value);
		free(first);
	}

	return i;
}

/**********************************************
** Usage : qEntrySave(pointer of the first entry, filename);
** Return: Number of saved entries.
** Do    : Save entries into file.
**********************************************/
int qEntrySave(Q_ENTRY *first, char *filename, bool encodevalue) {
	FILE *fp;
	int amount;

	if ((fp = qFileOpen(filename, "w")) == NULL) return -1;

	fprintf(fp, "# automatically generated by qDecoder at %s.\n", qGetGmtimeStr(0));
	fprintf(fp, "# %s\n", filename);
	for (amount = 0; first; first = first->next, amount++) {
		char *encvalue;

		if(encodevalue == true) encvalue = qUrlEncode(first->value);
		else encvalue = first->value;
		fprintf(fp, "%s=%s\n", first->name, encvalue);
		if(encodevalue == true) free(encvalue);
	}

	qFileClose(fp);
	return amount;
}

/**********************************************
** Usage : qEntryLoad(filename);
** Return: Success pointer of first entry, Fail NULL.
** Do    : Load entries from given filename.
**********************************************/
Q_ENTRY *qEntryLoad(char *filename, bool decodevalue) {
	Q_ENTRY *first, *entries;

	if ((first = qfDecoder(filename)) == NULL) return NULL;

	if(decodevalue == true) {
		for (entries = first; entries; entries = entries->next) {
			qUrlDecode(entries->value);
		}
	}

	return first;
}
