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

	qDecoderInit(true, TMPPATH, (1 * 60 * 60));	// MUST BE called at the first line of main()
	qDecoder();					// CAN NOT BE OMITTED in case of progress uploading.

	qContentType("text/html");

	printf("You entered: <b>%s</b>\n", qValueDefault("", "text"));

	for (i = 1; i <= 3; i++) {
		char *filename, *contenttype, *savepath;
		int length;

		char newpath[1024];

		if ((length = qiValue("binary%d.length", i)) > 0) {
			filename = qValue("binary%d.filename", i);
			contenttype = qValue("binary%d.contenttype", i);
			savepath = qValue("binary%d.savepath", i);

			sprintf(newpath, "%s/%s", BASEPATH, filename);
			if (rename(savepath, newpath) == -1) qError("Can not move uploaded file. %s-%s", savepath, newpath);

			printf("<br><a href=\"%s\">%s</a> (%d bytes, %s) saved.", newpath, filename, length, contenttype);
		}
	}

	printf("\n<p><hr>--[ DUMP INTERNAL DATA STRUCTURE ]--\n<pre>");
	qPrint();
	printf("\n</pre>\n");
	qFree();
	return 0;
}
