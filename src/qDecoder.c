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
 * @file qDecoder.c Web Query & Cookie Handling API
 *
 * qDecoder supports parsing
 *   @li COOKIE
 *   @li GET method
 *   @li POST method (application/x-www-form-urlencoded: default FORM encoding)
 *   @li POST method (multipart/form-data: especially used for file uploading)
 *
 * Anyway you don't care about this. You don't need to know which method
 * (COOKIE/GET/POST) is used for sending data. All you need to know is you
 * can get the value by calling qGetValue("value name"). (See examples/fetch.c)
 *
 * @code
 *   [HTML sample]
 *   <form action="your_program.cgi">
 *     <input type="text" name="query">
 *     <input type="submit">
 *   </form>
 *
 *   [Your Source]
 *   char *query;
 *   query = qGetValue("query");
 *   qPrint(); // if you want to see debugging information
 * @endcode
 *
 * The parsing sequence is (1)COOKIE (2)GET (3)POST. Thus if same query names
 * (which are sent by different method) exist, qGetValue() will return the value
 * of COOKIE.
 *
 * In case of multipart/form-data encoding(used for file uploading), qDecoder
 * supports 2 modes for handling file uploading. I just made a name for that.
 *
 * @li <b>default mode</b> : Uploading file will be processed only in memory.
 * (see examples/upload.c)
 * @li <b>file mode</b>	: Uploading file will be stored into unique temporary
 * file on the fly. (see examples/uploadfile.c)
 * There is sub mode named <b>progress mode</b>. This is same as file mode.
 * But user can see the progress bar while uploading.
 * When file mode is turned on and if you add some specific HTML codes
 * in your uploading page, progressive window will be shown.
 * (see examples/uploadprogress.c)
 *
 * Basically, when file is uploaded qDecoder store it's meta information like
 * below.
 * @li (VARIABLE_NAME)		- In the <b>default mode</b>, this is binary
 * data. In the <b>file mode</b> this value is same as
 * "(VARIABLE_NAME).savepath".
 * @li (VARIABLE_NAME).filename	- File name itself, path information will be
 * removed.
 * @li (VARIABLE_NAME).length	- File size, the number of bytes.
 * @li (VARIABLE_NAME).contenttype - Mime type like 'text/plain',
 * 'application/vnd.ms-excel'.
 * @li (VARIABLE_NAME).savepath	- Only appended only in <b>file mode</b>.
 * The file path where uploaded file is saved.
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
 * The return value of qDecoder() only counts variables. So this will be
 * regarded as 1.
 *
 * If you want to activate progress mode. Please follow below steps
 * @li 1) copy "examples/qDecoder-upload/" folder under your Web Document Root.
 * @li 2) see this sample codes.
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
 *     // change mode to "file mode".
 *     qDecoderInit(true, "/tmp", 86400); // must be called at
 *                                        // the first line of main();
 *     qDecoder();                        // must be called right next
 *                                        // of qDecoderInit().
 *
 *     (...your codes here...)
 *   }
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
/* to use setmode() function for converting WIN32's stream mode to _O_BINARY */
#include <io.h>
#else
#include <dirent.h>	/* to use opendir() */
#endif
#include "qDecoder.h"
#include "qInternal.h"

/*
 * static function prototypes
 */
static int  _parse_urlencoded(void);
static char *_get_query(char *method);
static int  _parse_query(char *query, char sepchar);
static int  _parse_multipart_data(void);
static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, int *finish);
static char *_parse_multipart_value_into_disk(char *boundary, char *savedir, char *filename, int *filelen, int *finish);

static char *_upload_getsavedir(char *upload_id, char *upload_savedir);
static void _upload_progressbar(char *upload_id);
static bool _upload_getstatus(char *upload_id, int *upload_tsize, int *upload_csize, char *upload_cname);
static bool _upload_clear_savedir(char *dir);
static int _upload_clear_base();

/*
 * static variables used for internal
 */
static Q_ENTRY *_first_entry = NULL;
static Q_ENTRY *_multi_last_entry = NULL;
static char _multi_last_key[1024];

static bool _upload_base_init = false;
static char _upload_base[1024];
static int _upload_clear_olderthan = 0;	/* seconds */

static int _cookie_cnt = 0, _get_cnt = 0, _post_cnt = 0, _new_cnt = 0; /* counts per methods */

/**
 * Initialize how qDecoder() handle file uploading(multipart/form-data).
 *
 * @param filemode	to turn on file mode set to true.
 *			false means default mode.
 * @param upload_base	the base path where the temporary created files are
 *			located. if filemode is false, just set to NULL.
 * @param clear_olderthan automatically remove temporary uploading file older
 *			than this secconds. to disable, set to 0.
 *
 * @return	in case of success, returns true.
 *		otherwise(upload_base not found) false.
 *
 * @note
 * You do not need to call for setting up memory mode, because it's default
 * operation. But if you want to turn file mode on, you must call this at the
 * first line of main().
 *
 * @code
 *   int main(...) {
 *     // change mode to "file mode".
 *     qDecoderInit(true, "/tmp", 86400); // must be called at
 *                                        // the first line of main();
 *     qDecoder();                        // must be called right next
 *                                        // of qDecoderInit().
 *     (...your codes here...)
 *   }
 * @endcode
 */
bool qDecoderInit(bool filemode, char *upload_base, int clear_olderthan) {
	if(filemode == false) {
		_upload_base_init = false;
	} else {
		if (qCheckFile(upload_base) == false) return false;
		qStrncpy(_upload_base, upload_base, sizeof(_upload_base));
		_upload_clear_olderthan = clear_olderthan;
		_upload_base_init = true;
	}

return true;
}

/**
 * Interprets encoded web query & cookie strings and stores them in the
 * internal linked-list.
 *
 * The parsing sequence is (1)COOKIE (2)GET (3)POST. Thus if same query names
 * (which are sent by different method) exist, qGetValue() will return the value
 * of COOKIE.
 *
 * This function, qDecoder(), is called automatically when you use qGetValue() or
 * related functions, so you do not need to call directly.
 *
 * @return	the number of received arguments(not stored).
 *		otherwise returns -1.
 */
int qDecoder(void) {
	int  amount = -1;
	char *content_type;

	if (_first_entry != NULL) return -1;

	content_type = getenv("CONTENT_TYPE");

	/* for GET method */
	if (content_type == NULL) {
		amount = _parse_urlencoded();

		/* if connection is upload progress dialog */
		if (_first_entry != NULL) {
			char *q_upload_id;

			q_upload_id = qGetValue("Q_UPLOAD_ID");

			if (q_upload_id != NULL) {
				if (_upload_base_init == false) qError("qDecoder(): qDecoderInit(true, ...) must be called before.");
				_upload_progressbar(q_upload_id);
				exit(0);
			}
		}
	}
	/* for POST method : application/x-www-form-urlencoded */
	else if (!strncmp(content_type, "application/x-www-form-urlencoded", strlen("application/x-www-form-urlencoded"))) {
		amount = _parse_urlencoded();
	}
	/* for POST method : multipart/form-data */
	else if (!strncmp(content_type, "multipart/form-data", strlen("multipart/form-data"))) {
		amount = _parse_multipart_data();
	}
	/* for stupid browser : Oracle Web Server */
	else {
		amount = _parse_urlencoded();
	}

	return amount;
}

/* For decode application/x-www-form-urlencoded, used by qDecoder() */
static int _parse_urlencoded(void) {
	char *query;
	int  amount;

	/* parse COOKIE */
	query = _get_query("COOKIE");
	_cookie_cnt = _parse_query(query, ';');
	amount = _cookie_cnt;
	if (query)free(query);

	/* parse GET method */
	query = _get_query("GET");
	_get_cnt = _parse_query(query, '&');
	amount += _get_cnt;
	if (query)free(query);

	/* parse POST method */
	query = _get_query("POST");
	_post_cnt = _parse_query(query, '&');
	amount += _post_cnt;
	if (query)free(query);

	return amount;
}

/* For fetch query used by _parse_urlencoded() */
static char *_get_query(char *method) {
	char *query;
	int cl, i;

	if (!strcmp(method, "GET")) {
		if (getenv("QUERY_STRING") == NULL) return NULL;
		query = strdup(getenv("QUERY_STRING"));
		/* SSI query handling */
		if (!strcmp(query, "") && getenv("REQUEST_URI") != NULL) {
			char *cp;
			for (cp = getenv("REQUEST_URI"); *cp != '\0'; cp++) {
				if (*cp == '?') {
					cp++;
					break;
				}
			}
			free(query);
			query = strdup(cp);
		}

		return query;
	} else if (!strcmp(method, "POST")) {
		if (getenv("REQUEST_METHOD") == NULL) return NULL;
		if (strcmp("POST", getenv("REQUEST_METHOD")))return NULL;
		if (getenv("CONTENT_LENGTH") == NULL) qError("_get_query(): Your browser sent a non-HTTP compliant message.");

		cl = atoi(getenv("CONTENT_LENGTH"));
		query = (char *)malloc(sizeof(char) * (cl + 1));
		for (i = 0; i < cl; i++)query[i] = fgetc(stdin);
		query[i] = '\0';
		return query;
	} else if (!strcmp(method, "COOKIE")) {
		if (getenv("HTTP_COOKIE") == NULL) return NULL;
		query = strdup(getenv("HTTP_COOKIE"));
		return query;
	}

	return NULL;
}

static int _parse_query(char *query, char sepchar) {
	int cnt;

	for (cnt = 0; query && *query; cnt++) {
		Q_ENTRY *entry;
		char *name, *value;

		value = _makeword(query, sepchar);
		name = qRemoveSpace(_makeword(value, '='));
		qUrlDecode(name);
		qUrlDecode(value);

		entry = qEntryAdd(_first_entry, name, value, 2);
		if (_first_entry == NULL) _first_entry = entry;
	}

	return cnt;
}

/* For decode multipart/form-data, used by qDecoder() */
static int _parse_multipart_data(void) {
	Q_ENTRY *entry;
	char *query, buf[1024];
	int  amount;

	char boundary[256];
	int  maxboundarylen; /* for check overflow attack */

	int  finish;

	/* for progress upload */
	int  upload_type = 0; /* 0: save into memory, 1: save into file */
	char upload_savedir[1024];
	char upload_tmppath[1024];

#ifdef _WIN32
	setmode(fileno(stdin), _O_BINARY);
	setmode(fileno(stdout), _O_BINARY);
#endif

	/*
	 * For parse COOKIE and GET method
	 */

	/* parse COOKIE */
	query = _get_query("COOKIE");
	_cookie_cnt = _parse_query(query, ';');
	amount = _cookie_cnt;
	if (query)free(query);

	/* parse GET method */
	query = _get_query("GET");
	_get_cnt = _parse_query(query, '&');
	amount += _get_cnt;
	if (query)free(query);

	/*
	 * For parse multipart/form-data method
	 */

	/* Force to check the boundary string length to defense overflow attack */
	maxboundarylen =  strlen("--");
	maxboundarylen += strlen(strstr(getenv("CONTENT_TYPE"), "boundary=") + strlen("boundary="));
	maxboundarylen += strlen("--");
	maxboundarylen += strlen("\r\n");
	if (maxboundarylen >= sizeof(boundary)) qError("_parse_multipart_data(): The boundary string is too long(Overflow Attack?). Stopping process.");

	/* find boundary string */
	snprintf(boundary, sizeof(boundary), "--%s", strstr(getenv("CONTENT_TYPE"), "boundary=") + strlen("boundary="));
	/* This is not necessary but, I can not trust MS Explore */
	qRemoveSpace(boundary);

	/* If you want to observe the string from stdin, enable this section. */
	/* This section is made for debugging.                                */
	if (0) {
		int i, j;
		qContentType("text/html");

		printf("Content Length = %s<br>\n", getenv("CONTENT_LENGTH"));
		printf("Boundary len %d : %s<br>\n", (int)strlen(boundary), boundary);
		for (i = 0; boundary[i] != '\0'; i++) printf("%02X ", boundary[i]);
		printf("<p>\n");

		for (j = 1; _fgets(buf, sizeof(buf), stdin) != NULL; j++) {
			printf("Line %d, len %d : %s<br>\n", j, (int)strlen(buf), buf);
			for (i = 0; buf[i] != '\0'; i++) printf("%02X ", buf[i]);
			printf("<p>\n");
		}
		exit(0);
	}

	/* check boundary */
	if (_fgets(buf, sizeof(buf), stdin) == NULL) qError("_parse_multipart_data(): Your browser sent a non-HTTP compliant message.");

	/* for explore 4.0 of NT, it sent \r\n before starting, fucking Micro$oft */
	if (!strcmp(buf, "\r\n")) _fgets(buf, sizeof(buf), stdin);

	if (strncmp(buf, boundary, strlen(boundary)) != 0) qError("_parse_multipart_data(): String format invalid.");

	for (finish = 0, _post_cnt = 0; finish != 1; amount++, _post_cnt++) {
		char *name = "", *value = NULL, *filename = "", *contenttype = "";
		int  valuelen = 0;

		/* check file save mode */
		if (_first_entry != NULL && upload_type == 0) {

			if (_upload_base_init == true) {
				char *upload_id;

				upload_id = qGetValue("Q_UPLOAD_ID");
				if (upload_id == NULL || strlen(upload_id) == 0)  upload_id = qUniqId(); /* not progress, just file mode */

				/* turn on the flag - save into file directly */
				upload_type = 1;

				/* generate temporary uploading directory path */
				if (_upload_getsavedir(upload_id, upload_savedir) == NULL) qError("_parse_multipart_data(): Invalid Q_UPLOAD_ID");

				/* first, we clear old temporary files */
				if (_upload_clear_base() < 0) qError("_parse_multipart_data(): Can not remove old temporary files at %s", _upload_base);

				/* if exists, remove whole directory */
				if (qCheckFile(upload_savedir) == 1) {
					if (_upload_clear_savedir(upload_savedir) == 0) qError("_parse_multipart_data(): Can not remove temporary uploading directory %s", upload_savedir);
				}

				/* make temporary uploading directory */
				if (mkdir(upload_savedir, 0755) == -1) qError("_parse_multipart_data(): Can not make temporary uploading directory %s", upload_savedir);

				/* save total contents length */
				snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_TSIZE", upload_savedir);
				if (qCountSave(upload_tmppath, atoi(getenv("CONTENT_LENGTH"))) == false) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);

				/* save start time */
				snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_START", upload_savedir);
				if (qCountSave(upload_tmppath, time(NULL)) == false) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);
			}
		}

		/* get information */
		while (_fgets(buf, sizeof(buf), stdin)) {
			if (!strcmp(buf, "\r\n")) break;
			else if (!qStrincmp(buf, "Content-Disposition: ", strlen("Content-Disposition: "))) {
				int c_count;

				/* get name field */
				name = strdup(buf + strlen("Content-Disposition: form-data; name=\""));
				for (c_count = 0; (name[c_count] != '\"') && (name[c_count] != '\0'); c_count++);
				name[c_count] = '\0';

				/* get filename field */
				if (strstr(buf, "; filename=\"") != NULL) {
					int erase;
					filename = strdup(strstr(buf, "; filename=\"") + strlen("; filename=\""));
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
					qRemoveSpace(filename);
				}
			} else if (!qStrincmp(buf, "Content-Type: ", strlen("Content-Type: "))) {
				contenttype = strdup(buf + strlen("Content-Type: "));
				qRemoveSpace(contenttype);
			}
		}

		/* get value field */
		if (strcmp(filename, "") && upload_type == 1) {
			char *savename = qStrReplace("tn", filename, " ", "_");
			value = _parse_multipart_value_into_disk(boundary, upload_savedir, savename, &valuelen, &finish);
			free(savename);
		} else {
			value = _parse_multipart_value_into_memory(boundary, &valuelen, &finish);
		}

		entry = qEntryAdd(_first_entry, name, value, 2);
		if (_first_entry == NULL) _first_entry = entry;

		/* store some additional info */
		if (strcmp(filename, "") != 0) {
			char *ename, *evalue;

			/* store data length, 'NAME.length'*/
			ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".length") + 1));
			evalue = (char *)malloc(sizeof(char) * 20 + 1);
			sprintf(ename,  "%s.length", name);
			sprintf(evalue, "%d", valuelen);
			qEntryAdd(_first_entry, ename, evalue, 2);

			/* store filename, 'NAME.filename'*/
			ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".filename") + 1));
			sprintf(ename,  "%s.filename", name);
			evalue = filename;
			qEntryAdd(_first_entry, ename, evalue, 2);

			/* store contenttype, 'NAME.contenttype'*/
			ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".contenttype") + 1));
			sprintf(ename,  "%s.contenttype", name);
			evalue = contenttype;
			qEntryAdd(_first_entry, ename, evalue, 2);

			_post_cnt += 3;

			if (upload_type == 1) {
				ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".savepath") + 1));
				sprintf(ename,  "%s.savepath", name);
				evalue = strdup(value);
				qEntryAdd(_first_entry, ename, evalue, 2);

				_post_cnt += 1;
			}
		}
	}

	if (upload_type == 1) { /* save end time */
		snprintf(upload_tmppath, sizeof(upload_tmppath), "%s/Q_UPLOAD_END", upload_savedir);
		if (qCountSave(upload_tmppath, time(NULL)) == false) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);
	}

	return amount;
}

static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, int *finish) {
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

	for (value = NULL, length = 0, mallocsize = (1024 * 16), c_count = 0; (c = fgetc(stdin)) != EOF; ) {
		if (c_count == 0) {
			value = (char *)malloc(sizeof(char) * mallocsize);
			if (value == NULL) qError("_parse_multipart_data(): Memory allocation fail.");
		} else if (c_count == mallocsize - 1) {
			char *valuetmp;

			mallocsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			valuetmp = (char *)malloc(sizeof(char) * mallocsize);
			if (valuetmp == NULL) qError("_parse_multipart_data(): Memory allocation fail.");
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
					*finish = 1;
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
					*finish = 1;
					break;
				}
			}
		}
	}

	if (c == EOF) qError("_parse_multipart_data(): Broken stream.");

	*valuelen = length;
	return value;
}

static char *_parse_multipart_value_into_disk(char *boundary, char *savedir, char *filename, int *filelen, int *finish) {
	char boundaryEOF[256], rnboundaryEOF[256];
	char boundaryrn[256], rnboundaryrn[256];
	int  boundarylen, boundaryEOFlen;

	/* input */
	char buffer[1024*8], *bp;
	int  bufc;
	int  c;

	/* output */
	static int upload_fcnt = 0; /* file save counter */
	FILE *upload_fp;
	char upload_path[1024];
	int  upload_length;

	/* temp */
	int i;

	/* set boundary strings */
	snprintf(boundaryEOF, sizeof(boundaryEOF), "%s--", boundary);
	snprintf(rnboundaryEOF, sizeof(rnboundaryEOF), "\r\n%s", boundaryEOF);
	snprintf(boundaryrn, sizeof(boundaryrn), "%s\r\n", boundary);
	snprintf(rnboundaryrn, sizeof(rnboundaryrn), "\r\n%s\r\n", boundary);

	boundarylen    = strlen(boundary);
	boundaryEOFlen = strlen(boundaryEOF);

	/* initialize */
	upload_fcnt++;
	snprintf(upload_path, sizeof(upload_path), "%s/%d-%s", savedir, upload_fcnt, filename);

	/* open file */
	upload_fp = fopen(upload_path, "w");
	if (upload_fp == NULL) qError("_parse_multipart_value_into_disk(): Can not open file %s", upload_path);

	/* read stream */
	for (upload_length = 0, bufc = 0, upload_length = 0; (c = fgetc(stdin)) != EOF; ) {
		if (bufc == sizeof(buffer) - 1) {
			int leftsize;

			/* save first 16KB */
			leftsize = boundarylen + 8;
			for (i = 0, bp = buffer; i < bufc - leftsize; i++) fputc(*bp++, upload_fp);
			memcpy(buffer, bp, leftsize);
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
					*finish = 1;
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
						*finish = 1;
						break;
					}
				}
			}
		}
	}

	if (c == EOF) qError("_parse_multipart_data(): Broken stream.");

	/* save lest */
	for (bp = buffer, i = 0; i < bufc; i++) fputc(*bp++, upload_fp);
	fclose(upload_fp);

	*filelen = upload_length;

	return strdup(upload_path);
}

static char *_upload_getsavedir(char *upload_id, char *upload_savedir) {
	char md5seed[1024];

	if (_upload_base_init == false || upload_id == NULL) return NULL;
	if (!strcmp(upload_id, "")) return NULL;

	snprintf(md5seed, sizeof(md5seed), "%s|%s|%s", QDECODER_PRIVATEKEY, qGetenvDefault("", "REMOTE_ADDR"), upload_id);
	snprintf(upload_savedir, sizeof(upload_savedir), "%s/Q_%s", _upload_base, qMd5Str(md5seed));

	return upload_savedir;
}

static void _upload_progressbar(char *upload_id) {
	int  drawrate = qGetInt("Q_UPLOAD_DRAWRATE");
	char *template = qGetValue("Q_UPLOAD_TEMPLATE");

	int last_csize = 0, freezetime = 0;
	int upload_tsize = 0, upload_csize = 0;
	char upload_cname[256];

	/* adjust drawrate */
	if(drawrate == 0) drawrate = 1000;
	else if(drawrate < 100) drawrate = 100;
	else if(drawrate > 3000) drawrate = 3000;

	/* check arguments */
	if (!strcmp(upload_id, "")) {
		printf("_print_progressbar(): Q_UPLOAD_ID is invalid.");
		return;
	}
	if (template == NULL) {
		printf("_print_progressbar(): Q_UPLOAD_TEMPLATE query not found.");
		return;
	}

	/* print out qDecoder logo */
	qContentType("text/html");

	/* print template */
	if(qSedFile(NULL, template, stdout) == 0) {
		printf("_print_progressbar(): Can not open %s", template);
		return;
	}
	if(fflush(stdout) != 0) return;

	/* draw progress bar */
	while(1) {
		upload_tsize = upload_csize = 0;

		_upload_getstatus(upload_id, &upload_tsize, &upload_csize, upload_cname);

		if(upload_tsize == 0 && upload_csize > 0) break; /* tsize file is removed. upload ended */

		if (last_csize < upload_csize) {
			qStrReplace("tr", upload_cname, "'", "`");

			printf("<script language='JavaScript'>");
			printf("if(Q_setProgress)Q_setProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
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
				printf("if(Q_setProgress)Q_setProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
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

static bool _upload_getstatus(char *upload_id, int *upload_tsize, int *upload_csize, char *upload_cname) {
#ifdef _WIN32
	return false;
#else
	DIR     *dp;
	struct  dirent *dirp;

	char upload_savedir[1024], upload_filepath[1024];

	/* initialize */
	*upload_tsize = *upload_csize = 0;
	strcpy(upload_cname, "");

	/* get basepath */
	if (_upload_getsavedir(upload_id, upload_savedir) == NULL) qError("_upload_getstatus(): Q_UPLOAD_ID does not set.");

	/* open upload folder */
	if ((dp = opendir(upload_savedir)) == NULL) return false;

	/* read tsize */
	snprintf(upload_filepath, sizeof(upload_filepath), "%s/Q_UPLOAD_TSIZE", upload_savedir);
	*upload_tsize = qCountRead(upload_filepath);

	while ((dirp = readdir(dp)) != NULL) {
		if (dirp->d_name[0] - '0' <= 0 || dirp->d_name[0] - '0' > 9) continue; /* first char must be a number */

		/* sort last filename */
		if (strcmp(upload_cname, dirp->d_name) < 0) qStrncpy(upload_cname, dirp->d_name, sizeof(upload_cname));

		snprintf(upload_filepath, sizeof(upload_filepath), "%s/%s", upload_savedir, dirp->d_name);
		*upload_csize += qFileSize(upload_filepath);
	}
	closedir(dp);

	if (strstr(upload_cname, "-") != NULL) {
		qStrncpy(upload_cname, strstr(upload_cname, "-") + 1, sizeof(upload_cname));
	}

	return true;
#endif
}

static bool _upload_clear_savedir(char *dir) {
#ifdef _WIN32
	return false;
#else
	DIR     *dp;
	struct  dirent *dirp;
	char    filepath[1024];

	/* open upload folder */
	if ((dp = opendir(dir)) == NULL) return false;

	while ((dirp = readdir(dp)) != NULL) {
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) continue;

		snprintf(filepath, sizeof(filepath), "%s/%s", dir, dirp->d_name);
		unlink(filepath);
	}
	closedir(dp);

	if (rmdir(dir) != 0) return false;
	return true;
#endif
}

static int _upload_clear_base() {
#ifdef _WIN32
	return 0;
#else
	DIR     *dp;
	struct  dirent *dirp;
	char    filepath[1024];
	int     delcnt = 0;
	time_t  now = time(NULL);

	if (_upload_base_init == false) return -1;
	if (_upload_clear_olderthan <= 0) return 0;

	/* open upload folder */
	if ((dp = opendir(_upload_base)) == NULL) return 0;

	while ((dirp = readdir(dp)) != NULL) {
		time_t starttime;

		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..") || strncmp(dirp->d_name, "Q_", 2) != 0) continue;

		snprintf(filepath, sizeof(filepath), "%s/%s/Q_UPLOAD_START", _upload_base, dirp->d_name);
		starttime = qCountRead(filepath);
		if (starttime > 0 && now - starttime < _upload_clear_olderthan) continue;

		snprintf(filepath, sizeof(filepath), "%s/%s", _upload_base, dirp->d_name);
		if (_upload_clear_savedir(filepath) == 0) {
			delcnt = -1;
			break;
		}
		delcnt++;
	}
	closedir(dp);

	return delcnt;
#endif
}

/**
 * Finds out the value.
 *
 * @param format	variable name
 *
 * @return	the pointer of the variable value. NULL if there is no such variable name.
 *
 * @note
 * @code
 *   char *test;
 *   test = qGetValue("name");
 *
 *   char *test;
 *   int i = 1;
 *   test = qGetValue("arg%d", i);
 * @endcode
 */
char *qGetValue(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();
	return qEntryGetValue(_first_entry, name);
}

/**
 * Finds out the value then converts to integer.
 *
 * @param format	variable name
 *
 * @return	converted value.
 *		otherwise(convertion error, not found) returns 0.
 *
 * @note
 * @code
 *   int num;
 *   num = qGetInt("num");
 * @endcode
 */
int qGetInt(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();
	return qEntryGetInt(_first_entry, name);
}

/**
 * Finds out the value, if there is no such variable returns default value.
 *
 * @param defstr	default value
 * @param format	variable name
 *
 * @return	the pointer of the variable value.
 *
 * @note
 * @code
 *   char *test;
 *   test = qGetValueDefault("this is default value", "name");
 * @endcode
 */
char *qGetValueDefault(char *defstr, char *format, ...) {
	char name[1024];
	va_list arglist;
	char *value;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();
	if ((value = qEntryGetValue(_first_entry, name)) == NULL) value = defstr;

	return value;
}

/**
 * Exit program using the qError() function if there is no such variable.
 *
 * @param errmsg	error message
 * @param format	variable name
 *
 * @return	the pointer of the variable value.
 *		if there is no such variable, display errmsg then quit program.
 *
 * @note
 * @code
 *   char *test;
 *   test = qGetValueNotEmpty("DO NOT USE MANUALLY", "name");
 * @endcode
 */
char *qGetValueNotEmpty(char *errmsg, char *format, ...) {
	char name[1024];
	va_list arglist;
	char *value;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();
	if ((value = qEntryGetValue(_first_entry, name)) == NULL) qError("%s", errmsg);
	if (!strcmp(value, "")) qError("%s", errmsg);

	return value;
}

/**
 * Find out the value with replacing tokens.
 *
 * Basically, qGetValueReplace() is the wrapping function of qStrReplace().
 * The difference is that the source string is the query value (the value of
 * the linked-list for names),
 * and the conversion can be directly done to the linked-list itself.
 *
 * Accordingly, the usage of arguments of qGetValueReplace() is basically the same
 * as that of qStrReplace(). The 'mode' argument is a character string made up
 * of two separate characters like "sr".
 *
 * @param mode	same as qStrReplace().
 *   @li "tn" : [t]oken transposition & stores results in a [n]ew space and
 *		returns
 *   @li "tr" : [t]oken transposition & [r]eplaces the linked-list
 *		itself
 *   @li "sn" : [s]tring transposition & stores results in a [n]ew space and
 *		returns
 *   @li "sr" : [s]tring transposition & [r]eplaces the linked-list itself
 * @param name	variable name of the linked-list
 * @param tokstr token or string which you wishes to be replaced.
 * @param word	the string to replace
 *
 * @return	the pointer of the replaced variable value. if you set the mode
 *		to "tn" or "sn", you must free yourself.
 *
 * @note
 * @code
 *   char *test;
 *
 *   // replace space, quotation, double quotation to '_'
 *   test = qGetValueReplace("tr", "name", " '\"", "_");
 *
 *   // replace "bad" to "good"
 *   test = qGetValueReplace("sr", "name", "bad", "good");
 * @endcode
 *
 * @see qStrReplace()
 */
char *qGetValueReplace(char *mode, char *name, char *tokstr, char *word) {
	Q_ENTRY *entries;
	char *retstr, *repstr, method, memuse, newmode[2+1];

	/* initialize pointers to avoid compile warnings */
	retstr = repstr = NULL;

	if (_first_entry == NULL) qDecoder();

	if (strlen(mode) != 2) qError("qGetValueReplace(): Unknown mode \"%s\".", mode);
	method = mode[0], memuse = mode[1];
	newmode[0] = method, newmode[1] = 'n', newmode[2] = '\0';

	if (method != 't' && method != 's') qError("qGetValueReplace(): Unknown mode \"%s\".", mode);
	if (memuse == 'n') { /* new */
		if ((repstr = qEntryGetValue(_first_entry, name)) != NULL) {
			retstr = qStrReplace(newmode, repstr, tokstr, word);
		} else retstr = NULL;
	} else if (memuse == 'r') { /* replace */
		/* To support multiful queries, it searches whole list and convert all of
		   matched ones due to the possibility of duplicated query name.
		   So when you need to do this replacement for duplicated query name,
		   you can call this once before qGetValueFirst(). */
		for (retstr = NULL, entries = _first_entry; entries; entries = entries->next) {
			if (!strcmp(name, entries->name)) {
				repstr = qStrReplace(newmode, entries->value, tokstr, word);
				free(entries->value);
				entries->value = repstr;
				if (retstr == NULL) retstr = repstr; /* To catch first matched one */
			}
		}
	} else qError("qGetValueReplace(): Unknown mode \"%s\".", mode);

	/* Return the value of first matched one */
	return retstr;
}

/**
 * Find out the first value.
 *
 * If the query has multi-same-variables(same variable names), you can use this.
 *
 * @param format	variable name
 *
 * @return	the pointer of the first variable value.
 *		otherwise(not found) returns NULL.
 *
 * @note
 * @code
 *   char *list;
 *   for(list = qGetValueFirst("checklist"); list; list = qGetValueNext()) {
 *     printf("checklist = %s<br>\n", list);
 *   }
 * @endcode
 */
char *qGetValueFirst(char *format, ...) {
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(_multi_last_key, sizeof(_multi_last_key)-1, format, arglist);
	_multi_last_key[sizeof(_multi_last_key)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();
	_multi_last_entry = _first_entry;

	return qGetValueNext();
}

/**
 * Find out the next value.
 *
 * @return	the pointer of the next variable value.
 *		otherwise(no more value) returns NULL.
 */
char *qGetValueNext(void) {
	Q_ENTRY *entries;

	for (entries = _multi_last_entry; entries; entries = entries->next) {
		if (!strcmp(_multi_last_key, entries->name)) {
			_multi_last_entry = entries->next;
			return (entries->value);
		}
	}
	_multi_last_entry = NULL;
	strcpy(_multi_last_key, "");

	return NULL;
}

/**
 * Add given variable to internal linked list.
 *
 * If same variable name exists, it'll be replaced.
 *
 * @param name		variable name to add
 * @param format	value string
 *
 * @return	the pointer of the value string.
 *		otherwise(can't add) returns NULL.
 *
 * @note
 * @code
 *   qAdd("NAME", "Seung-young Kim");
 * @endcode
 */
char *qAdd(char *name, char *format, ...) {
	Q_ENTRY *new_entry;
	char value[1024];
	va_list arglist;

	if (!strcmp(name, "")) return NULL;

	va_start(arglist, format);
	vsnprintf(value, sizeof(value)-1, format, arglist);
	value[sizeof(value)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();

	if (qGetValue(name) == NULL) _new_cnt++; /* if it's new entry, count up. */
	new_entry = qEntryAdd(_first_entry, name, value, 1);
	if (!_first_entry) _first_entry = new_entry;

	return qGetValue(name);
}

/**
 * Remove variable from internal linked list.
 *
 * Multi-same-variables will be removed too.
 *
 * @param format	variable name to remove
 *
 * @note
 * @code
 *   qRemove("NAME");
 * @endcode
 */
void qRemove(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (!strcmp(name, "")) qError("qRemove(): can not remove empty name.");

	switch (qGetType(name)) {
		case 'C' : {
			_cookie_cnt--;
			break;
		}
		case 'G' : {
			_get_cnt--;
			break;
		}
		case 'P' : {
			_post_cnt--;
			break;
		}
		case 'N' : {
			_new_cnt--;
			break;
		}
	}

	_first_entry = qEntryRemove(_first_entry, name);
}

/**
 * Get the type of variable.
 *
 * @param format	variable name to remove
 *
 * @return	type character
 *		@li COOKIE			: 'C'
 *		@li GET method			: 'G'
 *		@li POST method			: 'P'
 *		@li New(added by qAdd())	: 'N'
 *		@li Not found			: '-'
 *
 * @note
 * @code
 *   char t = qGetType("NAME");
 * @endcode
 */
char qGetType(char *format, ...) {
	char name[1024];
	va_list arglist;
	int v_no;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();

	v_no = qEntryGetNo(_first_entry, name);
	if ((1 <= v_no) && (v_no <= _cookie_cnt)) return 'C';
	else if ((_cookie_cnt + 1 <= v_no) && (v_no <= _cookie_cnt + _get_cnt)) return 'G';
	else if ((_cookie_cnt + _get_cnt + 1 <= v_no) && (v_no <= _cookie_cnt + _get_cnt + _post_cnt)) return 'P';
	else if ((_cookie_cnt + _get_cnt + _post_cnt <= v_no)) return 'N';

	return '-';
}

/**
 * Get the first entry pointer of internal linked list.
 *
 * @return	first entry pointer. if no data stored, returns NULL.
 *
 * @note
 * @code
 *   Q_ENTRY *entry = qGetFirstEntry();
 * @endcode
 */
Q_ENTRY *qGetFirstEntry(void) {
	if (_first_entry == NULL) qDecoder();
	return _first_entry;
}

/**
 * Print out internal data for debugging.
 *
 * @return	the number of variables
 *
 * @note
 * @code
 *   qPrint();
 * @endcode
 */
int qPrint(void) {
	int amount;
	if (_first_entry == NULL) qDecoder();
	amount = qEntryPrint(_first_entry);
	printf("\n");
	printf("COOKIE = %d , GET = %d , POST = %d, NEW = %d\n", _cookie_cnt, _get_cnt, _post_cnt, _new_cnt);
	return amount;
}

/**
 * Set cookie
 *
 * This should be used before qContentType() is called.
 *
 * When cookie is set up through qCookieSet(), the point of time when values
 * are handed over through qGetValue() is when the next program is called.
 * In some implementations, however, cookies need to be set up for the
 * simplicity of logic while, at the same time, this needs to be applied to
 * other routines.
 *
 * In this case, qAdd() can prevent the alteration of logic and the waste
 * of additional codes by adding values to the cookie linked-list.
 * But with regard to qCookieSet(), setting up cookies at clients (browsers)
 * does not succeed always. Thus, users should be careful when using
 * qAdd().
 *
 * @return	in case of success, returns true.
 *		otherwise(qContentType() is called before) false
 *
 * @note
 * @code
 *   char *name = "NAME", *value = "Kim";
 *
 *   // Apply the NAME=Kim information in the current domain and directory
 *   // for 30 days.
 *   qCookieSet(name, value, 30, NULL, NULL, NULL);
 *
 *   // Apply the NAME=Kim information to the "/" directory of
 *   // "ANYTHING.qdecoder.org"
 *   // until the browser is shut down.
 *   qCookieSet(name, value, 0, "/", ".qdecoder.org", NULL);
 *
 *   // As for the followings, cookies will be set up only when security
 *   // requirements are satisfied.
 *   qCookieSet(name, value, 0, NULL, NULL, "SECURE");
 * @endcode
 */
bool qCookieSet(char *name, char *value, int exp_days, char *path, char *domain, char *secure) {
	char *Name, *Value;
	char cookie[(4 * 1024) + 256];

	/* check content flag */
	if (qGetContentFlag() == 1) return false;

	/* Name=Value */
	Name = qUrlEncode(name), Value = qUrlEncode(value);
	snprintf(cookie, sizeof(cookie), "%s=%s", Name, Value);
	free(Name), free(Value);

	if (exp_days != 0) {
		time_t plus_sec;
		char gmt[256];
		plus_sec = (time_t)(exp_days * 24 * 60 * 60);
		qGetGmtime(gmt, plus_sec);
		strcat(cookie, "; expires=");
		strcat(cookie, gmt);
	}

	if (path != NULL) {
		if (path[0] != '/') qError("qCookieSet(): Path string(%s) must start with '/' character.", path);
		strcat(cookie, "; path=");
		strcat(cookie, path);
	}

	if (domain != NULL) {
		if (strstr(domain, "/") != NULL || strstr(domain, ".") == NULL) qError("qCookieSet(): Invalid domain name(%s).", domain);
		strcat(cookie, "; domain=");
		strcat(cookie, domain);
	}

	if (secure != NULL) {
		strcat(cookie, "; secure");
	}

	printf("Set-Cookie: %s\n", cookie);

	return true;
}

/**
 * Remove cookie
 *
 * This should be used before qContentType() is called
 * and the arguments(path, domain, secure) must be exactly same as the
 * arguments of qCookieSet().
 *
 * When cookies are removed through qCookieRemove(),
 * the point of time when values in linked-list removed is when the next
 * program is called. In some implementations, however, cookies need to be
 * removed for the simplicity of logic while, at the same time,
 * this needs to be applied to other routines.
 *
 * In this case, qRemove() can prevent the alteration of logic and
 * the waste of additional codes by removing values to the linked-list.
 * But with regard to qCookieRemove(), removing cookies at clients (browsers)
 * does not succeed always.
 *
 * Thus, users should be careful when using qRemove().
 *
 * @return	in case of success, returns true.
 *		otherwise(qContentType() is called before) false
 *
 * @note
 * @code
 *   qCookieSet("NAME", "VALUE", 0, NULL, NULL, NULL);
 *   qCookieRemove("NAME", NULL, NULL, NULL);
 *
 *   qCookieSet("NAME", "VALUE", 0, "/", "www.qdecoder.org", NULL);
 *   qCookieRemove("NAME", "/", "www.qdecoder.org", NULL);
 * @endcode
 */
bool qCookieRemove(char *name, char *path, char *domain, char *secure) {

	/* check content flag */
	if (qGetContentFlag() == 1) return false;

	qCookieSet(name, "", -1, path, domain, secure);

	return true;
}

/**
 * Find out the cookie value.
 *
 * This only find the cookie value. Of course, you can use qGetValue() instead
 * but qGetValue() finds out variable from query(GET/POST) too. So it's different.
 * For some security reason, sometimes you need to get only cookie value,
 * in case of that, use this.
 *
 * @param format	variable name
 *
 * @return	the pointer of the cookie value.
 *		otherwise(not found) returns NULL.
 *
 * @note
 * @code
 *   char *cookie;
 *   cookie = qCookieGetValue("NAME");
 *   if(cookie == NULL) printf("cookie not found.\n");
 * @endcode
 */
char *qCookieGetValue(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();

	if (qGetType(name) == 'C') {
		return qEntryGetValue(_first_entry, name);
	}

	return NULL;
}

/**
 * Finds out the cookie value then converts to integer.
 *
 * @param format	variable name
 *
 * @return	integer converted value.
 *		otherwise(convertion error, not found) returns 0.
 *
 * @note
 * @code
 *   int cookieint;
 *   cookieint = qCookieGetInt("NAME");
 * @endcode
 */
int qCookieGetInt(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (_first_entry == NULL) qDecoder();

	if (qGetType(name) == 'C') {
		return qEntryGetInt(_first_entry, name);
	}

	return 0;
}

/**
 * Deallocates the allocated memory by qDecoder().
 *
 * @note
 * @code
 *   qFree();
 * @endcode
 */
void qFree(void) {
	qEntryFree(_first_entry);
	_first_entry = NULL;
	_multi_last_entry = NULL;
	strcpy(_multi_last_key, "");
	_cookie_cnt = 0, _get_cnt = 0, _post_cnt = 0, _new_cnt = 0;
}

/**
 * Deallocates every allocated memory(including session data) and re-initialize.
 *
 * When you wish to make a daemon-type repetitive program or to initialize
 * qDecoder new, you can use this function. qReset() deallocates all the
 * allocated memory including the linked-list and restores internal static
 * variables to the initial condition.
 *
 * @note
 * @code
 *   qReset();
 * @endcode
 */
void qReset(void) {
	qFree();
	qSessionFree();

	/* reset static variables */
	qErrorLog(NULL);
	qErrorContact(NULL);
	qResetContentFlag();
}
