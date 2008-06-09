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
#define BASEPATH	"upload"
#define TMPPATH		"tmp"

int main(void) {
	int i;

	/* parse queries. */
	Q_ENTRY *req = qCgiRequestParseOption(NULL, true, TMPPATH, (1 * 60 * 60));
	if(req == NULL) qCgiResponseError(req, "Can't set option.");
	req = qCgiRequestParse(req);

	/* get queries */
	const char *text = qEntryGetStr(req, "text");
	if (text == NULL) qCgiResponseError(req, "Invalid usages.");

	/* result out */
	qCgiResponseSetContentType(req, "text/html");
	printf("You entered: <b>%s</b>\n", text);

	for (i = 1; i <= 3; i++) {
		int length =  qEntryGetIntf(req, "binary%d.length", i);
		if (length > 0) {
			const char *filename = qEntryGetStrf(req, "binary%d.filename", i);
			const char *contenttype = qEntryGetStrf(req, "binary%d.contenttype", i);
			const char *savepath = qEntryGetStrf(req, "binary%d.savepath", i);

			char newpath[1024];
			sprintf(newpath, "%s/%s", BASEPATH, filename);

			if (rename(savepath, newpath) == -1) qCgiResponseError(req, "Can't move uploaded file %s to %s", savepath, newpath);
			printf("<br>File %d : <a href=\"%s\">%s</a> (%d bytes, %s) saved.", i, newpath, filename, length, contenttype);
		}
	}

	/* dump */
	printf("\n<p><hr>--[ DUMP INTERNAL DATA STRUCTURE ]--\n<pre>");
	qEntryPrint(req, stdout, true);
	printf("\n</pre>\n");

	/* de-allocate */
	qEntryFree(req);
	return 0;
}
