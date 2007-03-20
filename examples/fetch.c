#include <stdio.h>
#include <stdlib.h>
#include "qDecoder.h"

void main(void) {
  qContentType("text/html");
  qDecoder();
  printf("You typed: <b>%s</b> \n", qValue("text"));
  qFree();
}
