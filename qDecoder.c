/***********************************************
** [Query String Decoder Version 3.1]
**
**  Source  Code Name : qDecoder.c
**  Include Code Name : qDecoder.h
**
** Programmed by 'Kim Seung-young'
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
void _autolink(int mode, char one);
char *_strtok2(char *str, char *token, char *retstop);

/**********************************************
** Static Values Definition used only internal
**********************************************/

static Entry *_first_entry = NULL;

/**********************************************
** Usage : qDecoder();
** Return: Number of Values
** Do    : Query Decode & Save it in linked list
**         It doesn't care Method
**********************************************/
int qDecoder(void){
  Entry *entries;
  char *query;
  int  amount;

  if(_first_entry != NULL) return -1;

  entries = (Entry *)malloc(sizeof(Entry));
  _first_entry = entries -> next = entries;

  query = _get_query("GET");
  _decode_query(query);
  for(amount = 0; query && *query; amount++){
	entries = entries->next;
	entries->value = _makeword(query, '&');
	entries->name  = _makeword(entries->value, '=');
	entries->next = (Entry *)malloc(sizeof(Entry));
  }
  if(query)free(query);

  query = _get_query("POST");
  _decode_query(query);
  for(; query && *query; amount++){
	entries = entries->next;
	entries->value = _makeword(query, '&');
	entries->name  = _makeword(entries->value, '=');
	entries->next = (Entry *)malloc(sizeof(Entry));
  }
  if(query)free(query);
  if(entries->next == _first_entry){
	free(entries);
	_first_entry = NULL;
  }
  else{
	free(entries->next);
	entries->next = NULL;
  }
  return amount;
}

/**********************************************
** Usage : qValue(Name);
** Return: Pointer of value string
** Do    : Get value string pointer
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
** Return: Pointer of the first entry
** Do    : Save file into linked list
**********************************************/
Entry *qfDecoder(char *filename){
  FILE  *fp;
  Entry *first, *entries;
  char  buf[1000 + 1];

  fp = fopen(filename, "rt");
  if(fp == NULL) return NULL;    

  entries = (Entry *)malloc(sizeof(Entry));
  first = entries -> next = entries;

  while(fgets(buf, 1000 + 1, fp)){
	entries = entries->next;
	entries->value = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
        strcpy(entries->value, buf);
	entries->name  = _makeword(entries->value, '=');
	entries->next = (Entry *)malloc(sizeof(Entry));

        qRemoveSpace(entries->name);
        qRemoveSpace(entries->value);
  }
  if(entries->next == first){
	free(entries);
	first = NULL;
  }
  else{
	free(entries->next);
	entries->next = NULL;
  }

  fclose(fp);

  return first;
}

/**********************************************
** Usage : qfValue(Pointer of the first Entry, Name);
** Return: Pointer of value string
** Do    : Get value string pointer
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
** Usage : qprintf(Mode, Format, Arg);
** Do    : Print message like printf
**         Mode 0 : Same as printf(), it means Accept HTML
**         Mode 1 : Print HTML TAG, Same as mode 0
**         Mode 2 : Mode 1 + Auto Link
**         Mode 3 : Mode 2 + _top frame link
**         Mode 4 : Waste HTML TAG
**         Mode 5 : Mode 4 + Auto Link
**         Mode 6 : Mode 5 + _top frame link
**********************************************/
int qPrintf(int mode, char *format, ...){
  char buf[1000 + 1];
  int  status;
  va_list arglist;

  va_start(arglist, format);
  status = vsprintf(buf, format, arglist);
  if(status == EOF) return status;
  if(strlen(buf) > 1000)qError("qprintf : Message is too long");

  if(mode == 0) status = printf("%s", buf);
  else {
    int i;

    for(i = 0; buf[i]; i++) _autolink(0, buf[i]);
    _autolink(mode, ' ');
  }

  return status;
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

  printf("<font color=red size=6><B>Error !!!</B></font>\n");
  printf("<br><br>\n");
  printf("<font size=3 face=arial>\n");
  printf("<i><b>%s</b></i>\n", buf);
  printf("</font>\n");
  printf("<br><br>\n");
  printf("<center><font size=2 face=arial>\n");
  printf("Made in Korea by 'Kim Seung-young', [Hongik Shinan Network Security]<br>\n");
  printf("홍익대학교 신안캠퍼스 전자전산공학과 94학번 김승영<br>\n");
  printf("<br><a href=\"javascript:history.back()\">BACK</a>");
  printf("</font></center>");
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
  nowlocaltime->tm_mon++;

  return nowlocaltime;
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
** Usage : qCheckEmail(E-mail Address);
** Return: If it is valid return 1. Or return 0;
** Do    : Check E-mail address
**********************************************/
int qCheckEmail(char *email){
  char *ptr, *token, retstop, buf[60+1];
  int i, flag;

  if(strlen(email) > 60) return 0;
  strcpy(buf, email);

  token = " ~`!@#$%^&*()_-+=|\\:;\"',.?/<>{}[]\r\n";
  ptr = _strtok2(buf, token, &retstop);
  for(flag = i = 1; ptr != NULL; i++){
    if((i == 1) && (retstop != '@'))flag = 0;
    if((i >= 2) && !(retstop == '.' || retstop == '\0'))flag = 0;
    ptr = _strtok2(NULL, token, &retstop);
  }
  if((i >=4) && (i <= 7)) return flag;
  return 0;
}

/**********************************************
** Usage : qRemoveSpace(Source string);
** Do    : Remove Space before string & after string
**         Remove CR, LF
**********************************************/
void qRemoveSpace(char *str){
  int i, j;
  
  if(!str)return;

  for(i = 0; str[i] != '\0'; i++){
    if(str[i] == '\r' || str[i] == '\n') str[i] = '\0';
  }

  for(j = 0; isspace(str[j]); j++);
  for(i = 0; str[j] != '\0'; i++, j++) str[i] = str[j];
  str[i] = '\0';

  for(i--; (i >= 0) && isspace(str[i]); i--);
  str[i+1] = '\0';
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

/**********************************************
** Usage : _autolink(Mode, character);
** Do    : Buffering the character and print HTML link
**         Mode 0 : Buffering character
**         Mode 1 : Fresh buffer, Print HTML TAG
**         Mode 2 : Mode 1 + Auto Link
**         Mode 3 : Mode 2 + Auto Link to _top frame
**         Mode 4 : Fresh Buffer, Waste HTML TAG
**         Mode 5 : Mode 4 + Auto Link
**         Mode 6 : Mode 5 + Auto Link to _top frame
**********************************************/
void _autolink(int mode, char one){
  char *buf;
  static int i, flag, bufsize;

  if(mode == 0){
    if(flag == 0){
      flag = 1, i = 0, bufsize = 1000; 
      buf = (char *)malloc(sizeof(char) * (bufsize + 1));
    }
    if(i >= bufsize){
       bufsize *= 2;
       buf = (char *)realloc(buf, sizeof(char) * (bufsize + 1));
    }
    buf[i] = one, buf[++i] = '\0';
  }
  else {
    char *ptr, retstop, *target, *token;
    int printhtml, autolink, linkflag, ignoreflag;

    if(flag == 0) return;

    switch(mode){
      case 1 : {printhtml = 1, autolink = 0, target = ""; break;}
      case 2 : {printhtml = 1, autolink = 1, target = ""; break;}
      case 3 : {printhtml = 1, autolink = 1, target = "_top"; break;}
      case 4 : {printhtml = 0, autolink = 0, target = ""; break;}
      case 5 : {printhtml = 0, autolink = 1, target = ""; break;}
      case 6 : {printhtml = 0, autolink = 1, target = "_top"; break;}
      default: {qError("_autolink() : Invalid Mode (%d)", mode); break;}
    }

    if(autolink == 1) token = " `(){}[]<>\"',\r\n";
    else token = "<>\r\n";

    ptr = _strtok2(buf, token, &retstop);
    for(linkflag = ignoreflag = 0; ptr != NULL;){
      if(autolink == 1){
        if(!strncmp(ptr, "http://", 7))linkflag = 1;
        else if(!strncmp(ptr, "ftp://", 6))linkflag = 1;     
        else if(!strncmp(ptr, "telnet://", 9))linkflag = 1;
        else if(!strncmp(ptr, "mailto:", 7))linkflag = 1;
        else if(!strncmp(ptr, "news:", 5))linkflag = 1;
        else linkflag = 0;
      }
      if(linkflag == 1 && ignoreflag == 0)
        printf("<a href=\"%s\" target=\"%s\">%s</a>", ptr, target, ptr);
      else if(linkflag == 0 && ignoreflag == 0)
        printf("%s", ptr);

      if(printhtml == 1) printf("%c", retstop);
      else {
        if(retstop == '\r') printf("\r");
        else if(retstop == '\n') printf("\n");
        else if(retstop == '<') ignoreflag = 1;
        else if(retstop == '>') ignoreflag = 0;
        else if(ignoreflag  == 0) printf("%c", retstop);
      }

      ptr = _strtok2(NULL, token, &retstop);
    }
    flag = 0;
    free(buf);
  }
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
    for(j=0; j < i; j++){
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
