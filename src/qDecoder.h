/*
 * Copyright 2000-2010 The qDecoder Project. All rights reserved.
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
 *
 * $Id$
 */

/**
 * qDecoder Header file
 *
 * @file	qDecoder.h
 */

#ifndef _QDECODER_H
#define _QDECODER_H

#define _Q_PRGNAME			"qDecoder"
#define _Q_VERSION			"10.1.5"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

typedef struct _Q_NOBJ_T	Q_NOBJ_T;
typedef struct _Q_NLOBJ_T	Q_NLOBJ_T;
typedef struct _Q_MUTEX_T	Q_MUTEX_T;

typedef struct _Q_ENTRY		Q_ENTRY;
typedef struct _Q_OBSTACK	Q_OBSTACK;
typedef struct _Q_HASHTBL	Q_HASHTBL;
typedef struct _Q_HASHARR	Q_HASHARR;
typedef struct _Q_QUEUE		Q_QUEUE;
typedef struct _Q_LOG		Q_LOG;
typedef struct _Q_DB		Q_DB;
typedef struct _Q_DBRESULT	Q_DBRESULT;

typedef struct _Q_HTTPCLIENT	Q_HTTPCLIENT;

/**
 * Variable type for named object
 */
struct _Q_NOBJ_T {
	char*		name;		/*!< object name */
	void*		data;		/*!< data object */
	size_t		size;		/*!< object size */
};

/**
 * Variable type for named object with link
 */
struct _Q_NLOBJ_T {
	char*		name;		/*!< object name */
	void*		data;		/*!< data object */
	size_t		size;		/*!< object size */
	Q_NLOBJ_T*	next;		/*!< link pointer */
};

/**
 * Variable type for lock management
 */
struct _Q_MUTEX_T {
	pthread_mutex_t	mutex;
	pthread_t	owner;
	int		count;
};

/**
 * Structure for linked-list data structure.
 */
struct _Q_ENTRY {
	Q_MUTEX_T	qmutex;		/*!< only used if compiled with --enable-threadsafe option */

	int		num;		/*!< number of objects */
	size_t		size;		/*!< total size of data objects, does not include name size */
	Q_NLOBJ_T*	first;		/*!< first object pointer */
	Q_NLOBJ_T*	last;		/*!< last object pointer */

	/* public member methods */
	void		(*lock)		(Q_ENTRY *entry);
	void		(*unlock)	(Q_ENTRY *entry);

	bool		(*put)		(Q_ENTRY *entry, const char *name, const void *data, size_t size, bool replace);
	bool		(*putStr)	(Q_ENTRY *entry, const char *name, const char *str, bool replace);
	bool		(*putStrf)	(Q_ENTRY *entry, bool replace, const char *name, const char *format, ...);
	bool		(*putStrParsed)	(Q_ENTRY *entry, const char *name, const char *str, bool replace);
	bool		(*putInt)	(Q_ENTRY *entry, const char *name, int num, bool replace);

	void*		(*get)		(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	void*		(*getCase)	(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	void*		(*getLast)	(Q_ENTRY *entry, const char *name, size_t *size, bool newmem);
	char*		(*getStr)	(Q_ENTRY *entry, const char *name, bool newmem);
	char*		(*getStrf)	(Q_ENTRY *entry, bool newmem, const char *namefmt, ...);
	char*		(*getStrCase)	(Q_ENTRY *entry, const char *name, bool newmem);
	char*		(*getStrLast)	(Q_ENTRY *entry, const char *name, bool newmem);
	int		(*getInt)	(Q_ENTRY *entry, const char *name);
	int		(*getIntCase)	(Q_ENTRY *entry, const char *name);
	int 		(*getIntLast)	(Q_ENTRY *entry, const char *name);
	bool		(*getNext)	(Q_ENTRY *entry, Q_NLOBJ_T *obj, const char *name, bool newmem);

	int		(*remove)	(Q_ENTRY *entry, const char *name);

	int 		(*getNum)	(Q_ENTRY *entry);
	int		(*getNo)	(Q_ENTRY *entry, const char *name);
	char*		(*parseStr)	(Q_ENTRY *entry, const char *str);
	void*		(*merge)	(Q_ENTRY *entry, size_t *size);

	bool		(*truncate)	(Q_ENTRY *entry);
	bool		(*save)		(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode);
	int		(*load)		(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode);
	bool		(*reverse)	(Q_ENTRY *entry);
	bool		(*print)	(Q_ENTRY *entry, FILE *out, bool print_data);
	bool		(*free)		(Q_ENTRY *entry);
};

/**
 * Structure for obstack data structure.
 */
struct _Q_OBSTACK {
	Q_ENTRY *stack;			/*!< first object pointer */

	bool		(*grow)		(Q_OBSTACK *obstack, const void *object, size_t size);
	bool		(*growStr)	(Q_OBSTACK *obstack, const char *str);
	bool		(*growStrf)	(Q_OBSTACK *obstack, const char *format, ...);

	void*		(*getFinal)	(Q_OBSTACK *obstack, size_t *size);
	ssize_t		(*writeFinal)	(Q_OBSTACK *obstack, int nFd);

	size_t		(*getSize)	(Q_OBSTACK *obstack);
	int		(*getNum)	(Q_OBSTACK *obstack);
	bool		(*free)		(Q_OBSTACK *obstack);
};

#define _Q_HASHTBL_RESIZE_MAG		(2)
#define _Q_HASHTBL_DEF_THRESHOLD	(80)
/**
 * Structure for hash-table data structure.
 */
struct _Q_HASHTBL {
	Q_MUTEX_T	qmutex;		/*!< only used if compiled with --enable-threadsafe option */

	int		max;		/*!< maximum hashtable size */
	int		num;		/*!< used slot counter */
	int		threshold;	/*!< if the percent of used slot counter exceeds this threshold percent, new larger table(max * _Q_HASHTBL_RESIZE_MAG) is allocated */
	int		resizeat;	/*!< calculated used amount = (max * threshold) / 100 */

	int*		count;		/*!< hash collision counter. 0 indicate empty slot, -1 is used for moved slot due to hash collision */
	int*		hash;		/*!< key hash. we use qHashFnv32() to generate hash integer */
	Q_NOBJ_T*	obj;

	/* public member methods */
	void		(*lock)		(Q_HASHTBL *tbl);
	void		(*unlock)	(Q_HASHTBL *tbl);

	bool		(*put)		(Q_HASHTBL *tbl, const char *name, const void *data, size_t size);
	bool		(*putStr)	(Q_HASHTBL *tbl, const char *name, const char *str);
	bool		(*putInt)	(Q_HASHTBL *tbl, const char *name, int num);

	void*		(*get)		(Q_HASHTBL *tbl, const char *name, size_t *size, bool newmem);
	char*		(*getStr)	(Q_HASHTBL *tbl, const char *name, bool newmem);
	int		(*getInt)	(Q_HASHTBL *tbl, const char *name);
	bool		(*getNext)	(Q_HASHTBL *tbl, Q_NOBJ_T *obj, int *idx, bool newmem);

	bool		(*remove)	(Q_HASHTBL *tbl, const char *name);

	int 		(*getNum)	(Q_HASHTBL *tbl);
	int		(*getMax)	(Q_HASHTBL *tbl);
	bool		(*resize)	(Q_HASHTBL *tbl, int max);
	bool		(*truncate)	(Q_HASHTBL *tbl);

	bool		(*print)	(Q_HASHTBL *tbl, FILE *out, bool print_data);
	bool		(*free)		(Q_HASHTBL *tbl);
};

#define _Q_HASHARR_MAX_KEYSIZE		(31+1)
#define _Q_HASHARR_DEF_VALUESIZE	(32)
/**
 * Structure for hash-table data structure based on array.
 */
struct _Q_HASHARR {
	int		count;				/*!< hash collision counter. 0 indicates empty slot, -1 is used for collision resolution, -2 is used for indicating linked block */
	int		hash;				/*!< key hash. we use qFnv32Hash() to generate hash integer */

	char		key[_Q_HASHARR_MAX_KEYSIZE];	/*!< key string, it can be truncated */
	int		keylen;				/*!< original key length */
	unsigned char keymd5[16];			/*!< md5 hash of the key */

	unsigned char value[_Q_HASHARR_DEF_VALUESIZE];	/*!< value */
	int		size;				/*!< value size */
	int		link;				/*!< next index of the value */

	/* public member methods */
	bool		(*put)		(Q_HASHARR *tbl, const char *key, const void *value, int size);
	bool		(*putStr)	(Q_HASHARR *tbl, const char *key, const char *str);
	bool		(*putInt)	(Q_HASHARR *tbl, const char *key, int num);

	void*		(*get)		(Q_HASHARR *tbl, const char *key, int *size);
	char*		(*getStr)	(Q_HASHARR *tbl, const char *key);
	int		(*getInt)	(Q_HASHARR *tbl, const char *key);
	const char*	(*getNext)	(Q_HASHARR *tbl, int *idx);

	bool		(*remove)	(Q_HASHARR *tbl, const char *key);
	bool		(*truncate)	(Q_HASHARR *tbl);

	bool		(*print)	(Q_HASHARR *tbl, FILE *out);
	int		(*getNum)	(Q_HASHARR *tbl);
	int		(*getMax)	(Q_HASHARR *tbl);
};

/**
 * Structure for array-based circular-queue data structure.
 */
struct _Q_QUEUE {
	Q_MUTEX_T	qmutex;		/*!< only used if compiled with --enable-threadsafe option */

	int		max;		/*!< maximum queue slots */
	int		used;		/*!< used queue slots */

	int		head;		/*!< head pointer */
	int		tail;		/*!< tail pointer */

	size_t		objsize;	/*!< object size */
	void*		objarr;		/*!< queue data memory pointer */

	/* public member methods */
	bool		(*push)		(Q_QUEUE *queue, const void *object);
	void*		(*popFirst)	(Q_QUEUE *queue, bool remove);
	void*		(*popLast)	(Q_QUEUE *queue, bool remove);
	int		(*getNum)	(Q_QUEUE *queue);
	int		(*getAvail)	(Q_QUEUE *queue);
	void		(*truncate)	(Q_QUEUE *queue);
	void		(*free)		(Q_QUEUE *queue);
	void		(*donothing)	(Q_QUEUE *queue);
};

/**
 * Structure for file log.
 */
struct _Q_LOG {
	Q_MUTEX_T	qmutex;		/*!< only used if compiled with --enable-threadsafe option */

	char	filepathfmt[PATH_MAX];	/*!< file file naming format like /somepath/qdecoder-%Y%m%d.log */
	char	filepath[PATH_MAX];	/*!< generated system path of log file */
	FILE	*fp;			/*!< file pointer of logpath */
	int	rotateinterval;		/*!< log file will be rotate in this interval seconds */
	int	nextrotate;		/*!< next rotate universal time, seconds */
	bool	logflush;		/*!< flag for immediate flushing */

	FILE	*outfp;			/*!< stream pointer for duplication */
	bool	outflush;		/*!< flag for immediate flushing for duplicated stream */

	/* public member methods */
	bool	(*write)		(Q_LOG *log, const char *str);
	bool	(*writef)		(Q_LOG *log, const char *format, ...);
	bool	(*duplicate)		(Q_LOG *log, FILE *outfp, bool flush);
	bool	(*flush)		(Q_LOG *log);
	bool	(*free)			(Q_LOG *log);
};

/* Database Support*/
#ifdef _mysql_h
#define _Q_ENABLE_MYSQL				(1)
/* mysql specific connector option */
#define _Q_MYSQL_OPT_RECONNECT			(1)
#define _Q_MYSQL_OPT_CONNECT_TIMEOUT		(10)
#define _Q_MYSQL_OPT_READ_TIMEOUT		(30)
#define _Q_MYSQL_OPT_WRITE_TIMEOUT		(30)
#endif	/* _mysql_h */

/**
 * Structure for independent database interface.
 */
struct _Q_DB {
	Q_MUTEX_T	qmutex;		/*!< only used if compiled with --enable-threadsafe option */

	bool connected;			/*!< if opened true, if closed false */

	struct {
		char*	dbtype;
		char*	addr;
		int	port;
		char*	username;
		char*	password;
		char*	database;
		bool	autocommit;
		bool	fetchtype;
	} info;				/*!< database connection infomation */

	/* for mysql database */
#ifdef _Q_ENABLE_MYSQL
	MYSQL		*mysql;
#endif

	/* public member methods */
	bool		(*open)			(Q_DB *db);
	bool		(*close)		(Q_DB *db);

	int		(*executeUpdate)	(Q_DB *db, const char *query);
	int		(*executeUpdatef)	(Q_DB *db, const char *format, ...);

	Q_DBRESULT*	(*executeQuery)		(Q_DB *db, const char *query);
	Q_DBRESULT*	(*executeQueryf)	(Q_DB *db, const char *format, ...);

	bool		(*beginTran)		(Q_DB *db);
	bool		(*endTran)		(Q_DB *db, bool commit);
	bool		(*commit)		(Q_DB *db);
	bool		(*rollback)		(Q_DB *db);

	bool		(*setFetchType)		(Q_DB *db, bool use);
	bool		(*getConnStatus)	(Q_DB *db);
	bool		(*ping)			(Q_DB *db);
	const char*	(*getError)		(Q_DB *db, unsigned int *errorno);
	bool		(*free)			(Q_DB *db);
};

/**
 * Structure for database result set.
 */
struct _Q_DBRESULT {
#ifdef _Q_ENABLE_MYSQL
	bool		fetchtype;
	MYSQL_RES	*rs;
	MYSQL_FIELD	*fields;
	MYSQL_ROW	row;
	int		cols;
	int		cursor;

	/* public member methods */
	const char*	(*getStr)		(Q_DBRESULT *result, const char *field);
	const char*	(*getStrAt)		(Q_DBRESULT *result, int idx);
	int		(*getInt)		(Q_DBRESULT *result, const char *field);
	int		(*getIntAt)		(Q_DBRESULT *result, int idx);
	bool		(*getNext)		(Q_DBRESULT *result);

	int     	(*getCols)		(Q_DBRESULT *result);
	int     	(*getRows)		(Q_DBRESULT *result);
	int     	(*getRow)		(Q_DBRESULT *result);

	bool		(*free)			(Q_DBRESULT *result);
#endif
};

/**
 * Structure for http client.
 */
struct _Q_HTTPCLIENT {
	int		socket;			/*!< socket descriptor */

	struct sockaddr_in addr;
	char		*hostname;
	int		port;

	int		timeoutms;		/*< wait timeout miliseconds*/
	bool		keepalive;		/*< keep-alive flag */
	char*		useragent;		/*< user-agent name */

	/* public member methods */
	void		(*setTimeout)		(Q_HTTPCLIENT *client, int timeoutms);
	void		(*setKeepalive)		(Q_HTTPCLIENT *client, bool keepalive);
	void		(*setUseragent)		(Q_HTTPCLIENT *client, const char *useragent);

	bool		(*open)			(Q_HTTPCLIENT *client);
	bool		(*sendRequest)		(Q_HTTPCLIENT *client, const char *method, const char *uri, Q_ENTRY *reqheaders);
	int		(*readResponse)		(Q_HTTPCLIENT *client, Q_ENTRY *resheaders);
	bool		(*get)			(Q_HTTPCLIENT *client, const char *uri, int fd, off_t *savesize, int *rescode, Q_ENTRY *reqheaders, Q_ENTRY *resheaders, bool (*callback)(void *userdata, off_t recvbytes), void *userdata);
	bool		(*put)			(Q_HTTPCLIENT *client, const char *uri, int fd, off_t length, int *retcode, Q_ENTRY *userheaders, Q_ENTRY *resheaders, bool (*callback)(void *userdata, off_t sentbytes), void *userdata);
	void*		(*cmd)			(Q_HTTPCLIENT *client, const char *method, const char *uri, int *rescode, size_t *contentslength, Q_ENTRY *reqheaders, Q_ENTRY *resheaders);
	bool		(*close)		(Q_HTTPCLIENT *client);
	void		(*free)			(Q_HTTPCLIENT *client);
};

#ifndef _DOXYGEN_SKIP

/* qDecoder C++ support */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * qCgiRequest.c
 */
extern	Q_ENTRY*	qCgiRequestParseOption(bool filemode, const char *basepath, int clearold);
extern	Q_ENTRY*	qCgiRequestParse(Q_ENTRY *request);
extern	Q_ENTRY*	qCgiRequestParseQueries(Q_ENTRY *request, const char *method);
extern	Q_ENTRY*	qCgiRequestParseCookies(Q_ENTRY *request);
extern	char*		qCgiRequestGetQueryString(const char *query_type);

/*
 * qCgiResponse.c
 */
extern	bool		qCgiResponseSetCookie(Q_ENTRY *request, const char *name, const char *value, int expire, const char *path, const char *domain, bool secure);
extern	bool		qCgiResponseRemoveCookie(Q_ENTRY *request, const char *name, const char *path, const char *domain, bool secure);

extern	bool		qCgiResponseSetContentType(Q_ENTRY *request, const char *mimetype);
extern	const char*	qCgiResponseGetContentType(Q_ENTRY *request);
extern	bool		qCgiResponseRedirect(Q_ENTRY *request, const char *uri);
extern	int		qCgiResponseDownload(Q_ENTRY *request, const char *filepath, const char *mimetype);
extern	void		qCgiResponseError(Q_ENTRY *request, char *format, ...);

/*
 * qSession.c
 */
extern	Q_ENTRY*	qSessionInit(Q_ENTRY *request, const char *dirpath);
extern	bool		qSessionSetTimeout(Q_ENTRY *session, time_t seconds);
extern	const char*	qSessionGetId(Q_ENTRY *session);
extern	time_t		qSessionGetCreated(Q_ENTRY *session);
extern	bool		qSessionSave(Q_ENTRY *session);
extern	bool		qSessionDestroy(Q_ENTRY *session);

/*
 * qHtml.c
 */
extern	bool		qHtmlPrintf(FILE *stream, int mode, const char *format, ...);
extern	bool		qHtmlPuts(FILE *stream, int mode, char *buf);

/*
 * qSem.c
 */
extern	int		qSemInit(const char *keyfile, int keyid, int nsems, bool ifexistdestroy);
extern	int		qSemGetId(const char *keyfile, int keyid);
extern	bool		qSemEnter(int semid, int semno);
extern	bool		qSemEnterNowait(int semid, int semno);
extern	bool		qSemEnterForce(int semid, int semno, int maxwaitms, bool *forceflag);
extern	bool		qSemLeave(int semid, int semno);
extern	bool		qSemCheck(int semid, int semno);
extern	bool		qSemFree(int semid);

/*
 * qShm.c
 */
extern	int		qShmInit(const char *keyfile, int keyid, size_t size, bool ifexistdestroy);
extern	int		qShmGetId(const char *keyfile, int keyid);
extern	void*		qShmGet(int shmid);
extern	bool		qShmFree(int shmid);

/*
 * qDatabase.c
 */
extern	Q_DB*		qDb(const char *dbtype, const char *addr, int port, const char *database, const char *username, const char *password, bool autocommit);

/*
 * qEntry.c
 */
extern	Q_ENTRY*	qEntry(void);

/*
 * qHashtbl.c
 */
extern	Q_HASHTBL*	qHashtbl(int max, bool resize, int threshold);

/*
 * qHasharr.c
 */
extern	size_t		qHasharrSize(int max);
extern	int		qHasharr(Q_HASHARR *tbl, size_t memsize);

/*
 * qQueue.c
 */
extern	Q_QUEUE*	qQueue(int max, size_t objsize);
extern	int		qQueueUsrmem(Q_QUEUE *queue, void* datamem, size_t memsize, size_t objsize);

/*
 * qObstack.c
 */
extern	Q_OBSTACK*	qObstack(void);

/*
 * qConfig.c
 */
extern	Q_ENTRY*	qConfigParseFile(Q_ENTRY *config, const char *filepath, char sepchar);
extern	Q_ENTRY*	qConfigParseStr(Q_ENTRY *config, const char *str, char sepchar);

/*
 * qLog.c
 */
extern	Q_LOG*		qLog(const char *filepathfmt, int rotateinterval, bool flush);

/*
 * qHttpClient.c
 */
extern	Q_HTTPCLIENT*	qHttpClient(const char *hostname, int port);

/*
 * qIo.c
 */
extern	int		qIoWaitReadable(int fd, int timeoutms);
extern	int		qIoWaitWritable(int fd, int timeoutms);
extern	ssize_t		qIoRead(void *buf, int fd, size_t nbytes, int timeoutms);
extern	ssize_t		qIoWrite(int fd, const void *data, size_t nbytes, int timeoutms);
extern	off_t		qIoSend(int outfd, int infd, off_t nbytes, int timeoutms);
extern	ssize_t		qIoGets(char *buf, size_t bufsize, int fd, int timeoutms);
extern	ssize_t		qIoPuts(int fd, const char *str, int timeoutms);
extern	ssize_t		qIoPrintf(int fd, int timeoutms, const char *format, ...);

/*
 * qSocket.c
 */
extern	int		qSocketOpen(const char *hostname, int port, int timeoutms);
extern	bool		qSocketClose(int sockfd, int timeoutms);
extern	bool		qSocketGetAddr(struct sockaddr_in *addr, const char *hostname, int port);

/*
 * qFile.c
 */
extern	bool		qFileLock(int fd);
extern	bool		qFileUnlock(int fd);
extern	bool		qFileExist(const char *filepath);
extern	void*		qFileLoad(const char *filepath, size_t *nbytes);
extern	void*		qFileRead(FILE *fp, size_t *nbytes);
extern	ssize_t		qFileSave(const char *filepath, const void *buf, size_t size, bool append);
extern	char*		qFileReadLine(FILE *fp);
extern	bool		qFileMkdir(const char *dirpath, mode_t mode, bool recursive);

extern	char*		qFileGetName(const char *filepath);
extern	char*		qFileGetDir(const char *filepath);
extern	char*		qFileGetExt(const char *filepath);
extern	off_t		qFileGetSize(const char *filepath);

extern	bool		qFileCheckPath(const char *path);
extern	char*		qFileCorrectPath(char *path);
extern	char*		qFileGetAbsPath(char *buf, size_t bufsize, const char *path);

/*
 * qString.c
 */
extern	char*		qStrTrim(char *str);
extern	char*		qStrTrimTail(char *str);
extern	char*		qStrUnchar(char *str, char head, char tail);
extern	char*		qStrReplace(const char *mode, char *srcstr, const char *tokstr, const char *word);
extern	char*		qStrCpy(char *dst, size_t size, const char *src);
extern	char*		qStrDupf(const char *format, ...);
extern	char*		qStrGets(char *buf, size_t size, char **offset);
extern	char*		qStrUpper(char *str);
extern	char*		qStrLower(char *str);
extern	char*		qStrRev(char *str);
extern	char*		qStrTok(char *str, const char *delimiters, char *retstop, int *offset);
extern	Q_ENTRY*	qStrTokenizer(const char *str, const char *delimiters);
extern	char*		qStrCommaNumber(int number);
extern	char*		qStrCatf(char *str, const char *format, ...);
extern	char*		qStrDupBetween(const char *str, const char *start, const char *end);
extern	char*		qStrUnique(const char *seed);
extern	bool		qStrTest(int (*testfunc)(int), const char *str);
extern	bool		qStrIsEmail(const char *email);
extern	bool		qStrIsUrl(const char *url);
extern	bool		qStrIsIpv4Addr(const char *str);
extern	char*		qStrConvEncoding(const char *fromstr, const char *fromcode, const char *tocode, float mag);

/*
 * qEncode.c
 */
extern	Q_ENTRY*	qParseQueries(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count);
extern	char*		qUrlEncode(const void *bin, size_t size);
extern	size_t		qUrlDecode(char *str);
extern	char*		qBase64Encode(const void *bin, size_t size);
extern	size_t		qBase64Decode(char *str);

/*
 * qHash.c
 */
extern	unsigned char*	qHashMd5(const void *data, size_t nbytes);
extern	char*		qHashMd5Str(const void *data, size_t nbytes);
extern	char*		qHashMd5File(const char *filepath, size_t *nbytes);
extern	unsigned int	qHashFnv32(unsigned int max, const void *data, size_t nbytes);

/*
 * qTime.c
 */
extern	char*		qTimeGetLocalStrf(char *buf, int size, time_t utctime, const char *format);
extern	char*		qTimeGetLocalStr(time_t utctime);
extern	const char*	qTimeGetLocalStaticStr(time_t utctime);
extern	char*		qTimeGetGmtStrf(char *buf, int size, time_t utctime, const char *format);
extern	char*		qTimeGetGmtStr(time_t utctime);
extern	const char*	qTimeGetGmtStaticStr(time_t utctime);
extern	time_t		qTimeParseGmtStr(const char *gmtstr);

/*
 * qSystem.c
 */
extern	const char*	qDecoderVersion(void);
extern	const char*	qSysGetEnv(const char *envname, const char *nullstr);
extern	char*		qSysCmd(const char *cmd);
extern	bool		qSysGetIp(char *buf, size_t bufsize);

/*
 * qUtil.c
 */
extern	int		qCountRead(const char *filepath);
extern	bool		qCountSave(const char *filepath, int number);
extern	int		qCountUpdate(const char *filepath, int number);
extern	bool		qSedStr(Q_ENTRY *entry, const char *srcstr, FILE *fpout);
extern	bool		qSedFile(Q_ENTRY *entry, const char *filepath, FILE *fpout);

#ifdef __cplusplus
}
#endif

#endif /* _DOXYGEN_SKIP */

#endif /*_QDECODER_H */
