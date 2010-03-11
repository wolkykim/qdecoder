/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE QDECODER PROJECT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE QDECODER PROJECT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "qDecoder.h"

void stringSample(void) {
	// get new obstack
	Q_OBSTACK *obstack = qObstack();

	// stack
	obstack->growStr(obstack, "(string)");			// for string
	obstack->growStrf(obstack, "(stringf:%s)", "hello");	// for formatted string
	obstack->grow(obstack, "(object)", sizeof("(object)"));	// same effects as above but this can be used for object or binary

	// final
	char *final = (char *)obstack->getFinal(obstack, NULL);

	// print out
	printf("[String Sample]\n");
	printf("Final string = %s\n", final);
	printf("Total Size = %zu, Number of Objects = %d\n", obstack->getSize(obstack), obstack->getNum(obstack));

	// free obstack
	free(final);
	obstack->free(obstack);
}

void objectSample(void) {
	// sample object
	struct sampleobj {
		int	num;
		char	str[10];
	} obj, *final;

	// get new obstack
	Q_OBSTACK *obstack = qObstack();

	// stack
	int i;
	for(i = 0; i < 3; i++) {
		// filling object with sample data
		obj.num  = i;
		sprintf(obj.str, "hello%d", i);

		// stack
		obstack->grow(obstack, (void *)&obj, sizeof(struct sampleobj));
	}

	// final
	final = (struct sampleobj *)obstack->getFinal(obstack, NULL);

	// print out
	printf("[Object Sample]\n");
	for(i = 0; i < obstack->getNum(obstack); i++) {
		printf("Object%d final = #%d, %s\n", i+1, final[i].num, final[i].str);
	}
	printf("Total Size = %zu, Number of Objects = %d\n", obstack->getSize(obstack), obstack->getNum(obstack));

	// free obstack
	free(final);
	obstack->free(obstack);
}

int main(void) {
	qCgiResponseSetContentType(NULL, "text/plain");

	stringSample();
	printf("\n");
	objectSample();

	return 0;
}
