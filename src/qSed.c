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
 *
 ---- ex) streamedit.html.in ----
<!--#include file="streamedit-header.html.in"-->
<p>Hi <b>${NAME}</b>.
<p>You got a really cool hobby.
<br>I'm sure that your hobby, <b>${HOBBY}</b>, can make your life more compatible.
<p>Bye :)
<!--#include file="streamedit-tailer.html.in"-->
---------------------

filename is an input(target) file while fpout stands for output streams. When you wish to display the results in files, open files in "w" and then, hand over the corresponding file pointers. And if you wish to display them on-screen, just specify stdout.

It interprets the SSI grammar. (Currently, only [an error occurred while processing this directive] is supported.) If there is the following lines in a document, the corresponding document is included in the display. And the replacement and SSI functions are valid for the included document. (Cascading)

<!--#include file="streamedit-header.html.in"-->

Note) The included file can be marked by relative paths on the basis of the location where CGI is executed. Or it may be marked by system absolute paths.

If you wish to use the SSI function only without replacing character strings, transmit the NULL value using the arg argument as follows:

ex) qSedFile(NULL, "streamedit.html.in", stdout);
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

/**
 * Perform text transformations on input string
 *
 * @param	entry		arguments to be transformed
 * @param	srcstr		source string
 * @param	fpout		output stream
 *
 * @return	true if successful, otherwise returns false.
 *
 * @code
 *   Q_ENTRY *args = qEntryInit();
 *   args = qEntryPutStr(args, "${NAME}", "The qDecoder Project");
 *   args = qEntryPutStr(args, "${HOBBY}", "Open Source Project");
 *   qSedFile(args, "${NAME} : ${HOBBY}", stdout);
 *   qEntryFree(args);
 * @endcode
 */
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
				else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
			} else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
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

/**
 * Perform text transformations on input file
 *
 * @param	entry		arguments to be transformed
 * @param	filepath	socket descriptor
 * @param	fpout		output stream
 *
 * @return	true if successful, otherwise returns false.
 *
 * @code
 *   <!--#include file="streamedit-header.html.in"-->
 *   <p>Hi <b>${NAME}</b>.
 *   <p>You got a really cool hobby.
 *   <br>I'm sure that your hobby, <b>${HOBBY}</b>, can give you more fun in your life time.
 *   <p>Bye :)
 *   <!--#include file="streamedit-tailer.html.in"-->
 * @endcode
 *
 * @code
 *   Q_ENTRY *args = qEntryInit();
 *   args = qEntryPutStr(args, "${NAME}", "The qDecoder Project");
 *   args = qEntryPutStr(args, "${HOBBY}", "Open Source Project");
 *   qSedFile(args, "streamedit.html.in", stdout);
 *   qEntryFree(args);
 * @endcode
 */
bool qSedFile(Q_ENTRY *entry, const char *filepath, FILE *fpout) {
	char *sp;
	int flag;

	if ((sp = qFileLoad(filepath, NULL)) == NULL) return false;
	flag = qSedStr(entry, sp, fpout);
	free(sp);

	return flag;
}
