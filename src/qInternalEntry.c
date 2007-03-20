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
** Linked List(Entry) Routines
**********************************************/

/**********************************************
** Usage : _EntryAddStr(pointer of the any entry, name);
** Return: New entry pointer.
** Do    : Add entry at last.
**********************************************/
Q_Entry *_EntryAddStr(Q_Entry *first, char *name, char *value) {
  Q_Entry *entries;

  entries = (Q_Entry *)malloc(sizeof(Q_Entry));
  entries->name  = strdup(name);
  entries->value = strdup(value);
  entries->next  = NULL;

  /* If first is not NULL, find last entry then make a link*/
  if(first) {
    for(; first->next; first = first->next);
    first->next = entries;
  }

  return entries;
}

/**********************************************
** Usage : _EntryValue(pointer of the first entry, name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find value string pointer.
**         It find value in linked list.
**********************************************/
char *_EntryValue(Q_Entry *first, char *name) {
  Q_Entry *entries;

  for(entries = first; entries; entries = entries->next) {
    if(!strcmp(name, entries->name))return (entries->value);
  }
  return NULL;
}

/**********************************************
** Usage : _EntryiValue(pointer of the first entry, name);
** Return: Success integer of value string, Fail 0.
** Do    : Find value string pointer and convert to integer.
**********************************************/
int _EntryiValue(Q_Entry *first, char *name) {
  char *str;

  str = _EntryValue(first, name);
  if(str == NULL) return 0;
  return atoi(str);
}

/**********************************************
** Usage : _EntryPrint(pointer of the first entry);
** Do    : Print all parsed value & name for debugging.
**********************************************/
void _EntryPrint(Q_Entry *first) {
  Q_Entry *entries;

  qContentType("text/html");

  for(entries = first; entries; entries = entries->next) {
    printf("'%s' = '%s'<br>\n" , entries->name, entries->value);
  }
}

/**********************************************
** Usage : _EntryFree(pointer of the first entry);
** Do    : Make free of linked list memory.
**********************************************/
void _EntryFree(Q_Entry *first) {
  Q_Entry *entries;

  for(; first; first = entries) {
    entries = first->next; /* copy next to tmp */
    free(first->name);
    free(first->value);
    free(first);
  }
}
