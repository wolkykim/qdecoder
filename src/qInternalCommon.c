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
** Usage : _x2c(HEX up character, HEX low character);
** Return: Hex value which is changed.
** Do    : Change two hex character to one hex value.
**********************************************/
char _x2c(char hex_up, char hex_low) {
  char digit;

  digit = 16 * (hex_up >= 'A' ? ((hex_up & 0xdf) - 'A') + 10 : (hex_up - '0'));
  digit += (hex_low >= 'A' ? ((hex_low & 0xdf) - 'A') + 10 : (hex_low - '0'));

  return (digit);
}


/**********************************************
** Usage : _makeword(source string, stop character);
** Return: Pointer of Parsed string.
** Do    : It copy source string before stop character.
**         The pointer of source string direct after stop character.
**********************************************/
char *_makeword(char *str, char stop) {
  char *word;
  int  len, i;

  for(len = 0; ((str[len] != stop) && (str[len])); len++);
  word = (char *)malloc(sizeof(char) * (len + 1));

  for(i = 0; i < len; i++)word[i] = str[i];
  word[i] = '\0';

  if(str[len])len++;
  for(i = len; str[i]; i++)str[i - len] = str[i];
  str[i - len] = '\0';

  return (word);
}

/*********************************************
** Usage : _strtok2(string, token stop string, return stop character);
** Do    : Find token string. (usage like strtok())
** Return: Pointer of token & character of stop.
**********************************************/
char *_strtok2(char *str, char *token, char *retstop) {
  static char *tokensp, *tokenep;
  int i, j;

  if(str != NULL) tokensp = tokenep = str;
  else tokensp = tokenep;

  for(i = strlen(token);*tokenep;tokenep++) {
    for(j = 0; j < i; j++) {
      if(*tokenep == token[j]) {
        *retstop = token[j];
        *tokenep = '\0';
        tokenep++;
        return tokensp;
      }
    }
  }

  *retstop = '\0';
  if(tokensp != tokenep) return tokensp;
  return NULL;
}

/*********************************************
** Usage : This function is perfectly same as fgets();
**********************************************/
char *_fgets(char *str, int size, FILE *stream) {
  int c;
  char *ptr;

  for(ptr = str; size > 1; size--) {
    c = fgetc(stream);
    if(c == EOF) break;
    *ptr++ = (char)c;
    if(c == '\n') break;
  }

  *ptr = '\0';
  if(strlen(str) == 0) return NULL;

  return str;
}

/*********************************************
** Usage : apply an advisory lock on an open file.
**********************************************/
int _flockopen(FILE *fp) {
#ifdef _WIN32
  return 0;
#else
#ifdef HAVE_FLOCK
  return flock(fileno(fp), LOCK_EX);
#else
  return 0;
#endif
#endif
}

/*********************************************
** Usage : remove an advisory lock on an open file.
**********************************************/
int _flockclose(FILE *fp) {
#ifdef _WIN32
  return 0;
#else
#ifdef HAVE_FLOCK
  return flock(fileno(fp), LOCK_UN);
#else
  return 0;
#endif
#endif
}
