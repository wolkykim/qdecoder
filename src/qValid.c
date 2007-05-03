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


/**********************************************
** Usage : qCheckEmail(email address);
** Return: If it is valid return 1. Or return 0.
** Do    : Check E-mail address.
**********************************************/
int qCheckEmail(char *email) {
	int i, alpa, dot, gol;

	if (email == NULL) return 0;

	for (i = alpa = dot = gol = 0; email[i] != '\0'; i++) {
		switch (email[i]) {
			case '@' : {
				if (alpa == 0) return 0;
				if (gol > 0)   return 0;
				gol++;
				break;
			}
			case '.' : {
				if ((i > 0)   && (email[i - 1] == '@')) return 0;
				if ((gol > 0) && (email[i - 1] == '.')) return 0;
				dot++;
				break;
			}
			default  : {
				alpa++;
				if ((email[i] >= '0') && (email[i] <= '9')) break;
				else if ((email[i] >= 'A') && (email[i] <= 'Z')) break;
				else if ((email[i] >= 'a') && (email[i] <= 'z')) break;
				else if ((email[i] == '-') || (email[i] == '_')) break;
				else return 0;
			}
		}
	}

	if ((alpa <= 3) || (gol == 0) || (dot == 0))return 0;

	return 1;
}

/**********************************************
** Usage : qCheckURL(internet address);
** Return: If it is valid return 1. Or return 0.
** Do    : Check valid URL.
**********************************************/
int qCheckURL(char *url) {
	if (!strncmp(url, "http://", 7)) return 1;
	else if (!strncmp(url, "ftp://", 6)) return 1;
	else if (!strncmp(url, "telnet://", 9)) return 1;
	else if (!strncmp(url, "mailto:", 7)) return 1;
	else if (!strncmp(url, "news:", 5)) return 1;
	return 0;
}

