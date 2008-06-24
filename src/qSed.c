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

#ifndef DISABLE_SED

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
** Usage : qSedStr(Entry pointer, fpout, arg);
** Return: Success 1
** Do    : Stream Editor.
**********************************************/
bool qSedStr(Q_ENTRY *entry, const char *srcstr, FILE *fpout) {
	if (srcstr == NULL) return false;

	/* Parsing */
	char *sp = (char *)srcstr;
	while (*sp != '\0') {
		/* SSI invocation */
		if (!strncmp(sp, SSI_INCLUDE_START, strlen(SSI_INCLUDE_START))) {
			char ssi_inc_file[MAX_LINEBUF], *endp;
			if ((endp = strstr(sp, SSI_INCLUDE_END)) != NULL) {
				sp += strlen(SSI_INCLUDE_START);
				qStrCpy(ssi_inc_file, sizeof(ssi_inc_file), sp, endp - sp);
				sp = (endp + strlen(SSI_INCLUDE_END)) - 1;

				if (qFileExist(ssi_inc_file) == true) qSedFile(entry, ssi_inc_file, fpout);
				else printf("[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
			} else printf("[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
			continue;
		}

		/* Pattern Matching */
		int flag;
		const Q_NLOBJ *obj;
		for (obj = (Q_NLOBJ*)qEntryFirst(entry), flag = 0; obj && flag == 0; obj = (Q_NLOBJ*)qEntryNext(entry)) {
			if (!strncmp(sp, obj->name, strlen(obj->name))) {
				fprintf(fpout, "%s", (char *)obj->object);
				sp += strlen(obj->name);
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			fprintf(fpout, "%c", *sp);
			sp++;
		}
	}

	return true;
}

/**********************************************
** Usage : qSedFile(filepath, fpout, arg);
** Return: Success 1, Fail open fail 0.
** Do    : Stream Editor.
**********************************************/
bool qSedFile(Q_ENTRY *entry, const char *filepath, FILE *fpout) {
	char *sp;
	int flag;

	if ((sp = qFileLoad(filepath, NULL)) == NULL) return false;
	flag = qSedStr(entry, sp, fpout);
	free(sp);

	return flag;
}

#endif /* DISABLE_SED */
