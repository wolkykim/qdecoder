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

Author:
  Seung-young Kim <wolkykim(at)ziom.co.kr>
************************************************************************/

#include "qDecoder.h"
#include "qInternal.h"

#define _INCLUDE_DIRECTIVE	"@INCLUDE "
#define _VAR_OPEN		"${"
#define _VAR_CLOSE		"}"
#define _VAR_CMD		'!'
#define _VAR_ENV		'%'

/**********************************************
** Usage : qfDecoder(file);
** Return: Success pointer of the first entry, Fail NULL.
** Do    : Save file into linked list.
**         Starting # is used for comments.
**
**         @INCLUDE other.conf          => (include other.conf)
**         base = /tmp                  => base = /tmp
**         log  = ${base}/log           => log  = /tmp/logVariable
**         host = ${!/bin/hostname -s}  => host = arena
**         path = ${%PATH}              => path = /usr/bin:/usr/sbin
**         #hmm = this is comments      => (skip comment)
**********************************************/
Q_Entry *qfDecoder(char *file) {
  Q_Entry *first, *entries;
  char *str, *p;

  p = str = qReadFile(file, NULL);
  if(str == NULL) return NULL;

  /* processing include directive */
  while((p = strstr(p, _INCLUDE_DIRECTIVE)) != NULL) {

    if(p == str || p[-1] == '\n') {
      char buf[1024], *e, *t = NULL;
      int len;

      /* parse filename */
      for(e = p + strlen(_INCLUDE_DIRECTIVE); *e != '\n' && *e != '\0'; e++);
      len = e - (p + strlen(_INCLUDE_DIRECTIVE));
      if(len >= sizeof(buf)) qError("qfDecoder(): Can't process %s directive.", _INCLUDE_DIRECTIVE);
      strncpy(buf, p + strlen(_INCLUDE_DIRECTIVE), len);
      buf[len] = '\0';
      qRemoveSpace(buf);

      /* adjust file path */
      if(!(buf[0] == '/' || buf[0] == '\\')) {
        char tmp[1024], *dir;
        dir = dirname(file);
        if(strlen(dir)+1+strlen(buf) >= sizeof(buf)) qError("qfDecoder(): Can't process %s directive.", _INCLUDE_DIRECTIVE);
        sprintf(tmp, "%s/%s", dir, buf);
        strcpy(buf, tmp);
      }

      /* read file */
      if(strlen(buf) == 0 || (t = qReadFile(buf, NULL)) == NULL) qError("qfDecoder(): Can't process '%s%s' directive.", _INCLUDE_DIRECTIVE, buf);

      /* replace */
      strncpy(buf, p, strlen(_INCLUDE_DIRECTIVE) + len);
      buf[strlen(_INCLUDE_DIRECTIVE) + len] = '\0';
      p = qStrReplace("sn", str, buf, t);
      free(t);
      free(str);
      str = p;
    }
    else {
      p += strlen(_INCLUDE_DIRECTIVE);
    }
  }

  /* decode */
  first = qsDecoder(str);
  free(str);

  /* processing ${} directive */
  for(entries = first; entries; entries = entries->next) {
    p = entries->value;
    while((p = strstr(p, _VAR_OPEN)) != NULL) {
      char buf[256], *e, *t;
      int len;

      /* parse variable name */
      if((e = strstr(p + strlen(_VAR_OPEN), _VAR_CLOSE)) == NULL) break;
      len = e - (p + strlen(_VAR_OPEN));
      if(len >= sizeof(buf)) break; /* length between ${ , } */
      strncpy(buf, p + strlen(_VAR_OPEN), len);
      buf[len] = '\0';
      qRemoveSpace(buf);

      /* find value */
      switch(buf[0]) {
        case _VAR_CMD : {
          t = "_NOT_SUPPORTED_YET_";
          break;
        }
        case _VAR_ENV : {
          t = qGetenvDefault("", buf+1);
          break;
        }
        default : {
          if((t = _EntryValue(first, buf)) == NULL) t = "";
          break;
        }
      }

      /* replace */
      strncpy(buf, p, strlen(_VAR_OPEN) + len + strlen(_VAR_CLOSE));
      buf[strlen(_VAR_OPEN) + len + strlen(_VAR_CLOSE)] = '\0';

      p = qStrReplace("sn", entries->value, buf, t);
      free(entries->value);
      entries->value = qRemoveSpace(p);
    }
  }

  return first;
}
