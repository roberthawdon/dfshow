#define PROGRAM_DESC "\nDF-SHOW: An interactive directory/file browser written for Unix-like systems.\nBased on the applications from the PC-DOS DF-EDIT suite by Larry Kroeker.\n"
#define CONF_NAME "dfshow.conf"
#define HOME_CONF_DIR ".dfshow"
#include <wchar.h>

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
int exittoshell();
