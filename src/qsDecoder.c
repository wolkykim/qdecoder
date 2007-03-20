/************************************************************************
qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org

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
************************************************************************/

#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Static Values Definition used only internal
**********************************************/

static Q_Entry *_multi_last_entry = NULL;
static char _multi_last_key[1024];


/**********************************************
** Usage : qsDecoder(string);
** Return: Success pointer of the first entry, Fail NULL.
** Do    : Save string into linked list.
           # is used for comments.
**********************************************/
Q_Entry *qsDecoder(char *str) {
  Q_Entry *first, *entries, *back;
  char  *org, *buf, *offset;
  int  eos;

  if(str == NULL) return NULL;

  first = entries = back = NULL;

  if((org = strdup(str)) == NULL) qError("qsDecoder(): Memory allocation fail.");

  for(buf = offset = org, eos = 0; eos == 0; ) {
    for(buf = offset; *offset != '\n' && *offset != '\0'; offset++);
    if(*offset == '\0') eos = 1;
    else *offset = '\0', offset++;

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
  }

  free(org);

  return first;
}

char *qsValue(Q_Entry *first, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qsValue(): Message is too long or invalid.");
  va_end(arglist);

  return _EntryValue(first, name);
}

int qsiValue(Q_Entry *first, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qsiValue(): Message is too long or invalid.");
  va_end(arglist);

  return _EntryiValue(first, name);
}

char *qsValueFirst(Q_Entry *first, char *format, ...) {
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(_multi_last_key, format, arglist);
  if(strlen(_multi_last_key) + 1 > sizeof(_multi_last_key) || status == EOF) qError("qfValueFirst(): Message is too long or invalid.");
  va_end(arglist);

  if(first == NULL) return NULL;
  _multi_last_entry = first;

  return qfValueNext();
}

char *qsValueNext(void) {
  Q_Entry *entries;

  for(entries = _multi_last_entry; entries; entries = entries->next) {
    if(!strcmp(_multi_last_key, entries->name)) {
      _multi_last_entry = entries->next;
      return (entries->value);
    }
  }
  _multi_last_entry = NULL;
  strcpy(_multi_last_key, "");

  return NULL;
}

int qsPrint(Q_Entry *first) {
  return _EntryPrint(first);
}

void qsFree(Q_Entry *first) {
  _EntryFree(first);
}

