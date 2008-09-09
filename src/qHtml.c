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
 * @file qHtml.c HTML documentation API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

#ifndef DISABLE_CGISUPPORT

/**********************************************
** Usage : qPrintf(mode, format, arg);
** Return: Sucess number of output bytes, Fail EOF.
** Do    : Print message like printf.
**         Mode : see qHtmlPuts()
**********************************************/
bool qHtmlPrintf(int mode, const char *format, ...) {
	char buf[1024*10];
	va_list arglist;
	int status;

	va_start(arglist, format);
	status = vsnprintf(buf, sizeof(buf), format, arglist);
	va_end(arglist);

	return qHtmlPuts(mode, buf);
}

/**********************************************
** Usage : qPuts(mode, string pointer);
** Do    : print HTML link as multi mode.
**
**         Mode 00 : Same as printf()
**         Mode 10  :Mode 0 + Convert
**
**         Mode 01 : Print HTML TAG
**         Mode 11 : Print HTML TAG + Convert
**
**         Mode 02 : Print HTML TAG + Auto Link
**         Mode 12 : Print HTML TAG + Auto Link + Convert
**
**         Mode 03 : Print HTML TAG + Auto Link(_top)
**         Mode 13 : Print HTML TAG + Auto Link(_top) + Convert
**         Mode 23 : Print HTML TAG + Auto Link(new window)
**         Mode 33 : Print HTML TAG + Auto Link(new window) + Convert
**
**         Mode 04 : Waste HTML TAG
**         Mode 14 : Waste HTML TAG + Convert
**
**         Mode 05 : Waste HTML TAG + Auto Link
**         Mode 15 : Waste HTML TAG + Auto Link + Convert
**
**         Mode 06 : Waste HTML TAG + Auto Link(_top)
**         Mode 16 : Waste HTML TAG + Auto Link(_top) + Convert
**         Mode 26 : Waste HTML TAG + Auto Link(new window)
**         Mode 36 : Waste HTML TAG + Auto Link(new window) + Convert
**
**         Convert : " "   -> " "
**                   "  "  -> " &nbsp;"
**                   "   " -> " &nbsp;&nbsp;"
**                   "\n"   -> "<br>\n"
**                   "\r\n" -> "<br>\n"
**
** You can use 1x mode, to wrap long lines with no <pre> tag.
** Note) It modify argument string.
**********************************************/
bool qHtmlPuts(int mode, char *buf) {

	if (buf == NULL) return false;

	if (mode == 0) printf("%s", buf);
	else if (mode == 10) {
		int i;
		for (i = 0; buf[i] != '\0'; i++) {
			switch (buf[i]) {
				case ' '  : {
					if ((i > 0) && (buf[i - 1] == ' ')) printf("&nbsp;");
					else printf(" ");
					break;
				}
				case '\r' : {
					break;
				}
				case '\n' : {
					printf("<br>\n");
					break;
				}
				default   : {
					printf("%c", buf[i]);
					break;
				}
			}
		}
	} else {
		char *ptr, retstop, lastretstop, *target, *deftarget, *token;
		int printhtml, autolink, convert, linkflag, ignoreflag;

		/* set defaults, mode 2*/
		printhtml = 1;
		autolink  = 1;
		target    = "_top";
		deftarget = "qnewwin";
		convert   = 0;

		switch (mode) {
			case 01 : {
				printhtml = 1, autolink = 0, target = "";
				convert = 0;
				break;
			}
			case 11 : {
				printhtml = 1, autolink = 0, target = "";
				convert = 1;
				break;
			}

			case 02 : {
				printhtml = 1, autolink = 1, target = "";
				convert = 0;
				break;
			}
			case 12 : {
				printhtml = 1, autolink = 1, target = "";
				convert = 1;
				break;
			}

			case 03 : {
				printhtml = 1, autolink = 1, target = "_top";
				convert = 0;
				break;
			}
			case 13 : {
				printhtml = 1, autolink = 1, target = "_top";
				convert = 1;
				break;
			}
			case 23 : {
				printhtml = 1, autolink = 1, target = deftarget;
				convert = 0;
				break;
			}
			case 33 : {
				printhtml = 1, autolink = 1, target = deftarget;
				convert = 1;
				break;
			}

			case 04 : {
				printhtml = 0, autolink = 0, target = "";
				convert = 0;
				break;
			}
			case 14 : {
				printhtml = 0, autolink = 0, target = "";
				convert = 1;
				break;
			}

			case 05 : {
				printhtml = 0, autolink = 1, target = "";
				convert = 0;
				break;
			}
			case 15 : {
				printhtml = 0, autolink = 1, target = "";
				convert = 1;
				break;
			}

			case 06 : {
				printhtml = 0, autolink = 1, target = "_top";
				convert = 0;
				break;
			}
			case 16 : {
				printhtml = 0, autolink = 1, target = "_top";
				convert = 1;
				break;
			}
			case 26 : {
				printhtml = 0, autolink = 1, target = deftarget;
				convert = 0;
				break;
			}
			case 36 : {
				printhtml = 0, autolink = 1, target = deftarget;
				convert = 1;
				break;
			}

			default: {
				DEBUG("_autolink(): Invalid Mode (%d).", mode);
				return false;
			}
		}

		token = " `(){}[]<>\"',\r\n";
		lastretstop = '0'; /* any character except space */
		ptr = qStrTok(buf, token, &retstop);

		for (linkflag = ignoreflag = 0; ptr != NULL;) {
			/* auto link */
			if (ignoreflag == 0) {
				if (autolink == 1) {
					if (!strncmp(ptr, "http://",        7)) linkflag = 1;
					else if (!strncmp(ptr, "https://",  8)) linkflag = 1;
					else if (!strncmp(ptr, "ftp://",    6)) linkflag = 1;
					else if (!strncmp(ptr, "telnet://", 9)) linkflag = 1;
					else if (!strncmp(ptr, "news:",     5)) linkflag = 1;
					else if (!strncmp(ptr, "mailto:",   7)) linkflag = 1;
					else if (qHtmlIsEmail(ptr) == 1)         linkflag = 2;
					else linkflag = 0;
				}
				if (linkflag == 1) printf("<a href=\"%s\" target=\"%s\">%s</a>", ptr, target, ptr);
				else if (linkflag == 2) printf("<a href=\"mailto:%s\">%s</a>", ptr, ptr);
				else printf("%s", ptr);
			}

			/* print */
			if (printhtml == 1) {
				if     (retstop == '<')  printf("&lt;");
				else if (retstop == '>')  printf("&gt;");
				else if (retstop == '\"') printf("&quot;");
				else if (retstop == '&')  printf("&amp;");

				else if (retstop == ' '  && convert == 1) {
					if (lastretstop == ' ' && strlen(ptr) == 0) printf("&nbsp;");
					else printf(" ");
				} else if (retstop == '\r' && convert == 1); /* skip when convert == 1 */
				else if (retstop == '\n' && convert == 1) printf("<br>\n");

				else if (retstop != '\0') printf("%c", retstop);
			} else {
				if     (retstop == '<') ignoreflag = 1;
				else if (retstop == '>') ignoreflag = 0;

				else if (retstop == '\"' && ignoreflag == 0) printf("&quot;");
				else if (retstop == '&'  && ignoreflag == 0) printf("&amp;");

				else if (retstop == ' '  && ignoreflag == 0 && convert == 1) {
					if (lastretstop == ' ' && strlen(ptr) == 0) printf("&nbsp;");
					else printf(" ");
				} else if (retstop == '\r' && ignoreflag == 0 && convert == 1); /* skip when convert == 1 */
				else if (retstop == '\n' && ignoreflag == 0 && convert == 1) printf("<br>\n");

				else if (retstop != '\0' && ignoreflag == 0) printf("%c", retstop);

			}

			lastretstop = retstop;
			ptr = qStrTok(NULL, token, &retstop);
		}
	}

	return true;
}


/**********************************************
** Usage : qCheckEmail(email address);
** Return: If it is valid return 1. Or return 0.
** Do    : Check E-mail address.
**********************************************/
bool qHtmlIsEmail(const char *email) {
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
bool qHtmlIsUrl(const char *url) {
	if (!strncmp(url, "http://", 7)) return true;
	else if (!strncmp(url, "ftp://", 6)) return true;
	else if (!strncmp(url, "telnet://", 9)) return true;
	else if (!strncmp(url, "mailto:", 7)) return true;
	else if (!strncmp(url, "news:", 5)) return true;
	return false;
}

#endif /* DISABLE_CGISUPPORT */
