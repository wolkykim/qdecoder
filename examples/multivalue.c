#include <stdio.h>
#include <stdlib.h>
#include "qDecoder.h"

int main(void) {
  char *list;

  qContentType("text/html");
  qDecoder();

  printf("You ordered ");
  if((list = qValueFirst("checklist")) == NULL) qError("You should check it.");
  for(; list; list = qValueNext()) {
    printf("<b>%s</b> \n", list);
  }

  qFree();
  return 0;
}
