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
** Usage : qCheckFile(filename);
** Return: If file exist, return 1. Or return 0.
** Do    : Check filethat file is existGet environment of CGI.
**********************************************/
int qCheckFile(char *filename) {
  FILE *fp;
  fp = fopen(filename, "rb");
  if(fp == NULL) return 0;
  fclose(fp);
  return 1;
}

/**********************************************
** Usage : qCatFile(filename);
** Return: Success number of characters, Fail -1.
** Do    : Stream out file.
**********************************************/
int qCatFile(char *filename) {
  FILE *fp;
  int c, counter;

  if((fp = fopen(filename, "rb")) == NULL) return -1;
  for(counter = 0; (c = fgetc(fp)) != EOF; counter++) {
    printf("%c", c);
  }

  fclose(fp);
  return counter;
}

/**********************************************
** Usage : qReadFile(filename, string pointer);
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
  if((fp = fopen(filename, "rb")) == NULL) return NULL;

  sp = (char *)malloc(fstat.st_size + 1);
  for(tmp = sp, i = 0; (c = fgetc(fp)) != EOF; tmp++, i++) *tmp = (char)c;
  *tmp = '\0';

  if(fstat.st_size != i) qError("qReadFile: Size(File:%d, Readed:%s) mismatch.", fstat.st_size, i);
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
  if((fp = fopen(filename, mode)) == NULL) return -1;
  for(i = 0; i < spsize; i++) fputc(*sp++, fp);
  fclose(fp);

  return i;
}

/*********************************************
** Usage : qfGetLine(file pointer);
** Return: Success string pointer, End of file NULL.
** Do    : Read one line from file pointer and alocate
**         memory to save string. And there is no limit
**         about length.
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
      int  i;

      memsize *= 2;

      /* Here, we do not use realloc(). Because sometimes it is unstable. */
      stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
      if(stringtmp == NULL) qError("qfGetLine(): Memory allocation fail.");
      for(i = 0; i < c_count; i++) stringtmp[i] = string[i];
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