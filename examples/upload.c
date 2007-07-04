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
	char *text;
	char *filedata, *filename, *contenttype, filepath[1024];
	int filelength;

	qContentType("text/html");
	qDecoder();

	text = qValueDefault("", "text");

	filedata   = qValue("binary");
	if ((filelength = qiValue("binary.length")) == 0) qError("Select file, please.");
	filename   = qValue("binary.filename");
	contenttype = qValue("binary.contenttype");
	sprintf(filepath, "%s/%s", BASEPATH, filename);

	if (qSaveStr(filedata, filelength, filepath, "w") < 0) {
		qError("File(%s) open fail. Please make sure CGI or directory has right permission.", filepath);
	}

	printf("You entered: <b>%s</b>\n", text);
	printf("<br><a href=\"%s\">%s</a> (%d bytes, %s) saved.", filepath, filename, filelength, contenttype);

	qFree();
	return 0;
}
