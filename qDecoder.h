#ifndef _QDECODER_H
#define _QDECODER_H

#include <time.h>

typedef struct Entry Entry;
struct Entry{
  char *name;
  char *value;
  struct Entry *next;
};

typedef struct Cgienv Cgienv;
struct Cgienv{
  char *auth_type;
  char *content_length;
  char *content_type;
  char *document_root;
  char *gateway_interface;
  char *http_accept;
  char *http_cookie;
  char *http_user_agent;
  char *query_string;
  char *remote_addr;
  char *remote_host;
  char *remote_user;
  char *remote_port;
  char *request_method;
  char *script_name;
  char *script_filename;
  char *server_name;
  char *server_protocol;
  char *server_port;
  char *server_software;
  char *server_admin;

  /*** Extended Information ***/
  int  year, mon, day, hour, min, sec;
};

int       qDecoder(void);
char      *qValue(char *name);
int       qiValue(char *name);
void      qPrint(void);
void      qFree(void);

Entry     *qfDecoder(char *filename);
char      *qfValue(Entry *first, char *name);
void      qfPrint(Entry *first);
void      qfFree(Entry *first);

int       qcDecoder(void);
char      *qcValue(char *name);
void      qcPrint(void);
void      qcFree(void);

void      qSetCookie(char *name, char *value, int exp_days, char *domain, char *path, char *secure);

char      *qURLencode(char *str);
void      qURLdecode(char *str);
void      qContentType(char *mimetype);
int       qPrintf(int mode, char *format, ...);
void      qPuts(int mode, char *buf);

void      qError(char *format, ...);
void      qErrorLog(char *filename);
void      qErrorContact(char *msg);

void      qCgienv(Cgienv *env);

struct tm *qGetTime(void);
time_t    qGetGMTTime(char *gmt, time_t plus_sec);

int       qCheckFile(char *filename);
int       qSendFile(char *filename);

int       qReadCounter(char *filename);
int       qSaveCounter(char *filename, int number);
int       qUpdateCounter(char *filename);

int       qCheckEmail(char *email);
int       qCheckURL(char *url);
char      *qRemoveSpace(char *str);
int       qStr09AZaz(char *str);

#endif
