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
#include "qDecoder.h"

/**
 * Get custom formmatted local time string.
 *
 * @param univtime	0 for current time, universal time for specific time
 * @param savebuf	where to string saved
 * @param bufsize	size of savebuf
 * @param format	format for strftime()
 *
 * @return	string pointer of savebuf
 *
 * @code
 *   char savebuf[30+1];
 *   qGetTimeStrf(0, savebuf, sizeof(savebuf), "%H:%M:%S"); // HH:MM:SS
 * @endcode
 */
char *qGetTimeStrf(time_t univtime, char *savebuf, int bufsize, char *format) {
	struct tm *localtm;

	if(univtime == 0) univtime = time(NULL);
	localtm = localtime(&univtime);

	strftime(savebuf, bufsize, format, localtm);

	return savebuf;
}

/**
 * Get local time string formatted by 'YYYYMMDDhhmmss'.
 *
 * @param univtime	0 for current time, universal time for specific time
 *
 * @return	string pointer which points time string formatted by 'YYYYMMDDhhmmss'
 *
 * @code
 *   printf("%s", qGetTimeStr(0));			// now
 *   printf("%s", qGetTimeStr(time(NULL)));		// same as above
 *   printf("%s", qGetTimeStr(time(NULL) + 86400));	// 1 day later
 * @endcode
 */
char *qGetTimeStr(time_t univtime) {
	static char timestr[14+1];
	return qGetTimeStrf(univtime, timestr, sizeof(timestr), "%Y%m%d%H%M%S");
}

 /**
 * Get local time string formatted like '02-Nov-2007 16:37:39 +0900'.
 *
 * @param univtime	0 for current time, universal time for specific time
 *
 * @return		string pointer which points time string.
 *
 * @code
 *   printf("%s", qGetLocaltimeStr(0));				// now
 *   printf("%s", qGetLocaltimeStr(time(NULL)));		// same as above
 *   printf("%s", qGetLocaltimeStr(time(NULL) + 86400));	// 1 day later
 * @endcode
 */
char *qGetLocaltimeStr(time_t univtime) {
	static char timestr[29+1];
	return qGetTimeStrf(univtime, timestr, sizeof(timestr), "%d-%b-%Y %H:%M:%S %z");
}

/**
 * Get gmt time string formatted like 'Wed, 11-Nov-2007 23:19:25 GMT'.
 *
 * @param univtime	0 for current time, universal time for specific time
 *
 * @return	string pointer which points gmt time string.
 *
 * @code
 *   printf("%s", qGetTimeStr(0));			// now
 *   printf("%s", qGetTimeStr(time(NULL) + 86400));	// 1 day later
 * @endcode
 */
char *qGetGmtimeStr(time_t univtime) {
	struct tm *gmtm;
	static char timestr[29+1];

	if(univtime == 0) univtime = time(NULL);
	gmtm = gmtime(&univtime);

	strftime(timestr, sizeof(timestr), "%a, %d %b %Y %H:%M:%S GMT", gmtm);
	return timestr;
}

/**
 * This parses GMT formatted time sting like 'Wed, 11-Nov-2007 23:19:25 GMT',
 * and returns as universal time.
 *
 * @param gmtstr	GMT formatted time string
 *
 * @return	universal time. in case of conversion error, returns -1.
 *
 * @code
 *   time_t t = time(NULL);
 *   char *s =  qGetGmtimeStr(t);
 *   printf("%d\n", t);
 *   printf("%s\n", s);
 *   printf("%d\n", qParseGmtimeStr(s)); // this must be same as t
 * @endcode
 */
time_t qParseGmtimeStr(char *gmtstr) {
	struct tm gmtm;
	if(strptime(gmtstr, "%a, %d %b %Y %H:%M:%S GMT", &gmtm) == NULL) return 0;

	return timegm(&gmtm);
}
