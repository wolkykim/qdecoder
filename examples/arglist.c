/*************************************************************
**  Official distribution site : ftp://ftp.hongik.com       **
**           Technical contact : nobreak@hongik.com         **
**                                                          **
**                        Developed by 'Seung-young Kim'    **
**                        Last updated at Dec 29, 1999      **
**                                                          **
**         Copyright (C) 1999 Hongik Internet, Inc.         **
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
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
