/*************************************************************
** qcValue v1.0                                             **
**                                                          **
** Simple CGI utility for SSI technique on Apache web server**
**                                                          **
**  Official distribution site : ftp://ftp.hongik.com       **
**           Technical contact : nobreak@hongik.com         **
**                                                          **
**                        Developed by 'Seung-young Kim'    **
**                        Last updated at Dec 29, 1999      **
**                                                          **
**      Designed by Perfectionist for Perfectionist!!!      **
**                                                          **
**         Copyright (C) 1999 Hongik Internet, Inc.         **
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qDecoder.h"

int main(void) {
  char *name;
  char *value;

  qDecoder();
  qcDecoder();

  qContentType("text/html");
  if((name = qValue("name")) == NULL) {
    printf("Illegal usages");
    return 0;
  }
  if((value = qcValue(name)) == NULL) value = "";
  printf("%s", value);

  qcFree();
  qFree();
  return 0;
}
