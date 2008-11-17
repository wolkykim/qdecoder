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

int main(void) {
	Q_ENTRY *req = qCgiRequestParse(NULL);
	qCgiResponseSetContentType(req, "text/html");

	const char *string = qEntryGetStr(req, "string");
	const char *query = qEntryGetStr(req, "query");

	char *qlist[256], **qp;
	int queries = qArgMake((char*)query, qlist);
	int matches = qArgMatch((char*)string, qlist);

	printf("String: <b>%s</b><br>\n", string);
	printf("Query Input: <b>%s</b><br>\n", query);

	printf("<hr>\n");

	printf("qArgMake(): <b>");
	for (qp = qlist; *qp; qp++) qPrintf(1, "\"%s\" ", *qp);
	printf("</b> (%d queries)<br>\n", queries);
	printf("qArgEmprint(): ");
	int tmatches = qArgEmprint(1, (char*)string, qlist);
	printf(" (%d query matched)<br>\n", tmatches);
	printf("qArgMatch(): %d query matched<br>\n", matches);

	qArgFree(qlist);

	qEntryFree(req);
	return 0;
}
