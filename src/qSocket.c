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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "qDecoder.h"

#ifdef __linux__
#include <sys/sendfile.h>
#endif

/**********************************************
** Internal Functions Definition
**********************************************/

int _StrToAddr(struct sockaddr_in *addr, unsigned char family, char *hostname, int port);

/**
 * Create a TCP socket for the remote host and port.
 *
 * @param	hostname	remote hostname
 * @param	port		remote port
 *
 * @return	the new socket descriptor, or -1 in case of invalid hostname, -2 in case of socket creation failure.
 *
 * @since	8.1R
 *
 * @note
 * @code
 * @endcode
 */

int qSocketOpen(char *hostname, int port) {
	int sockfd;
	struct sockaddr_in addr;

	/* host conversion */
	bzero((char*)&addr, sizeof(addr));
	if (_StrToAddr(&addr, AF_INET, hostname, port) != 1) return -1; /* invalid hostname */

	/* make sockfd */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2; /* sockfd creation fail */

	/* connect */
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(sockfd);
		return -3; /* connection fail */
	}

	return sockfd;
}

/**
 * Close socket.
 *
 * @param	sockfd	socket descriptor returned by qSocketOpen()
 *
 * @return	true on success, or false if an error occurred.
 *
 * @since	8.1R
 *
 * @note
 * @code
 * @endcode
 */
bool qSocketClose(int sockfd) {
	if(close(sockfd) == 0) return true;
	return false;
}

/**
 * Wait until the socket has some readable data.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	timeoutms	wait timeout micro-seconds
 *
 * @return	1 on readable, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 * @code
 * @endcode
 */
int qSocketWaitReadable(int sockfd, int timeoutms) {
	struct timeval tv;
	fd_set readfds;

	// time to wait
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	if (timeoutms > 0) {
		tv.tv_sec = (timeoutms / 1000), tv.tv_usec = ((timeoutms % 1000) * 1000);
		if (select(FD_SETSIZE, &readfds, NULL, NULL, &tv) < 0) return -1;
	} else if (timeoutms == 0) { // just poll
		tv.tv_sec = 0, tv.tv_usec = 0;
		if (select(FD_SETSIZE, &readfds, NULL, NULL, &tv) < 0) return -1;
	} else { //  blocks indefinitely
		if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) return -1;
	}

	if (!FD_ISSET(sockfd, &readfds)) return 0; // timeout

	return 1;
}

/**
 * Read data from socket.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	binary		data buffer pointer
 * @param	size		data buffer size
 * @param	timeoutms	wait timeout micro-seconds
 *
 * @return	the length of data readed on success, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 * @code
 * @endcode
 */
int qSocketRead(int sockfd, char *binary, int size, int timeoutms) {
	int sockstatus, readcnt;

	sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;
	readcnt = read(sockfd, binary, size);
	if (readcnt <= 0) return -1;

	return readcnt;
}

/**
 * Read line from the stream.
 * New-line characters '\r', '\n' will not be stored into buffer.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	str		data buffer pointer
 * @param	size		data buffer size
 * @param	timeoutms	wait timeout micro-seconds
 *
 * @return	the length of data readed on success, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	be sure the return length is not the length of stored data.
 *		it means how many bytes are readed from the socket.
 *		so the new-line characters will be counted, but not be stored.
 * @code
 * @endcode
 */
int qSocketGets(int sockfd, char *str, int size, int timeoutms) {
	char *ptr;
	int sockstatus, readcnt = 0;

	for (ptr = str; readcnt < (size - 1); ptr++) {
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
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
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	binary		data pointer
 * @param	size		sending size
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
int qSocketWrite(int sockfd, char *binary, int size) {
	return write(sockfd, binary, size);
}

/**
 * Send string with newline characters(\r\n) to socket.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	str		string pointer
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
int qSocketPuts(int sockfd, char *str) {
	char *buf;

	buf = (char *)malloc(strlen(str) + 2 + 1);
	if(buf == NULL) return -1;
	sprintf(buf, "%s\r\n", str);

	int sent = write(sockfd, buf, strlen(buf));
	free(buf);

	return sent;
}

/**
 * Send formatted string to socket.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	format		variable argument lists
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	the final length of formatted string must be less than 1024
 *		If you need to send more huge string, use qSocketPuts instead.
 * @code
 * @endcode
 */
int qSocketPrintf(int sockfd, char *format, ...) {
	char buf[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	return write(sockfd, buf, strlen(buf));
}

#define MAX_SENDFILE_CHUNK_SIZE		(1024 * 1024)
/**
 * Send file to socket.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	filepath	variable argument lists
 * @param	offset		file offset to send
 * @param	size		total bytes to send. 0 means send data until EOF.
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
ssize_t qSocketSendFile(int sockfd, char *filepath, off_t offset, ssize_t size) {
	struct stat filestat;
	int filefd;

	if((filefd = open(filepath, O_RDONLY , 0))  < 0) return -1;
	if (fstat(filefd, &filestat) < 0) return -1;

	ssize_t sent = 0;				// total size sent
	ssize_t rangesize = filestat.st_size - offset;	// total size to send
	if(size > 0 && size < rangesize) rangesize = size;

#ifndef __linux__
	char buf[MAX_SENDFILE_CHUNK_SIZE];
	if (offset > 0) lseek(filefd, offset, SEEK_SET);
#endif

	while(sent < rangesize) {
		size_t sendsize;	// this time sending size
		if(rangesize - sent > MAX_SENDFILE_CHUNK_SIZE) sendsize = MAX_SENDFILE_CHUNK_SIZE;
		else sendsize = rangesize - sent;

#ifdef __linux__
		ssize_t ret = sendfile(sockfd, filefd, &offset, sendsize);
		if(ret <= 0) break; // Connection closed by peer
#else
		// read
		size_t retr = read(filefd, buf, sizeof(buf));
		if (retr == 0) break;

		// write
		ssize_t ret = write(sockfd, buf, retr);
		if(retw <= 0) break; // Connection closed by peer
#endif
		sent += ret;
	}

	close(filefd);
	return sent;
}

/**
 * Store socket data into file directly.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	size		length of bytes to read from socket
 * @param	filepath	save file path
 * @param	oflag		constructed by a bitwise-inclusive OR of flags defined in <fcntl.h>.
 * @param	timeoutms	wait timeout micro-seconds
 *
 * @return	the number of bytes sent on success, -1 if an error(ex:socket closed, file open failed) occurred.
 *
 * @since	8.1R
 *
 * @note	timeoutms is not the total retrieving time.
 *		only affected if no data reached to socket until timeoutms reached.
 *		if some data are received, it will wait until timeoutms reached again.
 * @code
 *   qSocketSaveIntoFile(sockfd, 100, "/tmp/file.tmp", O_CREAT|O_TRUNC|O_WRONLY, 5000);
 * @endcode
 */
int qSocketSaveIntoFile(int sockfd, int size, char *filepath, int oflag, int timeoutms) {
	int filefd;
	char buf[1024*32]; // read buffer size

	int sockstatus, readbytes, readed, readsize;

	// stream readable?
	sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;

	// file open
	if ((filefd = open(filepath, oflag)) < 0) return -1;

	for (readbytes = 0; readbytes < size; readbytes += readed) {

		// calculate reading size
		if (size - readbytes < sizeof(buf)) readsize = size - readbytes;
		else readsize = sizeof(buf);

		// read data
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) {
			if(readbytes == 0) {
				close(filefd);
				unlink(filepath);
				return -1;
			}
			break;

		}

		readed = read(sockfd, buf, readsize);
		if (readed == 0) break; // eof
		else if (readed < 0 ) {
			if(readbytes == 0) {
				close(filefd);
				unlink(filepath);
				return -1;
			}
			break;
		}

		write(filefd, buf, readed);
	}

	close(filefd);
	return readbytes;
}

/**
 * Store socket data into memory directly.
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 * @param	size		length of bytes to read from socket
 * @param	mem		memory buffer pointer
 * @param	timeoutms	wait timeout micro-seconds
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	timeoutms is not the total retrieving time.
 *		only affected if no data reached to socket until timeoutms reached.
 *		if some data are received, it will wait until timeoutms reached again.
 * @code
 * @endcode
 */
int qSocketSaveIntoMemory(int sockfd, int size, char *mem, int timeoutms) {
	char *mp;
	int sockstatus, readbytes, readed, readsize;

	for (readbytes = 0, mp = mem; readbytes < size; readbytes += readed, mp += readed) {
		// calculate reading size
		readsize = size - readbytes;

		// wait data
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) {
			if(readbytes == 0) return -1;
			break;
		}

		// read data
		readed = read(sockfd, mp, readsize);
		if (readed == 0) break; // eof
		else if (readed < 0) {
			if(readbytes == 0) return -1;
			break;
		}
	}

	return readbytes;
}

/**
 * Convert the socket descriptor to FILE descriptor.
 *
 * So you can use the socket easily by using fprintf(), fscanf()...
 *
 * @param	sockfd		socket descriptor returned by qSocketOpen()
 *
 * @return	file descriptor on success, or NULL if an error occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
FILE *qSocketConv2file(int sockfd) {
	FILE *fp;

#ifdef _WIN32
	fp = fsdopen(sockfd);
#else
	fp = fdopen(sockfd, "r+");
#endif

	return fp;
}

/**********************************************
** Internal Functions
**********************************************/

int _StrToAddr(struct sockaddr_in *addr, unsigned char family, char *hostname, int port) {

	/* here we assume that the hostname argument contains ip address */
	if (!inet_aton(hostname, &addr->sin_addr)) { /* fail then try another way */
		struct hostent *hp;
		if ((hp = gethostbyname (hostname)) == 0) return 0; /* fail return 0 */
		memcpy (&addr->sin_addr, hp->h_addr, sizeof(struct in_addr));
	}
	addr->sin_family = family;
	addr->sin_port = htons(port);

	return 1;
}

#endif /* _WIN32 */
#endif /* DISABLE_SOCKET */
