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
 * @file qSed.c Server Side Include and Variable Replacement API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "qDecoder.h"
#include "qInternal.h"

#define	SSI_INCLUDE_START	"<!--#include file=\""
#define SSI_INCLUDE_END		"\"-->"

/**********************************************
** Usage : qSedArgAdd(entry pointer, name, value);
** Do    : Add given name and value to linked list.
**         If same name exists, it'll be replaced.
**
** ex) qValueAdd("NAME", "Seung-young Kim");
**********************************************/
Q_ENTRY *qSedArgAdd(Q_ENTRY *first, char *name, char *format, ...) {
	Q_ENTRY *new_entry;
	char value[1024];
	int status;
	va_list arglist;

	if (!strcmp(name, "")) qError("qSedArgAdd(): can not add empty name.");

	va_start(arglist, format);
	status = vsprintf(value, format, arglist);
	if (strlen(value) + 1 > sizeof(value) || status == EOF) qError("qSedArgAdd(): Message is too long or invalid.");
	va_end(arglist);

	new_entry = _EntryAdd(first, name, value, 1);
	if (!first) first = new_entry;

	return first;
}

/**********************************************
** Usage : qSedArgAddDirect(entry pointer, value);
** Do    : Add given name and value to linked list.
**         If same name exists, it'll be replaced.
**
** ex) qSedArgAddDirect(entries, "NAME", value);
**********************************************/
Q_ENTRY *qSedArgAddDirect(Q_ENTRY *first, char *name, char *value) {
	Q_ENTRY *new_entry;

	if (!strcmp(name, "")) qError("qSedArgAddDirect(): can not add empty name.");

	new_entry = _EntryAdd(first, name, value, 1);
	if (!first) first = new_entry;

	return first;
}

/**********************************************
** Usage : qPrint(pointer of the first Entry);
** Return: Amount of entries.
** Do    : Print all parsed values & names for debugging.
**********************************************/
int qSedArgPrint(Q_ENTRY *first) {
	return _EntryPrint(first);
}

/**********************************************
** Usage : qFree(pointer of the first Entry);
** Do    : Make free of linked list memory.
**********************************************/
void qSedArgFree(Q_ENTRY *first) {
	_EntryFree(first);
}

/**********************************************
** Usage : qSedStr(Entry pointer, fpout, arg);
** Return: Success 1
** Do    : Stream Editor.
**********************************************/
int qSedStr(Q_ENTRY *first, char *srcstr, FILE *fpout) {
	Q_ENTRY *entries;
	char *sp;

	if (srcstr == NULL) return 0;

	/* Parsing */
	for (sp = srcstr; *sp != '\0'; sp++) {
		int flag;

		/* SSI invocation */
		if (!strncmp(sp, SSI_INCLUDE_START, strlen(SSI_INCLUDE_START))) {
			char ssi_inc_file[1024], *endp;
			if ((endp = strstr(sp, SSI_INCLUDE_END)) != NULL) {
				sp += strlen(SSI_INCLUDE_START);
				strncpy(ssi_inc_file, sp, endp - sp);
				ssi_inc_file[endp-sp] = '\0';
				sp = (endp + strlen(SSI_INCLUDE_END)) - 1;

				if (qCheckFile(ssi_inc_file) == true) qSedFile(first, ssi_inc_file, fpout);
				else printf("[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
			} else printf("[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
			continue;
		}

		/* Pattern Matching */
		for (entries = first, flag = 0; entries && flag == 0; entries = entries->next) {
			if (!strncmp(sp, entries->name, strlen(entries->name))) {
				fprintf(fpout, "%s", entries->value);
				sp += strlen(entries->name) - 1;
				flag = 1;
			}
		}
		if (flag == 0) fprintf(fpout, "%c", *sp);
	}

	return 1;
}

/**********************************************
** Usage : qSedFile(filename, fpout, arg);
** Return: Success 1, Fail open fail 0.
** Do    : Stream Editor.
**********************************************/
int qSedFile(Q_ENTRY *first, char *filename, FILE *fpout) {
	char *sp;
	int flag;

	if ((sp = qReadFile(filename, NULL)) == NULL) return 0;
	flag = qSedStr(first, sp, fpout);
	free(sp);

	return flag;
}

