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
	Q_ENTRY *conf;
	char *protocol, *host;
	int port;

	/* Open configuration file */
	if (!(conf = qfDecoder(CONF_FILE))) qError("Configuration file(%s) not found.", CONF_FILE);

	/* Get variable */
	protocol = qfGetValue(conf, "PROTOCOL");
	host     = qfGetValue(conf, "HOST");
	port     = qfGetInt(conf, "PORT");

	/* Print out */
	qContentType("text/plain");
	printf("Protocol : %s\n", protocol);
	printf("Host     : %s\n", host);
	printf("Port     : %d\n", port);

	printf("\n--[CONFIGURATION DUMP]--\n");
	qfPrint(conf, stdout);

	/* Deallocate parsed entries */
	qfFree(conf);

	return 0;
}
