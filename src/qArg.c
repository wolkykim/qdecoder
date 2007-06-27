/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

/**
 * Pattern Scanning Functions
 *
 * Plays a similar function to the AWK command of UNIX systems.
 *
 * @file qArg.c
 * @note
 *   @code
 *     [Sample /etc/passwd]
 *     shpark:x:1008:1000:Sang-hyun Park:/home/shpark:/bin/csh
 *     teamwork:x:1011:1000:Seung-young Kim:/home/teamwork:/bin/csh
 *     kikuchi:x:1015:2000:KIKUCHI:/home/kikuchi:/bin/csh
 *   @endcode
 *   @code
 *     char array[7][1024];
 *     qAwkOpen("/etc/passwd", ':');
 *     for( ; qAwkNext(array) > 0; ) printf("ID=%s, Name=%s", array[0], array[5]);
 *     qAwkClose();
 *   @endcode
 */

#include "qDecoder.h"
#include "qInternal.h"

/**********************************************
** Usage : qArgMake(query string, token array);
** Return: Number of parsed tokens.
** Do    : This function parses query string and stores
**         each query into token array.
**
** ex) char *query="I am a \"pretty girl\".", *qlist[MAX_TOKENS];
**     int queries;
**     queries = qArgMake(query, qlist);
**********************************************/
int qArgMake(char *str, char **qlist) {
	char *query, *sp, *qp;
	int argc;

	query = sp = qRemoveSpace(strdup(str));

	for (argc = 0; *sp != '\0';) {
		switch (*sp) {
			case ' ': { /* skip space */
				sp++;
				break;
			}
			case '"': { /* Double quotation arounded string */
				qlist[argc] = qp = (char *)malloc(strlen(query) +  1);
				for (sp++; *sp != '\0'; sp++, qp++) {
					if (*sp == '"') {
						sp++;
						break;
					}
					*qp = *sp;
				}
				*qp = '\0';
				if (strlen(qlist[argc]) > 0) argc++;
				break;
			}
			default: {
				qlist[argc] = qp = (char *)malloc(strlen(query) +  1);
				for (; *sp != '\0'; sp++, qp++) {
					if (*sp == ' ' || *sp == '"') break;
					*qp = *sp;
				}
				*qp = '\0';
				argc++;
				break;
			}
		}
	}
	qlist[argc] = NULL;
	free(query);
	return argc;
}

/**********************************************
** Usage : qArgMatch(target string, token array);
** Return: Number of token matched. (Do not allow duplicated token matches)
**         This value is not equal to a value of qArgEmprint()
**********************************************/
int qArgMatch(char *str, char **qlist) {
	char **qp;
	int i;

	/* To keep the value i over zero */
	if (!*qlist) return 0;

	for (qp = qlist, i = 0; *qp != NULL; qp++) if (qStristr(str, *qp)) i++;

	return i;
}

/**********************************************
** Usage : qArgPrint(pointer of token array);
** Return: Amount of tokens.
** Do    : Print all parsed tokens for debugging.
**********************************************/
int qArgPrint(char **qlist) {
	char **qp;
	int amount;

	qContentType("text/html");

	for (amount = 0, qp = qlist; *qp; amount++, qp++) {
		printf("'%s' (%d bytes)<br>\n" , *qp, strlen(*qp));
	}

	return amount;
}

/**********************************************
** Usage : qArgEmprint(mode, target string, token array);
** Return: Number of matches. (Allow duplicated token matches)
**         This value is not equal to a value of qArgMatch().
** Do    : Make matched token bold in target string.
**********************************************/
int qArgEmprint(int mode, char *str, char **qlist) {
	char *sp, *freestr, *buf, *bp, *op;
	int  i, j, flag, matches;

	if (!*qlist) {
		qPuts(mode, str);
		return 0;
	}

	/* Set character pointer */
	op = str;
	sp = freestr = strdup(str);
	qStrupr(sp);

	if ((bp = buf = (char *)malloc(strlen(str) + 1)) == NULL) qError("Memory allocation fail.");

	for (matches = 0; *sp != '\0';) {
		for (i = 0, flag = 0; qlist[i] != NULL; i++) {
			if (!qStrincmp(sp, qlist[i], strlen(qlist[i]))) {
				*bp = '\0'; /* Mark string end */
				qPuts(mode, buf); /* flash buffer */
				bp = buf; /* reset buffer pointer */
				printf("<b>");
				for (j = 1; j <= (int)strlen(qlist[i]); j++) {
					printf("%c", *op++);
					sp++;
				}
				printf("</b>");
				flag = 1;
				matches++;
				break;
			}
		}
		if (flag == 0) {
			*bp++ = *op++;
			sp++;
		}
	}
	*bp = '\0'; /* Mark string end */
	qPuts(mode, buf); /* Flash buffer */

	free(buf);
	free(freestr);

	return matches;
}

/**********************************************
** Usage : qArgFree(pointer of token array);
** Do    : Free malloced token array.
**********************************************/
void qArgFree(char **qlist) {
	char **qp;
	for (qp = qlist; *qp; qp++) free(*qp);
	*qlist = NULL;
}
