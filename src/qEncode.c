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
 * @file qEncode.c Encoding/decoding API
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "md5/md5_global.h"
#include "md5/md5.h"
#include "qDecoder.h"
#include "qInternal.h"

#ifdef __linux__
#include <iconv.h>
#endif

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
 * Q_ENTRY entries = qDecodeQueryString(NULL, req->pszQueryString, '=', '&', NULL);
 * printf("category = %s\n", qEntryGetStr(entries, "category"));
 * printf("str = %s\n", qEntryGetStr(entries, "str"));
 * printf("sort = %s\n", qEntryGetStr(entries, "sort"));
 * qEntryFree(entries);
 * @endcode
 */
Q_ENTRY *qDecodeQueryString(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count) {
	if(entry == NULL) {
		entry = qEntryInit();
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

		if(qEntryPutStr(entry, name, value, false) == true) cnt++;
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
