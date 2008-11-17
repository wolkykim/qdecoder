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

/**
 * Get MD5 digested HEX string
 *
 * @param data		source object
 * @param nbytes	size of data
 *
 * @return		malloced 16+1 bytes digested HEX string which is terminated by NULL character
 *
 * @code
 *   unsigned char *md5 = qHashMd5((void*)"hello", 5);
 *   free(md5);
 * @endcode
 */
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

/**
 * Get MD5 digested ASCII string
 *
 * @param data		source object
 * @param nbytes	size of data
 *
 * @return		malloced 32+1 bytes digested ASCII string which is terminated by NULL character
 *
 * @code
 *   char *md5str = qHashMd5Str((void*)"hello", 5);
 *   printf("%s\n", md5str);
 *   free(md5str);
 * @endcode
 */
char *qHashMd5Str(const void *data, size_t nbytes) {
	if(data == NULL) return NULL;

	unsigned char *digest = qHashMd5(data, nbytes);
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

/**
 * Get MD5 digested ASCII string
 *
 * @param filepath	file path
 * @param nbytes	size of data. Set to NULL to digest end of file
 *
 * @return		malloced 32+1 bytes digested ASCII string which is terminated by NULL character
 *
 * @note
 * If the nbytes is set, qHashMd5File() will try to digest at lease nbytes then
 * store actual digested size into nbytes. So if you set nbytes to over size than file size,
 * finally nbytes will have actual file size.
 *
 * @code
 *   // case of digesting entire file
 *   char *md5str = qHashMd5File("/tmp/test.dat, NULL);
 *   printf("%s\n", md5str);
 *   free(md5str);
 *
 *   // case of nbytes is set to 1 bytes length
 *   size_t nbytes = 1;
 *   char *md5str = qHashMd5File("/tmp/test.dat, &nbytes);
 *   printf("%s %d\n", md5str, nbytes);
 *   free(md5str);
 *
 *   // case of nbytes is set to over size
 *   size_t nbytes = 100000;
 *   char *md5str = qHashMd5File("/tmp/test.dat, &nbytes);
 *   printf("%s %d\n", md5str, nbytes);
 *   free(md5str);
 * @endcode
 */
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

/**
 * Get FNV32 hash integer
 *
 * @param max		hash range. 0 ~ (max-1). Set to 0 for full range
 * @param data		source data
 * @param nbytes	size of data
 *
 * @return		FNV32 hash integer
 *
 * @code
 *   unsigned int fnv32;
 *
 *   fnv32 = qHashFnv32(0, (void*)"hello", 5);
 *   printf("%u\n", fnv32);
 *
 *   fnv32 = qHashFnv32(1000, (void*)"hello", 5);
 *   printf("%u\n", fnv32);
 * @endcode
 */
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
