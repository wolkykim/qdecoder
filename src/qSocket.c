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
#include "qInternal.h"

#if defined(ENABLE_SENDFILE) && defined(__linux__)
#include <sys/sendfile.h>
#define MAX_SENDFILE_CHUNK_SIZE		(1 * 1024 * 1024 * 1024)
#else
#define MAX_SENDFILE_CHUNK_SIZE		(64 * 1024)
#endif

static bool _getAddr(struct sockaddr_in *addr, const char *hostname, int port);

/**
 * Create a TCP socket for the remote host and port.
 *
 * @param	hostname	remote hostname
 * @param	port		remote port
 *
 * @return	the new socket descriptor, or -1 in case of invalid hostname, -2 in case of socket creation failure, -3 in case of connection failure.
 *
 * @since	8.1R
 *
 * @note
 * @code
 * @endcode
 */

int qSocketOpen(const char *hostname, int port) {
	/* host conversion */
	struct sockaddr_in addr;
	if (_getAddr(&addr, hostname, port) == false) return -1; /* invalid hostname */

	/* make sockfd */
	int sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2; /* sockfd creation fail */

	/* connect */
	if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		qSocketClose(sockfd);
		return -3; /* connection fail */
	}

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
 * @param	sockfd		socket descriptor
 * @param	timeoutms	wait timeout micro-seconds. if set to negative value, wait indefinitely.
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
 * Wait until the socket is writable.
 *
 * @param	sockfd		socket descriptor
 * @param	timeoutms	wait timeout micro-seconds. if set to negative value, wait indefinitely.
 *
 * @return	1 on writable, or 0 on timeout, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @note	does not need to set the socket as non-block mode.
 * @code
 * @endcode
 */
int qSocketWaitWritable(int sockfd, int timeoutms) {
	struct timeval tv;
	fd_set writefds;

	// time to wait
	FD_ZERO(&writefds);
	FD_SET(sockfd, &writefds);
	if (timeoutms > 0) {
		tv.tv_sec = (timeoutms / 1000), tv.tv_usec = ((timeoutms % 1000) * 1000);
		if (select(FD_SETSIZE, NULL, &writefds, NULL, &tv) < 0) return -1;
	} else if (timeoutms == 0) { // just poll
		tv.tv_sec = 0, tv.tv_usec = 0;
		if (select(FD_SETSIZE, NULL, &writefds, NULL, &tv) < 0) return -1;
	} else { //  blocks indefinitely
		if (select(FD_SETSIZE, NULL, &writefds, NULL, NULL) < 0) return -1;
	}

	if (!FD_ISSET(sockfd, &writefds)) return 0; // timeout

	return 1;
}

/**
 * Read data from socket.
 *
 * @param	binary		data buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		read size
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
int qSocketRead(char *binary, int sockfd, int nbytes, int timeoutms) {
	int sockstatus, readcnt;

	sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;
	readcnt = read(sockfd, binary, nbytes);
	if (readcnt <= 0) return -1;

	return readcnt;
}

/**
 * Read line from the stream.
 * New-line characters '\r', '\n' will not be stored into buffer.
 *
 * @param	str		data buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		read size
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
int qSocketGets(char *str, int sockfd, int nbytes, int timeoutms) {
	char *ptr;
	int sockstatus, readcnt = 0;

	for (ptr = str; readcnt < (nbytes - 1); ptr++) {
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
 * @param	sockfd		socket descriptor
 * @param	binary		data pointer
 * @param	nbytes		sending size
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
int qSocketWrite(int sockfd, const char *binary, int nbytes) {
	return write(sockfd, binary, nbytes);
}

/**
 * Send string with newline characters(\r\n) to socket.
 *
 * @param	sockfd		socket descriptor
 * @param	str		string pointer
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
int qSocketPuts(int sockfd, const char *str) {
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
 * @param	sockfd		socket descriptor
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
int qSocketPrintf(int sockfd, const char *format, ...) {
	char buf[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	return write(sockfd, buf, strlen(buf));
}

/**
 * Send file to socket.
 *
 * @param	sockfd		socket descriptor
 * @param	filepath	variable argument lists
 * @param	offset		file start to send
 * @param	nbytes		total bytes to send. 0 means send data until EOF.
 *
 * @return	the number of bytes sent on success, or -1 if an error(ex:socket closed) occurred.
 *
 * @since	8.1R
 *
 * @code
 * @endcode
 */
ssize_t qSocketSendfile(int sockfd, const char *filepath, off_t offset, ssize_t nbytes) {
	struct stat filestat;
	int filefd;

	if((filefd = open(filepath, O_RDONLY , 0))  < 0) return -1;
	if (fstat(filefd, &filestat) < 0) return -1;

	ssize_t sent = 0;				// total size sent
	ssize_t rangesize = filestat.st_size - offset;	// maximum available size can be sent
	if(nbytes > 0 && nbytes < rangesize) rangesize = nbytes; // set rangesize to requested size

#if !(defined(ENABLE_SENDFILE) && defined(__linux__))
	if (offset > 0) lseek(filefd, offset, SEEK_SET);
#endif

	while(sent < rangesize) {
		size_t sendsize;	// this time sending size
		if(rangesize - sent <= MAX_SENDFILE_CHUNK_SIZE) sendsize = rangesize - sent;
		else sendsize = MAX_SENDFILE_CHUNK_SIZE;

		ssize_t ret = 0;
#if defined(ENABLE_SENDFILE) && defined(__linux__)
		ret = sendfile(sockfd, filefd, &offset, sendsize);
#else
		ret = qFileSend(sockfd, filefd, sendsize);
#endif
		if(ret <= 0) break; // Connection closed by peer
		sent += ret;
	}

	close(filefd);
	return sent;
}

/**
 * Store socket data into file directly.
 *
 * @param	filefd		save file descriptor
 * @param	sockfd		socket descriptor
 * @param	nbytes		length of bytes to read from socket
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
 *   qSocketSaveIntoFile(filefd, sockfd, 100, 5000);
 * @endcode
 */
int qSocketSaveIntoFile(int filefd, int sockfd, int nbytes, int timeoutms) {
#define _Q_SAVEINTOFILE_CHUNK_SIZE	(4*1024)
	// stream readable?
	int sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;

	int readbytes, readed;
	for (readbytes = 0; readbytes < nbytes; readbytes += readed) {
		// calculate reading size
		int readsize;
		if (nbytes - readbytes < _Q_SAVEINTOFILE_CHUNK_SIZE) readsize = nbytes - readbytes;
		else readsize =_Q_SAVEINTOFILE_CHUNK_SIZE;

		// read data
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) break;

		readed = qFileSend(filefd, sockfd, readsize);
		if (readed <= 0) break; // eof
	}
	return readbytes;
}

/**
 * Store socket data into memory directly.
 *
 * @param	mem		memory buffer pointer
 * @param	sockfd		socket descriptor
 * @param	nbytes		length of bytes to read from socket
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
int qSocketSaveIntoMemory(char *mem, int sockfd, int nbytes, int timeoutms) {
	char *mp;
	int sockstatus, readbytes, readed, readsize;

	for (readbytes = 0, mp = mem; readbytes < nbytes; readbytes += readed, mp += readed) {
		// calculate reading size
		readsize = nbytes - readbytes;

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
	memset((void*)&addr, 0, sizeof(struct sockaddr_in));
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
