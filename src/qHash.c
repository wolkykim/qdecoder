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
 * @file qHash.c Encoding/decoding API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "md5/md5_global.h"
#include "md5/md5.h"
#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Usage : qMd5Digest(string);
** Return: MD5 digested static string pointer.
** Do    : 16 bytes digest binary data through MD5 algorithm.
**********************************************/
unsigned char *qHashMd5(const void *data, size_t nbytes) {
	if(data == NULL) return NULL;

	unsigned char *digest = (unsigned char*)malloc(sizeof(char) * (16 + 1));
	if (digest == NULL) return NULL;

	MD5_CTX context;
	MD5Init(&context);
	MD5Update(&context, (unsigned char*)data, (unsigned int)nbytes);
	MD5Final(digest, &context);
	digest[16] = '\0';

	return digest;
}

/**********************************************
** Usage : qMd5Str(string);
** Return: MD5 digested static string pointer.
** Do    : Strng converted digest string through MD5 algorithm.
**********************************************/
char *qHashMd5Str(const char *str, size_t nbytes) {
	if(str == NULL) return NULL;

	unsigned char *digest = qHashMd5(str, nbytes);
	if (digest == NULL) return NULL;

	char *md5hex = (char*)malloc(sizeof(char) * (16 * 2 + 1));
	if (md5hex == NULL) return NULL;

	int i;
	for (i = 0; i < 16; i++) {
		sprintf(md5hex + (i * 2), "%02x", digest[i]);
	}
	free(digest);

	return md5hex;
}

/**********************************************
** Usage : qMd5File(filepath);
** Return: MD5 digested static string pointer.
** Do    : Strng converted digest string through MD5 algorithm.
**********************************************/
char *qHashMd5File(const char *filepath, size_t *nbytes) {
	int fd = open(filepath, O_RDONLY, 0);
	if (fd < 0) return NULL;

	struct stat st;
	if (fstat(fd, &st) < 0) return NULL;

	size_t size = st.st_size;
	if(nbytes != NULL) {
		if(*nbytes > size) *nbytes = size;
		else size = *nbytes;
	}

	MD5_CTX context;
	MD5Init(&context);
	ssize_t retr = 0;
	unsigned char buf[256*1024], szDigest[16];
	while (size > 0) {
		if (size > sizeof(buf)) retr = read(fd, buf, sizeof(buf));
		else retr = read(fd, buf, size);
		if (retr < 0) break;
		MD5Update(&context, buf, retr);
		size -= retr;
	}
	close(fd);
	MD5Final(szDigest, &context);

	if(nbytes != NULL) *nbytes -= size;

	char *md5hex = (char*)malloc(sizeof(char) * (16 * 2 + 1));
	if (md5hex == NULL) return NULL;

	int i;
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
unsigned int qHashFnv32(unsigned int max, const void *data, size_t nbytes) {
	if(data == NULL) return 0;

	unsigned char *dp;
	unsigned int hval = (unsigned int)0x811c9dc5;

	for (dp = (unsigned char *)data; *dp && nbytes > 0; dp++, nbytes--) {
		hval *= (unsigned int)0x01000193;
		hval ^= (unsigned int)*dp;
	}
	if (max > 0) hval %= max;

	return hval;
}
