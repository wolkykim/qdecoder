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
#include <string.h>
#include "qDecoder.h"

#define SOCKET_TIMEOUT		(3 * 1000)

int dumpHttp(const char *hostname, int port) {
	// open socket
	int sockfd = qSocketOpen(hostname, port, SOCKET_TIMEOUT);
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
	for (lineno = 1; qSocketGets(buf, sizeof(buf), sockfd, SOCKET_TIMEOUT) >= 0; lineno++) {
		printf("%03d: %s\n", lineno, buf);
	}

	// close socket
	qSocketClose(sockfd);

	return 0;
}

int main(void) {
	Q_ENTRY *req = qCgiRequestParse(NULL);
	qCgiResponseSetContentType(req, "text/plain");

	const char *hostname = req->getStr(req, "hostname", false);
	if (hostname == NULL || strlen(hostname) == 0) qCgiResponseError(req, "Invalid usages.");

	int retflag = dumpHttp(hostname, 80);
	if (retflag < 0) {
		if (retflag == -1) qCgiResponseError(req, "Invalid hostname.");
		else if (retflag == -2) qCgiResponseError(req, "Can't create socket.");
		else if (retflag == -3) qCgiResponseError(req, "Connection failed.");
		else qCgiResponseError(req, "Unknown error.");
	}
	req->free(req);

	return 0;
}
