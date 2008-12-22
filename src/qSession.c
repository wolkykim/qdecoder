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
 * @file qSession.c HTTP Session Handling API
 */

#ifndef DISABLE_CGI

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#include "qDecoder.h"
#include "qInternal.h"

#ifdef _WIN32
#define	SESSION_DEFAULT_REPOSITORY	"C:\\Windows\\Temp"
#else
#define	SESSION_DEFAULT_REPOSITORY	"/tmp"
#endif

#define SESSION_ID			"QSESSIONID"
#define SESSION_PREFIX			"qsession-"
#define SESSION_STORAGE_EXTENSION	".properties"
#define SESSION_TIMEOUT_EXTENSION	".expire"
#define SESSION_TIMETOCLEAR_FILENAME	"qsession-timetoclear"

#define INTER_PREFIX			"_Q_"
#define INTER_SESSIONID			INTER_PREFIX "SESSIONID"
#define INTER_SESSION_REPO		INTER_PREFIX "REPOSITORY"
#define INTER_CREATED_SEC		INTER_PREFIX "CREATED"
#define INTER_INTERVAL_SEC		INTER_PREFIX "INTERVAL"
#define INTER_CONNECTIONS		INTER_PREFIX "CONNECTIONS"

#define SESSION_DEFAULT_TIMEOUT_INTERVAL	(30 * 60)

static bool _clearRepository(const char *session_repository_path);
static int _isValidSession(const char *filepath);
static bool _updateTimeout(const char *filepath, time_t timeout_interval);

/**
 * Initialize session
 *
 * @param request	a pointer of request structure returned by qCgiRequestParse()
 * @param dirpath	directory path where session data will be kept
 *
 * @return 	a pointer of malloced session data list (Q_ENTRY type)
 *
 * @note
 * The returned Q_ENTRY list must be de-allocated by calling qEntryFree().
 * And if you want to append or remove some user session data, use qEntry*()
 * functions then finally call qSessionSave() to store updated session data.
 */
Q_ENTRY *qSessionInit(Q_ENTRY *request, const char *dirpath) {
	/* check content flag */
	if (qCgiResponseGetContentType(request) != NULL) {
		DEBUG("Should be called before qRequestSetContentType().");
		return NULL;
	}

	Q_ENTRY *session = qEntryInit();
	if(session == NULL) return NULL;

	/* check session status & get session id */
	bool new_session;
	char *sessionkey;
	if(qEntryGetStr(request, SESSION_ID) != NULL) {
		sessionkey = strdup(qEntryGetStr(request, SESSION_ID));
		new_session = false;
	} else { /* new session */
		sessionkey = qStrUnique(getenv("REMOTE_ADDR"));
		new_session = true;
	}

	/* make storage path for session */
	char session_repository_path[MAX_PATHLEN];
	char session_storage_path[MAX_PATHLEN];
	char session_timeout_path[MAX_PATHLEN];
	time_t session_timeout_interval = (time_t)SESSION_DEFAULT_TIMEOUT_INTERVAL; /* seconds */

	if (dirpath != NULL) qStrCpy(session_repository_path, sizeof(session_repository_path), dirpath, sizeof(session_repository_path));
	else qStrCpy(session_repository_path, sizeof(session_repository_path), SESSION_DEFAULT_REPOSITORY, sizeof(session_repository_path));
	snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
	snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

	/* validate exist session */
	if (new_session == false) {
		int valid = _isValidSession(session_timeout_path);
		if (valid <= 0) { /* expired or not found */
			if(valid < 0) {
				_q_unlink(session_storage_path);
				_q_unlink(session_timeout_path);
			}

			/* remake storage path */
			free(sessionkey);
			sessionkey = qStrUnique(getenv("REMOTE_ADDR"));
			snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
			snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

			/* set flag */
			new_session = true;
		}
	}

	/* if new session, set session id */
	if (new_session == true) {
		qCgiResponseSetCookie(request, SESSION_ID, sessionkey, 0, "/", NULL, NULL);
		qEntryPutStr(request, SESSION_ID, sessionkey, true); /* force to add session_in to query list */

		/* save session informations */
		char created_sec[10+1];
		snprintf(created_sec, sizeof(created_sec), "%ld", (long int)time(NULL));
		qEntryPutStr(session, INTER_SESSIONID, sessionkey, false);
		qEntryPutStr(session, INTER_SESSION_REPO, session_repository_path, false);
		qEntryPutStr(session, INTER_CREATED_SEC, created_sec, false);
		qEntryPutInt(session, INTER_CONNECTIONS, 1, false);

		/* set timeout interval */
		qSessionSetTimeout(session, session_timeout_interval);
	} else { /* read session properties */

		/* read exist session informations */
		qEntryLoad(session, session_storage_path, '=', true);

		/* update session informations */
		int conns = qEntryGetInt(session, INTER_CONNECTIONS);
		qEntryPutInt(session, INTER_CONNECTIONS, ++conns, true);

		/* set timeout interval */
		qSessionSetTimeout(session, qEntryGetInt(session, INTER_INTERVAL_SEC));
	}

	free(sessionkey);

	/* set globals */
	return session;
}

/**
 * Set the auto-expiration seconds about user session
 *
 * @param session	a pointer of session structure
 * @param seconds	expiration seconds
 *
 * @return 	true if successful, otherwise returns false
 *
 * @note Default timeout is defined as SESSION_DEFAULT_TIMEOUT_INTERVAL. 1800 seconds
 */
bool qSessionSetTimeout(Q_ENTRY *session, time_t seconds) {
	if (seconds <= 0) return false;
	qEntryPutInt(session, INTER_INTERVAL_SEC, (int)seconds, true);
	return true;
}

/**
 * Get user session id
 *
 * @param session	a pointer of session structure
 *
 * @return 	a pointer of session identifier
 *
 * @note Do not free manually
 */
const char *qSessionGetId(Q_ENTRY *session) {
	return qEntryGetStr(session, INTER_SESSIONID);
}

/**
 * Get user session created time
 *
 * @param session	a pointer of session structure
 *
 * @return 	user session created time in UTC time seconds
 */
time_t qSessionGetCreated(Q_ENTRY *session) {
	const char *created = qEntryGetStr(session, INTER_CREATED_SEC);
	return (time_t)atol(created);
}

/**
 * Update session data
 *
 * @param session	a pointer of session structure
 *
 * @return 	true if successful, otherwise returns false
 */
bool qSessionSave(Q_ENTRY *session) {
	const char *sessionkey = qEntryGetStr(session, INTER_SESSIONID);
	const char *session_repository_path = qEntryGetStr(session, INTER_SESSION_REPO);
	int session_timeout_interval = qEntryGetInt(session, INTER_INTERVAL_SEC);
	if(sessionkey == NULL || session_repository_path == NULL) return false;

	char session_storage_path[MAX_PATHLEN];
	char session_timeout_path[MAX_PATHLEN];
	snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
	snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

	if (qEntrySave(session, session_storage_path, '=', true) == false) {
		DEBUG("Can't save session file %s", session_storage_path);
		return false;
	}
	if (_updateTimeout(session_timeout_path, session_timeout_interval) == 0) {
		DEBUG("Can't update file %s", session_timeout_path);
		return false;
	}

	_clearRepository(session_repository_path);
	return true;
}

/**
 * Destroy user session
 *
 * @param session	a pointer of session structure
 *
 * @return 	true if successful, otherwise returns false
 *
 * @note
 * If you only want to de-allocate session structure, just call qEntryFree().
 * This will remove all user session data permanantely and also free the session structure.
 */
bool qSessionDestroy(Q_ENTRY *session) {
	const char *sessionkey = qEntryGetStr(session, INTER_SESSIONID);
	const char *session_repository_path = qEntryGetStr(session, "INTER_SESSION_REPO");
	if(sessionkey == NULL || session_repository_path == NULL) {
		if(session != NULL) qEntryFree(session);
		return false;
	}

	char session_storage_path[MAX_PATHLEN];
	char session_timeout_path[MAX_PATHLEN];
	snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
	snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

	_q_unlink(session_storage_path);
	_q_unlink(session_timeout_path);

	if(session != NULL) qEntryFree(session);
	return true;
}

static bool _clearRepository(const char *session_repository_path) {
#ifdef _WIN32
	return false;
#else
	/* clear old session data */
	DIR *dp;
	if ((dp = opendir(session_repository_path)) == NULL) {
		DEBUG("Can't open session repository %s", session_repository_path);
		return false;
	}

	struct dirent *dirp;
	while((dirp = readdir(dp)) != NULL) {
		if (strstr(dirp->d_name, SESSION_PREFIX) && strstr(dirp->d_name, SESSION_TIMEOUT_EXTENSION)) {
			char timeoutpath[MAX_PATHLEN];
			snprintf(timeoutpath, sizeof(timeoutpath), "%s/%s", session_repository_path, dirp->d_name);
			if (_isValidSession(timeoutpath) <= 0) { /* expired */
				/* remove timeout */
				_q_unlink(timeoutpath);

				/* remove properties */
				timeoutpath[strlen(timeoutpath) - strlen(SESSION_TIMEOUT_EXTENSION)] = '\0';
				strcat(timeoutpath, SESSION_STORAGE_EXTENSION);
				_q_unlink(timeoutpath);
			}
		}
	}
	closedir(dp);

	return true;
#endif
}

/* session not found 0, session expired -1, session valid 1 */
static int _isValidSession(const char *filepath) {
	time_t timeout, timenow;
	double timediff;

	if ((timeout = (time_t)qCountRead(filepath)) == 0) return 0;

	timenow = time(NULL);
	timediff = difftime(timeout, timenow); /* return timeout - timenow */

	if (timediff >= 0) return 1; /* valid */
	return -1; /* expired */
}

/* success > 0, write fail 0 */
static bool _updateTimeout(const char *filepath, time_t timeout_interval) {
	if(timeout_interval <= 0) return false;

	if(qCountSave(filepath, time(NULL) + timeout_interval) <= 0) return false;
	return true;
}

#endif /* DISABLE_CGI */
