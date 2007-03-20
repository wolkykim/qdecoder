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

#define	SSI_INCLUDE_START	"<!--#include file=\""
#define SSI_INCLUDE_END		"\"-->"

/**********************************************
** Usage : qSedArgAdd(entry pointer, name, value);
** Do    : Add given name and value to linked list.
**         If same name exists, it'll be replaced.
**
** ex) qValueAdd("NAME", "Seung-young Kim");
**********************************************/
Q_Entry *qSedArgAdd(Q_Entry *first, char *name, char *format, ...) {
  Q_Entry *new_entry;
  char value[1024];
  int status;
  va_list arglist;

  if(!strcmp(name, "")) qError("qSedArgAdd(): can not add empty name.");

  va_start(arglist, format);
  status = vsprintf(value, format, arglist);
  if(strlen(value) + 1 > sizeof(value) || status == EOF) qError("qSedArgAdd(): Message is too long or invalid.");
  va_end(arglist);

  new_entry = _EntryAdd(first, name, value, 1);
  if(!first) first = new_entry;

  return first;
}

/**********************************************
** Usage : qSedArgAddDirect(entry pointer, value);
** Do    : Add given name and value to linked list.
**         If same name exists, it'll be replaced.
**
** ex) qSedArgAddDirect(entries, "NAME", value);
**********************************************/
Q_Entry *qSedArgAddDirect(Q_Entry *first, char *name, char *value) {
  Q_Entry *new_entry;

  if(!strcmp(name, "")) qError("qSedArgAddDirect(): can not add empty name.");

  new_entry = _EntryAdd(first, name, value, 1);
  if(!first) first = new_entry;

  return first;
}

/**********************************************
** Usage : qPrint(pointer of the first Entry);
** Return: Amount of entries.
** Do    : Print all parsed values & names for debugging.
**********************************************/
int qSedArgPrint(Q_Entry *first) {
  return _EntryPrint(first);
}

/**********************************************
** Usage : qFree(pointer of the first Entry);
** Do    : Make free of linked list memory.
**********************************************/
void qSedArgFree(Q_Entry *first) {
  _EntryFree(first);
}

/**********************************************
** Usage : qSedStr(Entry pointer, fpout, arg);
** Return: Success 1
** Do    : Stream Editor.
**********************************************/
int qSedStr(Q_Entry *first, char *srcstr, FILE *fpout) {
  Q_Entry *entries;
  char *sp;

  if(srcstr == NULL) return 0;

  /* Parsing */
  for(sp = srcstr; *sp != '\0'; sp++) {
    int flag;

    /* SSI invocation */
    if(!strncmp(sp, SSI_INCLUDE_START, strlen(SSI_INCLUDE_START))) {
      char ssi_inc_file[1024], *endp;
      if((endp = strstr(sp, SSI_INCLUDE_END)) != NULL) {
        sp += strlen(SSI_INCLUDE_START);
        strncpy(ssi_inc_file, sp, endp - sp);
        ssi_inc_file[endp-sp] = '\0';
        sp = (endp + strlen(SSI_INCLUDE_END)) - 1;

        if(qCheckFile(ssi_inc_file) == 1) qSedFile(first, ssi_inc_file, fpout);
        else printf("[qSedStr: an error occurred while processing 'include' directive - file(%s) open fail]", ssi_inc_file);
      }
      else printf("[qSedStr: an error occurred while processing 'include' directive - ending tag not found]");
      continue;
    }

    /* Pattern Matching */
    for(entries = first, flag = 0; entries && flag == 0; entries = entries->next) {
      if(!strncmp(sp, entries->name, strlen(entries->name))) {
        fprintf(fpout, "%s", entries->value);
        sp += strlen(entries->name) - 1;
        flag = 1;
      }
    }
    if(flag == 0) fprintf(fpout, "%c", *sp);
  }

  return 1;
}

/**********************************************
** Usage : qSedFile(filename, fpout, arg);
** Return: Success 1, Fail open fail 0.
** Do    : Stream Editor.
**********************************************/
int qSedFile(Q_Entry *first, char *filename, FILE *fpout) {
  char *sp;
  int flag;

  if((sp = qReadFile(filename, NULL)) == NULL) return 0;
  flag = qSedStr(first, sp, fpout);
  free(sp);

  return flag;
}

