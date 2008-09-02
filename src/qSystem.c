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

/**
 * @file qEnv.c Environment Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"

/**********************************************
** Usage : qGetenvDefault(default string, environment string name);
** Return: Environment string or 'nullstr'.
** Do    : Get environment string.
**         When it does not find 'envname', it will return 'nullstr'.
**********************************************/
const char *qSysGetEnv(const char *envname, const char *nullstr) {
	const char *envstr = getenv(envname);
	if (envstr != NULL) return envstr;
	return nullstr;
}

/**********************************************
** Usage : qCmd(external command);
** Return: Execution output, File not found NULL.
**********************************************/
char *qSysCmd(const char *cmd) {
	FILE *fp = popen(cmd, "r");
	if (fp == NULL) return NULL;
	char *str = qFileRead(fp, NULL);
	pclose(fp);

	if(str == NULL) str = strdup("");
	return str;
}
