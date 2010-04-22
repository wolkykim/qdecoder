/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
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
 *
 * $Id$
 */

/**
 * @file qUtil.c Miscellaneous utilities API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Read counter(integer) from file with advisory file locking.
 *
 * @param filepath	file path
 *
 * @return		counter value readed from file. in case of failure, returns 0.
 *
 * @code
 *   qCountSave("number.dat", 75);
 *   int count = qCountRead("number.dat");
 * @endcode
 *
 * @code
 *   ---- number.dat ----
 *   75
 *   --------------------
 * @endcode
 */
int qCountRead(const char *filepath) {
	int fd = open(filepath, O_RDONLY, 0);
	if(fd < 0) return 0;

	char buf[10+1];
	ssize_t readed = qIoRead(buf, fd, (sizeof(buf) - 1), 0);
	close(fd);

	if(readed > 0) {
		buf[readed] = '\0';
		return atoi(buf);
	}
	return 0;
}

/**
 * Save counter(integer) to file with advisory file locking.
 *
 * @param filepath	file path
 * @param number	counter integer value
 *
 * @return		true if successful, otherwise returns false.
 *
 * @note
 * @code
 *   qCountSave("number.dat", 75);
 * @endcode
 */
bool qCountSave(const char *filepath, int number) {
	int fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE);
	if(fd < 0) return false;

	ssize_t updated = qIoPrintf(fd, 0, "%d", number);
	close(fd);

	if(updated > 0) return true;
	return false;
}

/**
 * Increases(or decrease) the counter value as much as specified number
 * with advisory file locking.
 *
 * @param filepath	file path
 * @param number	how much increase or decrease
 *
 * @return		updated counter value. in case of failure, returns 0.
 *
 * @note
 * @code
 *   int count;
 *   count = qCountUpdate("number.dat", -3);
 * @endcode
 */
int qCountUpdate(const char *filepath, int number) {
	int counter = qCountRead(filepath);
	counter += number;
	if(qCountSave(filepath, counter) == true) return counter;
	return 0;
}

#define	SED_INCLUDE_START	"<!--#include file=\""
#define SED_INCLUDE_END		"\"-->"
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
		if (!strncmp(sp, SED_INCLUDE_START, strlen(SED_INCLUDE_START))) {
			char ssi_inc_file[MAX_LINEBUF], *endp;
			if ((endp = strstr(sp, SED_INCLUDE_END)) != NULL) {
				sp += strlen(SED_INCLUDE_START);
				strncpy(ssi_inc_file, sp, endp - sp);
				ssi_inc_file[endp - sp] = '\0';
				sp = (endp + strlen(SED_INCLUDE_END));

				if (qFileExist(ssi_inc_file) == true) qSedFile(entry, ssi_inc_file, fpout);
				else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
			} else fprintf(fpout, "[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
			continue;
		}

		/* Pattern Matching */
		int flag = 0;

		if(entry != NULL) {
 			Q_NLOBJ_T obj;
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
		}

		if (flag == 0) {
			fputc(*sp, fpout);
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
