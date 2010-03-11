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

/**
 * @file qEncode.c Encoding/decoding API
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "md5/md5.h"
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Parse URL encoded query string
 *
 * @param entry		a pointer of Q_ENTRY structure. NULL can be used
 * @param query		URL encoded string
 * @param equalchar	separater of key, value pair
 * @param sepchar	separater of line
 * @param count		if count is not NULL, a number of parsed entries are stored
 *
 * @return	a pointer of Q_ENTRY if successful, otherwise returns NULL
 *
 * @code
 * cont char query = "category=love&str=%C5%A5%B5%F0%C4%DA%B4%F5&sort=asc";
 * Q_ENTRY *entry = qDecodeQueryString(NULL, req->pszQueryString, '=', '&', NULL);
 * printf("category = %s\n", entry->getStr(entry, "category"));
 * printf("str = %s\n", entry->getStr(entry, "str"));
 * printf("sort = %s\n", entry->getStr(entry, "sort"));
 * entry->free(entry);
 * @endcode
 */
Q_ENTRY *qDecodeQueryString(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count) {
	if(entry == NULL) {
		entry = qEntry();
		if(entry == NULL) return NULL;
	}

	char *newquery = NULL;
	int cnt = 0;

	if(query != NULL) newquery = strdup(query);
	while (newquery && *newquery) {
		char *value = _q_makeword(newquery, sepchar);
		char *name = qStrTrim(_q_makeword(value, equalchar));
		qDecodeUrl(name);
		qDecodeUrl(value);

		if(entry->putStr(entry, name, value, false) == true) cnt++;
		free(name);
		free(value);
	}
	if(newquery != NULL) free(newquery);
	if(count != NULL) *count = cnt;

	return entry;
}

/**
 * Encode string as URL encoded string
 *
 * @param str		a pointer of source string
 *
 * @return	a malloced string pointer of URL encoded string in case of successful, otherwise returns NULL
 *
 * @code
 * char encstr = qEncodeUrl("hello 'qDecoder' world");
 * if(encstr != NULL) free(encstr);
 * @endcode
 */
char *qEncodeUrl(const char *str) {
	char *encstr, buf[2+1];
	unsigned char c;
	int i, j;

	if (str == NULL) return NULL;
	if ((encstr = (char *)malloc((strlen(str) * 3) + 1)) == NULL) return NULL;

	for (i = j = 0; str[i]; i++) {
		c = (unsigned char)str[i];
		if ((c >= '0') && (c <= '9')) encstr[j++] = c;
		else if ((c >= 'A') && (c <= 'Z')) encstr[j++] = c;
		else if ((c >= 'a') && (c <= 'z')) encstr[j++] = c;
		else if ((c == '@') || (c == '.') || (c == '/') || (c == '\\')
		         || (c == '-') || (c == '_') || (c == ':') ) encstr[j++] = c;
		else {
			sprintf(buf, "%02x", c);
			encstr[j++] = '%';
			encstr[j++] = buf[0];
			encstr[j++] = buf[1];
		}
	}
	encstr[j] = '\0';

	return encstr;
}

/**
 * Parse URL encoded string
 *
 * @param str		a pointer of URL encoded string
 *
 * @return	a string pointer of str in case of successful, otherwise returns NULL
 *
 * @note
 * This modify str directly
 *
 * @code
 * char str[256] = "hello%20%27qDecoder%27%20world";
 * printf("Before : %s\n", str);
 * qDecodeUrl(str);
 * printf("After  : %s\n", str);
 * @endcode
 */
char *qDecodeUrl(char *str) {
	int i, j;

	if (!str) return NULL;
	for (i = j = 0; str[j]; i++, j++) {
		switch (str[j]) {
			case '+': {
				str[i] = ' ';
				break;
			}
			case '%': {
				str[i] = _q_x2c(str[j + 1], str[j + 2]);
				j += 2;
				break;
			}
			default: {
				str[i] = str[j];
				break;
			}
		}
	}
	str[i] = '\0';

	return str;
}
