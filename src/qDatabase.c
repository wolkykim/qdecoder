/***************************************************************************
 * qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org
 *
 * Copyright (C) 2007 Seung-young Kim.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 **************************************************************************/

/**
 * @file qDatabase.c Database Independent Wrapper Functions
 *
 * @note
 * To use this API, you must include database header file before including qDecoder.h in your
 * source code like below. And please remember that qDecoder must be compiled with WITH_MYSQL
 * or WITH_SOME_DATABASE option.
 *
 * @code
 *   In your source)
 *   #include "mysql.h"
 *   #include "qDecoder.h"
 * @endcode
 *
 * Not documented yet, please refer below sample codes.
 * @code
 *   Q_DB *db = NULL;
 *   Q_DBRESULT *result = NULL;
 *
 *   db = qDbInit("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", TRUE);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 *
 *   // try to connect
 *   if (qDbOpen(db) == FALSE) {
 *     printf("WARNING: Can't connect to database.\n");
 *     return -1;
 *   }
 *
 *   // get results
 *   result = qDbExecuteQuery(db, "SELECT name, population FROM City");
 *   if (result != NULL) {
 *     printf("COLS : %d , ROWS : %d\n", qDbGetCols(result), qDbGetRows(result));
 *     while (qDbResultNext(result) > 0) {
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

#ifdef WITH_MYSQL
#include "mysql.h"
#endif

#include "qDecoder.h"
#include "qInternal.h"

/**
 * Under-development
 *
 * @since 8.1R
 *
 * @note
 * @code
 *   Q_DB *db = NULL;
 *   db = qDbInit("MYSQL", "dbhost.qdecoder.org", 3306, "test", "secret", "sampledb", TRUE);
 *   if (db == NULL) {
 *     printf("ERROR: Not supported database type.\n");
 *     return -1;
 *   }
 * @endcode
 */
Q_DB *qDbInit(char *dbtype, char *addr, int port, char *username, char *password, char *database, Q_BOOL autocommit) {
	Q_DB *db;
	char *support_db;

#ifdef _Q_WITH_MYSQL
	support_db = "MYSQL";
#else
	support_db = "";
#endif

	// check db type
	if (strcmp(dbtype, support_db)) return NULL;

	if ((db = (Q_DB *)malloc(sizeof(Q_DB))) == NULL) return NULL;
	memset((void *)db, 0, sizeof(Q_DB));
	db->connected = Q_FALSE;

	// set info
	strcpy(db->info.dbtype, dbtype);
	strcpy(db->info.addr, addr);
	db->info.port = port;
	strcpy(db->info.username, username);
	strcpy(db->info.password, password);
	strcpy(db->info.database, database);
	db->info.autocommit = autocommit;

	return db;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbOpen(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

	// if connected, close first
	if (db->connected == Q_TRUE) {
		qDbClose(db);
	}

#ifdef _Q_WITH_MYSQL
	// initialize mysql structure
	if (!mysql_init(&db->mysql)) {
		return Q_FALSE;
	}

	// set options
	//my_bool reconnect = 1;
	//mysql_options(&db->mysql, MYSQL_OPT_RECONNECT, &reconnect);

	// try to connect
	if (!mysql_real_connect(&db->mysql, db->info.addr, db->info.username, db->info.password, db->info.database, db->info.port, NULL, 0)) {
		return Q_FALSE;
	}

	// set auto-commit
	if (mysql_autocommit(&db->mysql, db->info.autocommit) != 0) {
		qDbClose(db);
		return Q_FALSE;
	}

	// set flag
	db->connected = Q_TRUE;

	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbClose(Q_DB *db) {
	if (db == NULL || db->connected == Q_FALSE) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	mysql_close(&db->mysql);
	db->connected = Q_FALSE;
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbFree(Q_DB *db)  {
	if (db == NULL) return Q_FALSE;
	qDbClose(db);
	free(db);
	return Q_TRUE;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
char *qDbGetErrMsg(Q_DB *db) {
	static char msg[1024];
	if (db == NULL || db->connected == Q_FALSE) return "(no opened db)";

#ifdef _Q_WITH_MYSQL
	strcpy(msg, mysql_error(&db->mysql));
#else
	strcpy(msg, "");
#endif

	return msg;
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbPing(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	if (db->connected == Q_TRUE && mysql_ping(&db->mysql) == 0) {
		return Q_TRUE;
	} else { // ping test failed
		if (db->connected == Q_TRUE) {
			qDbClose(db); // now db->connected == Q_FALSE;
		}

		if (qDbOpen(db) == Q_TRUE) { // try re-connect
			return Q_TRUE;
		}
	}

	return Q_FALSE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbGetLastConnStatus(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

	return db->connected;
}

/////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS - query
/////////////////////////////////////////////////////////////////////////

/**
 * Under-development
 *
 * @since 8.1R
 */
int qDbExecuteUpdate(Q_DB *db, char *pszQuery) {
	if (db == NULL || db->connected == Q_FALSE) return -1;

#ifdef _Q_WITH_MYSQL
	int affected;

	// query
	if (mysql_query(&db->mysql, pszQuery)) {
		qDbPing(db);
		return -1;
	}

	/* get affected rows */
	if ((affected = mysql_affected_rows(&db->mysql)) < 0) return -1;

	return affected;
#else
	return -1;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_DBRESULT *qDbExecuteQuery(Q_DB *db, char *pszQuery) {
	if (db == NULL || db->connected == Q_FALSE) return NULL;

#ifdef _Q_WITH_MYSQL
	// query
	if (mysql_query(&db->mysql, pszQuery)) {
		qDbPing(db);
		return NULL;
	}

	// store
	Q_DBRESULT *result = (Q_DBRESULT *)malloc(sizeof(Q_DBRESULT));
	if (result == NULL) return NULL;

	if ((result->rs = mysql_store_result(&db->mysql)) == NULL) {
		free(result);
		qDbPing(db);
		return NULL;
	}

	/* get meta data */
	result->fields = NULL;
	result->row = NULL;
	result->rows = mysql_num_rows(result->rs);
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
 * @since 8.1R
 */
int qDbGetRows(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->rows;
#else
	return 0;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
int qDbGetCols(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cols;
#else
	return 0;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
int qDbResultNext(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL) return 0;

	if ((result->row = mysql_fetch_row(result->rs)) == NULL) return 0;
	result->cursor++;

	return result->cursor;
#else
	return 0;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbResultFree(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL) return Q_FALSE;
	if (result->rs != NULL) {
		mysql_free_result(result->rs);
		result->rs = NULL;
	}
	free(result);
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
char *qDbGetValue(Q_DBRESULT *result, char *field) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL || result->cols <= 0) return NULL;

	if (result->fields == NULL) result->fields = mysql_fetch_fields(result->rs);

	int i;
	for (i = 0; i < result->cols; i++) {
		if (!strcasecmp(result->fields[i].name, field)) return qDbGetValueAt(result, i + 1);
	}

	return NULL;
#else
	return NULL;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
int qDbGetInt(Q_DBRESULT *result, char *field) {
	return atoi(qDbGetValue(result, field));
}

/**
 * Under-development
 *
 * @since 8.1R
 */
char *qDbGetValueAt(Q_DBRESULT *result, int idx) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL || idx <= 0 || idx > result->cols ) return NULL;
	return result->row[idx-1];
#else
	return NULL;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
int qDbGetIntAt(Q_DBRESULT *result, int idx) {
	return atoi(qDbGetValueAt(result, idx));
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbBeginTran(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	qDbExecuteUpdate(db, "START TRANSACTION");
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbEndTran(Q_DB *db, Q_BOOL commit) {
	if (db == NULL) return Q_FALSE;

	if (commit == Q_FALSE) return qDbRollback(db);
	return qDbCommit(db);
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbCommit(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	if (mysql_commit(&db->mysql) != 0) return Q_FALSE;
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

/**
 * Under-development
 *
 * @since 8.1R
 */
Q_BOOL qDbRollback(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	if (mysql_rollback(&db->mysql) != 0) {
		return Q_FALSE;
	}
	return Q_TRUE;
#else
	return 0;
#endif
}
