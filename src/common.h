#define PROGRAM_DESC "\nDF-SHOW: An interactive directory/file browser written for Unix-like systems.\nBased on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.\n"
#define CONF_NAME "dfshow.conf"
#define HOME_CONF_DIR ".dfshow"
#include <wchar.h>

typedef struct {
  char refLabel[16];
  wchar_t displayLabel[32];
  int hotKey;
  int displayLabelSize;
} menuDef;

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
  char charSetting[16];
} settingIndex;

typedef struct {
  int index;
  char refLabel[16];
  char value[16];
} t1CharValues;

int getch10th (void);
int cmp_menu_ref(const void *lhs, const void *rhs);
int splitPath(pathDirs **dirStruct, char *path);
int createParentsInput(char *path);
void createParentDirs(char *path);
void addMenuItem(menuDef **dfMenu, int *pos, char* refLabel, wchar_t* displayLabel, int hotKey);
void updateMenuItem(menuDef **dfMenu, int *menuSize, char* refLabel, wchar_t* displayLabel);
wchar_t * genMenuDisplayLabel(wchar_t* preMenu, menuDef* dfMenu, int size, wchar_t* postMenu, int comma);
int menuHotkeyLookup(menuDef* dfMenu, char* refLabel, int size);
int altHotkey(int key);
void mk_dir(char *path);
void setConfLocations();
void printVersion(char* programName);
char * dirFromPath(const char* myStr);
char * objectFromPath(const char* myStr);
void wPrintMenu(int line, int col, wchar_t *menustring);
void printMenu(int line, int col, char *menustring);
void wPrintLine(int line, int col, wchar_t *textString);
void printLine(int line, int col, char *textString);
void topLineMessage(const char *message);
int wReadLine(wchar_t *buffer, int buflen, wchar_t *oldbuf);
int readline(char *buffer, int buflen, char *oldbuf);
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
void printToggleSetting(int line, int col, wchar_t *settingLabel, int *setting, int *items, int invert);
void addT1CharValue(t1CharValues **values, int *totalItems, int *maxItem, char *refLabel, char *value);
void importSetting(settingIndex **settings, int *items, char *refLabel, wchar_t *textLabel, int type, int intSetting, int maxValue, char *charSetting, int invert);
void updateSetting(settingIndex **settings, int index, int type, int intSetting);
int intSettingValue(int *setting, int newValue);
void printSetting(int line, int col, settingIndex **settings, t1CharValues **values, int index, int charIndex, int type, int invert);
int textValueLookup(t1CharValues **values, int *items, char *refLabel, char *value);
