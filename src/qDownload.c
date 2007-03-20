/************************************************************************
qDecoder - C/C++ CGI Library                      http://www.qDecoder.org

Copyright (C) 2001 The qDecoder Project.
Copyright (C) 1999,2000 Hongik Internet, Inc.
Copyright (C) 1998 Nobreak Technologies, Inc.
Copyright (C) 1996,1997 Seung-young Kim.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Copyright Disclaimer:
  Hongik Internet, Inc., hereby disclaims all copyright interest.
  President, Christopher Roh, 6 April 2000

  Nobreak Technologies, Inc., hereby disclaims all copyright interest.
  President, Yoon Cho, 6 April 2000

  Seung-young Kim, hereby disclaims all copyright interest.
  Author, Seung-young Kim, 6 April 2000

Author:
  Seung-young Kim <nobreak@hongik.com>
  Hongik Internet, Inc. 17th Fl., Marine Center Bldg.,
  51, Sogong-dong, Jung-gu, Seoul, 100-070, Korea.
  Tel: +82-2-753-2553, Fax: +82-2-753-1302
************************************************************************/

#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Usage : qDownload(filename);
** Do    : Pump file to stdout, do not call qContentType().
** Return: Success 1, File not found 0.
**********************************************/
int qDownload(char *filename) {
  return qDownloadMime(filename, "application/octet-stream");
}

/**********************************************
** Usage : qDownloadMime(filename, mime);
** Do    : Pump file to stdout, should not call qContentType().
**         if mime is 'application/octet-stream', forcing download.
** Return: Success number of bytes sent, File not found -1.
**********************************************/
int qDownloadMime(char *filename, char *mime) {
  char *file, *c, *disposition;
  int sent;

  if(mime == NULL) mime = "application/octet-stream";

  if(filename == NULL) qError("qDownload(): Null pointer can not be used.");
  if(qCheckFile(filename) == 0) return -1;

  file = strdup(filename);

  /* Fetch filename in string which include directory name */
  for(c = file + strlen(file) - 1; c >= file && !(*c == '/' || *c == '\\'); c--);
  for(; c >= file; c--) *c = ' ';
  qRemoveSpace(file);

  if(!strcmp(mime, "application/octet-stream")) disposition = "attachment";
  else disposition = "inline";
  printf("Content-Disposition: %s;filename=\"%s\"\n", disposition, file);
  printf("Content-Transfer-Encoding: binary\n");
  printf("Accept-Ranges: bytes\n");
  printf("Content-Length: %ld\n", qFileSize(filename));
  printf("Connection: close\n");
  printf("Content-Type: %s\n", mime);
  printf("\n");
  free(file);

  sent = qCatFile(filename);
  return sent;
}