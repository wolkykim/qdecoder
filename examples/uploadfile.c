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
#define TMPPATH		"tmp"

int main(void) {
	int i;

	/* parse queries. */
	Q_ENTRY *req = qCgiRequestParseOption(true, TMPPATH, (1 * 60 * 60));
	if(req == NULL) qCgiResponseError(req, "Can't set option.");
	req = qCgiRequestParse(req);

	/* get queries */
	const char *text = req->getStr(req, "text");
	if (text == NULL) qCgiResponseError(req, "Invalid usages.");

	/* result out */
	qCgiResponseSetContentType(req, "text/html");
	printf("You entered: <b>%s</b>\n", text);

	for (i = 1; i <= 3; i++) {
		int length =  req->getIntf(req, "binary%d.length", i);
		if (length > 0) {
			const char *filename = req->getStrf(req, "binary%d.filename", i);
			const char *contenttype = req->getStrf(req, "binary%d.contenttype", i);
			const char *savepath = req->getStrf(req, "binary%d.savepath", i);

			char newpath[1024];
			sprintf(newpath, "%s/%s", BASEPATH, filename);

			if (rename(savepath, newpath) == -1) qCgiResponseError(req, "Can't move uploaded file %s to %s", savepath, newpath);
			printf("<br>File %d : <a href=\"%s\">%s</a> (%d bytes, %s) saved.", i, newpath, filename, length, contenttype);
		}
	}

	/* dump */
	printf("\n<p><hr>--[ DUMP INTERNAL DATA STRUCTURE ]--\n<pre>");
	req->print(req, stdout, true);
	printf("\n</pre>\n");

	/* de-allocate */
	req->free(req);
	return 0;
}
