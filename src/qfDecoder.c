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

#define CONF_INCLUDE_START	"@INCLUDE \""
#define CONF_INCLUDE_END	"\""

/**********************************************
** Usage : qfDecoder(filename);
** Return: Success pointer of the first entry, Fail NULL.
** Do    : Save file into linked list.
           # is used for comments.
**********************************************/
Q_Entry *qfDecoder(char *filename) {
  Q_Entry *ret;
  char *str, *p;

  p = str = qReadFile(filename, NULL);
  if(str == NULL) return NULL;

  while((p = strstr(p, CONF_INCLUDE_START))) {
    char tmpbuf[1024], *endp, *t1;

    /* For 'include directive.  Gets a filename and replaces the string with file contents.  */
    if((p == str || p[-1] == '\n') && (endp = strstr(p + strlen(CONF_INCLUDE_START), CONF_INCLUDE_END)) != NULL) {
      t1 = p+strlen(CONF_INCLUDE_START);
      strncpy(tmpbuf, t1, endp - t1);
      tmpbuf[endp-(t1)] = '\0';
      endp = (endp + strlen(CONF_INCLUDE_END)) - 1;

      t1 = qReadFile(tmpbuf, NULL);
      if (t1 == NULL) return NULL;
      memcpy (tmpbuf, p, endp-p+1);
      tmpbuf[endp-p+1] = '\0';
      p = qStrReplace("sn", str, tmpbuf, t1);
      free(t1);
      free(str);
      str = p;
    }
  }

  ret = qsDecoder(str);
  free(str);

  return ret;
}
