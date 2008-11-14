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

#ifndef _QINTERNAL_H
#define _QINTERNAL_H

#define QDECODER_PRIVATEKEY	"qDecoder-by-Seungyoung_Kim"

#ifdef BUILD_DEBUG
#define DEBUG(fmt, args...)	fprintf(stderr, "[DEBUG] " fmt " (%s:%d)\n", ##args, __FILE__, __LINE__);
#else
#define DEBUG(fms, args...)
#endif

/*
 * Internal Macros
 */
#define	MAX_PATHLEN		(1023+1)
#define	MAX_LINEBUF		(1023+1)
#define	CONST_STRLEN(x)		(sizeof(x) - 1)

/*
 * Internal Definitions
 */
#define	DEF_DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define	DEF_FILE_MODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

/*
 * qInternalCommon.c
 */
extern	char	_q_x2c(char hex_up, char hex_low);
extern	char*	_q_makeword(char *str, char stop);
extern	char*	_q_fgets(char *str, int size, FILE *stream);
extern	ssize_t	_q_writef(int fd, char *format, ...);
extern	ssize_t	_q_write(int fd, const void *buf, size_t nbytes);
extern	int	_q_unlink(const char *pathname);

/*
 * To prevent compiler warning
 */
extern	char*	strptime(const char *, const char *, struct tm *);


#endif	/* _QINTERNAL_H */
