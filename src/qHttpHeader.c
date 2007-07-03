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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "qDecoder.h"

/**********************************************
** Static Values Definition used only internal
**********************************************/

static int _content_type_flag = 0;


/**********************************************
** Usage : qContentType(mime type);
** Do    : Print content type once.
**********************************************/
void qContentType(char *mimetype) {
	if (_content_type_flag) return;

	printf("Content-Type: %s%c%c", mimetype, 10, 10);
	_content_type_flag = 1;
}

/**********************************************
** Usage : qGetContentFlag();
** Return: If qContentType() is executed before, returns 1. Or returns 0.
** Do    : Check execution of qContentType().
**********************************************/
int qGetContentFlag(void) {
	return _content_type_flag;
}

/**********************************************
** Usage : qResetContentFlag();
** Do    : Sets the internal flag of qContentType() to the initial status.
**********************************************/
void qResetContentFlag(void) {
	_content_type_flag = 0;
}

/**********************************************
** Usage : qRedirect(url);
** Do    : Redirect page using Location response-header.
**********************************************/
void qRedirect(char *url) {
	if (qGetContentFlag() == 1) qError("qRedirect(): qRedirect() must be called before qContentType() and any stream out.");
	printf("Location: %s\n\n", url);
}

/**********************************************
** Usage : qJavaScript(...);
** Do    : Print out some JavaScript code.
**********************************************/
void qJavaScript(char *format, ...) {
	char jscode[1024];
	int status;
	va_list arglist;

	va_start(arglist, format);
	status = vsprintf(jscode, format, arglist);
	if (strlen(jscode) + 1 > sizeof(jscode) || status == EOF) qError("qJavaScript(): Message is too long or invalid.");
	va_end(arglist);

	qContentType("text/html");
	printf("<html>\n");
	printf("<head>\n");
	printf("<script language='JavaScript'>\n");
	printf("%s\n", jscode);
	printf("</script>\n");
	printf("</head>\n");
	printf("</html>\n");
}
