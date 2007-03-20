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
** Usage : qfopen(path, mode);
** Return: Same as fclose().
** Do    : Open file with file lock.
**********************************************/
FILE *qfopen(char *path, char *mode) {
  FILE *stream;

  if((stream = fopen(path, mode)) == NULL) return NULL;
  _flockopen(stream);
  return stream;
}

/**********************************************
** Usage : qfclose(stream);
** Return: Same as fclose().
** Do    : Close the file stream which is opened by qfopen().
**********************************************/
int qfclose(FILE *stream) {
  _flockclose(stream);
  return fclose(stream);
}

/**********************************************
**Usage : qCheckFile(filename);
** Return: If file exist, return 1. Or return 0.
** Do    : Check filethat file is existGet environment of CGI.
**********************************************/
int qCheckFile(char *format, ...) {
  struct stat finfo;
  char filename[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(filename, format, arglist);
  if(strlen(filename) + 1 > sizeof(filename) || status == EOF) qError("qCheckFile(): File name is too long or invalid.");
  va_end(arglist);

  if(stat(filename, &finfo) < 0) return 0;

  return 1;
}

/**********************************************
** Usage : qCatFile(filename);
** Return: Success number of characters, Fail -1.
** Do    : Stream out file.
**********************************************/
int qCatFile(char *format, ...) {
  FILE *fp;
  char filename[1024];
  int status;
  va_list arglist;
  int c, counter;

  va_start(arglist, format);
  status = vsprintf(filename, format, arglist);
  if(strlen(filename) + 1 > sizeof(filename) || status == EOF) qError("qCatFile(): File name is too long or invalid.");
  va_end(arglist);

#ifdef _WIN32
  setmode(fileno(stdin), _O_BINARY);
  setmode(fileno(stdout), _O_BINARY);
#endif

  if((fp = qfopen(filename, "rb")) == NULL) return -1;
  for(counter = 0; (c = fgetc(fp)) != EOF; counter++) {
    putc(c, stdout);
  }

  qfclose(fp);
  return counter;
}

/**********************************************
** Usage : qReadFile(filename, integer pointer to store file size);
** Return: Success stream pointer, Fail NULL.
** Do    : Read file to malloced memory.
**********************************************/
char *qReadFile(char *filename, int *size) {
  FILE *fp;
  struct stat fstat;
  char *sp, *tmp;
  int c, i;

  if(size != NULL) *size = 0;
  if(stat(filename, &fstat) < 0) return NULL;
  if((fp = qfopen(filename, "rb")) == NULL) return NULL;

  sp = (char *)malloc(fstat.st_size + 1);
  for(tmp = sp, i = 0; (c = fgetc(fp)) != EOF; tmp++, i++) *tmp = (char)c;
  *tmp = '\0';

  if(fstat.st_size != i) qError("qReadFile: Size(File:%d, Readed:%d) mismatch.", fstat.st_size, i);
  fclose(fp);
  if(size != NULL) *size = i;
  return sp;
}

/**********************************************
** Usage : qSaveStr(string pointer, string size, filename, mode)
** Return: Success number bytes stored, File open fail -1.
** Do    : Store string to file.
**********************************************/
int qSaveStr(char *sp, int spsize, char *filename, char *mode) {
  FILE *fp;
  int i;
  if((fp = qfopen(filename, mode)) == NULL) return -1;
  for(i = 0; i < spsize; i++) fputc(*sp++, fp);
  qfclose(fp);

  return i;
}

/*********************************************
** Usage : qfGetLine(file pointer);
** Return: Success string pointer, End of file NULL.
** Do    : Read one line from file pointer without length
**         limitation. String will be saved into dynamically
**         allocated memory. The newline, if any, is retained.
**********************************************/
char *qfGetLine(FILE *fp) {
  int memsize;
  int c, c_count;
  char *string = NULL;

  for(memsize = 1024, c_count = 0; (c = fgetc(fp)) != EOF;) {
    if(c_count == 0) {
      string = (char *)malloc(sizeof(char) * memsize);
      if(string == NULL) qError("qfGetLine(): Memory allocation fail.");
    }
    else if(c_count == memsize - 1) {
      char *stringtmp;

      memsize *= 2;

      /* Here, we do not use realloc(). Because sometimes it is unstable. */
      stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
      if(stringtmp == NULL) qError("qfGetLine(): Memory allocation fail.");
      memcpy(stringtmp, string, c_count);
      free(string);
      string = stringtmp;
    }
    string[c_count++] = (char)c;
    if((char)c == '\n') break;
  }

  if(c_count == 0 && c == EOF) return NULL;

  string[c_count] = '\0';

  return string;
}

/**********************************************
** Usage : qFileSize(filename);
** Return: Size of file in byte, File not found -1.
**********************************************/
long qFileSize(char *filename) {
  struct stat finfo;

  if(stat(filename, &finfo) < 0) return -1;

  return finfo.st_size;
}

