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

#ifndef _QINTERNAL_H
#define _QINTERNAL_H

#include <fcntl.h>

/*
 * Internal Macros
 */
#define	CONST_STRLEN(x)		(sizeof(x) - 1)

#ifdef BUILD_DEBUG
#define DEBUG(fmt, args...)	fprintf(stderr, "[DEBUG] " fmt " (%s:%d)\n", ##args, __FILE__, __LINE__);
#else
#define DEBUG(fms, args...)
#endif

#ifdef ENABLE_THREADSAFE
#include <pthread.h>
#include <time.h>
#include <errno.h>
#define MUTEX_INIT(x,r)		{							\
	memset((void*)&x, 0, sizeof(Q_LOCK));						\
	pthread_mutexattr_t _mutexattr;							\
	if(r == true) pthread_mutexattr_settype(&_mutexattr, PTHREAD_MUTEX_RECURSIVE);	\
	pthread_mutexattr_setprotocol(&_mutexattr, PTHREAD_PRIO_INHERIT);		\
	int _ret;									\
	if((_ret = pthread_mutex_init(&x.mutex, &_mutexattr)) != 0) {			\
		DEBUG("MUTEX: can't initialize mutex [%d:%s]", _ret, strerror(_ret));	\
	}										\
	DEBUG("MUTEX: initialized");							\
}

#define	MUTEX_UNLOCK(x)		{							\
	if(!pthread_equal(x.owner, pthread_self())) DEBUG("MUTEX: unlock - owner mismatch");	\
	x.count--;									\
	DEBUG("MUTEX: try to unlock, cnt=%d", x.count);					\
	pthread_mutex_unlock(&x.mutex);							\
}

#define MAX_MUTEX_LOCK_WAIT_SEC	(3)
#define	MUTEX_LOCK(x)		{							\
	struct timespec _timeout;							\
	_timeout.tv_sec = MAX_MUTEX_LOCK_WAIT_SEC;					\
	_timeout.tv_nsec = 0;								\
	int _ret;									\
	while((_ret = pthread_mutex_timedlock(&x.mutex, &_timeout)) != 0) {		\
		if(_ret == ETIMEDOUT || _ret == EDEADLK) {				\
			DEBUG("MUTEX: can't get lock force to unlock mutex [%d:%s]", _ret, strerror(_ret));	\
			MUTEX_UNLOCK(x);						\
		} else {								\
			DEBUG("MUTEX: can;t get lock retry [%d:%s]", _ret, strerror(_ret));	\
		}									\
	}										\
	x.count++;									\
	x.owner = pthread_self();							\
	DEBUG("MUTEX: locked, cnt=%d", x.count);					\
}

#define MUTEX_DESTROY(x)	{							\
	if(x.count != 0) DEBUG("MUTEX: ");						\
	int _ret;									\
	while((_ret = pthread_mutex_destroy(&x.mutex)) != 0) {				\
		DEBUG("MUTEX: force to unlock mutex [%d:%s]", _ret, strerror(_ret));	\
		MUTEX_UNLOCK(x);							\
	}										\
	DEBUG("MUTEX: destroyed");							\
}

#else

#define	MUTEX_INIT(x,y)
#define	MUTEX_UNLOCK(x)
#define	MUTEX_LOCK(x)
#define	MUTEX_DESTROY(x)

#endif

/*
 * Internal Definitions
 */
#define QDECODER_PRIVATEKEY	"qDecoder-by-Seungyoung_Kim"
#define	MAX_LINEBUF		(1023+1)
#define	DEF_DIR_MODE		(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define	DEF_FILE_MODE		(S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

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
