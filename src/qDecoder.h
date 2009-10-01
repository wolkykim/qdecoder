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
 * qDecoder Header file
 *
 * @file qDecoder.h
 */

#ifndef _QDECODER_H
#define _QDECODER_H

#define _Q_VERSION			"9.0.4"

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

/**
 * Base Structures used for Internal - named object
 */
typedef struct Q_NLOBJ {
	char *name;			/*!< key name */
	void *object;			/*!< object data */
	int  size;			/*!< object size */
	struct Q_NLOBJ *next;		/*!< link pointer */
} Q_NLOBJ;

/**
 * Base Structures used for Internal - object
 */
typedef struct {
	void *object;			/*!< object data */
	int  size;			/*!< object size */
} Q_OBJ;

/**
 * Structure for linked-list data structure.
 */
typedef struct {
	int num;			/*!< number of objects */
	size_t size;			/*!< total size of objects */
	Q_NLOBJ *first;			/*!< first object pointer */
	Q_NLOBJ *last;			/*!< last object pointer */
	Q_NLOBJ *next;			/*!< next object pointer used by qEntryFirst() and qEntryNext() */
	Q_NLOBJ *cont;			/*!< next object pointer used by qEntryGet*() */
} Q_ENTRY;

#define _Q_HASHTBL_RESIZE_MAG		(2)
#define _Q_HASHTBL_DEF_THRESHOLD	(80)
/**
 * Structure for hash-table data structure.
 */
typedef struct {
	int	max;			/*!< maximum hashtable size */
	int	num;			/*!< used slot counter */
	int	threshold;		/*!< if the percent of used slot counter exceeds this threshold percent, new larger table(max * _Q_HASHTBL_RESIZE_MAG) is allocated */
	int	resizeat;		/*!< calculated used amount = (max * threshold) / 100 */

	int	*count;			/*!< hash collision counter. 0 indicate empty slot, -1 is used for moved slot due to hash collision */
	int	*hash;			/*!< key hash. we use qHashFnv32() to generate hash integer */
	char	**key;			/*!< key string */
	void	**value;		/*!< value */
	int	*size;			/*!< value size */
} Q_HASHTBL;

#define _Q_HASHARR_MAX_KEYSIZE		(31+1)
#define _Q_HASHARR_DEF_VALUESIZE	(32)
/**
 * Structure for hash-table data structure based on array.
 */
typedef struct {
	int	count;					/*!< hash collision counter. 0 indicates empty slot, -1 is used for collision resolution, -2 is used for indicating linked block */
	int	hash;					/*!< key hash. we use qFnv32Hash() to generate hash integer */

	char	key[_Q_HASHARR_MAX_KEYSIZE];		/*!< key string, it can be truncated */
	int	keylen;					/*!< original key length */
	unsigned char keymd5[16];			/*!< md5 hash of the key */

	unsigned char value[_Q_HASHARR_DEF_VALUESIZE];	/*!< value */
	int	size;					/*!< value size */
	int	link;					/*!< next index of the value */
} Q_HASHARR;

/**
 * Structure for array-based circular-queue data structure.
 */
typedef struct {
	int	max;			/*!< maximum queue slots */
	int	used;			/*!< used queue slots */

	int	head;			/*!< head pointer */
	int	tail;			/*!< tail pointer */

	size_t	objsize;		/*!< object size */
	void	*objarr;		/*!< external queue data memory pointer */
} Q_QUEUE;

/**
 * Structure for obstack data structure.
 */
typedef struct {
	Q_ENTRY *stack;			/*!< first object pointer */
	void	*final;			/*!< final object pointer */
} Q_OBSTACK;

/**
 * Structure for file log.
 */
typedef struct {
	char	filepathfmt[PATH_MAX];	/*!< file file naming format like /somepath/qdecoder-%Y%m%d.log */
	char	filepath[PATH_MAX];	/*!< generated system path of log file */
	FILE	*fp;			/*!< file pointer of logpath */
	int	rotateinterval;		/*!< log file will be rotate in this interval seconds */
	int	nextrotate;		/*!< next rotate universal time, seconds */
	bool	flush;			/*!< flag for immediate flushing */

	FILE	*outfp;			/*!< stream pointer for duplication */
	bool	outflush;		/*!< flag for immediate flushing for duplicated stream */
} Q_LOG;

/* Database Support*/
#ifdef _mysql_h
#define _Q_ENABLE_MYSQL				(1)
// mysql specific connector option
#define _Q_MYSQL_OPT_RECONNECT			(1)
#define _Q_MYSQL_OPT_CONNECT_TIMEOUT		(10)
#define _Q_MYSQL_OPT_READ_TIMEOUT		(30)
#define _Q_MYSQL_OPT_WRITE_TIMEOUT		(30)
#endif

/**
 * Structure for independent database interface.
 */
typedef struct {
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

	// for mysql database
#ifdef _Q_ENABLE_MYSQL
	MYSQL		*mysql;
#endif
} Q_DB;

/**
 * Structure for database result set.
 */
typedef struct {
#ifdef _Q_ENABLE_MYSQL
	bool		fetchtype;
	MYSQL_RES	*rs;
	MYSQL_FIELD	*fields;
	MYSQL_ROW	row;
	int		cols;
	int		cursor;
#endif
} Q_DBRESULT;

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
extern	bool		qHtmlIsEmail(const char *email);
extern	bool		qHtmlIsUrl(const char *url);

/*
 * qSocket.c
 */
extern	int		qSocketOpen(const char *hostname, int port, int timeoutms);
extern	bool		qSocketClose(int sockfd);
extern	int		qSocketWaitReadable(int sockfd, int timeoutms);
extern	int		qSocketWaitWritable(int sockfd, int timeoutms);
extern	ssize_t		qSocketRead(void *binary, int sockfd, size_t nbytes, int timeoutms);
extern	ssize_t		qSocketGets(char *str, int sockfd, size_t nbytes, int timeoutms);
extern	ssize_t		qSocketWrite(int sockfd, const void *binary, size_t nbytes);
extern	ssize_t		qSocketPuts(int sockfd, const char *str);
extern	ssize_t		qSocketPrintf(int sockfd, const char *format, ...);
extern	off_t		qSocketSendfile(int sockfd, int fd, off_t offset, off_t nbytes);
extern	off_t		qSocketSaveIntoFile(int fd, int sockfd, off_t nbytes, int timeoutms);
extern	ssize_t		qSocketSaveIntoMemory(char *mem, int sockfd, size_t nbytes, int timeoutms);

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
extern	Q_DB*		qDbInit(const char *dbtype, const char *addr, int port, const char *database, const char *username, const char *password, bool autocommit);
extern	bool		qDbOpen(Q_DB *db);
extern	bool		qDbClose(Q_DB *db);
extern	bool		qDbFree(Q_DB *db);
extern	const char*	qDbGetError(Q_DB *db, unsigned int *errorno);
extern	bool		qDbPing(Q_DB *db);
extern	bool		qDbGetLastConnStatus(Q_DB *db);
extern	bool		qDbSetFetchType(Q_DB *db, bool use);

extern	int		qDbExecuteUpdate(Q_DB *db, const char *query);
extern	int		qDbExecuteUpdatef(Q_DB *db, const char *format, ...);

extern	Q_DBRESULT*	qDbExecuteQuery(Q_DB *db, const char *query);
extern	Q_DBRESULT*	qDbExecuteQueryf(Q_DB *db, const char *format, ...);
extern	bool		qDbResultNext(Q_DBRESULT *result);
extern	bool		qDbResultFree(Q_DBRESULT *result);

extern	int     	qDbGetCols(Q_DBRESULT *result);
extern	int     	qDbGetRows(Q_DBRESULT *result);
extern	int     	qDbGetRow(Q_DBRESULT *result);

extern	const char*	qDbGetStr(Q_DBRESULT *result, const char *field);
extern	const char*	qDbGetStrAt(Q_DBRESULT *result, int idx);
extern	int		qDbGetInt(Q_DBRESULT *result, const char *field);
extern	int		qDbGetIntAt(Q_DBRESULT *result, int idx);

extern	bool		qDbBeginTran(Q_DB *db);
extern	bool		qDbEndTran(Q_DB *db, bool commit);
extern	bool		qDbCommit(Q_DB *db);
extern	bool		qDbRollback(Q_DB *db);

/*
 * qEntry.c
 */
extern	Q_ENTRY*	qEntryInit(void);
extern	const Q_NLOBJ*	qEntryFirst(Q_ENTRY *entry);
extern	const Q_NLOBJ*	qEntryNext(Q_ENTRY *entry);
extern	int		qEntryRemove(Q_ENTRY *entry, const char *name);
extern	bool		qEntryPut(Q_ENTRY *entry, const char *name, const void *object, int size, bool replace);
extern	bool		qEntryPutStr(Q_ENTRY *entry, const char *name, const char *str, bool replace);
extern	bool		qEntryPutStrf(Q_ENTRY *entry, const char *name, bool replace, char *format, ...);
extern	bool		qEntryPutStrParsed(Q_ENTRY *entry, const char *name, const char *str, bool replace);
extern	bool		qEntryPutInt(Q_ENTRY *entry, const char *name, int num, bool replace);
extern	const void*	qEntryGet(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetCase(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetNext(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetNextCase(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetLast(Q_ENTRY *entry, const char *name, int *size);
extern	const char*	qEntryGetStr(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrCase(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrf(Q_ENTRY *entry, char *format, ...);
extern	const char*	qEntryGetStrNext(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrNextCase(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrLast(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetInt(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetIntCase(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetIntf(Q_ENTRY *entry, char *format, ...);
extern	int		qEntryGetIntNext(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetIntNextCase(Q_ENTRY *entry, const char *name);
extern	int 		qEntryGetIntLast(Q_ENTRY *entry, const char *name);
extern	char*		qEntryParseStr(Q_ENTRY *entry, const char *str);
extern	int 		qEntryGetNum(Q_ENTRY *entry);
extern	int		qEntryGetNo(Q_ENTRY *entry, const char *name);
extern	bool		qEntryReverse(Q_ENTRY *entry);
extern	bool		qEntryPrint(Q_ENTRY *entry, FILE *out, bool print_object);
extern	bool		qEntryFree(Q_ENTRY *entry);
extern	bool		qEntrySave(Q_ENTRY *entry, const char *filepath, char sepchar, bool encode);
extern	int		qEntryLoad(Q_ENTRY *entry, const char *filepath, char sepchar, bool decode);

/*
 * qHashtbl.c
 */
extern	Q_HASHTBL*	qHashtblInit(int max, bool resize, int threshold);
extern	bool		qHashtblResize(Q_HASHTBL *tbl, int max);
extern	bool		qHashtblFree(Q_HASHTBL *tbl);
extern	bool		qHashtblPut(Q_HASHTBL *tbl, const char *key, const void *value, int size);
extern	bool		qHashtblPutStr(Q_HASHTBL *tbl, const char *key, const char *value);
extern	bool		qHashtblPutInt(Q_HASHTBL *tbl, const char *key, int value);
extern	void*		qHashtblGet(Q_HASHTBL *tbl, const char *key, int *size);
extern	char*		qHashtblGetStr(Q_HASHTBL *tbl, const char *key);
extern	int		qHashtblGetInt(Q_HASHTBL *tbl, const char *key);
extern	const char*	qHashtblGetFirstKey(Q_HASHTBL *tbl, int *idx);
extern	const char*	qHashtblGetNextKey(Q_HASHTBL *tbl, int *idx);
extern	bool		qHashtblRemove(Q_HASHTBL *tbl, const char *key);
extern	bool		qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue);
extern	bool		qHashtblStatus(Q_HASHTBL *tbl, int *used, int *max);

/*
 * qHasharr.c
 */
extern	size_t		qHasharrSize(int max);
extern	int		qHasharrInit(Q_HASHARR *tbl, size_t memsize);
extern	bool		qHasharrClear(Q_HASHARR *tbl);
extern	bool		qHasharrPut(Q_HASHARR *tbl, const char *key, const void *value, int size);
extern	bool		qHasharrPutStr(Q_HASHARR *tbl, const char *key, const char *value);
extern	bool		qHasharrPutInt(Q_HASHARR *tbl, const char *key, int value);
extern	void*		qHasharrGet(Q_HASHARR *tbl, const char *key, int *size);
extern	char*		qHasharrGetStr(Q_HASHARR *tbl, const char *key);
extern	int		qHasharrGetInt(Q_HASHARR *tbl, const char *key);
extern	const char*	qHasharrGetFirstKey(Q_HASHARR *tbl, int *idx);
extern	const char*	qHasharrGetNextKey(Q_HASHARR *tbl, int *idx);
extern	bool		qHasharrRemove(Q_HASHARR *tbl, const char *key);
extern	bool		qHasharrPrint(Q_HASHARR *tbl, FILE *out);
extern	bool		qHasharrStatus(Q_HASHARR *tbl, int *used, int *max);

/*
 * qQueue.c
 */
extern	size_t		qQueueSize(int max, size_t objsize);
extern	int		qQueueInit(Q_QUEUE *queue, void* datamem, size_t datamemsize, size_t objsize);
extern	bool		qQueueClear(Q_QUEUE *queue);
extern	bool		qQueuePush(Q_QUEUE *queue, const void *object);
extern	bool		qQueuePopFirst(Q_QUEUE *queue, void *object);
extern	bool		qQueuePopLast(Q_QUEUE *queue, void *object);
extern	bool		qQueueStatus(Q_QUEUE *queue, int *used, int *max);

/*
 * qObstack.c
 */
extern	Q_OBSTACK*	qObstackInit(void);
extern	bool		qObstackGrow(Q_OBSTACK *obstack, const void *object, size_t size);
extern	bool		qObstackGrowStr(Q_OBSTACK *obstack, const char *str);
extern	bool		qObstackGrowStrf(Q_OBSTACK *obstack, const char *format, ...);
extern	void*		qObstackFinish(Q_OBSTACK *obstack);
extern	void*		qObstackGetFinal(Q_OBSTACK *obstack);
extern	size_t		qObstackGetSize(Q_OBSTACK *obstack);
extern	int		qObstackGetNum(Q_OBSTACK *obstack);
extern	bool		qObstackFree(Q_OBSTACK *obstack);

/*
 * qConfig.c
 */
extern	Q_ENTRY*	qConfigParseFile(Q_ENTRY *config, const char *filepath, char sepchar);
extern	Q_ENTRY*	qConfigParseStr(Q_ENTRY *config, const char *str, char sepchar);

/*
 * qLog.c
 */
extern	Q_LOG*		qLogOpen(const char *filepathfmt, int rotateinterval, bool flush);
extern	bool		qLogClose(Q_LOG *log);
extern	bool		qLogDuplicate(Q_LOG *log, FILE *outfp, bool flush);
extern	bool		qLogFlush(Q_LOG *log);
extern	bool		qLog(Q_LOG *log, const char *format, ...);

/*
 * qString.c
 */
extern	char*		qStrTrim(char *str);
extern	char*		qStrTrimTail(char *str);
extern	char*		qStrUnchar(char *str, char head, char tail);
extern	char*		qStrReplace(const char *mode, char *srcstr, const char *tokstr, const char *word);
extern	char*		qStrCpy(char *dst, size_t dstsize, const char *src, size_t nbytes);
extern	char*		qStrUpper(char *str);
extern	char*		qStrLower(char *str);
extern	char*		qStrCaseStr(const char *s1, const char *s2);
extern	char*		qStrRev(char *str);
extern	char*		qStrTok(char *str, const char *token, char *retstop);
extern	Q_ENTRY*	qStrTokenizer(char *str, const char *delimiters);
extern	char*		qStrCommaNumber(int number);
extern	char*		qStrCatf(char *str, const char *format, ...);
extern	char*		qStrDupBetween(const char *str, const char *start, const char *end);
extern	char*		qStrUnique(const char *seed);
extern	bool		qStrIsAlnum(const char *str);
extern	char*		qStrConvEncoding(const char *fromstr, const char *fromcode, const char *tocode, float mag);

/*
 * qFile.c
 */
extern	bool		qFileLock(int fd);
extern	bool		qFileUnlock(int fd);
extern	bool		qFileExist(const char *filepath);
extern	char*		qFileGetName(const char *filepath);
extern	char*		qFileGetDir(const char *filepath);
extern	char*		qFileGetExt(const char *filepath);
extern	off_t		qFileGetSize(const char *filepath);
extern	off_t		qFileSend(int outfd, int infd, off_t nbytes);
extern	void*		qFileLoad(const char *filepath, size_t *nbytes);
extern	void*		qFileRead(FILE *fp, size_t *nbytes);
extern	ssize_t		qFileSave(const char *filepath, const void *buf, size_t size, bool append);
extern	char*		qFileReadLine(FILE *fp);

/*
 * qEncode.c
 */
extern	Q_ENTRY*	qDecodeQueryString(Q_ENTRY *entry, const char *query, char equalchar, char sepchar, int *count);
extern	char*		qEncodeUrl(const char *str);
extern	char*		qDecodeUrl(char *str);

/*
 * qHash.c
 */
extern	unsigned char*	qHashMd5(const void *data, size_t nbytes);
extern	char*		qHashMd5Str(const void *data, size_t nbytes);
extern	char*		qHashMd5File(const char *filepath, size_t *nbytes);
extern	unsigned int	qHashFnv32(unsigned int max, const void *data, size_t nbytes);

/*
 * qSed.c
 */
extern	bool		qSedStr(Q_ENTRY *entry, const char *srcstr, FILE *fpout);
extern	bool		qSedFile(Q_ENTRY *entry, const char *filepath, FILE *fpout);

/*
 * qCount.c
 */
extern	int		qCountRead(const char *filepath);
extern	bool		qCountSave(const char *filepath, int number);
extern	int		qCountUpdate(const char *filepath, int number);

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
extern	const char*	qSysGetEnv(const char *envname, const char *nullstr);
extern	char*		qSysCmd(const char *cmd);
extern	const char*	qDecoderVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* _DOXYGEN_SKIP */

#endif /*_QDECODER_H */
