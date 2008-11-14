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
#include "qDecoder.h"

int main(void) {
	Q_ENTRY *req = qCgiRequestParse(NULL);

	/* fetch queries */
	time_t expire = (time_t)qEntryGetInt(req, "expire");
	const char *mode = qEntryGetStr(req, "mode");
	const char *name   = qEntryGetStr(req, "name");
	const char *value  = qEntryGetStr(req, "value");

	/* start session. */
	Q_ENTRY *sess = qSessionInit(req, NULL);

	/* Mose case, you don't need to set timeout. this is just example */
	if (expire > 0) qSessionSetTimeout(sess, expire);

	switch (mode[0]) {
		case 's': {
			qEntryPutStr(sess, name, value, true);
			break;
		}
		case 'i': {
			qEntryPutInt(sess, name, atoi(value), true);
			break;
		}
		case 'r': {
			qEntryRemove(sess, name);
			break;
		}
		case 'd': {
			qSessionDestroy(sess);
			qCgiResponseSetContentType(req, "text/html");
			printf("Session destroied.\n");
			return 0;
			break;
		}
	}
	/* screen out */
	qCgiResponseSetContentType(req, "text/plain");
	qEntryPrint(sess, stdout, true);

	/* save session & free allocated memories */
	qSessionSave(sess);
	qEntryFree(sess);
	qEntryFree(req);
	return 0;
}
