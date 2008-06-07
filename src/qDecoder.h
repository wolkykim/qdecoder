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
	Q_NLOBJ *next;			/*!< next object pointer */
} Q_ENTRY;

/**
 * Structure for hash-table data structure.
 */
typedef struct {
	int	max;			/*!< maximum hashtable size */
	int	num;			/*!< used slot counter */

	int	*count;			/*!< hash collision counter. 0 indicate empty slot, -1 is used for moved slot due to hash collision */
	int	*hash;			/*!< key hash. we use qFnv32Hash() to generate hash integer */
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
		char	dbtype[16+1];
		char	addr[31+1];
		int	port;
		char	username[31+1];
		char	password[31+1];
		char	database[31+1];
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

/*
 * qSession.c
 */
extern	Q_ENTRY*	qSessionInit(Q_ENTRY *request, const char *dirpath);
extern	bool		qSessionSetTimeout(Q_ENTRY *session, time_t seconds);
extern	const char*	qSessionGetID(Q_ENTRY *session);
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
int	qSemInit(char *keyfile, int keyid, int nsems, bool autodestroy);
int	qSemGetId(char *keyfile, int keyid);
bool	qSemEnter(int semid, int semno);
bool	qSemEnterNowait(int semid, int semno);
bool	qSemLeave(int semid, int semno);
bool	qSemCheck(int semid, int semno);
bool	qSemFree(int semid);

/*
 * qShm.c
 */
int	qShmInit(char *keyfile, int keyid, size_t size, bool autodestroy);
int	qShmGetId(char *keyfile, int keyid);
void	*qShmGet(int shmid);
bool	qShmFree(int shmid);

/*
 * qSocket.c
 */
extern	int		qSocketOpen(const char *hostname, int port);
extern	bool		qSocketClose(int sockfd);
extern	int		qSocketWaitReadable(int sockfd, int timeoutms);
extern	int		qSocketWaitWritable(int sockfd, int timeoutms);
extern	int		qSocketRead(char *binary, int sockfd, int size, int timeoutms);
extern	int		qSocketGets(char *str, int sockfd, int size, int timeoutms);
extern	int		qSocketWrite(int sockfd, char *binary, int size);
extern	int		qSocketPuts(int sockfd, char *str);
extern	int		qSocketPrintf(int sockfd, char *format, ...);
extern	ssize_t		qSocketSendfile(int sockfd, char *filepath, off_t offset, ssize_t size);
extern	int		qSocketSaveIntoFile(int outfd, int sockfd, int size, int timeoutms);
extern	int		qSocketSaveIntoMemory(char *mem, int sockfd, int size, int timeoutms);

/*
 * qDatabase.c
 */
Q_DB	*qDbInit(char *dbtype, char *addr, int port, char *username, char *password, char *database, bool autocommit);
bool	qDbOpen(Q_DB *db);
bool	qDbClose(Q_DB *db);
bool	qDbFree(Q_DB *db);
char	*qDbGetErrMsg(Q_DB *db);
bool	qDbPing(Q_DB *db);
bool	qDbGetLastConnStatus(Q_DB *db);

int	qDbExecuteUpdate(Q_DB *db, char *query);
int	qDbExecuteUpdatef(Q_DB *db, char *format, ...);

Q_DBRESULT *qDbExecuteQuery(Q_DB *db, char *query);
Q_DBRESULT *qDbExecuteQueryf(Q_DB *db, char *format, ...);
bool	qDbResultNext(Q_DBRESULT *result);
bool	qDbResultFree(Q_DBRESULT *result);

int     qDbGetCols(Q_DBRESULT *result);
int     qDbGetRows(Q_DBRESULT *result);
int     qDbGetRow(Q_DBRESULT *result);

char	*qDbGetValue(Q_DBRESULT *result, char *field);
int	qDbGetInt(Q_DBRESULT *result, char *field);
char	*qDbGetValueAt(Q_DBRESULT *result, int idx);
int	qDbGetIntAt(Q_DBRESULT *result, int idx);

bool	qDbBeginTran(Q_DB *db);
bool	qDbEndTran(Q_DB *db, bool commit);
bool	qDbCommit(Q_DB *db);
bool	qDbRollback(Q_DB *db);

/*
 * qEntry.c
 */
extern	Q_ENTRY*	qEntryInit(void);
extern	const Q_NLOBJ*	qEntryFirst(Q_ENTRY *entry);
extern	const Q_NLOBJ*	qEntryNext(Q_ENTRY *entry);
extern	int		qEntryRemove(Q_ENTRY *entry, const char *name);
extern	bool		qEntryPut(Q_ENTRY *entry, const char *name, const void *object, int size, bool update);
extern	bool		qEntryPutStr(Q_ENTRY *entry, const char *name, const char *str, bool update);
extern	bool		qEntryPutStrf(Q_ENTRY *entry,  const char *name, bool update, char *format, ...);
extern	bool		qEntryPutInt(Q_ENTRY *entry, const char *name, int num, bool update);
extern	const void*	qEntryGet(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetNext(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetNoCase(Q_ENTRY *entry, const char *name, int *size);
extern	const void*	qEntryGetLast(Q_ENTRY *entry, const char *name, int *size);
extern	const char*	qEntryGetStr(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrNext(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrNoCase(Q_ENTRY *entry, const char *name);
extern	const char*	qEntryGetStrLast(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetInt(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetIntNext(Q_ENTRY *entry, const char *name);
extern	int		qEntryGetIntNoCase(Q_ENTRY *entry, const char *name);
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
Q_HASHTBL *qHashtblInit(int max);
bool	qHashtblFree(Q_HASHTBL *tbl);
bool	qHashtblPut(Q_HASHTBL *tbl, char *key, char *value, int size);
bool	qHashtblPutStr(Q_HASHTBL *tbl, char *key, char *value);
bool	qHashtblPutInt(Q_HASHTBL *tbl, char *key, int value);
char	*qHashtblGet(Q_HASHTBL *tbl, char *key, int *size);
char	*qHashtblGetStr(Q_HASHTBL *tbl, char *key);
int	qHashtblGetInt(Q_HASHTBL *tbl, char *key);
char	*qHashtblGetFirstKey(Q_HASHTBL *tbl, int *idx);
char	*qHashtblGetNextKey(Q_HASHTBL *tbl, int *idx);
bool	qHashtblRemove(Q_HASHTBL *tbl, char *key);
bool	qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue);
bool	qHashtblStatus(Q_HASHTBL *tbl, int *used, int *max);

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
Q_OBSTACK *qObstackInit(void);
bool	qObstackGrow(Q_OBSTACK *obstack, const void *data, int size);
bool	qObstackGrowStr(Q_OBSTACK *obstack, const char *str);
bool	qObstackGrowStrf(Q_OBSTACK *obstack, const char *format, ...);
void	*qObstackFinish(Q_OBSTACK *obstack);
void	*qObstackGetFinal(Q_OBSTACK *obstack);
int	qObstackGetSize(Q_OBSTACK *obstack);
int	qObstackGetNum(Q_OBSTACK *obstack);
bool	qObstackFree(Q_OBSTACK *obstack);

/*
 * qLog.c
 */
Q_LOG	*qLogOpen(char *logbase, char *filenameformat, int rotateinterval, bool flush);
bool	qLogClose(Q_LOG *log);
bool	qLogSetConsole(Q_LOG *log, bool consoleout);
bool	qLogFlush(Q_LOG *log);
bool	qLog(Q_LOG *log, char *format, ...);

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
char	*qCharEncode(char *fromstr, char *fromcode, char *tocode, float mag);
unsigned char *qMd5Hash(char *data, int len);
char	*qMd5Str(char *data, int len);
char	*qMd5File(char *filepath);
unsigned int qFnv32Hash(char *str, unsigned int max);

/*
 * qString.c
 */
bool	qPrintf(int mode, char *format, ...);
bool	qPuts(int mode, char *buf);
extern	char*	qStrTrim(char *str);
extern	char*	qStrTrimTail(char *str);
char	*qStrReplace(char *mode, char *srcstr, char *tokstr, char *word);
bool	qStr09AZaz(char *str);
extern	char*		qStrncpy(char *dst, const char *src, size_t n);
char	*qStrupr(char *str);
char	*qStrlwr(char *str);
char	*qStristr(char *big, char *small);
char	*qStrrev(char *str);
char	*qStrtok(char *str, char *token, char *retstop);
char	*qitocomma(int value);
char    *qStrcat(char *str, char *format, ...);
char	*qStrdupBetween(char *str, char *start, char *end);
char	*qUniqId(void);

/*
 * qFile.c
 */
extern	bool		qFileLock(int fd);
extern	bool		qFileUnlock(int fd);
extern	bool		qFileExist(const char *filepath);
extern	off_t		qFileGetSize(const char *filepath);
extern	size_t		qFileSend(int outfd, int infd, size_t size);
extern	char*		qFileLoad(const char *filepath, int *size);
int	qSaveStr(char *sp, int spsize, char *filepath, char *mode);
char	*qfReadFile(FILE *fp);
char	*qfGetLine(FILE *fp);

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
char	*qGetTimeStrf(time_t univtime, char *savebuf, int bufsize, char *format);
char	*qGetTimeStr(time_t univtime);
char	*qGetLocaltimeStr(time_t univtime);
char	*qGetGmtimeStr(time_t univtime);
time_t	qParseGmtimeStr(char *gmtstr);

/*
 * qError.c
 */
void	qError(char *format, ...);
void	qErrorLog(char *file);
void	qErrorContact(char *msg);

#ifdef __cplusplus
}
#endif

#endif /* _DOXYGEN_SKIP */

#endif /*_QDECODER_H */
