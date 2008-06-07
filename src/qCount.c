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
 * @file qCount.c Counter File Handling API
 *
 * Read/Write/Update counter file like below.
 *
 * @code
 *   ---- number.dat ----
 *   74
 *   --------------------
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "qDecoder.h"
#include "qInternal.h"

/**
 * Read counter(integer) from file with advisory file locking.
 *
 * @param filepath file path
 *
 * @return	counter value readed from file. in case of failure, returns 0.
 *
 * @note
 * @code
 *   int count;
 *   count = qCountRead("number.dat");
 * @endcode
 */
int qCountRead(const char *filepath) {
	int fd = open(filepath, O_RDONLY, 0);
	if(fd < 0) return 0;

	char buf[10+1];
	if(read(fd, buf, sizeof(buf)) <= 0) {
		close(fd);
		return 0;
	}

	return atoi(buf);
}

/**
 * Save counter(integer) to file with advisory file locking.
 *
 * @param filepath	file path
 * @param number	counter integer value
 *
 * @return	in case of success, returns true. otherwise false.
 *
 * @note
 * @code
 *   qCountSave("number.dat", 75);
 * @endcode
 */
bool qCountSave(const char *filepath, int number) {
	int fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE);
	if(fd < 0) return false;

	if(_writef(fd, "%d", number) <= 0) {
		close(fd);
		return false;
	}

	close(fd);
	return true;
}

/**
 * Increases(or decrease) the counter value as much as specified number
 * with advisory file locking.
 *
 * @param filepath	file path
 * @param number	how much increase or decrease
 *
 * @return	updated counter value. in case of failure, returns 0.
 *
 * @note
 * @code
 *   int count;
 *   count = qCountUpdate("number.dat", -3);
 * @endcode
 */
int qCountUpdate(const char *filepath, int number) {
	int counter = qCountRead(filepath);
	counter += number;
	return qCountSave(filepath, counter);
}
