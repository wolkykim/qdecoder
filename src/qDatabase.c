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
 * @file qDatabase.c Database Independent Wrapper API
 *
 * @note
 * To use this API, you must include database header file before including qDecoder.h in your
 * source code like below. And please remember that qDecoder must be compiled with ENABLE_MYSQL
 * or ENABLE_SOME_DATABASE option.
 *
 * @code
 *   [include order at your source codes]
 *   #include "mysql.h"
 *   #include "qDecoder.h"
 * @endcode
 *
 * Not documented yet, please refer below sample codes.
 * @code
 *   Q_DB *db = NULL;
 *   Q_DBRESULT *result = NULL;
 *
 *   db = qDbInit("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", true);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 *
 *   // try to connect
 *   if (qDbOpen(db) == false) {
 *     printf("WARNING: Can't connect to database.\n");
 *     return -1;
 *   }
 *
 *   // get results
 *   result = qDbExecuteQuery(db, "SELECT name, population FROM City");
 *   if (result != NULL) {
 *     printf("COLS : %d , ROWS : %d\n", qDbGetCols(result), qDbGetRows(result));
 *     while (qDbResultNext(result) == true) {
 *       char *pszName = qDbGetValue(result, "name");
 *       int   nPopulation = qDbGetInt(result, "population");
 *       printf("Country : %s , Population : %d\n", pszName, nPopulation);
 *     }
 *     qDbResultFree(result);
 *   }
 *
 *   // close connection
 *   qDbClose(db);
 *
 *   // free db object
 *   qDbFree(db);
 * @endcode
 */

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
 * @return	a pointer of Q_DB structure in case of successful, otherwise returns NULL.
 *
 * @code
 *   Q_DB *db = NULL;
 *   db = qDbInit("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", true);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 * @endcode
 */
Q_DB *qDbInit(const char *dbtype, const char *addr, int port, const char *username, const char *password, const char *database, bool autocommit) {
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

	return db;
}

/**
 * Connect to database server
 *
 * @param db		a pointer of Q_DB which is returned by qDbInit()
 *
 * @return	true if successful, otherwise returns false.
 */
bool qDbOpen(Q_DB *db) {
	if (db == NULL) return false;

	// if connected, close first
	if (db->connected == true) {
		qDbClose(db);
	}

#ifdef _Q_ENABLE_MYSQL
	// initialize handler
	if(db->mysql != NULL) qDbClose(db);

	db->mysql = mysql_init(NULL);
	if(db->mysql == NULL) return false;

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
		qDbClose(db); // free mysql handler
		return false;
	}

	// set auto-commit
	if (mysql_autocommit(db->mysql, db->info.autocommit) != 0) {
		qDbClose(db); // free mysql handler
		return false;
	}

	// set flag
	db->connected = true;

	return true;
#else
	return false;
#endif
}

/**
 * Disconnect from database server
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if successful, otherwise returns false.
 *
 * @note
 * Before you call qDbFree(), Q_DB structure still contains database information.
 * So you can re-connect to database using qDbOpen() without qDbInit().
 */
bool qDbClose(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	if(db->mysql != NULL) {
		mysql_close(db->mysql);
		db->mysql = NULL;
	}
	db->connected = false;
	return true;
#else
	return false;
#endif
}

/**
 * De-allocate Q_DB structure
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if successful, otherwise returns false.
 */
bool qDbFree(Q_DB *db)  {
	if (db == NULL) return false;
	qDbClose(db);

	free(db->info.dbtype);
	free(db->info.addr);
	free(db->info.username);
	free(db->info.password);
	free(db->info.database);
	free(db);

	return true;
}

/**
 * Get error number and message
 *
 * @param db		a pointer of Q_DB structure
 * @param errorno	if not NULL, error number will be stored
 *
 * @return	a pointer of error message string
 *
 * @note
 * Do not free returned error message
 */
const char *qDbGetError(Q_DB *db, unsigned int *errorno) {
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
 * Checks whether the connection to the server is working.
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if connection is alive, false if connection is not available and failed to reconnect
 *
 * @note
 * If the connection has gone down, an attempt to reconnect.
 */
bool qDbPing(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	if (db->connected == true && mysql_ping(db->mysql) == 0) {
		return true;
	} else { // ping test failed
		if (db->connected == true) {
			qDbClose(db); // now db->connected == false;
		}

		if (qDbOpen(db) == true) { // try re-connect
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
 * Get last connection status
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if the connection flag is set to alive, otherwise returns false
 *
 * @note
 * This function just returns the the connection status flag.
 */
bool qDbGetLastConnStatus(Q_DB *db) {
	if (db == NULL) return false;

	return db->connected;
}

/**
 * Set result fetching type
 *
 * @param db		a pointer of Q_DB structure
 * @param use		false for storing the results to client (default mode), true for fetching directly from server,
 *
 * @return	true if successful otherwise returns false
 *
 * @note
 * If qDbSetFetchType(db, true) is called, the results does not actually read into the client.
 * Instead, each row must be retrieved individually by making calls to qDbResultNext().
 * This reads the result of a query directly from the server without storing it in local buffer,
 * which is somewhat faster and uses much less memory than default behavior qDbSetFetchType(db, false).
 */
bool qDbSetFetchType(Q_DB *db, bool use) { // false : store, true : each row must be retrieved from the database
	if (db == NULL) return false;
	db->info.fetchtype = use;
	return true;
}

/////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS - query
/////////////////////////////////////////////////////////////////////////

/**
 * Executes the update DML
 *
 * @param db		a pointer of Q_DB structure
 * @param query		query string
 *
 * @return	a number of affected rows
 */
int qDbExecuteUpdate(Q_DB *db, const char *query) {
	if (db == NULL || db->connected == false) return -1;

#ifdef _Q_ENABLE_MYSQL
	int affected = -1;

	// query
	DEBUG("%s", query);
	if (mysql_query(db->mysql, query)) return -1;

	/* get affected rows */
	if ((affected = mysql_affected_rows(db->mysql)) < 0) return -1;

	return affected;
#else
	return -1;
#endif
}

/**
 * Executes the formatted update DML
 *
 * @param db		a pointer of Q_DB structure
 * @param format	query string format
 *
 * @return	a number of affected rows
 */
int qDbExecuteUpdatef(Q_DB *db, const char *format, ...) {
	char query[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(query, sizeof(query), format, arglist);
	va_end(arglist);

	return qDbExecuteUpdate(db, query);
}

/**
 * Executes the query
 *
 * @param db		a pointer of Q_DB structure
 * @param query		query string
 *
 * @return	a pointer of Q_DBRESULT
 */
Q_DBRESULT *qDbExecuteQuery(Q_DB *db, const char *query) {
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

	return result;
#else
	return NULL;
#endif
}

/**
 * Executes the formatted query
 *
 * @param db		a pointer of Q_DB structure
 * @param format	query string format
 *
 * @return	a pointer of Q_DBRESULT
 */
Q_DBRESULT *qDbExecuteQueryf(Q_DB *db, const char *format, ...) {
	char query[1024];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(query, sizeof(query), format, arglist);
	va_end(arglist);

	return qDbExecuteQuery(db, query);
}

/**
 * Retrieves the next row of a result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	true if successful, false if no more rows are left
 */
bool qDbResultNext(Q_DBRESULT *result) {
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
 * De-allocate the result
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	true if successful, otherwise returns false
 */
bool qDbResultFree(Q_DBRESULT *result) {
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

/**
 * Get the number of columns in the result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	the number of columns in the result set
 */
int qDbGetCols(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cols;
#else
	return 0;
#endif
}

/**
 * Get the number of rows in the result set
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	the number of rows in the result set
 */
int qDbGetRows(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return mysql_num_rows(result->rs);
#else
	return 0;
#endif
}

/**
 * Get the current row number
 *
 * @param result	a pointer of Q_DBRESULT
 *
 * @return	current fetching row number of the result set
 *
 * @note
 * This number is sequencial counter which is started from 1.
 */
int qDbGetRow(Q_DBRESULT *result) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cursor;
#else
	return 0;
#endif
}

/**
 * Get the result as string by field name
 *
 * @param result	a pointer of Q_DBRESULT
 * @param field		column name
 *
 * @return	a string pointer if successful, otherwise returns NULL.
 *
 * @note
 * Do not free returned string.
 */
const char *qDbGetStr(Q_DBRESULT *result, const char *field) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL || result->cols <= 0) return NULL;

	if (result->fields == NULL) result->fields = mysql_fetch_fields(result->rs);

	int i;
	for (i = 0; i < result->cols; i++) {
		if (!strcasecmp(result->fields[i].name, field)) return qDbGetStrAt(result, i + 1);
	}

	return NULL;
#else
	return NULL;
#endif
}

/**
 * Get the result as string by column number
 *
 * @param result	a pointer of Q_DBRESULT
 * @param idx		column number (first column is 1)
 *
 * @return	a string pointer if successful, otherwise returns NULL.
 */
const char *qDbGetStrAt(Q_DBRESULT *result, int idx) {
#ifdef _Q_ENABLE_MYSQL
	if (result == NULL || result->rs == NULL || result->cursor == 0 || idx <= 0 || idx > result->cols ) return NULL;
	return result->row[idx-1];
#else
	return NULL;
#endif
}

/**
 * Get the result as integer by field name
 *
 * @param result	a pointer of Q_DBRESULT
 * @param field		column name
 *
 * @return	a integer converted value
 */
int qDbGetInt(Q_DBRESULT *result, const char *field) {
	const char *val = qDbGetStr(result, field);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Get the result as integer by column number
 *
 * @param result	a pointer of Q_DBRESULT
 * @param idx		column number (first column is 1)
 *
 * @return	a integer converted value
 */
int qDbGetIntAt(Q_DBRESULT *result, int idx) {
	const char *val = qDbGetStrAt(result, idx);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Start transaction
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if successful, otherwise returns false
 */
bool qDbBeginTran(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	Q_DBRESULT *result;
	result = qDbExecuteQuery(db, "START TRANSACTION");
	if(result == NULL) return false;
	qDbResultFree(result);
	return true;
#else
	return false;
#endif
}

/**
 * End transaction
 *
 * @param db		a pointer of Q_DB structure
 * @param commit	true for commit, false for roll-back
 *
 * @return	true if successful, otherwise returns false
 */
bool qDbEndTran(Q_DB *db, bool commit) {
	if (db == NULL) return false;

	if (commit == false) return qDbRollback(db);
	return qDbCommit(db);
}

/**
 * Commit transaction
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if successful, otherwise returns false
 */
bool qDbCommit(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	if (mysql_commit(db->mysql) != 0) return false;
	return true;
#else
	return false;
#endif
}

/**
 * Roll-back transaction
 *
 * @param db		a pointer of Q_DB structure
 *
 * @return	true if successful, otherwise returns false
 */
bool qDbRollback(Q_DB *db) {
	if (db == NULL) return false;

#ifdef _Q_ENABLE_MYSQL
	if (mysql_rollback(db->mysql) != 0) {
		return false;
	}
	return true;
#else
	return 0;
#endif
}
