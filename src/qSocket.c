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
#include <sys/sendfile.h>

#include "qDecoder.h"

/**********************************************
** Internal Functions Definition
**********************************************/

int _StrToAddr(struct sockaddr_in *addr, unsigned char family, char *hostname, int port);


/**********************************************
** Usage : qSocketOpen("www.qdecoder.org", 80);
** Return: Success: socket descriptor(positive)
           Fail: returns -1 in case of invalid hostname
           Fail: returns -2 in case of socket creation fail
** Do    : Create a TCP socket for communication.
**********************************************/
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
 * Usage : qSocketClose(sockfd)
 * Return: returns the value 1 if successful,
 *         otherwise returns the value 0.
 * Do    : close socket.
 */
int qSocketClose(int sockfd) {
	if(close(sockfd) == 0) return 1;
	return 0;
}

/**********************************************
** Usage : qSocketWaitReadable(sockfd, 1000);
** Return: returns the value 1 when it is readable,
**         otherwise the given seconds exceeded(timeout) the value of 0 is returned.
**         -1 if an error occurred.
** Do    : Block the program until the socket has readable data.
** Notice: You don't need to set the socket as non-block mode.
**********************************************/
int qSocketWaitReadable(int sockfd, int timeoutms) {
	struct timeval tv;
	fd_set readfds;

	// time to wait
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	if (timeoutms > 0) {
		tv.tv_sec = (timeoutms / 1000), tv.tv_usec = (timeoutms % 1000);
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

/**********************************************
** Usage : qSocketRead(save_array_pointer, length, sockfd, 1000);
** Return: returns the length of data readed.
**         otherwise(timeout) the value 0 is returned.
**         returns -1 if an error occured.
** Do    : read socket stream.
** Notice: You don't need to set the socket as non-block mode.
**********************************************/
int qSocketRead(int sockfd, char *binary, int size, int timeoutms) {
	int sockstatus, readcnt;

	sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;
	readcnt = read(sockfd, binary, size);
	if (readcnt <= 0) return -1;

	return readcnt;
}

/**********************************************
** Usage : qSocketGets(save_array_pointer, array_max_length, sockfd, 1000);
** Return: returns the length of data readed.
**         otherwise(timeout) the value 0 is returned.
**         returns -1 if an error occured.
** Do    : read line from the stream. it does not contain the character '\r', '\n'.
**********************************************/
int qSocketGets(int sockfd, char *str, int size, int timeoutms) {
	char *ptr;
	int sockstatus, readcnt = 0;

	for (ptr = str; size > 1; size--, ptr++) {
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

/**********************************************
** Usage : qSocketWrite(data_array_pointer, data_length, sockfd);
** Return: returns the number of bytes sent if successful,
*          otherwise the value -1 is returned.
** Do    : send some data(text/binary) to socket stream .
**********************************************/
int qSocketWrite(int sockfd, char *binary, int size) {
	return write(sockfd, binary, size);
}

/**********************************************
** Usage : qSocketPuts(string_pointer, sockfd);
** Return: returns the number of bytes sent if successful,
*          otherwise the value -1 is returned.
** Do    : send one line with terminating newline character to socket stream.
**********************************************/
int qSocketPuts(int sockfd, char *str) {
	char *buf;

	buf = (char *)malloc(strlen(str) + 2 + 1);
	if(buf == NULL) return -1;
	sprintf(buf, "%s\r\n", str);

	return write(sockfd, buf, strlen(buf));
}

/**********************************************
** Usage : qSocketPrintf(sockfd, "Hello %s - %d day left", s, i);
** Return: returns total sent bytes, otherwise the value -1 returned.
** Do    : send formatted data to socket stream.
** Notice: the final length of formatted data must be less than 1024.
**         If you need to send more data, use qSocketPuts instead.
**********************************************/
int qSocketPrintf(int sockfd, char *format, ...) {
	char buf[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf)-1, format, arglist);
	buf[sizeof(buf)-1] = '\0';
	va_end(arglist);

	return write(sockfd, buf, strlen(buf));
}

/**********************************************
** Usage : qSocketSendFile(sockfd, "/home/mydata/hello.exe", 0);
** Return: returns total sent bytes, otherwise the value -1 returned.
** Do    : send file data to socket stream.
**********************************************/
#define MAX_SENDFILE_CHUNK_SIZE		(1024 * 1024)
ssize_t qSocketSendFile(int sockfd, char *filepath, off_t offset) {
	struct stat filestat;
	int filefd;

	if((filefd = open(filepath, O_RDONLY , 0))  < 0) return -1;
	if (fstat(filefd, &filestat) < 0) return -1;

	ssize_t rangesize = filestat.st_size - offset;
	ssize_t sent = 0;		// total size sent
	while(sent < rangesize) {
		size_t sendsize;	// this time sending size
		if(rangesize - sent > MAX_SENDFILE_CHUNK_SIZE) sendsize = MAX_SENDFILE_CHUNK_SIZE;
		else sendsize = rangesize - sent;

		ssize_t ret = sendfile(sockfd, filefd, &offset, sendsize);
		if(ret <= 0) break; // Connection closed by peer

		sent += ret;
	}
	close(filefd);

	return sent;
}

/**********************************************
** Usage : qSocketSaveIntoFile(sockfd, reading_length, 10, "/home/savedata/pic.jpg", "w");
** Return: returns total read bytes otherwise(timeout) the value -1 is returned.
** Do    : save stream data into file directly.
**********************************************/
// returns get bytes
int qSocketSaveIntoFile(int sockfd, int size, char *filepath, char *mode, int timeoutms) {
	FILE *fp;
	char buf[1024*32]; // read buffer size

	int sockstatus, readbytes, readed, readsize;

	// stream readable?
	sockstatus = qSocketWaitReadable(sockfd, timeoutms);
	if (sockstatus <= 0) return sockstatus;

	// file open
	if ((fp = fopen(filepath, mode)) == NULL) return 0;

	for (readbytes = 0; readbytes < size; readbytes += readed) {

		// calculate reading size
		if (size - readbytes < sizeof(buf)) readsize = size - readbytes;
		else readsize = sizeof(buf);

		// read data
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) {
			fclose(fp);
			unlink(filepath);
			return sockstatus;
		}

		readed = read(sockfd, buf, readsize);
		if (readed == 0) break; // eof
		else if (readed < 0 ) {
			fclose(fp);
			unlink(filepath);
			return -1; // error
		}

		fwrite(buf, readed, 1, fp);
	}

	fclose(fp);

	return readbytes;
}

int qSocketSaveIntoMemory(int sockfd, int size, char *mem, int timeoutms) {
	char *mp;
	int sockstatus, readbytes, readed, readsize;

	for (readbytes = 0, mp = mem; readbytes < size; readbytes += readed, mp += readed) {
		// calculate reading size
		readsize = size - readbytes;

		// wait data
		sockstatus = qSocketWaitReadable(sockfd, timeoutms);
		if (sockstatus <= 0) return sockstatus;

		// read data
		readed = read(sockfd, mp, readsize);
		if (readed == 0) break; // eof
		else if (readed < 0) return -1; // error
	}

	return readbytes;
}

/**********************************************
** Usage : qSocketConv2file(sockfd)
** Return: returns the pointer of converted file descriptor.
** Do    : convert the socket descriptor(int type) to
**         file descriptor(FILE type). So you can use the
**         stream easily by using fprintf(), fscanf()...
**********************************************/
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
