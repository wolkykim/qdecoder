/*
 * Copyright 2008 The qDecoder Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file qHtml.c HTML documentation API
 */

#ifndef DISABLE_CGI

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Print out HTML contents into stream
 *
 * @param stream	output stream such like stdout
 * @param mode		conversion mode
 * @param format	string format
 *
 * @return		true if successful, otherwise returns false
 *
 * @code
 *   Mode 00 : Same as printf()
 *   Mode 10  :Mode 0 + Convert
 *
 *   Mode 01 : Print HTML TAG using &lt and &gt
 *   Mode 11 : Print HTML TAG + Convert
 *
 *   Mode 02 : Print HTML TAG + Auto Link
 *   Mode 12 : Print HTML TAG + Auto Link + Convert
 *
 *   Mode 03 : Print HTML TAG + Auto Link(_top)
 *   Mode 13 : Print HTML TAG + Auto Link(_top) + Convert
 *   Mode 23 : Print HTML TAG + Auto Link(new window)
 *   Mode 33 : Print HTML TAG + Auto Link(new window) + Convert
 *
 *   Mode 04 : Ignore HTML TAG
 *   Mode 14 : Ignore HTML TAG + Convert
 *
 *   Mode 05 : Ignore HTML TAG + Auto Link
 *   Mode 15 : Ignore HTML TAG + Auto Link + Convert
 *
 *   Mode 06 : Ignore HTML TAG + Auto Link(_top)
 *   Mode 16 : Ignore HTML TAG + Auto Link(_top) + Convert
 *   Mode 26 : Ignore HTML TAG + Auto Link(new window)
 *   Mode 36 : Ignore HTML TAG + Auto Link(new window) + Convert
 *
 *   Convert : " "   -> " "
 *             "  "  -> " &nbsp;"
 *             "   " -> " &nbsp;&nbsp;"
 *             "\n"   -> "<br>\n"
 *             "\r\n" -> "<br>\n"
 * @endcode
 */
bool qHtmlPrintf(FILE *stream, int mode, const char *format, ...) {
	char *buf;
	DYNAMIC_VSPRINTF(buf, format);
	if(buf == NULL) false;

	bool ret = qHtmlPuts(stream, mode, buf);
	free(buf);

	return ret;
}

/**
 * Puts a string on the stream
 *
 * @param stream	output stream such like stdout
 * @param mode		conversion mode
 * @param buf		source string
 *
 * @return		true if successful, otherwise returns false
 *
 * @note	It modify argument string.
 */
bool qHtmlPuts(FILE *stream, int mode, char *buf) {

	if (buf == NULL) return false;

	if (mode == 0) fprintf(stream, "%s", buf);
	else if (mode == 10) {
		int i;
		for (i = 0; buf[i] != '\0'; i++) {
			switch (buf[i]) {
				case ' '  : {
					if ((i > 0) && (buf[i - 1] == ' ')) fprintf(stream, "&nbsp;");
					else fprintf(stream, " ");
					break;
				}
				case '\r' : {
					break;
				}
				case '\n' : {
					fprintf(stream, "<br>\n");
					break;
				}
				default   : {
					fprintf(stream, "%c", buf[i]);
					break;
				}
			}
		}
	} else {
		char *ptr, retstop, lastretstop, *target, *deftarget, *token;
		int printhtml, autolink, convert, linkflag, ignoreflag, offset;

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
		offset = 0;
		ptr = qStrTok(buf, token, &retstop, &offset);

		for (linkflag = ignoreflag = 0; ptr != NULL;) {
			/* auto link */
			if (ignoreflag == 0) {
				if (autolink == 1) {
					if (qStrIsUrl(ptr) == true) linkflag = 1;
					else if (qStrIsEmail(ptr) == true) linkflag = 2;
					else linkflag = 0;
				}
				if (linkflag == 1) fprintf(stream, "<a href=\"%s\" target=\"%s\">%s</a>", ptr, target, ptr);
				else if (linkflag == 2) fprintf(stream, "<a href=\"mailto:%s\">%s</a>", ptr, ptr);
				else fprintf(stream, "%s", ptr);
			}

			/* print */
			if (printhtml == 1) {
				if     (retstop == '<')  fprintf(stream, "&lt;");
				else if (retstop == '>')  fprintf(stream, "&gt;");
				else if (retstop == '\"') fprintf(stream, "&quot;");
				else if (retstop == '&')  fprintf(stream, "&amp;");

				else if (retstop == ' '  && convert == 1) {
					if (lastretstop == ' ' && strlen(ptr) == 0) fprintf(stream, "&nbsp;");
					else fprintf(stream, " ");
				} else if (retstop == '\r' && convert == 1); /* skip when convert == 1 */
				else if (retstop == '\n' && convert == 1) fprintf(stream, "<br>\n");

				else if (retstop != '\0') fprintf(stream, "%c", retstop);
			} else {
				if     (retstop == '<') ignoreflag = 1;
				else if (retstop == '>') ignoreflag = 0;

				else if (retstop == '\"' && ignoreflag == 0) fprintf(stream, "&quot;");
				else if (retstop == '&'  && ignoreflag == 0) fprintf(stream, "&amp;");

				else if (retstop == ' '  && ignoreflag == 0 && convert == 1) {
					if (lastretstop == ' ' && strlen(ptr) == 0) fprintf(stream, "&nbsp;");
					else fprintf(stream, " ");
				} else if (retstop == '\r' && ignoreflag == 0 && convert == 1); /* skip when convert == 1 */
				else if (retstop == '\n' && ignoreflag == 0 && convert == 1) fprintf(stream, "<br>\n");

				else if (retstop != '\0' && ignoreflag == 0) fprintf(stream, "%c", retstop);

			}

			lastretstop = retstop;
			ptr = qStrTok(buf, token, &retstop, &offset);
		}
	}

	return true;
}

#endif /* DISABLE_CGI */
