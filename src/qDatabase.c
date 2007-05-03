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

#ifdef WITH_MYSQL
#include "mysql.h"
#endif

#include "qDecoder.h"
#include "qInternal.h"

#ifdef _Q_SUPPORT_DATABASE

/**********************************************
** Usage :
** Return: If succeed, returns 1, Or, returns 0.
** Do    :
**********************************************/
Q_DB *qDbInit(char *dbtype, char *addr, int port, char *username, char *password, char *database, int autocommit) {
	Q_DB *db;

	// check db type
	if (strcmp(dbtype, _Q_SUPPORT_DATABASE)) return NULL;

	if ((db = (Q_DB *)malloc(sizeof(Q_DB))) == NULL) return NULL;
	memset((void *)db, 0, sizeof(Q_DB));
	db->connected = 0;

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
** Return: If succeed, returns 1, Or, returns 0.
** Do    :
**********************************************/
int qDbOpen(Q_DB *db) {
	if (db == NULL) return 0;

	// if connected, close first
	if (db->connected == 1) {
		qDbClose(db);
	}
#ifdef _Q_WITH_MYSQL
	// initialize mysql structure
	if (!mysql_init(&db->mysql)) {
		return 0;
	}

	// set options
	//my_bool reconnect = 1;
	//mysql_options(&db->mysql, MYSQL_OPT_RECONNECT, &reconnect);

	// try to connect
	if (!mysql_real_connect(&db->mysql, db->info.addr, db->info.username, db->info.password, db->info.database, db->info.port, NULL, 0)) {
		return 0;
	}

	// set auto-commit
	if (mysql_autocommit(&db->mysql, db->info.autocommit) != 0) {
		qDbClose(db);
		return 0;
	}

	// set flag
	db->connected = 1;

	return 1;
#else
	return 0;
#endif
}

int qDbClose(Q_DB *db) {
	if (db == NULL || db->connected == 0) return 1;

#ifdef _Q_WITH_MYSQL
	mysql_close(&db->mysql);
	db->connected = 0;
	return 1;
#else
	return 1;
#endif
}

char *qDbGetErrMsg(Q_DB *db) {
	static char msg[1024];
	if (db == NULL || db->connected == 0) return "(no opened db)";

#ifdef _Q_WITH_MYSQL
	strcpy(msg, mysql_error(&db->mysql));
#else
	strcpy(msg, "");
#endif

	return msg;
}

int qDbPing(Q_DB *db) {
	if (db == NULL) return 0;

#ifdef _Q_WITH_MYSQL
	if (db->connected == 1 && mysql_ping(&db->mysql) == 0) {
		return 1;
	} else { // ping test failed
		if (db->connected == 1) {
			qDbClose(db); // now db->connected == 0;
		}

		if (qDbOpen(db) == 1) { // try re-connect
			return 1;
		}
	}

	return 0;
#else
	return 0;
#endif
}

int qDbGetLastConnStatus(Q_DB *db) {
	if (db == NULL) return 0;

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
	if (db == NULL || db->connected == 0) return -1;

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
	if (db == NULL || db->connected == 0) return NULL;

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

int qDbResultFree(Q_DBRESULT *result) {
#ifdef _Q_WITH_MYSQL
	if (result == NULL) return 0;
	if (result->rs != NULL) {
		mysql_free_result(result->rs);
		result->rs = NULL;
	}
	free(result);
	return 1;
#else
	return 0;
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

int qDbBeginTran(Q_DB *db) {
	if (db == NULL) return 0;

#ifdef _Q_WITH_MYSQL
	qDbExecuteUpdate(db, "START TRANSACTION");
	return 1;
#else
	return 0;
#endif
}

int qDbEndTran(Q_DB *db, int nCommit) {
	if (db == NULL) return 0;

	if (nCommit == 0) return qDbRollback(db);
	return qDbCommit(db);
}

int qDbCommit(Q_DB *db) {
	if (db == NULL) return 0;

#ifdef _Q_WITH_MYSQL
	if (mysql_commit(&db->mysql) != 0) return 0;
	return 1;
#else
	return 0;
#endif
}

int qDbRollback(Q_DB *db) {
	if (db == NULL) return 0;

#ifdef _Q_WITH_MYSQL
	if (mysql_rollback(&db->mysql) != 0) {
		return 0;
	}
	return 1;
#else
	return 0;
#endif
}

#endif
