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
 * @file qDownload.c HTTP Download Handling API
 */

#ifndef DISABLE_CGISUPPORT

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"

/**
 * Force to send(download) file to client.
 *
 * Do not call qContentType() before. This will force to set mime-type to application/octet-stream.
 * So user can see downloading dialogue box even though you send text file.
 *
 * @param filename	file to send
 *
 * @return	 the number of bytes sent. otherwise(file not found) returns -1.
 */
int qDownload(char *filename) {
	return qDownloadMime(filename, "application/octet-stream");
}

/**
 * Force to send(download) file to client in accordance with given mime type.
 *
 * Do not call qContentType() before.
 *
 * The results of this function are the same as those acquired
 * when the corresponding files are directly linked to the Web.
 * But this is especially useful in preprocessing files to be downloaded
 * only with user certification and in enabling downloading those files,
 * which cannot be opned on the Web, only through specific programs.
 *
 * When mime is 'application/octet-stream', it isthe same as qDownload().
 * And since processes are executed until streams are terminated,
 * this is a file that can be linked on the Web.
 *
 * When it is to be used as preprocessing for the downloading count,
 * it is better to utilize qRedirect().
 *
 * @param filename	file to send
 * @param mime		mimefile to send
 *
 * @return	 the number of bytes sent. otherwise(file not found) returns -1.
 */
int qDownloadMime(char *filename, char *mime) {
	char *file, *c, *disposition;
	int sent;

	if (qGetContentFlag() == 1) qError("qDownloadMime(): qDownloadMime() must be called before qContentType() and any stream out.");

	if (mime == NULL) mime = "application/octet-stream";

	if (filename == NULL) qError("qDownload(): Null pointer can not be used.");
	if (qCheckFile(filename) == false) return -1;

	file = strdup(filename);

	/* Fetch filename in string which include directory name */
	for (c = file + strlen(file) - 1; c >= file && !(*c == '/' || *c == '\\'); c--);
	for (; c >= file; c--) *c = ' ';
	qRemoveSpace(file);

	if (!strcmp(mime, "application/octet-stream")) disposition = "attachment";
	else disposition = "inline";
	printf("Content-Disposition: %s;filename=\"%s\"\n", disposition, file);
	printf("Content-Transfer-Encoding: binary\n");
	printf("Accept-Ranges: bytes\n");
	printf("Content-Length: %ld\n", qFileSize(filename));
	printf("Connection: close\n");
	printf("Content-Type: %s\n", mime);
	printf("\n");
	free(file);

	sent = qCatFile(filename);
	return sent;
}

#endif /* DISABLE_CGISUPPORT */
