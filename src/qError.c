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
 * @file qError.c Error Handling API
 */

#ifndef WITHOUT_CGISUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "qDecoder.h"

/**********************************************
** Static Values Definition used only internal
**********************************************/

static char *_error_contact_info = NULL;
static char *_error_log_file = NULL;


/**********************************************
** Usage : qError(format, arg);
** Do    : Print error message.
**********************************************/
void qError(char *format, ...) {
	static int cnt = 0;
	char buf[1024];
	int logstatus;
	va_list arglist;

	if (cnt != 0) exit(1);
	cnt++;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf)-1, format, arglist);
	buf[sizeof(buf)-1] = '\0';
	va_end(arglist);

	logstatus = 0;
	if (_error_log_file != NULL) {
		FILE *fp;

		if ((fp = fopen(_error_log_file, "at")) == NULL) logstatus = -1;
		else {
			char *http_user_agent, *remote_host;
			struct tm *time;

			time = qGetTimeStructure(0);

			if ((http_user_agent = getenv("HTTP_USER_AGENT")) == NULL) http_user_agent = "null";
			if ((remote_host     = getenv("REMOTE_HOST"))     == NULL) {
				/* Patch for Apache 1.3 */
				if ((remote_host     = getenv("REMOTE_ADDR"))   == NULL) remote_host = "null";
			}

			fprintf(fp, "%04d/%02d/%02d(%02d:%02d) : '%s' from %s (%s)\n",
			        time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min,
			        buf, remote_host, http_user_agent);

			fclose(fp);
			logstatus = 1;
		}
	}

	if (getenv("REMOTE_ADDR") == NULL)  {
		printf("Error: %s\n", buf);
	} else {
		qContentType("text/html");

		if (_error_contact_info != NULL) {
			strcat(buf, _error_contact_info);
		}
		if (logstatus == -1) strcat(buf, " [ERROR LOGGING FAIL]");

		printf("<html>\n");
		printf("<head>\n");
		printf("<title>Error: %s</title>\n", buf);
		printf("<script language='JavaScript'>\n");
		printf("  alert(\"%s\");\n", buf);
		printf("  history.back();\n");
		printf("</script>\n");
		printf("</head>\n");
		printf("</html>\n");
	}

	qReset();
	exit(1);
}

/**********************************************
** Usage : qErrorLog(log filename);
** Do    : Turn Error log on.
**********************************************/
void qErrorLog(char *file) {
	_error_log_file = file;
}

/**********************************************
** Usage : qErrorContact(message);
** Do    : Error contact information.
**********************************************/
void qErrorContact(char *msg) {
	_error_contact_info = msg;
}

#endif /* WITHOUT_CGISUPPORT */
