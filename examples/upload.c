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
int main(void) {

	/* Parse queries. */
	Q_ENTRY *req = qCgiRequestParse(NULL);

	/* get queries */
	const char *text = qEntryGetStr(req, "text");
	const char *filedata   = qEntryGetStr(req, "binary");
	int filelength = qEntryGetInt(req, "binary.length");
	const char *filename   = qEntryGetStr(req, "binary.filename");
	const char *contenttype = qEntryGetStr(req, "binary.contenttype");

	/* check queries */
	if (text == NULL) qCgiResponseError(req, "Invalid usages.");
	if (filename == NULL || filelength == 0) qCgiResponseError(req, "Select file, please.");

	char  filepath[1024];
	sprintf(filepath, "%s/%s", BASEPATH, filename);

	if (qFileSave(filepath, filedata, filelength, false) < 0) {
		qCgiResponseError(req, "File(%s) open fail. Please make sure CGI or directory has right permission.", filepath);
	}

	/* result out */
	qCgiResponseSetContentType(req, "text/html");
	printf("You entered: <b>%s</b>\n", text);
	printf("<br><a href=\"%s\">%s</a> (%d bytes, %s) saved.", filepath, filename, filelength, contenttype);

	/* dump */
	printf("\n<p><hr>--[ DUMP INTERNAL DATA STRUCTURE ]--\n<pre>");
	qEntryPrint(req, stdout, false);
	printf("\n</pre>\n");

	/* de-allocate */
	qEntryFree(req);
	return 0;
}
