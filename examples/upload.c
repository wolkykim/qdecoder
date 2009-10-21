/*
 * Copyright 2008 The qDecoder Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qDecoder.h"

#define BASEPATH	"upload"
int main(void) {

	/* Parse queries. */
	Q_ENTRY *req = qCgiRequestParse(NULL);

	/* get queries */
	const char *text = req->getStr(req, "text", false);
	const char *filedata   = req->getStr(req, "binary", false);
	int filelength = req->getInt(req, "binary.length");
	const char *filename   = req->getStr(req, "binary.filename", false);
	const char *contenttype = req->getStr(req, "binary.contenttype", false);

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
	req->print(req, stdout, false);
	printf("\n</pre>\n");

	/* de-allocate */
	req->free(req);
	return 0;
}
