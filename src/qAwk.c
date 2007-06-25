/************************************************************************
qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org

Copyright (C) 2001 The qDecoder Project.
Copyright (C) 1999,2000 Hongik Internet, Inc.
Copyright (C) 1998 Nobreak Technologies, Inc.
Copyright (C) 1996,1997 Seung-young Kim.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


Copyright Disclaimer:
  Hongik Internet, Inc., hereby disclaims all copyright interest.
  President, Christopher Roh, 6 April 2000

  Nobreak Technologies, Inc., hereby disclaims all copyright interest.
  President, Yoon Cho, 6 April 2000

  Seung-young Kim, hereby disclaims all copyright interest.
  Author, Seung-young Kim, 6 April 2000

Author:
  Seung-young Kim <wolkykim(at)ziom.co.kr>
************************************************************************/

#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Usage : qAwkOpen(filename);
** Return: Success returns file pointer, Fail returns NULL.
** Do    : Open file to scan pattern. (similar to unix command awk)
**
** ex) qAwkOpen("source.dat", ':');
**********************************************/
FILE *qAwkOpen(char *filename) {
	FILE *fp;

	if (fp != NULL) qError("qAwkOpen(): There is already opened handle.");
	if ((fp = fopen(filename, "r")) == NULL) return NULL;
	return fp;
}

/**********************************************
** Usage : qAwkNext(array, field-delimeter);
** Return: Success number of field, End of file -1.
** Do    : Scan one line. (Unlimited line length)
**
** ex) char array[10][1024];
**     qAwkNext(array);
**********************************************/
int qAwkNext(FILE *fp, char array[][1024], char delim) {
	char *buf;
	int num;

	if (fp == NULL) qError("qAwkNext(): There is no opened handle.");
	if ((buf = qRemoveSpace(qfGetLine(fp))) == NULL) return -1;
	num = qAwkStr(array, buf, delim);
	free(buf);

	return num;
}

/**********************************************
** Usage : qAwkClose();
** Return: Success Q_TRUE, Otherwise Q_FALSE.
** Do    : Close file.
**********************************************/
Q_BOOL qAwkClose(FILE *fp) {
	if (fp == NULL) return Q_FALSE;
	if(fclose(fp) == 0) return Q_TRUE;
	return Q_FALSE;
}

/**********************************************
** Usage : qAwkStr(**array, srouce_string_pointer, );
** Return: returns the number of parsed fields.
** Do    : scan pattern from the string.
**********************************************/
int qAwkStr(char array[][1024], char *str, char delim) {
	char *bp1, *bp2;
	int i, exitflag;

	for (i = exitflag = 0, bp1 = bp2 = str; exitflag == 0; i++) {
		for (; *bp2 != delim && *bp2 != '\0'; bp2++);
		if (*bp2 == '\0') exitflag = 1;
		*bp2 = '\0';
		strcpy(array[i], bp1);
		bp1 = ++bp2;
	}

	return i;
}
