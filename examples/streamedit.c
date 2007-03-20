#include <stdio.h>
#include <stdlib.h>
#include "qDecoder.h"

#define SOURCE		"streamedit.html.in"

int main(void) {
  char *arg[2 + 1];
  char buf[2][256];
  char *name, *hobby;

  qContentType("text/html");

  if((name = qValue("name")) == NULL || (hobby = qValue("hobby")) == NULL) {
    qError("Query fetch fail.");
  }

  sprintf(buf[0], "s/${NAME}/%s/", name);
  sprintf(buf[1], "s/${HOBBY}/%s/", hobby);
  arg[0] = buf[0];
  arg[1] = buf[1];
  arg[2] = NULL;

  if(qSedFile(SOURCE, stdout, arg) == 0) qError("File(%s) not found.", SOURCE);
  return 0;
}
