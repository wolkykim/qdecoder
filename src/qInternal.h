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

#ifndef _QINTERNAL_H
#define _QINTERNAL_H

char    _x2c(char hex_up, char hex_low);
char    *_makeword(char *str, char stop);
char    *_fgets(char *str, int size, FILE *stream);
int     _flockopen(FILE *fp);
int     _flockclose(FILE *fp);

/**********************************************
** Internal Definition
**********************************************/
#define QDECODER_PRIVATEKEY	"qDecoder-by-Seung_young_Kim"

#endif	/* _QINTERNAL_H */
