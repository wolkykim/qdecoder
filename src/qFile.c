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
 * @file qFile.c File Handling API
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "qDecoder.h"
#include "qInternal.h"

#define MAX_FILESEND_CHUNK_SIZE		(64 * 1024)

/**
 * Lock opened file.
 *
 * @param fd		file descriptor
 *
 * @return		true if successful, otherwise returns false.
 *
 * @code
 *   // for file descriptor
 *   int fd = open(...);
 *   if(qFileLock(fd) == true) {
 *     (...atomic file access...)
 *     qFileUnlock(fd);
 *   }
 *
 *   // for FILE stream object
 *   FILE *fp = fopen(...);
 *   int fd = fileno(fp);
 *   if(qFileLock(fd) == true) {
 *     (...atomic file access...)
 *     qFileUnlock(fd);
 *   }
 * @endcode
 */
bool qFileLock(int fd) {
#ifdef _WIN32
	return false;
#else
	int ret = flock(fd, LOCK_EX);
	if(ret == 0) return true;
	return false;
#endif
}

/**
 * Unlock opened file.
 *
 * @param fd		file descriptor
 *
 * @return		true if successful, otherwise returns false.
 */
bool qFileUnlock(int fd) {
#ifdef _WIN32
	return false;
#else
	int ret = flock(fd, LOCK_EX);
	if(ret == 0) return true;
	return false;
#endif
}

/**
 * Check file existence.
 *
 * @param filepath	file or directory path
 *
 * @return		true if exists, otherwise returns false;
 */
bool qFileExist(const char *filepath) {
	struct stat finfo;
	if (stat(filepath, &finfo) < 0) return false;
	return true;
}

/**
 * Get filename from filepath
 *
 * @param filepath	file or directory path
 *
 * @return		malloced filename string
 */
char *qFileGetName(const char *filepath) {
	char *path = strdup(filepath);
	char *bname = basename(path);
	char *filename = strdup(bname);
	free(path);
	return filename;
}

/**
 * Get directory suffix from filepath
 *
 * @param filepath	file or directory path
 *
 * @return		malloced filepath string
 */
char *qFileGetDir(const char *filepath) {
	char *path = strdup(filepath);
	char *dname = dirname(path);
	char *dir = strdup(dname);
	free(path);
	return dir;
}

/**
 * Get extension from filepath.
 *
 * @param filepath	file or directory path
 *
 * @return		malloced extension string which is converted to lower case.
 */
char *qFileGetExt(const char *filepath) {
#define MAX_EXTENSION_LENGTH		(5)
	char *filename = qFileGetName(filepath);
	char *p = strrchr(filename, '.');
	char *ext = NULL;
	if(p != NULL && strlen(p+1) <= MAX_EXTENSION_LENGTH && qStrIsAlnum(p+1) == true) {
		ext = strdup(p+1);
		qStrLower(ext);
	} else {
		ext = strdup("");
	}

	free(filename);
	return ext;
}

/**
 * Get file size.
 *
 * @param filepath	file or directory path
 *
 * @return		the file size if exists, otherwise returns -1.
 */
off_t qFileGetSize(const char *filepath) {
	struct stat finfo;
	if (stat(filepath, &finfo) < 0) return -1;
	return finfo.st_size;
}

/**
 * Transfer data between file descriptors
 *
 * @param outfd		output file descriptor
 * @param infd		input file descriptor
 * @param size		the number of bytes to copy between file descriptors. 0 means transfer until end of infd.
 *
 * @return		the number of bytes written to outfd.
 */
ssize_t qFileSend(int outfd, int infd, size_t nbytes) {
	if(nbytes == 0) return 0;

	char buf[MAX_FILESEND_CHUNK_SIZE];

	ssize_t sent = 0; // total size sent
	while(sent < nbytes) {
		size_t sendsize;	// this time sending size
		if(nbytes - sent <= sizeof(buf)) sendsize = nbytes - sent;
		else sendsize = sizeof(buf);

		// read
		ssize_t retr = read(infd, buf, sendsize);
		DEBUG("read %zd", retr);
		if (retr <= 0) {
			if(sent == 0) return -1;
			break;
		}

		// write
		ssize_t retw = _write(outfd, buf, retr);
		DEBUG("write %zd", retw);
		if(retw <= 0) {
			if(sent == 0) return -1;
			break;
		}

		sent += retw;
		if(retr != retw) {
			DEBUG("size mismatch %zd, %zd", retr, retw);
			break;
		}
	}

	return sent;
}

/**
 * Load file into memory.
 *
 * @param filepath	file path
 * @param nbytes	has two purpost, one is to set how many bytes are readed. the other is actual the number loaded bytes will be stored. nbytes must be point 0 or NULL to read entire file.
 *
 *
 * @return		allocated memory pointer if successful, otherwise returns NULL.
 *
 * @code
 *   // loading text file
 *   char *text = (char *)qFileLoad("/tmp/text.txt", NULL);
 *
 *   // loading binary file
 *   int binlen = 0;
 *   char *bin = (char *)qFileLoad("/tmp/binary.bin", &binlen);
 *
 *   // loading partial
 *   int binlen = 10;
 *   char *bin = (char *)qFileLoad("/tmp/binary.bin", &binlen);
 * @endcode
 *
 * @note
 * This method actually allocates memory more than 1 bytes than filesize then append
 * '\0' character at the end. For example, when the file size is 10 bytes long, 10+1
 * bytes will allocated and the last byte is always '\0' character. So you can load
 * text file and use without appending '\0' character. By the way, *size still will
 * be returned the actual file size of 10.
 */
void *qFileLoad(const char *filepath, size_t *nbytes) {
	int fd;
	if((fd = open(filepath, O_RDONLY, 0)) < 0) return NULL;

	struct stat fs;
	if (fstat(fd, &fs) < 0) {
		close(fd);
		return NULL;
	}

	size_t size = fs.st_size;
	if(nbytes != NULL && *nbytes > 0 && *nbytes < fs.st_size) size = *nbytes;

	void *buf = malloc(size + 1);
	if(buf == NULL) {
		close(fd);
		return NULL;
	}

	ssize_t count = read(fd, buf, size);
	close(fd);

	if (count != size) {
		free(buf);
		return NULL;
	}

	((char*)buf)[count] = '\0';

	if(nbytes != NULL) *nbytes = count;
	return buf;
}

/**
 * Load file stream into memory.
 *
 * @param fp		FILE pointer
 * @param nbytes	has two purpost, one is to set how many bytes are readed. the other is actual the number loaded bytes will be stored. nbytes must be point 0 or NULL to read end of stream.
 *
 * @return		allocated memory pointer if successful, otherwise returns NULL.
 *
 * @note
 * This method append '\0' character at the end of stream. but nbytes only counts
 * actual readed bytes.
 */
void *qFileRead(FILE *fp, size_t *nbytes) {
	size_t memsize;
	size_t c_count;
	size_t size = 0;
	char *data = NULL;

	if(nbytes != NULL && *nbytes > 0) size = *nbytes;

	int c;
	for (memsize = 1024, c_count = 0; (c = fgetc(fp)) != EOF;) {
		if(size > 0 && c_count == size) break;

		if (c_count == 0) {
			data = (char*)malloc(sizeof(char) * memsize);
			if (data == NULL) {
				DEBUG("Memory allocation failed.");
				return NULL;
			}
		} else if (c_count == memsize - 1) {
			memsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			char *datatmp = (char*)malloc(sizeof(char) * (memsize + 1));
			if (datatmp == NULL) {
				DEBUG("Memory allocation failed.");
				free(data);
				return NULL;
			}
			memcpy(datatmp, data, c_count);
			free(data);
			data = datatmp;
		}
		data[c_count++] = (char)c;
	}

	if (c_count == 0 && c == EOF) return NULL;
	data[c_count] = '\0';

	if(nbytes != NULL) *nbytes = c_count;

	return (void*)data;
}

/**
 * Read string. Same as fgets but can be used for unlimited string line.
 *
 * @param fp		FILE pointer
 *
 * @return		allocated memory pointer if successful, otherwise returns NULL.
 */
char *qFileReadLine(FILE *fp) {
	int memsize;
	int c, c_count;
	char *string = NULL;

	for (memsize = 1024, c_count = 0; (c = fgetc(fp)) != EOF;) {
		if (c_count == 0) {
			string = (char *)malloc(sizeof(char) * memsize);
			if (string == NULL) {
				DEBUG("Memory allocation failed.");
				return NULL;
			}
		} else if (c_count == memsize - 1) {
			char *stringtmp;

			memsize *= 2;

			/* Here, we do not use realloc(). Because sometimes it is unstable. */
			stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
			if (stringtmp == NULL) {
				DEBUG("Memory allocation failed.");
				free(string);
				return NULL;
			}
			memcpy(stringtmp, string, c_count);
			free(string);
			string = stringtmp;
		}
		string[c_count++] = (char)c;
		if ((char)c == '\n') break;
	}

	if (c_count == 0 && c == EOF) return NULL;
	string[c_count] = '\0';

	return string;
}

/**
 * Save to file.
 *
 * @param filepath	file path
 * @param buf		data
 * @param size		the number of bytes to save
 * @param append	false for new(if exists truncate), true for appending
 *
 * @return		the number of bytes written if successful, otherwise returns -1.
 *
 * @code
 *   // save text
 *   char *text = "hello";
 *   qFileSave("/tmp/text.txt", (void*)text, strlen(text), false);
 *
 *   // save binary
 *   int integer1 = 75;
 *   qFileSave("/tmp/integer.bin, (void*)&integer, sizeof(int));
 * @endcode
 */
ssize_t qFileSave(const char *filepath, const void *buf, size_t size, bool append) {
	int fd;

	if(append == false) fd = open(filepath, O_CREAT|O_WRONLY|O_TRUNC, DEF_FILE_MODE);
	else fd = open(filepath, O_CREAT|O_WRONLY|O_APPEND, DEF_FILE_MODE);
	if(fd < 0) return -1;

	ssize_t count = write(fd, buf, size);
	close(fd);

	return count;
}

/**********************************************
** Usage : qCmd(external command);
** Return: Execution output, File not found NULL.
**********************************************/
char *qCmd(char *cmd) {
	FILE *fp;
	char *str;

	fp = popen(cmd, "r");
	if (fp == NULL) return NULL;
	str = qFileRead(fp, NULL);
	pclose(fp);

	if(str == NULL) str = strdup("");
	return str;
}
