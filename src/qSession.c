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

#ifndef WITHOUT_CGISUPPORT

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
#define INTER_CREATED_GMT		INTER_PREFIX "CREATED-GMT"
#define INTER_CREATED_SEC		INTER_PREFIX "CREATED"
#define INTER_INTERVAL_SEC		INTER_PREFIX "INTERVAL"
#define INTER_CONNECTIONS		INTER_PREFIX "CONNECTIONS"

#define SESSION_DEFAULT_TIMEOUT_INTERVAL	(30 * 60)


/**********************************************
** Internal Functions Definition
**********************************************/

static int _clearRepository(void);
static int _isValidSession(char *filename);
static time_t _updateTimeout(char *filename, time_t timeout_interval);


/**********************************************
** Static Values Definition used only internal
**********************************************/

static bool _session_started = false;
static bool _session_new = false;
static bool _session_modified = false;
static Q_ENTRY *_session_first_entry = NULL;

static char _session_repository_path[1024];
static char _session_storage_path[1024];
static char _session_timeout_path[1024];
static time_t _session_timeout_interval = (time_t)SESSION_DEFAULT_TIMEOUT_INTERVAL; /* seconds */


/**********************************************
** Usage : qSession(Repository Path);
** Return: New session 1 else 0.
** Do    : Start Session.
**
** ex) qSession(NULL);   // use default storage
**     qSession("/tmp"); // use /tmp for session storage
**********************************************/
/* Initialize session data */
int qSession(char *repository) {
	bool new_session;
	char *sessionkey;

	/* check if session already started */
	if (_session_started == true) return _session_new;
	_session_first_entry = NULL;
	_session_started = true;
	_session_modified = false;

	/* check content flag */
	if (qGetContentFlag() == 1) qError("qSession(): must be called before qContentType() and any stream out.");

	/* check session status & get session id */
	sessionkey = qGetValue(SESSION_ID);
	if (sessionkey == NULL) {  /* new session */
		sessionkey = qUniqId();
		new_session = true;
	} else {
		new_session = false;
	}

	/* make storage path for session */
	if (repository != NULL) qStrncpy(_session_repository_path, repository, sizeof(_session_repository_path));
	else qStrncpy(_session_repository_path, SESSION_DEFAULT_REPOSITORY, sizeof(_session_repository_path));
	snprintf(_session_storage_path, sizeof(_session_storage_path), "%s/%s%s%s", _session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
	snprintf(_session_timeout_path, sizeof(_session_timeout_path), "%s/%s%s%s", _session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

	/* validate exist session */
	if (new_session == false) {
		if (_isValidSession(_session_timeout_path) <= 0) { /* expired or not found */
			unlink(_session_storage_path);
			unlink(_session_timeout_path);

			/* remake storage path */
			sessionkey = qUniqId();
			snprintf(_session_storage_path, sizeof(_session_storage_path), "%s/%s%s%s", _session_repository_path, SESSION_PREFIX, sessionkey, SESSION_STORAGE_EXTENSION);
			snprintf(_session_timeout_path, sizeof(_session_timeout_path), "%s/%s%s%s", _session_repository_path, SESSION_PREFIX, sessionkey, SESSION_TIMEOUT_EXTENSION);

			/* set flag */
			new_session = true;
		}
	}

	/* if new session, set session id */
	if (new_session == true) {
		char created_sec[32];

		qCookieSet(SESSION_ID, sessionkey, 0, "/", NULL, NULL);
		qAdd(SESSION_ID, sessionkey); /* force to add session_in to query list */

		/* save session informations */
		snprintf(created_sec, sizeof(created_sec), "%ld", time(NULL));

		_session_first_entry = qEntryAdd(_session_first_entry, INTER_SESSIONID, sessionkey, 1);
		qEntryAdd(_session_first_entry, INTER_CREATED_GMT, qGetGmtimeStr(0), 1);
		qEntryAdd(_session_first_entry, INTER_CREATED_SEC, created_sec, 1);
		qEntryAdd(_session_first_entry, INTER_CONNECTIONS, "1", 1);

		/* set timeout interval */
		qSessionSetTimeout(_session_timeout_interval);
	}
	/* else read session properties */
	else {
		int conns;
		char connstr[16];

		/* read exist session informations */
		_session_first_entry = qEntryLoad(_session_storage_path, true);

		/* update session informations */
		conns = qSessionGetInt(INTER_CONNECTIONS);
		snprintf(connstr, sizeof(connstr), "%d", ++conns);
		qEntryAdd(_session_first_entry, INTER_CONNECTIONS, connstr, 1);

		/* set timeout interval */
		qSessionSetTimeout((time_t)atol(qSessionGetValue(INTER_INTERVAL_SEC)));
	}

	/* set globals */
	_session_new = new_session;
	return _session_new;
}

/**********************************************
** Usage : qSessionAdd(name, value);
** Return: Stored String pointer of value.
** Do    : Add session value.
**
** ex) qSessionAdd("name", "qDecoder");
**********************************************/
char *qSessionAdd(char *name, char *format, ...) {
	Q_ENTRY *new_entry;
	char value[1024];
	va_list arglist;

	if (_session_started == false) qError("qSessionAdd(): qSession() must be called before.");
	if (!strcmp(name, "")) qError("qSessionAdd(): can not add empty name.");
	if (!strncmp(name, INTER_PREFIX, strlen(INTER_PREFIX))) qError("qSessionAdd(): Name can not start with %s. It's reserved for internal uses.", INTER_PREFIX);

	va_start(arglist, format);
	vsnprintf(value, sizeof(value)-1, format, arglist);
	value[sizeof(value)-1] = '\0';
	va_end(arglist);

	new_entry = qEntryAdd(_session_first_entry, name, value, 1);
	if (!_session_first_entry) _session_first_entry = new_entry;

	/* set modified flag */
	_session_modified = true;

	return qSessionGetValue(name);
}

/**********************************************
** Usage : qSessionAddInt(name, integer);
** Return: Stored integer value.
** Do    : Add session value of integer type.
**
** ex) qSessionAddInt("count", 32);
**********************************************/
int qSessionAddInt(char *name, int valueint) {
	char value[32];

	snprintf(value, sizeof(value),"%d", valueint);
	qSessionAdd(name, value);

	return qSessionGetInt(name);
}

/**********************************************
** Usage : qSessionUpdateInt(name, plus integer);
** Return: Updated integer value.
** Do    : Update session value of integer type.
**
** ex) qSessionUpdateInt("count", -4);
**********************************************/
int qSessionUpdateInt(char *name, int plusint) {
	qSessionAddInt(name, qSessionGetInt(name) + plusint);

	return qSessionGetInt(name);
}

/**********************************************
** Usage : qSessionRemove(name);
** Do    : Remove session variable.
**
** ex) qSessionRemove("name");
**     qSessionRemove("%d.name", i);
**********************************************/
void qSessionRemove(char *format, ...) {
	char name[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	if (!strcmp(name, "")) qError("qAddRemove(): can not remove empty name.");
	if (_session_started == false) qError("qSessionRemove(): qSession() must be called before.");
	if (!strncmp(name, INTER_PREFIX, strlen(INTER_PREFIX))) qError("qSessionRemove(): can not remove reserved words.");

	_session_first_entry = qEntryRemove(_session_first_entry, name);

	/* set modified flag */
	_session_modified = true;
}

/**********************************************
** Usage : qSessionGetValue(name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Return session value.
**
** ex) char *value;
**     value = qSessionGetValue("name");
**     value = qSessionGetValue("%d.name", i);
**********************************************/
char *qSessionGetValue(char *format, ...) {
	char name[1024], *value;
	va_list arglist;

	if (_session_started == false) qError("qSessionGetValue(): qSession() must be called before.");

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	value = qEntryGetValue(_session_first_entry, name);

	return value;
}

/**********************************************
** Usage : qSessionGetInt(name);
** Return: Success integer of value, Fail 0.
** Do    : Return session value.
**
** ex) int value;
**     value = qSessionGetInt("count");
**********************************************/
int qSessionGetInt(char *format, ...) {
	char name[1024];
	int value;
	va_list arglist;

	if (_session_started == false) qError("qSessionGetInt(): qSession() must be called before.");

	va_start(arglist, format);
	vsnprintf(name, sizeof(name)-1, format, arglist);
	name[sizeof(name)-1] = '\0';
	va_end(arglist);

	value = qEntryGetInt(_session_first_entry, name);

	return value;
}

/**********************************************
** Usage : qSessionSetTimeout(interval seconds);
** Return: New expiration period.
** Do    : Change session expiration period.
**
** ex) qSessionSetTimeout((time_t)3600);
**********************************************/
time_t qSessionSetTimeout(time_t seconds) {
	char interval_sec[32];

	if (_session_started == false) qError("qSessionSetTimeout(): qSession() must be called before.");
	if (seconds <= (time_t)0) qError("qSessionSetTimeout(): can not set negative interval. Use qSessionDestory() instead.");

	_session_timeout_interval = seconds;

	/* save session informations */
	snprintf(interval_sec, sizeof(interval_sec), "%ld", (long)_session_timeout_interval);
	qEntryAdd(_session_first_entry, INTER_INTERVAL_SEC, interval_sec, 1);

	return _session_timeout_interval;
}

/**********************************************
** Usage : qSessionGetID();
** Return: String pointer of session id.
** Do    : Return current session id.
**
** ex) char *sessionid;
**     sessionid = qSessionGetID();
**********************************************/
char *qSessionGetID(void) {
	if (_session_started == false) qError("qSessionGetID(): qSession() must be called before.");
	return qSessionGetValue(INTER_SESSIONID);
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
time_t qSessionGetCreated(void) {
	time_t created;
	char *tmp;

	if (_session_started == false) qError("qSessionGetCreated(): qSession() must be called before.");
	tmp = qSessionGetValue(INTER_CREATED_SEC);
	created = (time_t)atol(tmp);

	return created;
}

/**********************************************
** Usage : qSessionPrint();
** Do    : Print all session variables for debugging
**********************************************/
int qSessionPrint(FILE *out) {
	if (_session_started == false) qError("qSessionPrint(): qSession() must be called before.");
	return qEntryPrint(_session_first_entry, out);
}

/**********************************************
** Usage : qSessionSave();
** Do    : Save session data immediately.
**********************************************/
void qSessionSave(void) {
	if (_session_started == false || _session_first_entry == NULL) return;
	if (_session_new == true && _session_modified == false) return;

	if (qEntrySave(_session_first_entry, _session_storage_path, true) == false) {
		qError("qSessionSave(): Can not access session repository(%s).", _session_storage_path);
	}
	if (_updateTimeout(_session_timeout_path, _session_timeout_interval) == 0) {
		qError("qSessionSave(): Can not access session repository(%s).", _session_timeout_path);
	}

	/* clear modified flag */
	_session_modified = false;
}

/**********************************************
** Usage : qSessionFree();
** Do    : Save session data and deallocate memories.
**********************************************/
/* Free & Save */
void qSessionFree(void) {
	if (_session_started == false) return;

	qSessionSave();
	_clearRepository();

	if (_session_first_entry) qEntryFree(_session_first_entry);
	_session_first_entry = NULL;
	_session_started = false;
	_session_new = false;
	_session_modified = false;
	_session_timeout_interval = (time_t)SESSION_DEFAULT_TIMEOUT_INTERVAL;
	strcpy(_session_repository_path, "");
	strcpy(_session_storage_path, "");
	strcpy(_session_timeout_path, "");
}

/**********************************************
** Usage : qSessionDestroy();
** Do    : Destroy current session and stored all session data
**         will be removed.
**********************************************/
void qSessionDestroy(void) {
	if (_session_started == false) qError("qSessionDestroy(): qSession() must be called before.");

	unlink(_session_storage_path);
	unlink(_session_timeout_path);
	if (_session_first_entry) qEntryFree(_session_first_entry);
	_session_first_entry = NULL;

	qSessionFree();

	if (qGetContentFlag() == 0) {
		qCookieRemove(SESSION_ID, "/", NULL, NULL);
	}
}

/**********************************************
** Internal Functions
**********************************************/

static int _clearRepository(void) {
#ifdef _WIN32
	return 0;
#else
	DIR *dp;
	struct dirent *dirp;
	char timeoutpath[1024];
	int clearcnt;

	if (_session_started == false) qError("_clearRepository(): qSession() must be called before.");
	snprintf(timeoutpath, sizeof(timeoutpath), "%s/%s", _session_repository_path, SESSION_TIMETOCLEAR_FILENAME);

	if (_isValidSession(timeoutpath) > 0) return 0; /* Valid */

	/* expired or not found, main routine start here */
	/* to prevent race condition, update time stamp first */
	if (_updateTimeout(timeoutpath, _session_timeout_interval) == 0) {
		qError("_clearRepository(): Can not access session repository(%s).", timeoutpath);
	}

	/* clear old session data */
	if ((dp = opendir(_session_repository_path)) == NULL) qError("_clearRepository(): Can not access session repository(%s).", _session_repository_path);

	for (clearcnt = 0; (dirp = readdir(dp)) != NULL; ) {
		if (strstr(dirp->d_name, SESSION_PREFIX) && strstr(dirp->d_name, SESSION_TIMEOUT_EXTENSION)) {
			snprintf(timeoutpath, sizeof(timeoutpath), "%s/%s", _session_repository_path, dirp->d_name);
			if (_isValidSession(timeoutpath) <= 0) { /* expired */
				/* remove timeout */
				unlink(timeoutpath);

				/* remove properties */
				timeoutpath[strlen(timeoutpath) - strlen(SESSION_TIMEOUT_EXTENSION)] = '\0';
				strcat(timeoutpath, SESSION_STORAGE_EXTENSION);
				unlink(timeoutpath);

				clearcnt++;
			}
		}
	}
	closedir(dp);

	return clearcnt;
#endif
}

/* session not found 0, session expired -1, session valid 1 */
static int _isValidSession(char *filename) {
	time_t timeout, timenow;
	double timediff;

	if ((timeout = (time_t)qCountRead(filename)) == 0) return 0;
	;
	timenow = time(NULL);
	timediff = difftime(timeout, timenow); /* return timeout - timenow */

	if (timediff >= (double)0) return 1; /* valid */
	return -1; /* expired */
}

/* success > 0, write fail 0 */
static time_t _updateTimeout(char *filename, time_t timeout_interval) {
	FILE *fp;
	time_t timeout;

	timeout = time(NULL);
	timeout += timeout_interval;

	if ((fp = fopen(filename, "w")) == NULL) return 0;
	fprintf(fp, "%ld\n", (long)timeout);
	fclose(fp);

	return timeout;
}

#endif /* WITHOUT_CGISUPPORT */
