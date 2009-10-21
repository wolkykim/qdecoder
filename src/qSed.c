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
 * @file qSed.c Server Side Include and Variable Replacement API
 *

 *
 * filename is an input(target) file while fpout stands for output streams.
 * When you wish to display the results in files, open files in "w" and then, hand over the corresponding file pointers.
 * And if you wish to display them on-screen, just specify stdout.
 *
 * It interprets the SSI grammar. (Currently, only [an error occurred while processing this directive] is supported.)
 * If there is the following lines in a document, the corresponding document is included in the display. And the replacement and SSI functions are valid for the included document. (Cascading)
 *
 * <!--#include file="streamedit-header.html.in"-->
 *
 * Note) The included file can be marked by relative paths on the basis of the location where CGI is executed. Or it may be marked by system absolute paths.
 *
 * If you wish to use the SSI function only without replacing character strings, transmit the NULL value using the arg argument as follows:
 *
 * ex) qSedFile(NULL, "streamedit.html.in", stdout);
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
 *   Q_ENTRY *args = qEntry();
 *   args = args->putStr(args, "${NAME}", "The qDecoder Project");
 *   args = args->putStr(args, "${HOBBY}", "Open Source Project");
 *   qSedStr(args, "Your name is ${NAME} and hobby is ${HOBBY}.", stdout);
 *   args->free(args);
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
				sp = (endp + strlen(SSI_INCLUDE_END));

				if (qFileExist(ssi_inc_file) == true) qSedFile(entry, ssi_inc_file, fpout);
				else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
			} else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
			continue;
		}

		/* Pattern Matching */
		int flag = 0;
 		Q_NLOBJ obj;
		memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
		entry->lock(entry);
 		while(entry->getNext(entry, &obj, NULL, false) == true) {
			if (!strncmp(sp, obj.name, strlen(obj.name))) {
				fprintf(fpout, "%s", (char *)obj.data);
				sp += strlen(obj.name);
				flag = 1;
				break;
			}
		}
 		entry->unlock(entry);

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
 *   Q_ENTRY *args = qEntry();
 *   args = args->putStr(args, "${NAME}", "The qDecoder Project");
 *   args = args->putStr(args, "${HOBBY}", "Open Source Project");
 *   qSedFile(args, "streamedit.html.in", stdout);
 *   args->free(args);
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
