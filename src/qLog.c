/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

#include "qDecoder.h"

/*
 * static function prototypes
 */
static int _realOpen(Q_LOG *log);

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_LOG *qLogOpen(char *logbase, char *filenameformat, int rotateinterval, Q_BOOL flush) {
	Q_LOG *log;

	/* malloc Q_LOG structure */
	if ((log = (Q_LOG *)malloc(sizeof(Q_LOG))) == NULL) return NULL;

	/* fill structure */
	strcpy(log->logbase, logbase);
	strcpy(log->nameformat, filenameformat);
	log->fp = NULL;
	log->console = Q_FALSE;
	log->rotateinterval = ((rotateinterval > 0) ? rotateinterval : 0);
	log->nextrotate = 0;
	log->flush = flush;

	if (_realOpen(log) == 0) {
		qLogClose(log);
		return NULL;
	}

	return log;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qLogClose(Q_LOG *log) {
	if (log == NULL) return Q_FALSE;
	if (log->fp != NULL) {
		fclose(log->fp);
		log->fp = NULL;
	}
	free(log);
	return Q_TRUE;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qLogSetConsole(Q_LOG *log, Q_BOOL onoff) {
	if (log == NULL) return Q_FALSE;
	log->console = onoff;
	return Q_TRUE;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qLogFlush(Q_LOG *log) {
	if (log == NULL || log->fp == NULL) return Q_FALSE;

	if (log->flush != Q_FALSE) return Q_TRUE; /* already flushed */

	if(fflush(log->fp) == 0) return Q_TRUE;
	return Q_FALSE;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qLog(Q_LOG *log, char *format, ...) {
	char szStr[1024];
	va_list arglist;
	time_t nowTime = time(NULL);

	if (log == NULL || log->fp == NULL) return Q_FALSE;

	va_start(arglist, format);
	vsprintf(szStr, format, arglist);
	va_end(arglist);

	/* console out */
	if (log->console != Q_FALSE) printf("%s(%d): %s\n", qGetTimeStr(), getpid(), szStr);

	/* check log rotate is needed*/
	if (log->nextrotate > 0 && nowTime >= log->nextrotate) {
		_realOpen(log);
	}

	/* log to file */
	if (fprintf(log->fp, "%s(%d): %s\n", qGetTimeStr(), getpid(), szStr) < 0) return Q_FALSE;

	/* check flash flag */
	if (log->flush != Q_FALSE) fflush(log->fp);

	return Q_TRUE;
}

/*
 * static functions
 */
static int _realOpen(Q_LOG *log) {
	time_t nowtime = time(NULL);

	/* generate filename */
	strftime(log->filename, sizeof(log->filename), log->nameformat, localtime(&nowtime));
	sprintf(log->logpath, "%s/%s", log->logbase, log->filename);
	if (log->fp != NULL) fclose(log->fp);
	if ((log->fp = fopen(log->logpath, "a")) == NULL) return 0;

	/* set next rotate time */
	if (log->rotateinterval > 0) {
		time_t ct = time(NULL);
		time_t dt = ct - mktime(gmtime(&ct));
		log->nextrotate = (((ct + dt) / log->rotateinterval) + 1) * log->rotateinterval - dt;
	} else {
		log->nextrotate = 0;
	}

	return 1;
}
