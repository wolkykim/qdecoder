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

#define CONF_FILE		"confparser.conf"

int main(void) {
  Q_ENTRY *conf;
  char *protocol, *host;
  int port;

  /* Open configuration file */
  if(!(conf = qfDecoder(CONF_FILE))) qError("Configuration file(%s) not found.", CONF_FILE);

  /* Get variable */
  protocol = qfValue(conf, "PROTOCOL");
  host     = qfValue(conf, "HOST");
  port     = qfiValue(conf, "PORT");

  /* Print out */
  qContentType("text/plain");
  printf("Protocol : %s\n", protocol);
  printf("Host     : %s\n", host);
  printf("Port     : %d\n", port);

  printf("\n--[CONFIGURATION DUMP]--\n");
  qfPrint(conf);

  /* Deallocate parsed entries */
  qfFree(conf);

  return 0;
}

