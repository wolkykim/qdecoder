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
#include "md5/md5_global.h"
#include "md5/md5.h"

/**********************************************
** Usage : qURLencode(string to encode);
** Return: Pointer of encoded str which is memory allocated.
** Do    : Encode string.
**********************************************/
char *qURLencode(char *str) {
	char *encstr, buf[2+1];
	unsigned char c;
	int i, j;

	if (str == NULL) return NULL;
	if ((encstr = (char *)malloc((strlen(str) * 3) + 1)) == NULL) return NULL;

	for (i = j = 0; str[i]; i++) {
		c = (unsigned char)str[i];
		if ((c >= '0') && (c <= '9')) encstr[j++] = c;
		else if ((c >= 'A') && (c <= 'Z')) encstr[j++] = c;
		else if ((c >= 'a') && (c <= 'z')) encstr[j++] = c;
		else if ((c == '@') || (c == '.') || (c == '/') || (c == '\\')
		         || (c == '-') || (c == '_') || (c == ':') ) encstr[j++] = c;
		else {
			sprintf(buf, "%02x", c);
			encstr[j++] = '%';
			encstr[j++] = buf[0];
			encstr[j++] = buf[1];
		}
	}
	encstr[j] = '\0';

	return encstr;
}

/**********************************************
** Usage : qURLdecode(query pointer);
** Return: Pointer of query string.
** Do    : Decode query string.
**********************************************/
char *qURLdecode(char *str) {
	int i, j;

	if (!str) return NULL;
	for (i = j = 0; str[j]; i++, j++) {
		switch (str[j]) {
			case '+': {
				str[i] = ' ';
				break;
			}
			case '%': {
				str[i] = _x2c(str[j + 1], str[j + 2]);
				j += 2;
				break;
			}
			default: {
				str[i] = str[j];
				break;
			}
		}
	}
	str[i] = '\0';

	return str;
}

/**********************************************
** Usage : qMD5Str(string);
** Return: MD5 digested static string pointer.
** Do    : Digest string through MD5 algorithm.
**********************************************/
char *qMD5Str(char *string) {
	MD5_CTX context;
	unsigned char digest[16];
	static char md5hex[16 * 2 + 1];
	int i;

	MD5Init(&context);
	MD5Update(&context, string, strlen(string));
	MD5Final(digest, &context);

	for (i = 0; i < 16; i++) {
		sprintf(md5hex + (i * 2), "%02x", digest[i]);
	}

	return md5hex;
}

/**********************************************
** Usage : qMD5File(filename);
** Return: MD5 digested static string pointer.
** Do    : Digest string through MD5 algorithm.
**********************************************/
char *qMD5File(char *filename) {
	char *sp, *md5hex;

	if ((sp = qReadFile(filename, NULL)) == NULL) return NULL;
	md5hex = qMD5Str(sp);
	free(sp);

	return md5hex;
}

/**********************************************
** Usage : qFnv32Hash(string, max);
** Return: unsigned 32 integer of FNV hash algorithm.
** Do    : FNV hash algorithm
**         if set max to 0, disable maximum limit
**********************************************/
unsigned int qFnv32Hash(char *str, unsigned int max) {
	unsigned char *s = (unsigned char *)str;
	unsigned int hval = (unsigned int)0x811c9dc5;

	while (*s) {
		hval *=  (unsigned int)0x01000193;
		hval ^= (unsigned int) * s++;
	}
	if (max > 0) hval %= max;

	return hval;
}
