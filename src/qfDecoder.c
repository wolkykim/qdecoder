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
** Usage : qfDecoder(filename);
** Return: Success pointer of the first entry, Fail NULL.
** Do    : Save file into linked list.
           # is used for comments.
**********************************************/
Q_Entry *qfDecoder(char *filename) {
  FILE  *fp;
  Q_Entry *first, *entries, *back;
  char  *buf;

  fp = fopen(filename, "rt");
  if(fp == NULL) return NULL;

  first = entries = back = NULL;

  while((buf = qfGetLine(fp)) != NULL) {
    qRemoveSpace(buf);
    if((buf[0] == '#') || (buf[0] == '\0')) continue;

    back = entries;
    entries = (Q_Entry *)malloc(sizeof(Q_Entry));
    if(back != NULL) back->next = entries;
    if(first == NULL) first = entries;

    entries->value = strdup(buf);
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qRemoveSpace(entries->name);
    qRemoveSpace(entries->value);

    free(buf);
  }

  fclose(fp);
  return first;
}

char *qfValue(Q_Entry *first, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qfValue(): Message is too long or invalid");
  va_end(arglist);

  return _EntryValue(first, name);
}

int qfiValue(Q_Entry *first, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qfiValue(): Message is too long or invalid");
  va_end(arglist);

  return _EntryiValue(first, name);
}

void qfPrint(Q_Entry *first) {
  _EntryPrint(first);
}

void qfFree(Q_Entry *first) {
  _EntryFree(first);
}