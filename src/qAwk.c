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
** Static Values Definition used only internal
**********************************************/

static FILE *_awkfp = NULL;
static char _awksep = ' ';


/**********************************************
** Usage : qAwkOpen(filename, field-separator);
** Return: Success 1, Fail 0.
** Do    : Open file to scan pattern. (similar to unix command awk)
**
** ex) qAwkOpen("source.dat", ':');
**********************************************/
int qAwkOpen(char *filename, char separator) {
  if(_awkfp != NULL) qError("qAwkOpen(): There is already opened handle.");
  if((_awkfp = fopen(filename, "rt")) == NULL) return 0;
  _awksep = separator;
  return 1;
}

/**********************************************
** Usage : qAwkNext(array);
** Return: Success number of field, End of file -1.
** Do    : Scan one line. (Unlimited line length)
**
** ex) char array[10][256];
**     qAwkNext(array);
**********************************************/
int qAwkNext(char array[][256]) {
  char *buf;
  char *bp1, *bp2;
  int i, exitflag;

  if(_awkfp == NULL) qError("qAwkNext(): There is no opened handle.");
  if((buf = qRemoveSpace(qfGetLine(_awkfp))) == NULL) return -1;

  for(i = exitflag = 0, bp1 = bp2 = buf; exitflag == 0; i++) {
    for(; *bp2 != _awksep && *bp2 != '\0'; bp2++);
    if(*bp2 == '\0') exitflag = 1;
    *bp2 = '\0';
    strcpy(array[i], bp1);
    bp1 = ++bp2;
  }
  free(buf);
  return i;
}

/**********************************************
** Usage : qAwkClose();
** Return: Success 1, Fail 0.
** Do    : Close file.
**********************************************/
int qAwkClose(void) {
  if(_awkfp == NULL) return 0;
  fclose(_awkfp);
  _awkfp = NULL;
  _awksep = ' ';

  return 1;
}

