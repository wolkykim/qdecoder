/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

#include "stdafx.h"
#include "URLencode.h"

char _x2c(char hex_up, char hex_low);


/**********************************************
** Usage : qURLencode(string to encode);
** Return: Pointer of encoded str which is memory allocated.
** Do    : Encode string.
**********************************************/
char *qURLencode(char *str) {
  char *encstr, buf[2+1];
  unsigned char c;
  int i, j;

  if(str == NULL) return NULL;
  if((encstr = (char *)malloc((strlen(str) * 3) + 1)) == NULL) return NULL;

  for(i = j = 0; str[i]; i++) {
    c = (unsigned char)str[i];
         if((c >= '0') && (c <= '9')) encstr[j++] = c;
    else if((c >= 'A') && (c <= 'Z')) encstr[j++] = c;
    else if((c >= 'a') && (c <= 'z')) encstr[j++] = c;
    else if((c == '@') || (c == '.') || (c == '/') || (c == '\\')
         || (c == '-') || (c == '_') || (c == ':') ) encstr[j++] = c;
    else {
      sprintf(buf, "%02x", c);
      encstr[j++] = '%';
      encstr[j++] = buf[0];
      encstr[j++] = buf[1];
    }
  }
  encstr[j] = '\0';

  return encstr;
}

/**********************************************
** Usage : qURLdecode(query pointer);
** Return: Pointer of query string.
** Do    : Decode query string.
**********************************************/
char *qURLdecode(char *str) {
  int i, j;

  if(!str) return NULL;
  for(i = j = 0; str[j]; i++, j++) {
    switch(str[j]) {
      case '+':{
        str[i] = ' ';
        break;
      }
      case '%':{
        str[i] = _x2c(str[j + 1], str[j + 2]);
        j += 2;
        break;
      }
      default:{
        str[i] = str[j];
        break;
      }
    }
  }
  str[i]='\0';

  return str;
}


/**********************************************
** You don't need to use below functions.
** It is just used internally.
**********************************************/

char _x2c(char hex_up, char hex_low){
  char digit;
  digit = 16 * (hex_up >= 'A' ? ((hex_up & 0xdf) - 'A') + 10 : (hex_up - '0'));
  digit += (hex_low >= 'A' ? ((hex_low & 0xdf) - 'A') + 10 : (hex_low - '0'));

  return (digit);
}
