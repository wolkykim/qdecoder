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
 * @file qQueue.c Circular-Queue Data Structure API
 *
 * @code
 *   ----[Sample codes]----
 *   struct myobj {
 *     int integer;
 *     char string[255+1];
 *   };
 *
 *   int main(void) {
 *     Q_QUEUE *queue = qQueue(100, sizeof(struct myobj));
 *
 *     // push object
 *     int i;
 *     for(i = 1; ; i++) {
 *       // set sample object
 *       struct myobj obj;
 *       obj.integer = i;
 *       sprintf(obj.string, "hello world %d", i);
 *
 *       // push object
 *       if(queue->push(queue, &obj) == false) break;
 *
 *       // print debug info
 *       printf("Push     : %d, %s\n", obj.integer, obj.string);
 *     }
 *
 *     // pop object from head & tail
 *     while(true) {
 *       struct myobj pop;
 *       if(queue->popFirst(queue, &pop) == false) break;
 *       printf("PopFirst : %d, %s\n", pop.integer, pop.string);
 *
 *       if(queue->popLast(queue, &pop) == false) break;
 *       printf("PopLast  : %d, %s\n", pop.integer, pop.string);
 *     }
 *     return 0;
 *   }
 *
 *   ----[Results]----
 *   Push     : 1, hello world 1
 *   Push     : 2, hello world 2
 *   Push     : 3, hello world 3
 *   Push     : 4, hello world 4
 *   Push     : 5, hello world 5
 *   Push     : 6, hello world 6
 *   Push     : 7, hello world 7
 *   Push     : 8, hello world 8
 *   Push     : 9, hello world 9
 *   Push     : 10, hello world 10
 *   PopFirst : 1, hello world 1
 *   PopLast  : 10, hello world 10
 *   PopFirst : 2, hello world 2
 *   PopLast  : 9, hello world 9
 *   PopFirst : 3, hello world 3
 *   PopLast  : 8, hello world 8
 *   PopFirst : 4, hello world 4
 *   PopLast  : 7, hello world 7
 *   PopFirst : 5, hello world 5
 *   PopLast  : 6, hello world 6
 * @endcode
 *
 * @note
 * Use "--enable-threadsafe" configure script option to use under multi-threaded environments.
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/*
 * Member method protos
 */
#ifndef _DOXYGEN_SKIP
static bool _push(Q_QUEUE *queue, const void *object);
static void *_popFirst(Q_QUEUE *queue, bool remove);
static void *_popLast(Q_QUEUE *queue, bool remove);
static int _getNum(Q_QUEUE *queue);
static int _getAvail(Q_QUEUE *queue);
static void _truncate(Q_QUEUE *queue);
static void _free(Q_QUEUE *queue);
static void _freeUsrmem(Q_QUEUE *queue);
#endif

/**
 * Initialize queue
 *
 * @param max		a number of maximum internal slots
 * @param objsize	size of queuing object
 *
 * @return		maximum number of queuing objects
 *
 * @code
 *     Q_QUEUE *queue = qQueue(100, sizeof(struct myobj));
 *     if(queue == NULL) {
 *       printf("Can't initialize queue.\n");
 *       return -1;
 *     }
 *
 *     // free
 *     queue.free(); // this will de-allocate every thing
 * @endcode
 */
Q_QUEUE *qQueue(int max, size_t objsize) {
	if(max <= 0) return NULL;

	// allocate structure
	Q_QUEUE *queue = (Q_QUEUE*)malloc(sizeof(Q_QUEUE));
	if(queue == NULL) return NULL;
	memset((void*)queue, 0, sizeof(Q_QUEUE));

	// allocate data memory
	size_t memsize = max * objsize;
	void *datamem = (void*)malloc(memsize);
	if(datamem == NULL) {
		DEBUG("qQueue: can't allocate memory %zu bytes.", memsize);
		free(queue);
		return NULL;
	}

	if(qQueueUsrmem(queue, datamem, memsize, objsize) == 0) {
		return NULL;
	}

	// change methods
	queue->free = _free;

	return queue;
}

/**
 * Initialize queue which uses user's memory.
 *
 * @param queue		a pointer of Q_QUEUE
 * @param datamem	a pointer of data memory
 * @param memsize	size of datamem
 * @param objsize	size of queuing object
 *
 * @return		maximum number of queuable objects
 *
 * @code
 *     size_t memsize = sizeof(obj) * 100;
 *     void *datamem = malloc(memsize);
 *     Q_QUEUE queue;
 *     if(qQueueUsrmem(&queue, datamem, memsize, sizeof(obj)) == 0) {
 *       printf("Can't initialize queue.\n");
 *       return -1;
 *     }
 *
 *     // free
 *     queue.free(); // this will not de-allocate structure but de-allocate internal MUTEX if it's compiled with thread-safe feature
 * @endcode
 *
 * @note
 * This implementation is designed to use statically allocated(fixed-size)
 * memory for flexibilities. So you can use this queue with in shared-memory
 * architecture to communicate with other processors.
 * The size of datamem can be calculated with (expected_max_data_in_queue * object_size).
 */
int qQueueUsrmem(Q_QUEUE *queue, void* datamem, size_t memsize, size_t objsize) {
	if(queue == NULL || datamem == NULL || memsize <= 0 || objsize <= 0) return 0;
	memset((void*)queue, 0, sizeof(Q_QUEUE));

	// calculate max
	int max = memsize / objsize;
	if(max < 1) return 0;

	// init structure
	queue->max = max;
	queue->objsize = objsize;
	queue->objarr = datamem;
	_truncate(queue);

	// initialize recrusive mutex
	Q_LOCK_INIT(queue->qlock, true);

	// assign methods
	queue->push	= _push;
	queue->popFirst	= _popFirst;
	queue->popLast	= _popLast;
	queue->getNum	= _getNum;
	queue->getAvail	= _getAvail;
	queue->free	= _freeUsrmem;

	return max;
}

/**
 * Q_QUEUE->push(): Push object into queue
 *
 * @param queue		a pointer of Q_QUEUE
 * @param object	object pointer which points object data to push
 *
 * @return		true if successful, otherwise(queue full or not initialized) returns false
 */
static bool _push(Q_QUEUE *queue, const void *object) {
	if(queue == NULL || object == NULL) return false;

	// check full
	if(queue->used == queue->max) {
		DEBUG("Queue full.");
		return false;
	}

	Q_LOCK_ENTER(queue->qlock);
	// append object
	void *dp = queue->objarr + (queue->objsize * queue->tail);
	memcpy(dp, object, queue->objsize);

	// adjust pointer
	queue->used++;
	queue->tail = (queue->tail + 1) % queue->max;
	Q_LOCK_LEAVE(queue->qlock);

	return true;
}

/**
 * Q_QUEUE->popFirst(): Pop first pushed object from queue.
 *
 * @param queue		a pointer of Q_QUEUE
 * @param remove	set true for pop & remove otherwise data will not be removed.
 *
 * @return		object pointer which malloced if successful, otherwise returns NULL
 *
 * @note	Can be used for FIFO implementation
 */
static void *_popFirst(Q_QUEUE *queue, bool remove) {
	if(queue == NULL) return false;

	// check empty
	if(queue->used == 0) {
		//DEBUG("Queue empty.");
		return NULL;
	}

	// malloc object
	void *object = malloc(queue->objsize);
	if(object == NULL) return NULL;

	Q_LOCK_ENTER(queue->qlock);

	// pop object
	void *dp = queue->objarr + (queue->objsize * queue->head);
	memcpy(object, dp, queue->objsize);

	// adjust pointer
	if(remove == true) {
		queue->used--;
		queue->head = (queue->head + 1) % queue->max;
	}

	Q_LOCK_LEAVE(queue->qlock);

	return object;
}

/**
 * Q_QUEUE->popLast(): Pop last pushed object from queue.
 *
 * @param queue		a pointer of Q_QUEUE
 * @param remove	set true for pop & remove otherwise data will not be removed.
 *
 * @return		object pointer which is malloced if successful, otherwise return NULL
 *
 * @note	Can be used for STACK implementation
 */
static void *_popLast(Q_QUEUE *queue, bool remove) {
	if(queue == NULL) return false;

	// check empty
	if(queue->used == 0) {
		DEBUG("Queue empty.");
		return NULL;
	}

	// malloc object
	void *object = malloc(queue->objsize);
	if(object == NULL) return NULL;

	Q_LOCK_ENTER(queue->qlock);

	// pop object
	int tail = (queue->tail > 0) ? (queue->tail - 1) : (queue->max - 1);
	void *dp = queue->objarr + (queue->objsize * queue->tail);
	memcpy(object, dp, queue->objsize);

	// adjust pointer
	if(remove == true) {
		queue->tail = tail;
		queue->used--;
	}

	Q_LOCK_LEAVE(queue->qlock);

	return object;
}

/**
 * Q_QUEUE->getNum(): Get number of objects queued
 *
 * @param queue		a pointer of Q_QUEUE
 *
 * @return		number of objects queued
 */
static int _getNum(Q_QUEUE *queue) {
	if(queue == NULL) return 0;
	return queue->used;
}

/**
 * Q_QUEUE->getAvail(): Get number of objects can be queued
 *
 * @param queue		a pointer of Q_QUEUE
 *
 * @return		number of objects queued
 */
static int _getAvail(Q_QUEUE *queue) {
	if(queue == NULL) return 0;
	return (queue->max - queue->used);
}

/**
 * Q_QUEUE->truncate(): Truncate queue
 *
 * @param queue		a pointer of Q_QUEUE
 *
 * @return		true if successful, otherwise returns false
 *
 * @note
 * You do not need to call this, after qQueueInit(). This is useful when you
 * reset all of data in the queue.
 */
static void _truncate(Q_QUEUE *queue) {
	Q_LOCK_ENTER(queue->qlock);
	queue->used = 0;
	queue->head = 0;
	queue->tail = 0;
	Q_LOCK_LEAVE(queue->qlock);
}

/**
 * Q_QUEUE->free(): De-allocate queue
 *
 * @param queue		a pointer of Q_QUEUE
 *
 * @return		true if successful, otherwise returns false
 */
static void _free(Q_QUEUE *queue) {
	if(queue == NULL) return;

	Q_LOCK_ENTER(queue->qlock);
	if(queue->objarr != NULL) free(queue->objarr);
	Q_LOCK_LEAVE(queue->qlock);
	Q_LOCK_DESTROY(queue->qlock);

	free(queue);
}

static void _freeUsrmem(Q_QUEUE *queue) {
	Q_LOCK_DESTROY(queue->qlock);
}

#endif /* DISABLE_DATASTRUCTURE */
