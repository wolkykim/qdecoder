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
 * qDecoder Header file
 *
 * @file qDecoder.h
 */

#ifndef _QDECODER_H
#define _QDECODER_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>

/**
 * Base Structures used for Internal
 */
typedef struct Q_NLOBJ {
	char *name;			/*!< key name */
	void *object;			/*!< object data */
	int  size;			/*!< object size */
	struct Q_NLOBJ *next;		/*!< link pointer */
} Q_NLOBJ;

typedef struct {
	void *object;			/*!< object data */
	int  size;			/*!< object size */
} Q_OBJ;

/**
 * Structure for linked-list data structure.
 */
typedef struct {
	int num;			/*!< number of objects */
	int size;			/*!< total size of objects */
	Q_NLOBJ *first;			/*!< first object pointer */
	Q_NLOBJ *last;			/*!< last object pointer */
	Q_NLOBJ *next;			/*!< next object pointer used by only for qEntryFirst() and qEntryNext()*/
	Q_NLOBJ *cont;			/*!< next object pointer used by qEntry*Next*() and member functions*/
} Q_ENTRY;

/**
 * Structure for hash-table data structure.
 */
typedef struct {
	int	max;			/*!< maximum hashtable size */
	int	num;			/*!< used slot counter */

	int	*count;			/*!< hash collision counter. 0 indicate empty slot, -1 is used for moved slot due to hash collision */
	int	*hash;			/*!< key hash. we use qHashFnv32() to generate hash integer */
	char	**key;			/*!< key string */
	char	**value;		/*!< value */
	int	*size;			/*!< value size */
} Q_HASHTBL;

/**
 * Structure for hash-table data structure based on array.
 */
#define _Q_HASHARR_MAX_KEYSIZE		(31+1)
#define _Q_HASHARR_DEF_VALUESIZE	(32)

typedef struct {
	int	count;					/*!< hash collision counter. 0 indicates empty slot, -1 is used for moved slot due to hash collision, -2 is used for indicating linked block */
	int	hash;					/*!< key hash. we use qFnv32Hash() to generate hash integer */

	char	key[_Q_HASHARR_MAX_KEYSIZE];		/*!< key string which can be size truncated */
	int	keylen;					/*!< original key length */
	char	keymd5[16];				/*!< md5 hash of the key */

	char	value[_Q_HASHARR_DEF_VALUESIZE];	/*!< value */
	int	size;					/*!< value size */
	int	link;					/*!< next index of the value. */
} Q_HASHARR;

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
	char	logbase[1024];		/*!< directory which log file is located  */
	char	nameformat[256];	/*!< file naming format like qdecoder-%Y%m%d.log */

	char	filename[256];		/*!< generated filename according to the name format */
	char	logpath[1024];		/*!< generated system path of log file */
	FILE	*fp;			/*!< file pointer of logpath */

	bool	console;		/*!< flag for console print out */
	int	rotateinterval;		/*!< log file will be rotate in this interval seconds */
	int	nextrotate;		/*!< next rotate universal time, seconds */
	bool	flush;			/*!< flag for immediate sync */
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
	MYSQL_RES	*rs;
	MYSQL_FIELD	*fields;
	MYSQL_ROW	row;
	int		rows;
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
extern	Q_ENTRY*	qCgiRequestParseOption(Q_ENTRY *request, bool filemode, const char *basepath, int clearold);
extern	Q_ENTRY*	qCgiRequestParse(Q_ENTRY *request);
extern	Q_ENTRY*	qCgiRequestParseQueries(Q_ENTRY *request, const char *method);
extern	Q_ENTRY*	qCgiRequestParseCookies(Q_ENTRY *request);
extern	char*		qCgiRequestGetQueryString(const char *query_type);

/*
 * qCgiResponse.c
 */
extern	bool		qCgiResponseSetCookie(Q_ENTRY *request, const char *name, const char *value, int exp_days, const char *path, const char *domain, bool secure);
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
 * qConfig.c
 */
extern	Q_ENTRY*	qConfigParseFile(Q_ENTRY *config, const char *filepath, char sepchar);
extern	Q_ENTRY*	qConfigParseStr(Q_ENTRY *config, const char *str, char sepchar);

/*
 * qArg.c
 */
int	qArgMake(char *str, char **qlist);
int	qArgMatch(char *str, char **qlist);
int	qArgEmprint(int mode, char *str, char **qlist);
int	qArgPrint(char **qlist);
void	qArgFree(char **qlist);

/*
 * qAwk.c
 */
FILE	*qAwkOpen(char *filepath);
int	qAwkNext(FILE *fp, char array[][1024], char delim);
bool	qAwkClose(FILE *fp);
int	qAwkStr(char array[][1024], char *str, char delim);

/*
 * qSed.c
 */
extern	bool		qSedStr(Q_ENTRY *entry, const char *srcstr, FILE *fpout);
extern	bool		qSedFile(Q_ENTRY *entry, const char *filepath, FILE *fpout);

/*
 * qSem.c
 */
extern	int		qSemInit(const char *keyfile, int keyid, int nsems, bool autodestroy);
extern	int		qSemGetId(const char *keyfile, int keyid);
extern	bool		qSemEnter(int semid, int semno);
extern	bool		qSemEnterNowait(int semid, int semno);
extern	bool		qSemLeave(int semid, int semno);
extern	bool		qSemCheck(int semid, int semno);
extern	bool		qSemFree(int semid);

/*
 * qShm.c
 */
extern	int		qShmInit(const char *keyfile, int keyid, size_t size, bool autodestroy);
extern	int		qShmGetId(const char *keyfile, int keyid);
extern	void*		qShmGet(int shmid);
extern	bool		qShmFree(int shmid);

/*
 * qSocket.c
 */
extern	int		qSocketOpen(const char *hostname, int port);
extern	bool		qSocketClose(int sockfd);
extern	int		qSocketWaitReadable(int sockfd, int timeoutms);
extern	int		qSocketWaitWritable(int sockfd, int timeoutms);
extern	ssize_t		qSocketRead(void *binary, int sockfd, size_t nbytes, int timeoutms);
extern	ssize_t		qSocketGets(char *str, int sockfd, size_t nbytes, int timeoutms);
extern	ssize_t		qSocketWrite(int sockfd, const void *binary, size_t nbytes);
extern	ssize_t		qSocketPuts(int sockfd, const char *str);
extern	ssize_t		qSocketPrintf(int sockfd, const char *format, ...);
extern	ssize_t		qSocketSendfile(int sockfd, const char *filepath, off_t offset, size_t nbytes);
extern	ssize_t		qSocketSaveIntoFile(int outfd, int sockfd, size_t nbytes, int timeoutms);
extern	ssize_t		qSocketSaveIntoMemory(char *mem, int sockfd, size_t nbytes, int timeoutms);

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
extern	bool		qEntryPut(Q_ENTRY *entry, const char *name, const void *object, int size, bool update);
extern	bool		qEntryPutStr(Q_ENTRY *entry, const char *name, const char *str, bool update);
extern	bool		qEntryPutStrf(Q_ENTRY *entry, const char *name, bool update, char *format, ...);
extern	bool		qEntryPutInt(Q_ENTRY *entry, const char *name, int num, bool update);
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
extern	Q_HASHTBL*	qHashtblInit(int max);
extern	bool		qHashtblFree(Q_HASHTBL *tbl);
extern	bool		qHashtblPut(Q_HASHTBL *tbl, const char *key, const char *value, int size);
extern	bool		qHashtblPutStr(Q_HASHTBL *tbl, const char *key, const char *str);
extern	bool		qHashtblPutInt(Q_HASHTBL *tbl, const char *key, int number);
extern	char*		qHashtblGet(Q_HASHTBL *tbl, const char *key, int *size);
extern	char*		qHashtblGetStr(Q_HASHTBL *tbl, const char *key);
extern	int		qHashtblGetInt(Q_HASHTBL *tbl, const char *key);
extern	char*		qHashtblGetFirstKey(Q_HASHTBL *tbl, int *idx);
extern	char*		qHashtblGetNextKey(Q_HASHTBL *tbl, int *idx);
extern	bool		qHashtblRemove(Q_HASHTBL *tbl, const char *key);
extern	bool		qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue);
extern	bool		qHashtblStatus(Q_HASHTBL *tbl, int *used, int *max);

/*
 * qHasharr.c
 */
size_t	qHasharrSize(int max);
bool	qHasharrInit(Q_HASHARR *tbl, size_t memsize);
bool	qHasharrClear(Q_HASHARR *tbl);
bool	qHasharrPut(Q_HASHARR *tbl, char *key, char *value, int size);
bool	qHasharrPutStr(Q_HASHARR *tbl, char *key, char *value);
bool	qHasharrPutInt(Q_HASHARR *tbl, char *key, int value);
char	*qHasharrGet(Q_HASHARR *tbl, char *key, int *size);
char	*qHasharrGetStr(Q_HASHARR *tbl, char *key);
int	qHasharrGetInt(Q_HASHARR *tbl, char *key);
char	*qHasharrGetFirstKey(Q_HASHARR *tbl, int *idx);
char	*qHasharrGetNextKey(Q_HASHARR *tbl, int *idx);
bool	qHasharrRemove(Q_HASHARR *tbl, char *key);
bool	qHasharrPrint(Q_HASHARR *tbl, FILE *out);
bool	qHasharrStatus(Q_HASHARR *tbl, int *used, int *max);

/*
 * qObstack.c
 */
extern	Q_OBSTACK*	qObstackInit(void);
extern	bool		qObstackGrow(Q_OBSTACK *obstack, const void *object, size_t size);
extern	bool		qObstackGrowStr(Q_OBSTACK *obstack, const char *str);
extern	bool		qObstackGrowStrf(Q_OBSTACK *obstack, const char *format, ...);
extern	void*		qObstackFinish(Q_OBSTACK *obstack);
extern	void*		qObstackGetFinal(Q_OBSTACK *obstack);
extern	int		qObstackGetSize(Q_OBSTACK *obstack);
extern	int		qObstackGetNum(Q_OBSTACK *obstack);
extern	bool		qObstackFree(Q_OBSTACK *obstack);

/*
 * qLog.c
 */
extern	Q_LOG*		qLogOpen(const char *logbase, const char *filenameformat, int rotateinterval, bool flush);
extern	bool		qLogClose(Q_LOG *log);
extern	bool		qLogSetConsole(Q_LOG *log, bool consoleout);
extern	bool		qLogFlush(Q_LOG *log);
extern	bool		qLog(Q_LOG *log, const char *format, ...);

/*
 * qEnv.c
 */
char	*qGetenvDefault(char *nullstr, char *envname);

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
extern	char*		qHashMd5Str(const char *str, size_t nbytes);
extern	char*		qHashMd5File(const char *filepath, size_t *nbytes);
extern	unsigned int	qHashFnv32(unsigned int max, const void *data, size_t nbytes);

/*
 * qString.c
 */
bool	qPrintf(int mode, char *format, ...);
bool	qPuts(int mode, char *buf);
extern	char*		qStrTrim(char *str);
extern	char*		qStrTrimTail(char *str);
extern	char*		qStrReplace(const char *mode, char *srcstr, const char *tokstr, const char *word);
extern	char*		qStrCpy(char *dst, size_t dstsize, const char *src, size_t nbytes);
extern	char*		qStrUpper(char *str);
extern	char*		qStrLower(char *str);
extern	char*		qStrCaseStr(const char *s1, const char *s2);
extern	char*		qStrRev(char *str);
extern	char*		qStrTok(char *str, const char *token, char *retstop);
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
extern	ssize_t		qFileSend(int outfd, int infd, size_t nbytes);
extern	void*		qFileLoad(const char *filepath, size_t *nbytes);
extern	void*		qFileRead(FILE *fp, size_t *nbytes);
extern	ssize_t		qFileSave(const char *filepath, const void *buf, size_t size, bool append);
extern	char*		qFileReadLine(FILE *fp);
char	*qCmd(char *cmd);

/*
 * qValid.c
 */
bool	qCheckEmail(char *email);
bool	qCheckURL(char *url);

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

#ifdef __cplusplus
}
#endif

#endif /* _DOXYGEN_SKIP */

#endif /*_QDECODER_H */
