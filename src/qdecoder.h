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
 * qDecoder Header file
 *
 * @file	qdecoder.h
 */

#ifndef _QDECODER_H
#define _QDECODER_H

#define _Q_PRGNAME			"qDecoder"
#define _Q_VERSION			"11.0.1"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

/*
 * Types and definitions
 */
typedef	struct _Q_ENTRY		Q_ENTRY;
typedef	struct _Q_ENTOBJ_T	Q_ENTOBJ_T;

typedef enum {
	Q_CGI_ALL = 0
	, Q_CGI_COOKIE
	, Q_CGI_GET
	, Q_CGI_POST
} Q_CGI_T;

/* qDecoder C++ support */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * qCgiRequest.c
 */
extern	Q_ENTRY*	qCgiRequestSetOption(Q_ENTRY *request, bool filemode, const char *basepath, int clearold);
extern	Q_ENTRY*	qCgiRequestParse(Q_ENTRY *request, Q_CGI_T method);
extern	char*		qCgiRequestGetQuery(Q_CGI_T method);

/*
 * qCgiResponse.c
 */
extern	bool		qCgiResponseSetCookie(Q_ENTRY *request, const char *name, const char *value, int expire, const char *path, const char *domain, bool secure);
extern	bool		qCgiResponseRemoveCookie(Q_ENTRY *request, const char *name, const char *path, const char *domain, bool secure);

extern	bool		qCgiResponseSetContentType(Q_ENTRY *request, const char *mimetype);
extern	const char*	qCgiResponseGetContentType(Q_ENTRY *request);
extern	bool		qCgiResponseRedirect(Q_ENTRY *request, const char *uri);
extern	int		qCgiResponseDownload(Q_ENTRY *request, const char *filepath, const char *mimetype);
extern	void		qCgiResponseError(Q_ENTRY *request, char *format, ...);

/*
 * qSession.c
 */
extern	Q_ENTRY*	qSessionInit(Q_ENTRY *request, const char *dirpath);
extern	bool		qSessionSetTimeout(Q_ENTRY *session, time_t seconds);
extern	const char*	qSessionGetId(Q_ENTRY *session);
extern	time_t		qSessionGetCreated(Q_ENTRY *session);
extern	bool		qSessionSave(Q_ENTRY *session);
extern	bool		qSessionDestroy(Q_ENTRY *session);

/*
 * qEntry.c - Linked-List Table
 */

/* Constructor. */
extern	Q_ENTRY*	qEntry(void);

/* Container details. */
struct _Q_ENTRY {
	int		num;		/*!< number of objects */
	size_t		size;		/*!< total size of data objects, does not include name size */
	Q_ENTOBJ_T*	first;		/*!< first object pointer */
	Q_ENTOBJ_T*	last;		/*!< last object pointer */

	/* public member methods */
	void		(*lock)		(Q_ENTRY *entry);
	void		(*unlock)	(Q_ENTRY *entry);

	bool		(*put)		(Q_ENTRY *entry, const char *name, const void *data, size_t size, bool replace);
	bool		(*putStr)	(Q_ENTRY *entry, const char *name, const char *str, bool replace);
	bool		(*putStrf)	(Q_ENTRY *entry, bool replace, const char *name, const char *format, ...);
	bool		(*putInt)	(Q_ENTRY *entry, const char *name, int num, bool replace);

	void*		(*get)		(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	void*		(*getCase)	(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	void*		(*getLast)	(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	char*		(*getStr)	(Q_ENTRY *entry, const char *name, bool newmem);
	char*		(*getStrf)	(Q_ENTRY *entry, bool newmem, const char *namefmt, ...);
	char*		(*getStrCase)	(Q_ENTRY *entry, const char *name, bool newmem);
	char*		(*getStrLast)	(Q_ENTRY *entry, const char *name, bool newmem);
	int		(*getInt)	(Q_ENTRY *entry, const char *name);
	int		(*getIntCase)	(Q_ENTRY *entry, const char *name);
	int 		(*getIntLast)	(Q_ENTRY *entry, const char *name);
	bool		(*getNext)	(Q_ENTRY *entry, Q_ENTOBJ_T *obj, const char *name, bool newmem);

	int		(*remove)	(Q_ENTRY *entry, const char *name);

	int 		(*getNum)	(Q_ENTRY *entry);

	bool		(*truncate)	(Q_ENTRY *entry);
	bool		(*save)		(Q_ENTRY *entry, const char *filepath);
	int		(*load)		(Q_ENTRY *entry, const char *filepath);
	bool		(*reverse)	(Q_ENTRY *entry);
	bool		(*print)	(Q_ENTRY *entry, FILE *out, bool print_data);
	bool		(*free)		(Q_ENTRY *entry);
};

struct _Q_ENTOBJ_T {
	char*		name;		/*!< object name */
	void*		data;		/*!< data object */
	size_t		size;		/*!< object size */
	Q_ENTOBJ_T*	next;		/*!< link pointer */
};

#ifdef __cplusplus
}
#endif

#endif /*_QDECODER_H */
