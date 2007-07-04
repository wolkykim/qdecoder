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
#include "qDecoder.h"

int main(void) {
	char *value;

	/* Parse (GET/COOKIE/POST) queries. This call is not necessary because
	   it will be called automatically when it is needed. */
	qDecoder();

	/* Print out context type */
	qContentType("text/html");

	/* If the query is not found, the variable will be set default string.
	   Also, you can use qValueDefault() or qValueNotEmpty() instead. */
	if (!(value = qValue("query"))) value = "";

	printf("You entered: <b>%s</b>\n", value);

	/* Do not free variables directly using free() function such like free(value)
	   You must use qFree() or qFreeAll() to deallocate memories */
	qFree();
	return 0;
}

