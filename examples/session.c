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

int main(void) {
  char *sessionid;
  char *mode, *name, *value;
  time_t expire;

  /* fetch queries */
  expire = (time_t)qiValue("expire");
  mode   = qValueNotEmpty("Mode not found", "mode");
  name   = qValue("name");
  value  = qValue("value");

  /* Set valid interval of this session */
  /* start session. this function should be called before qContentType(). */
  qSession(NULL);
  sessionid = qSessionGetID();

  /* Mose case, you don't need to set timeout but we use it here to give you a show case */
  if(expire > 0) qSessionSetTimeout(expire);

  switch(mode[0]) {
    case 's': {
      qSessionAdd(name, value);
      break;
    }
    case 'i': {
      qSessionAddInteger(name, atoi(value));
      break;
    }
    case 'r': {
      qSessionRemove(name);
      break;
    }
    case 'd': {
      qSessionDestroy();
      qContentType("text/html");
      printf("Session destroied.");
      return 0;
      break;
    }
  }

  /* screen out */
  qContentType("text/html");
  qSessionPrint();

  /* save session & free allocated memories */
  qSessionFree();
  return 0;
}

