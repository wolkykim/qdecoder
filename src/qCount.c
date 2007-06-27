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

#include "qDecoder.h"
#include "qInternal.h"


/**********************************************
** Usage : qCountRead(filename);
** Return: Success counter value, Fail 0.
** Do    : Read counter value.
**********************************************/
int qCountRead(char *filename) {
	FILE *fp;
	int  counter;

	if ((fp = qfopen(filename, "r")) == NULL) return 0;
	fscanf(fp, "%d", &counter);
	qfclose(fp);
	return counter;
}

/**********************************************
** Usage : qCountSave(filename, number);
** Return: Success Q_TRUE. Otherwise Q_FALSE.
** Do    : Save counter value.
**********************************************/
Q_BOOL qCountSave(char *filename, int number) {
	FILE *fp;

	if ((fp = qfopen(filename, "w")) == NULL) return Q_FALSE;
	fprintf(fp, "%d\n", number);
	qfclose(fp);

	return Q_TRUE;
}

/**********************************************
** Usage : qCountUpdate(filename, number);
** Return: Success current value + number, Fail 0.
** Do    : Update counter value.
**********************************************/
int qCountUpdate(char *filename, int number) {
	FILE *fp;
	int counter = 0;

	if ((fp = qfopen(filename, "r+")) != NULL) {
		fscanf(fp, "%d", &counter);
		fseek(fp, 0, SEEK_SET);
	} else if ((fp = fopen(filename, "w")) == NULL) return 0;
	counter += number;
	fprintf(fp, "%d\n", counter);
	qfclose(fp);
	return counter;
}
