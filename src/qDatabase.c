/************************************************************************
qDecoder - Web Application Interface for C/C++    http://www.qDecoder.org

Copyright (C) 2001 The qDecoder Project.
Copyright (C) 1999,2000 Hongik Internet, Inc.
Copyright (C) 1998 Nobreak Technologies, Inc.
Copyright (C) 1996,1997 Seung-young Kim.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


Copyright Disclaimer:
  Hongik Internet, Inc., hereby disclaims all copyright interest.
  President, Christopher Roh, 6 April 2000

  Nobreak Technologies, Inc., hereby disclaims all copyright interest.
  President, Yoon Cho, 6 April 2000

  Seung-young Kim, hereby disclaims all copyright interest.
  Author, Seung-young Kim, 6 April 2000

Author:
  Seung-young Kim <wolkykim(at)ziom.co.kr>
************************************************************************/
/**
 * Database Independent Wrapper Functions
 *
 * @file qDatabase.c
 * @note
 * To activate this feature, you must include database header file before including qDecoder.h like below.
 * And please remember that qDecoder must be compiled with WITH_MYSQL or WITH_SOME_DATABASE.
 *
 * @code
 * Example)
 * #include "mysql.h"
 * #include "qDecoder.h"
 * @endcode
 */

#ifdef WITH_MYSQL
#include "mysql.h"
#endif

#include "qDecoder.h"
#include "qInternal.h"

/**********************************************
** Usage :
** Return: Success returns Q_DB structure pointer.
**         Otherwise returns NULL.
** Do    :
**********************************************/
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

/**********************************************
** Usage :
** Return: Success Q_TRUE. Otherwise Q_FALSE.
** Do    :
**********************************************/
Q_BOOL qDbFree(Q_DB *db)  {
	if (db == NULL) return Q_FALSE;
	qDbClose(db);
	free(db);
	return Q_TRUE;
}

/**********************************************
** Usage :
** Return: Success Q_TRUE. Otherwise Q_FALSE.
** Do    :
**********************************************/
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

/**********************************************
** Usage :
** Return: Success Q_TRUE. Otherwise Q_FALSE.
** Do    :
**********************************************/
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

/**********************************************
** Usage :
** Return: error string pointer.
** Do    :
**********************************************/
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

/**********************************************
** Usage :
** Return: Success Q_TRUE. Otherwise Q_FALSE.
** Do    :
**********************************************/
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

/**********************************************
** Usage :
** Return: if connected Q_TRUE. Otherwise Q_FALSE.
** Do    :
**********************************************/
Q_BOOL qDbGetLastConnStatus(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

	return db->connected;
}

/////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS - query
/////////////////////////////////////////////////////////////////////////

/**********************************************
** Usage :
** Return: If succeed, returns affected rows, Or, returns -1.
** Do    :
**********************************************/
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

int qDbGetRows(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->rows;
#else
	return 0;
#endif
}

int qDbGetCols(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL) return 0;
	return result->cols;
#else
	return 0;
#endif
}

/**********************************************
** Usage :
** Return: If succeed, returns cursor position, Or, returns 0.
** Do    :
**********************************************/
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

int qDbGetInt(Q_DBRESULT *result, char *field) {
	return atoi(qDbGetValue(result, field));
}

char *qDbGetValueAt(Q_DBRESULT *result, int idx) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL || result->rs == NULL || idx <= 0 || idx > result->cols ) return NULL;
	return result->row[idx-1];
#else
	return NULL;
#endif
}

int qDbGetIntAt(Q_DBRESULT *result, int idx) {
	return atoi(qDbGetValueAt(result, idx));
}

/////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS - transaction
/////////////////////////////////////////////////////////////////////////

Q_BOOL qDbBeginTran(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	qDbExecuteUpdate(db, "START TRANSACTION");
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

Q_BOOL qDbEndTran(Q_DB *db, Q_BOOL commit) {
	if (db == NULL) return Q_FALSE;

	if (commit == Q_FALSE) return qDbRollback(db);
	return qDbCommit(db);
}

Q_BOOL qDbCommit(Q_DB *db) {
	if (db == NULL) return Q_FALSE;

#ifdef _Q_WITH_MYSQL
	if (mysql_commit(&db->mysql) != 0) return Q_FALSE;
	return Q_TRUE;
#else
	return Q_FALSE;
#endif
}

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
