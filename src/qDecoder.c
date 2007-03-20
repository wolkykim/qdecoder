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
#include "qInternal.h"

/**********************************************
** Internal Functions Definition
**********************************************/

static int  _parse_urlencoded(void);
static char *_get_query(char *method);
static int  _parse_query(char *query, char sepchar);
static int  _parse_multipart_data(void);
static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, int *finish);
static char *_parse_multipart_value_into_disk(char *boundary, char *savedir, char *filename, int *filelen, int *finish);

static char *_upload_getsavedir(char *upload_id, char *upload_savedir);
static void _upload_progressbar(char *upload_id);
static int _upload_getstatus(char *upload_id, int *upload_tsize, int *upload_csize, char *upload_cname);
static int _upload_clear_savedir(char *dir);
static int _upload_clear_base();

/**********************************************
** Static Values Definition used only internal
**********************************************/

static Q_Entry *_first_entry = NULL;
static Q_Entry *_multi_last_entry = NULL;
static char _multi_last_key[1024];

static int _upload_base_init = 0;
static char _upload_base[1024];
static int _upload_clear_olderthan = 0;

static int _cookie_cnt = 0, _get_cnt = 0, _post_cnt = 0, _new_cnt = 0; /* counts per methods */

/**********************************************
** Usage : qDecoder();
** Return: Success number of values, Fail -1.
** Do    : Decode Query & Cookie then save it in linked list.
**         It doesn't care Method.
**********************************************/
int qDecoder(void) {
  int  amount = -1;
  char *content_type;

  if(_first_entry != NULL) return -1;

  content_type = getenv("CONTENT_TYPE");

  /* for GET method */
  if(content_type == NULL) {
    amount = _parse_urlencoded();

    /* if connection is upload progress dialog */
    if(_first_entry != NULL) {
      char *q_upload_id;

      q_upload_id = qValue("Q_UPLOAD_ID");

      if(q_upload_id != NULL) {
      	if(_upload_base_init == 0) qError("qDecoder(): qDecoderSetUploadBase() must be called before.");
        _upload_progressbar(q_upload_id);
        exit(0);
      }
    }
  }
  /* for POST method : application/x-www-form-urlencoded */
  else if(!strncmp (content_type, "application/x-www-form-urlencoded", strlen("application/x-www-form-urlencoded"))) {
    amount = _parse_urlencoded();
  }
  /* for POST method : multipart/form-data */
  else if(!strncmp(content_type, "multipart/form-data", strlen("multipart/form-data"))) {
    amount = _parse_multipart_data();
  }
  /* for stupid browser : Oracle Web Server */
  else {
    amount = _parse_urlencoded();
  }

  return amount;
}

/**********************************************
** Usage : qDecoderSetUploadBase(path);
** Do    : Set temporary uploading directory base.
**********************************************/
void qDecoderSetUploadBase(char *dir, int olderthan) {
  strcpy(_upload_base, dir);
  _upload_clear_olderthan = olderthan;
  _upload_base_init = 1;
}

/* For decode application/x-www-form-urlencoded, used by qDecoder() */
static int _parse_urlencoded(void) {
  char *query;
  int  amount;

  /* parse COOKIE */
  query = _get_query("COOKIE");
  _cookie_cnt = _parse_query(query, ';');
  amount = _cookie_cnt;
  if(query)free(query);

  /* parse GET method */
  query = _get_query("GET");
  _get_cnt = _parse_query(query, '&');
  amount += _get_cnt;
  if(query)free(query);

  /* parse POST method */
  query = _get_query("POST");
  _post_cnt = _parse_query(query, '&');
  amount += _post_cnt;
  if(query)free(query);

  return amount;
}

/* For fetch query used by _parse_urlencoded() */
static char *_get_query(char *method) {
  char *query;
  int cl, i;

  if(!strcmp(method, "GET")) {
    if(getenv("QUERY_STRING") == NULL) return NULL;
    query = strdup(getenv("QUERY_STRING"));
    /* SSI query handling */
    if(!strcmp(query, "") && getenv("REQUEST_URI") != NULL) {
      char *cp;
      for(cp = getenv("REQUEST_URI"); *cp != '\0'; cp++) {
        if(*cp == '?') { cp++; break; }
      }
      free(query);
      query = strdup(cp);
    }

    return query;
  }
  else if(!strcmp(method, "POST")) {
    if(getenv("REQUEST_METHOD") == NULL) return NULL;
    if(strcmp("POST", getenv("REQUEST_METHOD")))return NULL;
    if(getenv("CONTENT_LENGTH") == NULL) qError("_get_query(): Your browser sent a non-HTTP compliant message.");

    cl = atoi(getenv("CONTENT_LENGTH"));
    query = (char *)malloc(sizeof(char) * (cl + 1));
    for(i = 0; i < cl; i++)query[i] = fgetc(stdin);
    query[i] = '\0';
    return query;
  }
  else if(!strcmp(method, "COOKIE")) {
    if(getenv("HTTP_COOKIE") == NULL) return NULL;
    query = strdup(getenv("HTTP_COOKIE"));
    return query;
  }

  return NULL;
}

static int _parse_query(char *query, char sepchar) {
  int cnt;

  for(cnt = 0; query && *query; cnt++) {
    Q_Entry *entry;
    char *name, *value;

    value = _makeword(query, sepchar);
    name = qRemoveSpace(_makeword(value, '='));
    qURLdecode(name);
    qURLdecode(value);

    entry = _EntryAdd(_first_entry, name, value, 2);
    if(_first_entry == NULL) _first_entry = entry;
  }

  return cnt;
}

/* For decode multipart/form-data, used by qDecoder() */
static int _parse_multipart_data(void) {
  Q_Entry *entry;
  char *query, buf[1024];
  int  amount;

  char boundary[256];
  int  maxboundarylen; /* for check overflow attack */

  int  finish;

  /* for progress upload */
  int  upload_type = 0; /* 0: save into memory, 1: save into file */
  char upload_savedir[1024];
  char upload_tmppath[1024];

#ifdef _WIN32
  setmode(fileno(stdin), _O_BINARY);
  setmode(fileno(stdout), _O_BINARY);
#endif

  /*
   * For parse COOKIE and GET method
   */

  /* parse COOKIE */
  query = _get_query("COOKIE");
  _cookie_cnt = _parse_query(query, ';');
  amount = _cookie_cnt;
  if(query)free(query);

  /* parse GET method */
  query = _get_query("GET");
  _get_cnt = _parse_query(query, '&');
  amount += _get_cnt;
  if(query)free(query);

  /*
   * For parse multipart/form-data method
   */

  /* Force to check the boundary string length to defense overflow attack */
  maxboundarylen =  strlen("--");
  maxboundarylen += strlen(strstr(getenv("CONTENT_TYPE"), "boundary=") + strlen("boundary="));
  maxboundarylen += strlen("--");
  maxboundarylen += strlen("\r\n");
  if(maxboundarylen >= sizeof(boundary)) qError("_parse_multipart_data(): The boundary string is too long(Overflow Attack?). Stopping process.");

  /* find boundary string */
  sprintf(boundary,    "--%s", strstr(getenv("CONTENT_TYPE"), "boundary=") + strlen("boundary="));
  /* This is not necessary but, I can not trust MS Explore */
  qRemoveSpace(boundary);

  /* If you want to observe the string from stdin, enable this section. */
  /* This section is made for debugging.                                */
  if(0) {
    int i, j;
    qContentType("text/html");

    printf("Content Length = %s<br>\n", getenv("CONTENT_LENGTH"));
    printf("Boundary len %d : %s<br>\n", (int)strlen(boundary), boundary);
    for(i=0; boundary[i] !='\0'; i++) printf("%02X ",boundary[i]);
    printf("<p>\n");

    for(j = 1; _fgets(buf, sizeof(buf), stdin) != NULL; j++) {
      printf("Line %d, len %d : %s<br>\n", j, (int)strlen(buf), buf);
      for(i=0; buf[i] !='\0'; i++) printf("%02X ",buf[i]);
      printf("<p>\n");
    }
    exit(0);
  }

  /* check boundary */
  if(_fgets(buf, sizeof(buf), stdin) == NULL) qError("_parse_multipart_data(): Your browser sent a non-HTTP compliant message.");

  /* for explore 4.0 of NT, it sent \r\n before starting, fucking Micro$oft */
  if(!strcmp(buf, "\r\n")) _fgets(buf, sizeof(buf), stdin);

  if(strncmp(buf, boundary, strlen(boundary)) != 0) qError("_parse_multipart_data(): String format invalid.");

  for(finish = 0, _post_cnt = 0; finish != 1; amount++, _post_cnt++) {
    char *name="", *value=NULL, *filename="", *contenttype="";
    int  valuelen = 0;

    /* check file save mode */
    if(_first_entry != NULL && upload_type == 0) {
      char *upload_id;

      upload_id = qValue("Q_UPLOAD_ID");

      if(upload_id != NULL) {
      	if(_upload_base_init == 0) qError("_parse_multipart_data(): qDecoderSetUploadBase() must be called before.");

      	if(strlen(upload_id) == 0) upload_id = qUniqueID();
        upload_type = 1; /* turn on the flag - save into file directly */

        /* generate temporary uploading directory path */
        if(_upload_getsavedir(upload_id, upload_savedir) == NULL) qError("_parse_multipart_data(): Invalid Q_UPLOAD_ID");

        /* first, we clear old temporary files */
        if(_upload_clear_base() < 0) qError("_parse_multipart_data(): Can not remove old temporary files at %s", _upload_base);

        /* if exists, remove whole directory */
        if(qCheckFile(upload_savedir) == 1) {
        	if(_upload_clear_savedir(upload_savedir) == 0) qError("_parse_multipart_data(): Can not remove temporary uploading directory %s", upload_savedir);
        }

        /* make temporary uploading directory */
        if(mkdir(upload_savedir, 0755) == -1) qError("_parse_multipart_data(): Can not make temporary uploading directory %s", upload_savedir);

        /* save total contents length */
        sprintf(upload_tmppath, "%s/Q_UPLOAD_TSIZE", upload_savedir);
        if(qCountSave(upload_tmppath, atoi(getenv("CONTENT_LENGTH"))) == 0) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);

        /* save start time */
        sprintf(upload_tmppath, "%s/Q_UPLOAD_START", upload_savedir);
        if(qCountSave(upload_tmppath, time(NULL)) == 0) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);
      }
    }

    /* get information */
    while(_fgets(buf, sizeof(buf), stdin)) {
      if(!strcmp(buf, "\r\n")) break;
      else if(!qStrincmp(buf, "Content-Disposition: ", strlen("Content-Disposition: "))) {
        int c_count;

        /* get name field */
        name = strdup(buf + strlen("Content-Disposition: form-data; name=\""));
        for(c_count = 0; (name[c_count] != '\"') && (name[c_count] != '\0'); c_count++);
        name[c_count] = '\0';

        /* get filename field */
        if(strstr(buf, "; filename=\"") != NULL) {
          int erase;
          filename = strdup(strstr(buf, "; filename=\"") + strlen("; filename=\""));
          for(c_count = 0; (filename[c_count] != '\"') && (filename[c_count] != '\0'); c_count++);
          filename[c_count] = '\0';
          /* erase '\' */
          for(erase = 0, c_count = strlen(filename) - 1; c_count >= 0; c_count--) {
            if(erase == 1) filename[c_count]= ' ';
            else {
              if(filename[c_count] == '\\') {
                erase = 1;
                filename[c_count] = ' ';
              }
            }
          }
          qRemoveSpace(filename);
        }
      }
      else if(!qStrincmp(buf, "Content-Type: ", strlen("Content-Type: "))) {
        contenttype = strdup(buf + strlen("Content-Type: "));
        qRemoveSpace(contenttype);
      }
    }

    /* get value field */
    if(strcmp(filename, "") && upload_type == 1) {
      value = _parse_multipart_value_into_disk(boundary, upload_savedir, filename, &valuelen, &finish);
    }
    else {
      value = _parse_multipart_value_into_memory(boundary, &valuelen, &finish);
    }

    entry = _EntryAdd(_first_entry, name, value, 2);
    if(_first_entry == NULL) _first_entry = entry;

    /* store some additional info */
    if(strcmp(filename, "") != 0) {
      char *ename, *evalue;

      /* store data length, 'NAME.length'*/
      ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".length") + 1));
      evalue = (char *)malloc(sizeof(char) * 20 + 1);
      sprintf(ename,  "%s.length", name);
      sprintf(evalue, "%d", valuelen);
      _EntryAdd(_first_entry, ename, evalue, 2);

      /* store filename, 'NAME.filename'*/
      ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".filename") + 1));
      sprintf(ename,  "%s.filename", name);
      evalue = filename;
      _EntryAdd(_first_entry, ename, evalue, 2);

      /* store contenttype, 'NAME.contenttype'*/
      ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".contenttype") + 1));
      sprintf(ename,  "%s.contenttype", name);
      evalue = contenttype;
      _EntryAdd(_first_entry, ename, evalue, 2);

      _post_cnt += 3;

      if(upload_type == 1) {
        ename  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".savepath") + 1));
        sprintf(ename,  "%s.savepath", name);
        evalue = strdup(value);
        _EntryAdd(_first_entry, ename, evalue, 2);

        _post_cnt += 1;
      }
    }
  }

  if(upload_type == 1) { /* save end time */
    sprintf(upload_tmppath, "%s/Q_UPLOAD_END", upload_savedir);
    if(qCountSave(upload_tmppath, time(NULL)) == 0) qError("_parse_multipart_data(): Can not save uploading information at %s", upload_tmppath);
  }

  return amount;
}

static char *_parse_multipart_value_into_memory(char *boundary, int *valuelen, int *finish) {
  char boundaryEOF[256], rnboundaryEOF[256];
  char boundaryrn[256], rnboundaryrn[256];
  int  boundarylen, boundaryEOFlen;

  char *value;
  int  length;
  int  c, c_count, mallocsize;

  /* set boundary strings */
  sprintf(boundaryEOF, "%s--", boundary);
  sprintf(rnboundaryEOF, "\r\n%s", boundaryEOF);
  sprintf(boundaryrn, "%s\r\n", boundary);
  sprintf(rnboundaryrn, "\r\n%s\r\n", boundary);

  boundarylen    = strlen(boundary);
  boundaryEOFlen = strlen(boundaryEOF);

  for(value = NULL, length = 0, mallocsize = (1024 * 16), c_count = 0; (c = fgetc(stdin)) != EOF; ) {
    if(c_count == 0) {
      value = (char *)malloc(sizeof(char) * mallocsize);
      if(value == NULL) qError("_parse_multipart_data(): Memory allocation fail.");
    }
    else if(c_count == mallocsize - 1) {
      char *valuetmp;

      mallocsize *= 2;

      /* Here, we do not use realloc(). Because sometimes it is unstable. */
      valuetmp = (char *)malloc(sizeof(char) * mallocsize);
      if(valuetmp == NULL) qError("_parse_multipart_data(): Memory allocation fail.");
      memcpy(valuetmp, value, c_count);
      free(value);
      value = valuetmp;
    }
    value[c_count++] = (char)c;

    /* check end */
    if((c == '\n') || (c == '-')) {
      value[c_count] = '\0';

      if((c_count - (2 + boundarylen + 2)) >= 0) {
        if(!strcmp(value + (c_count - (2 + boundarylen + 2)), rnboundaryrn)) {
          value[c_count - (2 + boundarylen + 2)] = '\0';
          length = c_count - (2 + boundarylen + 2);
          break;
        }
      }
      if((c_count - (2 + boundaryEOFlen)) >= 0) {
        if(!strcmp(value + (c_count - (2 + boundaryEOFlen)), rnboundaryEOF)) {
          value[c_count - (2 + boundaryEOFlen)] = '\0';
          length = c_count - (2 + boundaryEOFlen);
          *finish = 1;
          break;
        }
      }

      /* For Micro$oft Explore on MAC, they do not follow rules */
      if((c_count - (boundarylen + 2)) == 0) {
        if(!strcmp(value, boundaryrn)) {
          value[0] = '\0';
          length = 0;
          break;
        }
      }
      if((c_count - boundaryEOFlen) == 0) {
        if(!strcmp(value, boundaryEOF)) {
          value[0] = '\0';
          length = 0;
          *finish = 1;
          break;
        }
      }
    }
  }

  if(c == EOF) qError("_parse_multipart_data(): Broken stream.");

  *valuelen = length;
  return value;
}

static char *_parse_multipart_value_into_disk(char *boundary, char *savedir, char *filename, int *filelen, int *finish) {
  char boundaryEOF[256], rnboundaryEOF[256];
  char boundaryrn[256], rnboundaryrn[256];
  int  boundarylen, boundaryEOFlen;

  /* input */
  char buffer[1024*8], *bp;
  int  bufc;
  int  c;

  /* output */
  static int upload_fcnt = 0; /* file save counter */
  FILE *upload_fp;
  char upload_path[1024];
  int  upload_length;

  /* temp */
  int i;

  /* set boundary strings */
  sprintf(boundaryEOF, "%s--", boundary);
  sprintf(rnboundaryEOF, "\r\n%s", boundaryEOF);
  sprintf(boundaryrn, "%s\r\n", boundary);
  sprintf(rnboundaryrn, "\r\n%s\r\n", boundary);

  boundarylen    = strlen(boundary);
  boundaryEOFlen = strlen(boundaryEOF);

  /* initialize */
  upload_fcnt++;
  sprintf(upload_path, "%s/%d-%s", savedir, upload_fcnt, filename);

  /* open file */
  upload_fp = fopen(upload_path, "w");
  if(upload_fp == NULL) qError("_parse_multipart_value_into_disk(): Can not open file %s", upload_path);

  /* read stream */
  for(upload_length = 0, bufc = 0, upload_length = 0; (c = fgetc(stdin)) != EOF; ) {
    if(bufc == sizeof(buffer) - 1) {
      int leftsize;

      /* save first 16KB */
      leftsize = boundarylen + 8;
      for(i = 0, bp = buffer; i < bufc-leftsize; i++) fputc(*bp++, upload_fp);
      memcpy(buffer, bp, leftsize);
      bufc = leftsize;
    }
    buffer[bufc++] = (char)c;
    upload_length++;

    /* check end */
    if((c == '\n') || (c == '-')) {
      buffer[bufc] = '\0';

      if((bufc - (2 + boundarylen + 2)) >= 0) {
        if(!strcmp(buffer + (bufc - (2 + boundarylen + 2)), rnboundaryrn)) {
          bufc          -= (2 + boundarylen + 2);
          upload_length -= (2 + boundarylen + 2);
          break;
        }
      }
      if((bufc - (2 + boundaryEOFlen)) >= 0) {
        if(!strcmp(buffer + (bufc - (2 + boundaryEOFlen)), rnboundaryEOF)) {
          bufc          -= (2 + boundaryEOFlen);
          upload_length -= (2 + boundaryEOFlen);
          *finish = 1;
          break;
        }
      }

      /* For Micro$oft Explore on MAC, they do not follow rules */
      if(upload_length == bufc) {
        if((bufc - (boundarylen + 2)) == 0) {
          if(!strcmp(buffer, boundaryrn)) {
            bufc = 0;
            upload_length = 0;
            break;
          }
        }
        if((bufc - boundaryEOFlen) == 0) {
          if(!strcmp(buffer, boundaryEOF)) {
            bufc = 0;
            upload_length = 0;
            *finish = 1;
            break;
          }
        }
      }
    }
  }

  if(c == EOF) qError("_parse_multipart_data(): Broken stream.");

  /* save lest */
  for(bp = buffer, i = 0; i < bufc; i++) fputc(*bp++, upload_fp);
  fclose(upload_fp);

  *filelen = upload_length;

  return strdup(upload_path);
}

static char *_upload_getsavedir(char *upload_id, char *upload_savedir) {
  char md5seed[1024];

  if(_upload_base_init == 0 || upload_id == NULL) return NULL;
  if(!strcmp(upload_id, "")) return NULL;

  sprintf(md5seed, "%s|%s|%s", QDECODER_PRIVATEKEY, qGetenvDefault("", "REMOTE_ADDR"), upload_id);
  sprintf(upload_savedir, "%s/Q_%s", _upload_base, qMD5Str(md5seed));

  return upload_savedir;
}

static void _upload_progressbar(char *upload_id) {
  int  drawrate = qiValue("Q_UPLOAD_DRAWRATE");
  char *template = qValue("Q_UPLOAD_TEMPLATE");

  int last_csize = 0, freezetime = 0;
  int upload_tsize = 0, upload_csize = 0;
  char upload_cname[256];

  /* adjust drawrate */
  if(drawrate == 0) drawrate = 1000;
  else if(drawrate < 100) drawrate = 100;
  else if(drawrate > 3000) drawrate = 3000;

  /* check arguments */
  if(!strcmp(upload_id, "")) {
    printf("_print_progressbar(): Q_UPLOAD_ID is invalid.");
    return;
  }
  if(template == NULL) {
    printf("_print_progressbar(): Q_UPLOAD_TEMPLATE query not found.");
    return;
  }

  /* print out qDecoder logo */
  qContentType("text/html");

  /* print template */
  if(qSedFile(NULL, template, stdout) == 0) {
    printf("_print_progressbar(): Can not open %s", template);
    return;
  }
  if(fflush(stdout) != 0) return;;

  /* draw progress bar */
  while(1) {
    upload_tsize = upload_csize = 0;

    _upload_getstatus(upload_id, &upload_tsize, &upload_csize, upload_cname);

    if(upload_tsize == 0 && upload_csize > 0) break; /* tsize file is removed. upload ended */

    if(last_csize < upload_csize) {
      qStrReplace("tr", upload_cname, "'", "`");

      printf("<script language='JavaScript'>");
      printf("if(Q_setProgress)Q_setProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
      printf("</script>\n");

      last_csize = upload_csize;
      freezetime = 0;
    }
    else if(last_csize > upload_csize) {
      break; /* upload ended */
    }
    else {
      if(freezetime > 10000) {
        break; /* maybe upload connection is closed */
      }

      if(upload_csize > 0) {
        printf("<script language='JavaScript'>");
        printf("if(Q_setProgress)Q_setProgress(%d,%d,'%s');", upload_tsize, upload_csize, upload_cname);
        printf("</script>\n");
      }

      freezetime += drawrate;
    }

    fflush(stdout);
    usleep(drawrate * 1000);
  }

  printf("<script language='JavaScript'>");
  printf("window.close();");
  printf("</script>\n");

  fflush(stdout);
}

static int _upload_getstatus(char *upload_id, int *upload_tsize, int *upload_csize, char *upload_cname) {
  DIR     *dp;
  struct  dirent *dirp;

  char upload_savedir[1024], upload_filepath[1024];

  /* initialize */
  *upload_tsize = *upload_csize = 0;
  strcpy(upload_cname, "");

  /* get basepath */
  if(_upload_getsavedir(upload_id, upload_savedir) == NULL) qError("_upload_getstatus(): Q_UPLOAD_ID does not set.");

  /* open upload folder */
  if((dp = opendir(upload_savedir)) == NULL) return 0;

  /* read tsize */
  sprintf(upload_filepath, "%s/Q_UPLOAD_TSIZE", upload_savedir);
  *upload_tsize = qCountRead(upload_filepath);

  while((dirp = readdir(dp)) != NULL) {
    if(dirp->d_name[0]-'0' <= 0 || dirp->d_name[0]-'0' > 9) continue; /* first char must be a number */

    /* sort last filename */
    if(strcmp(upload_cname, dirp->d_name) < 0) strcpy(upload_cname, dirp->d_name);

    sprintf(upload_filepath, "%s/%s", upload_savedir, dirp->d_name);
    *upload_csize += qFileSize(upload_filepath);
  }
  closedir(dp);

  if(strstr(upload_cname, "-") != NULL) {
    strcpy(upload_cname, strstr(upload_cname, "-")+1);
  }

  return 1;
}

static int _upload_clear_savedir(char *dir) {
  DIR     *dp;
  struct  dirent *dirp;
  char    filepath[1024];

  /* open upload folder */
  if((dp = opendir(dir)) == NULL) return 0;

  while((dirp = readdir(dp)) != NULL) {
    if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) continue;

    sprintf(filepath, "%s/%s", dir, dirp->d_name);
    unlink(filepath);
  }
  closedir(dp);

  if(rmdir(dir) != 0) return 0;
  return 1;
}

static int _upload_clear_base() {
  DIR     *dp;
  struct  dirent *dirp;
  char    filepath[1024];
  int     delcnt = 0;
  time_t  now = time(NULL);

  if(_upload_base_init == 0) return -1;
  if(_upload_clear_olderthan <= 0) return 0;

  /* open upload folder */
  if((dp = opendir(_upload_base)) == NULL) return 0;

  while((dirp = readdir(dp)) != NULL) {
    time_t starttime;

    if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..") || strncmp(dirp->d_name, "Q_", 2) != 0) continue;

    sprintf(filepath, "%s/%s/Q_UPLOAD_START", _upload_base, dirp->d_name);
    starttime = qCountRead(filepath);
    if(starttime > 0 && now - starttime < _upload_clear_olderthan) continue;

    sprintf(filepath, "%s/%s", _upload_base, dirp->d_name);
    if(_upload_clear_savedir(filepath) == 0) {
    	delcnt = -1;
    	break;
    }
    delcnt++;
  }
  closedir(dp);

  return delcnt;
}

/**********************************************
** Usage : qValue(query name);
** Return: Success pointer of value string, Fail NULL.
** Do    : Find value string pointer.
**         It find value in linked list.
**********************************************/
char *qValue(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qValue(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();
  return _EntryValue(_first_entry, name);
}

/**********************************************
** Usage : qiValue(query name);
** Return: Success integer of value string, Fail 0.
** Do    : Find value string pointer and convert to integer.
**********************************************/
int qiValue(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qiValue(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();
  return _EntryiValue(_first_entry, name);
}

/**********************************************
** Usage : qValueDefault(default string, query name);
** Return: Success pointer of value string, Fail using default string.
** Do    : If the query is not found, default string is used instead.
**********************************************/
char *qValueDefault(char *defstr, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;
  char *value;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qValueDefault(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();
  if((value = _EntryValue(_first_entry, name)) == NULL) value = defstr;

  return value;
}

/**********************************************
** Usage : qValueNotEmpty(error message, query name);
** Return: Success pointer of value string, Fail error message.
** Do    : Find value string pointer which is not empty and NULL.
**         When the query is not found or the value string is
**         empty, error message will be shown using qError().
**********************************************/
char *qValueNotEmpty(char *errmsg, char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;
  char *value;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qValueNotEmpty(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();
  if((value = _EntryValue(_first_entry, name)) == NULL) qError("%s", errmsg);
  if(!strcmp(value, "")) qError("%s", errmsg);

  return value;
}

/**********************************************
** Usage : qValueReplace(mode, query name, token string, word);
** Return: String pointer which is new or replaced.
** Do    : Replace string or tokens as word from linked list
**         with given mode.
**
** Refer the description of qStrReplace() for more detail.
**********************************************/
char *qValueReplace(char *mode, char *name, char *tokstr, char *word) {
  Q_Entry *entries;
  char *retstr, *repstr, method, memuse, newmode[2+1];

  /* initialize pointers to avoid compile warnings */
  retstr = repstr = NULL;

  if(_first_entry == NULL) qDecoder();

  if(strlen(mode) != 2) qError("qValueReplace(): Unknown mode \"%s\".", mode);
  method = mode[0], memuse = mode[1];
  newmode[0] = method, newmode[1] = 'n', newmode[2] = '\0';

  if(method != 't' && method != 's') qError("qValueReplace(): Unknown mode \"%s\".", mode);
  if(memuse == 'n') { /* new */
    if((repstr = _EntryValue(_first_entry, name)) != NULL) {
      retstr = qStrReplace(newmode, repstr, tokstr, word);
    }
    else retstr = NULL;
  }
  else if(memuse == 'r') { /* replace */
    /* To support multiful queries, it searches whole list and convert all of
       matched ones due to the possibility of duplicated query name.
       So when you need to do this replacement for duplicated query name,
       you can call this once before qValueFirst(). */
    for(retstr = NULL, entries = _first_entry; entries; entries = entries->next) {
      if(!strcmp(name, entries->name)) {
    	repstr = qStrReplace(newmode, entries->value, tokstr, word);
        free(entries->value);
    	entries->value = repstr;
    	if(retstr == NULL) retstr = repstr; /* To catch first matched one */
      }
    }
  }
  else qError("qValueReplace(): Unknown mode \"%s\".", mode);

  /* Return the value of first matched one */
  return retstr;
}

/**********************************************
** Usage : qValueFirst(query name);
** Return: Success pointer of first value string, Fail NULL.
** Do    : Find first value string pointer.
**********************************************/
char *qValueFirst(char *format, ...) {
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(_multi_last_key, format, arglist);
  if(strlen(_multi_last_key) + 1 > sizeof(_multi_last_key) || status == EOF) qError("qValueFirst(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();
  _multi_last_entry = _first_entry;

  return qValueNext();
}

/**********************************************
** Usage : qValueNext();
** Return: Success pointer of next value string, Fail NULL.
** Do    : Find next value string pointer.
**********************************************/
char *qValueNext(void) {
  Q_Entry *entries;

  for(entries = _multi_last_entry; entries; entries = entries->next) {
    if(!strcmp(_multi_last_key, entries->name)) {
      _multi_last_entry = entries->next;
      return (entries->value);
    }
  }
  _multi_last_entry = NULL;
  strcpy(_multi_last_key, "");

  return NULL;
}

/**********************************************
** Usage : qValueAdd(name, value);
** Do    : Force to add given name and value to linked list.
**         If same name exists, it'll be replaced.
**
** ex) qValueAdd("NAME", "Seung-young Kim");
**********************************************/
char *qValueAdd(char *name, char *format, ...) {
  Q_Entry *new_entry;
  char value[1024];
  int status;
  va_list arglist;

  if(!strcmp(name, "")) qError("qValueAdd(): can not add empty name.");

  va_start(arglist, format);
  status = vsprintf(value, format, arglist);
  if(strlen(value) + 1 > sizeof(value) || status == EOF) qError("qValueAdd(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();

  if(qValue(name) == NULL) _new_cnt++; /* if it's new entry, count up. */
  new_entry = _EntryAdd(_first_entry, name, value, 1);
  if(!_first_entry) _first_entry = new_entry;

  return qValue(name);
}

/**********************************************
** Usage : qValueRemove(name);
** Do    : Remove entry from linked list.
**
** ex) qValueRemove("NAME");
**********************************************/
void qValueRemove(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qSessionRemove(): Message is too long or invalid.");
  va_end(arglist);

  if(!strcmp(name, "")) qError("qValueRemove(): can not remove empty name.");

  switch(qValueType(name)) {
    case 'C' : { _cookie_cnt--; break; }
    case 'G' : { _get_cnt--; break; }
    case 'P' : { _post_cnt--; break; }
    case 'N' : { _new_cnt--; break; }
  }

  _first_entry = _EntryRemove(_first_entry, name);
}

/**********************************************
** Usage : qValueType(name);
** Return: Cookie 'C', Get method 'G', Post method 'P', New data 'N', Not found '-'
** Do    : Returns type of query.
**
** ex) qValueRemove("NAME");
**********************************************/
char qValueType(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;
  int v_no;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qValue(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();

  v_no = _EntryNo(_first_entry, name);
  if((1 <= v_no) && (v_no <= _cookie_cnt)) return 'C';
  else if((_cookie_cnt+1 <= v_no) && (v_no <= _cookie_cnt+_get_cnt)) return 'G';
  else if((_cookie_cnt+_get_cnt+1 <= v_no) && (v_no <= _cookie_cnt+_get_cnt+_post_cnt)) return 'P';
  else if((_cookie_cnt+_get_cnt+_post_cnt <= v_no)) return 'N';

  return '-';
}

/**********************************************
** Usage : qGetFirstEntry();
** Do    : Return _first_entry.
**********************************************/
Q_Entry *qGetFirstEntry(void) {
  if(_first_entry == NULL) qDecoder();
  return _first_entry;
}

/**********************************************
** Usage : qPrint(pointer of the first Entry);
** Return: Amount of entries.
** Do    : Print all parsed values & names for debugging.
**********************************************/
int qPrint(void) {
  int amount;
  if(_first_entry == NULL) qDecoder();
  amount = _EntryPrint(_first_entry);
  printf("<hr>\n");
  printf("COOKIE = %d , GET = %d , POST = %d, NEW = %d\n", _cookie_cnt, _get_cnt, _post_cnt, _new_cnt);
  return amount;
}

/**********************************************
** Usage : qFree(pointer of the first Entry);
** Do    : Make free of linked list memory.
**********************************************/
void qFree(void) {
  _EntryFree(_first_entry);
  _first_entry = NULL;
  _multi_last_entry = NULL;
  strcpy(_multi_last_key, "");
  _cookie_cnt = 0, _get_cnt = 0, _post_cnt = 0, _new_cnt = 0;
}


/**********************************************
** Usage : qCookieSet(name, value, expire days, path, domain, secure);
** Do    : Set cookie.
**
** The 'exp_days' is number of days which expire the cookie.
** The current time + exp_days will be set.
** This function should be called before qContentType().
**
** ex) qCookieSet("NAME", "Kim", 30, NULL, NULL, NULL);
**********************************************/
void qCookieSet(char *name, char *value, int exp_days, char *path, char *domain, char *secure) {
  char *Name, *Value;
  char cookie[(4 * 1024) + 256];

  /* check content flag */
  if(qGetContentFlag() == 1) qError("qCookieSet(): must be called before qContentType() and any stream out.");

  /* Name=Value */
  Name = qURLencode(name), Value = qURLencode(value);
  sprintf(cookie, "%s=%s", Name, Value);
  free(Name), free(Value);

  if(exp_days != 0) {
    time_t plus_sec;
    char gmt[256];
    plus_sec = (time_t)(exp_days * 24 * 60 * 60);
    qGetGMTime(gmt, plus_sec);
    strcat(cookie, "; expires=");
    strcat(cookie, gmt);
  }

  if(path != NULL) {
    if(path[0] != '/') qError("qCookieSet(): Path string(%s) must start with '/' character.", path);
    strcat(cookie, "; path=");
    strcat(cookie, path);
  }

  if(domain != NULL) {
    if(strstr(domain, "/") != NULL || strstr(domain, ".") == NULL) qError("qCookieSet(): Invalid domain name(%s).", domain);
    strcat(cookie, "; domain=");
    strcat(cookie, domain);
  }

  if(secure != NULL) {
    strcat(cookie, "; secure");
  }

  printf("Set-Cookie: %s\n", cookie);

  /* if you want to use cookie variable immediately, uncommnet below */
  /*
  qValueAdd(name, value);
  */
}

/**********************************************
** Usage : qCookieRemove(name);
** Do    : Remove cookie.
**
** ex) qCookieRemove("NAME");
**********************************************/
void qCookieRemove(char *name, char *path, char *domain, char *secure) {

  /* check content flag */
  if(qGetContentFlag() == 1) qError("qCookieRemove(): must be called before qContentType() and any stream out.");

  qCookieSet(name, "", -1, path, domain, secure);

  /* if you want to remove cookie variable immediately, uncomment below */
  /*
  qValueRemove(name);
  */
}

/**********************************************
** Usage : qCookieValue(cookie name);
** Return: Success pointer of value string, Fail NULL.
** Do    : It only finds cookie value in the linked-list.
**********************************************/
char *qCookieValue(char *format, ...) {
  char name[1024];
  int status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(name, format, arglist);
  if(strlen(name) + 1 > sizeof(name) || status == EOF) qError("qValue(): Message is too long or invalid.");
  va_end(arglist);

  if(_first_entry == NULL) qDecoder();

  if(qValueType(name) == 'C') {
    return _EntryValue(_first_entry, name);
  }

  return NULL;
}
