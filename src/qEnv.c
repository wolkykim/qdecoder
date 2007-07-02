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

#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Usage : qGetenvDefault(default string, environment string name);
** Return: Environment string or 'nullstr'.
** Do    : Get environment string.
**         When it does not find 'envname', it will return 'nullstr'.
**********************************************/
char *qGetenvDefault(char *nullstr, char *envname) {
	char *envstr;

	if ((envstr = getenv(envname)) != NULL) return envstr;

	return nullstr;
}
