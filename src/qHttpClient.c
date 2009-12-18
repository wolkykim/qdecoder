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
 * @file qHttpClient.c HTTP client API
 */

#ifndef DISABLE_SOCKET

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "qDecoder.h"
#include "qInternal.h"

#ifndef _DOXYGEN_SKIP

static bool _open(Q_HTTPCLIENT *client, int timeoutms);
static void _setKeepalive(Q_HTTPCLIENT *client, bool keepalive);
static void _setUseragent(Q_HTTPCLIENT *client, const char *agentname);
static bool _put(Q_HTTPCLIENT *client, const char *putpath, Q_ENTRY *xheaders, int fd, off_t length, int timeoutms, int *retcode, bool (*callback)(void *userdata, off_t sentbytes), void *userdata);

static bool _close(Q_HTTPCLIENT *client);
static void _free(Q_HTTPCLIENT *client);

// internal usages
#define HTTP_NO_RESPONSE			(0)
#define HTTP_CODE_CONTINUE			(100)
#define HTTP_CODE_OK				(200)
#define HTTP_CODE_CREATED			(201)
#define	HTTP_CODE_NO_CONTENT			(204)
#define HTTP_CODE_MULTI_STATUS			(207)
#define HTTP_CODE_MOVED_TEMPORARILY		(302)
#define HTTP_CODE_NOT_MODIFIED			(304)
#define HTTP_CODE_BAD_REQUEST			(400)
#define HTTP_CODE_FORBIDDEN			(403)
#define HTTP_CODE_NOT_FOUND			(404)
#define HTTP_CODE_METHOD_NOT_ALLOWED		(405)
#define HTTP_CODE_REQUEST_TIME_OUT		(408)
#define HTTP_CODE_REQUEST_URI_TOO_LONG		(414)
#define HTTP_CODE_INTERNAL_SERVER_ERROR		(500)
#define HTTP_CODE_NOT_IMPLEMENTED		(501)
#define HTTP_CODE_SERVICE_UNAVAILABLE		(503)

#define	HTTP_PROTOCOL_10			"HTTP/1.0"
#define	HTTP_PROTOCOL_11			"HTTP/1.1"

static int _readResponse(int socket, int timeoutms);

#endif

/**
 *
 */
Q_HTTPCLIENT *qHttpClient(const char *hostname, int port) {
	// get remote address
	struct sockaddr_in addr;
	if(qSocketGetAddr(&addr, hostname, port) == false) {
		return NULL;
	}

	// allocate  object
	Q_HTTPCLIENT *client = (Q_HTTPCLIENT *)malloc(sizeof(Q_HTTPCLIENT));
	if(client == NULL) return NULL;
	memset((void*)client, 0, sizeof(Q_HTTPCLIENT));

	// initialize object
	client->socket = -1;

	memcpy((void*)&client->addr, (void*)&addr, sizeof(client->addr));
	client->hostname = strdup(hostname);
	client->port = port;

	client->keepalive = false;
	client->useragent = strdup("qDecoder/" _Q_VERSION);

	// member methods
	client->open		= _open;
	client->setKeepalive	= _setKeepalive;
	client->setUseragent	= _setUseragent;
	client->put		= _put;
	client->close		= _close;
	client->free		= _free;

	return client;
}

/**
 * Try to establish the connection.
 *
 * @param Q_HTTPCLIENT	HTTP object pointer
 * @param timeoutms	if timeoutms is greater than 0, connection timeout will be applied. (0 for default behavior)
 *
 * @return	true if successful, otherwise returns false
 *
 * @code
 *   // example for KEEP-ALIVE connection
 *   Q_HTTPCLIENT *client = qHttpClient("www.qdecoder.org", 80);
 *   if(client == NULL) return;
 *
 *   if(client->open(client) == false) return; // not necessary
 *   ...
 *   client->put(client, ...);
 *   ...
 *   client->close(client); // not necessary
 *   client->free(client);
 * @endcode
 *
 * @note
 * Don't need to open a connection explicitly, because it will make a connection automatically when it is necessary.
 * But you can still open a connection explicitily to check connection problem.
 */
static bool _open(Q_HTTPCLIENT *client, int timeoutms) {
	if(client->socket >= 0) {
		if(qSocketWaitWritable(client->socket, 0) > 0) return true;
		_close(client);
	}

	// create new socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	 	// sockfd creation fail
		return false;
	}

	// set to non-block socket
	int sockflag = 0;
	if(timeoutms >= 0) {
		sockflag = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, sockflag | O_NONBLOCK);
	}

	// try to connect
	int status = connect(sockfd, (struct sockaddr *)&client->addr, sizeof(client->addr));
	if(status < 0 && (errno != EINPROGRESS || qSocketWaitWritable(sockfd, timeoutms) <= 0) ) {
		// connection failed
		close(client->socket);
		return false;
	}

	// restore to block socket
	if(timeoutms >= 0) {
		fcntl(sockfd, F_SETFL, sockflag);
	}

	// store socket descriptor
	client->socket = sockfd;

	return true;
}

static void _setKeepalive(Q_HTTPCLIENT *client, bool keepalive) {
	client->keepalive = keepalive;
}

static void _setUseragent(Q_HTTPCLIENT *client, const char *useragent) {
	if(client->useragent != NULL) free(client->useragent);
	client->useragent = strdup(useragent);
}

#define MAX_SENDSIZE	(32 * 1024)
static bool _put(Q_HTTPCLIENT *client, const char *putpath, Q_ENTRY *xheaders, int fd, off_t length, int timeoutms, int *retcode,
		bool (*callback)(void *userdata, off_t sentbytes), void *userdata) {
	// reset retcode
	if(retcode != NULL) *retcode = 0;

	// get or open connection
	if(_open(client, timeoutms) == false) {
		return false;
	}

	// print out headers
	qSocketPrintf(client->socket, "PUT %s %s\r\n", putpath, (client->keepalive==true)?HTTP_PROTOCOL_11:HTTP_PROTOCOL_10);

	qSocketPrintf(client->socket, "Host: %s:%d\r\n", client->hostname, client->port);
	qSocketPrintf(client->socket, "Content-Length: %jd\r\n", length);
	qSocketPrintf(client->socket, "User-Agent: %s\r\n", client->useragent);
	qSocketPrintf(client->socket, "Connection: %s\r\n", (client->keepalive==true)?"Keep-Alive":"close");
	qSocketPrintf(client->socket, "Expect: 100-continue\r\n");
	qSocketPrintf(client->socket, "Date: %s\r\n", qTimeGetGmtStaticStr(0));

	// print out custom headers
	if(xheaders != NULL) {
		Q_NLOBJ_T obj;
		memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
		xheaders->lock(xheaders);
		while(xheaders->getNext(xheaders, &obj, NULL, false) == true) {
			qSocketPrintf(client->socket, "%s: %s\r\n", obj.name, (char*)obj.data);
		}
		xheaders->unlock(xheaders);
	}

	qSocketPrintf(client->socket, "\r\n");

	// wait 100-continue
	if(qSocketWaitReadable(client->socket, timeoutms) <= 0) {
		_close(client);
		return false;
	}

	// read response
	int rescode = _readResponse(client->socket, timeoutms);
	if(rescode != HTTP_CODE_CONTINUE) {
		if(retcode != NULL) *retcode = rescode;
		_close(client);
		return false;
	}

	// send data
	off_t sent = 0;
	if(callback != NULL) {
		if(callback(userdata, sent) == false) {
			_close(client);
			return false;
		}
	}
	if(length > 0) {
		while(sent < length) {
			size_t sendsize;	// this time sending size
			if(length - sent <= MAX_SENDSIZE) sendsize = length - sent;
			else sendsize = MAX_SENDSIZE;

			ssize_t ret = qFileSend(client->socket, fd, sendsize);
			if(ret <= 0) break; // Connection closed by peer
			sent += ret;

			if(callback != NULL) {
				if(callback(userdata, sent) == false) {
					_close(client);
					return false;
				}
			}
		}

		if(sent != length) {
			_close(client);
			return false;
		}

		if(callback != NULL) {
			if(callback(userdata, sent) == false) {
				_close(client);
				return false;
			}
		}
	}

	// read response
	rescode = _readResponse(client->socket, timeoutms);
	if(rescode == HTTP_NO_RESPONSE) {
			_close(client);
			return false;
	}
	if(retcode != NULL) *retcode = rescode;

	if(rescode != HTTP_CODE_CREATED) {
		_close(client);
		return false;
	}

	// close connection
	if(client->keepalive == false) {
		_close(client);
	}

	return true;
}

/**
 * Close the connection.
 *
 * @param Q_HTTPCLIENT	HTTP object pointer
 *
 * @note	A connection will be automatically closed.
 */
static bool _close(Q_HTTPCLIENT *client) {
	if(client->socket < 0) return true;

	// shutdown connection
	qSocketClose(client->socket);
	client->socket = -1;

	return true;
}

/**
 * De-allocate object.
 *
 * @param Q_HTTPCLIENT	HTTP object pointer
 *
 * @note	A connection will be automatically closed.
 */
static void _free(Q_HTTPCLIENT *client) {
	if(client->socket >= 0) {
		client->close(client);
	}

	if(client->hostname != NULL) free(client->hostname);
	if(client->useragent != NULL) free(client->useragent);

	free(client);
}

static int _readResponse(int socket, int timeoutms) {
	// read response
	char buf[1024];
	if(qSocketGets(buf, sizeof(buf),socket, timeoutms) <= 0) return HTTP_NO_RESPONSE;

	// parse response code
	if(strncmp(buf, "HTTP/", CONST_STRLEN("HTTP/"))) return HTTP_NO_RESPONSE;
	char *tmp = strstr(buf, " ");
	if(tmp == NULL) return HTTP_NO_RESPONSE;
	int rescode = atoi(tmp+1);
	if(rescode == 0) return HTTP_NO_RESPONSE;

	// read header until CRLF
	while(qSocketGets(buf, sizeof(buf),socket, timeoutms) > 0) {
		if(strlen(buf) == 0) break;
	}

	return rescode;
}

#endif /* DISABLE_SOCKET */
