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
** Static Values Definition used only internal
**********************************************/

static Q_Entry *_cookie_first_entry = NULL;


/**********************************************
** Usage : qcDecoder();
** Return: Success number of values, Fail -1.
** Do    : Decode COOKIES & Save it in linked list.
**********************************************/
int qcDecoder(void) {
  Q_Entry *entries, *back;
  char *query;
  int  amount;

  if(_cookie_first_entry != NULL) return -1;

  if(getenv("HTTP_COOKIE") == NULL) return 0;
  query = strdup(getenv("HTTP_COOKIE"));

  entries = back = NULL;

  for(amount = 0; *query; amount++) {
    back = entries;
    entries = (Q_Entry *)malloc(sizeof(Q_Entry));
    if(back != NULL) back->next = entries;
    if(_cookie_first_entry == NULL) _cookie_first_entry = entries;

    entries->value = _makeword(query, ';');
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qURLdecode(entries->name);
    qURLdecode(entries->value);
    qRemoveSpace(entries->name);
  }
  free(query);

  return amount;
}

char *qcValue(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qcValue(): Message is too long or invalid");
  va_end(arglist);

  if(_cookie_first_entry == NULL) qcDecoder();
  return _EntryValue(_cookie_first_entry, name);
}

int qciValue(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qciValue(): Message is too long or invalid");
  va_end(arglist);

  if(_cookie_first_entry == NULL) qcDecoder();
  return _EntryiValue(_cookie_first_entry, name);
}

void qcPrint(void) {
  if(_cookie_first_entry == NULL)qcDecoder();
  _EntryPrint(_cookie_first_entry);
}

void qcFree(void) {
 _EntryFree(_cookie_first_entry);
 _cookie_first_entry = NULL;
}

/**********************************************
** Usage : qSetCookie(name, value, expire days, path, domain, secure);
** Do    : Set cookie.
**
** The 'exp_days' is number of days which expire the cookie.
** The current time + exp_days will be set.
** This function should be called before qContentType().
**
** ex) qSetCookie("NAME", "Kim", 30, NULL, NULL, NULL);
**********************************************/
void qSetCookie(char *name, char *value, int exp_days, char *path, char *domain, char *secure) {
  char *Name, *Value;
  char cookie[(4 * 1024) + 256];

  /* Name=Value */
  Name = qURLencode(name), Value = qURLencode(value);
  sprintf(cookie, "%s=%s", Name, Value);
  free(Name), free(Value);

  if(exp_days != 0) {
    time_t plus_sec;
    char gmt[256];
    plus_sec = (time_t)(exp_days * 24 * 60 * 60);
    qGetGMTime(gmt, plus_sec);
    strcat(cookie, "; expires=");
    strcat(cookie, gmt);
  }

  if(path != NULL) {
    if(path[0] != '/') qError("qSetCookie(): Path string(%s) must start with '/' character.", path);
    strcat(cookie, "; path=");
    strcat(cookie, path);
  }

  if(domain != NULL) {
    if(strstr(domain, "/") != NULL || strstr(domain, ".") == NULL) qError("qSetCookie(): Invalid domain name(%s).", domain);
    strcat(cookie, "; domain=");
    strcat(cookie, domain);
  }

  if(secure != NULL) {
    strcat(cookie, "; secure");
  }

  printf("Set-Cookie: %s\n", cookie);
}

/**********************************************
** Usage : qAddCookie(name, value);
** Do    : Force to add given name and value to cookie's linked list.
**
** ex) qAddCookie("NAME", "Kim");
**********************************************/
void qAddCookie(char *name, char *value) {
  if(_cookie_first_entry == NULL) qcDecoder();
  _EntryAddStr(_cookie_first_entry, name, value);
}