/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

#include "qDecoder.h"
#define BASEPATH	"upload"

int main(void) {
  char *text;
  char *filedata, *filename, *contenttype, filepath[1024];
  int filelength;

  qContentType("text/html");
  qDecoder();

  text = qValueDefault("", "text");

  filedata   = qValue("binary");
  if((filelength = qiValue("binary.length")) == 0) qError("Select file, please.");
  filename   = qValue("binary.filename");
  contenttype = qValue("binary.contenttype");
  sprintf(filepath, "%s/%s", BASEPATH, filename);

  if(qSaveStr(filedata, filelength, filepath, "w") < 0) {
    qError("File(%s) open fail. Please make sure CGI or directory has right permission.", filepath);
  }

  printf("You entered: <b>%s</b>\n", text);
  printf("<br><a href=\"%s\">%s</a> (%d bytes, %s) saved.", filepath, filename, filelength, contenttype);

  qFree();
  return 0;
}
