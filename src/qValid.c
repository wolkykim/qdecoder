/**************************************************************************
 * qDecoder - Web Application Interface for C/C++   http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

/**
 * @file qValid.c Validation Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"

/**********************************************
** Usage : qCheckEmail(email address);
** Return: If it is valid return 1. Or return 0.
** Do    : Check E-mail address.
**********************************************/
bool qCheckEmail(char *email) {
	int i, alpa, dot, gol;

	if (email == NULL) return false;

	for (i = alpa = dot = gol = 0; email[i] != '\0'; i++) {
		switch (email[i]) {
			case '@' : {
				if (alpa == 0) return false;
				if (gol > 0)   return false;
				gol++;
				break;
			}
			case '.' : {
				if ((i > 0)   && (email[i - 1] == '@')) return false;
				if ((gol > 0) && (email[i - 1] == '.')) return false;
				dot++;
				break;
			}
			default  : {
				alpa++;
				if ((email[i] >= '0') && (email[i] <= '9')) break;
				else if ((email[i] >= 'A') && (email[i] <= 'Z')) break;
				else if ((email[i] >= 'a') && (email[i] <= 'z')) break;
				else if ((email[i] == '-') || (email[i] == '_')) break;
				else return false;
			}
		}
	}

	if ((alpa <= 3) || (gol == 0) || (dot == 0)) return false;

	return true;
}

/**********************************************
** Usage : qCheckUrl(internet address);
** Return: If it is valid return 1. Or return 0.
** Do    : Check valid URL.
**********************************************/
bool qCheckUrl(char *url) {
	if (!strncmp(url, "http://", 7)) return true;
	else if (!strncmp(url, "ftp://", 6)) return true;
	else if (!strncmp(url, "telnet://", 9)) return true;
	else if (!strncmp(url, "mailto:", 7)) return true;
	else if (!strncmp(url, "news:", 5)) return true;
	return false;
}
