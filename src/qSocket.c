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
int qSocketRead(char *binary, int size, int sockfd, int timeoutms) {
	int readcnt;

	if (qSocketWaitReadable(sockfd, timeoutms) <= 0) return 0;
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
int qSocketGets(char *str, int size, int sockfd, int timeoutms) {
	char *ptr;
	int readcnt = 0;

	if (qSocketWaitReadable(sockfd, timeoutms) <= 0) return 0;

	for (ptr = str; size > 1; size--, ptr++) {
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
int qSocketWrite(char *binary, int size, int sockfd) {
	return write(sockfd, binary, size);
}

/**********************************************
** Usage : qSocketPuts(string_pointer, sockfd);
** Return: returns the value 1 if successful,
**         otherwise the value 0 is returned.
** Do    : send one line with terminating newline character to socket stream.
**********************************************/
int qSocketPuts(char *str, int sockfd) {
	if (write(sockfd, str, strlen(str)) < 0) return 0;
	if (write(sockfd, "\n", 1) < 0) return 0;

	return 1;
}

/**********************************************
** Usage : qSocketPrintf(sockfd, "Hello %s - %d day left", s, i);
** Return: returns the value 1 if successful,
**         otherwise(connection closed by foreign host?) the value 0 is returned.
** Do    : send formatted data to socket stream.
** Notice: the final length of formatted data must be less than 1024.
**         If you need to send more data, use qSocketPuts instead.
**********************************************/
int qSocketPrintf(int sockfd, char *format, ...) {
	char buf[1024];
	va_list arglist;

	va_start(arglist, format);
	vsprintf(buf, format, arglist);
	va_end(arglist);

	if (write(sockfd, buf, strlen(buf)) < 0) return 0;

	return 1;
}

/**********************************************
** Usage : qSocketSendFile("/home/mydata/hello.exe", 0, sockfd);
** Return: returns total sent bytes, otherwise the value -1 returned.
** Do    : send file data to socket stream.
**********************************************/
int qSocketSendFile(char *filepath, int offset, int sockfd) {
	FILE *fp;
	char buf[1024*16];

	int readed, sended, sendbytes;

	// file open
	if ((fp = fopen(filepath, "r")) == NULL) return 0;

	// set offset
	if (offset > 0) {
		fseek(fp, offset, SEEK_SET);
	}

	for (sendbytes = 0; ; sendbytes += sended) {
		// read
		readed = fread(buf, 1, sizeof(buf), fp);
		if (readed == 0) break;

		// send
		sended = write(sockfd, buf, readed);

		// connection check
		if (readed != sended) {
			fclose(fp);
			return -1; // connection closed
		}
	}

	fclose(fp);

	return sendbytes;
}

/**********************************************
** Usage : qSocketSaveIntoFile(sockfd, reading_length, 10, "/home/savedata/pic.jpg", "w");
** Return: returns total read bytes otherwise(timeout) the value -1 is returned.
** Do    : save stream data into file directly.
**********************************************/
// returns get bytes
int qSocketSaveIntoFile(int sockfd, int size, int timeoutms, char *filepath, char *mode) {
	FILE *fp;
	char buf[1024*16]; // read buffer size

	int readbytes, readed, readsize;

	// stream readable?
	if (qSocketWaitReadable(sockfd, timeoutms) <= 0) return -1;

	// file open
	if ((fp = fopen(filepath, mode)) == NULL) return 0;

	for (readbytes = 0; readbytes < size; readbytes += readed) {

		// calculate reading size
		if (size - readbytes < sizeof(buf)) readsize = size - readbytes;
		else readsize = sizeof(buf);

		// read data
		readed = read(sockfd, buf, readsize);

		if (readed == 0) break; // EOF

		fwrite(buf, readed, 1, fp);
	}

	fclose(fp);

	return readbytes;
}

/**********************************************
** Usage : qSocketSetNonblock(sockfd)
** Return: returns the value 1 if successful,
**         otherwise the value 0 is returned.
** Do    : set the socket to non-blocking mode.
** Comment: DO NOT USE this function untile you surely know what it means.
**          Please use qSocketWaitReadable() instead.
**********************************************/
// 0: error, 1: ok
int qSocketSetNonblock(int sockfd) {
	int opts;

	opts = fcntl(sockfd, F_GETFL);
	if (opts < 0) return -1;
	if (fcntl(sockfd, F_SETFL, opts | O_NONBLOCK) < 0) return 0;

	return 1;
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

#endif
