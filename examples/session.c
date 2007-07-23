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
	char *sessionid;
	char *mode, *name, *value;
	time_t expire;

	/* fetch queries */
	expire = (time_t)qGetInt("expire");
	mode   = qGetValueNotEmpty("Mode not found", "mode");
	name   = qGetValue("name");
	value  = qGetValue("value");

	/* Set valid interval of this session */
	/* start session. this function should be called before qContentType(). */
	qSession(NULL);
	sessionid = qSessionGetID();

	/* Mose case, you don't need to set timeout but we use it here to give you a show case */
	if (expire > 0) qSessionSetTimeout(expire);

	switch (mode[0]) {
		case 's': {
			qSessionAdd(name, value);
			break;
		}
		case 'i': {
			qSessionAddInt(name, atoi(value));
			break;
		}
		case 'r': {
			qSessionRemove(name);
			break;
		}
		case 'd': {
			qSessionDestroy();
			qContentType("text/html");
			printf("Session destroied.");
			return 0;
			break;
		}
	}

	/* screen out */
	qContentType("text/html");
	qSessionPrint();

	/* save session & free allocated memories */
	qSessionFree();
	return 0;
}

