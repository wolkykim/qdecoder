/*
 * Copyright 2008 The qDecoder Project. All rights reserved.
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
 */

/**
 * @file qObstack.c qDecoder implementation of GNU obstack
 *
 * An qObstack is a pool of memory containing a stack of objects.
 * An qObstack can contain any number of objects of any size.
 *
 * @code
 *   [Code sample - String]
 *   Q_OBSTACK *obstack;
 *
 *   char *final;
 *   char *tmp = "CDE";
 *
 *   // get new obstack
 *   obstack = qObstackInit();
 *
 *   // stack
 *   qObstackGrowStr(obstack, "AB");		// no need to supply size
 *   qObstackGrowStrf(obstack, "%s", tmp);	// for formatted string
 *   qObstackGrow(obstack, "FGH", 3);	// same effects as above but this can
 *   					// be used for object or binary
 *
 *   // final
 *   final = (char *)qObstackFinish(obstack);
 *
 *
 *   // print out
 *   printf("Final string = %s\n", final);
 *   printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));
 *
 *   qObstackFree(obstack);
 *
 *   [Result]
 *   Final string = ABCDEFGH
 *   Total Size = 8, Number of Objects = 3
 * @endcode
 *
 * @code
 *   [Code sample - Object]
 *   struct sampleobj {
 *   	int	num;
 *   	char	str[10];
 *   };
 *
 *   Q_OBSTACK *obstack;
 *   int i;
 *
 *   // sample object
 *   struct sampleobj obj;
 *   struct sampleobj *final;
 *
 *   // get new obstack
 *   obstack = qObstackInit();
 *
 *   // stack
 *   for(i = 0; i < 3; i++) {
 *   	// filling object with sample data
 *   	obj.num  = i;
 *   	sprintf(obj.str, "hello%d", i);
 *
 *   	// stack
 *   	qObstackGrow(obstack, (void *)&obj, sizeof(struct sampleobj));
 *   }
 *
 *   // final
 *   final = (struct sampleobj *)qObstackFinish(obstack);
 *
 *   // print out
 *   qContentType("text/plain");
 *   for(i = 0; i < qObstackGetNum(obstack); i++) {
 *   	printf("Object%d final = %d, %s\n", i+1, final[i].num, final[i].str);
 *   }
 *   printf("Total Size = %d, Number of Objects = %d\n", qObstackGetSize(obstack), qObstackGetNum(obstack));
 *
 *   qObstackFree(obstack);
 *
 *   [Result]
 *   Object1 final = 0, hello0
 *   Object2 final = 1, hello1
 *   Object3 final = 2, hello2
 *   Total Size = 48, Number of Objects = 3
 * @endcode
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"

/**
 * Initialize object-stack
 *
 * @return		a pointer of Q_OBSTACK structure
 *
 * @code
 *   // allocate memory
 *   Q_OBSTACK *obstack = qObstackInit();
 *   qObstackFree(obstack);
 * @endcode
 */
Q_OBSTACK *qObstackInit(void) {
	Q_OBSTACK *obstack;

	obstack = (Q_OBSTACK *)malloc(sizeof(Q_OBSTACK));
	if(obstack == NULL) return NULL;

	memset((void *)obstack, 0, sizeof(Q_OBSTACK));
	obstack->stack = qEntry();
	if(obstack->stack == NULL) {
		free(obstack);
		return NULL;
	}

	return obstack;
}

/**
 * Stack object
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param object	a pointer of object data
 * @param size		size of object
 *
 * @return		true if successful, otherwise returns false
 */
bool qObstackGrow(Q_OBSTACK *obstack, const void *object, size_t size) {
	if(obstack == NULL || object == NULL || size <= 0) return false;
	return obstack->stack->put(obstack->stack, "", object, size, false);
}

/**
 * Stack string
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param str		a pointer of string
 *
 * @return		true if successful, otherwise returns false
 */
bool qObstackGrowStr(Q_OBSTACK *obstack, const char *str) {
	return qObstackGrow(obstack, (void *)str, strlen(str));
}

/**
 * Stack formatted string
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param format	string format
 *
 * @return		true if successful, otherwise returns false
 */
bool qObstackGrowStrf(Q_OBSTACK *obstack, const char *format, ...) {
	if(obstack == NULL) return false;

	char str[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(str, sizeof(str), format, arglist);
	va_end(arglist);

	return qObstackGrow(obstack, (void *)str, strlen(str));
}

/**
 * Stack formatted string
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		true if successful, otherwise returns false
 */
void *qObstackFinish(Q_OBSTACK *obstack) {
	if(obstack == NULL) return NULL;

	if(obstack->final != NULL) free(obstack->final);
	obstack->final = obstack->stack->merge(obstack->stack, NULL);
	if(obstack->final == NULL) obstack->final = strdup("");

	return obstack->final;
}

/**
 * Finalize objects and get merged single final object pointer
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		a pointer of finally merged object, otherwise returns NULL
 */
void *qObstackGetFinal(Q_OBSTACK *obstack) {
	if(obstack == NULL) return NULL;
	if(obstack->final == NULL) qObstackFinish(obstack);
	return obstack->final;
}

/**
 * Get stacked objects size
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		a pointer of finally merged object, otherwise returns NULL
 */
size_t qObstackGetSize(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->size;
}

/**
 * Get the number of stacked objects
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		a number of stacked objects, otherwise returns 0
 */
int qObstackGetNum(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->getNum(obstack->stack);
}

/**
 * De-allocate obstack
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		true if successful, otherwise returns false
 */
bool qObstackFree(Q_OBSTACK *obstack) {
	if(obstack == NULL) return false;
	obstack->stack->free(obstack->stack);
	if(obstack->final != NULL) free(obstack->final);
	free(obstack);
	return true;
}

#endif /* DISABLE_DATASTRUCTURE */
