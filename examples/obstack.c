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

void stringSample(void) {
	// get new obstack
	Q_OBSTACK *obstack = qObstackInit();

	// stack
	qObstackGrowStr(obstack, "(string)");			// for string
	qObstackGrowStrf(obstack, "(stringf:%s)", "hello");	// for formatted string
	qObstackGrow(obstack, "(object)", sizeof("(object)"));	// same effects as above but this can be used for object or binary

	// final
	char *final = (char *)qObstackFinish(obstack);

	// print out
	printf("[String Sample]\n");
	printf("Final string = %s\n", final);
	printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));

	// free obstack
	qObstackFree(obstack);
}

void objectSample(void) {
	// sample object
	struct sampleobj {
		int	num;
		char	str[10];
	} obj, *final;

	// get new obstack
	Q_OBSTACK *obstack = qObstackInit();

	// stack
	int i;
	for(i = 0; i < 3; i++) {
		// filling object with sample data
		obj.num  = i;
		sprintf(obj.str, "hello%d", i);

		// stack
		qObstackGrow(obstack, (void *)&obj, sizeof(struct sampleobj));
	}

	// final
	final = (struct sampleobj *)qObstackFinish(obstack);

	// print out
	printf("[Object Sample]\n");
	for(i = 0; i < qObstackGetNum(obstack); i++) {
		printf("Object%d final = %d, %s\n", i+1, final[i].num, final[i].str);
	}
	printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));

	// free obstack
	qObstackFree(obstack);
}

int main(void) {
	Q_ENTRY *req = qCgiRequestParse(NULL);
	qCgiResponseSetContentType(req, "text/plain");

	stringSample();
	printf("\n");
	objectSample();

	return 0;
}
