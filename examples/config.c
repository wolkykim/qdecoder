/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
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
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qDecoder.h"

#define CONF_FILE		"config.conf"

int main(void) {
	/* Parse queries. */
	Q_ENTRY *req = qCgiRequestParse(NULL);

	/* Parse configuration file */
	Q_ENTRY *conf = qConfigParseFile(NULL, CONF_FILE, '=');
	if(conf == NULL) qCgiResponseError(req, "Configuration file(%s) not found.", CONF_FILE);

	/* Get variable */
	const char *protocol	= conf->getStr(conf, "PROTOCOL", false);
	const char *host	= conf->getStr(conf, "HOST", false);
	int port		= conf->getInt(conf, "PORT");

	/* Print out */
	qCgiResponseSetContentType(req, "text/plain");
	printf("Protocol : %s\n", protocol);
	printf("Host     : %s\n", host);
	printf("Port     : %d\n", port);

	printf("\n--[CONFIGURATION DUMP]--\n");
	conf->print(conf, stdout, true);

	/* Deallocate parsed entries */
	conf->free(conf);
	req->free(req);

	return 0;
}
