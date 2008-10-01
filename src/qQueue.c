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
 * @file qQueue.c Circular-Queue Data Structure API
 *
 * This implementation is designed to use statically allocated(fixed-size)
 * memory for flexibilities. So you can use this queue with in shared-memory
 * architecture to communicate with other processors.
 *
 * @code
 *   ----[Sample codes]----
 *   struct myobj {
 *     int integer;
 *     char string[255+1];
 *   };
 *
 *   int main(void) {
 *     // allocate objects data memory
 *     size_t memsize = qQueueSize(10, sizeof(struct myobj));
 *     void *datamem = malloc(memsize);
 *
 *     // for static data memory use can use this way
 *     // char datamem[NNN];
 *
 *     // initialize queue
 *     Q_QUEUE queue;
 *     if(qQueueInit(&queue, datamem, memsize, sizeof(struct myobj)) == 0) {
 *       printf("Can't initialize queue.\n");
 *       return -1;
 *     }
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
 *       if(qQueuePush(&queue, &obj) == false) break;
 *
 *       // print debug info
 *       printf("Push     : %d, %s\n", obj.integer, obj.string);
 *     }
 *
 *     // pop object from head & tail
 *     while(true) {
 *       struct myobj pop;
 *       if(qQueuePopFirst(&queue, &pop) == false) break;
 *       printf("PopFirst : %d, %s\n", pop.integer, pop.string);
 *
 *       if(qQueuePopLast(&queue, &pop) == false) break;
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
 */

#ifndef DISABLE_DATASTRUCTURE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Get how much memory is needed for N objects
 *
 * @param max		a number of maximum internal slots
 * @param objsize	size of queuing object
 *
 * @return		memory size needed
 *
 * @note
 * This is useful when you decide how much memory(shared-memory) required for N object entries.
 */
size_t qQueueSize(int max, size_t objsize) {
	size_t memsize = objsize * max;
	return memsize;
}

/**
 * Initialize queue
 *
 * @param queue		a pointer of Q_QUEUE
 * @param datamem	a pointer of data memory
 * @param datamemsize	size of datamem
 * @param objsize	size of queuing object
 *
 * @return		maximum number of queuing objects
 *
 * @code
 *     // case of dynamic data memory
 *     size_t memsize = qQueueSize(10, sizeof(struct myobj));
 *     void *datamem = malloc(memsize);
 *     Q_QUEUE queue;
 *     if(qQueueInit(&queue, datamem, memsize, sizeof(int)) == 0) {
 *       printf("Can't initialize queue.\n");
 *       return -1;
 *     }
 *
 *     // case of static data memory
 *     char datamem[1024];
 *     Q_QUEUE queue;
 *     if(qQueueInit(&queue, datamem, sizeof(datamem), sizeof(int)) == 0) {
 *       printf("Can't initialize queue.\n");
 *       return -1;
 *     }
 * @endcode
 */
int qQueueInit(Q_QUEUE *queue, void* datamem, size_t datamemsize, size_t objsize) {
	if(queue == NULL || datamem == NULL || datamemsize <= 0 || objsize <= 0) return 0;

	// calculate max
	int max = datamemsize / objsize;
	if(max < 1) return false;

	// init structure
	queue->max = max;
	queue->objsize = objsize;
	queue->objarr = datamem;
	qQueueClear(queue);

	return max;
}

/**
 * Reset queue
 *
 * @param queue		a pointer of Q_QUEUE
 *
 * @return		true if successful, otherwise returns false
 *
 * @note
 * You do not need to call this, after qQueueInit(). This is useful when you
 * reset all of data in the queue.
 */
bool qQueueClear(Q_QUEUE *queue) {
	queue->used = 0;
	queue->head = 0;
	queue->tail = 0;

	return true;
}

/**
 * Push object into queue
 *
 * @param queue		a pointer of Q_QUEUE
 * @param object	object pointer which points object data to push
 *
 * @return		true if successful, otherwise(queue full or not initialized) returns false
 */
bool qQueuePush(Q_QUEUE *queue, const void *object) {
	if(queue == NULL || object == NULL) return false;

	// check full
	if(queue->used == queue->max) {
		DEBUG("Queue full.");
		return false;
	}

	// append object
	void *dp = queue->objarr + (queue->objsize * queue->tail);
	memcpy(dp, object, queue->objsize);

	// adjust pointer
	queue->used++;
	queue->tail = (queue->tail + 1) % queue->max;

	return true;
}

/**
 * Pop first pushed object from queue.
 *
 * @param queue		a pointer of Q_QUEUE
 * @param object	popped objected will be stored at this object pointer
 *
 * @return		true if successful, otherwise(queue empty or not initialized) returns false
 *
 * @note	Can be used for FIFO implementation
 */
bool qQueuePopFirst(Q_QUEUE *queue, void *object) {
	if(queue == NULL || object == NULL) return false;

	// check empty
	if(queue->used == 0) {
		DEBUG("Queue empty.");
		return false;
	}

	// pop object
	void *dp = queue->objarr + (queue->objsize * queue->head);
	memcpy(object, dp, queue->objsize);

	// adjust pointer
	queue->used--;
	queue->head = (queue->head + 1) % queue->max;

	return true;
}

/**
 * Pop last pushed object from queue.
 *
 * @param queue		a pointer of Q_QUEUE
 * @param object	popped objected will be stored at this object pointer
 *
 * @return		true if successful, otherwise(queue empty or not initialized) returns false
 *
 * @note	Can be used for STACK implementation
 */
bool qQueuePopLast(Q_QUEUE *queue, void *object) {
	if(queue == NULL || object == NULL) return false;

	// check empty
	if(queue->used == 0) {
		DEBUG("Queue empty.");
		return false;
	}

	// adjust pointer
	queue->tail = (queue->tail > 0) ? (queue->tail - 1) : (queue->max - 1);
	queue->used--;

	// pop object
	void *dp = queue->objarr + (queue->objsize * queue->tail);
	memcpy(object, dp, queue->objsize);

	return true;
}

/**
 * Get queue internal status
 *
 * @param queue		a pointer of Q_QUEUE
 * @param used		if not NULL, a number of pushed objects will be stored
 * @param max		if not NULL, the maximum number of pushable objects(queue size) will be stored
 *
 * @return		true if successful, otherwise returns false
 */
bool qQueueStatus(Q_QUEUE *queue, int *used, int *max) {
	if(queue == NULL) return false;

	if(used != NULL) *used = queue->used;
	if(max != NULL) *max = queue->max;

	return true;
}

#endif /* DISABLE_DATASTRUCTURE */
