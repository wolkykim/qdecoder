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
#ifdef BUILD_DEBUG
#define DEBUG(fmt, args...)	fprintf(stderr, "[DEBUG] " fmt " (%s:%d)\n", ##args, __FILE__, __LINE__);
#else
#define DEBUG(fms, args...)
#endif	/* BUILD_DEBUG */

/*
 * Macro Functions
 */
#define	CONST_STRLEN(x)		(sizeof(x) - 1)

#define	DYNAMIC_VSPRINTF(s, f)							\
do {										\
	size_t _strsize;							\
	for(_strsize = 1024; ; _strsize *= 2) {					\
		s = (char*)malloc(_strsize);					\
		if(s == NULL) {							\
			DEBUG("DYNAMIC_VSPRINTF(): can't allocate memory.");	\
			break;							\
		}								\
		va_list _arglist;						\
		va_start(_arglist, f);						\
		int _n = vsnprintf(s, _strsize, f, _arglist);			\
		va_end(_arglist);						\
		if(_n >= 0 && _n < _strsize) break;				\
		free(s);							\
	}									\
} while(0)

/*
 * Q_LOCK Macros
 */
#ifdef ENABLE_THREADSAFE

#ifndef _MULTI_THREADED
#define	_MULTI_THREADED
#endif

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define Q_LOCK_INIT(x,r)								\
do {											\
	memset((void*)&x, 0, sizeof(Q_LOCK_T));						\
	pthread_mutexattr_t _mutexattr;							\
	pthread_mutexattr_init(&_mutexattr);						\
	if(r == true) {									\
		pthread_mutexattr_settype(&_mutexattr, PTHREAD_MUTEX_RECURSIVE);	\
	}										\
	int _ret = pthread_mutex_init(&x.mutex, &_mutexattr);				\
	pthread_mutexattr_destroy(&_mutexattr);						\
	if(_ret != 0) {									\
		DEBUG("Q_LOCK: can't initialize mutex. [%d:%s]", _ret, strerror(_ret));	\
		exit(EXIT_FAILURE);							\
	}										\
	DEBUG("Q_LOCK: initialized.");							\
} while(0)
//pthread_mutexattr_setprotocol(&_mutexattr, PTHREAD_PRIO_INHERIT);

#define	Q_LOCK_LEAVE(x)									\
do {											\
	if(!pthread_equal(x.owner, pthread_self())) DEBUG("Q_LOCK: unlock - owner mismatch.");	\
	x.count--;									\
	pthread_mutex_unlock(&x.mutex);							\
} while(0)
//	DEBUG("Q_LOCK: unlock, cnt=%d", x.count);

#define MAX_MUTEX_LOCK_WAIT	(5000)
#define	Q_LOCK_ENTER(x)									\
do {											\
	while(true) {									\
		int _ret, i;								\
 		for(i = 0; (_ret = pthread_mutex_trylock(&x.mutex)) != 0 && i < MAX_MUTEX_LOCK_WAIT; i++) {	\
			if(i == 0) DEBUG("Q_LOCK: mutex is already locked - try again");	\
			usleep(1);							\
		}									\
		if(_ret == 0) break;							\
		DEBUG("Q_LOCK: can't get lock - force to unlock. [%d:%s]", _ret, strerror(_ret));	\
		Q_LOCK_LEAVE(x);							\
	}										\
	x.count++;									\
	x.owner = pthread_self();							\
} while(0)
//	DEBUG("Q_LOCK: locked, cnt=%d", x.count);

#define Q_LOCK_DESTROY(x)								\
do {											\
	if(x.count != 0) DEBUG("Q_LOCK: mutex counter is not 0.");			\
	int _ret;									\
	while((_ret = pthread_mutex_destroy(&x.mutex)) != 0) {				\
		DEBUG("Q_LOCK: force to unlock mutex. [%d:%s]", _ret, strerror(_ret));	\
		Q_LOCK_LEAVE(x);							\
	}										\
	DEBUG("Q_LOCK: destroyed.");							\
} while(0)

#else

#define	Q_LOCK_INIT(x,y)
#define	Q_LOCK_LEAVE(x)
#define	Q_LOCK_ENTER(x)
#define	Q_LOCK_DESTROY(x)

#endif	/* ENABLE_THREADSAFE */

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
extern	void	_q_die(const char *errstr);
extern	void*	_q_malloc(size_t size);
extern	char	_q_x2c(char hex_up, char hex_low);
extern	char*	_q_makeword(char *str, char stop);
extern	char*	_q_fgets(char *str, int size, FILE *stream);
extern	int	_q_unlink(const char *pathname);

/*
 * To prevent compiler warning
 */
extern	char*	strptime(const char *, const char *, struct tm *);

#endif	/* _QINTERNAL_H */
