/***********************************************
** [Query String Decoder Version 3.4.2]
**
** Last Modified : 1997/09/01
**
**  Source  Code Name : qDecoder.c
**  Include Code Name : qDecoder.h
**
** Programmed by 'Seung-young, Kim'
** Email : nobreak@shinan.hongik.ac.kr
**
** Hongik University Shinan Campus
**
**
** Designed by Perfectionist for Perfectionist!!!
**
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
char *_get_query(char *method);
void _decode_query(char *str);
char _x2c(char hex_up, char hex_low);
char *_makeword(char *str, char stop);
char *_strtok2(char *str, char *token, char *retstop);

/**********************************************
** Static Values Definition used only internal
**********************************************/

static Entry *_first_entry = NULL;
static Entry *_cookie_first_entry = NULL;

/**********************************************
** Usage : qDecoder();
** Return: Success number of values, Fail -1
** Do    : Decode Query & Save it in linked list
**         It doesn't care Method
**********************************************/
int qDecoder(void){
  Entry *entries, *back;
  char *query;
  int  amount;

  if(_first_entry != NULL) return -1;

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
    _decode_query(entries->name);
    _decode_query(entries->value);
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

    _decode_query(entries->name);
    _decode_query(entries->value);
  }
  if(query)free(query);

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
**********************************************/
Entry *qfDecoder(char *filename){
  FILE  *fp;
  Entry *first, *entries, *back;
  char  buf[1000 + 1];

  fp = fopen(filename, "rt");
  if(fp == NULL) return NULL;    

  first = entries = back = NULL;

  while(fgets(buf, sizeof(buf), fp)){
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(first == NULL) first = entries;

    entries->value = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
    strcpy(entries->value, buf);
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    qRemoveSpace(entries->name);
    qRemoveSpace(entries->value);
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
  query = (char *)malloc(sizeof(char) * (strlen(getenv("HTTP_COOKIE")) + 1));
  strcpy(query, getenv("HTTP_COOKIE"));

  entries = back = NULL;

  for(amount = 0; *query; amount++){
    back = entries;
    entries = (Entry *)malloc(sizeof(Entry));
    if(back != NULL) back->next = entries;
    if(_cookie_first_entry == NULL) _cookie_first_entry = entries;

    entries->value = _makeword(query, ';');
    entries->name  = _makeword(entries->value, '=');
    entries->next  = NULL;

    _decode_query(entries->name);
    _decode_query(entries->value);
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
** ex) qSetCookie("NAME", "Kim", "30", NULL, NULL, NULL);
**********************************************/
void qSetCookie(char *name, char *value, char *exp_days, char *domain, char *path, char *secure){
  char *Name, *Value;
  char cookie[(4 * 1024) + (0xFF) + 1];

  /* Name=Value */
  Name = qURLencode(name), Value = qURLencode(value);
  sprintf(cookie, "%s=%s", Name, Value);
  free(Name), free(Value);

  if(exp_days != NULL) {
    time_t plus_sec;
    char gmt[0xff];
    plus_sec = (time_t)(atoi(exp_days) * 24 * 60 * 60);
    qGetGMTTime(gmt, plus_sec);
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
    switch(c){
      case ' ': {
        encstr[j++] = '+';
        break;
      }
      default : {
        sprintf(buf, "%02x", c);
        encstr[j++] = '%';
        encstr[j++] = buf[0];
        encstr[j++] = buf[1];
        break;
      }
    }
  }
  encstr[j] = '\0';
  return encstr;
}

/**********************************************
** Usage : qContentType(Mime Type);
** Do    : Print Content Type Once
**********************************************/
void qContentType(char *mimetype){
  static flag = 0;  
  
  if(flag)return;

  printf("Content-type: %s%c%c", mimetype, 10, 10);
  flag = 1;
}

/**********************************************
** Usage : qPrintf(Mode, Format, Arg);
** Return: Sucess number of output bytes, Fail EOF
** Do    : Print message like printf
**         Mode 0 : Same as printf(), it means Accept HTML
**         Mode 1 : Print HTML TAG
**         Mode 2 : Mode 1 + Auto Link
**         Mode 3 : Mode 2 + _top frame link
**         Mode 4 : Waste HTML TAG
**         Mode 5 : Mode 4 + Auto Link
**         Mode 6 : Mode 5 + _top frame link
**********************************************/
int qPrintf(int mode, char *format, ...){
  char buf[1000+1];
  int  status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(buf, format, arglist);
  if(status == EOF) return status;
  if(strlen(buf) > 1000)qError("qprintf : Message is too long");

  qPuts(mode, buf);

  return status;
}

/**********************************************
** Usage : qPuts(Mode, String pointer);
** Do    : print HTML link as multi mode
**         Mode 0 : Same as printf()
**         Mode 1 : Print HTML TAG
**         Mode 2 : Mode 1 + Auto Link
**         Mode 3 : Mode 2 + Auto Link to _top frame
**         Mode 4 : Waste HTML TAG
**         Mode 5 : Mode 4 + Auto Link
**         Mode 6 : Mode 5 + Auto Link to _top frame
** Note) It modify argument string...
**********************************************/
void qPuts(int mode, char *buf){

  if(mode == 0) printf("%s", buf);
  else {
    char *ptr, retstop, *target, *token;
    int printhtml, autolink, linkflag, ignoreflag;

    /* set defaults */
    printhtml = 1;
    autolink  = 1;
    target    = "_top";

    switch(mode){
      case 1 : {printhtml = 1, autolink = 0, target = ""; break;}
      case 2 : {printhtml = 1, autolink = 1, target = ""; break;}
      case 3 : {printhtml = 1, autolink = 1, target = "_top"; break;}
      case 4 : {printhtml = 0, autolink = 0, target = ""; break;}
      case 5 : {printhtml = 0, autolink = 1, target = ""; break;}
      case 6 : {printhtml = 0, autolink = 1, target = "_top"; break;}
      default: {qError("_autolink() : Invalid Mode (%d)", mode); break;}
    }

    /* &를 삽입함은 자동링크에서 & 캐릭터 전까지만 링크가 됨을 뜻하나
      &lt;와 같은 문자를 < 가아닌 &lt;로 찍기위해 필요하다 */
    token = " `(){}[]<>&\"',\r\n";

    ptr = _strtok2(buf, token, &retstop);
    for(linkflag = ignoreflag = 0; ptr != NULL;){

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

      if(printhtml == 1){
        if     (retstop == '<')  printf("&lt;");
        else if(retstop == '>')  printf("&gt;");
        else if(retstop == '\"') printf("&quot;");  /* " */
        else if(retstop == '&')  printf("&amp;");
        else if(retstop != '\0') printf("%c", retstop);
      }
      else {
        if     (retstop == '<') ignoreflag = 1;
        else if(retstop == '>') ignoreflag = 0;
        else if(ignoreflag == 0 && retstop !='\0') printf("%c", retstop);
      }

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
  va_list arglist;
  va_start(arglist, format);

  qContentType("text/html");

  status = vsprintf(buf, format, arglist);
  if(strlen(buf) > 1000 || status == EOF){
    printf("qError() : Message is too long or not valid");
    exit(1);
  }

  printf("<html>\n");
  printf("<body bgcolor=white>\n\n");
  printf("  <font color=red size=6><B>Error !!!</B></font><br><br>\n\n");
  printf("  <font size=3><b><i>%s</i></b></font><br><br>\n\n", buf);
  printf("  <center><font size=2>\n");
  printf("    [<a href='http://www.hongik.ac.kr' target=_top>Hongik University</a> <a href='http://shinan.hongik.ac.kr' target=_top>Shinan Campus</a>]\n");
  printf("    [<a href='http://hsns.hongik.ac.kr' target=_top>Hongik Shinan Network Security</a>]<br>\n");
  printf("    Made in Korea by '<a href='mailto:nobreak@shinan.hongik.ac.kr'>Seung-young, Kim</a>'<p>\n");
  printf("    <a href='javascript:history.back()'>BACK</a>\n");
  printf("  </font></center>\n\n");
  printf("</body>\n");
  printf("</html>\n");
  exit(1);
}

/**********************************************
** Usage : qCgienv(Pointer of Cgienv);
** Do    : Get environment of CGI
**********************************************/
void qCgienv(Cgienv *env){
  struct tm *envtime;

  envtime = qGetTime();

  env->server_software   = getenv("SERVER_SOFTWARE");
  env->server_name       = getenv("SERVER_NAME");
  env->gateway_interface = getenv("GATEWAY_INTERFACE");
  env->server_protocol   = getenv("SERVER_PROTOCOL");
  env->server_port       = getenv("SERVER_PORT");
  env->request_method    = getenv("REQUEST_METHOD");
  env->http_accept       = getenv("HTTP_ACCEPT");
  env->path_info         = getenv("PATH_INFO");
  env->path_translated   = getenv("PATH_TRANSLATED");
  env->script_name       = getenv("SCRIPT_NAME");
  env->query_string      = getenv("QUERY_STRING");
  env->remote_host       = getenv("REMOTE_HOST");
  env->remote_addr       = getenv("REMOTE_ADDR");
  env->remote_user       = getenv("REMOTE_USER");
  env->auth_type         = getenv("AUTH_TYPE");
  env->content_type      = getenv("CONTENT_TYPE");
  env->content_length    = getenv("CONTENT_LENGTH");

  env->year = envtime->tm_year;
  env->mon  = envtime->tm_mon;
  env->day  = envtime->tm_mday;
  env->hour = envtime->tm_hour;
  env->min  = envtime->tm_min;
  env->sec  = envtime->tm_sec;
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
** Usage : qGetGMTTime(gmt, plus_sec);
** Do    : Make string of GMT Time for Cookie
** Return: Amount second from 1970/00/00 00:00:00
**
** plus_sec will be added to current time;
**********************************************/
time_t qGetGMTTime(char *gmt, time_t plus_sec) {
  time_t nowtime;
  char *buf;
  char gmt_wdy[3+1], gmt_dd[2+1], gmt_mon[3+1], gmt_yyyy[4+1];
  char gmt_hh[2+1],  gmt_mm[2+1], gmt_ss[2+1];

  nowtime = time(NULL);
  nowtime += plus_sec;
  buf = ctime(&nowtime);

  sscanf(buf, "%3s %3s %2s %2s:%2s:%2s %4s", gmt_wdy, gmt_mon, gmt_dd, gmt_hh, gmt_mm, gmt_ss, gmt_yyyy);
  sprintf(gmt, "%3s, %2s-%3s-%4s %2s:%2s:%2s GMT", gmt_wdy, gmt_dd, gmt_mon, gmt_yyyy, gmt_hh, gmt_mm, gmt_ss);

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
  int i, dotcount;

  if(email == NULL) return 0;

  if(strlen(email) > 60) return 0;

  for(i = dotcount = 0; email[i] != '\0'; i++){
    switch(email[i]) {
      case '@' : {
        dotcount++;
        if(dotcount != 1 || i == 0) return 0;
        break;
      }
      case '.' : {
        dotcount++;
        if(dotcount < 2 || dotcount > 5) return 0;
        if(email[i - 1] == '@' || email[i-1] == '.') return 0;
        break;
      }
      default  : {
             if((email[i] >= '0') && (email[i] <= '9')) break;
        else if((email[i] >= 'A') && (email[i] <= 'Z')) break;
        else if((email[i] >= 'a') && (email[i] <= 'z')) break;
        else if((email[i] == '-') || (email[i] == '_')) break;
        else return 0;
      }
    }
  }

  if(dotcount < 2 || dotcount > 5) return 0;
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
***********************************************
** You need not use below functions
** It is used by qDecoder();
***********************************************
**********************************************/

/**********************************************
** Usage : _get_query("GET");
**         _get_query("POST");
** Return: Pointer of Query string
** Do    : Return query string which is copied
**********************************************/
char *_get_query(char *method){
  char *query;
  int cl, i;

  if(!strcmp(method, "GET")){
        if(getenv("QUERY_STRING") == NULL) return NULL;
	query = (char *)malloc(sizeof(char) * (strlen(getenv("QUERY_STRING")) + 1));
	strcpy(query, getenv("QUERY_STRING"));
	return query;
  }
  if(!strcmp(method, "POST")){
        if(getenv("REQUEST_METHOD") == NULL) return NULL;
	if(strcmp("POST", getenv("REQUEST_METHOD")))return NULL;
	cl = atoi(getenv("CONTENT_LENGTH"));
	query = (char *)malloc(sizeof(char) * (cl + 1));
	for(i = 0; i < cl; i++)query[i] = fgetc(stdin);
	query[i] = '\0';
	return query;
  }
  return NULL;
}

/**********************************************
** Usage : _decode_query(Query Pointer);
** Return: Pointer of Query string
** Do    : Decode query string
**********************************************/
void _decode_query(char *str){
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
