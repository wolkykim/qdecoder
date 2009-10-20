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
#include <string.h>
#include "qDecoder.h"

int main(void) {
	/* Parse (GET/COOKIE/POST) queries. */
	Q_ENTRY *req = qCgiRequestParse(NULL);

	const char *mode = req->getStr(req, "mode");
	const char *name = req->getStr(req, "cname");
	const char *value = req->getStr(req, "cvalue");

	if (mode == NULL) { /* View Cookie */
		qCgiResponseSetContentType(req, "text/plain");
		printf("Total %d entries\n", req->getNum(req));
		req->print(req, stdout, true);
	} else if (!strcmp(mode, "set")) { /* Set Cookie */
		if (name == NULL || value == NULL) qCgiResponseError(req, "Query not found");
		if (strlen(name) == 0) qCgiResponseError(req, "Empty cookie name can not be stored.");

		qCgiResponseSetCookie(req, name, value, 0, NULL, NULL, false);
		qCgiResponseSetContentType(req, "text/html");
		printf("Cookie('%s'='%s') entry is stored.<br>Click <a href='cookie.cgi'>here</a> to view your cookies\n", name, value);
	} else if (!strcmp(mode, "remove")) { /* Remove Cookie */
		if (name == NULL) qCgiResponseError(req, "Query not found");
		if (!strcmp(name, "")) qCgiResponseError(req, "Empty cookie name can not be removed.");

		qCgiResponseRemoveCookie(req, name, NULL, NULL, false);
		qCgiResponseSetContentType(req, "text/html");
		printf("Cookie('%s') entry is removed.<br>Click <a href='cookie.cgi'>here</a> to view your cookies\n", name);
	} else {
		qCgiResponseError(req, "Unknown mode.");
	}

	req->free(req);
	return 0;
}
