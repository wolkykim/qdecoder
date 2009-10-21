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

#include "qDecoder.h"

#define INIT_MAX_TABLE_SIZE	(10)
#define NUM_OF_TEST_DATA	(80)

int main(void) {
	// create hash table with initially 10 data slots
	Q_HASHTBL *tbl = qHashtbl(INIT_MAX_TABLE_SIZE, true, 80);

	// insert some data into table
	int i;
	for(i = 1; i <= NUM_OF_TEST_DATA; i++) {
		char key[64], value[64];
		snprintf(key, sizeof(key), "key %d", i);
		snprintf(value, sizeof(value), "value %d", i);

		printf("[%d] INSERT DATA : KEY = '%s' (%d/%d/%d,%d%%) - ", i, key, tbl->num, tbl->resizeat, tbl->max, tbl->threshold);
		if(tbl->putStr(tbl, key, value) == true) {
			printf ("OK\n");
		} else {
			printf ("FAILED\n");
			break;
		}
	}

	// print out everything in the hash table
	tbl->print(tbl, stdout, true);

	// de-allocate hash table
	tbl->free(tbl);

	return 0;
}
