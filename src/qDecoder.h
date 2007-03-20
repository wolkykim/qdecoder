/************************************************************************
qDecoder - C/C++ CGI Library                      http://www.qDecoder.org

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
  5th Fl., Daewoong Bldg., 689-4, Yoksam, Kangnam, Seoul, Korea 135-080
  Tel: +82-2-562-8988, Fax: +82-2-562-8987
************************************************************************/

#ifndef _QDECODER_H
#define _QDECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
/* To use setmode() function for converting WIN32's stream mode to _O_BINARY */
#include <io.h>
#include <fcntl.h>
#endif

typedef struct Q_Entry Q_Entry;
struct Q_Entry{
  char *name;
  char *value;
  struct Q_Entry *next;
};

typedef struct Q_CGIenv Q_CGIenv;
struct Q_CGIenv{
  char *auth_type;
  char *content_length;
  char *content_type;
  char *document_root;
  char *gateway_interface;
  char *http_accept;
  char *http_accept_encoding;
  char *http_accept_language;
  char *http_connection;
  char *http_cookie;
  char *http_host;
  char *http_referer;
  char *http_user_agent;
  char *query_string;
  char *remote_addr;
  char *remote_host;
  char *remote_port;
  char *remote_user;
  char *request_method;
  char *request_uri;
  char *script_filename;
  char *script_name;
  char *server_admin;
  char *server_name;
  char *server_port;
  char *server_protocol;
  char *server_software;
  char *server_signature;
  char *unique_id;

  /* Miscellaneous Informations Supported by qDecoder */
  int  year, mon, day, hour, min, sec;
};

/* qDecoder C++ support */
#ifdef __cplusplus
extern "C" {
#endif

int       qDecoder(void);
char      *qValue(char *format, ...);
int       qiValue(char *format, ...);
char      *qValueDefault(char *defstr, char *format, ...);
char      *qValueNotEmpty(char *errmsg, char *format, ...);
char      *qValueReplace(char *mode, char *name, char *tokstr, char *word);
char      *qValueFirst(char *format, ...);
char      *qValueNext(void);
void      qPrint(void);
void      qFree(void);
Q_Entry   *qGetFirstEntry(void);

Q_Entry   *qfDecoder(char *filename);
char      *qfValue(Q_Entry *first, char *format, ...);
int       qfiValue(Q_Entry *first, char *format, ...);
void      qfPrint(Q_Entry *first);
void      qfFree(Q_Entry *first);

Q_Entry   *qsDecoder(char *str);
char      *qsValue(Q_Entry *first, char *format, ...);
int       qsiValue(Q_Entry *first, char *format, ...);
void      qsPrint(Q_Entry *first);
void      qsFree(Q_Entry *first);

int       qcDecoder(void);
char      *qcValue(char *format, ...);
int       qciValue(char *format, ...);
void      qcPrint(void);
void      qcFree(void);

void      qSetCookie(char *name, char *value, int exp_days, char *path, char *domain, char *secure);

int       qAwkOpen(char *filename, char separator);
int       qAwkNext(char array[][256]);
int       qAwkClose(void);

int       qSedStr(char *srcstr, FILE *fpout, char **arg);
int       qSedFile(char *filename, FILE *fpout, char **arg);

int       qArgMake(char *str, char **qlist);
void      qArgFree(char **qlist);
void      qArgPrint(char **qlist);
int       qArgMatch(char *str, char **qlist);
int       qArgEmprint(int mode, char *str, char **qlist);

char      *qURLencode(char *str);
char      *qURLdecode(char *str);
void      qContentType(char *mimetype);
int       qPrintf(int mode, char *format, ...);
void      qPuts(int mode, char *buf);

void      qError(char *format, ...);
void      qErrorLog(char *filename);
void      qErrorContact(char *msg);

void      qReset(void);

void      qCGIenv(Q_CGIenv *env);
char      *qGetEnv(char *envname, char *nullstr);
char      *qCGIname(void);

struct tm *qGetTime(void);
time_t    qGetGMTime(char *gmt, time_t plus_sec);

int       qCheckFile(char *filename);
int       qCatFile(char *filename);
char      *qReadFile(char *filename, int *size);
int       qSaveStr(char *sp, int spsize, char *filename, char *mode);
char      *qfGetLine(FILE *fp);

int       qDownload(char *filename);
int       qDownloadMime(char *filename, char *mime);
long      qFileSize(char *filename);
void      qRedirect(char *url);

int       qReadCounter(char *filename);
int       qSaveCounter(char *filename, int number);
int       qUpdateCounter(char *filename, int number);

int       qCheckEmail(char *email);
int       qCheckURL(char *url);

char      *qRemoveSpace(char *str);

int       qStr09AZaz(char *str);
char      *qStrupr(char *str);
char      *qStrlwr(char *str);
char      *qStristr(char *big, char *small);
int       qStricmp(char *s1, char *s2);
int       qStrincmp(char *s1, char *s2, size_t len);
char      *qitocomma(int value);
char      *qStrReplace(char *mode, char *srcstr, char *tokstr, char *word);

#ifdef __cplusplus
}
#endif

#endif
