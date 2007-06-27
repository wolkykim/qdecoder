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

char    _x2c(char hex_up, char hex_low);
char    *_makeword(char *str, char stop);
char    *_fgets(char *str, int size, FILE *stream);
int     _flockopen(FILE *fp);
int     _flockclose(FILE *fp);

Q_ENTRY *_EntryAdd(Q_ENTRY *first, char *name, char *value, int flag);
Q_ENTRY *_EntryRemove(Q_ENTRY *first, char *name);
char    *_EntryValue(Q_ENTRY *first, char *name);
char    *_EntryValueLast(Q_ENTRY *first, char *name);
int     _EntryiValue(Q_ENTRY *first, char *name);
int	_EntryiValueLast(Q_ENTRY *first, char *name);
int     _EntryNo(Q_ENTRY *first, char *name);
Q_ENTRY	*_EntryReverse(Q_ENTRY *first);
int     _EntryPrint(Q_ENTRY *first);
void    _EntryFree(Q_ENTRY *first);
int     _EntrySave(Q_ENTRY *first, char *filename);
Q_ENTRY *_EntryLoad(char *filename);

/**********************************************
** Internal Definition
**********************************************/
#define QDECODER_PRIVATEKEY	"qDecoder-by-Seung_young_Kim"
