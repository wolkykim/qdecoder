/*************************************************************
**  Official distribution site : ftp://ftp.hongik.com       **
**           Technical contact : nobreak@hongik.com         **
**                                                          **
**                        Developed by 'Seung-young Kim'    **
**                        Last updated at Nov 2, 1999       **
**                                                          **
**         Copyright (C) 1999 Hongik Internet, Inc.         **
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "qDecoder.h"

int main(void) {
  char *value;
  
  qContentType("text/html");
  qDecoder();
  
  /* If the query is not found, the variable value will be set default.
     Also, you can use qValueDefault() or qValueNotEmpty() instead. */
  if(!(value = qValue("query"))) value = "";

  printf("You typed: <b>%s</b> \n", value);

  /* It's not necessary that you free the variable value directly
     such like free(value). qFree() will do that for you. */
  qFree();
  return 0;
}
