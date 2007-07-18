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
 * qDecoder's linked list type
 */
typedef struct Q_ENTRY {
	char *name;		/*!< variable name */
	char *value;		/*!< value */
	struct Q_ENTRY *next;	/*!< next pointer */
} Q_ENTRY;

/* Database Support*/
#ifdef _mysql_h
#define _Q_WITH_MYSQL		(1)
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
#ifdef _Q_WITH_MYSQL
	MYSQL		mysql;
#endif
} Q_DB;

/**
 * Structure for Database Result Set
 */
typedef struct {
#ifdef _Q_WITH_MYSQL
	MYSQL_RES	*rs;
	MYSQL_FIELD	*fields;
	MYSQL_ROW	row;
	int		rows;
	int		cols;
	int		cursor;
#endif
} Q_DBRESULT;

/**
 * Structure for Log
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
char	*qValue(char *format, ...);
int	qiValue(char *format, ...);
char	*qValueDefault(char *defstr, char *format, ...);
char	*qValueNotEmpty(char *errmsg, char *format, ...);
char	*qValueReplace(char *mode, char *name, char *tokstr, char *word);
Q_ENTRY	*qGetFirstEntry(void);
char	*qValueFirst(char *format, ...);
char	*qValueNext(void);
char	*qValueAdd(char *name, char *format, ...);
void	qValueRemove(char *format, ...);
char	qValueType(char *format, ...);
int	qPrint(void);
void	qFree(void);

bool	qCookieSet(char *name, char *value, int exp_days, char *path, char *domain, char *secure);
bool	qCookieRemove(char *name, char *path, char *domain, char *secure);
char	*qCookieValue(char *format, ...);

void	qReset(void);

/*
 * qSession.c
 */
int	qSession(char *repository);
char	*qSessionAdd(char *name, char *format, ...);
int	qSessionAddInteger(char *name, int valueint);
int	qSessionUpdateInteger(char *name, int plusint);
void	qSessionRemove(char *format, ...);
char	*qSessionValue(char *format, ...);
int	qSessionValueInteger(char *format, ...);
int	qSessionPrint(void);
void	qSessionSave(void);
void	qSessionFree(void);
void	qSessionDestroy(void);
time_t	qSessionSetTimeout(time_t seconds);
char	*qSessionGetID(void);
time_t	qSessionGetCreated(void);

/*
* qfDecoder.c
 */
Q_ENTRY	*qfDecoder(char *filename);
char	*qfValue(Q_ENTRY *first, char *format, ...);
int	qfiValue(Q_ENTRY *first, char *format, ...);
char	*qfValueFirst(Q_ENTRY *first, char *format, ...);
char	*qfValueNext(void);
int	qfPrint(Q_ENTRY *first);
void	qfFree(Q_ENTRY *first);

/*
 * qsDecoder.c
 */
Q_ENTRY	*qsDecoder(char *str);
char	*qsValue(Q_ENTRY *first, char *format, ...);
int	qsiValue(Q_ENTRY *first, char *format, ...);
char	*qsValueFirst(Q_ENTRY *first, char *format, ...);
char	*qsValueNext(void);
int	qsPrint(Q_ENTRY *first);
void	qsFree(Q_ENTRY *first);

/*
 * qHttpHeader.c
 */
void	qContentType(char *mimetype);
int	qGetContentFlag(void);
void	qResetContentFlag(void);
void	qRedirect(char *url);
void	qJavaScript(char *format, ...);

/*
 * qError.c
 */
void	qError(char *format, ...);
void	qErrorLog(char *file);
void	qErrorContact(char *msg);

/*
 * qEnv.c
 */
char	*qGetenvDefault(char *nullstr, char *envname);

/*
 * qEncode.c
 */
char	*qURLencode(char *str);
char	*qURLdecode(char *str);
char	*qMD5Str(char *string);
char	*qMD5File(char *filename);
unsigned int qFnv32Hash(char *str, unsigned int max);

/*
 * qString.c
 */
int	qPrintf(int mode, char *format, ...);
void	qPuts(int mode, char *buf);
char	*qRemoveSpace(char *str);
char	*qRemoveTailSpace(char *str);
char	*qStrReplace(char *mode, char *srcstr, char *tokstr, char *word);
int	qStr09AZaz(char *str);
char	*qStrupr(char *str);
char	*qStrlwr(char *str);
char	*qStristr(char *big, char *small);
int	qStricmp(char *s1, char *s2);
int	qStrincmp(char *s1, char *s2, size_t len);
char	*qStrtok(char *str, char *token, char *retstop);
char	*qitocomma(int value);
char    *qStrcat(char *str, char *format, ...);
char	*qStrdupBetween(char *str, char *start, char *end);
char	*qUniqId(void);

/*
 * qFile.c
 */
FILE	*qfopen(char *path, char *mode);
int	qfclose(FILE *stream);
bool	qCheckFile(char *format, ...);
int	qCatFile(char *format, ...);
char	*qReadFile(char *filename, int *size);
int	qSaveStr(char *sp, int spsize, char *filename, char *mode);
long	qFileSize(char *filename);
char	*qfGetLine(FILE *fp);
char	*qfGets(FILE *fp);
char	*qCmd(char *cmd);

/*
 * qValid.c
 */
int	qCheckEmail(char *email);
int	qCheckURL(char *url);

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
Q_ENTRY	*qSedArgAdd(Q_ENTRY *first, char *name, char *format, ...);
Q_ENTRY *qSedArgAddDirect(Q_ENTRY *first, char *name, char *value);
int	qSedArgPrint(Q_ENTRY *first);
void	qSedArgFree(Q_ENTRY *first);
int	qSedStr(Q_ENTRY *first, char *srcstr, FILE *fpout);
int	qSedFile(Q_ENTRY *first, char *filename, FILE *fpout);

/*
 * qCount.c
 */
int	qCountRead(char *filename);
bool	qCountSave(char *filename, int number);
int	qCountUpdate(char *filename, int number);

/*
 * qDownload.c
 */
int	qDownload(char *filename);
int	qDownloadMime(char *filename, char *mime);

/*
 * qTime.c
 */
struct tm *qGetTime(void);
time_t	qGetGMTime(char *gmt, time_t plus_sec);
char	*qGetTimeStr(void);

/*
 * qLog.c
 */
Q_LOG	*qLogOpen(char *logbase, char *filenameformat, int rotateinterval, bool flush);
bool	qLogClose(Q_LOG *log);
bool	qLogSetConsole(Q_LOG *log, bool onoff);
bool	qLogFlush(Q_LOG *log);
bool	qLog(Q_LOG *log, char *format, ...);

/*
 * qSocket.c
 */
int	qSocketOpen(char *hostname, int port);
int	qSocketClose(int sockfd);
int	qSocketWaitReadable(int sockfd, int timeoutms);
int	qSocketRead(char *binary, int size, int sockfd, int timeoutms);
int	qSocketGets(char *str, int size, int sockfd, int timeoutms);
int	qSocketWrite(char *binary, int size, int sockfd);
int	qSocketPuts(char *str, int sockfd);
int	qSocketPrintf(int sockfd, char *format, ...);
int	qSocketSendFile(char *filepath, int offset, int sockfd);
int	qSocketSaveIntoFile(int sockfd, int size, int timeoutms, char *filepath, char *mode);
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
Q_DBRESULT *qDbExecuteQuery(Q_DB *db, char *query);

int     qDbGetRows(Q_DBRESULT *result);
int     qDbGetCols(Q_DBRESULT *result);
int	qDbResultNext(Q_DBRESULT *result);
bool	qDbResultFree(Q_DBRESULT *result);

char	*qDbGetValue(Q_DBRESULT *result, char *field);
int	qDbGetInt(Q_DBRESULT *result, char *field);
char	*qDbGetValueAt(Q_DBRESULT *result, int idx);
int	qDbGetIntAt(Q_DBRESULT *result, int idx);

bool	qDbBeginTran(Q_DB *db);
bool	qDbEndTran(Q_DB *db, bool commit);
bool	qDbCommit(Q_DB *db);
bool	qDbRollback(Q_DB *db);

/*
 * qShm.c
 */
int	qShmInit(char *keyfile, size_t size, bool autodestroy);
int	qShmGetId(char *keyfile);
void	*qShmGet(int shmid);
bool	qShmFree(int shmid);

/*
 * qSem.c
 */
int	qSemInit(char *keyfile, int nsems, bool autodestroy);
int	qSemGetId(char *keyfile);
bool	qSemEnter(int semid, int semno);
bool	qSemEnterNowait(int semid, int semno);
bool	qSemLeave(int semid, int semno);
bool	qSemFree(int semid);

/*
 * qEntry.c
 */

Q_ENTRY	*qEntryAdd(Q_ENTRY *first, char *name, char *value, int flag);
Q_ENTRY	*qEntryRemove(Q_ENTRY *first, char *name);
char	*qEntryValue(Q_ENTRY *first, char *name);
char	*qEntryValueLast(Q_ENTRY *first, char *name);
int	qEntryiValue(Q_ENTRY *first, char *name);
int	qEntryiValueLast(Q_ENTRY *first, char *name);
int	qEntryNo(Q_ENTRY *first, char *name);
Q_ENTRY	*qEntryReverse(Q_ENTRY *first);
int	qEntryPrint(Q_ENTRY *first);
void	qEntryFree(Q_ENTRY *first);
int	qEntrySave(Q_ENTRY *first, char *filename, bool encodevalue);
Q_ENTRY	*qEntryLoad(char *filename, bool decodevalue);

#ifdef __cplusplus
}
#endif

#endif /* _DOXYGEN_SKIP */

#endif /*_QDECODER_H */
