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

static char *_parseVariable(Q_ENTRY *config, char *value);

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
Q_ENTRY *qConfigParseFile(Q_ENTRY *config, const char *filepath, char sepchar) {
	char *str = qFileLoad(filepath, NULL);
	if (str == NULL) return NULL;

	/* processing include directive */
	char *p = str;;
	while ((p = strstr(p, _INCLUDE_DIRECTIVE)) != NULL) {
		if (p == str || p[-1] == '\n') {
			char buf[MAX_PATHLEN], *e, *t = NULL;
			int len;

			/* parse filename */
			for (e = p + CONST_STRLEN(_INCLUDE_DIRECTIVE); *e != '\n' && *e != '\0'; e++);
			len = e - (p + CONST_STRLEN(_INCLUDE_DIRECTIVE));
			if (len >= sizeof(buf)) {
				DEBUG("Can't process %s directive.", _INCLUDE_DIRECTIVE);
				free(str);
				return NULL;
			}

			qStrCpy(buf, sizeof(buf), p + CONST_STRLEN(_INCLUDE_DIRECTIVE), len);
			qStrTrim(buf);

			/* adjust file path */
			if (!(buf[0] == '/' || buf[0] == '\\')) {
				char tmp[MAX_PATHLEN];
				char *dir = qFileGetDir(filepath);
				if (strlen(dir) + 1 + strlen(buf) >= sizeof(buf)) {
					DEBUG("Can't process %s directive.", _INCLUDE_DIRECTIVE);
					free(str);
					return NULL;
				}
				snprintf(tmp, sizeof(tmp), "%s/%s", dir, buf);
				strcpy(buf, tmp);
			}

			/* read file */
			if (strlen(buf) == 0 || (t = qFileLoad(buf, NULL)) == NULL) {
				DEBUG("Can't process '%s%s' directive.", _INCLUDE_DIRECTIVE, buf);
				free(str);
				return NULL;
			}

			/* replace */
			qStrCpy(buf, sizeof(buf), p, CONST_STRLEN(_INCLUDE_DIRECTIVE) + len);
			p = qStrReplace("sn", str, buf, t);
			free(t);
			free(str);
			str = p;
		} else {
			p += CONST_STRLEN(_INCLUDE_DIRECTIVE);
		}
	}

	/* decode */
	config = qConfigParseStr(config, str, sepchar);
	free(str);

	return config;
}

Q_ENTRY *qConfigParseStr(Q_ENTRY *config, const char *str, char sepchar) {
	if (str == NULL) return NULL;

	if(config == NULL) {
		config = qEntryInit();
		if(config == NULL) return NULL;
	}

	char *org, *buf, *offset;
	for (org = buf = offset = strdup(str); *offset != '\0'; ) {
		/* get one line into buf */
		for (buf = offset; *offset != '\n' && *offset != '\0'; offset++);
		if (*offset != '\0') {
			*offset = '\0';
			offset++;
		}
		qStrTrim(buf);

		/* skip blank or comment line */
		if ((buf[0] == '#') || (buf[0] == '\0')) continue;

		/* parse & store */
		char *value = strdup(buf);
		char *name  = _makeword(value, sepchar);
		qStrTrim(value);
		qStrTrim(name);

		qEntryPutStr(config, name, value, true);

		free(name);
		free(value);
	}
	free(org);

	/* processing ${} directive */
	Q_NLOBJ *obj;
	for(obj = (Q_NLOBJ*)qEntryFirst(config); obj; obj = (Q_NLOBJ*)qEntryNext(config)) {
		obj->object = _parseVariable(config, obj->object);
	}

	return config;
}

#define _VAR			'$'
#define _VAR_OPEN		'{'
#define _VAR_CLOSE		'}'
#define _VAR_CMD		'!'
#define _VAR_ENV		'%'
static char *_parseVariable(Q_ENTRY *config, char *value) {
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
			char buf[MAX_LINEBUF];
			int len;

			len = e - s - 2; /* length between ${ , } */
			if (len >= (sizeof(buf) - 1)) continue; /* too long */
			qStrCpy(buf, sizeof(buf), s + 2, len);
			qStrTrim(buf);

			/* get the string to replace*/
			char *t;
			int freet = 0;
			switch (buf[0]) {
				case _VAR_CMD : {
					if ((t = qStrTrim(qCmd(buf + 1))) == NULL) t = "";
					else freet = 1;
					break;
				}
				case _VAR_ENV : {
					t = qGetenvDefault("", buf + 1);
					break;
				}
				default : {
					if ((t = (char *)qEntryGetStr(config, buf)) == NULL) {
						s = e; /* not found */
						continue;
					}
					break;
				}
			}

			/* replace */
			qStrCpy(buf, sizeof(buf), s, len + 3); /* ${value} */

			s = qStrReplace("sn", value, buf, t);
			if (freet == 1) free(t);
			free(value);
			value = s;

			loop = 1;
			break;
		}
	} while(loop == 1);

	return value;
}

#endif /* DISABLE_CONFIGPARSER */
