/***********************************************
** [Query String Decoder Version 4.2]
**
** Source  Code Name : qDecoder.c
** Include Code Name : qDecoder.h
**
** Last updated at July 7, 1998
**
** Developed by       'Seung-young, Kim'
** Technical contact : nobreak@nobreak.com
**
** (c) Nobreak Technologies, Inc.
**
** Designed by Perfectionist for Perfectionist!!!
**
** Example Usage :
**
** main(){
**   // automatically decode query when it is called first time
**   printf("Value = %s", qValue("Form_Name_String"));
**   // If you want to view all query data for debugging
**   qPrint();
**   // It's not necessary but for the perfectionist.
**   qFree();
** }
**
** If you use qPrint() or qValue(),
** qDecoder() is automatically called once..
**
**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "qDecoder.h"

/**********************************************
** Internal Functions Definition
**********************************************/
int  _parse_urlencoded(void);
char *_get_query(char *method);
int  _parse_multipart_data(void);
char *_fgetstring(char *buf, int maxlen, FILE *fp);
char *_fgetline(FILE *fp);

char _x2c(char hex_up, char hex_low);
char *_makeword(char *str, char stop);
char *_strtok2(char *str, char *token, char *retstop);


/**********************************************
** Static Values Definition used only internal
**********************************************/

static Entry *_first_entry = NULL;
static Entry *_cookie_first_entry = NULL;

static char  *_error_contact_info = NULL;
static char  *_error_log_filename = NULL;

/**********************************************
** Usage : qDecoder();
** Return: Success number of values, Fail -1
** Do    : Decode Query & Save it in linked list
**         It doesn't care Method
**********************************************/
int qDecoder(void){
  int  amount = -1;
  char *content_type;

  if(_first_entry != NULL) return -1;

  content_type = "";
  content_type = getenv("CONTENT_TYPE");

  if(content_type == NULL) {
    amount = _parse_urlencoded();
  }
  /* for application/x-www-form-urlencoded */
  else if(!strcmp (content_type, "application/x-www-form-urlencoded")) {
    amount = _parse_urlencoded();
  }
  /* for multipart/form-data */
  else if(!strncmp(content_type, "multipart/form-data", strlen("multipart/form-data"))) {
    amount = _parse_multipart_data();
  }
  else { /* For Oracle Web Server */
    amount = _parse_urlencoded();
  }

  return amount;
}

/* For decode application/s-www-form-urlencoded, used by qDecoder() */
int _parse_urlencoded(void){
  Entry *entries, *back;
  char *query;
  int  amount;

  entries = back = NULL;

  query = _get_query("GET");
  for(amount = 0; query && *query; amount++){
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(_first_entry == NULL) _first_entry = entries;

    entries->value = _makeword(query, '&');
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;
    qURLdecode(entries->name);
    qURLdecode(entries->value);
  }
  if(query)free(query);

  query = _get_query("POST");
  for(; query && *query; amount++){
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(_first_entry == NULL) _first_entry = entries;

    entries->value = _makeword(query, '&');
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qURLdecode(entries->name);
    qURLdecode(entries->value);
  }
  if(query)free(query);

  return amount;
}

/* For fetch query used by _parse_urlencoded() */
char *_get_query(char *method){
  char *query;
  int cl, i;

  if(!strcmp(method, "GET")){
    if(getenv("QUERY_STRING") == NULL) return NULL;
    query = strdup(getenv("QUERY_STRING"));
    return query;
  }
  if(!strcmp(method, "POST")){
    if(getenv("REQUEST_METHOD") == NULL) return NULL;
    if(strcmp("POST", getenv("REQUEST_METHOD")))return NULL;
    if(getenv("CONTENT_LENGTH") == NULL) qError("_get_query() : Your browser sent a non-HTTP compliant message.");

    cl = atoi(getenv("CONTENT_LENGTH"));
    query = (char *)malloc(sizeof(char) * (cl + 1));
    for(i = 0; i < cl; i++)query[i] = fgetc(stdin);
    query[i] = '\0';
    return query;
  }
  return NULL;
}

/* For decode multipart/form-data, used by qDecoder() */
int _parse_multipart_data(void) {
  Entry *entries, *back;
  int  amount;

  char *name = NULL, *value = NULL, *filename = NULL;
  int  valuelen;

  char boundary[0xff],     boundaryEOF[0xff];
  char rnboundaryrn[0xff], rnboundaryEOF[0xff];
  int  boundarylen,        boundaryEOFlen;

  char buf[1000];
  int  c, c_count;

  int  finish;

  entries = back = NULL;

  /* find boundary string */
  sprintf(boundary,    "--%s", strstr(getenv("CONTENT_TYPE"), "boundary=") + strlen("boundary="));
  /* This is not necessary but, I can not trust MS Explore */
  qRemoveSpace(boundary);

  sprintf(boundaryEOF, "%s--", boundary);

  sprintf(rnboundaryrn, "\r\n%s\r\n", boundary);
  sprintf(rnboundaryEOF, "\r\n%s", boundaryEOF);

  boundarylen    = strlen(boundary);
  boundaryEOFlen = strlen(boundaryEOF);


  /* If you want to observe the string from stdin, enable this section */
  /* This section is made for debugging                                */
  if(0) {
    int i, j;
    qContentType("text/html");

    printf("Content Length = %s<br>\n", getenv("CONTENT_LENGTH"));
    printf("Boundary len %d : %s<br>\n", strlen(boundary), boundary);
    for(i=0; boundary[i] !='\0'; i++) printf("%02X ",boundary[i]);
    printf("<p>\n");

    for(j = 1; _fgetstring(buf, sizeof(buf), stdin) != NULL; j++) {
      printf("Line %d, len %d : %s<br>\n", j, strlen(buf), buf);
      for(i=0; buf[i] !='\0'; i++) printf("%02X ",buf[i]);
      printf("<p>\n");
    }
    exit(0);
  }

  /* check boundary */
  if(_fgetstring(buf, sizeof(buf), stdin) == NULL) qError("_parse_multipart_data() : Your browser sent a non-HTTP compliant message.");

  /* for explore 4.0 of NT, it sent \r\n before starting, fucking Micro$oft */
  if(!strcmp(buf, "\r\n")) _fgetstring(buf, sizeof(buf), stdin);

  if(strncmp(buf, boundary, boundarylen) != 0) qError("_parse_multipart_data() : String format invalid.");

  for(amount = 0, finish = 0; finish != 1; amount++){
    /* get name field */
    _fgetstring(buf, sizeof(buf), stdin);
    name = (char *)malloc(sizeof(char) * (strlen(buf) - strlen("Content-Disposition: form-data; name=\"") + 1));
    strcpy(name, buf + strlen("Content-Disposition: form-data; name=\""));
    for(c_count = 0; (name[c_count] != '\"') && (name[c_count] != '\0'); c_count++);
    name[c_count] = '\0';

    /* get filename field */
    if(strstr(buf, "; filename=\"") != NULL) {
      int erase;
      filename = (char *)malloc(sizeof(char) * (strlen(buf) - strlen("Content-Disposition: form-data; name=\"") + 1));
      strcpy(filename, strstr(buf, "; filename=\"") + strlen("; filename=\""));
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
    else filename = "";

    /* skip header */
    for(;;) {
      _fgetstring(buf, sizeof(buf), stdin);
      if (!strcmp(buf, "\r\n")) break;
    }

    /* get value field */
    for(valuelen = 10000, c_count = 0; (c = fgetc(stdin)) != EOF; ) {
      if(c_count == 0) {
        value = (char *)malloc(sizeof(char) * (valuelen + 1));
        if(value == NULL) qError("_parse_multipart_data() : Memory allocation fail.");
      }
      else if(c_count == valuelen - 1) {
        char *valuetmp;
        int  i;

        valuelen *= 2;

        /* Here, we do not use realloc(). Because sometimes it is unstable. */
        valuetmp = (char *)malloc(sizeof(char) * (valuelen + 1));
        if(valuetmp == NULL) qError("_parse_multipart_data() : Memory allocation fail.");
        for(i = 0; i < c_count; i++) valuetmp[i] = value[i];
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
            valuelen = c_count - (2 + boundarylen + 2);
            break;
          }
        }
        if((c_count - (2 + boundaryEOFlen)) >= 0) {
          if(!strcmp(value + (c_count - (2 + boundaryEOFlen)), rnboundaryEOF)) {
            value[c_count - (2 + boundaryEOFlen)] = '\0';
            valuelen = c_count - (2 + boundaryEOFlen);
            finish = 1;
            break;
          }
        }

        /* For Micro$oft Explore on MAC, they do not follow rules */
        if((c_count - (boundarylen + 2)) == 0) {
          char boundaryrn[0xff];
          sprintf(boundaryrn, "%s\r\n", boundary);
          if(!strcmp(value, boundaryrn)) {
            value[0] = '\0';
            valuelen = 0;
            break;
          }
        }
        if((c_count - boundaryEOFlen) == 0) {
          if(!strcmp(value, boundaryEOF)) {
            value[0] = '\0';
            valuelen = 0;
            finish = 1;
            break;
          }
        }
      }
    }

    if(c == EOF) qError("_parse_multipart_data() : Internal bug at '%s'.", name);

    /* store in linked list */
    /* store data */
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(_first_entry == NULL) _first_entry = entries;

    entries->name  = name;
    entries->value = value;
    entries->next  = NULL;

    if(strcmp(filename, "") != 0) {
      /* store data length, 'NAME.length'*/
      back = entries;
      entries = (Entry *)malloc(sizeof(Entry));
      back->next = entries;

      entries->name  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".length") + 1));
      entries->value = (char *)malloc(sizeof(char) * 20 + 1);
      sprintf(entries->name,  "%s.length", name);
      sprintf(entries->value, "%d", valuelen);

      /* store transfer filename, 'NAME.filename'*/
      back = entries;
      entries = (Entry *)malloc(sizeof(Entry));
      back->next = entries;

      entries->name  = (char *)malloc(sizeof(char) * (strlen(name) + strlen(".filename") + 1));
      entries->value = filename;
      sprintf(entries->name,  "%s.filename", name);
      entries->next  = NULL;
    }
  }

  return amount;
}

/**********************************************
** Usage : qValue(Name);
** Return: Success pointer of value string, Fail NULL
** Do    : Find value string pointer
**         It find value in linked list
**********************************************/
char *qValue(char *name){
  Entry *entries;

  if(_first_entry == NULL)qDecoder();

  for(entries = _first_entry; entries; entries = entries->next){
    if(!strcmp(name, entries->name))return (entries->value);
  }
  return NULL;
}

int qiValue(char *name){
  char *str;

  str = qValue(name);
  if(str == NULL) return 0;
  return atoi(str);
}


/**********************************************
** Usage : qPrint();
** Do    : Print all parsed value & name for debugging
**********************************************/
void qPrint(void){
  Entry *entries;

  if(_first_entry == NULL)qDecoder();

  qContentType("text/html");

  for(entries = _first_entry; entries; entries = entries->next){
    printf("'%s' = '%s'<br>\n" , entries->name, entries->value);
  }
}

/**********************************************
** Usage : qFree();
** Do    : Make free of linked list memory
**********************************************/
void qFree(void){
  Entry *next;

  for(; _first_entry; _first_entry = next){
    next = _first_entry->next;
    free(_first_entry->name);
    free(_first_entry->value);
    free(_first_entry);
  }
  _first_entry = NULL;
}

/**********************************************
** Usage : qfDecoder(filename);
** Return: Success pointer of the first entry, Fail NULL
** Do    : Save file into linked list
           # is used for comments
**********************************************/
Entry *qfDecoder(char *filename){
  FILE  *fp;
  Entry *first, *entries, *back;
  char  *buf;

  fp = fopen(filename, "rt");
  if(fp == NULL) return NULL;    

  first = entries = back = NULL;

  while((buf = _fgetline(fp)) != NULL){
    qRemoveSpace(buf);
    if((buf[0] == '#') || (buf[0] == '\0')) continue;

    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(first == NULL) first = entries;

    entries->value = strdup(buf);
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qRemoveSpace(entries->name);
    qRemoveSpace(entries->value);

    free(buf);
  }

  fclose(fp);
  return first;
}

/**********************************************
** Usage : qfValue(Pointer of the first Entry, Name);
** Return: Success pointer of value string, Fail NULL
** Do    : Find value string pointer
**         It find value in linked list
**********************************************/
char *qfValue(Entry *first, char *name){
  Entry *entries;

  for(entries = first; entries; entries = entries->next){
    if(!strcmp(name, entries->name))return (entries->value);
  }
  return NULL;
}

/**********************************************
** Usage : qfPrint(Pointer of the first Entry);
** Do    : Print all parsed value & name for debugging
**********************************************/
void qfPrint(Entry *first){
  Entry *entries;

  qContentType("text/html");

  for(entries = first; entries; entries = entries->next){
    printf("'%s' = '%s'<br>\n" , entries->name, entries->value);
  }
}

/**********************************************
** Usage : qfFree(Pointer of the first Entry);
** Do    : Make free of linked list memory
**********************************************/
void qfFree(Entry *first){
  Entry *next;

  for(; first; first = next){
    next = first->next;
    free(first->name);
    free(first->value);
    free(first);
  }
  first = NULL;
}

/**********************************************
** Usage : qsDecoder(string);
** Return: Success pointer of the first entry, Fail NULL
** Do    : Save string into linked list
           # is used for comments
**********************************************/
Entry *qsDecoder(char *str){
  Entry *first, *entries, *back;
  char  *org, *buf, *offset;
  int  eos;

  if(str == NULL) return NULL;

  first = entries = back = NULL;

  if((org = strdup(str)) == NULL) qError("qsDecoder() : Memory allocation fail.");

  for(buf = offset = org, eos = 0; eos == 0; ) {
    for(buf = offset; *offset != '\n' && *offset != '\0'; offset++);
    if(*offset == '\0') eos = 1;
    else *offset = '\0', offset++;

    qRemoveSpace(buf);
    if((buf[0] == '#') || (buf[0] == '\0')) continue;

    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(first == NULL) first = entries;

    entries->value = strdup(buf);
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qRemoveSpace(entries->name);
    qRemoveSpace(entries->value);
  }

  free(org);

  return first;
}

/**********************************************
** Usage : qcDecoder();
** Return: Success number of values, Fail -1
** Do    : Decode COOKIES & Save it in linked list
**********************************************/
int qcDecoder(void){
  Entry *entries, *back;
  char *query;
  int  amount;

  if(_cookie_first_entry != NULL) return -1;

  if(getenv("HTTP_COOKIE") == NULL) return 0;
  query = strdup(getenv("HTTP_COOKIE"));

  entries = back = NULL;

  for(amount = 0; *query; amount++){
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(_cookie_first_entry == NULL) _cookie_first_entry = entries;

    entries->value = _makeword(query, ';');
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qURLdecode(entries->name);
    qURLdecode(entries->value);
    qRemoveSpace(entries->name);
  }
  free(query);

  return amount;
}

/**********************************************
** Usage : qcValue(Name);
** Return: Success pointer of value string, Fail NULL
** Do    : Find COOKIE value string pointer
**         It find value in linked list
**********************************************/
char *qcValue(char *name){
  Entry *entries;

  if(_cookie_first_entry == NULL)qcDecoder();

  for(entries = _cookie_first_entry; entries; entries = entries->next){
    if(!strcmp(name, entries->name))return (entries->value);
  }
  return NULL;
}

/**********************************************
** Usage : qcPrint();
** Do    : Print all parsed value & name for debugging
**********************************************/
void qcPrint(void){
  Entry *entries;

  if(_cookie_first_entry == NULL)qcDecoder();

  qContentType("text/html");

  for(entries = _cookie_first_entry; entries; entries = entries->next){
    printf("'%s' = '%s'<br>\n" , entries->name, entries->value);
  }
}

/**********************************************
** Usage : qcFree();
** Do    : Make free of linked list memory
**********************************************/
void qcFree(void){
  Entry *next;

  for(; _cookie_first_entry; _cookie_first_entry = next){
    next = _cookie_first_entry->next;
    free(_cookie_first_entry->name);
    free(_cookie_first_entry->value);
    free(_cookie_first_entry);
  }
  _cookie_first_entry = NULL;
}

/**********************************************
** Usage : qSetCookie();
** Do    : Set Cookie
**
** The 'exp_days' is number of days which expire the cookie
** The current time + exp_days will be set
** This is must called before qContentType();
**
** ex) qSetCookie("NAME", "Kim", 30, NULL, NULL, NULL);
**********************************************/
void qSetCookie(char *name, char *value, int exp_days, char *domain, char *path, char *secure){
  char *Name, *Value;
  char cookie[(4 * 1024) + (0xFF) + 1];

  /* Name=Value */
  Name = qURLencode(name), Value = qURLencode(value);
  sprintf(cookie, "%s=%s", Name, Value);
  free(Name), free(Value);

  if(exp_days != 0) {
    time_t plus_sec;
    char gmt[0xff];
    plus_sec = (time_t)(exp_days * 24 * 60 * 60);
    qGetGMTime(gmt, plus_sec);
    strcat(cookie, "; expires=");
    strcat(cookie, gmt);
  }

  if(domain != NULL) {
    strcat(cookie, "; domain=");
    strcat(cookie, domain);
  }

  if(path != NULL) {
    strcat(cookie, "; path=");
    strcat(cookie, path);
  }

  if(secure != NULL) {
    strcat(cookie, "; secure");
  }

  printf("Set-Cookie: %s\n", cookie);
}

/**********************************************
** Usage : qURLencode(string to encode);
** Return: Pointer of encoded str which is memory allocated.
** Do    : Encode string
**********************************************/
char *qURLencode(char *str){
  char *encstr, buf[2+1];
  int i, j;
  unsigned char c;
  if(str == NULL) return NULL;
  if((encstr = (char *)malloc((strlen(str) * 3) + 1)) == NULL) return NULL;

  for(i = j = 0; str[i]; i++){
    c = (unsigned char)str[i];
         if (c == ' ') encstr[j++] = '+';
    else if ((c >= '0') && (c <= '9')) encstr[j++] = c;
    else if ((c >= 'A') && (c <= 'Z')) encstr[j++] = c;
    else if ((c >= 'a') && (c <= 'z')) encstr[j++] = c;
    else if ((c == '@') || (c == '.')) encstr[j++] = c;
    else {
      sprintf(buf, "%02x", c);
      encstr[j++] = '%';
      encstr[j++] = buf[0];
      encstr[j++] = buf[1];
    }
  }
  encstr[j] = '\0';

  return encstr;
}

/**********************************************
** Usage : qURLdecode(Query Pointer);
** Return: Pointer of Query string
** Do    : Decode query string
**********************************************/
void qURLdecode(char *str){
  int i, j;

  if(!str)return;
  for(i = j = 0; str[j]; i++, j++){
    switch(str[j]){
      case '+':{
        str[i] = ' ';
        break;
      }
      case '%':{
        str[i] = _x2c(str[j + 1], str[j + 2]);
        j += 2;
        break;
      }
      default:{
        str[i] = str[j];
        break;
      }
    }
  }
  str[i]='\0';
}

/**********************************************
** Usage : qContentType(Mime Type);
** Do    : Print Content Type Once
**********************************************/
void qContentType(char *mimetype){
  static int flag = 0;  
  
  if(flag)return;

  printf("Content-type: %s%c%c", mimetype, 10, 10);
  flag = 1;
}

/**********************************************
** Usage : qPrintf(Mode, Format, Arg);
** Return: Sucess number of output bytes, Fail EOF
** Do    : Print message like printf
**         Mode : see qPuts()
**********************************************/
int qPrintf(int mode, char *format, ...){
  char buf[1000+1];
  int  status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(buf, format, arglist);
  if(status == EOF) return status;
  if(strlen(buf) > 1000)qError("qprintf() : Message is too long");

  qPuts(mode, buf);

  return status;
}

/**********************************************
** Usage : qPuts(Mode, String pointer);
** Do    : print HTML link as multi mode
**         Mode 0  : Same as printf()
**         Mode 1  : Print HTML TAG
**         Mode 2  : Mode 1 + Auto Link
**         Mode 3  : Mode 2 + Auto Link to _top frame
**         Mode 4  : Waste HTML TAG
**         Mode 5  : Mode 4 + Auto Link
**         Mode 6  : Mode 5 + Auto Link to _top frame
**
**         Mode 10  :Mode 0 + Convert
**         Mode 11 : Print HTML TAG + Convert
**         Mode 12 : Mode 11 + Auto Link
**         Mode 13 : Mode 12 + Auto Link to _top frame
**         Mode 14 : Waste HTML TAG + Convert
**         Mode 15 : Mode 14 + Auto Link
**         Mode 16 : Mode 15 + Auto Link to _top frame
**
**         Convert : " "   -> " "
**                   "  "  -> " &nbsp;"
**                   "   " -> " &nbsp;&nbsp;"
**                   "\n"   -> "<br>\n"
**                   "\r\n" -> "<br>\n"
**
** You can use 1x mode, to wrap long lines with no <pre> tag.
** Note) It modify argument string...
**********************************************/
void qPuts(int mode, char *buf){

  if(buf == NULL) return;

  if(mode == 0) printf("%s", buf);
  else if(mode == 10) {
    int i;
    for(i = 0; buf[i] != '\0'; i++) {
      switch(buf[i]) {
        case ' '  : {
          if((i > 0) && (buf[i - 1] == ' ')) printf("&nbsp;");
          else printf(" ");
          break;
        }
        case '\r' : {
          break;
        }
        case '\n' : {
          printf("<br>\n");
          break;
        }
        default   : {
          printf("%c", buf[i]);
          break;
        }
      }
    }
  }
  else {
    char *ptr, retstop, lastretstop, *target, *token;
    int printhtml, autolink, convert, linkflag, ignoreflag;

    /* set defaults, mode 2*/
    printhtml = 1;
    autolink  = 1;
    target    = "_top";
    convert   = 0;

    switch(mode){
      case 1  : {printhtml = 1, autolink = 0, target = "";     convert = 0; break;}
      case 2  : {printhtml = 1, autolink = 1, target = "";     convert = 0; break;}
      case 3  : {printhtml = 1, autolink = 1, target = "_top"; convert = 0; break;}
      case 4  : {printhtml = 0, autolink = 0, target = "";     convert = 0; break;}
      case 5  : {printhtml = 0, autolink = 1, target = "";     convert = 0; break;}
      case 6  : {printhtml = 0, autolink = 1, target = "_top"; convert = 0; break;}
      case 11 : {printhtml = 1, autolink = 0, target = "";     convert = 1; break;}
      case 12 : {printhtml = 1, autolink = 1, target = "";     convert = 1; break;}
      case 13 : {printhtml = 1, autolink = 1, target = "_top"; convert = 1; break;}
      case 14 : {printhtml = 0, autolink = 0, target = "";     convert = 1; break;}
      case 15 : {printhtml = 0, autolink = 1, target = "";     convert = 1; break;}
      case 16 : {printhtml = 0, autolink = 1, target = "_top"; convert = 1; break;}

      default: {qError("_autolink() : Invalid Mode (%d)", mode); break;}
    }

    token = " `(){}[]<>&\"',\r\n";

    lastretstop = '0'; /* any character except space */
    ptr = _strtok2(buf, token, &retstop);

    for(linkflag = ignoreflag = 0; ptr != NULL;){

      /* auto link */
      if(ignoreflag == 0) {
        if(autolink == 1){
          if(!strncmp(ptr, "http://",        7)) linkflag = 1;
          else if(!strncmp(ptr, "ftp://",    6)) linkflag = 1;     
          else if(!strncmp(ptr, "telnet://", 9)) linkflag = 1;
          else if(!strncmp(ptr, "news:",     5)) linkflag = 1;
          else if(!strncmp(ptr, "mailto:",   7)) linkflag = 1;
          else if(qCheckEmail(ptr) == 1)         linkflag = 2;
          else linkflag = 0;
        }
        if(linkflag == 1) printf("<a href='%s' target='%s'>%s</a>", ptr, target, ptr);
        else if(linkflag == 2) printf("<a href='mailto:%s' target='%s'>%s</a>", ptr, target, ptr);
        else printf("%s", ptr);
      }

      /* print */
      if(printhtml == 1){
        if     (retstop == '<')  printf("&lt;");
        else if(retstop == '>')  printf("&gt;");
        else if(retstop == '\"') printf("&quot;");
        else if(retstop == '&')  printf("&amp;");

        else if(retstop == ' '  && convert == 1) {
          if(lastretstop == ' ' && strlen(ptr) == 0) printf("&nbsp;");
          else printf(" ");
        }
        else if(retstop == '\r' && convert == 1); /* skip when convert == 1 */
        else if(retstop == '\n' && convert == 1) printf("<br>\n");

        else if(retstop != '\0') printf("%c", retstop);
      }
      else {
        if     (retstop == '<') ignoreflag = 1;
        else if(retstop == '>') ignoreflag = 0;

        else if(retstop == '\"' && ignoreflag == 0) printf("&quot;");
        else if(retstop == '&'  && ignoreflag == 0) printf("&amp;");

        else if(retstop == ' '  && ignoreflag == 0 && convert == 1) {
          if(lastretstop == ' ' && strlen(ptr) == 0) printf("&nbsp;");
          else printf(" ");
        }
        else if(retstop == '\r' && ignoreflag == 0 && convert == 1); /* skip when convert == 1 */
        else if(retstop == '\n' && ignoreflag == 0 && convert == 1) printf("<br>\n");

        else if(retstop != '\0' && ignoreflag == 0) printf("%c", retstop);

      }

      lastretstop = retstop;
      ptr = _strtok2(NULL, token, &retstop);
    }
  }
}

/**********************************************
** Usage : qError(Format, Arg);
** Do    : Print error message
**********************************************/
void qError(char *format, ...){
  char buf[1000 + 1];
  int status;
  int logstatus;
  va_list arglist;

  va_start(arglist, format);

  status = vsprintf(buf, format, arglist);
  if(strlen(buf) > 1000 || status == EOF){
    printf("qError() : Message is too long or not valid");
    exit(1);
  }

  logstatus = 0;
  if(_error_log_filename != NULL) {
    FILE *fp;

    if((fp = fopen(_error_log_filename, "at")) == NULL) logstatus = -1;
    else {
      char *http_user_agent, *remote_host;
      struct tm *time;

      time = qGetTime();

      if((http_user_agent = getenv("HTTP_USER_AGENT")) == NULL) http_user_agent = "null";
      if((remote_host     = getenv("REMOTE_HOST"))     == NULL) {
        /* Fetch for Apache 1.3 */
        if((remote_host     = getenv("REMOTE_ADDR"))   == NULL) remote_host = "null";
      }

      fprintf(fp, "%04d/%02d/%02d(%02d:%02d) : '%s' from %s (%s)\n",
              time->tm_year, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min,
              buf, remote_host, http_user_agent);

      fclose(fp);
      logstatus = 1;
    }
  }

  if(getenv("REMOTE_ADDR") == NULL)  {
    printf("Error: %s\n", buf);
  }
  else {
    qContentType("text/html");

    if(_error_contact_info != NULL) {
      strcat(buf, " [Administrator:");
      strcat(buf, _error_contact_info);
      strcat(buf, "]");
    }
    if(logstatus == -1) strcat(buf, " [ERROR LOGGING FAIL]");

    printf("<html>\n");
    printf("<head>\n");
    printf("<title>Error: %s</title>\n", buf);
    printf("<script language='JavaScript'>\n");
    printf("  alert(\"%s\");\n", buf);
    printf("  history.back();\n");
    printf("</script>\n");
    printf("</head>\n");
    printf("</html>\n");
  }

  qFree();
  qcFree();

  exit(1);
}

/**********************************************
** Usage : qErrorLog(logFilename);
** Do    : Turn Error log on
**********************************************/
void qErrorLog(char *filename) {
  _error_log_filename = filename;
}

/**********************************************
** Usage : qErrorContact(logFilename);
** Do    : Error contact information
**********************************************/
void qErrorContact(char *msg) {
  _error_contact_info = msg;
}


/**********************************************
** Usage : qCgienv(Pointer of Cgienv);
** Do    : Get environment of CGI
**********************************************/
void qCgienv(Cgienv *env){
  struct tm *envtime;

  envtime = qGetTime();

  env->auth_type         = qGetEnv("AUTH_TYPE", NULL);
  env->content_length    = qGetEnv("CONTENT_LENGTH", NULL);
  env->content_type      = qGetEnv("CONTENT_TYPE", NULL);
  env->document_root     = qGetEnv("DOCUMENT_ROOT", NULL);
  env->gateway_interface = qGetEnv("GATEWAY_INTERFACE", NULL);
  env->http_accept       = qGetEnv("HTTP_ACCEPT", NULL);
  env->http_cookie       = qGetEnv("HTTP_COOKIE", NULL);
  env->http_user_agent   = qGetEnv("HTTP_USER_AGENT", NULL);
  env->query_string      = qGetEnv("QUERY_STRING", NULL);
  env->remote_addr       = qGetEnv("REMOTE_ADDR", NULL);
  env->remote_host       = qGetEnv("REMOTE_HOST", env->remote_addr);
  env->remote_user       = qGetEnv("REMOTE_USER", NULL);
  env->remote_port       = qGetEnv("REMOTE_PORT", NULL);
  env->request_method    = qGetEnv("REQUEST_METHOD", NULL);
  env->script_name       = qGetEnv("SCRIPT_NAME", NULL);
  env->script_filename   = qGetEnv("SCRIPT_FILENAME", NULL);
  env->server_name       = qGetEnv("SERVER_NAME", NULL);
  env->server_protocol   = qGetEnv("SERVER_PROTOCOL", NULL);
  env->server_port       = qGetEnv("SERVER_PORT", NULL);
  env->server_software   = qGetEnv("SERVER_SOFTWARE", NULL);
  env->server_admin      = qGetEnv("SERVER_ADMIN", NULL);

  /* qDecoder Supported Extended Informations */
  env->year = envtime->tm_year;
  env->mon  = envtime->tm_mon;
  env->day  = envtime->tm_mday;
  env->hour = envtime->tm_hour;
  env->min  = envtime->tm_min;
  env->sec  = envtime->tm_sec;
}

/**********************************************
** Usage : qGetEnv(Environment string name, Fail return string pointer);
** Return: Environment string or 'nullstr'.
** Do    : Get environment string.
**         When 'getenv' does not find 'envname', it will return 'nullstr.
**********************************************/
char *qGetEnv(char *envname, char *nullstr) {
  char *envstr;

  if((envstr = getenv(envname)) != NULL) return envstr;

  return nullstr;
}

/**********************************************
** Usage : qCGIname();
** Return: CGI filename.
**********************************************/
char *qCGIname(void) {
  static char cgi_name[0xff];
  char *c;

  if(getenv("SCRIPT_NAME") == NULL) return NULL;

  strcpy(cgi_name, getenv("SCRIPT_NAME"));

  /* Fetch filename in string which include directory name */
  for(c = cgi_name + strlen(cgi_name) - 1; c >= cgi_name && !(*c == '/' || *c == '\\'); c--);
  for(; c >= cgi_name; c--) *c = ' ';
  qRemoveSpace(cgi_name);

  return cgi_name;
}

/**********************************************
** Usage : qGetTime(void);
** Return: Pointer of struct tm
** Do    : Get Time
**********************************************/
struct tm *qGetTime(void){
  time_t nowtime;
  struct tm *nowlocaltime;

  nowtime = time(NULL);
  nowlocaltime = localtime(&nowtime);
  nowlocaltime->tm_year += 1900;
  nowlocaltime->tm_mon++;

  return nowlocaltime;
}

/**********************************************
** Usage : qGetGMTime(gmt, plus_sec);
** Do    : Make string of GMT Time for Cookie
** Return: Amount second from 1970/00/00 00:00:00
**
** plus_sec will be added to current time;
**********************************************/
time_t qGetGMTime(char *gmt, time_t plus_sec) {
  time_t nowtime;
  struct tm *nowgmtime;

  nowtime = time(NULL);
  nowtime += plus_sec;
  nowgmtime = gmtime(&nowtime);

  strftime(gmt, 0xff, "%a, %d-%b-%Y %H:%M:%S %Z", nowgmtime);

  return nowtime;
}

/**********************************************
** Usage : qCheckFile(Filename);
** Return: If file exist, return 1. Or return 0;
** Do    : Check filethat file is existGet environment of CGI
**********************************************/
int qCheckFile(char *filename){
  FILE *fp;
  fp = fopen(filename, "rb");
  if(fp == NULL) return 0;
  fclose(fp);
  return 1;
}

/**********************************************
** Usage : qSendFile(filename);
** Return: Success 1, Fail 0
** Do    : Print file to stdout
**********************************************/
int qSendFile(char *filename){
  FILE *fp;
  int tmp;

  fp = fopen(filename, "rb");
  if(fp == NULL)return 0;
  while((tmp = fgetc(fp)) != EOF)printf("%c", tmp);  
  fclose(fp);
  return 1;
}

/**********************************************
** Usage : qDownload(filename);
** Do    : Pump file to stdout, do not call qContentType().
**********************************************/
void qDownload(char *filename) {
  char *file, *c;

  if(filename == NULL) qError("qDownload() : Null pointer can not be used.");
  if(qCheckFile(filename) == 0) qError("qDownload() : File(%s) not found.", filename);

  file = strdup(filename);

  /* Fetch filename in string which include directory name */
  for(c = file + strlen(file) - 1; c >= file && !(*c == '/' || *c == '\\'); c--);
  for(; c >= file; c--) *c = ' ';
  qRemoveSpace(file);

  printf ("Content-type: application/octet-stream\n");
  printf ("Content-disposition: attachment; filename=\"%s\"\n\n", file);
  free(file);

  qSendFile(filename);
}

/**********************************************
** Usage : qReadCounter(filename);
** Return: Success counter value, Fail 0
** Do    : Read counter value
**********************************************/
int qReadCounter(char *filename){
  FILE *fp;
  int  counter;

  fp = fopen(filename, "rt");
  if(fp == NULL) return 0;
  fscanf(fp, "%d", &counter);
  fclose(fp);
  return counter;
}

/**********************************************
** Usage : qSaveCounter(filename, number);
** Return: Success 1, Fail 0
** Do    : Save counter value
**********************************************/
int qSaveCounter(char *filename, int number){
  FILE *fp;

  fp = fopen(filename, "wt");
  if(fp == NULL)return 0;
  fprintf(fp, "%d\n", number);
  fclose(fp);
  return 1;
}

/**********************************************
** Usage : qUpdateCount(filename, number);
** Return: Success Current Value + 1, Fail 0
** Do    : Update counter value, Save +1
**********************************************/

int qUpdateCounter(char *filename){
  FILE *fp;
  int counter = 0;

  if((fp = fopen(filename, "r+t")) != NULL) {
    fscanf(fp, "%d", &counter);
    fseek(fp, 0, SEEK_SET);
  }
  else if((fp = fopen(filename, "wt")) == NULL) return 0;

  fprintf(fp, "%d\n", ++counter);
  fclose(fp);
  return counter;
}


/**********************************************
** Usage : qCheckEmail(E-mail Address);
** Return: If it is valid return 1. Or return 0;
** Do    : Check E-mail address
**********************************************/
int qCheckEmail(char *email){
  int i, alpa, dot, gol;

  if(email == NULL) return 0;

  for(i = alpa = dot = gol = 0; email[i] != '\0'; i++){
    switch(email[i]) {
      case '@' : {
        if(alpa == 0) return 0;
        if(gol > 0)   return 0;
        gol++;
        break;
      }
      case '.' : {
        if((i > 0)   && (email[i - 1] == '@')) return 0;
        if((gol > 0) && (email[i - 1] == '.')) return 0;
        dot++;
        break;
      }
      default  : {
        alpa++;
             if((email[i] >= '0') && (email[i] <= '9')) break;
        else if((email[i] >= 'A') && (email[i] <= 'Z')) break;
        else if((email[i] >= 'a') && (email[i] <= 'z')) break;
        else if((email[i] == '-') || (email[i] == '_')) break;
        else return 0;
      }
    }
  }

  if((alpa <= 3) || (gol == 0) || (dot == 0))return 0;

  return 1;
}

/**********************************************
** Usage : qCheckURL(internet address);
** Return: If it is valid return 1. Or return 0;
** Do    : Check URL
**********************************************/
int qCheckURL(char *url){
  if(!strncmp(url, "http://", 7)) return 1;
  else if(!strncmp(url, "ftp://", 6)) return 1;     
  else if(!strncmp(url, "telnet://", 9)) return 1;
  else if(!strncmp(url, "mailto:", 7)) return 1;
  else if(!strncmp(url, "news:", 5)) return 1;
  return 0;
}

/**********************************************
** Usage : qRemoveSpace(Source string);
** Return: Pointer of str
** Do    : Remove Space before string & after string
**         Remove CR, LF
**********************************************/
char *qRemoveSpace(char *str){
  int i, j;
  
  if(!str)return NULL;

  for(j = 0; str[j] == ' ' || str[j] == 9; j++);
  for(i = 0; str[j] != '\0'; i++, j++) str[i] = str[j];
  for(i--; (i >= 0) && (str[i] == ' ' || str[i] == 9 || str[i] == '\r' || str[i] == '\n'); i--);
  str[i+1] = '\0';

  return str;
}

/**********************************************
** Usage : qStr09AZaz(string);
** Return: Valid 1, Invalid 0
** Do    : Check characters of string is in 0-9, A-Z, a-z
**********************************************/
int qStr09AZaz(char *str){
  for(; *str; str++){
    if((*str >= '0') && (*str <= '9')) continue;
    else if((*str >= 'A') && (*str <= 'Z')) continue;
    else if((*str >= 'a') && (*str <= 'z')) continue;
    else return 0;
  }
  return 1;
}

/**********************************************
** Usage : qStrBig(string);
** Return: Pointer of converted string
** Do    : Convert small char to big char
**********************************************/
char *qStrBig(char *str) {
  char *tmp;

  if(str == NULL) return NULL;
  tmp = str;
  for( ; *str; str++) if(*str >= 'a' && *str <= 'z') *str -= 32;
  return tmp;
}

/**********************************************
** Usage : qStrFind(string, token);
** Return: Finded 1, Fail 0
** Do    : Find token whih no case-censitive
**********************************************/
int qStrFind(char *orgstr, char *tokstr) {
  char *org, *tok;
  int  findflag;

  if(orgstr == NULL || tokstr == NULL) return 0;

  if((org = strdup(orgstr)) == NULL) return 0;
  if((tok = strdup(tokstr)) == NULL) { free(org); return 0; }

  qStrBig(org), qStrBig(tok);

  if(strstr(org, tok) == NULL) findflag = 0;
  else                         findflag = 1;

  free(org), free(tok);

  return findflag;
}


/**********************************************
***********************************************
** You don't need to use below functions
** It is used by qDecoder();
***********************************************
**********************************************/

/**********************************************
** Usage : _x2c(HEX Up character, HEX Low character);
** Return: Hex value which is changed
** Do    : Change two hex character to one hex value
**********************************************/
char _x2c(char hex_up, char hex_low){
  char digit;

  digit = 16 * (hex_up >= 'A' ? ((hex_up & 0xdf) - 'A') + 10 : (hex_up - '0'));
  digit += (hex_low >= 'A' ? ((hex_low & 0xdf) - 'A') + 10 : (hex_low - '0'));

  return (digit);
}


/**********************************************
** Usage : _makeword(Source string, Stop character);
** Return: Pointer of Parsed string
** Do    : It copy source string before stop character
**         The pointer of source string direct after stop character
**********************************************/
char *_makeword(char *str, char stop){
  char *word;
  int  len, i;

  for(len = 0; ((str[len] != stop) && (str[len])); len++);
  word = (char *)malloc(sizeof(char) * (len + 1));

  for(i = 0; i < len; i++)word[i] = str[i];
  word[i] = '\0';

  if(str[len])len++;
  for(i = len; str[i]; i++)str[i - len] = str[i];
  str[i - len] = '\0';

  return (word);
}

/*********************************************
** Usage : _strtok2(String, TokenStopString, ReturnStopCharacter);
** Do    : Find token string (usage like strtok())
** Return: Pointer of token & character of stop
**********************************************/
char *_strtok2(char *str, char *token, char *retstop){
  static char *tokensp, *tokenep;
  int i, j;

  if(str != NULL) tokensp = tokenep = str;
  else tokensp = tokenep;

  for(i = strlen(token);*tokenep;tokenep++){
    for(j = 0; j < i; j++){
      if(*tokenep == token[j]){
        *retstop = token[j];
        *tokenep = '\0';
        tokenep++;
        return tokensp;
      }
    }
  }

  *retstop = '\0';
  if(tokensp != tokenep) return tokensp;
  return NULL;
}

/*********************************************
** Usage : This function is perfectly same as fgets();
**********************************************/
char *_fgetstring(char *buf, int maxlen, FILE *fp) {
  int i, c;

  for(i = 0; i < (maxlen - 1); i++) {
    c = fgetc(fp);
    if(c == EOF) break;
    buf[i] = (char)c;
    if(c == '\n') {
      i++;
      break;
    }
  }
  if(i == 0) return NULL;

  buf[i] = '\0';
  return buf;
}

/*********************************************
** Usage : This function is allocate memory for save string.
**         And there is no limit about length.
**********************************************/
char *_fgetline(FILE *fp) {
  int memsize;
  int c, c_count;
  char *string = NULL;

  for(memsize = 1000, c_count = 0; (c = fgetc(fp)) != EOF;) {
    if(c_count == 0) {
      string = (char *)malloc(sizeof(char) * (memsize + 1));
      if(string == NULL) qError("_fgetline() : Memory allocation fail.");
    }
    else if(c_count == memsize - 1) {
      char *stringtmp;
      int  i;

      memsize *= 2;

      /* Here, we do not use realloc(). Because sometimes it is unstable. */
      stringtmp = (char *)malloc(sizeof(char) * (memsize + 1));
      if(stringtmp == NULL) qError("_fgetline() : Memory allocation fail.");
      for(i = 0; i < c_count; i++) stringtmp[i] = string[i];
      free(string);
      string = stringtmp;
    }
    string[c_count++] = (char)c;
    if((char)c == '\n') break;
  }

  if(c_count == 0 && c == EOF) return NULL;

  string[c_count] = '\0';

  return string;
}
