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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "qDecoder.h"
#include "qInternal.h"

#ifndef _DOXYGEN_SKIP

static bool _open(Q_HTTPCLIENT *client);
static void _setTimeout(Q_HTTPCLIENT *client, int timeoutms);
static void _setKeepalive(Q_HTTPCLIENT *client, bool keepalive);
static void _setUseragent(Q_HTTPCLIENT *client, const char *agentname);
static bool _sendRequest(Q_HTTPCLIENT *client, const char *method, const char *uri, Q_ENTRY *reqheaders);
static int _readResponse(Q_HTTPCLIENT *client, Q_ENTRY *resheaders);
static bool _get(Q_HTTPCLIENT *client, const char *uri, int fd, off_t *savesize, int *rescode,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders,
		bool (*callback)(void *userdata, off_t recvbytes), void *userdata);
static bool _put(Q_HTTPCLIENT *client, const char *uri, int fd, off_t length, int *rescode,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders,
		bool (*callback)(void *userdata, off_t sentbytes), void *userdata);
static void *_cmd(Q_HTTPCLIENT *client, const char *method, const char *uri,
		int *rescode, size_t *contentslength,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders);
static bool _close(Q_HTTPCLIENT *client);
static void _free(Q_HTTPCLIENT *client);

// internal usages
static bool _setSocketOption(int socket);

#endif

//
// HTTP RESPONSE CODE
//
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

#define	HTTP_PROTOCOL_11			"HTTP/1.1"

//
// TCP SOCKET DEFINITION
//
#define	SET_TCP_LINGER_TIMEOUT			(15)		/*< linger seconds, 0 for disable */
#define SET_TCP_NODELAY				(1)		/*< 0 for disable */
#define	MAX_SHUTDOWN_WAIT			(5 * 1000)	/*< maximum shutdown wait, unit is ms */
#define MAX_ATOMIC_DATA_SIZE			(32 * 1024)	/*< maximum sending bytes, used in PUT method */

/**
 * Initialize & create new HTTP client.
 *
 * @param hostname	remote IP or FQDN domain name
 * @param port		remote port number
 *
 * @return		HTTP client object if succcessful, otherwise returns NULL.
 *
 * @code
 *   // create new HTTP client
 *   Q_HTTPCLIENT *httpClient = qHttpClient("www.qdecoder.org", 80);
 *   if(httpClient == NULL) return;
 *
 *   // set options
 *   httpClient->setKeepalive(httpClient, true);
 *
 *   // make a connection
 *   if(httpClient->open(httpClient) == false) return;
 *
 *   // upload files
 *   httpClient->put(httpClient, ...);
 *   httpClient->put(httpClient, ...); // this will be done within same connection using KEEP-ALIVE
 *
 *   // close connection - not necessary, just for example
 *   httpClient->close(httpClient);
 *
 *   // de-allocate HTTP client object
 *   httpClient->free(httpClient);
 * @endcode
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

	// member methods
	client->setTimeout	= _setTimeout;
	client->setKeepalive	= _setKeepalive;
	client->setUseragent	= _setUseragent;

	client->open		= _open;
	client->sendRequest	= _sendRequest;
	client->readResponse	= _readResponse;
	client->get		= _get;
	client->put		= _put;
	client->cmd		= _cmd;
	client->close		= _close;
	client->free		= _free;

	// init client
	_setTimeout(client, 0);
	_setKeepalive(client, false);
	_setUseragent(client, _Q_PRGNAME "/" _Q_VERSION);

	return client;
}

/**
 * Q_HTTPCLIENT->setTimeout(): Set connection wait timeout.
 *
 * @param client	Q_HTTPCLIENT object pointer
 * @param timeoutms	timeout mili-seconds. 0 for system defaults
 *
 * @code
 *   httpClient->setTimeout(httpClient, 0);    // default
 *   httpClient->setTimeout(httpClient, 5000); // 5 seconds
 * @endcode
 */
static void _setTimeout(Q_HTTPCLIENT *client, int timeoutms) {
	if(timeoutms <= 0) timeoutms = -1;
	client->timeoutms = timeoutms;
}

/**
 * Q_HTTPCLIENT->setKeepalive(): Set KEEP-ALIVE feature on/off.
 *
 * @param client	Q_HTTPCLIENT object pointer
 * @param keepalive	true to set keep-alive on, false to set keep-alive off
 *
 * @code
 *   httpClient->setKeepalive(httpClient, true);  // keep-alive on
 *   httpClient->setKeepalive(httpClient, false); // keep-alive off
 * @endcode
 */
static void _setKeepalive(Q_HTTPCLIENT *client, bool keepalive) {
	client->keepalive = keepalive;
}

/**
 * Q_HTTPCLIENT->setUseragent(): Set user-agent string.
 *
 * @param client	Q_HTTPCLIENT object pointer
 * @param useragent	user-agent string
 *
 * @code
 *   httpClient->setUseragent(httpClient, "qDecoderAgent/1.0");
 * @endcode
 */
static void _setUseragent(Q_HTTPCLIENT *client, const char *useragent) {
	if(client->useragent != NULL) free(client->useragent);
	client->useragent = strdup(useragent);
}

/**
 * Q_HTTPCLIENT->open(): Open(establish) connection to the remote host.
 *
 * @param client	Q_HTTPCLIENT object pointer
 *
 * @return	true if successful, otherwise returns false
 *
 * @note
 * Don't need to open a connection unless you definitely need to do this, because qHttpClient open a connection automatically when it's needed.
 * This function also can be used to veryfy a connection failure with remote host.
 *
 * @code
 *   if(httpClient->open(httpClient) == false) return;
 * @endcode
 */
static bool _open(Q_HTTPCLIENT *client) {
	if(client->socket >= 0) {
		if(qIoWaitWritable(client->socket, 0) > 0) return true;
		_close(client);
	}

	// create new socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	 	DEBUG("sockfd creation failed.");
		return false;
	}

	// set to non-block socket if timeout is set
	int sockflag = 0;
	if(client->timeoutms > 0) {
		sockflag = fcntl(sockfd, F_GETFL, 0);
		fcntl(sockfd, F_SETFL, sockflag | O_NONBLOCK);
	}

	// try to connect
	int status = connect(sockfd, (struct sockaddr *)&client->addr, sizeof(client->addr));
	if(status < 0 && (errno != EINPROGRESS || qIoWaitWritable(sockfd, client->timeoutms) <= 0) ) {
	 	DEBUG("connection failed.");
		close(client->socket);
		return false;
	}

	// restore to block socket
	if(client->timeoutms > 0) {
		fcntl(sockfd, F_SETFL, sockflag);
	}

	// set socket option
	_setSocketOption(sockfd);

	// store socket descriptor
	client->socket = sockfd;

	return true;
}

/**
 * Q_HTTPCLIENT->sendRequest(): Send HTTP request to the remote host.
 *
 * @param client	Q_HTTPCLIENT object pointer
 * @param method	HTTP method name
 * @param uri		URI string for the method. ("/path" or "http://.../path")
 * @param reqheaders	Q_ENTRY pointer which contains additional user request headers. (can be NULL)
 *
 * @return	true if successful, otherwise returns false
 *
 * @note
 * 3 default headers(Host, User-Agent, Connection) will be sent if reqheaders does not have those headers in it.
 *
 * @code
 *   Q_ENTRY *reqheaders = qEntry();
 *   reqheaders->putStr(reqheaders,  "Date", qTimeGetGmtStaticStr(0), true);
 *
 *   httpClient->sendRequest(client, "DELETE", "/img/qdecoder.png", reqheaders);
 * @endcode
 */
static bool _sendRequest(Q_HTTPCLIENT *client, const char *method, const char *uri, Q_ENTRY *reqheaders) {
	if(_open(client) == false) {
		return false;
	}

	// generate request headers if necessary
	bool freeReqHeaders = false;
	if(reqheaders == NULL) {
		reqheaders = qEntry();
		if(reqheaders == NULL) return false;
		freeReqHeaders = true;
	}

	// append default headers
	if(reqheaders->getCase(reqheaders, "Host", NULL, false) == NULL) {
		reqheaders->putStrf(reqheaders, true, "Host", "%s:%d", client->hostname, client->port);
	}
	if(reqheaders->getCase(reqheaders, "User-Agent", NULL, false) == NULL) {
		reqheaders->putStr(reqheaders, "User-Agent", client->useragent, true);
	}
	if(reqheaders->getCase(reqheaders, "Connection", NULL, false) == NULL) {
		reqheaders->putStr(reqheaders, "Connection", (client->keepalive==true)?"Keep-Alive":"close", true);
	}

	// create stream buffer
	Q_OBSTACK *outBuf = qObstack();
	if(outBuf == NULL) return false;

	// buffer out command
	outBuf->growStrf(outBuf, "%s %s %s\r\n",
		method,
		uri,
		HTTP_PROTOCOL_11
	);

	// buffer out headers
	Q_NLOBJ_T obj;
	memset((void*)&obj, 0, sizeof(obj)); // must be cleared before call
	reqheaders->lock(reqheaders);
	while(reqheaders->getNext(reqheaders, &obj, NULL, false) == true) {
		outBuf->growStrf(outBuf, "%s: %s\r\n", obj.name, (char*)obj.data);
	}
	reqheaders->unlock(reqheaders);

	outBuf->growStrf(outBuf, "\r\n");

	// stream out
	size_t towrite = outBuf->getSize(outBuf);
	ssize_t written = outBuf->writeFinal(outBuf, client->socket);

	// de-allocate
	outBuf->free(outBuf);
	if(freeReqHeaders == true) reqheaders->free(reqheaders);

	if(written > 0 && written == towrite) return true;
	return false;
}

/**
 * Q_HTTPCLIENT->readResponse(): Read and parse HTTP response from the remote host.
 *
 * @param client	Q_HTTPCLIENT object pointer
 * @param resheaders	Q_ENTRY pointer for storing response headers. (can be NULL)
 *
 * @return	numeric HTTP response code if successful, otherwise returns 0.
 *
 * @code
 *   // send request
 *   httpClient->sendRequest(client, "DELETE", "/img/qdecoder.png", NULL);
 *
 *   // read response
 *   Q_ENTRY *resheaders = qEntry();
 *   int rescode = httpClient->readResponse(client, resheaders);
 * @endcode
 */
static int _readResponse(Q_HTTPCLIENT *client, Q_ENTRY *resheaders) {
	// read response
	char buf[1024];
	if(qIoGets(buf, sizeof(buf), client->socket, client->timeoutms) <= 0) return HTTP_NO_RESPONSE;

	// parse response code
	if(strncmp(buf, "HTTP/", CONST_STRLEN("HTTP/"))) return HTTP_NO_RESPONSE;
	char *tmp = strstr(buf, " ");
	if(tmp == NULL) return HTTP_NO_RESPONSE;
	int rescode = atoi(tmp+1);
	if(rescode == 0) return HTTP_NO_RESPONSE;

	// read headers
	while(qIoGets(buf, sizeof(buf), client->socket, client->timeoutms) > 0) {
		if(buf[0] == '\0') break;
		if(resheaders != NULL) {
			// parse header
			char *value = strstr(buf, ":");
			if(value != NULL) {
				*value = '\0';
				value += 1;
				qStrTrim(value);
			} else {
				// missing colon
				value = "";
			}
			resheaders->putStr(resheaders, buf, value, true);
		}
	}

	return rescode;
}

/**
 * Q_HTTPCLIENT->get(): Download file from remote host using GET method.
 *
 * @param client	Q_HTTPCLIENT object pointer.
 * @param uri		remote URL for downloading file. ("/path" or "http://.../path")
 * @param fd		opened file descriptor for writing.
 * @param savesize	if not NULL, the length of stored bytes will be stored. (can be NULL)
 * @param rescode	if not NULL, remote response code will be stored. (can be NULL)
 * @param reqheaders	Q_ENTRY pointer which contains additional user request headers. (can be NULL)
 * @param resheaders	Q_ENTRY pointer for storing response headers. (can be NULL)
 * @param callback	set user call-back function. (can be NULL)
 * @param userdata	set user data for call-back. (can be NULL)
 *
 * @return	true if successful, otherwise returns false
 *
 * @note
 * The call-back function will be called peridically whenever it send data as much as MAX_ATOMIC_DATA_SIZE.
 * To stop uploading, return false in the call-back function, then PUT process will be stopped immediately.
 * If a connection was not opened, it will open a connection automatically.
 *
 * @code
 *   struct userdata {
 *     ...
 *   };
 *
 *   static bool callback(void *userdata, off_t sentbytes) {
 *     struct userdata *pMydata = (struct userdata*)userdata;
 *     ...(codes)...
 *     if(need_to_cancel) return false; // stop file uploading immediately
 *     return true;
 *   }
 *
 *   main() {
 *     // create new HTTP client
 *     Q_HTTPCLIENT *httpClient = qHttpClient("www.qdecoder.org", 80);
 *     if(httpClient == NULL) return;
 *
 *     // open file
 *     int nFd = open("/tmp/test.data", O_WRONLY | O_CREAT, 0644);
 *
 *     // set additional custom headers
 *     Q_ENTRY *pReqHeaders = qEntry();
 *     Q_ENTRY *pResHeaders = qEntry();
 *
 *     // set userdata
 *     struct userdata mydata;
 *     ...(codes)...
 *
 *     // send file
 *     int nRescode = 0;
 *     off_t nSavesize = 0;
 *     bool bRet = httpClient->get(httpClient, "/img/qdecoder.png", nFd, &nSavesize, &nRescode,
 *                                 pReqHeaders, pResHeaders,
 *                                 callback, (void*)&mydata);
 *     // to print out request, response headers
 *     pReqHeaders->print(pReqHeaders, stdout, true);
 *     pResHeaders->print(pResHeaders, stdout, true);
 *
 *     // check results
 *     if(bRet == false) {
 *       ...(error occured)...
 *     }
 *
 *     // free resources
 *     httpClient->free(httpClient);
 *     pReqHeaders->free(pReqHeaders);
 *     pResHeaders->free(pResHeaders);
 *     close(nFd);
 *   }
 * @endcode
 */
static bool _get(Q_HTTPCLIENT *client, const char *uri, int fd, off_t *savesize, int *rescode,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders,
		bool (*callback)(void *userdata, off_t recvbytes), void *userdata) {

	// reset rescode
	if(rescode != NULL) *rescode = 0;
	if(savesize != NULL) *savesize = 0;

	// generate request headers if necessary
	bool freeReqHeaders = false;
	if(reqheaders == NULL) {
		reqheaders = qEntry();
		freeReqHeaders = true;
	}

	// add additional headers
	reqheaders->putStr(reqheaders,  "Accept", "*/*", true);

	// send request
	bool sendRet = _sendRequest(client, "GET", uri, reqheaders);
	if(freeReqHeaders == true) reqheaders->free(reqheaders);
	if(sendRet == false) return false;

	// generate response headers if necessary
	bool freeResHeaders = false;
	if(resheaders == NULL) {
		resheaders = qEntry();
		freeResHeaders = true;
	}

	// read response
	int resno = _readResponse(client, resheaders);
	if(rescode != NULL) *rescode = resno;

	// parse contents-length
	const char *clen_header = resheaders->getStrCase(resheaders, "Content-Length", false);
	off_t length = 0;
	if(clen_header != NULL) length = atoll(clen_header);

	if(freeResHeaders == true) resheaders->free(resheaders);

	// check response code
	if(resno != HTTP_CODE_OK) {
		_close(client);
		if(freeResHeaders == true) resheaders->free(resheaders);
		return false;
	}

	// retrieve data
	off_t recv = 0;
	if(callback != NULL) {
		if(callback(userdata, recv) == false) {
			_close(client);
			return false;
		}
	}
	if(length > 0) {
		while(recv < length) {
			size_t recvsize;	// this time receive size
			if(length - recv < MAX_ATOMIC_DATA_SIZE) recvsize = length - recv;
			else recvsize = MAX_ATOMIC_DATA_SIZE;

			ssize_t ret = qIoSend(fd, client->socket, recvsize, client->timeoutms);
			if(ret <= 0) break; // Connection closed by peer
			recv += ret;
			if(savesize != NULL) *savesize = recv;

			if(callback != NULL) {
				if(callback(userdata, recv) == false) {
					_close(client);
					return false;
				}
			}
		}

		if(recv != length) {
			_close(client);
			return false;
		}

		if(callback != NULL) {
			if(callback(userdata, recv) == false) {
				_close(client);
				return false;
			}
		}
	}

	// close connection
	if(client->keepalive == false) {
		_close(client);
	}

	return true;
}

/**
 * Q_HTTPCLIENT->put(): Upload file to remote host using PUT method.
 *
 * @param client	Q_HTTPCLIENT object pointer.
 * @param uri		remote URL for uploading file. ("/path" or "http://.../path")
 * @param fd		opened file descriptor for reading.
 * @param length	send size.
 * @param rescode	if not NULL, remote response code will be stored. (can be NULL)
 * @param reqheaders	Q_ENTRY pointer which contains additional user request headers. (can be NULL)
 * @param resheaders	Q_ENTRY pointer for storing response headers. (can be NULL)
 * @param callback	set user call-back function. (can be NULL)
 * @param userdata	set user data for call-back. (can be NULL)
 *
 * @return	true if successful, otherwise returns false
 *
 * @note
 * The call-back function will be called peridically whenever it send data as much as MAX_ATOMIC_DATA_SIZE.
 * To stop uploading, return false in the call-back function, then PUT process will be stopped immediately.
 * If a connection was not opened, it will open a connection automatically.
 *
 * @code
 *   struct userdata {
 *     ...
 *   };
 *
 *   static bool callback(void *userdata, off_t sentbytes) {
 *     struct userdata *pMydata = (struct userdata*)userdata;
 *     ...(codes)...
 *     if(need_to_cancel) return false; // stop file uploading immediately
 *     return true;
 *   }
 *
 *   main() {
 *     // create new HTTP client
 *     Q_HTTPCLIENT *httpClient = qHttpClient("www.qdecoder.org", 80);
 *     if(httpClient == NULL) return;
 *
 *     // open file
 *     int nFd = open(...);
 *     off_t nFileSize = ...;
 *     char *pFileMd5sum = ...;
 *     time_t nFileDate = ...;
 *
 *     // set additional custom headers
 *     Q_ENTRY *pReqHeaders = qEntry();
 *     pReqHeaders->putStr(pReqHeaders, "X-FILE-MD5SUM", pFileMd5sum, true);
 *     pReqHeaders->putInt(pReqHeaders, "X-FILE-DATE", nFileDate, true);
 *
 *     // set userdata
 *     struct userdata mydata;
 *     ...(codes)...
 *
 *     // send file
 *     int nRescode = 0;
 *     Q_ENTRY *pResHeaders = qEntry();
 *     bool bRet = httpClient->put(httpClient, "/img/qdecoder.png", nFd, nFileSize, &nRescode,
 *                                      pReqHeaders, pResHeaders,
 *                                      callback, (void*)&mydata);
 *     // to print out request, response headers
 *     pReqHeaders->print(pReqHeaders, stdout, true);
 *     pResHeaders->print(pResHeaders, stdout, true);
 *
 *     // check results
 *     if(bRet == false) {
 *       ...(error occured)...
 *     }
 *
 *     // free resources
 *     httpClient->free(httpClient);
 *     pReqHeaders->free(pReqHeaders);
 *     pResHeaders->free(pResHeaders);
 *     close(nFd);
 *   }
 * @endcode
 */
static bool _put(Q_HTTPCLIENT *client, const char *uri, int fd, off_t length, int *rescode,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders,
		bool (*callback)(void *userdata, off_t sentbytes), void *userdata) {

	// reset rescode
	if(rescode != NULL) *rescode = 0;

	// generate request headers
	bool freeReqHeaders = false;
	if(reqheaders == NULL) {
		reqheaders = qEntry();
		freeReqHeaders = true;
	}

	// add additional headers
	reqheaders->putStrf(reqheaders, true, "Content-Length", "%jd", length);
	reqheaders->putStr(reqheaders,  "Expect", "100-continue", true);

	// send request
	bool sendRet =_sendRequest(client, "PUT", uri, reqheaders);
	if(freeReqHeaders == true) reqheaders->free(reqheaders);
	if(sendRet == false) return false;

	// wait 100-continue
	if(qIoWaitReadable(client->socket, client->timeoutms) <= 0) {
		DEBUG("timed out %d", client->timeoutms);
		_close(client);
		return false;
	}

	// read response
	int resno = _readResponse(client, resheaders);
	if(resno != HTTP_CODE_CONTINUE) {
		if(rescode != NULL) *rescode = resno;
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
			if(length - sent < MAX_ATOMIC_DATA_SIZE) sendsize = length - sent;
			else sendsize = MAX_ATOMIC_DATA_SIZE;

			ssize_t ret = qIoSend(client->socket, fd, sendsize, client->timeoutms);
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
	resno = _readResponse(client, resheaders);
	if(rescode != NULL) *rescode = resno;

	if(resno == HTTP_NO_RESPONSE) {
		_close(client);
		return false;
	}

	if(resno != HTTP_CODE_CREATED) {
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
 * Q_HTTPCLIENT->cmd(): Send custom method to remote host.
 *
 * @param client	Q_HTTPCLIENT object pointer.
 * @param method	method name.
 * @param uri		remote URL for uploading file. ("/path" or "http://.../path")
 * @param rescode	if not NULL, remote response code will be stored. (can be NULL)
 * @param contentslength if not NULL, the contents length will be stored. (can be NULL)
 * @param reqheaders	Q_ENTRY pointer which contains additional user request headers. (can be NULL)
 * @param resheaders	Q_ENTRY pointer for storing response headers. (can be NULL)
 *
 * @return	malloced contents data if successful, otherwise returns NULL
 *
 * @code
 *   int nResCode;
 *   size_t nContentsLength;
 *   void *contents = httpClient->cmd(httpClient, "DELETE" "/img/qdecoder.png",
 *                                      &nRescode, &nContentsLength
 *                                      NULL, NULL);
 *   if(contents == NULL) {
 *     ...(error occured)...
 *   } else {
 *     printf("Response code : %d\n", nResCode);
 *     printf("Contents length : %zu\n", nContentsLength);
 *     printf("Contents : %s\n", (char*)contents);  // if contents is printable
 *     free(contents);  // de-allocate
 *   }
 * @endcode
 */
static void *_cmd(Q_HTTPCLIENT *client, const char *method, const char *uri,
		int *rescode, size_t *contentslength,
		Q_ENTRY *reqheaders, Q_ENTRY *resheaders) {

	// reset rescode
	if(rescode != NULL) *rescode = 0;
	if(contentslength != NULL) *contentslength = 0;

	// send request
	if(_sendRequest(client, method, uri, reqheaders) == false) {
		return NULL;
	}

	// read response
	bool freeResHeaders = false;
	if(resheaders == NULL) {
		resheaders = qEntry();
		freeResHeaders = true;
	}

	int resno = _readResponse(client, resheaders);
	if(rescode != NULL) *rescode = resno;

	// parse contents-length
	size_t length = resheaders->getInt(resheaders, "Content-Length");
	if(freeResHeaders == true) resheaders->free(resheaders);

	// malloc data
	void *contents = NULL;
	if(length > 0) {
		contents = malloc(length + 1);
		if(contents != NULL) {
			if(qIoRead(contents, client->socket, length, client->timeoutms) != length) {
				free(contents);
				contents = NULL;
			} else {
				*(char*)(contents + length) = '\0';
			}
		}
	}

	// close connection
	if(client->keepalive == false) {
		_close(client);
	}

	if(contents == NULL) {
		contents = strdup("");
		length = strlen(contents);
	}
	if(contentslength != NULL) *contentslength = length;

	return contents;
}

/**
 * Q_HTTPCLIENT->close(): Close the connection.
 *
 * @param Q_HTTPCLIENT	HTTP object pointer
 *
 * @return	true if successful, otherwise returns false
 *
 * @code
 *   httpClient->close(httpClient);
 * @endcode
 */
static bool _close(Q_HTTPCLIENT *client) {
	if(client->socket < 0) return false;

	// shutdown connection
	if(MAX_SHUTDOWN_WAIT >= 0 && shutdown(client->socket, SHUT_WR) == 0) {
		char buf[1024];
		while(qIoRead(buf, client->socket, sizeof(buf), MAX_SHUTDOWN_WAIT) > 0);
	}
	close(client->socket);
	client->socket = -1;

	return true;
}

/**
 * Q_HTTPCLIENT->free(): De-allocate object.
 *
 * @param Q_HTTPCLIENT	HTTP object pointer
 *
 * @note
 * If the connection was not closed, it will close the connection first prior to de-allocate object.
 *
 * @code
 *   httpClient->free(httpClient);
 * @endcode
 */
static void _free(Q_HTTPCLIENT *client) {
	if(client->socket >= 0) {
		client->close(client);
	}

	if(client->hostname != NULL) free(client->hostname);
	if(client->useragent != NULL) free(client->useragent);

	free(client);
}


#ifndef _DOXYGEN_SKIP
static bool _setSocketOption(int socket) {
	bool ret = true;

	// linger option
	if(SET_TCP_LINGER_TIMEOUT > 0) {
		struct linger li;
		li.l_onoff = 1;
		li.l_linger = SET_TCP_LINGER_TIMEOUT;
		if(setsockopt(socket, SOL_SOCKET, SO_LINGER, &li, sizeof(struct linger)) < 0) {
			ret = false;
		}
	}

	// nodelay option
	if(SET_TCP_NODELAY > 0) {
		int so_tcpnodelay = 1;
		if(setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &so_tcpnodelay, sizeof(so_tcpnodelay)) < 0) {
			ret = false;
		}
	}

	return ret;
}
#endif /* _DOXYGEN_SKIP */

#endif /* DISABLE_SOCKET */
