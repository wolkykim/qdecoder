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
 * @file qLog.c Rotating File Logging API
 */

#ifndef DISABLE_LOG

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
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
Q_LOG *qLogOpen(char *logbase, char *filenameformat, int rotateinterval, bool flush) {
	Q_LOG *log;

	/* malloc Q_LOG structure */
	if ((log = (Q_LOG *)malloc(sizeof(Q_LOG))) == NULL) return NULL;

	/* fill structure */
	qStrncpy(log->logbase, logbase, sizeof(log->logbase)-1);
	qStrncpy(log->nameformat, filenameformat, sizeof(log->nameformat)-1);
	log->fp = NULL;
	log->console = false;
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
bool qLogClose(Q_LOG *log) {
	if (log == NULL) return false;
	if (log->fp != NULL) {
		fclose(log->fp);
		log->fp = NULL;
	}
	free(log);
	return true;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
bool qLogSetConsole(Q_LOG *log, bool consoleout) {
	if (log == NULL) return false;
	log->console = consoleout;
	return true;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
bool qLogFlush(Q_LOG *log) {
	if (log == NULL || log->fp == NULL) return false;

	if (log->flush == true) return true; /* already flushed */

	if(fflush(log->fp) == 0) return true;
	return false;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
bool qLog(Q_LOG *log, char *format, ...) {
	char buf[1024];
	va_list arglist;
	time_t nowTime = time(NULL);

	if (log == NULL || log->fp == NULL) return false;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	/* console out */
	if (log->console == true) printf("%s\n", buf);

	/* check log rotate is needed*/
	if (log->nextrotate > 0 && nowTime >= log->nextrotate) {
		_realOpen(log);
	}

	/* log to file */
	if (fprintf(log->fp, "%s\n", buf) < 0) return false;

	/* check flash flag */
	if (log->flush == true) fflush(log->fp);

	return true;
}

/*
 * static functions
 */
static int _realOpen(Q_LOG *log) {
	time_t nowtime = time(NULL);

	/* generate filename */
	strftime(log->filename, sizeof(log->filename), log->nameformat, localtime(&nowtime));
	snprintf(log->logpath, sizeof(log->logpath), "%s/%s", log->logbase, log->filename);
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

#endif /* DISABLE_LOG */
