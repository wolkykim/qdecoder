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
 * Under-development
 *
 * @since not released yet
 *
 * @note
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
 */
bool qDbGetLastConnStatus(Q_DB *db) {
	if (db == NULL) return false;

	return db->connected;
}

/**
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Retrieves the current row number
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
 */
int qDbGetInt(Q_DBRESULT *result, const char *field) {
	const char *val = qDbGetStr(result, field);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Under-development
 *
 * @since not released yet
 */
int qDbGetIntAt(Q_DBRESULT *result, int idx) {
	const char *val = qDbGetStrAt(result, idx);
	if(val == NULL) return 0;
	return atoi(val);
}

/**
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
 */
bool qDbEndTran(Q_DB *db, bool commit) {
	if (db == NULL) return false;

	if (commit == false) return qDbRollback(db);
	return qDbCommit(db);
}

/**
 * Under-development
 *
 * @since not released yet
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
 * Under-development
 *
 * @since not released yet
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
