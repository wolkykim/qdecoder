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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/file.h>
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

	for (len = 0; ((str[len] != stop) && (str[len])); len++);
	word = (char *)malloc(sizeof(char) * (len + 1));

	for (i = 0; i < len; i++)word[i] = str[i];
	word[i] = '\0';

	if (str[len])len++;
	for (i = len; str[i]; i++)str[i - len] = str[i];
	str[i - len] = '\0';

	return (word);
}

/*********************************************
** Usage : This function is perfectly same as fgets();
**********************************************/
char *_fgets(char *str, int size, FILE *stream) {
	int c;
	char *ptr;

	for (ptr = str; size > 1; size--) {
		c = fgetc(stream);
		if (c == EOF) break;
		*ptr++ = (char)c;
		if (c == '\n') break;
	}

	*ptr = '\0';
	if (strlen(str) == 0) return NULL;

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
