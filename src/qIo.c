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
 * @file qIo.c I/O Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "qDecoder.h"
#include "qInternal.h"

#define MAX_IOSEND_SIZE		(32 * 1024)

/**
 * Test & wait until the file descriptor has readable data.
 *
 * @param	fd		file descriptor
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	1 if readable, 0 on timeout, -1 if an error occurred.
 *
 * @note
 * The argument timeoutms can be used to set maximum wait time for a socket descriptor.
 * In terms of general file descriptor, timeoutms may normally 0.
 */
int qIoWaitReadable(int fd, int timeoutms) {
	struct pollfd fds[1];

	fds[0].fd = fd;
	fds[0].events = POLLIN;

	int pollret = poll(fds, 1, timeoutms);
	if(pollret <= 0) return pollret;

	if(fds[0].revents & POLLIN) return 1;
	return -1;
}

/**
 * Test & wait until the file descriptor is ready for writing.
 *
 * @param	fd		file descriptor
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	1 if writable, 0 on timeout, -1 if an error occurred.
 */
int qIoWaitWritable(int fd, int timeoutms) {
	struct pollfd fds[1];

	fds[0].fd = fd;
	fds[0].events = POLLOUT;

	int pollret = poll(fds, 1, timeoutms);
	if(pollret <= 0) return pollret;

	if(fds[0].revents & POLLOUT) return 1;
	return -1;
}

/**
 * Read from a file descriptor.
 *
 * @param	buf		data buffer pointer to write to
 * @param	fd		file descriptor
 * @param	nbytes		the number of bytes to read from file descriptor & write into buffer
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	the number of bytes read if successful, otherwise returns -1.
 */
ssize_t qIoRead(void *buf, int fd, size_t nbytes, int timeoutms) {
	if(nbytes == 0) return 0;

	ssize_t total  = 0;
	while(total < nbytes) {
		if(qIoWaitReadable(fd, timeoutms) <= 0) break;
		errno = 0;
		ssize_t rsize = read(fd, buf + total, nbytes - total);
		if(rsize <= 0) {
			if(errno == EAGAIN || errno == EINPROGRESS) {
				// possible with non-block io
				usleep(1);
				continue;
			}
			break;
		}
		total += rsize;
	}

	if(total > 0) return total;
	return -1;
}

/**
 * Write to a file descriptor.
 *
 * @param	fd		file descriptor
 * @param	buf		data buffer pointer to read from
 * @param	nbytes		the number of bytes to write to file descriptor & read from buffer
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	the number of bytes written if successful, otherwise returns -1.
 */
ssize_t qIoWrite(int fd, const void *buf, size_t nbytes, int timeoutms) {
	if(nbytes == 0) return 0;

	ssize_t total  = 0;
	while(total < nbytes) {
		if(qIoWaitWritable(fd, timeoutms) <= 0) break;
		errno = 0;
		ssize_t wsize = write(fd, buf + total, nbytes - total);
		if(wsize <= 0){
			if(errno == EAGAIN || errno == EINPROGRESS) {
				// possible with non-block io
				usleep(1);
				continue;
			}
			break;
		}
		total += wsize;
	}

	if(total > 0) return total;
	return -1;
}

/**
 * Transfer data between file descriptors
 *
 * @param	outfd		output file descriptor
 * @param	infd		input file descriptor
 * @param	nbytes		the number of bytes to copy between file descriptors. 0 means transfer until end of infd.
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	the number of bytes written if successful, otherwise returns -1.
 */
off_t qIoSend(int outfd, int infd, off_t nbytes, int timeoutms) {
	if(nbytes == 0) return 0;

	unsigned char buf[MAX_IOSEND_SIZE];

	off_t total = 0; // total size sent
	while(total < nbytes) {
		size_t chunksize; // this time sending size
		if(nbytes - total <= sizeof(buf)) chunksize = nbytes - total;
		else chunksize = sizeof(buf);

		// read
		ssize_t rsize = qIoRead(buf, infd, chunksize, timeoutms);
		if (rsize <= 0) break;
		DEBUG("read %zd", rsize);

		// write
		ssize_t wsize = qIoWrite(outfd, buf, rsize, timeoutms);
		if(wsize <= 0) break;
		DEBUG("write %zd", wsize);

		total += wsize;
		if(rsize != wsize) {
			DEBUG("size mismatch. read:%zd, write:%zd", rsize, wsize);
			break;
		}
	}

	if(total > 0) return total;
	return -1;
}

/**
 * Read a line from a file descriptor into the buffer pointed to until either a terminating newline or EOF.
 * New-line characters(CR, LF ) will not be stored into buffer.
 *
 * @param	buf		data buffer pointer
 * @param	bufsize		buffer size
 * @param	fd		file descriptor
 * @param	timeoutms	wait timeout milliseconds
 *
 * @return	the number of bytes read from file descriptor if successful, otherwise returns -1.
 *
 * @note	Be sure the return value does not mean the length of actual stored data.
 *		It means how many bytes are readed from the file descriptor,
 *		so the new-line characters will be counted, but not be stored.
 */
ssize_t qIoGets(char *buf, size_t bufsize, int fd, int timeoutms) {
	if(bufsize <= 1) return -1;

	ssize_t readcnt = 0;
	char *ptr;
	for (ptr = buf; readcnt < (bufsize - 1); ptr++) {
		if(qIoWaitReadable(fd, timeoutms) <= 0) break;
		errno = 0;
		ssize_t rsize = read(fd, ptr, 1);
		if(rsize != 1) {
			if(errno == EAGAIN || errno == EINPROGRESS) {
				// possible with non-block io
				usleep(1);
				continue;
			}
			break;
		}

		readcnt++;
		if (*ptr == '\r') ptr--;
		else if (*ptr == '\n') break;
	}

	*ptr = '\0';

	if(readcnt > 0) return readcnt;
	return -1;
}

/**
 * Writes the string and a trailing newline to file descriptor.
 *
 * @param	fd		file descriptor
 * @param	str		string pointer
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 *
 * @return	the number of bytes written including trailing newline characters if successful, otherwise returns -1.
 */
ssize_t qIoPuts(int fd, const char *str, int timeoutms) {
	return qIoPrintf(fd, timeoutms, "%s\n", str);
}

/**
 * Formatted output to a file descriptor
 *
 * @param	fd		file descriptor
 * @param	timeoutms	wait timeout milliseconds. 0 for no wait, -1 for infinite wait
 * @param	format		format string
 *
 * @return	the number of bytes written if successful, otherwise returns -1.
 */
ssize_t qIoPrintf(int fd, int timeoutms, const char *format, ...) {
	char *buf;
	DYNAMIC_VSPRINTF(buf, format);
	if(buf == NULL) return -1;

	ssize_t ret = qIoWrite(fd, buf, strlen(buf), timeoutms);
	free(buf);

	return ret;
}
