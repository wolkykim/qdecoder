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
 * @file qEncode.c Encoding/decoding API
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "md5/md5_global.h"
#include "md5/md5.h"
#include "qDecoder.h"
#include "qInternal.h"

#ifdef __linux__
#include <iconv.h>
#endif

/**
 * Parse url query string
 */
Q_ENTRY *qDecodeQueryString(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count) {
	if(entry == NULL) {
		entry = qEntryInit();
		if(entry == NULL) return NULL;
	}

	char *newquery = NULL;
	int cnt = 0;

	if(query != NULL) newquery = strdup(query);
	while (newquery && *newquery) {
		char *value = _makeword(newquery, sepchar);
		char *name = qStrTrim(_makeword(value, equalchar));
		qDecodeUrl(name);
		qDecodeUrl(value);

		if(qEntryPutStr(entry, name, value, false) == true) cnt++;
		free(name);
		free(value);
	}
	if(newquery != NULL) free(newquery);
	if(count != NULL) *count = cnt;

	return entry;
}

/**********************************************
** Usage : qEncodeUrl(string to encode);
** Return: Pointer of encoded str which is memory allocated.
** Do    : Encode string.
**********************************************/
char *qEncodeUrl(const char *str) {
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
** Usage : qDecodeUrl(query pointer);
** Return: Pointer of query string.
** Do    : Decode query string.
**********************************************/
char *qDecodeUrl(char *str) {
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


/**
 * qCharEncode("ÇÑ±Û", "EUC-KR", "UTF-8", 2);
 *
 * @return malloced string pointer.
 */
#ifdef __linux__
char *qCharEncode(char *fromstr, char *fromcode, char *tocode, float mag) {
	char *tostr, *tp;
	size_t fromsize, tosize;

	if(fromstr == NULL) return NULL;

	fromsize = strlen(fromstr) + 1;
	tosize = (int)(mag * fromsize) + 1;
	tostr = tp = (char *)malloc(tosize);
	if(tostr == NULL) return NULL;

	iconv_t it = iconv_open(tocode, fromcode);
	if(it < 0) {
		DEBUG("iconv_open() failed. errno=%d", errno);
		return NULL;
	}
	int ret = iconv(it, &fromstr, &fromsize, &tostr, &tosize);
	if(ret < 0) {
		DEBUG("iconv() failed. errno=%d", errno);
		free(tostr);
		return NULL;
	}
	iconv_close(it);

	return tp;
}
#endif

/**********************************************
** Usage : qMd5Digest(string);
** Return: MD5 digested static string pointer.
** Do    : 16 bytes digest binary data through MD5 algorithm.
**********************************************/
unsigned char *qMd5Hash(char *data, int len) {
	static unsigned char digest[16 + 1];
	MD5_CTX context;

	MD5Init(&context);
	MD5Update(&context, data, len);
	MD5Final(digest, &context);

	// for safety
	digest[16] = '\0';
	return digest;
}

/**********************************************
** Usage : qMd5Str(string);
** Return: MD5 digested static string pointer.
** Do    : Strng converted digest string through MD5 algorithm.
**********************************************/
char *qMd5Str(char *data, int len) {
	static char md5hex[16 * 2 + 1];
	unsigned char *digest;
	int i;

	digest = qMd5Hash(data, len);
	for (i = 0; i < 16; i++) {
		sprintf(md5hex + (i * 2), "%02x", digest[i]);
	}

	return md5hex;
}

/**********************************************
** Usage : qMd5File(filepath);
** Return: MD5 digested static string pointer.
** Do    : Strng converted digest string through MD5 algorithm.
**********************************************/
char *qMd5File(char *filepath) {
	static char md5hex[16 * 2 + 1];
	int f, i;
	off_t n;
	struct stat stbuf;
	unsigned char szBuffer[256*1024], szDigest[16];
	MD5_CTX context;

	MD5Init(&context);
	if ((f = open(filepath, O_RDONLY)) < 0)
		return NULL;
	if (fstat(f, &stbuf) < 0)
		return NULL;
	n = stbuf.st_size;
	i = 0;
	while (n > 0) {
		if (n > sizeof(szBuffer))
			i = read(f, szBuffer, sizeof(szBuffer));
		else
			i = read(f, szBuffer, n);
		if (i < 0)
			break;
		MD5Update(&context, szBuffer, i);
		n -= i;
	}
	close(f);
	if (i < 0)
		return NULL;

	(void)MD5Final(szDigest, &context);

	if (md5hex == NULL)
		return NULL;

	for (i = 0; i < 16; i++) {
		sprintf(md5hex + (i * 2), "%02x", szDigest[i]);
	}

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
