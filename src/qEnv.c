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
** Usage : qGetEnv(environment string name, fail return string pointer);
** Return: Environment string or 'nullstr'.
** Do    : Get environment string.
**         When it does not find 'envname', it will return 'nullstr'.
**********************************************/
char *qGetEnv(char *envname, char *nullstr) {
  char *envstr;

  if((envstr = getenv(envname)) != NULL) return envstr;

  return nullstr;
}

/**********************************************
** Usage : qCGIenv(pointer of Cgienv);
** Do    : Get environment of CGI.
**********************************************/
void qCGIenv(Q_CGIenv *env) {
  struct tm *envtime;

  envtime = qGetTime();

  env->auth_type		= qGetEnv("AUTH_TYPE",	NULL);
  env->content_length		= qGetEnv("CONTENT_LENGTH", NULL);
  env->content_type		= qGetEnv("CONTENT_TYPE", NULL);
  env->document_root		= qGetEnv("DOCUMENT_ROOT", NULL);
  env->gateway_interface	= qGetEnv("GATEWAY_INTERFACE",	NULL);
  env->http_accept		= qGetEnv("HTTP_ACCEPT", NULL);
  env->http_accept_encoding	= qGetEnv("HTTP_ACCEPT_ENCODING", NULL);
  env->http_accept_language	= qGetEnv("HTTP_ACCEPT_LANGUAGE", NULL);
  env->http_connection		= qGetEnv("HTTP_CONNECTION", NULL);
  env->http_cookie		= qGetEnv("HTTP_COOKIE", NULL);
  env->http_host		= qGetEnv("HTTP_HOST",	NULL);
  env->http_referer		= qGetEnv("HTTP_REFERER", NULL);
  env->http_user_agent		= qGetEnv("HTTP_USER_AGENT", NULL);
  env->query_string		= qGetEnv("QUERY_STRING", NULL);
  env->remote_addr		= qGetEnv("REMOTE_ADDR", NULL);
  env->remote_host		= qGetEnv("REMOTE_HOST", env->remote_addr);
  env->remote_port		= qGetEnv("REMOTE_PORT", NULL);
  env->remote_user		= qGetEnv("REMOTE_USER", NULL);
  env->request_method		= qGetEnv("REQUEST_METHOD", NULL);
  env->request_uri		= qGetEnv("REQUEST_URI", NULL);
  env->script_filename		= qGetEnv("SCRIPT_FILENAME", NULL);
  env->script_name		= qGetEnv("SCRIPT_NAME", NULL);
  env->server_admin		= qGetEnv("SERVER_ADMIN", NULL);
  env->server_name		= qGetEnv("SERVER_NAME", NULL);
  env->server_port		= qGetEnv("SERVER_PORT", NULL);
  env->server_protocol		= qGetEnv("SERVER_PROTOCOL", NULL);
  env->server_signature		= qGetEnv("SERVER_SIGNATURE", NULL);
  env->server_software		= qGetEnv("SERVER_SOFTWARE", NULL);
  env->unique_id		= qGetEnv("UNIQUE_ID",	NULL);

  /* qDecoder Supported Extended Informations */
  env->year = envtime->tm_year;
  env->mon  = envtime->tm_mon;
  env->day  = envtime->tm_mday;
  env->hour = envtime->tm_hour;
  env->min  = envtime->tm_min;
  env->sec  = envtime->tm_sec;
}

/**********************************************
** Usage : qCGIname();
** Return: CGI filename.
**********************************************/
char *qCGIname(void) {
  static char cgi_name[256];
  char *c;

  if(getenv("SCRIPT_NAME") == NULL) return NULL;

  strcpy(cgi_name, getenv("SCRIPT_NAME"));

  /* Fetch filename in string which include directory name */
  for(c = cgi_name + strlen(cgi_name) - 1; c >= cgi_name && !(*c == '/' || *c == '\\'); c--);
  for(; c >= cgi_name; c--) *c = ' ';
  qRemoveSpace(cgi_name);

  return cgi_name;
}