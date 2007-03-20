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

char    _x2c(char hex_up, char hex_low);
char    *_makeword(char *str, char stop);
char    *_strtok2(char *str, char *token, char *retstop);
char    *_fgetstring(char *buf, int maxlen, FILE *fp);

Q_Entry *_EntryAddStr(Q_Entry *first, char *name, char *value);
char    *_EntryValue(Q_Entry *first, char *name);
int     _EntryiValue(Q_Entry *first, char *name);
void    _EntryPrint(Q_Entry *first);
void    _EntryFree(Q_Entry *first);