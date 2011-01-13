/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
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
 *
 * $Id$
 */

/**
 * @file qCgiResponse.c CGI Response API
 */

#ifdef ENABLE_FASTCGI
#include "fcgi_stdio.h"
#else
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "qdecoder.h"
#include "internal.h"

/**
 * Set cookie
 *
 * @param request	a pointer of request structure
 * @param name		cookie name
 * @param value		cookie value
 * @param expire	expire related time in seconds (0 means end of session)
 * @param path		cookie path (NULL can current path)
 * @param domain	cookie domain (NULL means current domain)
 * @param secure	secure flag
 *
 * @return	true in case of success, otherwise returns false
 *
 * @code
 *   // Apply cookie in the current domain and directory for 1 day.
 *   qCgiResponseSetCookie(req, "NAME", "qDecoder", 86400, NULL, NULL, false);
 *
 *   // Apply cookie to the "/" directory of "*.qdecoder.org" until the
 *   // browser is closed.
 *   qCgiResponseSetCookie(req, name, value, 0, "/", ".qdecoder.org", false);
 *
 *   // As for the followings, cookies will be set up only when security
 *   // requirements are satisfied.
 *   qCgiResponseSetCookie(req, name, value, 0, NULL, NULL, true);
 * @endcode
 */
bool qCgiResponseSetCookie(Q_ENTRY *request, const char *name, const char *value, int expire, const char *path, const char *domain, bool secure) {
	/* check content flag */
	if (qCgiResponseGetContentType(request) != NULL) {
		DEBUG("Should be called before qCgiResponseSetContentType().");
		return false;
	}

	/* name=value */
	char *encname = _qdecoder_urlencode(name, strlen(name));
	char *encvalue = _qdecoder_urlencode(value, strlen(value));
	char cookie[(4 * 1024) + 256];
	snprintf(cookie, sizeof(cookie), "%s=%s", encname, encvalue);
	free(encname), free(encvalue);

	if (expire != 0) {
		char gmtstr[sizeof(char) * (CONST_STRLEN("Mon, 00 Jan 0000 00:00:00 GMT") + 1)];
		time_t utctime = time(NULL);
		struct tm *gmtm = gmtime(&utctime);
		strftime(gmtstr, sizeof(gmtstr), "%a, %d %b %Y %H:%M:%S GMT", gmtm);

		strcat(cookie, "; expires=");
		strcat(cookie, gmtstr);
	}

	if (path != NULL) {
		if (path[0] != '/') {
			DEBUG("Path string(%s) must start with '/' character.", path);
			return false;
		}
		strcat(cookie, "; path=");
		strcat(cookie, path);
	}

	if (domain != NULL) {
		if (strstr(domain, "/") != NULL || strstr(domain, ".") == NULL) {
			DEBUG("Invalid domain name(%s).", domain);
			return false;
		}
		strcat(cookie, "; domain=");
		strcat(cookie, domain);
	}

	if (secure == true) {
		strcat(cookie, "; secure");
	}

	printf("Set-Cookie: %s\n", cookie);

	return true;
}

/**
 * Remove cookie
 *
 * @param request	a pointer of request structure
 * @param name		cookie name
 * @param path		cookie path
 * @param domain	cookie domain
 * @param secure	secure flag
 *
 * @return      true in case of success, otherwise returns false
 *
 * @code
 *   qCgiResponseSetCookie(req, "NAME", "VALUE", 0, NULL, NULL, NULL);
 *   qCgiResponseRemoveCookie(req, "NAME", NULL, NULL, NULL);
 *
 *   qCgiResponseSetCookie(req, "NAME", "VALUE", 0, "/", "www.qdecoder.org", NULL);
 *   qCgiResponseRemoveCookie(req, "NAME", "/", "www.qdecoder.org", NULL);
 * @endcode
 */
bool qCgiResponseRemoveCookie(Q_ENTRY *request, const char *name, const char *path, const char *domain, bool secure) {
        return qCgiResponseSetCookie(request, name, "", -1, path, domain, secure);
}

/**
 * Set responding content-type
 *
 * @param request	a pointer of request structure
 * @param mimetype	mimetype
 *
 * @return      true in case of success, otherwise returns false
 *
 * @code
 *   qCgiResponseSetContentType(req, "text/html");
 * @endcode
 */
bool qCgiResponseSetContentType(Q_ENTRY *request, const char *mimetype) {
	if(request != NULL && request->getStr(request, "_Q_CONTENTTYPE", false) != NULL) {
		DEBUG("alreay set.");
		return false;
	}

	printf("Content-Type: %s\n\n", mimetype);

	if(request != NULL) request->putStr(request, "_Q_CONTENTTYPE", mimetype, true);
	return true;
}

/**
 * Get content-type
 *
 * @param request	a pointer of request structure
 *
 * @return      a pointer of mimetype string in case of success, otherwise returns NULL
 *
 * @code
 *   qCgiResponseSetContentType(req, "text/html");
 * @endcode
 */
const char *qCgiResponseGetContentType(Q_ENTRY *request) {
	return request->getStr(request, "_Q_CONTENTTYPE", false);
}

/**
 * Send redirection header
 *
 * @param request	a pointer of request structure
 * @param uri		new URI
 *
 * @return      true in case of success, otherwise returns false
 *
 * @code
 *   qCgiResponseRedirect(req, "http://www.qdecoder.org/");
 * @endcode
 */
bool qCgiResponseRedirect(Q_ENTRY *request, const char *uri) {
	if(qCgiResponseGetContentType(request) != NULL) {
		DEBUG("Should be called before qCgiResponseSetContentType().");
		return false;
	}

	printf("Location: %s\n\n", uri);
	return true;
}

/**
 * Force to send(download) file to client in accordance with given mime type.
 *
 * @param request	a pointer of request structure
 * @param filepath	file to send
 * @param mimetype	mimetype. NULL can be used for "application/octet-stream" mimetype.
 *
 * @return	 	the number of bytes sent. otherwise(file not found) returns -1.
 *
 * @note
 * Do not call qCgiResponseGetContentType() before.
 * The results of this function are the same as those acquired
 * when the corresponding files are directly linked to the Web.
 * But this is especially useful in preprocessing files to be downloaded
 * only with user certification and in enabling downloading those files,
 * which cannot be opend on the Web, only through specific programs.
 */
int qCgiResponseDownload(Q_ENTRY *request, const char *filepath, const char *mimetype) {
	if (qCgiResponseGetContentType(request) != NULL) {
		DEBUG("Should be called before qCgiResponseSetContentType().");
		return -1;
	}

	int fd;
	if (filepath == NULL || (fd = open(filepath, O_RDONLY, 0)) < 0) {
		DEBUG("Can't open file.");
		return -1;
	}

	const char *mime;
	if(mimetype == NULL) mime = "application/octet-stream";
	else mime = mimetype;

	char *disposition;
	if (!strcmp(mime, "application/octet-stream")) disposition = "attachment";
	else disposition = "inline";

	char *filename = _qdecoder_filename(filepath);
	off_t filesize = _qdecoder_filesize(filepath);

	printf("Content-Disposition: %s;filename=\"%s\"\n", disposition, filename);
	printf("Content-Transfer-Encoding: binary\n");
	printf("Accept-Ranges: bytes\n");
	printf("Content-Length: %zu\n", (size_t)filesize);
	printf("Connection: close\n");
	qCgiResponseSetContentType(request, mime);

	free(filename);

	fflush(stdout);

	int sent = _qdecoder_iosend(fileno(stdout), fd, filesize);

	close(fd);
	return sent;
}

/**
 * Print out HTML error page and exit program
 *
 * @param request	a pointer of request structure
 * @param format	error message
 *
 * @return      none
 *
 * @code
 *   qCgiResponseError(req, "Error: can't find userid.");
 * @endcode
 */
void qCgiResponseError(Q_ENTRY *request, char *format, ...) {
	char *buf;
	DYNAMIC_VSPRINTF(buf, format);
	if(buf == NULL) {
		exit(EXIT_FAILURE);
	}

	if (getenv("REMOTE_ADDR") == NULL)  {
		printf("Error: %s\n", buf);
	} else {
		qCgiResponseSetContentType(request, "text/html");

		printf("<html>\n");
		printf("<head>\n");
		printf("<title>Error: %s</title>\n", buf);
		printf("<script language='JavaScript'>\n");
		printf("  alert(\"%s\");\n", buf);
		printf("  history.back();\n");
		printf("</script>\n");
		printf("</head>\n");
		printf("</html>\n");
	}

	free(buf);
	if(request != NULL) request->free(request);
	exit(EXIT_FAILURE);
}
