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
 * @file qTime.c Date and Time Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Get custom formmatted local time string.
 *
 * @param buf		save buffer
 * @param size		buffer size
 * @param utctime	0 for current time, universal time for specific time
 * @param format	format for strftime()
 *
 * @return		string pointer of buf
 *
 * @code
 *   char *timestr = qTimeGetLocalStrf(0, "%H:%M:%S"); // HH:MM:SS
 *   free(timestr);
 *   char *timestr = qTimeGetLocalStrf(0, "%Y%m%d%H%M%S"); // YYMMDDhhmmss
 *   free(timestr);
 * @endcode
 */
char *qTimeGetLocalStrf(char *buf, int size, time_t utctime, const char *format) {
	if(utctime == 0) utctime = time(NULL);
	struct tm *localtm = localtime(&utctime);

	strftime(buf, size, format, localtm);

	return buf;
}

 /**
 * Get local time string formatted like '02-Nov-2007 16:37:39 +0900'.
 *
 * @param utctime	0 for current time, universal time for specific time
 *
 * @return		mallocked string pointer of time string
 *
 * @code
 *   char *timestr;
 *   timestr = qGetLocaltimeStr(0);			// now
 *   free(timestr);
 *   timestr = qGetLocaltimeStr(time(NULL));		// same as above
 *   free(timestr);
 *   timestr = qGetLocaltimeStr(time(NULL) - 86400));	// 1 day before
 *   free(timestr);
 * @endcode
 */
char *qTimeGetLocalStr(time_t utctime) {
	int size = sizeof(char) * (CONST_STRLEN("00-00-0000 00:00:00 +0000") + 1);
	char *timestr = (char *)malloc(size);
	qTimeGetLocalStrf(timestr, size, utctime, "%d-%b-%Y %H:%M:%S %z");
	return timestr;
}

/**
 * Get custom formmatted GMT time string.
 *
 * @param buf		save buffer
 * @param size		buffer size
 * @param utctime	0 for current time, universal time for specific time
 * @param format	format for strftime()
 *
 * @return		string pointer of buf
 *
 * @code
 *   char timestr[8+1];
 *   qTimeGetGmtStrf(buf, sizeof(buf), 0, "%H:%M:%S"); // HH:MM:SS
 * @endcode
 */
char *qTimeGetGmtStrf(char *buf, int size, time_t utctime, const char *format) {
	if(utctime == 0) utctime = time(NULL);
	struct tm *gmtm = gmtime(&utctime);

	strftime(buf, size, format, gmtm);
	return buf;
}

/**
 * Get GMT time string formatted like 'Wed, 11-Nov-2007 23:19:25 GMT'.
 *
 * @param utctime	0 for current time, universal time for specific time
 *
 * @return		malloced string pointer which points GMT time string.
 *
 * @code
 *   printf("%s", qGetTimeStr(0));			// now
 *   printf("%s", qGetTimeStr(time(NULL) + 86400));	// 1 day later
 * @endcode
 */
char *qTimeGetGmtStr(time_t utctime) {
	int size = sizeof(char) * (CONST_STRLEN("Mon, 00-Jan-0000 00:00:00 GMT") + 1);
	char *timestr = (char*)malloc(size);
	qTimeGetGmtStrf(timestr, size, utctime, "%a, %d %b %Y %H:%M:%S GMT");
	return timestr;
}

/**
 * This parses GMT/Timezone(+/-) formatted time sting like
 * 'Sun, 04 May 2008 18:50:39 GMT', 'Mon, 05 May 2008 03:50:39 +0900'
 * and returns as universal time.
 *
 * @param gmtstr	GMT/Timezone(+/-) formatted time string
 *
 * @return	universal time(UTC). in case of conversion error, returns -1.
 *
 * @code
 *   time_t t = time(NULL);
 *   char *s =  qTimeGetGmtStr(t);
 *   printf("%d\n", t);
 *   printf("%s\n", s);
 *   printf("%d\n", qTimeParseGmtStr(s)); // this must be same as t
 *   free(s);
 * @endcode
 */
extern char *strptime(const char *, const char *r, struct tm *);
time_t qTimeParseGmtStr(const char *gmtstr) {
	struct tm gmtm;
	if(strptime(gmtstr, "%a, %d %b %Y %H:%M:%S", &gmtm) == NULL) return 0;
	time_t utc = timegm(&gmtm);
	if(utc < 0) return -1;

	// parse timezone
	char *p;
	if((p = strstr(gmtstr, "+")) != NULL) {
		utc -= ((atoi(p + 1) / 100) * 60 * 60);
		if(utc < 0) return -1;
	} else if((p = strstr(gmtstr, "-")) != NULL) {
		utc += ((atoi(p + 1) / 100) * 60 * 60);
	}

	return utc;
}
