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
 *   Q_ENTRY *req = qCgiRequestParse(NULL);
 *   const char *color = qEntryGetStr(req, "color");
 *   printf("color = %s\n", color);
 *   qEntryFree(req);
 * @endcode
 *
 * The storing sequence is (1)COOKIE (2)GET (3)POST. Thus if same query names
 * (which are sent by different method) exist, COOKIE query will be returned.
 *
 * If you would like to get POST queries only. See below sample codes.
 * @code
 *   Q_ENTRY *req = qCgiRequestParseQueries(NULL, "POST");
 *   const char *color = qEntryGetStr(req, "color");
 *   printf("color = %s\n", color);
 * @endcode
 *
 * If you would like to get POST and COOKIE queries only. See below sample codes.
 * @code
 *   Q_ENTRY *req = NULL;
 *   req = qCgiRequestParseCookies(req);
 *   req = qCgiRequestParseQueries(req, "POST");
 *   const char *color = qEntryGetStr(req, "color");
 *   printf("color = %s\n", color);
 *   qEntryFree(req);
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
 * You can switch to file mode by calling qCgiRequestParseOption().
 * @code
 *   Q_ENTRY *req = qCgiRequestParseOption(true, "/tmp", 86400);
 *   req = qCgiRequestParse(req);
 *   (...your codes here...)
 *   qEntryFree(req);
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
 *
 *   [Code sample]
 *   int main(...) {
 *     // if you use "progress mode", this codes should be located at
 *     // the first line in main().
 *     Q_ENTRY *req = qCgiRequestParseOption(true, "/tmp", 86400);
 *     req = qCgiRequestParse(req);
 *
 *     (...your codes here...)
 *
 *     qEntryFree(req);
 *   }
 * @endcode
 */

#ifndef DISABLE_CGI

#ifdef ENABLE_FASTCGI
#include "fcgi_stdio.h"
#else
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _WIN32
#include <dirent.h>	/* to use opendir() */
#endif
#include "qDecoder.h"
#include "qInternal.h"

/*
 * static function prototypes
 */
static int  _parse_multipart(Q_ENTRY *request);
static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, bool *finish);
static char *_parse_multipart_value_into_disk(const char *boundary, const char *savedir, const char *filename, int *filelen, bool *finish);

static char *_upload_getsavedir(char *upload_savedir, int size, const char *upload_id, const char *upload_basepath);
static void _upload_progressbar(Q_ENTRY *request, const char *upload_id, const char *upload_basepath);
static bool _upload_getstatus(const char *upload_id, const char *upload_basepath, int *upload_tsize, int *upload_csize, char *upload_cname, int upload_cname_size);
static bool _upload_clear_savedir(char *dirpath);
static bool _upload_clear_base(const char *upload_basepath, int upload_clearold);

/**
 * Set request parsing option for file uploading in case of multipart/form-data encoding.
 *
 * @param request	if request is a NULL pointer, the function allocates, initializes, and returns a new object. otherwise use it.
 * @param filemode	false(default) for parsing in memory, true for storing attached files to file
 * @param basepath	the base path where the uploaded files are located.
 *			can be NULL if filemode is false.
 * @param clearold	automatically remove temporary uploading file older
 *			than this secconds. to disable, set to 0.
 *
 * @return	an initialized Q_ENTRY* handle, NULL if basepath can not be found.
 *
 * @note
 * This method should be called before qCgiRequestParse().
 *
 * @code
 *   Q_ENTRY *req = qCgiRequestParseOption(true, "/tmp", 86400);
 *   req = qCgiRequestParse(req);
 * @endcode
 */
Q_ENTRY *qCgiRequestParseOption(bool filemode, const char *basepath, int clearold) {
	/* init entry structure */
	Q_ENTRY *request = qEntryInit();
	if(request == NULL) return NULL;

	if(filemode == true) {
		if(basepath == NULL || qFileExist(basepath) == false) {
			qEntryFree(request);
			return NULL;
		}
		qEntryPutStr(request, "_Q_UPLOAD_BASEPATH", basepath, true);
		qEntryPutInt(request, "_Q_UPLOAD_CLEAROLD", clearold, true);
	}

	return request;
}

/**
 * Parse web request(COOKIE/GET/POST) query into name and value fairs then store into the Q_ENTRY structure.
 *
 * @param request	if request is a NULL pointer, the function allocates, initializes, and returns a new object.
 *			but if you called qCgiParseRequestOption() to set options, pass it's pointer.
 *
 * @return		Q_ENTRY* handle if successful, NULL if there was insufficient memory to allocate a new object.
 *
 * @code
 *   Q_ENTRY *req = qCgiRequestParse(NULL);
 *   printf("%s\n", qEntryGetStr(req, "name"));
 * @endcode
 *
 * @note
 * The parsing sequence is (1)COOKIE, (2)GET (3)POST. Thus if same query names
 * (which are sent by different method) exist, qGetValue() will return the value
 * sent by GET method.
 */
Q_ENTRY *qCgiRequestParse(Q_ENTRY *request) {
	/* init entry structure */
	if(request == NULL) {
		request = qEntryInit();
		if(request == NULL) return NULL;
	}

	qCgiRequestParseCookies(request);
	qCgiRequestParseQueries(request, NULL);

	return request;
}

/**
 * Parse GET/POST queries into name and value fairs then store into the Q_ENTRY structure.
 *
 * @param request	if request is a NULL pointer, the function allocates, initializes, and returns a new object.
 *			otherwise reuse(append at) Q_ENTRY* handle.
 * @param method	"GET" or "POST". NULL can be used for parse both GET/POST queries.
 *
 * @return		Q_ENTRY* handle if successful, NULL if there was insufficient memory to allocate a new object.
 *
 * @code
 *   Q_ENTRY *getpost = qCgiRequestParseQueries(NULL, NULL);
 *   printf("%s\n", qEntryGetStr(getpost, "name"));
 *
 *   // if you would like to parse only POST queries.
 *   Q_ENTRY *post = qCgiRequestParseQueries(NULL, "POST");
 *   printf("%s\n", qEntryGetStr(post, "name"));
 * @endcode
 */
Q_ENTRY *qCgiRequestParseQueries(Q_ENTRY *request, const char *method) {
	/* init entry structure */
	if(request == NULL) {
		request = qEntryInit();
		if(request == NULL) return NULL;
	}

	if(method == NULL || !strcmp(method, "GET")) { /* parse GET method */
		char *query = qCgiRequestGetQueryString("GET");
		if(query != NULL) {
			qDecodeQueryString(request, query, '=', '&', NULL);
			free(query);
		}

		/* check uploading progress dialog */
		const char *upload_id = qEntryGetStr(request, "Q_UPLOAD_ID");
		if (upload_id != NULL) {
			const char *upload_basepath = qEntryGetStr(request, "_Q_UPLOAD_BASEPATH");
			if (upload_basepath != NULL) {
				_upload_progressbar(request, upload_id, upload_basepath);
				exit(EXIT_SUCCESS);
			} else {
				DEBUG("Force to switch to memory operation mode.");
			}
		}
	}

	if(method == NULL || !strcmp(method, "POST")) { /* parse POST method */
		const char *content_type = qSysGetEnv("CONTENT_TYPE", "");
		if (!strncmp(content_type, "application/x-www-form-urlencoded", CONST_STRLEN("application/x-www-form-urlencoded"))) {
			char *query = qCgiRequestGetQueryString("POST");
			if(query != NULL) {
				qDecodeQueryString(request, query, '=', '&', NULL);
				free(query);
			}
		} else if (!strncmp(content_type, "multipart/form-data", CONST_STRLEN("multipart/form-data"))) {
			_parse_multipart(request);
		}
	}

	return request;
}

/**
 * Parse cookies into name and value fairs then store into the Q_ENTRY structure.
 *
 * @param request	if request is a NULL pointer, the function allocates, initializes, and returns a new object.
 *			otherwise cookie value will be appended at Q_ENTRY* handle.
 *
 * @return		Q_ENTRY* handle if successful, NULL if there was insufficient memory to allocate a new object.
 *
 * @code
 *   Q_ENTRY *cookies = qCgiRequestParseCookies(NULL);
 *   printf("%s\n", qEntryGetStr(cookies, "name"));
 * @endcode
 */
Q_ENTRY *qCgiRequestParseCookies(Q_ENTRY *request) {
	/* init entry structure */
	if(request == NULL) {
		request = qEntryInit();
		if(request == NULL) return NULL;
	}

	/* parse COOKIE */
	char *query = qCgiRequestGetQueryString("COOKIE");
	if(query != NULL) {
		qDecodeQueryString(request, query, '=', ';', NULL);
		free(query);
	}

	return request;
}

/**
 * Get unparsed query string.
 *
 * @param query_type	GET, POST, COOKIE (case-sensitive)
 *
 * @return		query string which is memory allocated if successful, otherwise returns NULL;
 *
 * @code
 *   char *query = qCgiRequestGetQueryString("GET");
 *   if(query != NULL) {
 *     printf("%s\n", query);
 *     free(query);
 *   }
 * @endcode
 */
char *qCgiRequestGetQueryString(const char *query_type) {
	if (!strcmp(query_type, "GET")) {
		char *request_method = getenv("REQUEST_METHOD");
		if (request_method != NULL && strcmp(request_method, "GET")) return NULL;

		char *query_string = getenv("QUERY_STRING");
		char *req_uri = getenv("REQUEST_URI");
		if (query_string == NULL) return NULL;

		char *query = NULL;
		/* SSI query handling */
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
	} else if (!strcmp(query_type, "POST")) {
		char *request_method = getenv("REQUEST_METHOD");
		char *content_length = getenv("CONTENT_LENGTH");
		if (request_method == NULL || strcmp(request_method, "POST") || content_length == NULL) return NULL;

		int i, cl = atoi(content_length);
		char *query = (char *)malloc(sizeof(char) * (cl + 1));
		for (i = 0; i < cl; i++)query[i] = fgetc(stdin);
		query[i] = '\0';
		return query;
	} else if (!strcmp(query_type, "COOKIE")) {
		char *http_cookie = getenv("HTTP_COOKIE");
		if (http_cookie == NULL) return NULL;
		char *query = strdup(http_cookie);
		return query;
	}

	return NULL;
}

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
	qStrCpy(boundary_orig, sizeof(boundary_orig), strstr(getenv("CONTENT_TYPE"), "boundary=") + CONST_STRLEN("boundary="), sizeof(boundary_orig));
	qStrTrim(boundary_orig);
	qStrUnchar(boundary_orig, '"', '"');
	snprintf(boundary, sizeof(boundary), "--%s", boundary_orig);

	/* If you want to observe the string from stdin, enable this section. */
	/* This section is made for debugging.                                */
	if (false) {
		int i, j;
		qCgiResponseSetContentType(request, "text/html");

		printf("Content Length = %s<br>\n", getenv("CONTENT_LENGTH"));
		printf("Boundary len %d : %s<br>\n", strlen(boundary), boundary);
		for (i = 0; boundary[i] != '\0'; i++) printf("%02X ", boundary[i]);
		printf("<p>\n");

		for (j = 1; _q_fgets(buf, sizeof(buf), stdin) != NULL; j++) {
			printf("Line %d, len %d : %s<br>\n", j, strlen(buf), buf);
			//for (i = 0; buf[i] != '\0'; i++) printf("%02X ", buf[i]);
			printf("<br>\n");
		}
		exit(EXIT_SUCCESS);
	}

	/* check boundary */
	if (_q_fgets(buf, sizeof(buf), stdin) == NULL) {
		DEBUG("Bbrowser sent a non-HTTP compliant message.");
		return amount;
	}

	/* for explore 4.0 of NT, it sent \r\n before starting, fucking Micro$oft */
	if (!strcmp(buf, "\r\n")) _q_fgets(buf, sizeof(buf), stdin);

	if (strncmp(buf, boundary, strlen(boundary)) != 0) {
		DEBUG("Invalid string format.");
		return amount;
	}

	bool upload_filesave = false; /* false: save into memory, true: save into file */
	const char *upload_basepath = qEntryGetStr(request, "_Q_UPLOAD_BASEPATH");
	char upload_savedir[MAX_PATHLEN];
	bool  finish;
	for (finish = false; finish == false; amount++) {
		/* check file save mode */
		while(upload_filesave == false && upload_basepath != NULL && amount == 1) {
			char *content_length = getenv("CONTENT_LENGTH");
			if(content_length == NULL) {
				break;
			}

			char upload_id_new[32+1];
			char *upload_id = (char *)qEntryGetStr(request, "Q_UPLOAD_ID");
			if (upload_id == NULL || strlen(upload_id) == 0) {
				/* not progress, just file mode */
				char *uniq = qStrUnique(getenv("REMOTE_ADDR"));
				qStrCpy(upload_id_new, sizeof(upload_id_new), uniq, sizeof(upload_id_new));
				free(uniq);
				upload_id = upload_id_new;
			}

			/* generate temporary uploading directory path */
			if (_upload_getsavedir(upload_savedir, sizeof(upload_savedir), upload_id, upload_basepath) == NULL) {
				DEBUG("Invalid base path %s", upload_basepath);
				break;
			}

			/* first, we clear old temporary files */
			int upload_clearold = qEntryGetInt(request, "_Q_UPLOAD_CLEAROLD");
			if (_upload_clear_base(upload_basepath, upload_clearold) == false) {
				DEBUG("Can't remove old temporary files at %s", upload_basepath);
			}

			/* if exists, remove whole directory */
			if (qFileExist(upload_savedir) == true && _upload_clear_savedir(upload_savedir) == false) {
				DEBUG("Can not remove temporary uploading directory %s", upload_savedir);
				break;
			}

			/* make temporary uploading directory */
			if (mkdir(upload_savedir, DEF_DIR_MODE) != 0) {
				DEBUG("Can not make temporary uploading directory %s", upload_savedir);
				break;
			}

			/* save total contents length */
			char upload_tmppath[MAX_PATHLEN];
			snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_TSIZE", upload_savedir);
			if (qCountSave(upload_tmppath, atoi(content_length)) == false) {
				DEBUG("Can not save uploading information at %s", upload_tmppath);
				break;
			}

			/* save start time */
			snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_START", upload_savedir);
			if (qCountSave(upload_tmppath, time(NULL)) == false) {
				DEBUG("Can't save uploading information at %s", upload_tmppath);
				break;
			}

			/* turn on the flag - save into file directly */
			upload_filesave = true;
			break;
		}

		char *name = NULL, *value = NULL, *filename = NULL, *contenttype = NULL;
		int valuelen = 0;

		/* get information */
		while (_q_fgets(buf, sizeof(buf), stdin)) {
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
					qStrTrim(filename);

					if(strlen(filename) == 0) { /* empty attachment */
						free(filename);
						filename = NULL;
					}
				}
			} else if (!strncasecmp(buf, "Content-Type: ", CONST_STRLEN("Content-Type: "))) {
				contenttype = strdup(buf + CONST_STRLEN("Content-Type: "));
				qStrTrim(contenttype);
			}
		}

		/* check */
		if(name == NULL) {
			DEBUG("bug or invalid format.");
			continue;
		}

		/* get value field */
		if (filename != NULL && upload_filesave == true) {
			char *savename = qStrReplace("tn", filename, " ", "_");
			value = _parse_multipart_value_into_disk(boundary, upload_savedir, savename, &valuelen, &finish);
			free(savename);

			if(value != NULL) qEntryPutStr(request, name, value, false);
			else qEntryPutStr(request, name, "(parsing failure)", false);
		} else {
			value = _parse_multipart_value_into_memory(boundary, &valuelen, &finish);

			if(value != NULL) qEntryPut(request, name, value, valuelen+1, false);
			else qEntryPutStr(request, name, "(parsing failure)", false);
		}

		/* store some additional info */
		if (value != NULL && filename != NULL) {
			char ename[255+10+1];

			/* store data length, 'NAME.length'*/
			snprintf(ename, sizeof(ename), "%s.length", name);
			qEntryPutInt(request, ename, valuelen, false);

			/* store filename, 'NAME.filename'*/
			snprintf(ename, sizeof(ename), "%s.filename", name);
			qEntryPutStr(request, ename, filename, false);

			/* store contenttype, 'NAME.contenttype'*/
			snprintf(ename, sizeof(ename), "%s.contenttype", name);
			qEntryPutStr(request, ename, ((contenttype!=NULL)?contenttype:""), false);

			if (upload_filesave == true) {
				snprintf(ename, sizeof(ename), "%s.savepath", name);
				qEntryPutStr(request, ename, value, false);
			}
		}

		/* free stuffs */
		if(name != NULL) free(name);
		if(value != NULL) free(value);
		if(filename != NULL) free(filename);
		if(contenttype != NULL) free(contenttype);
	}

	if (upload_filesave == true) { /* save end time */
		char upload_tmppath[MAX_PATHLEN];
		snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_END", upload_savedir);
		if (qCountSave(upload_tmppath, time(NULL)) == false) {
			DEBUG("Can't save uploading information at %s", upload_tmppath);
		}
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

	/* save current upload file */
	char upload_file[MAX_PATHLEN];
	snprintf(upload_file, sizeof(upload_file), "%s/Q_UPLOAD_FILE", savedir);
	qFileSave(upload_file, filename, strlen(filename), false);

	/* open temp file */
	char upload_path[MAX_PATHLEN];
	snprintf(upload_path, sizeof(upload_path), "%s/Q_FILE_XXXXXX", savedir);

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

static char *_upload_getsavedir(char *upload_savedir, int size, const char *upload_id, const char *upload_basepath) {
	if(upload_savedir == NULL || upload_id == NULL || upload_basepath == NULL || strlen(upload_basepath) == 0) return NULL;

        char md5seed[1024];
        snprintf(md5seed, sizeof(md5seed), "%s|%s|%s", QDECODER_PRIVATEKEY, qSysGetEnv("REMOTE_ADDR", ""), upload_id);
        char *md5str = qHashMd5Str(md5seed, strlen(md5seed));
        snprintf(upload_savedir, size, "%s/Q_%s", upload_basepath, md5str);
        free(md5str);

        return upload_savedir;
}

static void _upload_progressbar(Q_ENTRY *request, const char *upload_id, const char *upload_basepath) {
	int  drawrate = qEntryGetInt(request, "Q_UPLOAD_DRAWRATE");
	const char *template = qEntryGetStr(request, "Q_UPLOAD_TEMPLATE");

	int last_csize = 0, freezetime = 0;
	int upload_tsize = 0, upload_csize = 0;
	char upload_cname[256];

	/* adjust drawrate */
	if(drawrate == 0) drawrate = 1000;
	else if(drawrate < 100) drawrate = 100;
	else if(drawrate > 3000) drawrate = 3000;

	/* check arguments */
	if (!strcmp(upload_id, "")) {
		DEBUG("Q_UPLOAD_ID is invalid.");
		return;
	}
	if (template == NULL) {
		DEBUG("Q_UPLOAD_TEMPLATE query not found.");
		return;
	}

	/* print out qDecoder logo */
	qCgiResponseSetContentType(request, "text/html");

	/* print template */
	if(qSedFile(NULL, template, stdout) == 0) {
		DEBUG("Can't open %s", template);
		return;
	}
	if(fflush(stdout) != 0) return;

	/* draw progress bar */
	int failcnt = 0;
	while(failcnt < 5) {
		upload_tsize = upload_csize = 0;

		if(_upload_getstatus(upload_id, upload_basepath, &upload_tsize, &upload_csize, upload_cname, sizeof(upload_cname)) == false) {
			failcnt++;
			sleep(1);
			continue;
		}

		if(upload_tsize == 0 && upload_csize > 0) break; /* tsize file is removed. upload ended */

		if (last_csize < upload_csize) {
			qStrReplace("tr", upload_cname, "'", "`");

			printf("<script language='JavaScript'>");
			printf("if(qSetProgress)qSetProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
			printf("</script>\n");

			last_csize = upload_csize;
			freezetime = 0;
		} else if (last_csize > upload_csize) {
			break; /* upload ended */
		} else {
			if (freezetime > 10000) {
				break; /* maybe upload connection is closed */
			}

			if (upload_csize > 0) {
				printf("<script language='JavaScript'>");
				printf("if(qSetProgress)qSetProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
				printf("</script>\n");
			}

			freezetime += drawrate;
		}

		fflush(stdout);
		usleep(drawrate * 1000);
	}

	printf("<script language='JavaScript'>");
	printf("window.close();");
	printf("</script>\n");

	fflush(stdout);
}

static bool _upload_getstatus(const char *upload_id, const char *upload_basepath, int *upload_tsize, int *upload_csize, char *upload_cname, int upload_cname_size) {
#ifdef _WIN32
	return false;
#else
	DIR     *dp;
	struct  dirent *dirp;
	char upload_savedir[MAX_PATHLEN], tmppath[MAX_PATHLEN];

	/* get basepath */
	if (_upload_getsavedir(upload_savedir, sizeof(upload_savedir), upload_id, upload_basepath) == NULL) {
		DEBUG("Q_UPLOAD_ID is not set.");
		return false;
	}

	/* open upload folder */
	dp = opendir(upload_savedir);
	if (dp == NULL) {
		DEBUG("Can't open %s", upload_savedir);
		return false;
	}

	/* read tsize */
	snprintf(tmppath, sizeof(tmppath), "%s/Q_UPLOAD_TSIZE", upload_savedir);
	*upload_tsize = qCountRead(tmppath);

	/* read currnet upload filename */
	snprintf(tmppath, sizeof(tmppath), "%s/Q_UPLOAD_FILE", upload_savedir);
	char *upload_file = qFileLoad(tmppath, NULL);
	if(upload_file != NULL) {
		qStrCpy(upload_cname, upload_cname_size, upload_file, upload_cname_size);
		free(upload_file);
	} else {
		qStrCpy(upload_cname, upload_cname_size, "-", upload_cname_size);
	}

	/* get csize */
	*upload_csize = 0;
	while ((dirp = readdir(dp)) != NULL) {
		if (strncmp(dirp->d_name, "Q_FILE_", CONST_STRLEN("Q_FILE_"))) continue;

		snprintf(tmppath, sizeof(tmppath), "%s/%s", upload_savedir, dirp->d_name);
		*upload_csize += qFileGetSize(tmppath);
	}
	closedir(dp);

	return true;
#endif
}

static bool _upload_clear_savedir(char *dirpath) {
#ifdef _WIN32
	return false;
#else
	/* open upload folder */
	DIR     *dp;
	if ((dp = opendir(dirpath)) == NULL) return false;

	struct  dirent *dirp;
	while ((dirp = readdir(dp)) != NULL) {
		if (strncmp(dirp->d_name, "Q_", CONST_STRLEN("Q_"))) continue;

		char filepath[MAX_PATHLEN];
		snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, dirp->d_name);
		unlink(filepath);
	}
	closedir(dp);

	if (rmdir(dirpath) != 0) return false;
	return true;
#endif
}

static bool _upload_clear_base(const char *upload_basepath, int upload_clearold) {
#ifdef _WIN32
	return false;
#else
	if (upload_clearold <= 0) return false;

	bool haserror = false;

	/* open upload folder */
	DIR     *dp;
	if ((dp = opendir(upload_basepath)) == NULL) return false;

	time_t  now = time(NULL);
	struct  dirent *dirp;
	while ((dirp = readdir(dp)) != NULL) {
		time_t starttime;

		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..") || strncmp(dirp->d_name, "Q_", 2) != 0) continue;

		char filepath[MAX_PATHLEN];
		snprintf(filepath, sizeof(filepath), "%s/%s/Q_UPLOAD_START", upload_basepath, dirp->d_name);
		starttime = qCountRead(filepath);
		if (starttime > 0 && now - starttime < upload_clearold) continue;

		snprintf(filepath, sizeof(filepath), "%s/%s", upload_basepath, dirp->d_name);
		if (_upload_clear_savedir(filepath) == false) {
			haserror = true;
			break;
		}
	}
	closedir(dp);

	if(haserror == true) return false;
	return true;
#endif
}

#endif /* DISABLE_CGI */
