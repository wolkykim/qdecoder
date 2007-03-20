/************************************************************************
qDecoder - C/C++ CGI Library                      http://www.qDecoder.org

Copyright (C) 2001 The qDecoder Project.
Copyright (C) 1999,2000 Hongik Internet, Inc.
Copyright (C) 1998 Nobreak Technologies, Inc.
Copyright (C) 1996,1997 Seung-young Kim.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Copyright Disclaimer:
  Hongik Internet, Inc., hereby disclaims all copyright interest.
  President, Christopher Roh, 6 April 2000

  Nobreak Technologies, Inc., hereby disclaims all copyright interest.
  President, Yoon Cho, 6 April 2000

  Seung-young Kim, hereby disclaims all copyright interest.
  Author, Seung-young Kim, 6 April 2000

Author:
  Seung-young Kim <nobreak@hongik.com>
  Hongik Internet, Inc. 17th Fl., Marine Center Bldg.,
  51, Sogong-dong, Jung-gu, Seoul, 100-070, Korea.
  Tel: +82-2-753-2553, Fax: +82-2-753-1302
************************************************************************/

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

  for(argc = 0; *sp != '\0';) {
    switch(*sp) {
      case ' ': { /* skip space */
      	sp++;
        break;
      }
      case '"': { /* Double quotation arounded string */
      	qlist[argc] = qp = (char *)malloc(strlen(query) +  1);
      	for(sp++; *sp != '\0'; sp++, qp++) {
          if(*sp == '"') { sp++; break; }
      	  *qp = *sp;
      	}
      	*qp = '\0';
      	if(strlen(qlist[argc]) > 0) argc++;
      	break;
      }
      default: {
      	qlist[argc] = qp = (char *)malloc(strlen(query) +  1);
      	for(; *sp != '\0'; sp++, qp++) {
          if(*sp == ' ' || *sp == '"') break;
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
  if(!*qlist) return 0;

  for(qp = qlist, i = 0; *qp != NULL; qp++) if(qStristr(str, *qp)) i++;

  return i;
}

/**********************************************
** Usage : qArgPrint(pointer of token array);
** Do    : Print all parsed tokens for debugging.
**********************************************/
void qArgPrint(char **qlist) {
  char **qp;

  qContentType("text/html");

  for(qp = qlist; *qp; qp++) {
    printf("'%s' (%d bytes)<br>\n" , *qp, strlen(*qp));
  }
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

  if(!*qlist) {
    qPuts(mode, str);
    return 0;
  }

  /* Set character pointer */
  op = str;
  sp = freestr = strdup(str);
  qStrupr(sp);

  if((bp = buf = (char *)malloc(strlen(str) + 1)) == NULL) qError("Memory allocation fail.");

  for(matches = 0; *sp != '\0';) {
    for(i = 0, flag = 0; qlist[i] != NULL; i++) {
      if(!qStrincmp(sp, qlist[i], strlen(qlist[i]))) {
        *bp = '\0'; /* Mark string end */
        qPuts(mode, buf); /* flash buffer */
        bp = buf; /* reset buffer pointer */
      	printf("<b>");
        for(j = 1; j <= (int)strlen(qlist[i]); j++) {
          printf("%c", *op++);
          sp++;
        }
      	printf("</b>");
      	flag = 1;
      	matches++;
      	break;
      }
    }
    if(flag == 0) {
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
  for(qp = qlist; *qp; qp++) free(*qp);
  *qlist = NULL;
}