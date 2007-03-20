typedef struct Entry Entry;
struct Entry{
  char *name;
  char *value;
  struct Entry *next;
};

typedef struct Cgienv Cgienv;
struct Cgienv{
  char *server_software;
  char *server_name;
  char *gateway_interface;
  char *server_protocol;
  char *server_port;
  char *request_method;
  char *http_accept;
  char *path_info;
  char *path_translated;
  char *script_name;
  char *query_string;
  char *remote_host;
  char *remote_addr;
  char *remote_user;
  char *auth_type;
  char *content_type;
  char *content_length;
  /*** Extended Information ***/
  int  year, mon, day, hour, min, sec;
};

int       qDecoder(void);
void      qFree(void);
char      *qValue(char *name);
void      qPrint(void);
void      qContentType(char *mimetype);
void      qCgienv(Cgienv *env);
int       qCheckFile(char *filename);
struct tm *qGetTime(void);
int       qSendFile(char *filename);
void      qError(char *str);