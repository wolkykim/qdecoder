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
 *   // get new obstack
 *   Q_OBSTACK *obstack = qObstack();
 *
 *   // stack
 *   obstack->growStr(obstack, "AB");		// no need to supply size
 *   obstack->growStrf(obstack, "%s", "CDE");	// for formatted string
 *   obstack->grow(obstack, "FGH", 3);		// same effects as above but this can
 *   						// be used for object or binary
 *
 *   // final
 *   char *final = (char *)obstack->getFinal(obstack, NULL);
 *
 *   // print out
 *   printf("Final string = %s\n", final);
 *   printf("Total Size = %d, Number of Objects = %d\n", obstack->getSize(obstack), obstack->getNum(obstack));
 *
 *   free(final);
 *   obstack->free(obstack);
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
 *   // get new obstack
 *   Q_OBSTACK *obstack = qObstack();
 *
 *   // stack
 *   int i;
 *   struct sampleobj obj;
 *   for(i = 0; i < 3; i++) {
 *   	// filling object with sample data
 *   	obj.num  = i;
 *   	sprintf(obj.str, "hello%d", i);
 *
 *   	// stack
 *   	obstack->grow(obstack, (void *)&obj, sizeof(struct sampleobj));
 *   }
 *
 *   // final
 *   struct sampleobj *final = (struct sampleobj *)obstack->getFinal(obstack, NULL);
 *
 *   // print out
 *   qContentType("text/plain");
 *   for(i = 0; i < obstack->getNum(obstack); i++) {
 *   	printf("Object%d final = %d, %s\n", i+1, final[i].num, final[i].str);
 *   }
 *   printf("Total Size = %d, Number of Objects = %d\n", obstack->getSize(obstack), obstack->getNum(obstack));
 *
 *   obstack->free(obstack);
 *
 *   [Result]
 *   Object1 final = 0, hello0
 *   Object2 final = 1, hello1
 *   Object3 final = 2, hello2
 *   Total Size = 48, Number of Objects = 3
 * @endcode
 *
 * @note
 * Use "--enable-threadsafe" configure script option to use under multi-threaded environments.
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/*
 * Member method protos
 */
#ifndef _DOXYGEN_SKIP
static bool _grow(Q_OBSTACK *obstack, const void *object, size_t size);
static bool _growStr(Q_OBSTACK *obstack, const char *str);
static bool _growStrf(Q_OBSTACK *obstack, const char *format, ...);
static void *_getFinal(Q_OBSTACK *obstack, size_t *size);
static ssize_t _writeFinal(Q_OBSTACK *obstack, int nFd);
static size_t _getSize(Q_OBSTACK *obstack);
static int _getNum(Q_OBSTACK *obstack);
static bool _free(Q_OBSTACK *obstack);
#endif

/**
 * Initialize object-stack
 *
 * @return		a pointer of Q_OBSTACK structure
 *
 * @code
 *   // allocate memory
 *   Q_OBSTACK *obstack = qObstack();
 *   obstack->free(obstack);
 * @endcode
 */
Q_OBSTACK *qObstack(void) {
	Q_OBSTACK *obstack;

	obstack = (Q_OBSTACK *)malloc(sizeof(Q_OBSTACK));
	if(obstack == NULL) return NULL;

	memset((void *)obstack, 0, sizeof(Q_OBSTACK));
	obstack->stack = qEntry();
	if(obstack->stack == NULL) {
		free(obstack);
		return NULL;
	}

	// methods
	obstack->grow		= _grow;
	obstack->growStr	= _growStr;
	obstack->growStrf	= _growStrf;
	obstack->getFinal	= _getFinal;
	obstack->writeFinal	= _writeFinal;
	obstack->getSize	= _getSize;
	obstack->getNum		= _getNum;
	obstack->free		= _free;

	return obstack;
}

/**
 * Q_OBSTACK->grow(): Stack object
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param object	a pointer of object data
 * @param size		size of object
 *
 * @return		true if successful, otherwise returns false
 */
static bool _grow(Q_OBSTACK *obstack, const void *object, size_t size) {
	if(obstack == NULL || object == NULL || size <= 0) return false;
	return obstack->stack->put(obstack->stack, "", object, size, false);
}

/**
 * Q_OBSTACK->growStr(): Stack string
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param str		a pointer of string
 *
 * @return		true if successful, otherwise returns false
 */
static bool _growStr(Q_OBSTACK *obstack, const char *str) {
	return _grow(obstack, (void *)str, strlen(str));
}

/**
 * Q_OBSTACK->growStrf(): Stack formatted string
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param format	string format
 *
 * @return		true if successful, otherwise returns false
 */
static bool _growStrf(Q_OBSTACK *obstack, const char *format, ...) {
	if(obstack == NULL) return false;

	char *str;
	DYNAMIC_VSPRINTF(str, format);
	if(str == NULL) return false;

	bool ret = _grow(obstack, (void *)str, strlen(str));
	free(str);

	return ret;
}

/**
 * Q_OBSTACK->getFinal(): Get merged single final object
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param size		if size is not NULL, merged object size will be stored.
 *
 * @return		a pointer of finally merged object(malloced), otherwise returns NULL
 *
 * @note
 * User should take care of malloced object.
 */
static void *_getFinal(Q_OBSTACK *obstack, size_t *size) {
	if(obstack == NULL) return NULL;

	void *final = obstack->stack->merge(obstack->stack, size);
	if(final == NULL) return NULL;

	return final;
}

/**
 * Q_OBSTACK->writeFinal(): Write out merged final data to the file descriptor
 *
 * @param obstack	a pointer of Q_OBSTACK
 * @param nFd		a file descriptor for writing out.
 *
 * @return		the number of bytes written, otherwise returns -1;
 *
 * @note
 * It writes out merged data with only one system call.
 * To do this, it uses writev(), so the operation does not cost much.
 */
static ssize_t _writeFinal(Q_OBSTACK *obstack, int nFd) {
	if(obstack == NULL) return -1;

	// get obstack info
	size_t size = _getSize(obstack);
	int num = _getNum(obstack);
	if(size == 0 || num == 0) return 0;

	// allocate iovector
	struct iovec *vectors = (struct iovec *)malloc(sizeof(struct iovec) * num);
	if(vectors == NULL) return -1;

	// map data to vector
	Q_ENTRY *entry = obstack->stack;
	Q_LOCK_ENTER(entry->qlock);
	Q_NLOBJ_T *obj;
	int objcnt;
	for(objcnt = 0, obj = entry->first; obj; obj = obj->next, objcnt++) {
		vectors[objcnt].iov_base = obj->data;
		vectors[objcnt].iov_len = obj->size;
	}
	Q_LOCK_LEAVE(entry->qlock);

	// double check
	if(objcnt != num) {
		free(vectors);
		return -1;
	}

	// write
	ssize_t written = writev(nFd, vectors, objcnt);

	// free
	free(vectors);

	return written;
}

/**
 * Q_OBSTACK->getSize(): Get stacked objects size
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		a pointer of finally merged object, otherwise returns NULL
 */
static size_t _getSize(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->size;
}

/**
 * Q_OBSTACK->getNum(): Get the number of stacked objects
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		a number of stacked objects, otherwise returns 0
 */
static int _getNum(Q_OBSTACK *obstack) {
	if(obstack == NULL) return 0;
	return obstack->stack->getNum(obstack->stack);
}

/**
 * Q_OBSTACK->free(): De-allocate obstack
 *
 * @param obstack	a pointer of Q_OBSTACK
 *
 * @return		true if successful, otherwise returns false
 */
static bool _free(Q_OBSTACK *obstack) {
	if(obstack == NULL) return false;
	obstack->stack->free(obstack->stack);
	free(obstack);
	return true;
}

#endif /* DISABLE_DATASTRUCTURE */
