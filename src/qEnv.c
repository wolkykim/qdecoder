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
** Usage : qGetenvDefault(default string, environment string name);
** Return: Environment string or 'nullstr'.
** Do    : Get environment string.
**         When it does not find 'envname', it will return 'nullstr'.
**********************************************/
char *qGetenvDefault(char *nullstr, char *envname) {
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

  env->auth_type		= qGetenvDefault(NULL, "AUTH_TYPE");
  env->content_length		= qGetenvDefault(NULL, "CONTENT_LENGTH");
  env->content_type		= qGetenvDefault(NULL, "CONTENT_TYPE");
  env->document_root		= qGetenvDefault(NULL, "DOCUMENT_ROOT");
  env->gateway_interface	= qGetenvDefault(NULL, "GATEWAY_INTERFACE");
  env->http_accept		= qGetenvDefault(NULL, "HTTP_ACCEPT");
  env->http_accept_encoding	= qGetenvDefault(NULL, "HTTP_ACCEPT_ENCODING");
  env->http_accept_language	= qGetenvDefault(NULL, "HTTP_ACCEPT_LANGUAGE");
  env->http_connection		= qGetenvDefault(NULL, "HTTP_CONNECTION");
  env->http_cookie		= qGetenvDefault(NULL, "HTTP_COOKIE");
  env->http_host		= qGetenvDefault(NULL, "HTTP_HOST");
  env->http_referer		= qGetenvDefault(NULL, "HTTP_REFERER");
  env->http_user_agent		= qGetenvDefault(NULL, "HTTP_USER_AGENT");
  env->query_string		= qGetenvDefault(NULL, "QUERY_STRING");
  env->remote_addr		= qGetenvDefault(NULL, "REMOTE_ADDR");
  env->remote_host		= qGetenvDefault(env->remote_addr, "REMOTE_HOST");
  env->remote_port		= qGetenvDefault(NULL, "REMOTE_PORT");
  env->remote_user		= qGetenvDefault(NULL, "REMOTE_USER");
  env->request_method		= qGetenvDefault(NULL, "REQUEST_METHOD");
  env->request_uri		= qGetenvDefault(NULL, "REQUEST_URI");
  env->script_filename		= qGetenvDefault(NULL, "SCRIPT_FILENAME");
  env->script_name		= qGetenvDefault(NULL, "SCRIPT_NAME");
  env->server_admin		= qGetenvDefault(NULL, "SERVER_ADMIN");
  env->server_name		= qGetenvDefault(NULL, "SERVER_NAME");
  env->server_port		= qGetenvDefault(NULL, "SERVER_PORT");
  env->server_protocol		= qGetenvDefault(NULL, "SERVER_PROTOCOL");
  env->server_signature		= qGetenvDefault(NULL, "SERVER_SIGNATURE");
  env->server_software		= qGetenvDefault(NULL, "SERVER_SOFTWARE");
  env->unique_id		= qGetenvDefault(NULL, "UNIQUE_ID");

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

