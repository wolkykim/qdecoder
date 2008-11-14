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
 * @file qCgiResponse.c CGI Cookie Handling API
 */

#ifndef DISABLE_CGISUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "qDecoder.h"
#include "qInternal.h"

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
	char *encname = qEncodeUrl(name);
	char *encvalue = qEncodeUrl(value);
	char cookie[(4 * 1024) + 256];
	snprintf(cookie, sizeof(cookie), "%s=%s", encname, encvalue);
	free(encname), free(encvalue);

	if (expire != 0) {
		char *gmtstr = qTimeGetGmtStr(time(NULL) + expire);
		strcat(cookie, "; expires=");
		strcat(cookie, gmtstr);
		free(gmtstr);
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
	if(qEntryGetStr(request, "_Q_CONTENTTYPE") != NULL) {
		DEBUG("alreay set.");
		return false;
	}

	printf("Content-Type: %s\n\n", mimetype);

	qEntryPutStr(request, "_Q_CONTENTTYPE", mimetype, true);
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
	return qEntryGetStr(request, "_Q_CONTENTTYPE");
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

	char *filename = qFileGetName(filepath);

	printf("Content-Disposition: %s;filename=\"%s\"\n", disposition, filename);
	printf("Content-Transfer-Encoding: binary\n");
	printf("Accept-Ranges: bytes\n");
	printf("Content-Length: %zu\n", (size_t)qFileGetSize(filepath));
	printf("Connection: close\n");
	qCgiResponseSetContentType(request, mime);

	free(filename);

	fflush(stdout);
	int sent = qFileSend(fileno(stdout), fd, 0);

	close(fd);
	return sent;
}
/**
 * Generate and print out HTML error page
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
	char buf[MAX_LINEBUF];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

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

	qEntryFree(request);
	exit(EXIT_FAILURE);
}

#endif /* DISABLE_CGISUPPORT */
