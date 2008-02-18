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

/**
 * Structure for linked-list data structure.
 */
typedef struct Q_ENTRY {
	char *name;		/*!< variable name */
	char *value;		/*!< value */
	struct Q_ENTRY *next;	/*!< next pointer */
} Q_ENTRY;

/**
 * Structure for hash-table data structure.
 */
typedef struct {
	int	max;			/*!< maximum hashtable size  */
	int	num;			/*!< used slot counter */

	int	*count;			/*!< hash collision counter. 0 indicate empty slot, -1 is used for temporary move slot dut to hash collision */
	int	*hash;			/*!< key hash. we use qFnv32Hash() to generate hash integer */
	char	**key;			/*!< key string */
	char	**value;		/*!< value */
	int	*size;			/*!< value size */
} Q_HASHTBL;

#define _Q_HASHARR_MAX_KEYLEN		(31)
#define _Q_HASHARR_DEF_VALUESIZE	(32)
/**
 * Structure for hash-table data structure based on array.
 */
typedef struct {
	int	count;					/*!< hash collision counter. 0 indicates empty slot, -1 is used for temporary move slot dut to hash collision, -2 is used for indicating linked block */
	int	hash;					/*!< key hash. we use qFnv32Hash() to generate hash integer */

	char	key[_Q_HASHARR_MAX_KEYLEN+1];		/*!< key string which can be size truncated */
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
	int	size;			/*!< total object size */
	int	num;			/*!< number of objects */
	Q_ENTRY *first;		/*!< first object pointer */
	Q_ENTRY	*last;		/*!< last object pointer */
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
#define _Q_ENABLE_MYSQL		(1)
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
	MYSQL		mysql;
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
 * qDecoder.c
 */
bool	qDecoderInit(bool filemode, char *upload_base, int clear_olderthan);
int	qDecoder(void);
char	*qGetValue(char *format, ...);
int	qGetInt(char *format, ...);
char	*qGetValueDefault(char *defstr, char *format, ...);
char	*qGetValueNotEmpty(char *errmsg, char *format, ...);
char	*qGetValueReplace(char *mode, char *name, char *tokstr, char *word);
Q_ENTRY	*qGetFirstEntry(void);
char	*qGetValueFirst(char *format, ...);
char	*qGetValueNext(void);
char	*qAdd(char *name, char *format, ...);
void	qRemove(char *format, ...);
char	qGetType(char *format, ...);
int	qPrint(FILE *out);
void	qFree(void);

bool	qCookieSet(char *name, char *value, int exp_days, char *path, char *domain, char *secure);
bool	qCookieRemove(char *name, char *path, char *domain, char *secure);
char	*qCookieGetValue(char *format, ...);
int	qCookieGetInt(char *format, ...);

void	qReset(void);

/*
 * qSession.c
 */
int	qSession(char *repository);
char	*qSessionAdd(char *name, char *format, ...);
int	qSessionAddInt(char *name, int valueint);
int	qSessionUpdateInt(char *name, int plusint);
void	qSessionRemove(char *format, ...);
char	*qSessionGetValue(char *format, ...);
int	qSessionGetInt(char *format, ...);
time_t	qSessionSetTimeout(time_t seconds);
char	*qSessionGetID(void);
time_t	qSessionGetCreated(void);
int	qSessionPrint(FILE *out);
void	qSessionSave(void);
void	qSessionFree(void);
void	qSessionDestroy(void);

/*
 * qHttpHeader.c
 */
void	qContentType(char *mimetype);
int	qGetContentFlag(void);
void	qResetContentFlag(void);
void	qRedirect(char *url);
void	qJavaScript(char *format, ...);

/*
 * qDownload.c
 */
int	qDownload(char *filename);
int	qDownloadMime(char *filename, char *mime);

/*
 * qfDecoder.c
 */
Q_ENTRY	*qfDecoder(char *filename);
char	*qfGetValue(Q_ENTRY *first, char *format, ...);
int	qfGetInt(Q_ENTRY *first, char *format, ...);
char	*qfGetValueFirst(Q_ENTRY *first, char *format, ...);
char	*qfGetValueNext(void);
int	qfPrint(Q_ENTRY *first, FILE *out);
void	qfFree(Q_ENTRY *first);

/*
 * qsDecoder.c
 */
Q_ENTRY	*qsDecoder(char *str);
char	*qsGetValue(Q_ENTRY *first, char *format, ...);
int	qsGetInt(Q_ENTRY *first, char *format, ...);
char	*qsGetValueFirst(Q_ENTRY *first, char *format, ...);
char	*qsGetValueNext(void);
int	qsPrint(Q_ENTRY *first, FILE *out);
void	qsFree(Q_ENTRY *first);

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
FILE	*qAwkOpen(char *filename);
int	qAwkNext(FILE *fp, char array[][1024], char delim);
bool	qAwkClose(FILE *fp);
int	qAwkStr(char array[][1024], char *str, char delim);

/*
 * qSed.c
 */
Q_ENTRY	*qSedAdd(Q_ENTRY *first, char *name, char *format, ...);
Q_ENTRY *qSedAddDirect(Q_ENTRY *first, char *name, char *value);
int	qSedStr(Q_ENTRY *first, char *srcstr, FILE *fpout);
int	qSedFile(Q_ENTRY *first, char *filename, FILE *fpout);
int	qSedPrint(Q_ENTRY *first, FILE *out);
void	qSedFree(Q_ENTRY *first);

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
int	qSocketOpen(char *hostname, int port);
int	qSocketClose(int sockfd);
int	qSocketWaitReadable(int sockfd, int timeoutms);
int	qSocketRead(int sockfd, char *binary, int size, int timeoutms);
int	qSocketGets(int sockfd, char *str, int size, int timeoutms);
int	qSocketWrite(int sockfd, char *binary, int size);
int	qSocketPuts(int sockfd, char *str);
int	qSocketPrintf(int sockfd, char *format, ...);
ssize_t	qSocketSendFile(int sockfd, char *filepath, off_t offset);
int	qSocketSaveIntoFile(int sockfd, int size, char *filepath, char *mode, int timeoutms);
int	qSocketSaveIntoMemory(int sockfd, int size, char *mem, int timeoutms);
FILE	*qSocketConv2file(int sockfd);

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

Q_ENTRY	*qEntryAdd(Q_ENTRY *first, char *name, char *value, int flag);
Q_ENTRY	*qEntryAddInt(Q_ENTRY *first, char *name, int value, int flag);
Q_ENTRY	*qEntryRemove(Q_ENTRY *first, char *name);
char	*qEntryGetValue(Q_ENTRY *first, char *name);
char	*qEntryGetValueLast(Q_ENTRY *first, char *name);
char	*qEntryGetValueNoCase(Q_ENTRY *first, char *name);
int	qEntryGetInt(Q_ENTRY *first, char *name);
int	qEntryGetIntLast(Q_ENTRY *first, char *name);
int	qEntryGetIntNoCase(Q_ENTRY *first, char *name);
int	qEntryGetNo(Q_ENTRY *first, char *name);
Q_ENTRY	*qEntryReverse(Q_ENTRY *first);
int	qEntryPrint(Q_ENTRY *first, FILE *out);
void	qEntryFree(Q_ENTRY *first);
int	qEntrySave(Q_ENTRY *first, char *filename, bool encodevalue);
Q_ENTRY	*qEntryLoad(char *filename, bool decodevalue);

/*
 * qHashtbl.c
 */
Q_HASHTBL *qHashtblInit(int max);
bool	qHashtblPut(Q_HASHTBL *tbl, char *key, char *value, int size);
bool	qHashtblPutStr(Q_HASHTBL *tbl, char *key, char *value);
bool	qHashtblPutInt(Q_HASHTBL *tbl, char *key, int value);
char	*qHashtblGet(Q_HASHTBL *tbl, char *key, int *size);
char	*qHashtblGetStr(Q_HASHTBL *tbl, char *key);
int	qHashtblGetInt(Q_HASHTBL *tbl, char *key);
bool	qHashtblRemove(Q_HASHTBL *tbl, char *key);
void	qHashtblPrint(Q_HASHTBL *tbl, FILE *out, bool showvalue);
bool	qHashtblFree(Q_HASHTBL *tbl);
void	qHashtblStatus(Q_HASHTBL *tbl, int *used, int *max);

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
bool	qHasharrRemove(Q_HASHARR *tbl, char *key);
void	qHasharrPrint(Q_HASHARR *tbl, FILE *out);
void	qHasharrStatus(Q_HASHARR *tbl, int *used, int *max);

/*
 * qObstack.c
 */
Q_OBSTACK *qObstackInit(void);
bool	qObstackGrow(Q_OBSTACK *obstack, void *data, int size);
bool	qObstackGrowStr(Q_OBSTACK *obstack, char *str);
bool	qObstackGrowStrf(Q_OBSTACK *obstack, char *format, ...);
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
char	*qUrlEncode(char *str);
char	*qUrlDecode(char *str);
char	*qCharEncode(char *fromstr, char *fromcode, char *tocode, float mag);
unsigned char *qMd5Hash(char *data, int len);
char	*qMd5Str(char *data, int len);
char	*qMd5File(char *filename);
unsigned int qFnv32Hash(char *str, unsigned int max);

/*
 * qString.c
 */
bool	qPrintf(int mode, char *format, ...);
bool	qPuts(int mode, char *buf);
char	*qRemoveSpace(char *str);
char	*qRemoveTailSpace(char *str);
char	*qStrReplace(char *mode, char *srcstr, char *tokstr, char *word);
bool	qStr09AZaz(char *str);
char	*qStrncpy(char *dst, char *src, size_t sizeofdst);
char	*qStrupr(char *str);
char	*qStrlwr(char *str);
char	*qStristr(char *big, char *small);
char	*qStrtok(char *str, char *token, char *retstop);
char	*qitocomma(int value);
char    *qStrcat(char *str, char *format, ...);
char	*qStrdupBetween(char *str, char *start, char *end);
char	*qUniqId(void);

/*
 * qFile.c
 */
FILE	*qFileOpen(char *path, char *mode);
int	qFileClose(FILE *stream);
char	*qfReadFile(FILE *fp);
char	*qfGetLine(FILE *fp);
bool	qCheckFile(char *format, ...);
int	qCatFile(char *format, ...);
char	*qReadFile(char *filename, int *size);
int	qSaveStr(char *sp, int spsize, char *filename, char *mode);
long	qFileSize(char *filename);
char	*qCmd(char *cmd);

/*
 * qValid.c
 */
bool	qCheckEmail(char *email);
bool	qCheckURL(char *url);

/*
 * qCount.c
 */
int	qCountRead(char *filename);
bool	qCountSave(char *filename, int number);
int	qCountUpdate(char *filename, int number);

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
