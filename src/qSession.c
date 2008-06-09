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
 * @file qSession.c HTTP Session Handling API
 */

#ifndef DISABLE_CGISUPPORT

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


/**********************************************
** Define Paragraph
**********************************************/

#ifdef _WIN32
#define	SESSION_DEFAULT_REPOSITORY	"C:\\Windows\\Temp"
#else
#define	SESSION_DEFAULT_REPOSITORY	"/tmp"
#endif

#define SESSION_ID			"QSESSIONID"
#define SESSION_PREFIX			"qsession-"
#define SESSION_STORAGE_EXTENSION	".properties"
#define SESSION_TIMEOUT_EXTENSION	".timeout"
#define SESSION_TIMETOCLEAR_FILENAME	"qsession-timetoclear"

#define INTER_PREFIX			"_Q_"
#define INTER_SESSIONID			INTER_PREFIX "SESSIONID"
#define INTER_SESSION_REPO		INTER_PREFIX "REPOSITORY"
#define INTER_CREATED_GMT		INTER_PREFIX "CREATED-GMT"
#define INTER_CREATED_SEC		INTER_PREFIX "CREATED"
#define INTER_INTERVAL_SEC		INTER_PREFIX "INTERVAL"
#define INTER_CONNECTIONS		INTER_PREFIX "CONNECTIONS"

#define SESSION_DEFAULT_TIMEOUT_INTERVAL	(30 * 60)


/**********************************************
** Internal Functions Definition
**********************************************/

static bool _clearRepository(const char *session_repository_path);
static int _isValidSession(const char *filepath);
static bool _updateTimeout(const char *filepath, time_t timeout_interval);


/**********************************************
** Usage : qSession(Repository Path);
** Return: New session 1 else 0.
** Do    : Start Session.
**
** ex) qSession(NULL);   // use default storage
**     qSession("/tmp"); // use /tmp for session storage
**********************************************/
/* Initialize session data */
Q_ENTRY *qSessionInit(Q_ENTRY *request, const char *dirpath) {
	/* check content flag */
	if (qCgiResponseGetContentType(request) != NULL) {
		DEBUG("Should be called before qRequestSetContentType().");
		return NULL;
	}

	/* check session status & get session id */
	bool new_session;
	const char *sessionkey;
	if(request != NULL && qEntryGetStr(request, SESSION_ID) != NULL) {
		sessionkey = qEntryGetStr(request, SESSION_ID);
		new_session = false;
	} else { /* new session */
		sessionkey = qUniqId();
		new_session = true;
	}

	/* make storage path for session */
	char session_repository_path[MAX_PATHLEN];
	char session_storage_path[MAX_PATHLEN];
	char session_timeout_path[MAX_PATHLEN];
	time_t session_timeout_interval = (time_t)SESSION_DEFAULT_TIMEOUT_INTERVAL; /* seconds */

	if (dirpath != NULL) qStrncpy(session_repository_path, dirpath, sizeof(session_repository_path)-1);
	else qStrncpy(session_repository_path, SESSION_DEFAULT_REPOSITORY, sizeof(session_repository_path)-1);
	snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
	snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

	/* validate exist session */
	if (new_session == false) {
		int valid = _isValidSession(session_timeout_path);
		if (valid <= 0) { /* expired or not found */
			if(valid < 0) {
				unlink(session_storage_path);
				unlink(session_timeout_path);
			}

			/* remake storage path */
			sessionkey = qUniqId();
			snprintf(session_storage_path, sizeof(session_storage_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
			snprintf(session_timeout_path, sizeof(session_timeout_path), "%s/%s%s%s", session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

			/* set flag */
			new_session = true;
		}
	}

	Q_ENTRY *session = qEntryInit();
	if(session == NULL) return NULL;

	/* if new session, set session id */
	if (new_session == true) {
		qCgiResponseSetCookie(request, SESSION_ID, sessionkey, 0, "/", NULL, NULL);
		qEntryPutStr(request, SESSION_ID, sessionkey, true); /* force to add session_in to query list */

		/* save session informations */
		char created_sec[10+1];
		snprintf(created_sec, sizeof(created_sec), "%ld", time(NULL));
		qEntryPutStr(session, INTER_SESSIONID, sessionkey, false);
		qEntryPutStr(session, INTER_SESSION_REPO, session_repository_path, false);
		qEntryPutStr(session, INTER_CREATED_GMT, qGetGmtimeStr(0), false);
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

	/* set globals */
	return session;
}

/**********************************************
** Usage : qSessionSetTimeout(interval seconds);
** Return: New expiration period.
** Do    : Change session expiration period.
**
** ex) qSessionSetTimeout((time_t)3600);
**********************************************/
bool qSessionSetTimeout(Q_ENTRY *session, time_t seconds) {
	if (seconds <= 0) return false;
	qEntryPutInt(session, INTER_INTERVAL_SEC, (int)seconds, true);
	return true;
}

/**********************************************
** Usage : qSessionGetID();
** Return: String pointer of session id.
** Do    : Return current session id.
**
** ex) char *sessionid;
**     sessionid = qSessionGetID();
**********************************************/
const char *qSessionGetID(Q_ENTRY *session) {
	return qEntryGetStr(session, INTER_SESSIONID);
}

/**********************************************
** Usage : qSessionGetCreated();
** Return: Value of time in seconds since 0 hours,
**         0 minutes, 0 seconds, January 1, 1970.
** Do    : Return session created time in seconds.
**
** ex) time_t created;
**     struct tm *gmtime;
**     created = qSessionGetCreated();
**     gmtime = gmtime(&created);
**********************************************/
time_t qSessionGetCreated(Q_ENTRY *session) {
	const char *created = qEntryGetStr(session, INTER_CREATED_SEC);
	return (time_t)atol(created);
}

/**********************************************
** Usage : qSessionSave();
** Do    : Save session data immediately.
**********************************************/
bool qSessionSave(Q_ENTRY *session) {
	const char *sessionkey = qEntryGetStr(session, INTER_SESSIONID);
	const char *session_repository_path = qEntryGetStr(session, "INTER_SESSION_REPO");
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

/**********************************************
** Usage : qSessionDestroy();
** Do    : Destroy current session and stored all session data
**         will be removed.
**********************************************/
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

	unlink(session_storage_path);
	unlink(session_timeout_path);

	if(session != NULL) qEntryFree(session);
	return true;
}

/**********************************************
** Internal Functions
**********************************************/

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
				unlink(timeoutpath);

				/* remove properties */
				timeoutpath[strlen(timeoutpath) - strlen(SESSION_TIMEOUT_EXTENSION)] = '\0';
				strcat(timeoutpath, SESSION_STORAGE_EXTENSION);
				unlink(timeoutpath);
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
	;
	timenow = time(NULL);
	timediff = difftime(timeout, timenow); /* return timeout - timenow */

	if (timediff >= (double)0) return 1; /* valid */
	return -1; /* expired */
}

/* success > 0, write fail 0 */
static bool _updateTimeout(const char *filepath, time_t timeout_interval) {
	if(timeout_interval <= 0) return false;

	if(qCountUpdate(filepath, timeout_interval) <= 0) return false;
	return true;
}

#endif /* DISABLE_CGISUPPORT */
