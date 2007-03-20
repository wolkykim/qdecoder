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

  /*** qDecoder Supported Extended Informations ***/
  int  year, mon, day, hour, min, sec;
};

/* qDecoder C++ support */
#ifdef  __cplusplus
extern "C" {
#endif

int       qDecoder(void);
char      *qValue(char *name);
int       qiValue(char *name);
void      qPrint(void);
void      qFree(void);

Entry     *qfDecoder(char *filename);
char      *qfValue(Entry *first, char *name);
void      qfPrint(Entry *first);
void      qfFree(Entry *first);

Entry     *qsDecoder(char *str);
#define   qsValue(A, B) qfValue(A, B)
#define   qsPrint(A) qfPrint(A)
#define   qsFree(A) qfFree(A)

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
char      *qGetEnv(char *envname, char *nullstr);
char      *qCGIname(void);

struct tm *qGetTime(void);
time_t    qGetGMTime(char *gmt, time_t plus_sec);

int       qCheckFile(char *filename);
int       qSendFile(char *filename);

void      qDownload(char *filename);

int       qReadCounter(char *filename);
int       qSaveCounter(char *filename, int number);
int       qUpdateCounter(char *filename);

int       qCheckEmail(char *email);
int       qCheckURL(char *url);
char      *qRemoveSpace(char *str);
int       qStr09AZaz(char *str);

char      *qStrBig(char *str);
int       qStrFind(char *orgstr, char *tokstr);

#ifdef  __cplusplus
}
#endif

#endif
