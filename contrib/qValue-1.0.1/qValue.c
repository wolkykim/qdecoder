/*************************************************************
** qValue v1.0.1                                            **
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

int main(int argc, char *argv[]) {
  char *value;

  if(argc != 2) {
    printf("Illegal usages");
    return 0;
  }

  qDecoder();

  if((value = qValue(argv[1])) == NULL) value = "";
  printf("%s", value);

  qFree();
  return 0;
}
