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
 * @file qDatabase.c Database Independent Wrapper API
 *
 * @note
 * To use this API, you must include database header file before including qDecoder.h in your
 * source code like below. And please remember that qDecoder must be compiled with ENABLE_MYSQL
 * or ENABLE_SOME_DATABASE option.
 *
 * @code
 *   [mysql.h header should be included prior to qDecoder.h header to let qDecoder know that.]
 *   #include "mysql.h"
 *   #include "qDecoder.h"
 * @endcode
 *
 * @code
 *   Q_DB *db = NULL;
 *   Q_DBRESULT *result = NULL;
 *
 *   db = qDb("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", true);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 *
 *   // try to connect
 *   if (db->open(db) == false) {
 *     printf("WARNING: Can't connect to database.\n");
 *     return -1;
 *   }
 *
 *   // get results
 *   result = db->executeQuery(db, "SELECT name, population FROM City");
 *   if (result != NULL) {
 *     printf("COLS : %d , ROWS : %d\n", result->getCols(result), result->getRows(result));
 *     while (result->getNext(result) == true) {
 *       char *pszName = result->getStr(result, "name");
 *       int   nPopulation = result->getInt(result, "population");
 *       printf("Country : %s , Population : %d\n", pszName, nPopulation);
 *     }
 *     result->free(result);
 *   }
 *
 *   // close connection
 *   db->close(db);
 *
 *   // free db object
 *   db->free(db);
 * @endcode
 *
 * @note
 * Use "--enable-threadsafe" configure script option to use under multi-threaded environments.
 */

#if defined(ENABLE_MYSQL) || defined( _DOXYGEN_SKIP)

#ifdef ENABLE_MYSQL
#include "mysql.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "qDecoder.h"
#include "qInternal.h"

/*
 * Member method protos
 */
#ifndef _DOXYGEN_SKIP
// Q_DB object
static bool		_open(Q_DB *db);
static bool		_close(Q_DB *db);

static int		_executeUpdate(Q_DB *db, const char *query);
static int		_executeUpdatef(Q_DB *db, const char *format, ...);
static Q_DBRESULT*	_executeQuery(Q_DB *db, const char *query);
static Q_DBRESULT*	_executeQueryf(Q_DB *db, const char *format, ...);

static bool		_beginTran(Q_DB *db);
static bool		_commit(Q_DB *db);
static bool		_rollback(Q_DB *db);

static bool		_setFetchType(Q_DB *db, bool use);
static bool		_getConnStatus(Q_DB *db);
static bool		_ping(Q_DB *db);
static const char*	_getError(Q_DB *db, unsigned int *errorno);
static bool		_free(Q_DB *db);

// Q_DBRESULT object
static const char*	_resultGetStr(Q_DBRESULT *result, const char *field);
static const char*	_resultGetStrAt(Q_DBRESULT *result, int idx);
static int		_resultGetInt(Q_DBRESULT *result, const char *field);
static int		_resultGetIntAt(Q_DBRESULT *result, int idx);
static bool		_resultGetNext(Q_DBRESULT *result);

static int		_resultGetCols(Q_DBRESULT *result);
static int		_resultGetRows(Q_DBRESULT *result);
static int		_resultGetRow(Q_DBRESULT *result);

static bool		_resultFree(Q_DBRESULT *result);

#endif

/**
 * Initialize internal connector structure
 *
 * @param dbtype	database server type. currently "MYSQL" is only supported
 * @param addr		ip or fqdn address.
 * @param port		port number
 * @param username	database username
 * @param password	database password
 * @param database	database server type. currently "MYSQL" is only supported
 * @param autocommit	sets autocommit mode on if autocommit is true, off if autocommit is false
 *
 * @return	a pointer of Q_DB object in case of successful, otherwise returns NULL.
 *
 * @code
 *   Q_DB *db = qDb("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", true);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 * @endcode
 */
Q_DB *qDb(const char *dbtype, const char *addr, int port, const char *username, const char *password, const char *database, bool autocommit) {
	// check db type
#ifdef _Q_ENABLE_MYSQL
	if (strcmp(dbtype, "MYSQL")) return NULL;
#else
	return NULL;
#endif
	if(dbtype == NULL || addr == NULL || username == NULL || password == NULL || database == NULL) return NULL;

	// initialize
	Q_DB *db;
	if ((db = (Q_DB *)malloc(sizeof(Q_DB))) == NULL) return NULL;
	memset((void *)db, 0, sizeof(Q_DB));
	db->connected = false;

	// set common structure
	db->info.dbtype = strdup(dbtype);
	db->info.addr = strdup(addr);
	db->info.port = port;
	db->info.username = strdup(username);
	db->info.password = strdup(password);
	db->info.database = strdup(database);
	db->info.autocommit = autocommit;
	db->info.fetchtype = false; // store mode

	// set db handler
#ifdef _Q_ENABLE_MYSQL
	db->mysql = NULL;
#endif

	// assign methods
	db->open		= _open;
	db->close		= _close;

	db->executeUpdate	= _executeUpdate;
	db->executeUpdatef	= _executeUpdatef;
	db->executeQuery	= _executeQuery;
	db->executeQueryf	= _executeQueryf;

	db->beginTran		= _beginTran;
	db->commit		= _commit;
	db->rollback		= _rollback;

	db->setFetchType	= _setFetchType;
	db->getConnStatus	= _getConnStatus;
	db->ping		= _ping;
	db->getError		= _getError;
	db->free		= _free;

	// initialize recrusive mutex
	Q_MUTEX_INIT(db->qmutex, true);

	return db;
}

/**
 * Q_DB->open(): Connect to database server
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false.
 */
static bool _open(Q_DB *db) {
	if (db == NULL) return false;

	// if connected, close first
	if (db->connected == true) {
		_close(db);
	}

#ifdef _Q_ENABLE_MYSQL
	Q_MUTEX_ENTER(db->qmutex);

	// initialize handler
	if(db->mysql != NULL) _close(db);

	if(mysql_library_init(0, NULL, NULL) != 0) {
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}

	if((db->mysql = mysql_init(NULL)) == NULL) {
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}

	// set options
	my_bool reconnect = _Q_MYSQL_OPT_RECONNECT;
	unsigned int connect_timeout = _Q_MYSQL_OPT_CONNECT_TIMEOUT;
	unsigned int read_timeout = _Q_MYSQL_OPT_READ_TIMEOUT;
	unsigned int write_timeout = _Q_MYSQL_OPT_WRITE_TIMEOUT;

	if(reconnect != false) mysql_options(db->mysql, MYSQL_OPT_RECONNECT, (char *)&reconnect);
	if(connect_timeout > 0) mysql_options(db->mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&connect_timeout);
	if(read_timeout > 0) mysql_options(db->mysql, MYSQL_OPT_READ_TIMEOUT, (char *)&read_timeout);
	if(write_timeout > 0) mysql_options(db->mysql, MYSQL_OPT_WRITE_TIMEOUT, (char *)&write_timeout);

	// try to connect
	if(mysql_real_connect(db->mysql, db->info.addr, db->info.username, db->info.password, db->info.database, db->info.port, NULL, 0) == NULL) {
		_close(db); // free mysql handler
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}

	// set auto-commit
	if (mysql_autocommit(db->mysql, db->info.autocommit) != 0) {
		_close(db); // free mysql handler
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}

	// set flag
	db->connected = true;
	Q_MUTEX_LEAVE(db->qmutex);
	return true;
#else
	return false;
#endif
}

/**
 * Q_DB->close(): Disconnect from database server
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false.
 *
 * @note
 * Unless you call Q_DB->free(), Q_DB object will keep the database information.
 * So you can re-connect to database using Q_DB->open().
 */
static bool _close(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	Q_MUTEX_ENTER(db->qmutex);

	if(db->mysql != NULL) {
		mysql_close(db->mysql);
		db->mysql = NULL;
		mysql_library_end();
	}
	db->connected = false;

	Q_MUTEX_LEAVE(db->qmutex);
	return true;
#else
	return false;
#endif
}

/**
 * Q_DB->executeUpdate(): Executes the update DML
 *
 * @param db		a pointer of Q_DB object
 * @param query		query string
 *
 * @return	a number of affected rows
 */
static int _executeUpdate(Q_DB *db, const char *query) {
	if (db == NULL || db->connected == false) return -1;

#ifdef _Q_ENABLE_MYSQL
	Q_MUTEX_ENTER(db->qmutex);

	int affected = -1;

	// query
	DEBUG("%s", query);
	if (mysql_query(db->mysql, query) == 0) {
		/* get affected rows */
		if ((affected = mysql_affected_rows(db->mysql)) < 0) affected = -1;
	}

	Q_MUTEX_LEAVE(db->qmutex);
	return affected;
#else
	return -1;
#endif
}

/**
 * Q_DB->executeUpdatef(): Executes the formatted update DML
 *
 * @param db		a pointer of Q_DB object
 * @param format	query string format
 *
 * @return	a number of affected rows, otherwise returns -1
 */
static int _executeUpdatef(Q_DB *db, const char *format, ...) {
	char *query;
	DYNAMIC_VSPRINTF(query, format);
	if(query == NULL) return -1;

	int affected = _executeUpdate(db, query);
	free(query);

	return affected;
}

/**
 * Q_DB->executeQuery(): Executes the query
 *
 * @param db		a pointer of Q_DB object
 * @param query		query string
 *
 * @return	a pointer of Q_DBRESULT if successful, otherwise returns NULL
 */
static Q_DBRESULT *_executeQuery(Q_DB *db, const char *query) {
	if (db == NULL || db->connected == false) return NULL;

#ifdef _Q_ENABLE_MYSQL
	// query
	DEBUG("%s", query);
	if (mysql_query(db->mysql, query)) return NULL;

	// store
	Q_DBRESULT *result = (Q_DBRESULT *)malloc(sizeof(Q_DBRESULT));
	if (result == NULL) return NULL;

	result->fetchtype = db->info.fetchtype;
	if(result->fetchtype == false) {
		result->rs = mysql_store_result(db->mysql);
	} else {
		result->rs = mysql_use_result(db->mysql);
	}
	if (result->rs == NULL) {
		free(result);
		return NULL;
	}

	/* get meta data */
	result->fields = NULL;
	result->row = NULL;
	result->cols = mysql_num_fields(result->rs);
	result->cursor = 0;

	/* assign methods */
	result->getStr		= _resultGetStr;
	result->getStrAt	= _resultGetStrAt;
	result->getInt		= _resultGetInt;
	result->getIntAt	= _resultGetIntAt;
	result->getNext		= _resultGetNext;

	result->getCols		= _resultGetCols;
	result->getRows		= _resultGetRows;
	result->getRow		= _resultGetRow;

	result->free		= _resultFree;

	return result;
#else
	return NULL;
#endif
}

/**
 * Q_DB->executeQueryf(): Executes the formatted query
 *
 * @param db		a pointer of Q_DB object
 * @param format	query string format
 *
 * @return	a pointer of Q_DBRESULT if successful, otherwise returns NULL
 */
static Q_DBRESULT *_executeQueryf(Q_DB *db, const char *format, ...) {
	char *query;
	DYNAMIC_VSPRINTF(query, format);
	if(query == NULL) return NULL;

	Q_DBRESULT *ret = db->executeQuery(db, query);
	free(query);
	return ret;
}

/**
 * Q_DB->beginTran(): Start transaction
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false
 *
 * @code
 *   db->beginTran(db);
 *   (... insert/update/delete ...)
 *   db->commit(db);
 * @endcode
 *
 * @note
 * This operation will raise lock if you compile "--enable-threadsafe" option
 * to protect thread-safe operation. In this case, before calling Q_DB->commit()
 * or Q_DB->rollback(), another threads will be hold.
 */
static bool _beginTran(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	Q_MUTEX_ENTER(db->qmutex);
	if(db->qmutex.count != 1) {
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}

	Q_DBRESULT *result;
	result = db->executeQuery(db, "START TRANSACTION");
	if(result == NULL) {
		Q_MUTEX_LEAVE(db->qmutex);
		return false;
	}
	result->free(result);
	return true;
#else
	return false;
#endif
}

/**
 * Q_DB->commit(): Commit transaction
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false
 */
static bool _commit(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	bool ret = false;
	if (mysql_commit(db->mysql) == 0) {
		ret = true;
	}

	if(db->qmutex.count > 0) {
		Q_MUTEX_LEAVE(db->qmutex);
	}
	return ret;
#else
	return false;
#endif
}

/**
 * Q_DB->rellback(): Roll-back and abort transaction
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false
 */
static bool _rollback(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	bool ret = false;
	if (mysql_rollback(db->mysql) == 0) {
		ret = true;
	}

	if(db->qmutex.count > 0) {
		Q_MUTEX_LEAVE(db->qmutex);
	}
	return ret;
#else
	return 0;
#endif
}

/**
 * Q_DB->setFetchType(): Set result fetching type
 *
 * @param db		a pointer of Q_DB object
 * @param use		false for storing the results to client (default mode), true for fetching directly from server,
 *
 * @return	true if successful otherwise returns false
 *
 * @note
 * If Q_DB->setFetchType(db, true) is called, the results does not actually read into the client.
 * Instead, each row must be retrieved individually by making calls to Q_DBRESULT->getNext().
 * This reads the result of a query directly from the server without storing it in local buffer,
 * which is somewhat faster and uses much less memory than default behavior Q_DB->setFetchType(db, false).
 */
static bool _setFetchType(Q_DB *db, bool use) { // false : store, true : each row must be retrieved from the database
	if (db == NULL) return false;
	db->info.fetchtype = use;
	return true;
}

/**
 * Q_DB->getConnStatus(): Get last connection status
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if the connection flag is set to alive, otherwise returns false
 *
 * @note
 * This function just returns the the connection status flag.
 */
static bool _getConnStatus(Q_DB *db) {
	if (db == NULL) return false;

	return db->connected;
}

/**
 * Q_DB->ping(): Checks whether the connection to the server is working.
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if connection is alive, false if connection is not available and failed to reconnect
 *
 * @note
 * If the connection has gone down, an attempt to reconnect.
 */
static bool _ping(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	if (db->connected == true && mysql_ping(db->mysql) == 0) {
		return true;
	} else { // ping test failed
		if (_open(db) == true) { // try re-connect
			DEBUG("Connection recovered.");
			return true;
		}
	}

	return false;
#else
	return false;
#endif
}

/**
 * Q_DB->getError(): Get error number and message
 *
 * @param db		a pointer of Q_DB object
 * @param errorno	if not NULL, error number will be stored
 *
 * @return	a pointer of error message string
 *
 * @note
 * Do not free returned error message
 */
static const char *_getError(Q_DB *db, unsigned int *errorno) {
	if (db == NULL || db->connected == false) return "(no opened db)";

	unsigned int eno = 0;
	const char *emsg;
#ifdef _Q_ENABLE_MYSQL
	eno = mysql_errno(db->mysql);
	if(eno == 0) emsg = "(no error)";
	else emsg = mysql_error(db->mysql);
#else
	emsg = "(not implemented)";
#endif

	if(errorno != NULL) *errorno = eno;
	return emsg;
}

/**
 * Q_DB->free(): De-allocate Q_DB structure
 *
 * @param db		a pointer of Q_DB object
 *
 * @return	true if successful, otherwise returns false.
 */
static bool _free(Q_DB *db)  {
	if (db == NULL) return false;

	Q_MUTEX_ENTER(db->qmutex);

	_close(db);

	free(db->info.dbtype);
	free(db->info.addr);
	free(db->info.username);
	free(db->info.password);
	free(db->info.database);
	free(db);

	Q_MUTEX_LEAVE(db->qmutex);
	Q_MUTEX_DESTROY(db->qmutex);

	return true;
}

/**
 * Q_DBRESULT->getStr(): Get the result as string by field name
 *
 * @param result	a pointer of Q_DBRESULT
 * @param field		column name
 *
 * @return	a string pointer if successful, otherwise returns NULL.
 *
 * @note
 * Do not free returned string.
 */
static const char *_resultGetStr(Q_DBRESULT *result, const char *field) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL || result->cols <= 0) return NULL;

	if (result->fields == NULL) result->fields = mysql_fetch_fields(result->rs);

	int i;
	for (i = 0; i < result->cols; i++) {
		if (!strcasecmp(result->fields[i].name, field)) return result->getStrAt(result, i + 1);
	}

	return NULL;
#else
	return NULL;
#endif
}

/**
 * Q_DBRESULT->getStrAt(): Get the result as string by column number
 *
 * @param result	a pointer of Q_DBRESULT
 * @param idx		column number (first column is 1)
 *
 * @return	a string pointer if successful, otherwise returns NULL.
 */
static const char *_resultGetStrAt(Q_DBRESULT *result, int idx) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL || result->cursor == 0 || idx <= 0 || idx > result->cols ) return NULL;
	return result->row[idx-1];
#else
	return NULL;
#endif
}

/**
 * Q_DBRESULT->getInt(): Get the result as integer by field name
 *
 * @param result	a pointer of Q_DBRESULT
 * @param field		column name
 *
 * @return	a integer converted value
 */
static int _resultGetInt(Q_DBRESULT *result, const char *field) {
	const char *val = result->getStr(result, field);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Q_DBRESULT->getIntAt(): Get the result as integer by column number
 *
 * @param result	a pointer of Q_DBRESULT
 * @param idx		column number (first column is 1)
 *
 * @return	a integer converted value
 */
static int _resultGetIntAt(Q_DBRESULT *result, int idx) {
	const char *val = result->getStrAt(result, idx);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Q_DBRESULT->getNext(): Retrieves the next row of a result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	true if successful, false if no more rows are left
 */
static bool _resultGetNext(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return false;

	if ((result->row = mysql_fetch_row(result->rs)) == NULL) return false;
	result->cursor++;

	return true;
#else
	return false;
#endif
}

/**
 * Q_DBRESULT->getCols(): Get the number of columns in the result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	the number of columns in the result set
 */
static int _resultGetCols(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cols;
#else
	return 0;
#endif
}

/**
 * Q_DBRESULT->getRows(): Get the number of rows in the result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	the number of rows in the result set
 */
static int _resultGetRows(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return mysql_num_rows(result->rs);
#else
	return 0;
#endif
}

/**
 * Q_DBRESULT->getRow(): Get the current row number
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	current fetching row number of the result set
 *
 * @note
 * This number is sequencial counter which is started from 1.
 */
static int _resultGetRow(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cursor;
#else
	return 0;
#endif
}

/**
 * Q_DBRESULT->free(): De-allocate the result
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	true if successful, otherwise returns false
 */
static bool _resultFree(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL) return false;
	if (result->rs != NULL) {
		if(result->fetchtype == true) {
			while(mysql_fetch_row(result->rs) != NULL);
		}
		mysql_free_result(result->rs);
		result->rs = NULL;
	}
	free(result);
	return true;
#else
	return false;
#endif
}

#endif
