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

#define CONF_FILE		"confparser.conf"

int main(void) {
	/* Parse queries. */
	Q_ENTRY *req = qCgiRequestParse(NULL);

	/* Parse configuration file */
	Q_ENTRY *conf = qConfigParseFile(NULL, CONF_FILE, '=');
	if(conf == NULL) qCgiResponseError(req, "Configuration file(%s) not found.", CONF_FILE);

	/* Get variable */
	const char *protocol = qEntryGetStr(conf, "PROTOCOL");
	const char *host     = qEntryGetStr(conf, "HOST");
	int port       = qEntryGetInt(conf, "PORT");

	/* Print out */
	qCgiResponseSetContentType(req, "text/plain");
	printf("Protocol : %s\n", protocol);
	printf("Host     : %s\n", host);
	printf("Port     : %d\n", port);

	printf("\n--[CONFIGURATION DUMP]--\n");
	qEntryPrint(conf, stdout, true);

	/* Deallocate parsed entries */
	qEntryFree(conf);
	qEntryFree(req);

	return 0;
}
