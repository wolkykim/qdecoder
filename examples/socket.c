/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

#include "qDecoder.h"

#define SOCKET_TIMEOUT		(10)

int dumpHttp(char *hostname) {
  int sockfd;
  char buf[1024];
  int lineno;

  // open socket
  sockfd = qSocketOpen(hostname, 80);
  if(sockfd < 0) return sockfd;

  // send data
  qSocketPrintf(sockfd, "GET / HTTP/1.1\n");
  qSocketPrintf(sockfd, "Host: %s\n", hostname);
  qSocketPrintf(sockfd, "Accept: */*\n");
  qSocketPrintf(sockfd, "User-Agent: qDecoder Bot\n");
  qSocketPrintf(sockfd, "Connection: close\n");
  qSocketPrintf(sockfd, "\n");

  // read data
  for(lineno = 1; ; lineno++) {
    // read line from socket
    if(qSocketGets(buf, sizeof(buf), sockfd, SOCKET_TIMEOUT) <= 0) qError("Timeout or socket closed.");

    // if the http header block ended, stop reading.
    if(strlen(buf) == 0) break;

    // print header
    printf("%d: %s\n", lineno, buf);
  }

  // close socket
  qSocketClose(sockfd);

  return 0;
}

int main(void) {
  char *hostname;
  int retflag;

  qContentType("text/plain");

  hostname = qValueDefault("", "hostname");
  if(strlen(hostname) == 0) qError("Invalid usages.");

  retflag = dumpHttp(hostname);
  if(retflag < 0) {
    if(retflag == -1) qError("Invalid hostname.");
    else if(retflag == -2) qError("Can't create socket.");
    else if(retflag == -3) qError("Connection failed.");
    else qError("Unknown error.");
  }

  return 0;
}

