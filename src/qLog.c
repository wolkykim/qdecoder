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

#include "qDecoder.h"

/**********************************************
** Internal Functions Definition
**********************************************/

static int _realOpen(Q_LOG *log);

/**********************************************
** Usage :
** Return:
**********************************************/
Q_LOG *qLogOpen(char *pszLogBase, char *pszFilenameFormat, int nRotateInterval) {
  Q_LOG *log;

  /* malloc Q_LOG structure */
  if((log = (Q_LOG *)malloc(sizeof(Q_LOG))) == NULL) return NULL;

  /* fill structure */
  strcpy(log->szLogBase, pszLogBase);
  strcpy(log->szFilenameFormat, pszFilenameFormat);
  log->fp = NULL;
  log->nConsole = 0;
  log->nRotateInterval = (nRotateInterval > 0)?nRotateInterval:0;
  log->nNextRotate = 0;

  if(_realOpen(log) == 0) {
    qLogClose(log);
    return NULL;
  }

  return log;
}

/**********************************************
** Usage :
** Return:
**********************************************/
static int _realOpen(Q_LOG *log) {
  time_t nowtime = time(NULL);

  /* generate filename */
  strftime(log->szFilename, sizeof(log->szFilename), log->szFilenameFormat, localtime(&nowtime));
  sprintf(log->szLogPath, "%s/%s", log->szLogBase, log->szFilename);
  if(log->fp != NULL) fclose(log->fp);
  if((log->fp = fopen(log->szLogPath, "a")) == NULL) return 0;

  /* set next rotate time */
  if(log->nRotateInterval > 0) {
    time_t ct = time(NULL);
    time_t dt = ct - mktime(gmtime(&ct));
    log->nNextRotate = (((ct + dt) / log->nRotateInterval) + 1) * log->nRotateInterval;
  }
  else {
    log->nNextRotate = 0;
  }

  return 1;
}

/**********************************************
** Usage :
** Return:
**********************************************/
int qLogClose(Q_LOG *log) {
  if(log == NULL) return 0;
  if(log->fp != NULL) {
    fclose(log->fp);
    log->fp = NULL;
  }
  free(log);
  return 1;
}

/**********************************************
** Usage :
** Return:
**********************************************/
int qLogSetConsole(Q_LOG *log, int nFlag) {
  if(log == NULL) return 0;
  log->nConsole = ((nFlag == 0) ? 0 : 1);
  return 1;
}

/**********************************************
** Usage :
** Return: Success 0, otherwise EOF
**********************************************/
int qLogFlush(Q_LOG *log) {
  if(log == NULL || log->fp == NULL) return EOF;

  return fflush(log->fp);
}

/**********************************************
** Usage :
** Return:
**********************************************/
int qLog(Q_LOG *log, char *pszFormat, ...) {
  char szStr[1024];
  va_list arglist;
  time_t nowTime = time(NULL);

  if(log == NULL || log->fp == NULL) return 0;

  va_start(arglist, pszFormat);
  vsprintf(szStr, pszFormat, arglist);
  va_end(arglist);

  /* console out */
  if(log->nConsole) printf("%s(%d): %s\n", qGetTimeStr(), getpid(), szStr);

  /* check log rotate is needed*/
  if(log->nNextRotate > 0 && nowTime >= log->nNextRotate) {
    _realOpen(log);
  }

  fprintf(log->fp, "%s(%d): %s\n", qGetTimeStr(), getpid(), szStr);
}
