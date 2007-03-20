/************************************************************************
qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org

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
  Seung-young Kim <wolkykim(at)ziom.co.kr>
************************************************************************/

#include "qDecoder.h"

int main(void) {
  char *string, *query;
  char *qlist[256], **qp;
  int queries, tmatches, matches;

  qContentType("text/html");
  qDecoder();

  string = qValueNotEmpty("Type any string.", "string");
  query = qValueNotEmpty("Type any query.", "query");

  queries = qArgMake(query, qlist);
  matches = qArgMatch(string, qlist);

  printf("String: <b>%s</b><br>\n", string);
  printf("Query Input: <b>%s</b><br>\n", query);

  printf("<hr>\n");

  printf("qArgMake(): <b>");
  for(qp = qlist; *qp; qp++) qPrintf(1, "\"%s\" ", *qp);
  printf("</b> (%d queries)<br>\n", queries);
  printf("qArgEmprint(): "); tmatches = qArgEmprint(1, string, qlist); printf(" (%d query matched)<br>\n", tmatches);
  printf("qArgMatch(): %d query matched<br>\n", matches);

  qArgFree(qlist);
  qFree();
  return 0;
}

