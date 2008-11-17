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
 * @file qLog.c Rotating File Logger API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include "qDecoder.h"

static int _realOpen(Q_LOG *log);

/**
 * Open ratating-log file
 *
 * @param logbase		log directory
 * @param filenameformat	filename format. formatting argument is same as strftime()
 * @param rotateinterval	rotating interval seconds
 * @param flush			set to true if you want to flush everytime logging. false for buffered logging
 *
 * @return		a pointer of Q_LOG structure
 *
 * @note
 * rotateinterval is not relative time. If you set it to 3600, log file will be rotated at every hour.
 * And filenameformat is same as strftime(). So If you want to log with hourly rotating, filenameformat
 * must be defined at least hourly format, such like "xxx-%Y%m%d%H.log". You can set it to "xxx-%H.log"
 * for daily overwriting.
 *
 * @code
 *   Q_LOG *log = qLogOpen("/tmp", "qdecoder-%Y%m%d.err", 86400, false);
 *   qLogClose(log);
 * @endcode
 */
Q_LOG *qLogOpen(const char *logbase, const char *filenameformat, int rotateinterval, bool flush) {
	Q_LOG *log;

	/* malloc Q_LOG structure */
	if ((log = (Q_LOG *)malloc(sizeof(Q_LOG))) == NULL) return NULL;

	/* fill structure */
	qStrCpy(log->logbase, sizeof(log->logbase), logbase, sizeof(log->logbase));
	qStrCpy(log->nameformat, sizeof(log->nameformat), filenameformat, sizeof(log->nameformat));
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
 * Close ratating-log file
 *
 * @param log		a pointer of Q_LOG
 *
 * @return		true if successful, otherewise returns false
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
 * Set screen out
 *
 * @param log		a pointer of Q_LOG
 * @param consoleout	if set it to true, logging messages will be printed out into stderr
 *
 * @return		true if successful, otherewise returns false
 */
bool qLogSetConsole(Q_LOG *log, bool consoleout) {
	if (log == NULL) return false;
	log->console = consoleout;
	return true;
}

/**
 * Flush buffered log
 *
 * @param log		a pointer of Q_LOG
 *
 * @return		true if successful, otherewise returns false
 */
bool qLogFlush(Q_LOG *log) {
	if (log == NULL || log->fp == NULL) return false;

	if (log->flush == true) return true; /* already flushed */

	if(fflush(log->fp) == 0) return true;
	return false;
}

/**
 * Log messages
 *
 * @param log		a pointer of Q_LOG
 * @param format	messages format
 *
 * @return		true if successful, otherewise returns false
 */
bool qLog(Q_LOG *log, const char *format, ...) {
	char buf[1024];
	va_list arglist;
	time_t nowTime = time(NULL);

	if (log == NULL || log->fp == NULL) return false;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	/* console out */
	if (log->console == true) fprintf(stderr, "%s\n", buf);

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

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////

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
