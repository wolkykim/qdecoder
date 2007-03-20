#include <stdio.h>
#include <stdlib.h>
#include "qDecoder.h"

int main(void) {
  FILE *fp;
  char *filedata, *filename;
  int filelength, i;

  qContentType("text/html");
  qDecoder();

  filedata   = qValue("binary");
  if((filelength = qiValue("binary.length")) == 0) qError("Select file, please.");
  filename   = qValue("binary.filename");

  if((fp = fopen(filename, "wb")) == NULL) qError("File open(wb) fail. Please make sure CGI(6755) or Directory(777) has right permission.");
  for(i = filelength; i > 0; i--) fprintf(fp, "%c", *(filedata++));
  fclose(fp);

  printf("<a href=\"%s\">%s</a> (%d bytes) saved.", filename, filename, filelength);
  qFree();
  return 0;
}
