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
 * @file qAwk.c File & String Tokenizing API
 *
 * Plays a similar function to the AWK command of UNIX systems.
 *
 * @code
 *   char array[7][1024];
 *   qAwkOpen("/etc/passwd", ':');
 *   for( ; qAwkNext(array) > 0; ) printf("ID=%s, NAME=%s\n", array[0], array[4]);
 *   qAwkClose();
 * @endcode
 * @code
 *   [Sample /etc/passwd]
 *   qdecoder:x:1001:1000:The qDecoder Project:/home/qdecoder:/bin/csh
 *   wolkykim:x:1002:1000:Seung-young Kim:/home/wolkykim:/bin/csh
 *   ziom:x:1003:1000:Ziom Corporation:/home/kikuchi:/bin/csh
 *
 *   [Output]
 *   ID=qdecoder, NAME=The qDecoder Project
 *   ID=wolkykim, NAME=Seung-young Kim
 *   ID=ziom, NAME=Ziom Corporation
 * @endcode
 *
 * @note
 * Maximum token length(not a line or file length) is fixed to 1023(1024-1).
 * @code
 *   token1|token2...(length over 1023)...|tokenN
 * @endcode
 * token1 and tokenN is ok. But token2 will be stored only first 1023 bytes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qDecoder.h"

/**
 * Open file for tokenizing
 *
 * @param filename file path to open
 *
 * @return	file pointer. in case of failure, returns NULL.
 */
FILE *qAwkOpen(char *filename) {
	FILE *fp;

	if ((fp = fopen(filename, "r")) == NULL) return NULL;
	return fp;
}

/**
 * Read one line from opened file pointer to tokenize.
 *
 * @param fp	file pointer which is retruned by qAwkOpen()
 * @param array splitted tokens will be stored here
 * @param delim	delimeter
 *
 * @return	the number of parsed(stored in array) tokens.
 * 		otherwise(end of file or something), returns -1.
 */
int qAwkNext(FILE *fp, char array[][1024], char delim) {
	char *buf;
	int num;

	if (fp == NULL) qError("qAwkNext(): There is no opened handle.");
	if ((buf = qRemoveSpace(qfGetLine(fp))) == NULL) return -1;
	num = qAwkStr(array, buf, delim);
	free(buf);

	return num;
}

/**
 * Close opened FILE pointer.
 *
 * @param fp	file pointer which is retruned by qAwkOpen()
 *
 * @return	in case of success, returns true. otherwise false.
 */
bool qAwkClose(FILE *fp) {
	if (fp == NULL) return false;
	if(fclose(fp) == 0) return true;
	return false;
}

/**
 * String Tokenizer
 *
 * @param array splitted tokens will be stored here
 * @param str	string to tokenize
 * @param delim	delimeter
 *
 * @return	returns the number of parsed(stored in array) tokens.
 */
int qAwkStr(char array[][1024], char *str, char delim) {
	char *bp1, *bp2;
	int i, exitflag;

	for (i = exitflag = 0, bp1 = bp2 = str; exitflag == 0; i++) {
		for (; *bp2 != delim && *bp2 != '\0'; bp2++);
		if (*bp2 == '\0') exitflag = 1;
		*bp2 = '\0';
		strncpy(array[i], bp1, 1023);
		array[i][1023] = '\0';
		bp1 = ++bp2;
	}

	return i;
}
