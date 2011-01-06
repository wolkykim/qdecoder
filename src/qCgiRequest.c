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
 * @file qCgiRequest.c Web Query & Cookie Handling API
 *
 * qDecoder supports parsing
 *   @li COOKIE
 *   @li GET method
 *   @li POST method (application/x-www-form-urlencoded: default FORM encoding)
 *   @li POST method (multipart/form-data: especially used for file uploading)
 *
 * Anyway you don't care about this. You don't need to know which method
 * (COOKIE/GET/POST) is used for sending data.
 *
 * @code
 *   [HTML sample]
 *   <form action="your_program.cgi">
 *     <input type="text" name="color">
 *     <input type="submit">
 *   </form>
 *
 *   [Your Source]
 *   Q_ENTRY *req = qCgiRequestParse(NULL, 0);
 *   const char *color = req->getStr(req, "color", false);
 *   printf("color = %s\n", color);
 *   req->free(req);
 * @endcode
 *
 * The storing sequence is (1)COOKIE (2)GET (3)POST. Thus if same query names
 * (which are sent by different method) exist, COOKIE query will be returned.
 *
 * If you would like to get POST queries only. See below sample codes.
 * @code
 *   Q_ENTRY *req = qCgiRequestParse(NULL, Q_CGI_POST);
 *   const char *color = req->getStr(req, "color", false);
 *   printf("color = %s\n", color);
 *   req->free(req);
 * @endcode
 *
 * If you would like to get POST and COOKIE queries only. See below sample codes.
 * @code
 *   Q_ENTRY *req = NULL;
 *   req = qCgiRequestParse(req, Q_CGI_COOKIE);
 *   req = qCgiRequestParse(req, Q_CGI_POST);
 *   const char *color = req->getStr(req, "color", false);
 *   printf("color = %s\n", color);
 *   req->free(req);
 * @endcode

 * In case of multipart/form-data encoding(used for file uploading), qDecoder
 * supports 2 modes for handling file uploading. I just made a name for that.
 *
 * @li <b>default mode</b> : Uploading file will be processed only in memory.
 * (see examples/upload.c)
 * @li <b>file mode</b>	: Uploading file will be stored into unique temporary
 * file on the fly.
 * There is sub mode named <b>progress mode</b>. This is same as file mode.
 * But user can see the progress bar while uploading.
 * When file mode is turned on and if you add some specific HTML codes
 * in your uploading page, progressive window will be shown automatically.
 * (see examples/uploadprogress.c)
 *
 * You can switch to file mode by calling qCgiRequestSetOption().
 * @code
 *   Q_ENTRY *req = qCgiRequestSetOption(NULL, true, "/tmp", 86400);
 *   req = qCgiRequestParse(req, 0);
 *   (...your codes here...)
 *   req->free(req);
 * @endcode
 *
 * Basically, when file is uploaded qDecoder store it's meta information like
 * below.
 * @li (VARIABLE_NAME)		- In the <b>default mode</b>, this is binary
 * data. In the <b>file mode</b> this value is same as "(VARIABLE_NAME).savepath".
 * @li (VARIABLE_NAME).filename	- File name itself, path information will be removed.
 * @li (VARIABLE_NAME).length	- File size, the number of bytes.
 * @li (VARIABLE_NAME).contenttype - Mime type like 'text/plain'.
 * @li (VARIABLE_NAME).savepath	- Only appended only in <b>file mode</b>.
 * The file path where the uploaded file is saved.
 *
 * @code
 *   [default mode example]
 *   binary = (...binary data...)
 *   binary.filename = hello.xls
 *   binary.length = 3292
 *   binary.contenttype = application/vnd.ms-excel
 *
 *   [file mode example]
 *   binary = tmp/Q_7b91698bc6d6ac7eaac93e71ce729b7c/1-hello.xls
 *   binary.filename = hello.xls
 *   binary.length = 3292
 *   binary.contenttype = application/vnd.ms-excel
 *   binary.savepath = tmp/Q_7b91698bc6d6ac7eaac93e71ce729b7c/1-hello.xls
 * @endcode
 *
 * If you want to activate progress mode. Please follow below steps
 * @li 1) copy "examples/qDecoder-upload/" folder under your Web Document Root.
 * @li 2) modify your HTML codes like below.
 * @code
 *   [HTML sample]
 *   <script language="JavaScript"
 *           src="/_(SOME_PATH)_/qDecoder-upload/qDecoder-upload.js"></script>
 *   <form method="post" action="your_program.cgi"
 *         enctype="multipart/form-data" onSubmit="return Q_UPLOAD(this);">
 *     <input type="hidden" name="Q_UPLOAD_ID" value="">
 *
 *     Input text: <input type="text" name="text">
 *     <br>Select file: <input type="file" name="binary1">
 *     <br>Another file: <input type="file" name="binary2">
 *     <br><input type="submit">
 *   </form>
 *   int main(...) {
 *     // the first line in main().
 *     Q_ENTRY *req = qCgiRequestSetOption(NULL, true, "/tmp", 86400);
 *     req = qCgiRequestParse(req, 0);
 *
 *     (...your codes here...)
 *
 *     req->free(req);
 *   }
 * @endcode
 */

#ifdef ENABLE_FASTCGI
#include "fcgi_stdio.h"
#else
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <dirent.h>	/* to use opendir() */
#endif
#include "qDecoder.h"
#include "qInternal.h"

#ifndef _DOXYGEN_SKIP
static int  _parse_multipart(Q_ENTRY *request);
static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, bool *finish);
static char *_parse_multipart_value_into_disk(const char *boundary, const char *savedir, const char *filename, int *filelen, bool *finish);
static int _upload_clear_base(const char *upload_basepath, int upload_clearold);
static Q_ENTRY *_parse_query(Q_ENTRY *request, const char *query, char equalchar, char sepchar, int *count);
#endif

/**
 * Set request parsing option for file uploading in case of multipart/form-data encoding.
 *
 * @param request	Q_ENTRY container pointer that options will be set. NULL can be used to create a new container.
 * @param filemode	false for parsing in memory, true for storing attached files into file-system directly.
 * @param basepath	the base path where the uploaded files are located. Set to NULL if filemode is false.
 * @param clearold	saved files older than this seconds will be removed automatically. Set to 0 to disable.
 *
 * @return	Q_ENTRY container pointer, otherwise returns NULL.
 *
 * @note
 * This method should be called before calling qCgiRequestParse().
 *
 * @code
 *   Q_ENTRY *req = qCgiRequestSetOption(NULL, true, "/tmp", 86400);
 *   req = qCgiRequestParse(req, 0);
 *   req->free(req);
 * @endcode
 */
Q_ENTRY *qCgiRequestSetOption(Q_ENTRY *request, bool filemode, const char *basepath, int clearold) {
	// initialize entry structure
	if(request == NULL) {
		request = qEntry();
		if(request == NULL) return NULL;
	}

	if(filemode == true) {
		if(basepath == NULL || access(basepath, R_OK|W_OK|X_OK) != 0) {
			request->free(request);
			return NULL;
		}

		// clear old files
		if(_upload_clear_base(basepath, clearold) < 0) {
			request->free(request);
			return NULL;
		}

		// save info
		request->putStr(request, "_Q_UPLOAD_BASEPATH", basepath, true);
		request->putInt(request, "_Q_UPLOAD_CLEAROLD", clearold, true);

	} else {
		request->remove(request, "_Q_UPLOAD_BASEPATH");
		request->remove(request, "_Q_UPLOAD_CLEAROLD");
	}

	return request;
}

/**
 * Parse one or more request(COOKIE/GET/POST) queries.
 *
 * @param request	Q_ENTRY container pointer that parsed key/value pairs will be stored. NULL can be used to create a new container.
 * @param method	Target mask consists of one or more of Q_CGI_COOKIE, Q_CGI_GET and Q_CGI_POST. Q_CGI_ALL or 0 can be used for parsing all of those types.
 *
 * @return		Q_ENTRY* handle if successful, NULL if there was insufficient memory to allocate a new object.
 *
 * @code
 *   Q_ENTRY *req = qCgiRequestParse(NULL, 0);
 *   char *name = req->getStr(req, "name", false);
 *   if(name != NULL) printf("%s\n", name);
 *   req->free(req);
 * @endcode
 *
 * @note
 * The queries are parsed with sequence of (1)COOKIE, (2)GET (3)POST.
 */
Q_ENTRY *qCgiRequestParse(Q_ENTRY *request, Q_CGI_T method) {
	// initialize entry structure
	if(request == NULL) {
		request = qEntry();
		if(request == NULL) return NULL;
	}

	// parse COOKIE
	if(method == Q_CGI_ALL || (method & Q_CGI_COOKIE) != 0) {
		char *query = qCgiRequestGetQuery(Q_CGI_COOKIE);
		if(query != NULL) {
			_parse_query(request, query, '=', ';', NULL);
			free(query);
		}
	}

	// parse GET method
	if(method == Q_CGI_ALL || (method & Q_CGI_GET) != 0){
		char *query = qCgiRequestGetQuery(Q_CGI_GET);
		if(query != NULL) {
			_parse_query(request, query, '=', '&', NULL);
			free(query);
		}
	}

	//  parse POST method
	if(method == Q_CGI_ALL || (method & Q_CGI_POST) != 0) {
		const char *content_type = getenv("CONTENT_TYPE");
		if(content_type == NULL) content_type = "";
		if (!strncmp(content_type, "application/x-www-form-urlencoded", CONST_STRLEN("application/x-www-form-urlencoded"))) {
			char *query = qCgiRequestGetQuery(Q_CGI_POST);
			if(query != NULL) {
				_parse_query(request, query, '=', '&', NULL);
				free(query);
			}
		} else if (!strncmp(content_type, "multipart/form-data", CONST_STRLEN("multipart/form-data"))) {
			_parse_multipart(request);
		}
	}

	return request;
}

/**
 * Get raw query string.
 *
 * @param method	One of Q_CGI_COOKIE, Q_CGI_GET and Q_CGI_POST.
 *
 * @return		malloced query string otherwise returns NULL;
 *
 * @code
 *   char *query = qCgiRequestGetQuery(Q_CGI_GET);
 *   if(query != NULL) {
 *     printf("%s\n", query);
 *     free(query);
 *   }
 * @endcode
 */
char *qCgiRequestGetQuery(Q_CGI_T method) {
	if (method == Q_CGI_GET) {
		char *request_method = getenv("REQUEST_METHOD");
		if (request_method != NULL && strcmp(request_method, "GET")) return NULL;

		char *query_string = getenv("QUERY_STRING");
		char *req_uri = getenv("REQUEST_URI");
		if (query_string == NULL) return NULL;

		char *query = NULL;

		// SSI query handling
		if (strlen(query_string) == 0 && req_uri != NULL) {
			char *cp;
			for (cp = req_uri; *cp != '\0'; cp++) {
				if (*cp == '?') {
					cp++;
					break;
				}
			}
			query = strdup(cp);
		} else {
			query = strdup(query_string);
		}

		return query;
	} else if (method == Q_CGI_POST) {
		char *request_method = getenv("REQUEST_METHOD");
		char *content_length = getenv("CONTENT_LENGTH");
		if (request_method == NULL || strcmp(request_method, "POST") || content_length == NULL) return NULL;

		int i, cl = atoi(content_length);
		char *query = (char *)malloc(sizeof(char) * (cl + 1));
		for (i = 0; i < cl; i++)query[i] = fgetc(stdin);
		query[i] = '\0';
		return query;
	} else if (method == Q_CGI_COOKIE) {
		char *http_cookie = getenv("HTTP_COOKIE");
		if (http_cookie == NULL) return NULL;
		char *query = strdup(http_cookie);
		return query;
	}

	return NULL;
}

#ifndef _DOXYGEN_SKIP

/* For decode multipart/form-data, used by qRequestInit() */
static int _parse_multipart(Q_ENTRY *request) {
#ifdef _WIN32
	setmode(fileno(stdin), _O_BINARY);
	setmode(fileno(stdout), _O_BINARY);
#endif

	char buf[MAX_LINEBUF];
	int  amount = 0;

	/*
	 * For parse multipart/form-data method
	 */

	/* Force to check the boundary string length to defense overflow attack */
	char boundary[256];
	int maxboundarylen = CONST_STRLEN("--");
	maxboundarylen += strlen(strstr(getenv("CONTENT_TYPE"), "boundary=") + CONST_STRLEN("boundary="));
	maxboundarylen += CONST_STRLEN("--");
	maxboundarylen += CONST_STRLEN("\r\n");
	if (maxboundarylen >= sizeof(boundary)) {
		DEBUG("The boundary string is too long(Overflow Attack?). stopping process.");
		return amount;
	}

	/* find boundary string - Hidai Kenichi made this patch for handling quoted boundary string */
	char boundary_orig[256];
	_qdecoder_strcpy(boundary_orig, sizeof(boundary_orig), strstr(getenv("CONTENT_TYPE"), "boundary=") + CONST_STRLEN("boundary="));
	_qdecoder_strtrim(boundary_orig);
	_qdecoder_strunchar(boundary_orig, '"', '"');
	snprintf(boundary, sizeof(boundary), "--%s", boundary_orig);

	/* If you want to observe the string from stdin, enable this section. */
	/* This section is made for debugging.                                */
	if (false) {
		int i, j;
		qCgiResponseSetContentType(request, "text/html");

		printf("Content Length = %s<br>\n", getenv("CONTENT_LENGTH"));
		printf("Boundary len %zu : %s<br>\n", strlen(boundary), boundary);
		for (i = 0; boundary[i] != '\0'; i++) printf("%02X ", boundary[i]);
		printf("<p>\n");

		for (j = 1; _qdecoder_fgets(buf, sizeof(buf), stdin) != NULL; j++) {
			printf("Line %d, len %zu : %s<br>\n", j, strlen(buf), buf);
			//for (i = 0; buf[i] != '\0'; i++) printf("%02X ", buf[i]);
			printf("<br>\n");
		}
		exit(EXIT_SUCCESS);
	}

	/* check boundary */
	if (_qdecoder_fgets(buf, sizeof(buf), stdin) == NULL) {
		DEBUG("Bbrowser sent a non-HTTP compliant message.");
		return amount;
	}

	/* for explore 4.0 of NT, it sent \r\n before starting, fucking Micro$oft */
	if (!strcmp(buf, "\r\n")) _qdecoder_fgets(buf, sizeof(buf), stdin);

	if (strncmp(buf, boundary, strlen(boundary)) != 0) {
		DEBUG("Invalid string format.");
		return amount;
	}

	/* check file save mode */
	bool upload_filesave = false; /* false: save into memory, true: save into file */
	const char *upload_basepath = request->getStr(request, "_Q_UPLOAD_BASEPATH", false);
	if(upload_basepath != NULL) upload_filesave = true;

	bool  finish;
	for (finish = false; finish == false; amount++) {
		char *name = NULL, *value = NULL, *filename = NULL, *contenttype = NULL;
		int valuelen = 0;

		/* get information */
		while (_qdecoder_fgets(buf, sizeof(buf), stdin)) {
			if (!strcmp(buf, "\r\n")) break;
			else if (!strncasecmp(buf, "Content-Disposition: ", CONST_STRLEN("Content-Disposition: "))) {
				int c_count;

				/* get name field */
				name = strdup(buf + CONST_STRLEN("Content-Disposition: form-data; name=\""));
				for (c_count = 0; (name[c_count] != '\"') && (name[c_count] != '\0'); c_count++);
				name[c_count] = '\0';

				/* get filename field */
				if (strstr(buf, "; filename=\"") != NULL) {
					int erase;
					filename = strdup(strstr(buf, "; filename=\"") + CONST_STRLEN("; filename=\""));
					for (c_count = 0; (filename[c_count] != '\"') && (filename[c_count] != '\0'); c_count++);
					filename[c_count] = '\0';
					/* remove directory from path, erase '\' */
					for (erase = 0, c_count = strlen(filename) - 1; c_count >= 0; c_count--) {
						if (erase == 1) filename[c_count] = ' ';
						else {
							if (filename[c_count] == '\\') {
								erase = 1;
								filename[c_count] = ' ';
							}
						}
					}
					_qdecoder_strtrim(filename);

					if(strlen(filename) == 0) { /* empty attachment */
						free(filename);
						filename = NULL;
					}
				}
			} else if (!strncasecmp(buf, "Content-Type: ", CONST_STRLEN("Content-Type: "))) {
				contenttype = strdup(buf + CONST_STRLEN("Content-Type: "));
				_qdecoder_strtrim(contenttype);
			}
		}

		/* check */
		if(name == NULL) {
			DEBUG("bug or invalid format.");
			continue;
		}

		/* get value field */
		if (filename != NULL && upload_filesave == true) {
			char *tp, *savename = strdup(filename);
			for(tp = savename; *tp != '\0'; tp++) if(*tp == ' ') *tp = '_'; // replace ' ' to '_'
			value = _parse_multipart_value_into_disk(boundary, upload_basepath, savename, &valuelen, &finish);
			free(savename);

			if(value != NULL) request->putStr(request, name, value, false);
			else request->putStr(request, name, "(parsing failure)", false);
		} else {
			value = _parse_multipart_value_into_memory(boundary, &valuelen, &finish);

			if(value != NULL) request->put(request, name, value, valuelen+1, false);
			else request->putStr(request, name, "(parsing failure)", false);
		}

		/* store some additional info */
		if (value != NULL && filename != NULL) {
			char ename[255+10+1];

			/* store data length, 'NAME.length'*/
			snprintf(ename, sizeof(ename), "%s.length", name);
			request->putInt(request, ename, valuelen, false);

			/* store filename, 'NAME.filename'*/
			snprintf(ename, sizeof(ename), "%s.filename", name);
			request->putStr(request, ename, filename, false);

			/* store contenttype, 'NAME.contenttype'*/
			snprintf(ename, sizeof(ename), "%s.contenttype", name);
			request->putStr(request, ename, ((contenttype!=NULL)?contenttype:""), false);

			if (upload_filesave == true) {
				snprintf(ename, sizeof(ename), "%s.savepath", name);
				request->putStr(request, ename, value, false);
			}
		}

		/* free stuffs */
		if(name != NULL) free(name);
		if(value != NULL) free(value);
		if(filename != NULL) free(filename);
		if(contenttype != NULL) free(contenttype);
	}

	return amount;
}

#define	_Q_MULTIPART_CHUNK_SIZE		(64 * 1024)
static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, bool *finish) {
	char boundaryEOF[256], rnboundaryEOF[256];
	char boundaryrn[256], rnboundaryrn[256];
	int  boundarylen, boundaryEOFlen;

	char *value;
	int  length;
	int  c, c_count, mallocsize;

	/* set boundary strings */
	snprintf(boundaryEOF, sizeof(boundaryEOF), "%s--", boundary);
	snprintf(rnboundaryEOF, sizeof(rnboundaryEOF), "\r\n%s", boundaryEOF);
	snprintf(boundaryrn, sizeof(boundaryrn), "%s\r\n", boundary);
	snprintf(rnboundaryrn, sizeof(rnboundaryrn), "\r\n%s\r\n", boundary);

	boundarylen    = strlen(boundary);
	boundaryEOFlen = strlen(boundaryEOF);

	for (value = NULL, length = 0, mallocsize = _Q_MULTIPART_CHUNK_SIZE, c_count = 0; (c = fgetc(stdin)) != EOF; ) {
		if (c_count == 0) {
			value = (char *)malloc(sizeof(char) * mallocsize);
			if (value == NULL) {
				DEBUG("Memory allocation fail.");
				*finish = true;
				return NULL;
			}
		} else if (c_count == mallocsize - 1) {
			char *valuetmp;

			mallocsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			valuetmp = (char *)malloc(sizeof(char) * mallocsize);
			if (valuetmp == NULL) {
				DEBUG("Memory allocation fail.");
				free(value);
				*finish = true;
				return NULL;
			}
			memcpy(valuetmp, value, c_count);
			free(value);
			value = valuetmp;
		}
		value[c_count++] = (char)c;

		/* check end */
		if ((c == '\n') || (c == '-')) {
			value[c_count] = '\0';

			if ((c_count - (2 + boundarylen + 2)) >= 0) {
				if (!strcmp(value + (c_count - (2 + boundarylen + 2)), rnboundaryrn)) {
					value[c_count - (2 + boundarylen + 2)] = '\0';
					length = c_count - (2 + boundarylen + 2);
					break;
				}
			}
			if ((c_count - (2 + boundaryEOFlen)) >= 0) {
				if (!strcmp(value + (c_count - (2 + boundaryEOFlen)), rnboundaryEOF)) {
					value[c_count - (2 + boundaryEOFlen)] = '\0';
					length = c_count - (2 + boundaryEOFlen);
					*finish = true;
					break;
				}
			}

			/* For Micro$oft Explore on MAC, they do not follow rules */
			if ((c_count - (boundarylen + 2)) == 0) {
				if (!strcmp(value, boundaryrn)) {
					value[0] = '\0';
					length = 0;
					break;
				}
			}
			if ((c_count - boundaryEOFlen) == 0) {
				if (!strcmp(value, boundaryEOF)) {
					value[0] = '\0';
					length = 0;
					*finish = true;
					break;
				}
			}
		}
	}

	if (c == EOF) {
		DEBUG("Broken stream.");
		if(value != NULL) free(value);
		*finish = true;
		return NULL;
	}

	*valuelen = length;
	return value;
}

static char *_parse_multipart_value_into_disk(const char *boundary, const char *savedir, const char *filename, int *filelen, bool *finish) {
	char boundaryEOF[256], rnboundaryEOF[256];
	char boundaryrn[256], rnboundaryrn[256];
	int  boundarylen, boundaryEOFlen;

	/* input */
	char buffer[_Q_MULTIPART_CHUNK_SIZE];
	int  bufc;
	int  c;

	/* set boundary strings */
	snprintf(boundaryEOF, sizeof(boundaryEOF), "%s--", boundary);
	snprintf(rnboundaryEOF, sizeof(rnboundaryEOF), "\r\n%s", boundaryEOF);
	snprintf(boundaryrn, sizeof(boundaryrn), "%s\r\n", boundary);
	snprintf(rnboundaryrn, sizeof(rnboundaryrn), "\r\n%s\r\n", boundary);

	boundarylen    = strlen(boundary);
	boundaryEOFlen = strlen(boundaryEOF);

	/* open temp file */
	char upload_path[PATH_MAX];
	snprintf(upload_path, sizeof(upload_path), "%s/q_XXXXXX", savedir);

	int upload_fd = mkstemp(upload_path);
	if (upload_fd < 0) {
		DEBUG("Can't open file %s", upload_path);
		*finish = true;
		return NULL;
	}

	/* change permission */
	fchmod(upload_fd, DEF_FILE_MODE);

	/* read stream */
	int  upload_length;
	for (upload_length = 0, bufc = 0, upload_length = 0; (c = fgetc(stdin)) != EOF; ) {
		if (bufc == sizeof(buffer) - 1) {
			/* save */
			int leftsize = boundarylen + 8;
			int savesize = bufc - leftsize;
			write(upload_fd, buffer, savesize);
			memcpy(buffer, buffer+savesize, leftsize);
			bufc = leftsize;
		}
		buffer[bufc++] = (char)c;
		upload_length++;

		/* check end */
		if ((c == '\n') || (c == '-')) {
			buffer[bufc] = '\0';

			if ((bufc - (2 + boundarylen + 2)) >= 0) {
				if (!strcmp(buffer + (bufc - (2 + boundarylen + 2)), rnboundaryrn)) {
					bufc          -= (2 + boundarylen + 2);
					upload_length -= (2 + boundarylen + 2);
					break;
				}
			}
			if ((bufc - (2 + boundaryEOFlen)) >= 0) {
				if (!strcmp(buffer + (bufc - (2 + boundaryEOFlen)), rnboundaryEOF)) {
					bufc          -= (2 + boundaryEOFlen);
					upload_length -= (2 + boundaryEOFlen);
					*finish = true;
					break;
				}
			}

			/* For Micro$oft Explore on MAC, they do not follow rules */
			if (upload_length == bufc) {
				if ((bufc - (boundarylen + 2)) == 0) {
					if (!strcmp(buffer, boundaryrn)) {
						bufc = 0;
						upload_length = 0;
						break;
					}
				}
				if ((bufc - boundaryEOFlen) == 0) {
					if (!strcmp(buffer, boundaryEOF)) {
						bufc = 0;
						upload_length = 0;
						*finish = true;
						break;
					}
				}
			}
		}
	}

	if (c == EOF) {
		DEBUG("Broken stream.");
		*finish = true;
		return NULL;
	}

	/* save rest */
	write(upload_fd, buffer, bufc);
	close(upload_fd);

	*filelen = upload_length;

	return strdup(upload_path);
}

static int _upload_clear_base(const char *upload_basepath, int upload_clearold) {
#ifdef _WIN32
	return false;
#else
	if (upload_clearold <= 0) return -1;

	/* open upload folder */
	DIR     *dp;
	if ((dp = opendir(upload_basepath)) == NULL) return false;

	time_t now = time(NULL);
	int removed = 0;
	struct  dirent *dirp;
	while ((dirp = readdir(dp)) != NULL) {
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..") || strncmp(dirp->d_name, "q_", 2) != 0) continue;

		char filepath[PATH_MAX];
		snprintf(filepath, sizeof(filepath), "%s/%s", upload_basepath, dirp->d_name);

		// check file date
		struct stat filestat;
		if(stat(filepath, &filestat) != 0) continue;
		if(filestat.st_mtime >= now + upload_clearold) continue;

		// remove file
		if(_qdecoder_unlink(filepath) == 0) removed++;
	}
	closedir(dp);

	return removed;
#endif
}

static Q_ENTRY *_parse_query(Q_ENTRY *request, const char *query, char equalchar, char sepchar, int *count) {
	if(request == NULL) {
		request = qEntry();
		if(request == NULL) return NULL;
	}

	char *newquery = NULL;
	int cnt = 0;

	if(query != NULL) newquery = strdup(query);
	while (newquery && *newquery) {
		char *value = _qdecoder_makeword(newquery, sepchar);
		char *name = _qdecoder_strtrim(_qdecoder_makeword(value, equalchar));
		_qdecoder_urldecode(name);
		_qdecoder_urldecode(value);

		if(request->putStr(request, name, value, false) == true) cnt++;
		free(name);
		free(value);
	}
	if(newquery != NULL) free(newquery);
	if(count != NULL) *count = cnt;

	return request;
}

#endif /* _DOXYGEN_SKIP */
