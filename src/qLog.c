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
#include "qInternal.h"

static bool _realOpen(Q_LOG *log);

/**
 * Open ratating-log file
 *
 * @param filepathfmt		filename format. formatting argument is same as strftime()
 * @param rotateinterval	rotating interval seconds, set 0 to disable rotation
 * @param flush			set to true if you want to flush everytime logging. false for buffered logging
 *
 * @return		a pointer of Q_LOG structure
 *
 * @note
 * rotateinterval is not relative time. If you set it to 3600, log file will be rotated at every hour.
 * And filenameformat is same as strftime(). So If you want to log with hourly rotating, filenameformat
 * must be defined including hour format like "/somepath/xxx-%Y%m%d%H.log". You can set it to
 * "/somepath/xxx-%H.log" for daily overrided log file.
 *
 * @code
 *   Q_LOG *log = qLogOpen("/tmp/qdecoder-%Y%m%d.err", 86400, false);
 *   qLogClose(log);
 * @endcode
 */
Q_LOG *qLogOpen(const char *filepathfmt, int rotateinterval, bool flush) {
	Q_LOG *log;

	/* malloc Q_LOG structure */
	if ((log = (Q_LOG *)malloc(sizeof(Q_LOG))) == NULL) return NULL;

	/* fill structure */
	memset((void *)(log), 0, sizeof(Q_LOG));
	qStrCpy(log->filepathfmt, sizeof(log->filepathfmt), filepathfmt, sizeof(log->filepathfmt));
	if(rotateinterval > 0) log->rotateinterval = rotateinterval;
	log->flush = flush;

	if (_realOpen(log) == false) {
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
 * Duplicate log string into other stream
 *
 * @param log		a pointer of Q_LOG
 * @param fp		logging messages will be printed out into this stream. set NULL to disable.
 * @param flush		set to true if you want to flush everytime duplicating.
 *
 * @return		true if successful, otherewise returns false
 *
 * @code
 *   qLogDuplicate(log, stdout, true);	// enable console out with flushing
 *   qLogDuplicate(log, stderr, false);	// enable console out
 *   qLogDuplicate(log, NULL, false);	// disable console out (default)
 * @endcode
 */
bool qLogDuplicate(Q_LOG *log, FILE *outfp, bool flush) {
	if (log == NULL) return false;
	log->outfp = outfp;
	log->outflush = flush;
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
	if (log == NULL) return false;

	if (log->fp != NULL &&log->flush == false) fflush(log->fp);
	if (log->outfp != NULL && log->outflush == false) fflush(log->outfp);

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

	/* duplicate stream */
	if (log->outfp != NULL) {
		fprintf(log->outfp, "%s\n", buf);
		if(log->outflush == true) fflush(log->outfp);
	}

	/* check log rotate is needed*/
	if (log->nextrotate > 0 && nowTime >= log->nextrotate) {
		_realOpen(log);
	}

	/* log to file */
	if (fprintf(log->fp, "%s\n", buf) < 0) return false;
	if (log->flush == true) fflush(log->fp);

	return true;
}

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
/////////////////////////////////////////////////////////////////////////

static bool _realOpen(Q_LOG *log) {
	const time_t nowtime = time(NULL);

	/* generate filename */
	char newfilepath[PATH_MAX];
	strftime(newfilepath, sizeof(newfilepath), log->filepathfmt, localtime(&nowtime));

	/* open or re-open log file */
	if (log->fp == NULL) {
		if ((log->fp = fopen(newfilepath, "a")) == NULL) {
			DEBUG("_realOpen: Can't open log file '%s'.", newfilepath);
			return false;
		}
		qStrCpy(log->filepath, sizeof(log->filepath), newfilepath, sizeof(log->filepath));
	} else if(strcmp(log->filepath, newfilepath)) { /* have opened stream, only reopen if new filename is different with existing one */
		FILE *newfp = fopen(newfilepath, "a");
		if (newfp != NULL) {
			fclose(log->fp);
			log->fp = newfp;
			qStrCpy(log->filepath, sizeof(log->filepath), newfilepath, sizeof(log->filepath));
		} else {
			DEBUG("_realOpen: Can't open log file '%s' for rotating.", newfilepath);
		}
	} else {
		DEBUG("_realOpen: skip re-opening log file.");
	}

	/* set next rotate time */
	if (log->rotateinterval > 0) {
		time_t ct = time(NULL);
		time_t dt = ct - mktime(gmtime(&ct));
		log->nextrotate = (((ct + dt) / log->rotateinterval) + 1) * log->rotateinterval - dt;
	} else {
		log->nextrotate = 0;
	}

	return true;
}
