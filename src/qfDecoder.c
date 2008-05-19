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
 * @file qfDecoder.c Configuration File Handling API
 */

#ifndef DISABLE_CONFIGPARSER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include "qDecoder.h"
#include "qInternal.h"

#define _INCLUDE_DIRECTIVE	"@INCLUDE "
#define _VAR			'$'
#define _VAR_OPEN		'{'
#define _VAR_CLOSE		'}'
#define _VAR_CMD		'!'
#define _VAR_ENV		'%'

static char *parseValue(Q_ENTRY *first, char *value);

static Q_ENTRY *_multi_last_entry = NULL;
static char _multi_last_key[1024];

/**********************************************
** Usage : qfDecoder(filepath, '=');
** Return: Success pointer of the first entry, Fail NULL.
** Do    : Save file into linked list.
**         Starting # is used for comments.
**
**         @INCLUDE other.conf          => (include other.conf)
**         base = /tmp                  => base = /tmp
**         log  = ${base}/log           => log  = /tmp/logVariable
**         host = ${!/bin/hostname -s}  => host = arena
**         path = ${%PATH}              => path = /usr/bin:/usr/sbin
**         #hmm = this is comments      => (skip comment)
**********************************************/
Q_ENTRY *qfDecoder(char *filepath, char sepchar) {
	Q_ENTRY *first, *entries;
	char *str, *p;

	p = str = qReadFile(filepath, NULL);
	if (str == NULL) return NULL;

	/* processing include directive */
	while ((p = strstr(p, _INCLUDE_DIRECTIVE)) != NULL) {

		if (p == str || p[-1] == '\n') {
			char buf[1024], *e, *t = NULL;
			int len;

			/* parse filename */
			for (e = p + strlen(_INCLUDE_DIRECTIVE); *e != '\n' && *e != '\0'; e++);
			len = e - (p + strlen(_INCLUDE_DIRECTIVE));
			if (len >= sizeof(buf)) {
				DEBUG("Can't process %s directive.", _INCLUDE_DIRECTIVE);
				free(str);
				return NULL;
			}
			strncpy(buf, p + strlen(_INCLUDE_DIRECTIVE), len);
			buf[len] = '\0';
			qRemoveSpace(buf);

			/* adjust file path */
			if (!(buf[0] == '/' || buf[0] == '\\')) {
				char tmp[1024], *dir, *newfile;
				newfile = strdup(filepath);
				dir = dirname(newfile);
				if (strlen(dir) + 1 + strlen(buf) >= sizeof(buf)) {
					DEBUG("Can't process %s directive.", _INCLUDE_DIRECTIVE);
					free(str);
					return NULL;
				}
				snprintf(tmp, sizeof(tmp), "%s/%s", dir, buf);
				strcpy(buf, tmp);
				free(newfile);
			}

			/* read file */
			if (strlen(buf) == 0 || (t = qReadFile(buf, NULL)) == NULL) {
				DEBUG("Can't process '%s%s' directive.", _INCLUDE_DIRECTIVE, buf);
				free(str);
				return NULL;
			}

			/* replace */
			strncpy(buf, p, strlen(_INCLUDE_DIRECTIVE) + len);
			buf[strlen(_INCLUDE_DIRECTIVE) + len] = '\0';
			p = qStrReplace("sn", str, buf, t);
			free(t);
			free(str);
			str = p;
		} else {
			p += strlen(_INCLUDE_DIRECTIVE);
		}
	}

	/* decode */
	first = qsDecoder(str, sepchar);
	free(str);

	/* processing ${} directive */
	for (entries = first; entries; entries = entries->next) {
		str = parseValue(first, entries->value);
		if(str != NULL) {
			entries->value = str; /* do not need to free entries->value */
		}
	}

	return first;
}

char *qfGetValue(Q_ENTRY *first, char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name), format, arglist);
	va_end(arglist);

	return qEntryGetValueLast(first, name);
}

int qfGetInt(Q_ENTRY *first, char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name), format, arglist);
	va_end(arglist);

	return qEntryGetIntLast(first, name);
}


char *qfGetValueFirst(Q_ENTRY *first, char *format, ...) {
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(_multi_last_key, sizeof(_multi_last_key), format, arglist);
	va_end(arglist);

	if (first == NULL) return NULL;
	_multi_last_entry = first;

	return qfGetValueNext();
}

char *qfGetValueNext(void) {
	Q_ENTRY *entries;

	for (entries = _multi_last_entry; entries; entries = entries->next) {
		if (!strcmp(_multi_last_key, entries->name)) {
			_multi_last_entry = entries->next;
			return (entries->value);
		}
	}
	_multi_last_entry = NULL;
	strcpy(_multi_last_key, "");

	return NULL;
}

int qfPrint(Q_ENTRY *first, FILE *out) {
	return qEntryPrint(first, out);
}

void qfFree(Q_ENTRY *first) {
	qEntryFree(first);
}

static char *parseValue(Q_ENTRY *first, char *value) {
	int loop;

	do {
		char *s, *e;
		int bcnt;

		loop = 0;

		/* find ${ */
		for(s = value; *s != '\0'; s++) {
			if(!(*s == _VAR && *(s+1) == _VAR_OPEN)) continue;

			/* found ${, try to find }. s points $ */
			bcnt = 1; /* braket open counter */
			for(e = s + 2; *e != '\0'; e++) {
				if(*e == _VAR && *(e+1) == _VAR_OPEN) { /* found internal ${ */
					s = e - 1; /* e is always bigger than s, so negative overflow never occured */
					break;
				}
				else if(*e == _VAR_OPEN) bcnt++;
				else if(*e == _VAR_CLOSE) bcnt--;
				else continue;

				if(bcnt == 0) break;
			}
			if(*e == '\0') break; /* braket mismatch */
			if(bcnt > 0) continue; /* found internal ${ */

			/* found atomic ${ }. pick internal string */
			char buf[1024];
			int len;

			len = e - s - 2; /* length between ${ , } */
			if (len >= (sizeof(buf) - 1)) continue; /* too long */
			strncpy(buf, s + 2, len);
			buf[len] = '\0';
			qRemoveSpace(buf);

			/* get the string to replace*/
			char *t;
			int freet = 0;
			switch (buf[0]) {
				case _VAR_CMD : {
					if ((t = qRemoveSpace(qCmd(buf + 1))) == NULL) t = "";
					else freet = 1;
					break;
				}
				case _VAR_ENV : {
					t = qGetenvDefault("", buf + 1);
					break;
				}
				default : {
					if ((t = qEntryGetValueLast(first, buf)) == NULL) {
						s = e; /* not found */
						continue;
					}
					break;
				}
			}

			/* replace */
			strncpy(buf, s, len + 3); /* ${value} */
			buf[len + 3] = '\0';

			s = qStrReplace("sn", value, buf, t);
			if (freet == 1) free(t);
			free(value);
			value = qRemoveSpace(s);
			/* printf("%s\n", value); */

			loop = 1;
			break;
		}
	} while(loop == 1);

	return value;
}

#endif /* DISABLE_CONFIGPARSER */
