/************************************************************************
qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org

Copyright (C) 2001 The qDecoder Project.
Copyright (C) 1999,2000 Hongik Internet, Inc.
Copyright (C) 1998 Nobreak Technologies, Inc.
Copyright (C) 1996,1997 Seung-young Kim.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Copyright Disclaimer:
  Hongik Internet, Inc., hereby disclaims all copyright interest.
  President, Christopher Roh, 6 April 2000

  Nobreak Technologies, Inc., hereby disclaims all copyright interest.
  President, Yoon Cho, 6 April 2000

  Seung-young Kim, hereby disclaims all copyright interest.
  Author, Seung-young Kim, 6 April 2000
************************************************************************/

#ifndef _WIN32

#include "qDecoder.h"
#include "qInternal.h"
#include <sys/socket.h> /* for qSocket.c */
#include <netinet/in.h> /* for qSocket.c */
#include <netdb.h>      /* for qSocket.c */

/**********************************************
** Define Paragraph
**********************************************/


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
  if(_StrToAddr(&addr, AF_INET, hostname, port) != 1) return -1; /* invalid hostname */

  /* make sockfd */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -2; /* sockfd creation fail */

  /* connect */
  if(connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(sockfd);
    return -3; /* connection fail */
  }

  return sockfd;
}

/**********************************************
** Usage : qSocketclose(sockfd)
** Return: returns the value 0 if successful,
**         otherwise the value -1 is returned.
** Do    : delete socket descriptor.
**********************************************/
int qSocketClose(int sockfd) {
  return close(sockfd);
}

/**********************************************
** Usage : qSocketWaitReadable(sockfd, 10);
** Return: returns the value 1 if successful,
**         otherwise the given seconds exceeded(timeout)
**         the value of 0 is returned.
** Do    : block the program until the socket has readable data
**         or given seconds.
** Notice: You don't need to set the socket as non-block mode.
**********************************************/
int qSocketWaitReadable(int sockfd, int timeoutsec) {
  struct timeval tv;
  fd_set readfds;

  // time to wait
  tv.tv_sec = timeoutsec, tv.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);
  select(sockfd+1, &readfds, NULL, NULL, &tv);
  if (!FD_ISSET(sockfd, &readfds)) return 0; // timeout

  return 1;
}

/**********************************************
** Usage : qSocketRead(save_array_pointer, length, sockfd, 10);
** Return: returns the length of data readed.
**         otherwise(timeout) the value -1 is returned.
**         you can guess that timeout occured.
** Do    : read socket stream.
** Notice: You don't need to set the socket as non-block mode.
**********************************************/
int qSocketRead(char *binary, int size, int sockfd, int timeoutsec) {
  char *ptr;
  int readcnt;

  if(qSocketWaitReadable(sockfd, timeoutsec) == 0) return -1;

  for(ptr = binary, readcnt = 0; size > 0; size--, ptr++, readcnt++) {
    if(read(sockfd, ptr, 1) != 1) break;
  }

  return readcnt;
}

/**********************************************
** Usage : qSocketGets(save_array_pointer, array_max_length, sockfd, 10);
** Return: Success: returns the string pointer.
**         Half fail: timeout occured but has some data readed: returns the string pointer.
**         Fail: timeout occured and has no data readed: returns NULL.
** Do    : read line from the stream. it does not contain the character '\r', '\n'.
**********************************************/
char *qSocketGets(char *str, int size, int sockfd, int timeoutsec) {
  char *ptr;

  if(qSocketWaitReadable(sockfd, timeoutsec) == 0) return NULL;

  for(ptr = str; size > 1; size--, ptr++) {
    if(read(sockfd, ptr, 1) != 1) {
    	if(ptr == str) return NULL;
    	break;
    }
    if(*ptr == '\n') break;
    if(*ptr == '\r') ptr--;
  }

  *ptr = '\0';
  return str;
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
  if(write(sockfd, str, strlen(str)) < 0) return 0;
  if(write(sockfd, "\r\n", 2) < 0) return 0;

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
  int sendcnt;

  va_start(arglist, format);
  vsprintf(buf, format, arglist);
  va_end(arglist);

  if(write(sockfd, buf, strlen(buf)) < 0) return 0;

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
  if((fp = fopen(filepath, "r")) == NULL) return 0;

  // set offset
  if(offset > 0) {
    fseek(fp, offset, SEEK_SET);
  }

  for(sendbytes = 0; ; sendbytes += sended) {
    // read
    readed = fread(buf, 1, sizeof(buf), fp);
    if(readed == 0) break;

    // send
    sended = write(sockfd, buf, readed);

    // connection check
    if(readed != sended) {
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
int qSocketSaveIntoFile(int sockfd, int size, int timeoutsec, char *filepath, char *mode) {
  FILE *fp;
  char buf[1024*16]; // read buffer size

  int readbytes, readed, readsize;

  // stream readable?
  if(qSocketWaitReadable(sockfd, timeoutsec) == 0) return -1;

  // file open
  if((fp = fopen(filepath, mode)) == NULL) return 0;

  for(readbytes = 0; readbytes < size; readbytes += readed) {

    // calculate reading size
    if(size - readbytes < sizeof(buf)) readsize = size - readbytes;
    else readsize = sizeof(buf);

    // read data
    readed = read(sockfd, buf, readsize);

    if(readed == 0) break; // EOF

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
** Comment: DO NOT USE this function for some compatibility reason.
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
  fp=fsdopen(sockfd);
#else
  fp=fdopen(sockfd, "r+");
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
    if((hp = gethostbyname (hostname)) == 0) return 0; /* fail return 0 */
    memcpy (&addr->sin_addr, hp->h_addr, sizeof(struct in_addr));
  }
  addr->sin_family = family;
  addr->sin_port = htons(port);

  return 1;
}

#endif
