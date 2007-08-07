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
	Q_OBSTACK *obstack;

	char *final;
	int *tmp = "CDE";

	obstack = qObstackInit();		// get new obstack

	qObstackGrowStr(obstack, "AB");		// no need to supply size
	qObstackGrowStrf(obstack, "%s", tmp);	// for formatted string
	qObstackGrow(obstack, "FGH", 3);	// same effects as above but this can
						// be used for object or binary

	qContentType("text/plain");
	final = (char *)qObstackFinish(obstack);
	printf("%s\n", final);

	qObstackFree(obstack);
}
