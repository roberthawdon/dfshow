#define PROGRAM_DESC "\nDF-SHOW: An interactive directory/file browser written for Unix-like systems.\nBased on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.\n"
#define CONF_NAME "dfshow.conf"
#define HOME_CONF_DIR ".dfshow"

#define M_NONE 0x00
#define M_NORMAL 0x01

#include <stdbool.h>
#include "customtypes.h"

void refreshScreen(char *application);
int getch10th (void);
int setDynamicWChar(wchar_t **str, const wchar_t *format, ...);
int setDynamicChar(char **str, const char *format, ...);
int splitString(splitStrStruct **result, char *input, int splitChar, bool filePath);
int createParentsInput(char *path);
void createParentDirs(char *path);
void mk_dir(char *path);
void setConfLocations();
void printVersion(char* programName);
char * dirFromPath(const char* myStr);
char * objectFromPath(const char* myStr);
int check_dir(char *pwd);
int check_file(char *file);
int check_exec(const char *object);
int check_last_char(const char *str, const char *chk);
int check_first_char(const char *str, const char *chk);
int check_numbers_only(const char *s);
char *str_replace(char *orig, char *rep, char *with);
char * read_line(FILE *fin);
void showManPage(const char * prog);
int can_run_command(const char *cmd);
char * commandFromPath(const char *cmd);
int countArguments(const char *cmd);
void buildCommandArguments(const char *cmd, char **args, size_t items);
int launchExternalCommand(char *cmd, char **args, ushort_t mode);
void sigintHandle(int sig);
int exittoshell();
