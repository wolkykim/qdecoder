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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"

#define SOCKET_TIMEOUT		(5 * 1000)

int dumpHttp(const char *hostname, int port) {
	// open socket
	int sockfd = qSocketOpen(hostname, port);
	if (sockfd < 0) return sockfd;

	// send data
	qSocketPrintf(sockfd, "GET / HTTP/1.1\n");
	qSocketPrintf(sockfd, "Host: %s\n", hostname);
	qSocketPrintf(sockfd, "Accept: */*\n");
	qSocketPrintf(sockfd, "User-Agent: qDecoder Bot\n");
	qSocketPrintf(sockfd, "Connection: close\n");
	qSocketPrintf(sockfd, "\n");

	// read data
	char buf[1024];
	int lineno;
	for (lineno = 1; qSocketGets(buf, sockfd, sizeof(buf), SOCKET_TIMEOUT) >= 0; lineno++) {
		printf("%03d: %s\n", lineno, buf);
	}

	// close socket
	qSocketClose(sockfd);

	return 0;
}

int main(void) {
	Q_ENTRY *req = qCgiRequestParse(NULL);
	qCgiResponseSetContentType(req, "text/plain");

	const char *hostname = qEntryGetStr(req, "hostname");
	if (hostname == NULL || strlen(hostname) == 0) qCgiResponseError(req, "Invalid usages.");

	int retflag = dumpHttp(hostname, 80);
	if (retflag < 0) {
		if (retflag == -1) qCgiResponseError(req, "Invalid hostname.");
		else if (retflag == -2) qCgiResponseError(req, "Can't create socket.");
		else if (retflag == -3) qCgiResponseError(req, "Connection failed.");
		else qCgiResponseError(req, "Unknown error.");
	}

	return 0;
}
