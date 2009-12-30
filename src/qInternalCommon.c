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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include "qDecoder.h"
#include "qInternal.h"

// exit with failure status
void _q_die(const char *errstr) {
	fprintf(stderr, "%s\n", errstr);
	exit(EXIT_FAILURE);
}

// exit when allocation is failed
void *_q_malloc(size_t size) {
	void *mem = malloc(size);
	if(mem == NULL) {
		_q_die("malloc");
	}
	return mem;
}

// Change two hex character to one hex value.
char _q_x2c(char hex_up, char hex_low) {
	char digit;

	digit = 16 * (hex_up >= 'A' ? ((hex_up & 0xdf) - 'A') + 10 : (hex_up - '0'));
	digit += (hex_low >= 'A' ? ((hex_low & 0xdf) - 'A') + 10 : (hex_low - '0'));

	return digit;
}

char *_q_makeword(char *str, char stop) {
	char *word;
	int  len, i;

	for (len = 0; ((str[len] != stop) && (str[len])); len++);
	word = (char *)malloc(sizeof(char) * (len + 1));

	for (i = 0; i < len; i++) word[i] = str[i];
	word[i] = '\0';

	if (str[len])len++;
	for (i = len; str[i]; i++) str[i - len] = str[i];
	str[i - len] = '\0';

	return word;
}

// This function is perfectly same as fgets();
char *_q_fgets(char *str, int size, FILE *stream) {
	int c;
	char *ptr;

	for (ptr = str; size > 1; size--) {
		c = fgetc(stream);
		if (c == EOF) break;
		*ptr++ = (char)c;
		if (c == '\n') break;
	}

	*ptr = '\0';
	if (strlen(str) == 0) return NULL;

	return str;
}

ssize_t _q_write(int fd, const void *buf, size_t nbytes) {
	if(nbytes == 0) return 0;

	ssize_t sent = 0;

	while(sent < nbytes) {
		int status = qSocketWaitWritable(fd, 1000);
		if(status == 0) continue;
		else if(status < 0) break;

		ssize_t wsize = write(fd, buf+sent, nbytes-sent);
		if(wsize <= 0) break;
		sent += wsize;
	}

	if(sent > 0) return sent;
	return -1;
}

ssize_t _q_writef(int fd, const char *format, ...) {
	char *buf;
	DYNAMIC_VSPRINTF(buf, format);
	if(buf == NULL) return -1;

	ssize_t ret = _q_write(fd, buf, strlen(buf));
	free(buf);
	return ret;
}

/* win32 compatible */
int _q_unlink(const char *pathname) {
#ifdef _WIN32
	return _unlink(pathname);
#endif
	return unlink(pathname);
}
