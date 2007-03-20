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
char      *qValue(char *name);
void      qPrint(void);
void      qFree(void);

Entry     *qfDecoder(char *filename);
char      *qfValue(Entry *first, char *name);
void      qfPrint(Entry *first);
void      qfFree(Entry *first);

void      qContentType(char *mimetype);
int       qPrintf(int mode, char *format, ...);
void      qPuts(int mode, char *buf);
void      qError(char *format, ...);

void      qCgienv(Cgienv *env);
struct tm *qGetTime(void);

int       qCheckFile(char *filename);
int       qSendFile(char *filename);

int       qReadCounter(char *filename);
int       qSaveCounter(char *filename, int number);
int       qUpdateCounter(char *filename);

int       qCheckEmail(char *email);
int       qCheckURL(char *url);
char      *qRemoveSpace(char *str);
int       qStr09AZaz(char *str);
char      *qfgets(char *str, int length, FILE *fp);
