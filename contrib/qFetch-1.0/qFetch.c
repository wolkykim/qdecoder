/*************************************************************
** qFetch v1.0                                              **
**                                                          **
** Simple CGI utility for SSI technique on Apache web server**
**                                                          **
**  Official distribution site : ftp://ftp.hongik.com       **
**           Technical contact : nobreak@hongik.com         **
**                                                          **
**                        Developed by 'Seung-young Kim'    **
**                        Last updated at Nov 26, 1999      **
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
  char *data;

  if(argc != 2) {
    printf("Illegal usages");
    return 0;
  }
  if((data = qValue(argv[1])) == NULL) data = "";
  printf("%s", data);

  qFree();
  return 0;
}
