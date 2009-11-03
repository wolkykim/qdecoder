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
 * @file qSocket.c Socket Handling API
 */

#ifndef DISABLE_SOCKET
#ifndef _WIN32

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "qDecoder.h"
#include "qInternal.h"

#if defined(ENABLE_SENDFILE) && defined(__linux__)
#include <sys/sendfile.h>
#define MAX_SENDFILE_CHUNK_SIZE		(1 * 1024 * 1024 * 1024)
#else
#define MAX_SENDFILE_CHUNK_SIZE		(64 * 1024)
#endif
#define MAX_SAVEINTOFILE_CHUNK_SIZE	(64 * 1024)

static bool _getAddr(struct sockaddr_in *addr, const char *hostname, int port);
/**
 * Create a TCP socket for the remote host and port.
 *
 * @param	hostname	remote hostname
 * @param	port		remote port
 * @param	timeoutms	wait timeout milliseconds. if set to negative value, wait indefinitely.
 *
 * @return	the new socket descriptor, or -1 in case of invalid hostname, -2 in case of socket creation failure, -3 in case of connection failure.
 *
 * @since	8.1R
 */
int qSocketOpen(const char *hostname, int port, int timeoutms) {
	/* host conversion */
	struct sockaddr_in addr;
	if (_getAddr(&addr, hostname, port) == false) return -1; /* invalid hostname */

	/* create new socket */
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2; /* sockfd creation fail */

	/* set to non-block socket*/
	int flags = fcntl(sockfd, F_GETFL, 0);
	if(timeoutms >= 0) fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	/* try to connect */
	int status = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if( status < 0 && (errno != EINPROGRESS || qSocketWaitWritable(sockfd, timeoutms) <= 0) ) {
		qSocketClose(sockfd);
		return -3; /* connection failed */
	}

	/* restore to block socket */
	if(timeoutms >= 0) fcntl(sockfd, F_SETFL, flags);

	return sockfd;
}

/**
 * Close socket.
 *
 * @param	sockfd	socket descriptor
 *
 * @return	true on success, or false if an error occurred.
 *
 * @since	8.1R
 */
bool qSocketClose(int sockfd) {
	if(close(sockfd) == 0) return true;
	return false;
}

/**
 * Wait until the socket has some readable data.
 *
 * @param	sockfd		socket descriptor
 * @param	timeoutms	wait timeout milliseconds. if set to negative value, wait indefinitely.
 *
 * @return	1 on readable, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 */
int qSocketWaitReadable(int sockfd, int timeoutms) {
	struct pollfd fds[1];

	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	int status = poll(fds, 1, timeoutms);
	if(status <= 0) return status;

	if(fds[0].revents & POLLIN) return 1;
	return -1;
}

/**
 * Wait until the socket is writable.
 *
 * @param	sockfd		socket descriptor
 * @param	timeoutms	wait timeout mili-seconds. if set to negative value, wait indefinitely.
 *
 * @return	1 on writable, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 */
int qSocketWaitWritable(int sockfd, int timeoutms) {
	struct pollfd fds[1];

	fds[0].fd = sockfd;
	fds[0].events = POLLOUT;

	int status = poll(fds, 1, timeoutms);
	if(status <= 0) return status;

	if(fds[0].revents & POLLOUT) return 1;
	return -1;
}

/**
 * Read data from socket.
 *
 * @param	binary		data buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		read size
 * @param	timeoutms	wait timeout milliseconds
 *
 * @return	the length of data readed on success, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 */
ssize_t qSocketRead(void *binary, int sockfd, size_t nbytes, int timeoutms) {
	if(nbytes == 0) return 0;

	ssize_t readbytes = 0;
	int sockstatus = 0;
	while(readbytes < nbytes) {
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if(sockstatus <= 0) break;

		ssize_t readsize = read(sockfd, binary+readbytes, nbytes-readbytes);
		if(readsize <= 0) {
			sockstatus = -1;
			break;
		}
		readbytes += readsize;
	}

	if(readbytes > 0) return readbytes;
	return sockstatus;
}

/**
 * Read line from the stream.
 * New-line characters(CR, LF ) will not be stored into buffer.
 *
 * @param	str		data buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		read size
 * @param	timeoutms	wait timeout milliseconds
 *
 * @return	the length of data readed on success, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	be sure the return length is not the length of stored data.
 *		it means how many bytes are readed from the socket.
 *		so the new-line characters will be counted, but not be stored.
 */
ssize_t qSocketGets(char *str, int sockfd, size_t nbytes, int timeoutms) {
	char *ptr;
	ssize_t readcnt = 0;
	for (ptr = str; readcnt < (nbytes - 1); ptr++) {
		int sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) {
			*ptr = '\0';
			return sockstatus;
		}

		if (read(sockfd, ptr, 1) != 1) {
			if (ptr == str) return -1;
			break;
		}

		readcnt++;
		if (*ptr == '\n') break;
		if (*ptr == '\r') ptr--;
	}

	*ptr = '\0';
	return readcnt;
}

/**
 * Send string or binary data to socket.
 *
 * @param	sockfd		socket descriptor
 * @param	binary		data pointer
 * @param	nbytes		sending size
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 */
ssize_t qSocketWrite(int sockfd, const void *binary, size_t nbytes) {
	return _q_write(sockfd, binary, nbytes);
}

/**
 * Send string with newline characters(CRLF) to socket.
 *
 * @param	sockfd		socket descriptor
 * @param	str		string pointer
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 */
ssize_t qSocketPuts(int sockfd, const char *str) {
	char *buf = (char *)malloc(strlen(str) + 2 + 1);
	if(buf == NULL) return -1;
	sprintf(buf, "%s\r\n", str);

	ssize_t sent = _q_write(sockfd, buf, strlen(buf));
	free(buf);

	return sent;
}

/**
 * Send formatted string to socket.
 *
 * @param	sockfd		socket descriptor
 * @param	format		variable argument lists
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 */
ssize_t qSocketPrintf(int sockfd, const char *format, ...) {
	char *buf;
	DYNAMIC_VSPRINTF(buf, format);
	if(buf == NULL) return -1;

	ssize_t ret = _q_write(sockfd, buf, strlen(buf));
	free(buf);
	return ret;
}

/**
 * Send file to socket.
 *
 * @param	sockfd		socket descriptor (out)
 * @param	fd		file descriptor (in)
 * @param	offset		file offset to send
 * @param	nbytes		total bytes to send. 0 means send data until EOF.
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 */
off_t qSocketSendfile(int sockfd, int fd, off_t offset, off_t nbytes) {
	struct stat filestat;
	if (fstat(fd, &filestat) < 0) return -1;

	off_t sent = 0;					// total size sent
	off_t rangesize = filestat.st_size - offset;	// maximum available size can be sent
	if(rangesize < 0) rangesize = 0;
	else if(nbytes > 0 && nbytes < rangesize) rangesize = nbytes; // set rangesize to requested size

#if !(defined(ENABLE_SENDFILE) && defined(__linux__))
	if (offset > 0) lseek(fd, offset, SEEK_SET);
#endif

	while(sent < rangesize) {
		size_t sendsize;	// this time sending size
		if(rangesize - sent <= MAX_SENDFILE_CHUNK_SIZE) sendsize = rangesize - sent;
		else sendsize = MAX_SENDFILE_CHUNK_SIZE;

		ssize_t ret = 0;
#if defined(ENABLE_SENDFILE) && defined(__linux__)
		ret = sendfile(sockfd, fd, &offset, sendsize);
#else
		ret = qFileSend(sockfd, fd, sendsize);
#endif
		if(ret <= 0) break; // Connection closed by peer
		sent += ret;
	}

	return sent;
}

/**
 * Store socket data into file directly.
 *
 * @param	fd		file descriptor (out)
 * @param	sockfd		socket descriptor (in)
 * @param	nbytes		length of bytes to read from socket
 * @param	timeoutms	wait timeout milliseconds
 *
 * @return	the number of bytes wrote on success, -1 if an error(ex:socket closed, file open failed) occurred.
 *
 * @since	8.1R
 *
 * @note	timeoutms is not the total retrieving time.
 *		only affected if no data reached to socket until timeoutms reached.
 *		if some data are received, it will wait until timeoutms reached again.
 * @code
 *   qSocketSaveIntoFile(fd, sockfd, 100, 5000);
 * @endcode
 */
off_t qSocketSaveIntoFile(int fd, int sockfd, off_t nbytes, int timeoutms) {
	if(nbytes <= 0) return 0;

	off_t readbytes, readed;
	for (readbytes = 0; readbytes < nbytes; readbytes += readed) {
		// calculate reading size
		int readsize;
		if (nbytes - readbytes < MAX_SAVEINTOFILE_CHUNK_SIZE) readsize = nbytes - readbytes;
		else readsize = MAX_SAVEINTOFILE_CHUNK_SIZE;

		// read data
		if (qSocketWaitReadable(sockfd, timeoutms) <= 0) {
			if(readbytes == 0) return -1;
			break;
		}

		readed = qFileSend(fd, sockfd, readsize);
		if (readed <= 0) {
			if(readbytes == 0) return -1;
			break;
		}
	}
	return readbytes;
}

/**
 * Store socket data into memory directly.
 *
 * @param	mem		memory buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		length of bytes to read from socket
 * @param	timeoutms	wait timeout milliseconds
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	timeoutms is not the total retrieving time.
 *		only affected if no data reached to socket until timeoutms reached.
 *		if some data are received, it will wait until timeoutms reached again.
 */
ssize_t qSocketSaveIntoMemory(char *mem, int sockfd, size_t nbytes, int timeoutms) {
	if(nbytes <= 0) return 0;

	char *mp;
	ssize_t readbytes, readed;
	for (readbytes = 0, mp = mem; readbytes < nbytes; readbytes += readed, mp += readed) {
		// calculate reading size
		size_t readsize = nbytes - readbytes;

		// wait data
		if (qSocketWaitReadable(sockfd, timeoutms) <= 0) {
			if(readbytes == 0) return -1;
			break;
		}

		// read data
		readed = read(sockfd, mp, readsize);
		if (readed <= 0) {
			if(readbytes == 0) return -1;
			break;
		}
	}

	return readbytes;
}

/**
 * Convert hostname to sockaddr_in structure.
 *
 * @param	addr		sockaddr_in structure pointer
 * @param	hostname	IP string address or hostname
 * @param	port		port number
 *
 * @return	true if successful, otherwise returns false.
 *
 * @since	8.1R
 */
static bool _getAddr(struct sockaddr_in *addr, const char *hostname, int port) {
	/* here we assume that the hostname argument contains ip address */
	memset((void*)addr, 0, sizeof(struct sockaddr_in));
	if (!inet_aton(hostname, &addr->sin_addr)) { /* fail then try another way */
		struct hostent *hp;
		if ((hp = gethostbyname (hostname)) == 0) return false;
		memcpy (&addr->sin_addr, hp->h_addr, sizeof(struct in_addr));
	}
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	return true;
}

#endif /* _WIN32 */
#endif /* DISABLE_SOCKET */
