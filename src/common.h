#define PROGRAM_DESC "\nDF-SHOW: An interactive directory/file browser written for Unix-like systems.\nBased on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.\n"
#define CONF_NAME "dfshow.conf"
#define HOME_CONF_DIR ".dfshow"

#define SETTING_BOOL   0
#define SETTING_SELECT 1
#define SETTING_MULTI  2
#define SETTING_FREE   3

#include <wchar.h>

typedef struct {
  char directories[256];
} pathDirs;

typedef struct {
  int type;
  char refLabel[16];
  wchar_t textLabel[32];
  int invert;
  int intSetting;
  int maxValue;
} settingIndex;

typedef struct {
  int index;
  char refLabel[16];
  char value[16];
} t1CharValues;

typedef struct {
  int index;
  char refLabel[16];
  char settingLabel[16];
  int value;
  int boolVal;
} t2BinValues;

typedef struct node {
  int val;
  struct node * next;
} node_t;

int getch10th (void);
int splitPath(pathDirs **dirStruct, char *path);
int createParentsInput(char *path);
void createParentDirs(char *path);
void mk_dir(char *path);
void setConfLocations();
void printVersion(char* programName);
char * dirFromPath(const char* myStr);
char * objectFromPath(const char* myStr);
void wPrintLine(int line, int col, wchar_t *textString);
void printLine(int line, int col, char *textString);
void topLineMessage(const char *message);
int check_dir(char *pwd);
int check_file(char *file);
void clear_workspace();
int check_exec(const char *object);
int check_last_char(const char *str, const char *chk);
int check_first_char(const char *str, const char *chk);
int check_numbers_only(const char *s);
char *str_replace(char *orig, char *rep, char *with);
char * read_line(FILE *fin);
void showManPage(const char * prog);
int can_run_command(const char *cmd);
void sigintHandle(int sig);
int exittoshell();
void addT1CharValue(t1CharValues **values, int *totalItems, int *maxItem, char *refLabel, char *value);
void addT2BinValue(t2BinValues **values, int *totalItems, int *maxItem, char *refLabel, char *settingLabel, int reset);
void populateBool(t2BinValues **values, char *refLabel, int setting, int maxValue);
void adjustBinSetting(settingIndex **settings, t2BinValues **values, char *refLabel, int *setting, int maxValue);
void importSetting(settingIndex **settings, int *items, char *refLabel, wchar_t *textLabel, int type, int intSetting, int maxValue, int invert);
void updateSetting(settingIndex **settings, int index, int type, int intSetting);
int intSettingValue(int *setting, int newValue);
void printSetting(int line, int col, settingIndex **settings, t1CharValues **values, t2BinValues **bins, int index, int charIndex, int binIndex, int type, int invert);
int textValueLookup(t1CharValues **values, int *items, char *refLabel, char *value);
